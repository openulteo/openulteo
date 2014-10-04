/**
 * Copyright (C) 2010-2011 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechevalier <david@ulteo.com> 2010-2011
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
 **/

#if !defined(DEVREDIR_H)
#define DEVREDIR_H

#include "arch.h"
#include "parse.h"


/* config constant */
#define RDPDR_CFG_GLOBAL                        "Globals"
#define RDPDR_CFG_NAME                          "Name"
#define RDPDR_CFG_LOGGING                       "Logging"
#define RDPDR_CFG_LOG_DIR                       "LogDir"
#define RDPDR_CFG_LOG_LEVEL                     "LogLevel"
#define RDPDR_CFG_LOG_ENABLE_SYSLOG             "EnableSyslog"
#define RDPDR_CFG_LOG_SYSLOG_LEVEL              "SyslogLevel"

/* protocol message */
/* RDPDR_HEADER */
/* Component */
#define RDPDR_CTYP_CORE                         0x4472
#define RDPDR_CTYP_PRN                          0x5052
/* PacketId */
#define PAKID_CORE_SERVER_ANNOUNCE              0x496E
#define PAKID_CORE_CLIENTID_CONFIRM             0x4343
#define PAKID_CORE_CLIENT_NAME                  0x434E
#define PAKID_CORE_DEVICELIST_ANNOUNCE          0x4441
#define PAKID_CORE_DEVICE_REPLY                 0x6472
#define PAKID_CORE_DEVICE_IOREQUEST             0x4952
#define PAKID_CORE_DEVICE_IOCOMPLETION          0x4943
#define PAKID_CORE_SERVER_CAPABILITY            0x5350
#define PAKID_CORE_CLIENT_CAPABILITY            0x4350
#define PAKID_CORE_DEVICELIST_REMOVE            0x444D
#define PAKID_PRN_CACHE_DATA                    0x5043
#define PAKID_CORE_USER_LOGGEDON                0x554C
#define PAKID_PRN_USING_XPS                     0x5543


/* CAPABILITY HEADER */
/* Capability type */
#define CAP_GENERAL_TYPE                        0x0001
#define CAP_PRINTER_TYPE                        0x0002
#define CAP_PORT_TYPE                           0x0003
#define CAP_DRIVE_TYPE                          0x0004
#define CAP_SMARTCARD_TYPE                      0x0005

/* Version */
#define GENERAL_CAPABILITY_VERSION_01           0x00000001
#define GENERAL_CAPABILITY_VERSION_02           0x00000002
#define PRINT_CAPABILITY_VERSION_01             0x00000001
#define PORT_CAPABILITY_VERSION_01              0x00000001
#define DRIVE_CAPABILITY_VERSION_01             0x00000001
#define DRIVE_CAPABILITY_VERSION_02             0x00000002
#define SMARTCARD_CAPABILITY_VERSION_01          0x00000001


#define OS_TYPE_WINNT                           0x00000002

/* VersionMinor */
#define RDP6                                    0x000C
#define RDP52                                   0x000A
#define RDP51                                   0x0005
#define RDP5                                    0x0002

/* ioCode1 */
#define RDPDR_IRP_MJ_CREATE                     0x00000001
#define RDPDR_IRP_MJ_CLEANUP                    0x00000002
#define RDPDR_IRP_MJ_CLOSE                      0x00000004
#define RDPDR_IRP_MJ_READ                       0x00000008
#define RDPDR_IRP_MJ_WRITE                      0x00000010
#define RDPDR_IRP_MJ_FLUSH_BUFFERS              0x00000020
#define RDPDR_IRP_MJ_SHUTDOWN                   0x00000040
#define RDPDR_IRP_MJ_DEVICE_CONTROL             0x00000080
#define RDPDR_IRP_MJ_QUERY_VOLUME_INFORMATION   0x00000100
#define RDPDR_IRP_MJ_SET_VOLUME_INFORMATION     0x00000200
#define RDPDR_IRP_MJ_QUERY_INFORMATION          0x00000400
#define RDPDR_IRP_MJ_SET_INFORMATION            0x00000800
#define RDPDR_IRP_MJ_DIRECTORY_CONTROL          0x00001000
#define RDPDR_IRP_MJ_LOCK_CONTROL               0x00002000
#define RDPDR_IRP_MJ_QUERY_SECURITY             0x00004000
#define RDPDR_IRP_MJ_SET_SECURITY               0x00008000
#define RDPDR_IRP_MJ_ALL                        0x0000FFFF

/* extraFlag1 */
#define ENABLE_ASYNCIO                          0x00000001

/* device Type */
#define RDPDR_DTYP_SERIAL                       0x00000001
#define RDPDR_DTYP_PARALLEL                     0x00000002
#define RDPDR_DTYP_PRINT                        0x00000004
#define RDPDR_DTYP_FILESYSTEM                   0x00000008
#define RDPDR_DTYP_SMARTCARD                    0x00000020

/* io operation: major function */
#define IRP_MJ_CREATE                           0x00000000
#define IRP_MJ_CLOSE                            0x00000002
#define IRP_MJ_READ                             0x00000003
#define IRP_MJ_WRITE                            0x00000004
#define IRP_MJ_DEVICE_CONTROL                   0x0000000E
#define IRP_MJ_QUERY_VOLUME_INFORMATION         0x0000000A
#define IRP_MJ_SET_VOLUME_INFORMATION           0x0000000B
#define IRP_MJ_QUERY_INFORMATION                0x00000005
#define IRP_MJ_SET_INFORMATION                  0x00000006
#define IRP_MJ_DIRECTORY_CONTROL                0x0000000C
#define IRP_MJ_LOCK_CONTROL                     0x00000011

