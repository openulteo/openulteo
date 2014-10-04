/**
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2011, 2012, 2013
 * Author Thomas MOUTON <thomas@ulteo.com> 2012
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

/*
   rdesktop: A Remote Desktop Protocol client.
   Miscellaneous protocol constants
   Copyright (C) Matthew Chapman 1999-2008

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* modified for xrdp */

#if !defined(XRDP_CONSTANTS_H)
#define XRDP_CONSTANTS_H

#define DEFAULT_CODEPAGE     "UTF-8"
#define WINDOWS_CODEPAGE     "UTF-16LE"
#define CP1252_CODEPAGE      "CP1252"

#define XRDP_SOCKET_PATH               "/var/spool/xrdp/"
#define PRINTER_SOCKET_NAME            "/var/spool/xrdp/xrdp_printer"
#define MANAGEMENT_SOCKET_NAME         "/var/spool/xrdp/xrdp_management"

#define REQUEST_ACTION_DELETE           "delete"
#define REQUEST_ACTION_ADD              "add"
#define REQUEST_ACTION_PURGE            "purge"
#define REQUEST_ACTION_DELETE           "delete"

#define REQUEST_TYPE_PRINTER            "printer"
#define REQUEST_TYPE_PRINTERS           "printers"


#define DEFAULT_CHANNEL_PRIORITY        "seamrdp,main"


/* TCP port for Remote Desktop Protocol */
#define TCP_PORT_RDP                   3389

#define ISO_PDU_CR                     0xE0 /* Connection Request */
#define ISO_PDU_CC                     0xD0 /* Connection Confirm */
#define ISO_PDU_DR                     0x80 /* Disconnect Request */
#define ISO_PDU_DT                     0xF0 /* Data */
#define ISO_PDU_ER                     0x70 /* Error */

/* MCS PDU codes */
#define MCS_EDRQ                       1  /* Erect Domain Request */
#define MCS_DPUM                       8  /* Disconnect Provider Ultimatum */
#define MCS_AURQ                       10 /* Attach User Request */
#define MCS_AUCF                       11 /* Attach User Confirm */
#define MCS_CJRQ                       14 /* Channel Join Request */
#define MCS_CJCF                       15 /* Channel Join Confirm */
#define MCS_SDRQ                       25 /* Send Data Request */
#define MCS_SDIN                       26 /* Send Data Indication */

// Negotiation Flags
#define TYPE_RDP_NEG_REQ               0x01
#define TYPE_RDP_NEG_RSP               0x02
#define RDP_NEG_REQ_LEN                0x0008
#define RDP_NEG_RSP_LEN                0x0008

// Negotiation Response flags
#define EXTENDED_CLIENT_DATA_SUPPORTED 0x01
#define DYNVC_GFX_PROTOCOL_SUPPORTED   0x02
#define RDP_NEGRSP_RESERVED            0x04

// Negociation Response protocol
#define PROTOCOL_RDP                   0x00000000
#define PROTOCOL_SSL                   0x00000001
#define PROTOCOL_HYBRID                0x00000002
#define PROTOCOL_HYBRID_EX             0x00000008

#define MCS_CONNECT_INITIAL            0x7f65
#define MCS_CONNECT_RESPONSE           0x7f66

#define BER_TAG_BOOLEAN                1
#define BER_TAG_INTEGER                2
#define BER_TAG_OCTET_STRING           4
#define BER_TAG_RESULT                 10
#define MCS_TAG_DOMAIN_PARAMS          0x30

#define MCS_GLOBAL_CHANNEL             1003
#define MCS_USERCHANNEL_BASE           1001

/* Order Flags */
#define ORDERFLAGS_EXTRA_FLAGS         0x0080
#define NEGOTIATEORDERSUPPORT          0x0002
#define ZEROBOUNDSDELTASSUPPORT        0x0008
#define COLORINDEXSUPPORT              0x0020
#define SOLIDPATTERNBRUSHONLY          0x0040

/* Order Flags ex */
#define ORDERFLAGS_EX_CACHE_BITMAP_REV3_SUPPORT    0x0002
#define ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT  0x0004

