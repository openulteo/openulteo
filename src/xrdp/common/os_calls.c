/**
 * Copyright (C) 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2012
 * Author Thomas MOUTON <thomas@ulteo.com> 2012
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2012
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
   Copyright (c) 2004-2009 Jay Sorg

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   generic operating system calls

   put all the os / arch define in here you want
*/

#if defined(HAVE_CONFIG_H)
#include "config_ac.h"
#endif
#if defined(_WIN32)
#include <windows.h>
#include <winsock.h>
#else
/* fix for solaris 10 with gcc 3.3.2 problem */
#if defined(sun) || defined(__sun)
#define ctid_t id_t
#endif
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <time.h>
#include <grp.h>
#include <dirent.h>
#include <sys/mman.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <locale.h>

#include "os_calls.h"
#include "arch.h"

/* for clearenv() */
#if defined(_WIN32)
#else
extern char** environ;
#endif

#define SESSIONS_DIR 	"/var/spool/sessions"
#define XRDP_DIR 	"/var/spool/xrdp"


/* for solaris */
#if !defined(PF_LOCAL)
#define PF_LOCAL AF_UNIX
#endif
#if !defined(INADDR_NONE)
#define INADDR_NONE ((unsigned long)-1)
#endif

/*****************************************************************************/
void APP_CC
g_init(void)
{
#if defined(_WIN32)
  WSADATA wsadata;

  WSAStartup(2, &wsadata);
#endif
  setlocale(LC_CTYPE, "");
}

int APP_CC
g_is_root(){
	#if defined(_WIN32)
		//not implemented
		return 0;
	#else
	char* user = getenv("USER");
	return strncmp(user, "root", g_strlen(user));
	#endif

}

/*****************************************************************************/
void APP_CC
g_deinit(void)
{
#if defined(_WIN32)
  WSACleanup();
#endif
}

/*****************************************************************************/
/* allocate memory, returns a pointer to it, size bytes are allocated,
   if zero is non zero, each byte will be set to zero */
void* APP_CC
g_malloc(int size, int zero)
{
  char* rv;

  rv = (char*)malloc(size);
  if (zero)
  {
    if (rv != 0)
    {
      memset(rv, 0, size);
    }
  }
  return rv;
}

/*****************************************************************************/
/* free the memory pointed to by ptr, ptr can be zero */
void APP_CC
g_free(void* ptr)
{
  if (ptr != 0)
  {
    free(ptr);
  }
}

/*****************************************************************************/
/* output text to stdout, try to use g_write / g_writeln instead to avoid
   linux / windows EOL problems */
void DEFAULT_CC
g_printf(const char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stdout, format, ap);
  va_end(ap);
}

/*****************************************************************************/
int DEFAULT_CC
g_sprintf(char* dest, const char* format, ...)
{
  va_list ap;
  int size;
  va_start(ap, format);
  size = vsprintf(dest, format, ap);
  va_end(ap);
  return size;
}

/*****************************************************************************/
void DEFAULT_CC
g_snprintf(char* dest, int len, const char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  vsnprintf(dest, len, format, ap);
  va_end(ap);
}

/*****************************************************************************/
void DEFAULT_CC
g_writeln(const char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stdout, format, ap);
  va_end(ap);
#if defined(_WIN32)
  g_printf("\r\n");
#else
  g_printf("\n");
#endif
}

/*****************************************************************************/
void DEFAULT_CC
g_write(const char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stdout, format, ap);
  va_end(ap);
}

/*****************************************************************************/
/* produce a hex dump */
void APP_CC
g_hexdump(char* p, int len)
{
  unsigned char* line;
  int i;
  int thisline;
  int offset;

  line = (unsigned char*)p;
  offset = 0;
  while (offset < len)
  {
    g_printf("%04x ", offset);
    thisline = len - offset;
    if (thisline > 16)
    {
      thisline = 16;
    }
    for (i = 0; i < thisline; i++)
    {
      g_printf("%02x ", line[i]);
    }
    for (; i < 16; i++)
    {
      g_printf("   ");
    }
    for (i = 0; i < thisline; i++)
    {
      g_printf("%c", (line[i] >= 0x20 && line[i] < 0x7f) ? line[i] : '.');
    }
    g_writeln("");
    offset += thisline;
    line += thisline;
  }
}

/*****************************************************************************/
void APP_CC
g_memset(void* ptr, int val, int size)
{
  memset(ptr, val, size);
}

/*****************************************************************************/
void APP_CC
g_memcpy(void* d_ptr, const void* s_ptr, int size)
{
  memcpy(d_ptr, s_ptr, size);
}

/*****************************************************************************/
int APP_CC
g_getchar(void)
{
  return getchar();
}

/*****************************************************************************/
int APP_CC
g_tcp_set_no_delay(int sck)
{
#if defined(_WIN32)
  int option_value;
  int option_len;
#else
  int option_value;
  unsigned int option_len;
#endif

  option_len = sizeof(option_value);
  /* SOL_TCP IPPROTO_TCP */
  if (getsockopt(sck, IPPROTO_TCP, TCP_NODELAY, (char*)&option_value,
                 &option_len) == 0)
  {
    if (option_value == 0)
    {
      option_value = 1;
      option_len = sizeof(option_value);
      setsockopt(sck, IPPROTO_TCP, TCP_NODELAY, (char*)&option_value,
                 option_len);
    }
  }
  return 0;
}

/*****************************************************************************/
/* returns a newly created socket or -1 on error */
int APP_CC
g_tcp_socket(void)
{
#if defined(_WIN32)
  int rv;
  int option_value;
  int option_len;
#else
  int rv;
  int option_value;
  unsigned int option_len;
#endif

  /* in win32 a socket is an unsigned int, in linux, its an int */
  rv = (int)socket(PF_INET, SOCK_STREAM, 0);
  if (rv < 0)
  {
    return -1;
  }
  option_len = sizeof(option_value);
  if (getsockopt(rv, SOL_SOCKET, SO_REUSEADDR, (char*)&option_value,
                 &option_len) == 0)
  {
    if (option_value == 0)
    {
      option_value = 1;
      option_len = sizeof(option_value);
      setsockopt(rv, SOL_SOCKET, SO_REUSEADDR, (char*)&option_value,
                 option_len);
    }
  }
  option_len = sizeof(option_value);
  if (getsockopt(rv, SOL_SOCKET, SO_SNDBUF, (char*)&option_value,
                 &option_len) == 0)
  {
    if (option_value < (1024 * 32))
    {
      option_value = 1024 * 32;
      option_len = sizeof(option_value);
      setsockopt(rv, SOL_SOCKET, SO_SNDBUF, (char*)&option_value,
                 option_len);
    }
  }
  return rv;
}

/*****************************************************************************/
int APP_CC
g_tcp_local_socket(void)
{
#if defined(_WIN32)
  return 0;
#else
  return socket(PF_LOCAL, SOCK_STREAM, 0);
#endif
}

/*****************************************************************************/
void APP_CC
g_tcp_close(int sck)
{
  if (sck == 0)
  {
    return;
  }
  shutdown(sck, 2);
#if defined(_WIN32)
  closesocket(sck);
#else
  close(sck);
#endif
}

/*****************************************************************************/
/* returns error, zero is good */
int APP_CC
g_tcp_connect(int sck, const char* address, const char* port)
{
  struct sockaddr_in s;
  struct hostent* h;

  g_memset(&s, 0, sizeof(struct sockaddr_in));
  s.sin_family = AF_INET;
  s.sin_port = htons((tui16)atoi(port));
  s.sin_addr.s_addr = inet_addr(address);
  if (s.sin_addr.s_addr == INADDR_NONE)
  {
    h = gethostbyname(address);
    if (h != 0)
    {
      if (h->h_name != 0)
      {
        if (h->h_addr_list != 0)
        {
          if ((*(h->h_addr_list)) != 0)
          {
            s.sin_addr.s_addr = *((int*)(*(h->h_addr_list)));
          }
        }
      }
    }
  }
  return connect(sck, (struct sockaddr*)&s, sizeof(struct sockaddr_in));
}