/* io operation: minor function */
#define IRP_MN_QUERY_DIRECTORY                  0x00000001
#define IRP_MN_NOTIFY_CHANGE_DIRECTORY          0x00000002

/* InformationClass for file */
#define FileFsVolumeInformation                 0x00000001
#define FileFsSizeInformation                   0x00000003
#define FileFsAttributeInformation              0x00000005
#define FileFsFullSizeInformation               0x00000007
#define FileFsDeviceInformation                 0x00000004
#define FileBasicInformation                    0x00000004
#define FileStandardInformation                 0x00000005
#define FileAttributeTagInformation             0x00000035



/* InformationClass for directory */
#define FileDirectoryInformation                0x00000001
#define FileFullDirectoryInformation            0x00000002
#define FileBothDirectoryInformation            0x00000003
#define FileNamesInformation                    0x0000000C

/* InformationClass for setinformation */
#define FileBasicInformation                    0x00000004
#define FileEndOfFileInformation                0x00000014
#define FileDispositionInformation              0x0000000D
#define FileRenameInformation                   0x0000000A
#define FileAllocationInformation               0x00000019


/* DesiredAccess */
/*    for File */
#define FILE_READ_DATA                          0x00000001
#define FILE_WRITE_DATA                         0x00000002
#define FILE_APPEND_DATA                        0x00000004
#define FILE_READ_EA                            0x00000008
#define FILE_WRITE_EA                           0x00000010
#define FILE_EXECUTE                            0x00000020
#define FILE_READ_ATTRIBUTES                    0x00000080
#define FILE_WRITE_ATTRIBUTES                   0x00000100
#define FILE_EXECUTE_ATTRIBUTES                 0x00001000
#define DELETE                                  0x00010000
#define READ_CONTROL                            0x00020000
#define WRITE_DAC                               0x00040000
#define WRITE_OWNER                             0x00080000
#define SYNCHRONIZE                             0x00100000
#define ACCESS_SYSTEM_SECURITY                  0x01000000
#define MAXIMUM_ALLOWED                         0x02000000
#define GENERIC_ALL                             0x10000000
#define GENERIC_EXECUTE                         0x20000000
#define GENERIC_WRITE                           0x40000000
#define GENERIC_READ                            0x80000000

/*    for Directory */
#define FILE_LIST_DIRECTORY                     0x00000001
#define FILE_ADD_FILE                           0x00000002
#define FILE_ADD_SUBDIRECTORY                   0x00000004
#define FILE_READ_EA                            0x00000008
#define FILE_WRITE_EA                           0x00000010
#define FILE_TRAVERSE                           0x00000020
#define FILE_DELETE_CHILD                       0x00000040
#define FILE_READ_ATTRIBUTES                    0x00000080
#define FILE_WRITE_ATTRIBUTES                   0x00000100
#define DELETE                                  0x00010000
#define READ_CONTROL                            0x00020000
#define WRITE_DAC                               0x00040000
#define WRITE_OWNER                             0x00080000
#define SYNCHRONIZE                             0x00100000
#define ACCESS_SYSTEM_SECURITY                  0x01000000
#define MAXIMUM_ALLOWED                         0x02000000
#define GENERIC_ALL                             0x10000000
#define GENERIC_EXECUTE                         0x20000000
#define GENERIC_WRITE                           0x40000000
#define GENERIC_READ                            0x80000000

/* FileAttributes */
#define FILE_ATTRIBUTE_ARCHIVE                  0x00000020
#define FILE_ATTRIBUTE_COMPRESSED               0x00000800
#define FILE_ATTRIBUTE_DIRECTORY                0x00000010
#define FILE_ATTRIBUTE_ENCRYPTED                0x00004000
#define FILE_ATTRIBUTE_HIDDEN                   0x00000002
#define FILE_ATTRIBUTE_NORMAL                   0x00000080
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED      0x00002000
#define FILE_ATTRIBUTE_OFFLINE                  0x00001000
#define FILE_ATTRIBUTE_READONLY                 0x00000001
#define FILE_ATTRIBUTE_REPARSE_POINT            0x00000400
#define FILE_ATTRIBUTE_SPARSE_FILE              0x00000200
#define FILE_ATTRIBUTE_SYSTEM                   0x00000004
#define FILE_ATTRIBUTE_TEMPORARY                0x00000100

/* sharedAccess */
#define FILE_SHARE_READ                         0x00000001
#define FILE_SHARE_WRITE                        0x00000002
#define FILE_SHARE_DELETE                       0x00000004

/* CreateDisposition */
#define FILE_SUPERSEDE                          0x00000000
#define FILE_OPEN                               0x00000001
#define FILE_CREATE                             0x00000002
#define FILE_OPEN_IF                            0x00000003
#define FILE_OVERWRITE                          0x00000004
#define FILE_OVERWRITE_IF                       0x00000005

/* CreateOptions */
#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008
#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_RANDOM_ACCESS                      0x00000800
#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000
#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000


struct device
{
	int device_id;
	int device_type;
	int ready;
};


typedef struct {
	int device;
	char path[256];
	int last_req;
	int request_param;
	int file_id ;
	int message_id;
} Action;

#endif