// emt packet sec flags
#define SEC_AUTODETECT_REQ             0x1000
#define SEC_AUTODETECT_RSP             0x2000

#define TYPE_ID_AUTODETECT_REQUEST     0x00
#define TYPE_ID_AUTODETECT_RESPONSE    0x01

#define SEC_AUTODETECT_REQ_LENGTH      0x6

#define RDP_RTT_REQUEST_TYPE           0x1001
#define RDP_RTT_RESPONSE_TYPE          0x0000
#define RDP_BW_CONNECT_START           0x1014
#define RDP_BW_SESSION_START           0x0014
#define RDP_BW_PAYLOAD                 0x0002
#define RDP_BW_CONNECT_STOP            0x002B
#define RDP_BW_SESSION_STOP            0x0429
#define RDP_BW_RESULTS                 0x0003

#define field_baseRTT_averageRTT       0x0840
#define field_bandwidth_averageRTT     0x0880
#define field_all                      0x08C0

/* RDP secure transport constants */
#define SEC_RANDOM_SIZE                32
#define SEC_MODULUS_SIZE               64
#define SEC_PADDING_SIZE               8
#define SEC_EXPONENT_SIZE              4

#define SEC_CLIENT_RANDOM              0x0001
#define SEC_ENCRYPT                    0x0008
#define SEC_LOGON_INFO                 0x0040
#define SEC_LICENCE_NEG                0x0080

#define SEC_TAG_SRV_INFO               0x0c01
#define SEC_TAG_SRV_CRYPT              0x0c02
#define SEC_TAG_SRV_CHANNELS           0x0c03
#define SC_MCS_MSGCHANNEL              0x0c04

#define SEC_TAG_CLI_INFO               0xc001
#define SEC_TAG_CLI_CRYPT              0xc002
#define SEC_TAG_CLI_CHANNELS           0xc003
#define SEC_TAG_CLI_4                  0xc004
#define CS_MCS_MSGCHANNEL              0xc006

#define SEC_TAG_PUBKEY                 0x0006
#define SEC_TAG_KEYSIG                 0x0008

#define SEC_RSA_MAGIC                  0x31415352 /* RSA1 */

/* RDP licensing constants */
#define LICENCE_TOKEN_SIZE             10
#define LICENCE_HWID_SIZE              20
#define LICENCE_SIGNATURE_SIZE         16

#define LICENCE_TAG_DEMAND             0x01
#define LICENCE_TAG_AUTHREQ            0x02
#define LICENCE_TAG_ISSUE              0x03
#define LICENCE_TAG_REISSUE            0x04
#define LICENCE_TAG_PRESENT            0x12
#define LICENCE_TAG_REQUEST            0x13
#define LICENCE_TAG_AUTHRESP           0x15
#define LICENCE_TAG_RESULT             0xff

#define LICENCE_TAG_USER               0x000f
#define LICENCE_TAG_HOST               0x0010

/* RDP PDU codes */
#define RDP_PDU_DEMAND_ACTIVE          1
#define RDP_PDU_CONFIRM_ACTIVE         3
#define RDP_PDU_REDIRECT               4
#define RDP_PDU_DEACTIVATE             6
#define RDP_PDU_DATA                   7

#define RDP_DATA_PDU_UPDATE            2
#define RDP_DATA_PDU_CONTROL           20
#define RDP_DATA_PDU_POINTER           27
#define RDP_DATA_PDU_INPUT             28
#define RDP_DATA_PDU_SYNCHRONISE       31
#define RDP_DATA_PDU_BELL              34
#define RDP_DATA_PDU_LOGON             38
#define RDP_DATA_PDU_FONT2             39
#define RDP_DATA_PDU_TYPE2_FONTMAP     40
#define RDP_DATA_PDU_SET_IME_STATUS    45
#define RDP_DATA_PDU_DISCONNECT        47

#define RDP_CTL_REQUEST_CONTROL        1
#define RDP_CTL_GRANT_CONTROL          2
#define RDP_CTL_DETACH                 3
#define RDP_CTL_COOPERATE              4

#define RDP_UPDATE_ORDERS              0
#define RDP_UPDATE_BITMAP              1
#define RDP_UPDATE_PALETTE             2
#define RDP_UPDATE_SYNCHRONIZE         3