/*****************************************************************************/
/* returns error, zero is good */
int APP_CC
g_tcp_local_connect(int sck, const char* port)
{
#if defined(_WIN32)
  return -1;
#else
  struct sockaddr_un s;

  memset(&s, 0, sizeof(struct sockaddr_un));
  s.sun_family = AF_UNIX;
  strcpy(s.sun_path, port);
  return connect(sck, (struct sockaddr*)&s, sizeof(struct sockaddr_un));
#endif
}

/*****************************************************************************/
int APP_CC
g_tcp_set_blocking(int sck)
{
  unsigned long i;
  i = fcntl(sck, F_GETFL);
  i = i | ~O_NONBLOCK;
  fcntl(sck, F_SETFL, i);
  return 0;
}

/*****************************************************************************/
int APP_CC
g_tcp_set_non_blocking(int sck)
{
  unsigned long i;

#if defined(_WIN32)
  i = 1;
  ioctlsocket(sck, FIONBIO, &i);
#else
  i = fcntl(sck, F_GETFL);
  i = i | O_NONBLOCK;
  fcntl(sck, F_SETFL, i);
#endif
  return 0;
}

/*****************************************************************************/
/* returns error, zero is good */
int APP_CC
g_tcp_bind(int sck, char* port)
{
  struct sockaddr_in s;

  memset(&s, 0, sizeof(struct sockaddr_in));
  s.sin_family = AF_INET;
  s.sin_port = htons((tui16)atoi(port));
  s.sin_addr.s_addr = INADDR_ANY;
  return bind(sck, (struct sockaddr*)&s, sizeof(struct sockaddr_in));
}

/*****************************************************************************/
int APP_CC
g_tcp_local_bind(int sck, char* port)
{
#if defined(_WIN32)
  return -1;
#else
  struct sockaddr_un s;

  memset(&s, 0, sizeof(struct sockaddr_un));
  s.sun_family = AF_UNIX;
  strcpy(s.sun_path, port);
  return bind(sck, (struct sockaddr*)&s, sizeof(struct sockaddr_un));
#endif
}

/*****************************************************************************/
/* returns the fd limit ,0 for unlimited or -1 if there is error */
int APP_CC
g_get_fd_limit()
{
  int limit = sysconf(_SC_OPEN_MAX);
  if (limit == -1) {
    if (errno == EINVAL) {
      return -1;
    }
    return 0;
  }
  return limit;
}

/*****************************************************************************/
/* returns error, zero is good */
int APP_CC
g_tcp_listen(int sck)
{
  return listen(sck, 2);
}

/*****************************************************************************/
int APP_CC
g_tcp_accept(int sck)
{
  struct sockaddr_in s;
#if defined(_WIN32)
  signed int i;
#else
  unsigned int i;
#endif

  i = sizeof(struct sockaddr_in);
  memset(&s, 0, i);
  return accept(sck, (struct sockaddr*)&s, &i);
}

/*****************************************************************************/
void APP_CC
g_sleep(int msecs)
{
#if defined(_WIN32)
  Sleep(msecs);
#else
  usleep(msecs * 1000);
#endif
}

/*****************************************************************************/
int APP_CC
g_tcp_last_error_would_block(int sck)
{
#if defined(_WIN32)
  return WSAGetLastError() == WSAEWOULDBLOCK;
#else
  return (errno == EWOULDBLOCK) || (errno == EAGAIN) || (errno == EINPROGRESS);
#endif
}

/*****************************************************************************/
int APP_CC
g_tcp_recv(int sck, void* ptr, int len, int flags)
{
#if defined(_WIN32)
  return recv(sck, (char*)ptr, len, flags);
#else
  int size_read = 0;
  int res = 0;

  do
  {
    res = recv(sck, ptr + size_read, len - size_read, flags|MSG_NOSIGNAL|MSG_WAITALL);

    if (res <= 0)
    {
      return res;
    }
    size_read += res;
    if (size_read != len && errno != 0)
    {
      return size_read;
    }
  }
  while (size_read < len);

  return size_read;
#endif
}

/*****************************************************************************/
int APP_CC
g_tcp_send(int sck, const void* ptr, int len, int flags)
{
#if defined(_WIN32)
  return send(sck, (const char*)ptr, len, flags);
#else
  int size_send = 0;
  int res = 0;

  do
  {
    res = send(sck, ptr + size_send, len - size_send, flags|MSG_NOSIGNAL);
    if (res <= 0)
    {
    	return res;
    }
    size_send += res;
    if (res != len && errno != 0)
    {
      return size_send;
    }
  }
  while (size_send < len);


  return size_send;
#endif
}

/*****************************************************************************/
/* returns boolean */
int APP_CC
g_tcp_socket_ok(int sck)
{
#if defined(_WIN32)
  int opt;
  int opt_len;
#else
  int opt;
  unsigned int opt_len;
#endif

  opt_len = sizeof(opt);
  if (getsockopt(sck, SOL_SOCKET, SO_ERROR, (char*)(&opt), &opt_len) == 0)
  {
    if (opt == 0)
    {
      return 1;
    }
  }
  return 0;
}

/*****************************************************************************/
/* wait 'millis' milliseconds for the socket to be able to write */
/* returns boolean */
int APP_CC
g_tcp_can_send(int sck, int millis)
{
  fd_set wfds;
  struct timeval time;
  int rv;

  time.tv_sec = millis / 1000;
  time.tv_usec = (millis * 1000) % 1000000;
  FD_ZERO(&wfds);
  if (sck > 0)
  {
    FD_SET(((unsigned int)sck), &wfds);
    rv = select(sck + 1, 0, &wfds, 0, &time);
    if (rv > 0)
    {
      return g_tcp_socket_ok(sck);
    }
  }
  return 0;
}

/*****************************************************************************/
/* wait 'millis' milliseconds for the socket to be able to receive */
/* returns boolean */
int APP_CC
g_tcp_can_recv(int sck, int millis)
{
  fd_set rfds;
  struct timeval time;
  int rv;

  time.tv_sec = millis / 1000;
  time.tv_usec = (millis * 1000) % 1000000;
  FD_ZERO(&rfds);
  if (sck > 0)
  {
    FD_SET(((unsigned int)sck), &rfds);
    rv = select(sck + 1, &rfds, 0, 0, &time);
    if (rv > 0)
    {
      return g_tcp_socket_ok(sck);
    }
  }
  return 0;
}

/*****************************************************************************/
/* create a unix socket */
int APP_CC
g_create_unix_socket(const char *socket_filename)
{
  int sock;
  struct sockaddr_un addr;

  /* Create socket */
  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    return 1;
  }
  g_memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  g_strncpy(addr.sun_path, socket_filename, sizeof(addr.sun_path));
  unlink(socket_filename);
  if (bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0)
  {
    return 1;
  }
  /* Listen on the socket */
  if (listen(sock, 5) < 0)
  {
    return 1;
  }
  return sock;
}

/*****************************************************************************/
/* return the struct ucred which informs about the processus user identity */
/* return 0 on success */
int APP_CC
g_unix_get_socket_user_cred(int sock, uid_t* uid)
{
  socklen_t cred_len = 0;
  struct ucred cred;

  if (uid == NULL)
  {
    return 1;
  }

  cred_len = sizeof(struct ucred);

  if (getsockopt(sock, SOL_SOCKET, SO_PEERCRED, (char*)&cred, &cred_len) == 0)
  {
    *uid = cred.uid;
    return 0;
  }

  return 1;
}

