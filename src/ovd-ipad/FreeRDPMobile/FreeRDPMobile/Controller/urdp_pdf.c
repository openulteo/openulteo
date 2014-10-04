/*
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <config.h>
#include "urdp_config.h"
#include "log.h"
#include <freerdp/utils/stream.h>
#include <freerdp/utils/thread.h>
#include <freerdp/utils/svc_plugin.h>
#include <rdpdr_constants.h>
#include <printer/printer_main.h>

typedef struct urdpPdfPrinter_t urdpPdfPrinter;
struct urdpPdfPrinter_t {
	rdpPrinter printer;
	LIST* jobs;
	rdpSvcPlugin* plugin;
};

/**
 * Return the name of the job filename
 *
 * The char* returned must be freed by the caller.
 */
char* urdp_pdf_get_job_filename(const rdpPrintJob* printJob) {
	char* filename = malloc(sizeof(char) * PATH_MAX);

	sprintf(filename, "%s/printjob%d.pdf", SPOOL_DIR, printJob->id);
	return filename;
}

/// Here we have in data the PDF data as received from the server.
static void urdp_pdf_write_printjob(rdpPrintJob* printjob, uint8* data, int size) {
	FILE* fp;
	
	log_warning("Write %i job data", size);

	// Path of the PDF
	char* filename = urdp_pdf_get_job_filename(printjob);

	fp = fopen(filename, "a+b");
	if (fp == NULL) {
		log_warning("failed to open file %s", filename);
		return;
	}
	if (fwrite(data, 1, size, fp) < size) {
		log_warning("failed to write file %s", filename);
	}
	fclose(fp);
	log_warning("Write %i job data", size);

	free(filename);
}

/**
 * Simply called when the document has finished transmission
 *
 * Transmits the information to guacd that a PDF job is ready.
 */
static void urdp_pdf_close_printjob(rdpPrintJob* printjob) {
	log_warning("Close %i job", printjob->id);

	char* filename = urdp_pdf_get_job_filename(printjob);

	// Communicate filename to main loop
	RDP_EVENT* event;
	event = xnew(RDP_EVENT);
	event->event_class = PDF_PRINTER_CLASS
	event->event_type = 0;
	//event->on_event_free_callback = free;
	event->user_data = filename;
	svc_plugin_send_event(((urdpPdfPrinter*) printjob->printer)->plugin, event);

	list_remove(((urdpPdfPrinter*) printjob->printer)->jobs, printjob);
}

static rdpPrintJob* urdp_pdf_find_printjob(rdpPrinter* printer, uint32 id) {
	rdpPrintJob* printjob = (rdpPrintJob*) list_peek(((urdpPdfPrinter*) printer)->jobs);
	while (printjob) {
		if (printjob->id == id)
			return printjob;

		printjob = (rdpPrintJob*) list_next(((urdpPdfPrinter*) printer)->jobs, printjob);
	}
	return NULL;
}

static rdpPrintJob* urdp_pdf_create_printjob(rdpPrinter* printer, uint32 id) {
	rdpPrintJob* printjob = xnew(rdpPrintJob);

	if (access(SPOOL_DIR, W_OK) != 0) {
		log_error("Error , %s : cannot open to write PDF files", SPOOL_DIR);
	}

	log_info("Create job %i", id);

	printjob->id = id;
	printjob->printer = printer;

	printjob->Write = urdp_pdf_write_printjob;
	printjob->Close = urdp_pdf_close_printjob;

	list_enqueue(((urdpPdfPrinter*) printer)->jobs, printjob);

	char* filename = urdp_pdf_get_job_filename(printjob);
	FILE* fp = fopen(filename, "w+b");
	fclose(fp);
	free(filename);

	return printjob;
}

static void urdp_pdf_free_printer(rdpPrinter* printer) {
	rdpPrintJob* printjob;

	while ((printjob = list_dequeue(((urdpPdfPrinter*) printer)->jobs)) != NULL) {
		urdp_pdf_close_printjob(printjob);
	}
	list_free(((urdpPdfPrinter*) printer)->jobs);
	xfree(printer->name);
	xfree(printer);
}

/*****************************************************************************
 * FreeRDP printer_main.c
 *****************************************************************************/

typedef struct _PRINTER_DEVICE PRINTER_DEVICE;
struct _PRINTER_DEVICE {
	DEVICE device;
	rdpPrinter* printer;
	LIST* irp_list;
	freerdp_thread* thread;
};