#define RDP_POINTER_SYSTEM             1
#define RDP_POINTER_MOVE               3
#define RDP_POINTER_COLOR              6
#define RDP_POINTER_CACHED             7

#define RDP_NULL_POINTER               0
#define RDP_DEFAULT_POINTER            0x7F00

#define RDP_INPUT_SYNCHRONIZE          0
#define RDP_INPUT_CODEPOINT            1
#define RDP_INPUT_VIRTKEY              2
#define RDP_INPUT_SCANCODE             4
#define RDP_INPUT_MOUSE                0x8001

/* Device flags */
#define KBD_FLAG_RIGHT                 0x0001
#define KBD_FLAG_EXT                   0x0100
#define KBD_FLAG_QUIET                 0x1000
#define KBD_FLAG_DOWN                  0x4000
#define KBD_FLAG_UP                    0x8000

/* These are for synchronization; not for keystrokes */
#define KBD_FLAG_SCROLL                0x0001
#define KBD_FLAG_NUMLOCK               0x0002
#define KBD_FLAG_CAPITAL               0x0004

/* See T.128 */
#define RDP_KEYPRESS                   0
#define RDP_KEYRELEASE                 (KBD_FLAG_DOWN | KBD_FLAG_UP)

#define MOUSE_FLAG_MOVE                0x0800
#define MOUSE_FLAG_BUTTON1             0x1000
#define MOUSE_FLAG_BUTTON2             0x2000
#define MOUSE_FLAG_BUTTON3             0x4000
#define MOUSE_FLAG_BUTTON4             0x0280
#define MOUSE_FLAG_BUTTON5             0x0380
#define MOUSE_FLAG_DOWN                0x8000

/* Raster operation masks */
#define ROP2_S(rop3)                   (rop3 & 0xf)
#define ROP2_P(rop3)                   ((rop3 & 0x3) | ((rop3 & 0x30) >> 2))

#define ROP2_COPY                      0xc
#define ROP2_XOR                       0x6
#define ROP2_AND                       0x8
#define ROP2_NXOR                      0x9
#define ROP2_OR                        0xe

#define MIX_TRANSPARENT                0
#define MIX_OPAQUE                     1

#define TEXT2_VERTICAL                 0x04
#define TEXT2_IMPLICIT_X               0x20

/* RDP bitmap cache (version 2) constants */
#define BMPCACHE2_C0_CELLS             0x78
#define BMPCACHE2_C1_CELLS             0x78
#define BMPCACHE2_C2_CELLS             0x150
#define BMPCACHE2_NUM_PSTCELLS         0x9f6

#define PDU_FLAG_FIRST                 0x01
#define PDU_FLAG_LAST                  0x02

/* Maps to generalCapabilitySet in T.128 page 138 */

/* RDP capabilities */
#define RDP_CAPSET_GENERAL             1
#define RDP_CAPLEN_GENERAL             0x18
#define OS_MAJOR_TYPE_UNIX             4
#define OS_MINOR_TYPE_XSERVER          7

#define RDP_CAPSET_BITMAP              2
#define RDP_CAPLEN_BITMAP              0x1C

#define RDP_CAPSET_ORDER               3
#define RDP_CAPLEN_ORDER               0x58
#define ORDER_CAP_NEGOTIATE            2
#define ORDER_CAP_NOSUPPORT            4

#define RDP_CAPSET_BMPCACHE            4
#define RDP_CAPLEN_BMPCACHE            0x28

#define RDP_CAPSET_JPEGCACHE           99
#define RDP_CAPLEN_JPEGCACHE           0x06

#define RDP_CAPSET_CONTROL             5
#define RDP_CAPLEN_CONTROL             0x0C

#define RDP_CAPSET_ACTIVATE            7
#define RDP_CAPLEN_ACTIVATE            0x0C

#define RDP_CAPSET_POINTER             8
#define RDP_CAPLEN_POINTER             0x0a
#define RDP_CAPLEN_POINTER_MONO        0x08

#define RDP_CAPSET_SHARE               9
#define RDP_CAPLEN_SHARE               0x08