/*****************************************************************************/
/* connect to a socket unix */
int APP_CC
g_unix_connect(const char* socket_filename)
{
  int sock, len;
  struct sockaddr_un saun;

  /* Create socket */
  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    return 0;
  }
  /* Connect to server */
  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, socket_filename);
  len = sizeof(saun.sun_family) + strlen(saun.sun_path);
  if (connect(sock, (struct sockaddr *) &saun, len) < 0)
  {
  	close(sock);
  	return 0;
  }
  return sock;
}


/*****************************************************************************/
/* wait a connection on a unix socket*/
int APP_CC
g_wait_connection(int server_socket)
{
  fd_set rfds;
  int slaves, client;
  struct sockaddr_un fsaun;
  socklen_t fromlen;
  FD_ZERO(&rfds);
  FD_SET(server_socket, &rfds);

  /* See if any slaves are trying to connect. */
  slaves = select(server_socket + 1, &rfds, NULL, NULL, NULL);
  if (slaves == -1)
  {
    return -1;
  }
  /* Return if no waiting slaves */
  else if (slaves == 0)
  {
    return 0;
  }
  /* Accept connection */
  fromlen = sizeof(fsaun);
  client = accept(server_socket, (struct sockaddr *) &fsaun, &fromlen);
  if (client <= 0 )
  {
    //close(server_socket);
    return -1;
  }
  return client;
}

/*****************************************************************************/
int APP_CC
g_tcp_select(int sck1, int sck2)
{
  fd_set rfds;
  struct timeval time;
  int max;
  int rv;

  time.tv_sec = 0;
  time.tv_usec = 0;
  FD_ZERO(&rfds);
  if (sck1 > 0)
  {
    FD_SET(((unsigned int)sck1), &rfds);
  }
  if (sck2 > 0)
  {
    FD_SET(((unsigned int)sck2), &rfds);
  }
  max = sck1;
  if (sck2 > max)
  {
    max = sck2;
  }
  rv = select(max + 1, &rfds, 0, 0, &time);
  if (rv > 0)
  {
    rv = 0;
    if (FD_ISSET(((unsigned int)sck1), &rfds))
    {
      rv = rv | 1;
    }
    if (FD_ISSET(((unsigned int)sck2), &rfds))
    {
      rv = rv | 2;
    }
  }
  else
  {
    rv = 0;
  }
  return rv;
}

/*****************************************************************************/
/* returns 0 on error */
tbus APP_CC
g_create_wait_obj(char* name)
{
#ifdef _WIN32
  tbus obj;

  obj = (tbus)CreateEvent(0, 1, 0, name);
  return obj;
#else
  tbus obj;
  struct sockaddr_un sa;
  int len;
  int sck;
  int i;

  if( g_directory_exist(XRDP_DIR) < 0)
  {
  	g_mkdir(XRDP_DIR);
  	g_chmod_hex(XRDP_DIR, 0xffff);
  }
  sck = socket(PF_UNIX, SOCK_DGRAM, 0);
  if (sck < 0)
  {
    return 0;
  }
  memset(&sa, 0, sizeof(sa));
  sa.sun_family = AF_UNIX;
  if ((name == 0) || (strlen(name) == 0))
  {
    g_random((char*)&i, sizeof(i));
    sprintf(sa.sun_path, "%s/auto%8.8x", XRDP_DIR, i);
    while (g_file_exist(sa.sun_path))
    {
      g_random((char*)&i, sizeof(i));
      sprintf(sa.sun_path, "%s/auto%8.8x", XRDP_DIR, i);
    }
  }
  else
  {
    sprintf(sa.sun_path, "%s/%s", XRDP_DIR, name);
  }
  unlink(sa.sun_path);
  len = sizeof(sa);
  if (bind(sck, (struct sockaddr*)&sa, len) < 0)
  {
    close(sck);
    return 0;
  }
  obj = (tbus)sck;
  return obj;
#endif
}

/*****************************************************************************/
/* returns 0 on error */
tbus APP_CC
g_create_wait_obj_from_socket(tbus socket, int write)
{
#ifdef _WIN32
  /* Create and return corresponding event handle for WaitForMultipleObjets */
  WSAEVENT event;
  long lnetevent;

  event = WSACreateEvent();
  lnetevent = (write ? FD_WRITE : FD_READ) | FD_CLOSE;
  if (WSAEventSelect(socket, event, lnetevent) == 0)
  {
    return (tbus)event;
  }
  else
  {
    return 0;
  }
#else
  return socket;
#endif
}

/*****************************************************************************/
void APP_CC
g_delete_wait_obj_from_socket(tbus wait_obj)
{
#ifdef _WIN32
  if (wait_obj == 0)
  {
    return;
  }
  WSACloseEvent((HANDLE)wait_obj);
#else
#endif
}

/*****************************************************************************/
/* returns error */
int APP_CC
g_set_wait_obj(tbus obj)
{
#ifdef _WIN32
  if (obj == 0)
  {
    return 0;
  }
  SetEvent((HANDLE)obj);
  return 0;
#else
  socklen_t sa_size;
  int s;
  struct sockaddr_un sa;

  if (obj == 0)
  {
    return 0;
  }
  if (g_tcp_can_recv((int)obj, 0))
  {
    /* already signalled */
    return 0;
  }
  sa_size = sizeof(sa);
  if (getsockname((int)obj, (struct sockaddr*)&sa, &sa_size) < 0)
  {
    return 1;
  }
  s = socket(PF_UNIX, SOCK_DGRAM, 0);
  if (s < 0)
  {
    return 1;
  }
  sendto(s, "sig", 4, 0, (struct sockaddr*)&sa, sa_size);
  close(s);
  return 0;
#endif
}

/*****************************************************************************/
/* returns error */
int APP_CC
g_reset_wait_obj(tbus obj)
{
#ifdef _WIN32
  if (obj == 0)
  {
    return 0;
  }
  ResetEvent((HANDLE)obj);
  return 0;
#else
  char buf[64];

  if (obj == 0)
  {
    return 0;
  }
  while (g_tcp_can_recv((int)obj, 0))
  {
    recvfrom((int)obj, &buf, 64, 0, 0, 0);
  }
  return 0;
#endif
}

/*****************************************************************************/
/* returns boolean */
int APP_CC
g_is_wait_obj_set(tbus obj)
{
#ifdef _WIN32
  if (obj == 0)
  {
    return 0;
  }
  if (WaitForSingleObject((HANDLE)obj, 0) == WAIT_OBJECT_0)
  {
    return 1;
  }
  return 0;
#else
  if (obj == 0)
  {
    return 0;
  }
  return g_tcp_can_recv((int)obj, 0);
#endif
}

/*****************************************************************************/
/* returns error */
int APP_CC
g_delete_wait_obj(tbus obj)
{
#ifdef _WIN32
  if (obj == 0)
  {
    return 0;
  }
  /* Close event handle */
  CloseHandle((HANDLE)obj);
  return 0;
#else
  socklen_t sa_size;
  struct sockaddr_un sa;

  if (obj == 0)
  {
    return 0;
  }
  sa_size = sizeof(sa);
  if (getsockname((int)obj, (struct sockaddr*)&sa, &sa_size) < 0)
  {
    return 1;
  }
  close((int)obj);
  unlink(sa.sun_path);
  return 0;
#endif
}

