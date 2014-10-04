/**
 * FreeRDP: A Remote Desktop Protocol client.
 * Print Virtual Channel - Ulteo Open Virtual Desktop PDF printer
 *
 *
 * Copyright 2012 Ulteo SAS http://www.ulteo.com
 *    Author: Jocelyn DELALANDE <j.delalande@ulteo.com>
 *
 * Inspired by printer_cups.h - Copyright 2010-2011 Vic Lee
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <freerdp/utils/memory.h>
#include <freerdp/utils/svc_plugin.h>

#include "rdpdr_constants.h"
#include "rdpdr_types.h"
#include "printer_main.h"

#include "printer_ulteo_pdf.h"

typedef struct rdp_ulteo_pdf_printer_driver rdpUlteoPdfPrinterDriver;
typedef struct rdp_ulteo_pdf_printer rdpUlteoPdfPrinter;
typedef struct rdp_ulteo_pdf_print_job rdpUlteoPdfPrintJob;

struct rdp_ulteo_pdf_printer_driver {
	rdpPrinterDriver driver;
	int id_sequence;
};

struct rdp_ulteo_pdf_printer {
	rdpPrinter printer;
	rdpPrintJob* printjob;
	int spool_fifo;
	char spool_dir[MAX_PATH_SIZE];
};

/**
 * rm -rf command (utility)
 */
static int rmrf(const char *path) {
	DIR *d = opendir(path);
	size_t path_len = strlen(path);
	int r = -1;
	if (d) {
		struct dirent *p;
		r = 0;
		while (!r && (p=readdir(d))) {
			int r2 = -1;
			char *buf;
			size_t len;

			/* Skip the names "." and ".." as we don't want to recurse on them. */
			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
				continue;
			}

			len = path_len + strlen(p->d_name) + 2;
			buf = malloc(len);

			if (buf) {
				struct stat statbuf;
				snprintf(buf, len, "%s/%s", path, p->d_name);

				if (!stat(buf, &statbuf)) {
					if (S_ISDIR(statbuf.st_mode)) {
						r2 = rmrf(buf);
					}
					else {
						r2 = unlink(buf);
					}
				}
				free(buf);
			}

			r = r2;
		}
		closedir(d);
	}
	if (!r) {
		r = rmdir(path);
	}
	return r;
}

/**
 * Single-instance (singleton-style)
 * It's not a printer driver but a printer-manager driver.
 */
static rdpUlteoPdfPrinterDriver* ulteo_pdf_driver = NULL;

/**
 * Return the name of the job filename
 *
 * The char* returned must be freed by the caller.
 */
char* printer_ulteo_pdf_get_job_filename(const rdpPrintJob* printJob) {
	rdpUlteoPdfPrinter* printer = (rdpUlteoPdfPrinter*)printJob->printer;
	char* filename = malloc(MAX_PATH_SIZE);

	snprintf(filename, MAX_PATH_SIZE, "%s/%08x.pdf", printer->spool_dir, printJob->id);
	return filename;
}

/**
 * Write the PDF data in a file
 */
static void printer_ulteo_pdf_write_printjob(rdpPrintJob* printjob, uint8* data, int size) {
	FILE* fp;
	char* filename = printer_ulteo_pdf_get_job_filename(printjob);

	fp = fopen(filename, "a+b");
	if (fp == NULL) {
		DEBUG_WARN("Ulteo printer : Failed to open %s", filename);
		return;
	}

	if (fwrite(data, 1, size, fp) < size) {
		DEBUG_WARN("Ulteo printer : Failed to write %s", filename);
	}

	fclose(fp);
	DEBUG_WARN("Ulteo printer : Writing to %s (%i bytes)", filename, size);

	free(filename);
}

/**
 * Write has finished
 */