#define RDP_CAPSET_COLCACHE            10
#define RDP_CAPLEN_COLCACHE            0x08

#define RDP_CAPSET_INPUT               13
#define RDP_CAPLEN_INPUT               0x58
#define INPUT_FLAG_UNICODE             0x10
#define INPUT_FLAG_SCANCODES           0x01

#define RDP_CAPSET_FONT                14
#define RDP_CAPLEN_FONT                0x04

#define RDP_CAPSET_BRUSHCACHE          15
#define RDP_CAPLEN_BRUSHCACHE          0x08

#define RDP_CAPSET_BITMAP_OFFSCREEN    18
#define RDP_CAPLEN_BITMAP_OFFSCREEN    0x08

#define RDP_CAPSET_BMPCACHE2           19
#define RDP_CAPLEN_BMPCACHE2           0x28
#define BMPCACHE2_FLAG_PERSIST         ((long)1<<31)

#define RDP_CAPSET_VIRCHAN             20
#define RDP_CAPLEN_VIRCHAN             0x08

#define RDP_SOURCE                     "MSTSC"

#define FASTPATH_OUTPUT_SUPPORTED      0x0001

#define FASTPATH_OUTPUT_ACTION_FASTPATH  0x0
#define FASTPATH_OUTPUT_SECURE_CHECKSUM  0x1
#define FASTPATH_OUTPUT_ENCRYPTED        0x2


#define FASTPATH_OUTPUT_SECURE_CHECKSUM  0x1
#define FASTPATH_OUTPUT_ENCRYPTED        0x2

#define FASTPATH_HEADER_LENGTH 11
#define FASTPATH_UPDATE_COMPRESSED_HEADER_LENGTH 4
#define FASTPATH_UPDATE_HEADER_LENGTH 3

/* Fast Path update type */
#define FASTPATH_UPDATETYPE_ORDERS       0x0
#define FASTPATH_UPDATETYPE_BITMAP       0x1
#define FASTPATH_UPDATETYPE_PALETTE      0x2
#define FASTPATH_UPDATETYPE_SYNCHRONIZE  0x3
#define FASTPATH_UPDATETYPE_SURFCMDS     0x4
#define FASTPATH_UPDATETYPE_PTR_NULL     0x5
#define FASTPATH_UPDATETYPE_PTR_DEFAULT  0x6
#define FASTPATH_UPDATETYPE_PTR_POSITION 0x8
#define FASTPATH_UPDATETYPE_COLOR        0x9
#define FASTPATH_UPDATETYPE_CACHED       0xA
#define FASTPATH_UPDATETYPE_POINTER      0xB

/* Fast Path fragmentation flags */
#define FASTPATH_FRAGMENT_SINGLE         0x0
#define FASTPATH_FRAGMENT_LAST           0x1
#define FASTPATH_FRAGMENT_FIRST          0x2
#define FASTPATH_FRAGMENT_NEXT           0x3

/* Fast Path compression flags */
#define FASTPATH_OUTPUT_COMPRESSION_USED 0x2

/* Logon flags */
#define RDP_LOGON_AUTO                 0x0008
#define RDP_LOGON_NORMAL               0x0033
#define RDP_COMPRESSION                0x0080
#define RDP_COMPRESSION_TYPE_MASK      0x00001E00
#define RDP_LOGON_BLOB                 0x0100
#define RDP_LOGON_LEAVE_AUDIO          0x2000

#define RDP5_DISABLE_NOTHING           0x00
#define RDP5_NO_WALLPAPER              0x01
#define RDP5_NO_FULLWINDOWDRAG         0x02
#define RDP5_NO_MENUANIMATIONS         0x04
#define RDP5_NO_THEMING                0x08
#define RDP5_NO_CURSOR_SHADOW          0x20
#define RDP5_NO_CURSORSETTINGS         0x40 /* disables cursor blinking */

/* order flags */
#define NEGOTIATEORDERSUPPORT          0x0002
#define ZEROBOUNDSDELTASSUPPORT        0x0008
#define COLORINDEXSUPPORT              0x0020
#define SOLIDPATTERNBRUSHONLY          0x0040
#define ORDERFLAGS_EXTRA_FLAGS         0x0080

