/**
 * Copyright (C) 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2012
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

#include <cp1252_rdp.h>


#ifdef HAVE_ICONV
static int g_iconv_works = 1;
#endif



/*****************************************************************************/
/* Output a string in CP1252 */
void
cp1252_rdp_out_str(struct stream* s, char *string, int len)
{
#ifdef HAVE_ICONV
	size_t ibl = strlen(string), obl = len + 2;
	static iconv_t iconv_h = (iconv_t) - 1;
	char *pin = string, *pout = (char *) s->p;

	memset(pout, 0, len + 4);

	if (g_iconv_works)
	{
		if (iconv_h == (iconv_t) - 1)
		{
			size_t i = 1, o = 4;
			if ((iconv_h = iconv_open(CP1252_CODEPAGE, DEFAULT_CODEPAGE)) == (iconv_t) - 1)
			{
				printf("rdp_out_unistr: iconv_open[%s -> %s] fail %p\n",
						DEFAULT_CODEPAGE, WINDOWS_CODEPAGE, iconv_h);

				g_iconv_works = 0;
				cp1252_rdp_out_str(s, string, len);
				return;
			}
			if (iconv(iconv_h, (ICONV_CONST char **) &pin, &i, &pout, &o) == (size_t) - 1)
			{
				iconv_close(iconv_h);
				iconv_h = (iconv_t) - 1;
				printf("rdp_out_unistr: iconv(1) fail, errno %d %s\n", errno, strerror(errno));

				g_iconv_works = 0;
				cp1252_rdp_out_str(s, string, len);
				return;
			}
			pin = string;
			pout = (char *) s->p;
		}

		if (iconv(iconv_h, (ICONV_CONST char **) &pin, &ibl, &pout, &obl) == (size_t) - 1)
		{
			iconv_close(iconv_h);
			iconv_h = (iconv_t) - 1;
			printf("rdp_out_unistr: iconv(2) fail, errno %d %s\n", errno, strerror(errno));

			g_iconv_works = 0;
			cp1252_rdp_out_str(s, string, len);
			return;
		}

		s->p += len + 2;

	}
	else
#endif
	{
		out_uint8p(s, string, len);
	}
}

/*****************************************************************************/
int APP_CC
cp1252_rdp_in_str(struct stream* s, char *string, int str_size, int in_len)
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
			if ((iconv_h = iconv_open(DEFAULT_CODEPAGE, CP1252_CODEPAGE)) == (iconv_t) - 1)
			{
				printf("cp1252_rdp_in_str: Iconv_open[%s -> %s] fail %p\n", CP1252_CODEPAGE, DEFAULT_CODEPAGE, iconv_h);
				g_iconv_works = 0;
				return cp1252_rdp_in_str(s, string, str_size, in_len);
			}
		}

		if (iconv(iconv_h, (ICONV_CONST char **) &pin, &ibl, &pout, &obl) == (size_t) - 1)
		{
			if (errno == E2BIG)
			{
				printf( "cp1252_rdp_in_str: Server sent an unexpectedly long string, truncating\n");
			}
			else
			{
				iconv_close(iconv_h);
				iconv_h = (iconv_t) - 1;
				printf("cp1252_rdp_in_str: Iconv fail, errno %d %s\n", errno, strerror(errno));
				g_iconv_works = 0;
				return cp1252_rdp_in_str(s, string, str_size, in_len);
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
		in_uint8a(s, string, in_len);
		return in_len;
	}
}