static void printer_ulteo_pdf_close_printjob(rdpPrintJob* printjob) {
	char* filename = printer_ulteo_pdf_get_job_filename(printjob);
	rdpUlteoPdfPrinter* printer = (rdpUlteoPdfPrinter*)(printjob->printer);

	DEBUG_WARN("Ulteo printer : Closing %s", filename);

	/* Communicate with guacd */
	if (write(printer->spool_fifo, filename, strlen(filename)+1) <= 0) {
		DEBUG_WARN("Ulteo printer : Can't write in fifo %s", printer->spool_dir);
		perror("reason : ");
	}

	free(filename);
	((rdpUlteoPdfPrinter*)printjob->printer)->printjob = NULL;
}

/**
 * New printer job
 */
static rdpPrintJob* printer_ulteo_pdf_create_printjob(rdpPrinter* rdp_printer, uint32 id) {
	rdpUlteoPdfPrinter* printer = (rdpUlteoPdfPrinter*)rdp_printer;
	rdpPrintJob* printjob = xnew(rdpPrintJob);

	/* Can write in spool ? */
	if (access(printer->spool_dir, W_OK) != 0) {
		DEBUG_WARN("Ulteo printer : Can't open the spool directory (%s)", printer->spool_dir);
		return NULL;
	}

	/* No other job in progress ? */
	if (printer->printjob != NULL) {
		DEBUG_WARN("Ulteo printer : Job in progress. Refusing task");
		return NULL;
	}

	/* Job struct */
	printjob->id = (uint32) clock();
	printjob->printer = rdp_printer;

	DEBUG_WARN("Ulteo printer : New job %x", printjob->id);

	/* Callbacks */
	printjob->Write = printer_ulteo_pdf_write_printjob;
	printjob->Close = printer_ulteo_pdf_close_printjob;

	/* Printer current job */
	printer->printjob = printjob;

	return printjob;
}

/**
 * Return job by id
 */
static rdpPrintJob* printer_ulteo_pdf_find_printjob(rdpPrinter* rdp_printer, uint32 id) {
	rdpUlteoPdfPrinter* printer = (rdpUlteoPdfPrinter*)rdp_printer;

	if ((printer->printjob == NULL) ||  (printer->printjob->id != id))
		return NULL;

	return (rdpPrintJob*)printer->printjob;
}

/**
 * Destructor : Unload printer
 */
static void printer_ulteo_pdf_free_printer(rdpPrinter* rdp_printer) {
	rdpUlteoPdfPrinter* printer = (rdpUlteoPdfPrinter*)rdp_printer;

	/* Free job (if any) */
	if (printer->printjob)
		printer->printjob->Close((rdpPrintJob*)printer->printjob);

	/* Remove spool dir */
	rmrf(printer->spool_dir);

	xfree(rdp_printer->name);
	xfree(printer);
}

/**
 * Constructor : Load the printer
 */
static rdpPrinter* printer_ulteo_pdf_new_printer(rdpUlteoPdfPrinterDriver* ulteo_pdf_driver, const char* name, boolean is_default) {
	rdpUlteoPdfPrinter* printer;

	printer = xnew(rdpUlteoPdfPrinter);

	/* Description */
	printer->printer.id = ulteo_pdf_driver->id_sequence++;
	printer->printer.name = name ? xstrdup(name) : "Ulteo OVD Printer";
	printer->printer.driver = "Ulteo TS Printer Driver";
	printer->printer.is_default = is_default;

	/* Callbacks */
	printer->printer.CreatePrintJob = printer_ulteo_pdf_create_printjob;
	printer->printer.FindPrintJob = printer_ulteo_pdf_find_printjob;
	printer->printer.Free = printer_ulteo_pdf_free_printer;

	/* spool is set later in extrainit() */
	printer->spool_fifo = -1;

	return (rdpPrinter*)printer;
}

/**
 * Enum installed printers
 */
static rdpPrinter** printer_ulteo_pdf_enum_printers(rdpPrinterDriver* driver) {
	rdpPrinter** printers;
	
	printers = (rdpPrinter**)xzalloc(sizeof(rdpPrinter*) * 2);
	printers[0] = printer_ulteo_pdf_new_printer((rdpUlteoPdfPrinterDriver*)driver, "Ulteo OVD Printer",true);
	return printers;
}