/* orderSupportEx Flags */
#define ORDERFLAGS_EX_CACHE_BITMAP_REV3_SUPPORT   0x0002
#define ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT 0x0004

/* alternate secondary order */
#define TS_SECONDARY                   0x02
#define TS_ALTSEC_FRAME_MARKER         0x0D

#define TS_FRAME_START                 0x00000000
#define TS_FRAME_END                   0x00000001

/* Keymap flags */
#define MapRightShiftMask              (1 << 0)
#define MapLeftShiftMask               (1 << 1)
#define MapShiftMask                   (MapRightShiftMask | MapLeftShiftMask)

#define MapRightAltMask                (1 << 2)
#define MapLeftAltMask                 (1 << 3)
#define MapAltGrMask                   MapRightAltMask

#define MapRightCtrlMask               (1 << 4)
#define MapLeftCtrlMask                (1 << 5)
#define MapCtrlMask                    (MapRightCtrlMask | MapLeftCtrlMask)

#define MapRightWinMask                (1 << 6)
#define MapLeftWinMask                 (1 << 7)
#define MapWinMask                     (MapRightWinMask | MapLeftWinMask)

#define MapNumLockMask                 (1 << 8)
#define MapCapsLockMask                (1 << 9)

#define MapLocalStateMask              (1 << 10)

#define MapInhibitMask                 (1 << 11)

#define MASK_ADD_BITS(var, mask)       (var |= mask)
#define MASK_REMOVE_BITS(var, mask)    (var &= ~mask)
#define MASK_HAS_BITS(var, mask)       ((var & mask)>0)
#define MASK_CHANGE_BIT(var, mask, active) \
                  (var = ((var & ~mask) | (active ? mask : 0)))

/* Clipboard constants, "borrowed" from GCC system headers in
   the w32 cross compiler */

#define CF_TEXT                        1
#define CF_BITMAP                      2
#define CF_METAFILEPICT                3
#define CF_SYLK                        4
#define CF_DIF                         5
#define CF_TIFF                        6
#define CF_OEMTEXT                     7
#define CF_DIB                         8
#define CF_PALETTE                     9
#define CF_PENDATA                     10
#define CF_RIFF                        11
#define CF_WAVE                        12
#define CF_UNICODETEXT                 13
#define CF_ENHMETAFILE                 14
#define CF_HDROP                       15
#define CF_LOCALE                      16
#define CF_MAX                         17
#define CF_OWNERDISPLAY                128
#define CF_DSPTEXT                     129
#define CF_DSPBITMAP                   130
#define CF_DSPMETAFILEPICT             131
#define CF_DSPENHMETAFILE              142
#define CF_PRIVATEFIRST                512
#define CF_PRIVATELAST                 767
#define CF_GDIOBJFIRST                 768
#define CF_GDIOBJLAST                  1023

/* Sound format constants */
#define WAVE_FORMAT_PCM	               1
#define WAVE_FORMAT_ADPCM              2
#define WAVE_FORMAT_ALAW               6
#define WAVE_FORMAT_MULAW              7

/* Virtual channel options */
#define CHANNEL_OPTION_INITIALIZED     0x80000000
#define CHANNEL_OPTION_ENCRYPT_RDP     0x40000000
#define CHANNEL_OPTION_COMPRESS_RDP    0x00800000
#define CHANNEL_OPTION_SHOW_PROTOCOL   0x00200000

/* NT status codes for RDPDR */
#define STATUS_SUCCESS                 0x00000000
#define STATUS_PENDING                 0x00000103

#define STATUS_NO_MORE_FILES           0x80000006
#define STATUS_DEVICE_PAPER_EMPTY      0x8000000e
#define STATUS_DEVICE_POWERED_OFF      0x8000000f
#define STATUS_DEVICE_OFF_LINE         0x80000010
#define STATUS_DEVICE_BUSY             0x80000011