static void printer_process_irp_create(PRINTER_DEVICE* printer_dev, IRP* irp) {
	rdpPrintJob* printjob = NULL;

	if (printer_dev->printer != NULL)
		printjob = printer_dev->printer->CreatePrintJob(printer_dev->printer, irp->devman->id_sequence++);

	if (printjob != NULL) {
		stream_write_uint32(irp->output, printjob->id);
		/* FileId */

		log_warning("printjob id: %d", printjob->id);
	} else {
		stream_write_uint32(irp->output, 0);
		/* FileId */
		irp->IoStatus = STATUS_PRINT_QUEUE_FULL;

		log_warning("error creating print job.");
	}

	irp->Complete(irp);
}

static void printer_process_irp_close(PRINTER_DEVICE* printer_dev, IRP* irp) {
	rdpPrintJob* printjob = NULL;

	if (printer_dev->printer != NULL)
		printjob = printer_dev->printer->FindPrintJob(printer_dev->printer, irp->FileId);

	if (printjob == NULL) {
		irp->IoStatus = STATUS_UNSUCCESSFUL;

		log_warning("printjob id %d not found.", irp->FileId);
	} else {
		printjob->Close(printjob);

		log_warning("printjob id %d closed.", irp->FileId);
	}

	stream_write_zero(irp->output, 4);
	/* Padding(4) */

	irp->Complete(irp);
}

static void printer_process_irp_write(PRINTER_DEVICE* printer_dev, IRP* irp) {
	rdpPrintJob* printjob = NULL;
	uint32 Length;
	uint64 Offset;

	stream_read_uint32(irp->input, Length);
	stream_read_uint64(irp->input, Offset);
	(void) Offset;
	stream_seek(irp->input, 20);
	/* Padding */

	if (printer_dev->printer != NULL)
		printjob = printer_dev->printer->FindPrintJob(printer_dev->printer, irp->FileId);

	if (printjob == NULL) {
		irp->IoStatus = STATUS_UNSUCCESSFUL;
		Length = 0;

		log_warning("printjob id %d not found.", irp->FileId);
	} else {
		printjob->Write(printjob, stream_get_tail(irp->input), Length);

		log_debug("printjob id %d written %d bytes.", irp->FileId, Length);
	}

	stream_write_uint32(irp->output, Length);
	stream_write_uint8(irp->output, 0);
	/* Padding */

	irp->Complete(irp);
}

static void printer_process_irp(PRINTER_DEVICE* printer_dev, IRP* irp) {
	switch (irp->MajorFunction) {
	case IRP_MJ_CREATE:
		printer_process_irp_create(printer_dev, irp);
		break;

	case IRP_MJ_CLOSE:
		printer_process_irp_close(printer_dev, irp);
		break;

	case IRP_MJ_WRITE:
		printer_process_irp_write(printer_dev, irp);
		break;

	default:
		log_warning("MajorFunction 0x%X not supported", irp->MajorFunction);
		irp->IoStatus = STATUS_NOT_SUPPORTED;
		irp->Complete(irp);
		break;
	}
}

static void printer_process_irp_list(PRINTER_DEVICE* printer_dev) {
	IRP* irp;

	while (1) {
		if (freerdp_thread_is_stopped(printer_dev->thread))
			break;

		freerdp_thread_lock(printer_dev->thread);
		irp = (IRP*) list_dequeue(printer_dev->irp_list);
		freerdp_thread_unlock(printer_dev->thread);

		if (irp == NULL)
			break;

		printer_process_irp(printer_dev, irp);
	}
}

static void* printer_thread_func(void* arg) {
	PRINTER_DEVICE* printer_dev = (PRINTER_DEVICE*) arg;

	while (1) {
		freerdp_thread_wait(printer_dev->thread);

		if (freerdp_thread_is_stopped(printer_dev->thread))
			break;

		freerdp_thread_reset(printer_dev->thread);
		printer_process_irp_list(printer_dev);
	}

	freerdp_thread_quit(printer_dev->thread);

	return NULL;
}

static void printer_irp_request(DEVICE* device, IRP* irp) {
	PRINTER_DEVICE* printer_dev = (PRINTER_DEVICE*) device;

	freerdp_thread_lock(printer_dev->thread);
	list_enqueue(printer_dev->irp_list, irp);
	freerdp_thread_unlock(printer_dev->thread);

	freerdp_thread_signal(printer_dev->thread);
}