/*****************************************************************************/
/* returns error */
int APP_CC
g_obj_wait(tbus* read_objs, int rcount, tbus* write_objs, int wcount,
           int mstimeout)
{
#ifdef _WIN32
  HANDLE handles[256];
  DWORD count;
  DWORD error;
  int j;
  int i;

  j = 0;
  count = rcount + wcount;
  for (i = 0; i < rcount; i++)
  {
    handles[j++] = (HANDLE)(read_objs[i]);
  }
  for (i = 0; i < wcount; i++)
  {
    handles[j++] = (HANDLE)(write_objs[i]);
  }
  if (mstimeout < 1)
  {
    mstimeout = INFINITE;
  }
  error = WaitForMultipleObjects(count, handles, FALSE, mstimeout);
  if (error == WAIT_FAILED)
  {
    return 1;
  }
  return 0;
#else
  fd_set rfds;
  fd_set wfds;
  struct timeval time;
  struct timeval* ptime;
  int i;
  int max;
  int sck;

  max = 0;
  if (mstimeout < 1)
  {
    ptime = 0;
  }
  else
  {
    time.tv_sec = mstimeout / 1000;
    time.tv_usec = (mstimeout % 1000) * 1000;
    ptime = &time;
  }
  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
  for (i = 0; i < rcount; i++)
  {
    sck = (int)(read_objs[i]);
    FD_SET(sck, &rfds);
    if (sck > max)
    {
      max = sck;
    }
  }
  for (i = 0; i < wcount; i++)
  {
    sck = (int)(write_objs[i]);
    FD_SET(sck, &wfds);
    if (sck > max)
    {
      max = sck;
    }
  }
  i = select(max + 1, &rfds, &wfds, 0, ptime);
  if (i < 0)
  {
    /* these are not really errors */
    if ((errno == EAGAIN) ||
        (errno == EWOULDBLOCK) ||
        (errno == EINPROGRESS) ||
        (errno == EINTR)) /* signal occurred */
    {
      return 0;
    }
    return 1;
  }
  return 0;
#endif
}

/*****************************************************************************/
void APP_CC
g_random(char* data, int len)
{
#if defined(_WIN32)
  int index;

  srand(g_time1());
  for (index = 0; index < len; index++)
  {
    data[index] = (char)rand(); /* rand returns a number between 0 and
                                   RAND_MAX */
  }
#else
  int fd;

  memset(data, 0x44, len);
  fd = open("/dev/urandom", O_RDONLY);
  if (fd == -1)
  {
    fd = open("/dev/random", O_RDONLY);
  }
  if (fd != -1)
  {
    if (read(fd, data, len) != len)
    {
    }
    close(fd);
  }
#endif
}

/*****************************************************************************/
int APP_CC
g_abs(int i)
{
  return abs(i);
}

/*****************************************************************************/
int APP_CC
g_memcmp(const void* s1, const void* s2, int len)
{
  return memcmp(s1, s2, len);
}