#define STATUS_INVALID_HANDLE          0xc0000008
#define STATUS_INVALID_PARAMETER       0xc000000d
#define STATUS_NO_SUCH_FILE            0xc000000f
#define STATUS_INVALID_DEVICE_REQUEST  0xc0000010
#define STATUS_ACCESS_DENIED           0xc0000022
#define STATUS_OBJECT_NAME_COLLISION   0xc0000035
#define STATUS_DISK_FULL               0xc000007f
#define STATUS_FILE_IS_A_DIRECTORY     0xc00000ba
#define STATUS_NOT_SUPPORTED           0xc00000bb
#define STATUS_TIMEOUT                 0xc0000102
#define STATUS_CANCELLED               0xc0000120

/* RDPDR constants */
#define RDPDR_MAX_DEVICES              0x10
#define DEVICE_TYPE_SERIAL             0x01
#define DEVICE_TYPE_PARALLEL           0x02
#define DEVICE_TYPE_PRINTER            0x04
#define DEVICE_TYPE_DISK               0x08
#define DEVICE_TYPE_SCARD              0x20

#define FILE_DIRECTORY_FILE            0x00000001
#define FILE_NON_DIRECTORY_FILE        0x00000040
#define FILE_OPEN_FOR_FREE_SPACE_QUERY 0x00800000

/* RDP5 disconnect PDU */
#define exDiscReasonNoInfo                            0x0000
#define exDiscReasonAPIInitiatedDisconnect            0x0001
#define exDiscReasonAPIInitiatedLogoff                0x0002
#define exDiscReasonServerIdleTimeout                 0x0003
#define exDiscReasonServerLogonTimeout                0x0004
#define exDiscReasonReplacedByOtherConnection         0x0005
#define exDiscReasonOutOfMemory                       0x0006
#define exDiscReasonServerDeniedConnection            0x0007
#define exDiscReasonServerDeniedConnectionFips        0x0008
#define exDiscReasonLicenseInternal                   0x0100
#define exDiscReasonLicenseNoLicenseServer            0x0101
#define exDiscReasonLicenseNoLicense                  0x0102
#define exDiscReasonLicenseErrClientMsg               0x0103
#define exDiscReasonLicenseHwidDoesntMatchLicense     0x0104
#define exDiscReasonLicenseErrClientLicense           0x0105
#define exDiscReasonLicenseCantFinishProtocol         0x0106
#define exDiscReasonLicenseClientEndedProtocol        0x0107
#define exDiscReasonLicenseErrClientEncryption        0x0108
#define exDiscReasonLicenseCantUpgradeLicense         0x0109
#define exDiscReasonLicenseNoRemoteConnections        0x010a

#define DEFAULT_XRDP_CONNECTIVITY_CHECK_INTERVAL  60

#define RDP_ORDER_STANDARD   0x01
#define RDP_ORDER_SECONDARY  0x02
#define RDP_ORDER_BOUNDS     0x04
#define RDP_ORDER_CHANGE     0x08
#define RDP_ORDER_DELTA      0x10
#define RDP_ORDER_LASTBOUNDS 0x20
#define RDP_ORDER_SMALL      0x40
#define RDP_ORDER_TINY       0x80

#define RDP_ORDER_DESTBLT   0
#define RDP_ORDER_PATBLT    1
#define RDP_ORDER_SCREENBLT 2
#define RDP_ORDER_LINE      9
#define RDP_ORDER_RECT      10
#define RDP_ORDER_DESKSAVE  11
#define RDP_ORDER_MEMBLT    13
#define RDP_ORDER_TRIBLT    14
#define RDP_ORDER_POLYLINE  22
#define RDP_ORDER_TEXT2     27

#define RDP_ORDER_RAW_BMPCACHE  0
#define RDP_ORDER_COLCACHE      1
#define RDP_ORDER_BMPCACHE      2
#define RDP_ORDER_FONTCACHE     3
#define RDP_ORDER_RAW_BMPCACHE2 4
#define RDP_ORDER_BMPCACHE2     5
#define RDP_ORDER_BRUSHCACHE    7
#define RDP_ORDER_JPEGCACHE    99

/* drawable types */
#define WND_TYPE_BITMAP  0
#define WND_TYPE_WND     1
#define WND_TYPE_SCREEN  2
#define WND_TYPE_BUTTON  3
#define WND_TYPE_IMAGE   4
#define WND_TYPE_EDIT    5
#define WND_TYPE_LABEL   6
#define WND_TYPE_COMBO   7
#define WND_TYPE_SPECIAL 8
#define WND_TYPE_LISTBOX 9

