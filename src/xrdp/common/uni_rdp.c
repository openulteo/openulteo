/**
 * Copyright (C) 2008-2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2010
 * Author James B. MacLean <macleajb@ednet.ns.ca> 2012
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

#include <uni_rdp.h>


#ifdef HAVE_ICONV
static int g_iconv_works = 1;
#endif



/*****************************************************************************/
/* Output a string in Unicode */
void
uni_rdp_out_str(struct stream* s, char *string, int len)
{
#ifdef HAVE_ICONV
	size_t ibl = strlen(string) + 1, obl = len ;
	static iconv_t iconv_h = (iconv_t) - 1;
	char *pin = string, *pout = (char *) s->p;

	memset(pout, 0, len + 4);

	if (g_iconv_works)
	{
		if (iconv_h == (iconv_t) - 1)
		{
			if ((iconv_h = iconv_open(WINDOWS_CODEPAGE, DEFAULT_CODEPAGE)) == (iconv_t) - 1)
			{
				printf("rdp_out_unistr: iconv_open[%s -> %s] fail %p\n",
						DEFAULT_CODEPAGE, WINDOWS_CODEPAGE, iconv_h);

				g_iconv_works = 0;
				uni_rdp_out_str(s, string, len);
				return;
			}
			pin = string;
			pout = (char *) s->p;
		}

		if (iconv(iconv_h, (ICONV_CONST char **) &pin, &ibl, &pout, &obl) == (size_t) - 1)
		{
			iconv_close(iconv_h);
			iconv_h = (iconv_t) - 1;
			printf("rdp_out_unistr: iconv(1) fail, errno %d %s\n", errno, strerror(errno));

			g_iconv_works = 0;
			uni_rdp_out_str(s, string, len);
			return;
		}

		s->p += len;

	}
	else
#endif
	{
		int i = 0, j = 0;

		while (i < len)
		{
			s->p[i++] = string[j++];
			s->p[i++] = 0;
		}

		s->p += len;
	}
}

/*****************************************************************************/
int APP_CC
uni_rdp_in_str(struct stream* s, char *string, int str_size, int in_len)
{
#ifdef HAVE_ICONV
  size_t ibl = in_len;
  size_t obl = str_size - 1;
  char *pin = (char *) s->p;
  char *pout = string;
  static iconv_t iconv_h = (iconv_t) - 1;

  if (g_iconv_works)
  {
    if (iconv_h == (iconv_t) - 1)
    {
      if ((iconv_h = iconv_open(DEFAULT_CODEPAGE, WINDOWS_CODEPAGE)) == (iconv_t) - 1)
      {
      	printf("rdpdr_disk[printer_dev_in_unistr]: Iconv_open[%s -> %s] fail %p",
					WINDOWS_CODEPAGE, DEFAULT_CODEPAGE, iconv_h);
        g_iconv_works = 0;
        return uni_rdp_in_str(s, string, str_size, in_len);
      }
    }

    if (iconv(iconv_h, (ICONV_CONST char **) &pin, &ibl, &pout, &obl) == (size_t) - 1)
    {
      if (errno == E2BIG)
      {
      	printf( "rdpdr_disk[printer_dev_in_unistr]: "
							"Server sent an unexpectedly long string, truncating");
      }
      else
      {
        iconv_close(iconv_h);
        iconv_h = (iconv_t) - 1;
        printf("rdpdr_disk[printer_dev_in_unistr]: "
							"Iconv fail, errno %d %s\n", errno, strerror(errno));
        g_iconv_works = 0;
        return uni_rdp_in_str(s, string, str_size, in_len);
      }
    }

    /* we must update the location of the current STREAM for future reads of s->p */
    s->p += in_len;
    *pout = 0;
    return pout - string;
  }
  else
#endif
  {
    int i = 0;
    int len = in_len / 2;
    int rem = 0;

    if (len > str_size - 1)
    {
      len = str_size - 1;
      rem = in_len - 2 * len;
    }
    while (i < len)
    {
      in_uint8a(s, &string[i++], 1);
      in_uint8s(s, 1);
    }
    in_uint8s(s, rem);
    string[len] = 0;
    return len;
  }
}