static void printer_free(DEVICE* device) {
	PRINTER_DEVICE* printer_dev = (PRINTER_DEVICE*) device;
	IRP* irp;

	freerdp_thread_stop(printer_dev->thread);
	freerdp_thread_free(printer_dev->thread);

	while ((irp = (IRP*) list_dequeue(printer_dev->irp_list)) != NULL)
		irp->Discard(irp);
	list_free(printer_dev->irp_list);

	if (printer_dev->printer)
		printer_dev->printer->Free(printer_dev->printer);

	xfree(printer_dev->device.name);

	xfree(printer_dev);
}

void printer_register(PDEVICE_SERVICE_ENTRY_POINTS pEntryPoints, rdpPrinter* printer) {
	PRINTER_DEVICE* printer_dev;
	char* port;
	UNICONV* uniconv;
	uint32 Flags;
	size_t DriverNameLen;
	char* DriverName;
	size_t PrintNameLen;
	char* PrintName;
	uint32 CachedFieldsLen;
	uint8* CachedPrinterConfigData;

	port = xmalloc(10);
	snprintf(port, 10, "PRN%d", printer->id);

	printer_dev = xnew(PRINTER_DEVICE);

	printer_dev->device.type = RDPDR_DTYP_PRINT;
	printer_dev->device.name = port;
	printer_dev->device.IRPRequest = printer_irp_request;
	printer_dev->device.Free = printer_free;

	printer_dev->printer = printer;

	CachedFieldsLen = 0;
	CachedPrinterConfigData = NULL;

	log_debug("Printer '%s' registered", printer->name);

	Flags = 0;
	if (printer->is_default)
		Flags |= RDPDR_PRINTER_ANNOUNCE_FLAG_DEFAULTPRINTER;

	uniconv = freerdp_uniconv_new();
	DriverName = freerdp_uniconv_out(uniconv, printer->driver, &DriverNameLen);
	PrintName = freerdp_uniconv_out(uniconv, printer->name, &PrintNameLen);
	freerdp_uniconv_free(uniconv);

	printer_dev->device.data = stream_new(28 + DriverNameLen + PrintNameLen + CachedFieldsLen);

	stream_write_uint32(printer_dev->device.data, Flags);
	stream_write_uint32(printer_dev->device.data, 0);
	/* CodePage, reserved */
	stream_write_uint32(printer_dev->device.data, 0);
	/* PnPNameLen */
	stream_write_uint32(printer_dev->device.data, DriverNameLen + 2);
	stream_write_uint32(printer_dev->device.data, PrintNameLen + 2);
	stream_write_uint32(printer_dev->device.data, CachedFieldsLen);
	stream_write(printer_dev->device.data, DriverName, DriverNameLen);
	stream_write_uint16(printer_dev->device.data, 0);
	stream_write(printer_dev->device.data, PrintName, PrintNameLen);
	stream_write_uint16(printer_dev->device.data, 0);
	if (CachedFieldsLen > 0) {
		stream_write(printer_dev->device.data, CachedPrinterConfigData, CachedFieldsLen);
	}

	xfree(DriverName);
	xfree(PrintName);

	printer_dev->irp_list = list_new();
	printer_dev->thread = freerdp_thread_new();

	pEntryPoints->RegisterDevice(pEntryPoints->devman, (DEVICE*) printer_dev);

	freerdp_thread_start(printer_dev->thread, printer_thread_func, printer_dev);
}

int pdfDeviceServiceEntry(PDEVICE_SERVICE_ENTRY_POINTS pEntryPoints) {
	urdpPdfPrinter* pdf_printer;

	pdf_printer = xnew(urdpPdfPrinter);

	pdf_printer->printer.id = 1;
	pdf_printer->printer.name = strdup(pEntryPoints->plugin_data->data[1]);
	/* This is the PDF Ulteo driver. On printing, the printer transmits a PDF back to the client. */
	pdf_printer->printer.driver = "Ulteo TS Printer Driver";
	pdf_printer->printer.is_default = true;

	pdf_printer->printer.CreatePrintJob = urdp_pdf_create_printjob;
	pdf_printer->printer.FindPrintJob = urdp_pdf_find_printjob;
	pdf_printer->printer.Free = urdp_pdf_free_printer;

	pdf_printer->jobs = list_new();
	pdf_printer->plugin = pEntryPoints->devman->plugin;

	printer_register(pEntryPoints, (rdpPrinter*) pdf_printer);

	return 0;
}