/* button states */
#define BUTTON_STATE_UP   0
#define BUTTON_STATE_DOWN 1

/* messages */
#define WM_PAINT       3
#define WM_KEYDOWN     15
#define WM_KEYUP       16
#define WM_KEYUNICODE  17
#define WM_MOUSEMOVE   100
#define WM_LBUTTONUP   101
#define WM_LBUTTONDOWN 102
#define WM_RBUTTONUP   103
#define WM_RBUTTONDOWN 104
#define WM_BUTTON3UP   105
#define WM_BUTTON3DOWN 106
#define WM_BUTTON4UP   107
#define WM_BUTTON4DOWN 108
#define WM_BUTTON5UP   109
#define WM_BUTTON5DOWN 110
#define WM_INVALIDATE  200

#define CB_ITEMCHANGE  300

/* System limit */
#define DEFAULT_FD_LIMIT   1024

/* JPEG Buffer size */
#define JPEG_BUFFER_SIZE 16384

#define IMAGE_TILE_MAX_BUFFER_SIZE 16384 // 64*64*4

/* Image policy type */
#define IMAGE_COMP_POLICY_FULL 0
#define IMAGE_COMP_POLICY_ADAPTATIVE 1

/* tile compression type */
#define RAW_TILE 0
#define RLE_TILE 1
#define JPEG_TILE 2


#define CONNECTION_TYPE_UNKNOWN             0x00 // Unknown connection type
#define CONNECTION_TYPE_MODEM               0x01 // Modem (56 Kbps)
#define CONNECTION_TYPE_BROADBAND_LOW       0x02 // Low-speed broadband (256 Kbps - 2 Mbps)
#define CONNECTION_TYPE_SATELLITE           0x03 // Satellite (2 Mbps - 16 Mbps with high latency)
#define CONNECTION_TYPE_BROADBAND_HIGH      0x04 // High-speed broadband (2 Mbps - 10 Mbps)
#define CONNECTION_TYPE_WAN                 0x05 // WAN (10 Mbps or higher with high latency)
#define CONNECTION_TYPE_LAN                 0x06 // LAN (10 Mbps or higher)
/*    The server SHOULD attempt to detect the connection
    type. If the connection type can be successfully
    determined then the performance flags, sent by the
    client in the performanceFlags field of the Extended
    Info Packet ([MS-RDPBCGR] section 2.2.1.11.1.1.1), SHOULD be
    ignored and the server SHOULD determine the
    appropriate set of performance flags to apply to the
    remote session (based on the detected connection
    type).

      If the RNS_UD_CS_SUPPORT_NETCHAR_AUTODETECT (0x0080)
    flag is not set in the earlyCapabilityFlags field,
    then this value SHOULD be ignored.
*/
#define CONNECTION_TYPE_AUTODETECT          0x07

/* IME Status */
#define IME_STATE_CLOSED       0x000
#define IME_STATE_OPEN         0x001
#define IME_CMODE_NATIVE       0x001 // ! NATIVE = Alphanum
#define IME_CMODE_KATAKANA     0x002 // ! KATAKANA = Hiragana
#define IME_CMODE_FULLSHAPE    0x008 // ! FULLSHAPE = half-width
#define IME_CMODE_ROMAN        0x010 // Roman input
#define IME_CMODE_CHARCODE     0x020 // Character-code input
#define IME_CMODE_HANJACONVERT 0x040 // Hanja conversion enabled
#define IME_CMODE_SOFTKBD      0x080 // On-screen keyboard enabled
#define IME_CMODE_NOCONVERSION 0x100 // IME conversion is inactive
#define IME_CMODE_EUDC         0x200 // End-User Defined Character mode
#define IME_CMODE_SYMBOL       0x400 // Symbol conversion mode
#define IME_CMODE_FIXED        0x800 // Fixed conversion mode

#define FRAME_RATE_MIN      40
#define FRAME_RATE_MAX      1000

#endif