/*****************************************************************************/
/* returns -1 on error, else return handle or file descriptor */
int APP_CC
g_file_open(const char* file_name)
{
#if defined(_WIN32)
  return (int)CreateFileA(file_name, GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
#else
  int rv;

  rv =  open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (rv == -1)
  {
    /* can't open read / write, try to open read only */
    rv =  open(file_name, O_RDONLY);
  }
  return rv;
#endif
}

/*****************************************************************************/
/* returns -1 on error, else return 0 */
int APP_CC
g_create_symlink(const char* source, const char* dest)
{
#if defined(_WIN32)
  return -1;
#else
  return symlink(source, dest);
#endif
}

/*****************************************************************************/
/* returns -1 on error, else return handle or file descriptor */
int APP_CC
g_file_append(const char* file_name)
{
#if defined(_WIN32)
  return (int)CreateFileA(file_name, GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
#else
  return  open(file_name, O_APPEND | O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
#endif
}


/*****************************************************************************/
/* returns error, always 0 */
int APP_CC
g_file_close(int fd)
{
#if defined(_WIN32)
  CloseHandle((HANDLE)fd);
#else
  close(fd);
#endif
  return 0;
}

/*****************************************************************************/
/* read from file, returns the number of bytes read or -1 on error */
int APP_CC
g_file_read(int fd, char* ptr, int len)
{
#if defined(_WIN32)
  if (ReadFile((HANDLE)fd, (LPVOID)ptr, (DWORD)len, (LPDWORD)&len, 0))
  {
    return len;
  }
  else
  {
    return -1;
  }
#else
  return read(fd, ptr, len);
#endif
}

/*****************************************************************************/
/* write to file, returns the number of bytes writen or -1 on error */
int APP_CC
g_file_write(int fd, char* ptr, int len)
{
#if defined(_WIN32)
  if (WriteFile((HANDLE)fd, (LPVOID)ptr, (DWORD)len, (LPDWORD)&len, 0))
  {
    return len;
  }
  else
  {
    return -1;
  }
#else
  return write(fd, ptr, len);
#endif
}

/*****************************************************************************/
/* copy file, returns the number of bytes written or -1 on error */
int APP_CC
g_file_copy(const char* src, const char *dst)
{
  int fd_to, fd_from;
  char buf[4096];
  ssize_t nread;
  int saved_errno;
  struct stat stat_in;

  fd_from = open(src, O_RDONLY);
  if (fd_from < 0)
    return -1;

  if (fstat(fd_from, &stat_in) < 0)
    return -1;

  fd_to = open(dst, O_WRONLY | O_CREAT | O_EXCL, stat_in.st_mode & 0777);
  if (fd_to < 0)
    goto out_error;

  while ((nread = read(fd_from, buf, sizeof buf)) > 0)
  {
    char *out_ptr = buf;
    ssize_t nwritten;

    do {
      nwritten = write(fd_to, out_ptr, nread);

      if (nwritten >= 0)
      {
        nread -= nwritten;
        out_ptr += nwritten;
      }
      else if (errno != EINTR)
      {
        goto out_error;
      }
    } while (nread > 0);
  }

  if (nread == 0)
  {
    if (close(fd_to) < 0)
    {
      fd_to = -1;
      goto out_error;
    }
    close(fd_from);

    /* Success! */
    return 0;
  }

  out_error:
  saved_errno = errno;

  close(fd_from);
  if (fd_to >= 0)
    close(fd_to);

  errno = saved_errno;
  return -1;
}

/*****************************************************************************/
/* copy file with mmap, returns the number of bytes written or -1 on error */
int APP_CC
g_file_copy_mmap(const char* src, const char *dst)
{
  int fd_in, fd_out;
  char *p_in, *p_out;
  struct stat stat_in;
  int ret = -1;

  if ((src == NULL) || (dst == NULL))
    goto end0;

  if ((fd_in = open(src, O_RDONLY)) == -1)
    goto end0;

  if (fstat(fd_in, &stat_in) < 0)
    goto end1;

  if ((fd_out = open(dst, O_RDWR | O_CREAT | O_EXCL, stat_in.st_mode & 0777)) == -1)
    goto end1;

  if (lseek(fd_out, stat_in.st_size-1, SEEK_SET) == -1)
    goto end2;

  if (write(fd_out, "", 1) != 1)
    goto end2;

  if ((p_in = mmap(0, stat_in.st_size, PROT_READ, MAP_SHARED, fd_in, 0)) == MAP_FAILED)
    goto end2;

  if ((p_out = mmap(0, stat_in.st_size, PROT_WRITE, MAP_SHARED, fd_out, 0)) == MAP_FAILED)
    goto end3;

  memcpy(p_out, p_in, stat_in.st_size);
  ret = 0;

  munmap(p_out, stat_in.st_size);

  end3: munmap(p_in, stat_in.st_size);
  end2: close(fd_out);
  end1: close(fd_in);
  end0: return ret;
}

/*****************************************************************************/
/* move file pointer, returns offset on success, -1 on failure */
int APP_CC
g_file_seek(int fd, int offset)
{
#if defined(_WIN32)
  int rv;

  rv = (int)SetFilePointer((HANDLE)fd, offset, 0, FILE_BEGIN);
  if (rv == (int)INVALID_SET_FILE_POINTER)
  {
    return -1;
  }
  else
  {
    return rv;
  }
#else
  return (int)lseek(fd, offset, SEEK_SET);
#endif
}

/*****************************************************************************/
/* get the file size */
int APP_CC
g_file_size(char* filename)
{
	struct stat filestatus;
	stat( filename, &filestatus );
	return filestatus.st_size;
}

/*****************************************************************************/
/* do a write lock on a file */
/* return boolean */
int APP_CC
g_file_lock(int fd, int start, int len)
{
#if defined(_WIN32)
  return LockFile((HANDLE)fd, start, 0, len, 0);
#else
  struct flock lock;

  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = start;
  lock.l_len = len;
  if (fcntl(fd, F_SETLK, &lock) == -1)
  {
    return 0;
  }
  return 1;
#endif
}

/*****************************************************************************/
/* returns error */
int APP_CC
g_chmod_hex(const char* filename, int flags)
{
#if defined(_WIN32)
  return 0;
#else
  int fl;

  fl = 0;
  fl |= (flags & 0x4000) ? S_ISUID : 0;
  fl |= (flags & 0x2000) ? S_ISGID : 0;
  fl |= (flags & 0x1000) ? S_ISVTX : 0;
  fl |= (flags & 0x0400) ? S_IRUSR : 0;
  fl |= (flags & 0x0200) ? S_IWUSR : 0;
  fl |= (flags & 0x0100) ? S_IXUSR : 0;
  fl |= (flags & 0x0040) ? S_IRGRP : 0;
  fl |= (flags & 0x0020) ? S_IWGRP : 0;
  fl |= (flags & 0x0010) ? S_IXGRP : 0;
  fl |= (flags & 0x0004) ? S_IROTH : 0;
  fl |= (flags & 0x0002) ? S_IWOTH : 0;
  fl |= (flags & 0x0001) ? S_IXOTH : 0;
  return chmod(filename, fl);
#endif
}

int APP_CC
g_chown(const char* filename, const char* user)
{
	int uid;
	if (g_getuser_info(user, 0, &uid, 0, 0, 0) == 1)
	{
		return -1;
	}
	return chown(filename, uid, -1) ;
}

/*****************************************************************************/
/* make possible creation of a file by creating missing dirs */
int APP_CC
g_make_access(const char* filename)
{
	if (g_mkdirs(filename) == 0)
	{
		return g_remove_dir(filename);
	}
	return 1;
}


/*****************************************************************************/
/* returns error, always zero */
int APP_CC
g_mkdirs(const char* dirname)
{
	char *pos;
	char* dirname_copy;
	struct stat st;
	dirname_copy = malloc(strlen(dirname)+1);
	sprintf(dirname_copy,"%s/", dirname);
	/* first / */
	pos = strchr(dirname_copy,'/');
	if (pos == NULL)
	{
		return 1;
	}
	/* first usable position */
	pos = strchr(pos+1,'/');
	while (pos != NULL)
	{
		*pos = 0;
		mkdir(dirname_copy, S_IRWXU);
		if ( stat(dirname_copy, &st) == -1)
		{
		  return 1;
		}
		*pos = '/';
		pos = strchr(pos+1,'/');
	}
	return 0;
}

int APP_CC
g_mkdir(const char* dirname)
{
#if defined(_WIN32)
  return 0;
#else
  mkdir(dirname, S_IRWXU);
  return 0;
#endif
}


/*****************************************************************************/
/* gets the current working directory and puts up to maxlen chars in
   dirname 
   always returns 0 */
char* APP_CC
g_get_current_dir(char* dirname, int maxlen)
{
#if defined(_WIN32)
  GetCurrentDirectoryA(maxlen, dirname);
  return 0;
#else
  if (getcwd(dirname, maxlen) == 0)
  {
  }
  return 0;
#endif
}

/*****************************************************************************/
/* returns error, zero on success and -1 on failure */
int APP_CC
g_set_current_dir(char* dirname)
{
#if defined(_WIN32)
  if (SetCurrentDirectoryA(dirname))
  {
    return 0;
  }
  else
  {
    return -1;
  }
#else
  return chdir(dirname);
#endif
}


/*****************************************************************************/
/* returns boolean, non zero if the file exists */
int APP_CC
g_file_exist(const char* filename)
{
#if defined(_WIN32)
  return 0; // use FileAge(filename) <> -1
#else
  return access(filename, F_OK) == 0;
#endif
}

/*****************************************************************************/
/* returns boolean, non zero if the directory exists */
int APP_CC
g_directory_exist(const char* dirname)
{
#if defined(_WIN32)
  return 0; // use GetFileAttributes and check return value
            // is not -1 and FILE_ATTRIBUT_DIRECTORY bit is set
#else
  struct stat st;

  if (stat(dirname, &st) == 0)
  {
    return S_ISDIR(st.st_mode);
  }
  else
  {
    return 0;
  }
#endif
}

/*****************************************************************************/
/* returns boolean */
int APP_CC
g_create_dir(const char* dirname)
{
#if defined(_WIN32)
  return CreateDirectoryA(dirname, 0); // test this
#else
  return mkdir(dirname, (mode_t)-1) == 0;
#endif
}

/*****************************************************************************/
/* returns boolean */
int APP_CC
g_remove_dir(const char* dirname)
{
#if defined(_WIN32)
  return RemoveDirectoryA(dirname); // test this
#else
  return rmdir(dirname) == 0;
#endif
}

/*****************************************************************************/
/* returns boolean */
int APP_CC
g_remove_dirs(const char* dir_path)
{
	char path[256];
	struct dirent *dir_entry;
	DIR *dir;
	dir = opendir(dir_path );
	if( dir == NULL)
	{
		return 1;
	}
	while((dir_entry = readdir(dir)) != NULL)
	{
		if( g_strcmp(dir_entry->d_name, ".") == 0 || g_strcmp(dir_entry->d_name, "..") == 0)
		{
			continue;
		}
		g_sprintf(path, "%s/%s", dir_path, dir_entry->d_name);
		if(g_directory_exist(path))
		{
			g_remove_dirs(path);
		}
		else
		{
			g_file_delete(path);
		}
	}
	closedir(dir);
	return g_remove_dir(dir_path);
}

/*****************************************************************************/
/* returns non zero if the file was deleted */
int APP_CC
g_file_delete(const char* filename)
{
#if defined(_WIN32)
  return DeleteFileA(filename);
#else
  return unlink(filename) != -1;
#endif
}

/*****************************************************************************/
/* returns file size, -1 on error */
int APP_CC
g_file_get_size(const char* filename)
{
#if defined(_WIN32)
  return -1;
#else
  struct stat st;

  if (stat(filename, &st) == 0)
  {
    return (int)(st.st_size);
  }
  else
  {
    return -1;
  }
#endif
}

/*****************************************************************************/
/* returns length of text */
int APP_CC
g_strlen(const char* text)
{
  if (text == 0)
  {
    return 0;
  }
  return strlen(text);
}

/*****************************************************************************/
/* returns dest */
char* APP_CC
g_strcpy(char* dest, const char* src)
{
  if (src == 0 && dest != 0)
  {
    dest[0] = 0;
    return dest;
  }
  if (dest == 0 || src == 0)
  {
    return 0;
  }
  return strcpy(dest, src);
}

/*****************************************************************************/
/* returns dest */
char* APP_CC
g_strncpy(char* dest, const char* src, int len)
{
  return strncpy(dest, src, len);
}

/*****************************************************************************/
/* returns dest */
char* APP_CC
g_strcat(char* dest, const char* src)
{
  if (dest == 0 || src == 0)
  {
    return dest;
  }
  return strcat(dest, src);
}

/*****************************************************************************/
/* if in = 0, return 0 else return newly alloced copy of in */
char* APP_CC
g_strdup(const char* in)
{
  int len;
  char* p;

  if (in == 0)
  {
    return 0;
  }
  len = g_strlen(in);
  p = (char*)g_malloc(len + 1, 0);
  g_strcpy(p, in);
  return p;
}

/*****************************************************************************/
char* APP_CC
g_strstr(const char* c1, const char* c2)
{
	return strstr(c1,c2);
}

/*****************************************************************************/
char* APP_CC
g_strchr(const char* c1, int c)
{
	return strchr(c1,c);
}

/*****************************************************************************/
int APP_CC
g_strcmp(const char* c1, const char* c2)
{
  return strcmp(c1, c2);
}

/*****************************************************************************/
int APP_CC
g_strncmp(const char* c1, const char* c2, int len)
{
  return strncmp(c1, c2, len);
}

/*****************************************************************************/
int APP_CC
g_strcasecmp(const char* c1, const char* c2)
{
#if defined(_WIN32)
  return stricmp(c1, c2);
#else
  return strcasecmp(c1, c2);
#endif
}

/*****************************************************************************/
int APP_CC
g_strncasecmp(const char* c1, const char* c2, int len)
{
#if defined(_WIN32)
  return strnicmp(c1, c2, len);
#else
  return strncasecmp(c1, c2, len);
#endif
}

/*****************************************************************************/
int APP_CC
g_str_replace_first(char * buffer, char * s, char * by)
{
	char * p = strstr(buffer, s);
	if (p != NULL)
	{
		size_t len_p = strlen(p);
		size_t len_s = strlen(s);
		size_t len_by = strlen(by);
		if (len_s != len_by)
		{
			memmove(p + len_by, p + len_s, len_p - len_s + 1);
		}
		strncpy(p, by, len_by);
		return 0;
	}
	return 1;
}

/*****************************************************************************/
int APP_CC
g_str_replace_all(char * buffer, char * s, char * by) {
	while (g_str_replace_first(buffer, s, by) == 0);

	return 0;
}

/*****************************************************************************/
int APP_CC
g_str_end_with(char * buffer, char * end)
{
	int buffer_len = strlen(buffer);
	int end_len = strlen(end);
	int increment = 0;

	if (end_len > buffer_len)
	{
		return 1;
	}
	increment = buffer_len - end_len;
	return strncmp(buffer + increment, end, end_len);

}

/*****************************************************************************/
int APP_CC
g_atoi(char* str)
{
  if (str == 0)
  {
    return 0;
  }
  return atoi(str);
}

/*****************************************************************************/
long APP_CC
g_atol(char* str)
{
  if (str == 0)
  {
    return 0;
  }
  return atol(str);
}

/*****************************************************************************/
int APP_CC
g_htoi(char* str)
{
  int len;
  int index;
  int rv;
  int val;
  int shift;

  rv = 0;
  len = strlen(str);
  index = len - 1;
  shift = 0;
  while (index >= 0)
  {
    val = 0;
    switch (str[index])
    {
      case '1':
        val = 1;
        break;
      case '2':
        val = 2;
        break;
      case '3':
        val = 3;
        break;
      case '4':
        val = 4;
        break;
      case '5':
        val = 5;
        break;
      case '6':
        val = 6;
        break;
      case '7':
        val = 7;
        break;
      case '8':
        val = 8;
        break;
      case '9':
        val = 9;
        break;
      case 'a':
      case 'A':
        val = 10;
        break;
      case 'b':
      case 'B':
        val = 11;
        break;
      case 'c':
      case 'C':
        val = 12;
        break;
      case 'd':
      case 'D':
        val = 13;
        break;
      case 'e':
      case 'E':
        val = 14;
        break;
      case 'f':
      case 'F':
        val = 15;
        break;
    }
    rv = rv | (val << shift);
    index--;
    shift += 4;
  }
  return rv;
}

/*****************************************************************************/
int APP_CC
g_pos(char* str, const char* to_find)
{
  char* pp;

  pp = strstr(str, to_find);
  if (pp == 0)
  {
    return -1;
  }
  return (pp - str);
}

/*****************************************************************************/
int APP_CC
g_mbstowcs(twchar* dest, const char* src, int n)
{
  wchar_t* ldest;
  int rv;

  ldest = (wchar_t*)dest;
  rv = mbstowcs(ldest, src, n);
  return rv;
}

/*****************************************************************************/
int APP_CC
g_wcstombs(char* dest, const twchar* src, int n)
{
  const wchar_t* lsrc;
  int rv;

  lsrc = (const wchar_t*)src;
  rv = wcstombs(dest, lsrc, n);
  return rv;
}

/*****************************************************************************/
int APP_CC
g_split(char* str, struct token* tokens, char c)
{
  int count = 0;
  int size = 0;
  char* p = 0;
  if (strlen(str)==0)
  {
    return 0;
  }
  char* str_cpy = str;
  p = strchr(str_cpy,c);
  while(p != NULL)
  {
	//printf("the charactere : %c\n",*p);
	size = p-str_cpy;
	//printf("size : %i\n",size);
	if (size > 0 )
	{
      strncpy(tokens[count].str, str_cpy, size);
      tokens[count].str[size] = 0;
      //printf("token : %s\n",tokens[count].str);
      tokens[count].next = &tokens[count+1];
      count++;
	}
	str_cpy += size+1;
    p = strchr(str_cpy,c);
  }
  if (strlen (str_cpy)>0)
  {
	//printf("toto : %s\n",str_cpy);
    strncpy(tokens[count].str, str_cpy, strlen(str_cpy));
    count++;
  }
  //printf("Token count : %i\n",count);
  tokens[count].next = 0;
  return count;
}

struct list* APP_CC
g_str_split_to_list(const char* str, const char c)
{
  int size = 0;
  char* p = 0;
  if (! str || strlen(str) == 0)
  {
    return NULL;
  }

  struct list* tokensList = list_create();
  tokensList->auto_free = 1;

  const char* str_cpy = str;
  do
  {
    p = strchr(str_cpy, c);
    if (p == NULL)
    {
      size = g_strlen(str_cpy);
    }
    else
    {
      size = p-str_cpy;
    }

    if (size > 0)
    {
      char* token_buffer = g_malloc(sizeof(char) * (size + 1), 0);

      strncpy(token_buffer, str_cpy, size);
      token_buffer[size] = '\0';

      list_add_item(tokensList, (tbus) token_buffer);
    }
    str_cpy += size+1;
  }
  while(p != NULL);

  return tokensList;
}

/*****************************************************************************/
/* returns error */
/* trim spaces and tabs, anything <= space */
/* trim_flags 1 trim left, 2 trim right, 3 trim both, 4 trim through */
/* this will always shorten the string or not change it */
int APP_CC
g_strtrim(char* str, int trim_flags)
{
  int index;
  int len;
  int text1_index;
  int got_char;
  wchar_t* text;
  wchar_t* text1;

  len = mbstowcs(0, str, 0);
  if (len < 1)
  {
    return 0;
  }
  if ((trim_flags < 1) || (trim_flags > 4))
  {
    return 1;
  }
  text = (wchar_t*)malloc(len * sizeof(wchar_t) + 8);
  text1 = (wchar_t*)malloc(len * sizeof(wchar_t) + 8);
  text1_index = 0;
  mbstowcs(text, str, len + 1);
  switch (trim_flags)
  {
    case 4: /* trim through */
      for (index = 0; index < len; index++)
      {
        if (text[index] > 32)
        {
          text1[text1_index] = text[index];
          text1_index++;
        }
      }
      text1[text1_index] = 0;
      break;
    case 3: /* trim both */
      got_char = 0;
      for (index = 0; index < len; index++)
      {
        if (got_char)
        {
          text1[text1_index] = text[index];
          text1_index++;
        }
        else
        {
          if (text[index] > 32)
          {
            text1[text1_index] = text[index];
            text1_index++;
            got_char = 1;
          }
        }
      }
      text1[text1_index] = 0;
      len = text1_index;
      /* trim right */
      for (index = len - 1; index >= 0; index--)
      {
        if (text1[index] > 32)
        {
          break;
        }
      }
      text1_index = index + 1;
      text1[text1_index] = 0;
      break;
    case 2: /* trim right */
      /* copy it */
      for (index = 0; index < len; index++)
      {
        text1[text1_index] = text[index];
        text1_index++;
      }
      /* trim right */
      for (index = len - 1; index >= 0; index--)
      {
        if (text1[index] > 32)
        {
          break;
        }
      }
      text1_index = index + 1;
      text1[text1_index] = 0;
      break;
    case 1: /* trim left */
      got_char = 0;
      for (index = 0; index < len; index++)
      {
        if (got_char)
        {
          text1[text1_index] = text[index];
          text1_index++;
        }
        else
        {
          if (text[index] > 32)
          {
            text1[text1_index] = text[index];
            text1_index++;
            got_char = 1;
          }
        }
      }
      text1[text1_index] = 0;
      break;
  }
  wcstombs(str, text1, text1_index + 1);
  free(text);
  free(text1);
  return 0;
}

/*****************************************************************************/
long APP_CC
g_load_library(char* in)
{
#if defined(_WIN32)
  return (long)LoadLibraryA(in);
#else
  return (long)dlopen(in, RTLD_LOCAL | RTLD_LAZY);
#endif
}

/*****************************************************************************/
int APP_CC
g_free_library(long lib)
{
  if (lib == 0)
  {
    return 0;
  }
#if defined(_WIN32)
  return FreeLibrary((HMODULE)lib);
#else
  return dlclose((void*)lib);
#endif
}

/*****************************************************************************/
/* returns NULL if not found */
void* APP_CC
g_get_proc_address(long lib, const char* name)
{
  if (lib == 0)
  {
    return 0;
  }
#if defined(_WIN32)
  return GetProcAddress((HMODULE)lib, name);
#else
  return dlsym((void*)lib, name);
#endif
}

/*****************************************************************************/
char*
g_get_dlerror()
{
  return dlerror();
}
/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_system(char* aexec)
{
#if defined(_WIN32)
  return 0;
#else
  return system(aexec);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_su(const char* username, int display, struct list* command, int tag)
{
  int pid = 0;
  long session_handle = 0;
  int error = 0;
  int pw_uid = 0;
  int pw_gid = 0;
  int uid = 0;
  char pw_shell[256] = {0};
  char pw_dir[256] = {0};
  char pw_gecos[256] = {0};
  char text[256] = {0};


  pid = g_fork();
  if (pid == -1)
  {
     printf("Error while forking\n");
     return 0;
  }
  else if (pid == 0) /* child sesman */
  {
    session_handle = auth_userpass("su", (char*)username, NULL);
    auth_start_session(session_handle, display);

    error = g_getuser_info(username, &pw_gid, &pw_uid, pw_shell, pw_dir,
                           pw_gecos);
    if (error == 0)
    {
      error = g_setgid(pw_gid);
      if (error == 0)
      {
        error = g_initgroups(username, pw_gid);
      }
      if (error == 0)
      {
        uid = pw_uid;
        error = g_setuid(uid);
      }
      if (error == 0)
      {
        g_clearenv();
        g_setenv("SHELL", pw_shell, 1);
        g_setenv("PATH", "/bin:/usr/bin:/usr/X11R6/bin:/usr/local/bin", 1);
        g_setenv("USER", username, 1);
        g_sprintf(text, "%d", uid);
        g_setenv("UID", text, 1);
        g_setenv("HOME", pw_dir, 1);
        if (tag)
        {
                g_setenv("XRDP_PROCESS", "1", 1);
        }
        g_set_current_dir(pw_dir);
        g_sprintf(text, ":%d.0", display);
        g_setenv("DISPLAY", text, 1);
      }
      else
      {
      	printf("error getting user info for user %s\n", username);
      }
    }

    g_execvp((char*)command->items[0], ((char**)command->items));
    printf("failed to exec command %s\n", (char*)command->items[0]);
    g_exit(0);
  }
  return pid;
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_launch_process(int display, struct list* command, int tag)
{
  int pid = 0;
  char text[256] = {0};

  pid = g_fork();
  if (pid == -1)
  {
     printf("Error while forking\n");
     return 0;
  }
  else if (pid == 0) /* child sesman */
  {
        if (tag)
        {
  	       g_setenv("XRDP_PROCESS", "1", 1);
        }
  	g_sprintf(text, ":%d.0", display);
  	g_setenv("DISPLAY", text, 1);

    g_execvp((char*)command->items[0], ((char**)command->items));
    printf("failed to exec command %s\n", (char*)command->items[0]);
    g_exit(0);
  }
  return pid;
}


/*****************************************************************************/
/* does not work in win32 */
char* APP_CC
g_get_strerror(void)
{
#if defined(_WIN32)
  return 0;
#else
  return strerror(errno);
#endif
}

/*****************************************************************************/
int APP_CC
g_get_errno(void)
{
#if defined(_WIN32)
  return GetLastError();
#else
  return errno;
#endif
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_execvp(const char* p1, char* args[])
{
#if defined(_WIN32)
  return 0;
#else
  return execvp(p1, args);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_user_execvp(const char* p1, char* args[])
{
#if defined(_WIN32)
  return 0;
#else
  return execvp(p1, args);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_execlp3(const char* a1, const char* a2, const char* a3)
{
#if defined(_WIN32)
  return 0;
#else
  return execlp(a1, a2, a3, (void*)0);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
void APP_CC
g_signal_child_stop(void (*func)(int))
{
#if defined(_WIN32)
#else
  signal(SIGCHLD, func);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
void APP_CC
g_signal_hang_up(void (*func)(int))
{
#if defined(_WIN32)
#else
  signal(SIGHUP, func);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
void APP_CC
g_signal_user_interrupt(void (*func)(int))
{
#if defined(_WIN32)
#else
  signal(SIGINT, func);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
void APP_CC
g_signal_kill(void (*func)(int))
{
#if defined(_WIN32)
#else
  signal(SIGKILL, func);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
void APP_CC
g_signal_terminate(void (*func)(int))
{
#if defined(_WIN32)
#else
  signal(SIGTERM, func);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
void APP_CC
g_signal_pipe(void (*func)(int))
{
#if defined(_WIN32)
#else
  signal(SIGPIPE, func);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
void APP_CC
g_wait_child(int sig) {
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0 || (pid < 0 && errno == EINTR));
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_fork(void)
{
#if defined(_WIN32)
  return 0;
#else
  return fork();
#endif
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_daemonize(char* pid_file)
{
#if defined(_WIN32)
  return 0;
#else
  pid_t pid;
  int fd;
  char buffer[10];

  /* make sure we can write to pid file */
  if (g_file_exist(pid_file))
  {
  	g_writeln("process already exist, quitting");
  	g_exit(1);
  }
  fd = g_file_open(pid_file); 
  if (fd == -1)
  {
  	g_writeln("running in daemon mode with no access to pid files, quitting");
  	g_exit(1);
  }
  if (g_file_write(fd, "0", 1) == -1)
  {
  	g_writeln("running in daemon mode with no access to pid files, quitting");
  	g_exit(1);
  }

  /* fork */
  pid = g_fork();
  /* error */
	if(pid < 0)
	{
		g_exit(1);
	}
	/* parent process */
	if( pid == 0 )
	{
		if(chdir("/") < 0)
		{
			g_exit(1);
		}
		if(setsid() < 0)
		{
			g_exit(1);
		}
		umask(0);
		 /* Redirect standard files to /dev/null */
		if( freopen( "/dev/null", "r", stdin)
				&& freopen( "/dev/null", "w", stdout)
				&& freopen( "/dev/null", "w", stderr))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	/* save pid number to pid file */
	g_sprintf(buffer, "%i", pid);
	fd = g_file_open(pid_file);
	g_file_write(fd, buffer, strlen(buffer));
	g_file_close(fd);
	g_exit(0);
	return 0;
#endif
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_setgid(int pid)
{
#if defined(_WIN32)
  return 0;
#else
  return setgid(pid);
#endif
}

/*****************************************************************************/
/* returns error, zero is success, non zero is error */
/* does not work in win32 */
int APP_CC
g_initgroups(const char* user, int gid)
{
#if defined(_WIN32)
  return 0;
#else
  return initgroups(user, gid);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
/* returns user id */
int APP_CC
g_getuid(void)
{
#if defined(_WIN32)
  return 0;
#else
  return getuid();
#endif
}

/*****************************************************************************/
/* does not work in win32 */
/* On success, zero is returned. On error, -1 is returned */
int APP_CC
g_setuid(int pid)
{
#if defined(_WIN32)
  return 0;
#else
  return setuid(pid);
#endif
}

/*****************************************************************************/
/* does not work in win32
   returns pid of process that exits or zero if signal occurred */
int APP_CC
g_waitchild(void)
{
#if defined(_WIN32)
  return 0;
#else
  int wstat;
  int rv;

  rv = waitpid(0, &wstat, WNOHANG);
  if (rv == -1)
  {
    if (errno == EINTR) /* signal occurred */
    {
      rv = 0;
    }
  }
  return rv;
#endif
}

/*****************************************************************************/
/* does not work in win32
   returns pid of process that exits or zero if signal occurred */
int APP_CC
g_waitpid(int pid)
{
#if defined(_WIN32)
  return 0;
#else
  int rv;

  rv = waitpid(pid, 0, 0);
  if (rv == -1)
  {
    if (errno == EINTR) /* signal occurred */
    {
      rv = 0;
    }
  }
  return rv;
#endif
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_testpid(int pid)
{
#if defined(_WIN32)
  return 0;
#else
  return waitpid(pid, 0, WNOHANG);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
void APP_CC
g_clearenv(void)
{
#if defined(_WIN32)
#else
  environ = 0;
#endif
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_setenv(const char* name, const char* value, int rewrite)
{
#if defined(_WIN32)
  return 0;
#else
  return setenv(name, value, rewrite);
#endif
}

/*****************************************************************************/
/* does not work in win32 */
char* APP_CC
g_getenv(const char* name)
{
#if defined(_WIN32)
  return 0;
#else
  return getenv(name);
#endif
}

/*****************************************************************************/
int APP_CC
g_exit(int exit_code)
{
  _exit(exit_code);
  return 0;
}

/*****************************************************************************/
int APP_CC
g_getpid(void)
{
#if defined(_WIN32)
  return (int)GetCurrentProcessId();
#else
  return (int)getpid();
#endif
}

/*****************************************************************************/
/* does not work in win32 */
int APP_CC
g_sigterm(int pid)
{
#if defined(_WIN32)
  return 0;
#else
  return kill(pid, SIGTERM);
#endif
}

/*****************************************************************************/
/* returns 0 if ok */
/* does not work in win32 */
int APP_CC
g_getuser_info(const char* username, int* gid, int* uid, char* shell,
               char* dir, char* gecos)
{
#if defined(_WIN32)
  return 1;
#else
  struct passwd* pwd_1;

  pwd_1 = getpwnam(username);
  if (pwd_1 != 0)
  {
    if (gid != 0)
    {
      *gid = pwd_1->pw_gid;
    }
    if (uid != 0)
    {
      *uid = pwd_1->pw_uid;
    }
    if (dir != 0)
    {
      g_strcpy(dir, pwd_1->pw_dir);
    }
    if (shell != 0)
    {
      g_strcpy(shell, pwd_1->pw_shell);
    }
    if (gecos != 0)
    {
      g_strcpy(gecos, pwd_1->pw_gecos);
    }
    return 0;
  }
  return 1;
#endif
}

/*****************************************************************************/
/* returns 0 if ok */
/* does not work in win32 */
int APP_CC
g_getuser_name(char* username, int uid)
{
  struct passwd* pass;

  if (username == NULL)
  {
    return 1;
  }

  pass = getpwuid(uid);
  if (pass && username)
  {
    g_strncpy(username, pass->pw_name, 1025);
    return 0;
  }

  return 1;
}

/*****************************************************************************/
/* returns 0 if ok */
/* does not work in win32 */
int APP_CC
g_getgroup_info(const char* groupname, int* gid)
{
#if defined(_WIN32)
  return 1;
#else
  struct group* g;

  g = getgrnam(groupname);
  if (g != 0)
  {
    if (gid != 0)
    {
      *gid = g->gr_gid;
    }
    return 0;
  }
  return 1;
#endif
}

/*****************************************************************************/
/* returns error */
/* if zero is returned, then ok is set */
/* does not work in win32 */
int APP_CC
g_check_user_in_group(const char* username, int gid, int* ok)
{
#if defined(_WIN32)
  return 1;
#else
  struct group* groups;
  int i;

  groups = getgrgid(gid);
  if (groups == 0)
  {
    return 1;
  }
  *ok = 0;
  i = 0;
  while (0 != groups->gr_mem[i])
  {
    if (0 == g_strcmp(groups->gr_mem[i], username))
    {
      *ok = 1;
      break;
    }
    i++;
  }
  return 0;
#endif
}

/*****************************************************************************/
/* returns the time since the Epoch (00:00:00 UTC, January 1, 1970),
   measured in seconds.
   for windows, returns the number of seconds since the machine was
   started. */
int APP_CC
g_time1(void)
{
#if defined(_WIN32)
  return GetTickCount() / 1000;
#else
  return time(0);
#endif
}

/*****************************************************************************/
/* returns the number of milliseconds since the machine was
   started. */
int APP_CC
g_time2(void)
{
#if defined(_WIN32)
  return (int)GetTickCount();
#else
  struct tms tm;
  clock_t num_ticks;

  num_ticks = times(&tm);
  return (int)(num_ticks * 10);
#endif
}

/*****************************************************************************/
/* returns time in milliseconds, uses gettimeofday
   does not work in win32 */
int APP_CC
g_time3(void)
{
#if defined(_WIN32)
  return 0;
#else
  struct timeval tp;

  gettimeofday(&tp, 0);
  return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
#endif
}

/*****************************************************************************/
int APP_CC
g_get_display_num_from_display(char* display_text)
{
  int index;
  int mode;
  int host_index;
  int disp_index;
  int scre_index;
  char host[256];
  char disp[256];
  char scre[256];

  index = 0;
  host_index = 0;
  disp_index = 0;
  scre_index = 0;
  mode = 0;

  if (display_text == NULL)
  {
	  return 0;
  }

  while (display_text[index] != 0)
  {
    if (display_text[index] == ':')
    {
      mode = 1;
    }
    else if (display_text[index] == '.')
    {
      mode = 2;
    }
    else if (mode == 0)
    {
      host[host_index] = display_text[index];
      host_index++;
    }
    else if (mode == 1)
    {
      disp[disp_index] = display_text[index];
      disp_index++;
    }
    else if (mode == 2)
    {
      scre[scre_index] = display_text[index];
      scre_index++;
    }
    index++;
  }
  host[host_index] = 0;
  disp[disp_index] = 0;
  scre[scre_index] = 0;

  return g_atoi(disp);
}