/**
 * Factory
 */
static rdpPrinter* printer_ulteo_pdf_get_printer(rdpPrinterDriver* driver, const char* name) {
	rdpUlteoPdfPrinterDriver* ulteo_pdf_driver = (rdpUlteoPdfPrinterDriver*)driver;

	return (rdpPrinter*)(printer_ulteo_pdf_new_printer(ulteo_pdf_driver, name, ulteo_pdf_driver->id_sequence == 1 ? true : false));
}

/**
 * Stage 2 initializer
 */
static int printer_ulteo_pdf_extra_init(char** plugin_data, rdpPrinter* rdp_printer) {
	rdpUlteoPdfPrinter* printer = (rdpUlteoPdfPrinter*)rdp_printer;
	rdp_guac_client_data* client_data = (rdp_guac_client_data*) plugin_data[3];
	rdpSettings* settings = client_data->settings;
	char* path = malloc(MAX_PATH_SIZE);
	char* fifo = malloc(MAX_PATH_SIZE);
	int fd_read, fd_write;

	snprintf(path, MAX_PATH_SIZE, "%s/%s", FREERDP_ULTEO_SPOOL_PATH, settings->username);
	snprintf(fifo, MAX_PATH_SIZE, "%s/%s/fifo", FREERDP_ULTEO_SPOOL_PATH, settings->username);

	/* Make the path */
	if (mkdir(path, 0755) != 0) {
		if (errno == EEXIST) {
			rmrf(path);
			if (mkdir(path, 0755) != 0) {;
				perror("Cannot create session spool dir: ");
				goto error_1;
			}
		} else {
			perror("Cannot create session spool dir: ");
			goto error_1;
		}
	}

	/* Check permissions */
	if (access(path, W_OK) != 0) {
		perror("Cannot open spool dir: ");
		goto error_2;
	}

	/* Make the Fifo */
	if (mkfifo(fifo, S_IRWXU) != 0) {
		perror("Cannot create spool fifo: ");
		goto error_2;
	}

	/* Open the fifo for Reading */
	/* nonblock mode, instead, we will block forever on open() */
	fd_read = open(fifo, O_RDONLY|O_NONBLOCK);
	if (fd_read <= 0) {
		perror("Cannot open FIFO for reading: ");
		goto error_2;
	}

	/* Open the fifo for writing */
	fd_write = open(fifo, O_WRONLY);
	if (fd_write <= 0) {
		perror("Cannot open FIFO for writing: ");
		goto error_2;
	}

	/* Configure the printer */
	strncpy(printer->spool_dir, path, MAX_PATH_SIZE);
	printer->spool_fifo = fd_write;

	/* Configure the guacamole context */
	client_data->printjob_notif_fifo = fd_read;

	return 0;

	error_2:
		rmrf(path);
	error_1:
		printer->spool_dir[0] = '\0';
		printer->spool_fifo = -1;
		client_data->printjob_notif_fifo = -1;
		free(path);
		free(fifo);
		return -1;
}

/**
 * Driver constructor (singleton style)
 */
rdpPrinterDriver* printer_ulteo_pdf_get_driver(void) {
	if (ulteo_pdf_driver == NULL) {
		ulteo_pdf_driver = xnew(rdpUlteoPdfPrinterDriver);

		/* Callbacks */
		ulteo_pdf_driver->driver.EnumPrinters = printer_ulteo_pdf_enum_printers;
		ulteo_pdf_driver->driver.GetPrinter = printer_ulteo_pdf_get_printer;
		ulteo_pdf_driver->driver.ExtraInit = printer_ulteo_pdf_extra_init;

		ulteo_pdf_driver->id_sequence = 1;
	}

	return (rdpPrinterDriver*)ulteo_pdf_driver;
}

