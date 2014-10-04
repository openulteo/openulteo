/*
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

   xrdp: A Remote Desktop Protocol server.
   Copyright (C) Jay Sorg 2004-2009

   simple functions

*/
#include "funcs.h"

 /*****************************************************************************/
int list_add_rect(struct list* l, int left, int top, int right, int bottom) {
	struct xrdp_rect* r;
	if (left <= right && top <= bottom)
	{
		r = (struct xrdp_rect*) g_malloc(sizeof(struct xrdp_rect), 1);
		r->left = left;
		r->top = top;
		r->right = right;
		r->bottom = bottom;
		list_add_item(l, (tbus) r);
	}
	return 0;
}

/*****************************************************************************/
/* returns int */
int APP_CC
rect_height(struct xrdp_rect* rect)
{
	return (rect->bottom - rect->top);
}

/*****************************************************************************/
/* returns int */
int APP_CC
rect_width(struct xrdp_rect* rect)
{
	return (rect->right - rect->left);
}

/*****************************************************************************/
/* returns int */
int APP_CC
rect_adjacency(struct xrdp_rect* in1, struct xrdp_rect* in2)
{
	if (in1->left == in2->right &&
		((in2->top <= in1->top && in1->top < in2->bottom) ||
		 (in2->top < in1->bottom && in1->bottom <= in2->bottom)))
		return 1;
	if (in1->right == in2->left &&
		((in2->top <= in1->top && in1->top < in2->bottom) ||
		 (in2->top < in1->bottom && in1->bottom <= in2->bottom)))
		return 2;
	if (in1->top == in2->bottom &&
		((in2->left <= in1->left && in1->left < in2->right) ||
		 (in2->left < in1->right && in1->right <= in2->right)))
		return 3;
	if (in1->bottom == in2->top &&
		((in2->left <= in1->left && in1->left < in2->right) ||
		 (in2->left < in1->right && in1->right <= in2->right)))
		return 4;
	return 0;
}

/*****************************************************************************/
/* returns boolean */
int APP_CC
rect_equal(struct xrdp_rect* in1, struct xrdp_rect* in2)
{
  if (in2->left != in1->left)
  {
	  return 0;
  }
  if (in2->top != in1->top)
  {
	  return 0;
  }
  if (in2->right != in1->right)
  {
	  return 0;
  }
  if (in2->bottom != in1->bottom)
  {
	  return 0;
  }
  return 1;
}


/*****************************************************************************/
/* returns boolean */
int APP_CC
rect_substract(struct xrdp_rect* in1, struct xrdp_rect* in2, struct list* out) {

	if (in1->left <= in2->left &&
		in1->top <= in2->top &&
		in1->right >= in2->right &&
		in1->bottom >= in2->bottom) {
		list_add_rect(out, in1->left, in1->top, in1->right, in2->top);
		list_add_rect(out, in1->left, in2->top, in2->left, in2->bottom);
		list_add_rect(out, in2->right, in2->top, in1->right, in2->bottom);
		list_add_rect(out, in1->left, in2->bottom, in1->right, in1->bottom);
	} else if (in1->left <= in2->left &&
		in1->right >= in2->right &&
		in1->bottom < in2->bottom &&
		in1->top <= in2->top) { /* partially covered(whole top) */
		list_add_rect(out, in1->left, in1->top, in1->right, in2->top);
		list_add_rect(out, in1->left, in2->top, in2->left, in1->bottom);
		list_add_rect(out, in2->right, in2->top, in1->right, in1->bottom);
	} else if (in1->top <= in2->top &&
			   in1->bottom >= in2->bottom &&
			   in1->right < in2->right &&
			   in1->left <= in2->left) { /* partially covered(left) */
		list_add_rect(out, in1->left, in1->top, in1->right, in2->top);
		list_add_rect(out, in1->left, in2->top, in2->left, in2->bottom);
		list_add_rect(out, in1->left, in2->bottom, in1->right, in1->bottom);
	} else if (in1->left <= in2->left &&
			   in1->right >= in2->right &&
			   in1->top > in2->top &&
			   in1->bottom >= in2->bottom) { /* partially covered(bottom) */
		list_add_rect(out, in1->left, in1->top, in2->left, in2->bottom);
		list_add_rect(out, in2->right, in1->top, in1->right, in2->bottom);
		list_add_rect(out, in1->left, in2->bottom, in1->right, in1->bottom);
	} else if (in1->top <= in2->top &&
			   in1->bottom >= in2->bottom &&
			   in1->left > in2->left &&
			   in1->right >= in2->right) { /* partially covered(right) */
		list_add_rect(out, in1->left, in1->top, in1->right, in2->top);
		list_add_rect(out, in2->right, in2->top, in1->right, in2->bottom);
		list_add_rect(out, in1->left, in2->bottom, in1->right, in1->bottom);
	} else if (in1->left <= in2->left &&
			   in1->top <= in2->top &&
			   in1->right < in2->right &&
			   in1->bottom < in2->bottom) { /* partially covered(top left) */
		list_add_rect(out, in1->left, in1->top, in1->right, in2->top);
		list_add_rect(out, in1->left, in2->top, in2->left, in1->bottom);
	} else if (in1->left <= in2->left &&
			   in1->bottom >= in2->bottom &&
			   in1->right < in2->right &&
			   in1->top > in2->top) { /* partially covered(bottom left) */
		list_add_rect(out, in1->left, in1->top, in2->left, in2->bottom);
		list_add_rect(out, in1->left, in2->bottom, in1->right, in1->bottom);
	} else if (in1->left > in2->left &&
			   in1->right >= in2->right &&
			   in1->top <= in2->top &&
			   in1->bottom < in2->bottom) { /* partially covered(top right) */
		list_add_rect(out, in1->left, in1->top, in1->right, in2->top);
		list_add_rect(out, in2->right, in2->top, in1->right, in1->bottom);
	} else if (in1->left > in2->left &&
			   in1->right >= in2->right &&
			   in1->top > in2->top &&
			   in1->bottom >= in2->bottom) { /* partially covered(bottom right) */
		list_add_rect(out, in2->right, in1->top, in1->right, in2->bottom);
		list_add_rect(out, in1->left, in2->bottom, in1->right, in1->bottom);
	} else if (in1->left > in2->left &&
			   in1->top <= in2->top &&
			   in1->right < in2->right &&
			   in1->bottom >= in2->bottom) { /* 2 rects, one on each end */
		list_add_rect(out, in1->left, in1->top, in1->right, in2->top);
		list_add_rect(out, in1->left, in2->bottom, in1->right, in1->bottom);
	} else if (in1->left <= in2->left &&
			   in1->top > in2->top &&
			   in1->right >= in2->right &&
			   in1->bottom < in2->bottom) { /* 2 rects, one on each end */
		list_add_rect(out, in1->left, in1->top, in2->left, in1->bottom);
		list_add_rect(out, in2->right, in1->top, in1->right, in1->bottom);
	} else if (in1->left > in2->left &&
			   in1->right < in2->right &&
			   in1->top <= in2->top &&
			   in1->bottom < in2->bottom) { /* partially covered(top) */
		list_add_rect(out, in1->left, in1->top, in1->right, in2->top);
	} else if (in1->top > in2->top &&
			   in1->bottom < in2->bottom &&
			   in1->left <= in2->left &&
			   in1->right < in2->right) { /* partially covered(left) */
		list_add_rect(out, in1->left, in1->top, in2->left, in1->bottom);
	} else if (in1->left > in2->left &&
			   in1->right < in2->right &&
			   in1->bottom >= in2->bottom &&
			   in1->top > in2->top) { /* partially covered(bottom) */
		list_add_rect(out, in1->left, in2->bottom, in1->right, in1->bottom);
	} else if (in1->top > in2->top &&
			   in1->bottom < in2->bottom &&
			   in1->right >= in2->right &&
			   in1->left > in2->left) { /* partially covered(right) */
		list_add_rect(out, in2->right, in1->top, in1->right, in1->bottom);
	} else if (in1->left > in2->left &&
			   in1->top > in2->top &&
			   in1->right < in2->right &&
			   in1->bottom < in2->bottom) { /* totally contained, 4 rects */
	}
	return 0;
}

/*****************************************************************************/
/* returns boolean */
int APP_CC
rect_union(struct xrdp_rect* in1, struct xrdp_rect* in2, struct list* out) {
	struct xrdp_rect tmp;
	if (!rect_intersect(in1, in2, &tmp)) {
		return 0;
	}
	if (rect_equal(in1, in2)) {
		return 0; 
	} 
	if (in1->left <= in2->left &&
		in1->top <= in2->top &&
		in1->right >= in2->right &&
		in1->bottom >= in2->bottom) {
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
	} else if (in1->left <= in2->left &&
		in1->right >= in2->right &&
		in1->bottom < in2->bottom &&
		in1->top <= in2->top) { /* partially covered(whole top) */
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
		list_add_rect(out, in2->left, in1->bottom, in2->right, in2->bottom);
	} else if (in1->top <= in2->top &&
			   in1->bottom >= in2->bottom &&
			   in1->right < in2->right &&
			   in1->left <= in2->left) { /* partially covered(left) */
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
		list_add_rect(out, in1->right, in2->top, in2->right, in2->bottom);
	} else if (in1->left <= in2->left &&
			   in1->right >= in2->right &&
			   in1->top > in2->top &&
			   in1->bottom >= in2->bottom) { /* partially covered(bottom) */
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
		list_add_rect(out, in2->left, in2->top, in2->right, in1->top);
	} else if (in1->top <= in2->top &&
			   in1->bottom >= in2->bottom &&
			   in1->left > in2->left &&
			   in1->right >= in2->right) { /* partially covered(right) */
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
		list_add_rect(out, in2->left, in2->top, in1->left, in2->bottom);
	} else if (in1->left <= in2->left &&
			   in1->top <= in2->top &&
			   in1->right < in2->right &&
			   in1->bottom < in2->bottom) { /* partially covered(top left) */
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
		list_add_rect(out, in1->right, in2->top, in2->right, in1->bottom);
		list_add_rect(out, in2->left, in1->bottom, in2->right, in2->bottom);
	} else if (in1->left <= in2->left &&
			   in1->bottom >= in2->bottom &&
			   in1->right < in2->right &&
			   in1->top > in2->top) { /* partially covered(bottom left) */
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
		list_add_rect(out, in2->left, in2->top, in2->right, in1->top);
		list_add_rect(out, in1->right, in1->top, in2->right, in2->bottom);
	} else if (in1->left > in2->left &&
			   in1->right >= in2->right &&
			   in1->top <= in2->top &&
			   in1->bottom < in2->bottom) { /* partially covered(top right) */
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
		list_add_rect(out, in2->left, in2->top, in1->left, in2->bottom);
		list_add_rect(out, in2->left, in1->bottom, in2->right, in2->bottom);
	} else if (in1->left > in2->left &&
			   in1->right >= in2->right &&
			   in1->top > in2->top &&
			   in1->bottom >= in2->bottom) { /* partially covered(bottom right) */
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
		list_add_rect(out, in2->left, in2->top, in2->right, in1->top);
		list_add_rect(out, in2->left, in1->top, in1->left, in2->bottom);
	} else if (in1->left > in2->left &&
			   in1->top <= in2->top &&
			   in1->right < in2->right &&
			   in1->bottom >= in2->bottom) { /* 2 rects, one on each end */
		list_add_rect(out, in1->left, in1->top, in1->right, in1->bottom);
		list_add_rect(out, in2->left, in2->top, in1->left, in2->bottom);
		list_add_rect(out, in1->right, in2->top, in2->right, in2->bottom);
	} else if (in1->left <= in2->left &&
			   in1->top > in2->top &&
			   in1->right >= in2->right &&
			   in1->bottom < in2->bottom) { /* 2 rects, one on each end */
		list_add_rect(out, in2->left, in2->top, in2->right, in2->bottom);
		list_add_rect(out, in1->left, in1->top, in2->left, in1->bottom);
		list_add_rect(out, in2->right, in1->top, in1->right, in1->bottom);
	 } else if (in1->left > in2->left &&
			 in1->right < in2->right &&
			 in1->top <= in2->top &&
			 in1->bottom < in2->bottom) { /* partially covered(top) */
		 list_add_rect(out, in2->left, in2->top, in2->right, in2->bottom);
		 list_add_rect(out, in1->left, in1->top, in1->right, in2->top);
	} else if (in1->top > in2->top &&
			   in1->bottom < in2->bottom &&
			   in1->left <= in2->left &&
			   in1->right < in2->right) { /* partially covered(left) */
		list_add_rect(out, in2->left, in2->top, in2->right, in2->bottom);
		list_add_rect(out, in1->left, in1->top, in2->left, in1->bottom);
	} else if (in1->left > in2->left &&
			   in1->right < in2->right &&
			   in1->bottom >= in2->bottom &&
			   in1->top > in2->top) { /* partially covered(bottom) */
		list_add_rect(out, in2->left, in2->top, in2->right, in2->bottom);
		list_add_rect(out, in1->left, in2->bottom, in1->right, in1->bottom);
	} else if (in1->top > in2->top &&
			   in1->bottom < in2->bottom &&
			   in1->right >= in2->right &&
			   in1->left > in2->left) { /* partially covered(right) */
		list_add_rect(out, in2->left, in2->top, in2->right, in2->bottom);
		list_add_rect(out, in2->right, in1->top, in1->right, in1->bottom);
	} else if (in1->left > in2->left &&
			   in1->top > in2->top &&
			   in1->right < in2->right &&
			   in1->bottom < in2->bottom) { /* totally contained, 4 rects */
		list_add_rect(out, in2->left, in2->top, in2->right, in2->bottom);
	}
	return 0;
}
 
/*****************************************************************************/
/* returns boolean */
int APP_CC
rect_contains_pt(struct xrdp_rect* in, int x, int y)
{
  if (x < in->left)
  {
    return 0;
  }
  if (y < in->top)
  {
    return 0;
  }
  if (x >= in->right)
  {
    return 0;
  }
  if (y >= in->bottom)
  {
    return 0;
  }
  return 1;
}

/*****************************************************************************/
int APP_CC
rect_intersect(struct xrdp_rect* in1, struct xrdp_rect* in2,
               struct xrdp_rect* out)
{
  int rv;
  struct xrdp_rect dumby;

  if (out == 0)
  {
    out = &dumby;
  }
  *out = *in1;
  if (in2->left > in1->left)
  {
    out->left = in2->left;
  }
  if (in2->top > in1->top)
  {
    out->top = in2->top;
  }
  if (in2->right < in1->right)
  {
    out->right = in2->right;
  }
  if (in2->bottom < in1->bottom)
  {
    out->bottom = in2->bottom;
  }
  rv = !ISRECTEMPTY(*out);
  if (!rv)
  {
    g_memset(out, 0, sizeof(struct xrdp_rect));
  }
  return rv;
}

/*****************************************************************************/
/* returns boolean */
int APP_CC
rect_contained_by(struct xrdp_rect* in1, int left, int top,
                  int right, int bottom)
{
  if (left < in1->left || top < in1->top ||
      right > in1->right || bottom > in1->bottom)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/*****************************************************************************/
/* returns int */
int APP_CC
rect_update_bounding_box(struct xrdp_rect* BB, struct xrdp_rect* r, int DELTA)
{
  struct xrdp_rect inter;
  if (rect_equal(BB, r))
  {
    return 0;
  }
  if (rect_contained_by(r, BB->left, BB->top, BB->right, BB->bottom))
  {
    BB->left = r->left;
    BB->top = r->top;
    BB->bottom = r->bottom;
    BB->right = r->right;
    return 0;
  }
  if (rect_contained_by(BB, r->left, r->top, r->right, r->bottom))
  {
    return 0;
  }
  struct xrdp_rect tmp;
  tmp.left = BB->left - DELTA;
  tmp.right = BB->right + DELTA;
  tmp.top = BB->top - DELTA;
  tmp.bottom = BB->bottom + DELTA;

  if (rect_intersect(BB, r, &inter) || rect_intersect(&tmp, r, &inter))
  {
    BB->top = MIN(BB->top, r->top);
    BB->left = MIN(BB->left, r->left);
    BB->right = MAX(BB->right, r->right);
    BB->bottom = MAX(BB->bottom, r->bottom);
    return 0;
  }
  else
  {
    return 1;
  }
  return -1;
}

/*****************************************************************************/
/* adjust the bounds to fit in the bitmap */
/* return false if there is nothing to draw else return true */
int APP_CC
check_bounds(struct xrdp_bitmap* b, int* x, int* y, int* cx, int* cy)
{
  if (*x >= b->width)
  {
    return 0;
  }
  if (*y >= b->height)
  {
    return 0;
  }
  if (*x < 0)
  {
    *cx += *x;
    *x = 0;
  }
  if (*y < 0)
  {
    *cy += *y;
    *y = 0;
  }
  if (*cx <= 0)
  {
    return 0;
  }
  if (*cy <= 0)
  {
    return 0;
  }
  if (*x + *cx > b->width)
  {
    *cx = b->width - *x;
  }
  if (*y + *cy > b->height)
  {
    *cy = b->height - *y;
  }
  return 1;
}

/*****************************************************************************/
/* add a ch at index position in text, index starts at 0 */
/* if index = -1 add it to the end */
int APP_CC
add_char_at(char* text, int text_size, twchar ch, int index)
{
  int len;
  int i;
  twchar* wstr;

  len = g_mbstowcs(0, text, 0);
  wstr = (twchar*)g_malloc((len + 16) * sizeof(twchar), 0);
  g_mbstowcs(wstr, text, len + 1);
  if ((index >= len) || (index < 0))
  {
    wstr[len] = ch;
    wstr[len + 1] = 0;
    g_wcstombs(text, wstr, text_size);
    g_free(wstr);
    return 0;
  }
  for (i = (len - 1); i >= index; i--)
  {
    wstr[i + 1] = wstr[i];
  }
  wstr[i + 1] = ch;
  wstr[len + 1] = 0;
  g_wcstombs(text, wstr, text_size);
  g_free(wstr);
  return 0;
}

/*****************************************************************************/
/* remove a ch at index position in text, index starts at 0 */
/* if index = -1 remove it from the end */
int APP_CC
remove_char_at(char* text, int text_size, int index)
{
  int len;
  int i;
  twchar* wstr;

  len = g_mbstowcs(0, text, 0);
  if (len <= 0)
  {
    return 0;
  }
  wstr = (twchar*)g_malloc((len + 16) * sizeof(twchar), 0);
  g_mbstowcs(wstr, text, len + 1);
  if ((index >= (len - 1)) || (index < 0))
  {
    wstr[len - 1] = 0;
    g_wcstombs(text, wstr, text_size);
    g_free(wstr);
    return 0;
  }
  for (i = index; i < (len - 1); i++)
  {
    wstr[i] = wstr[i + 1];
  }
  wstr[len - 1] = 0;
  g_wcstombs(text, wstr, text_size);
  g_free(wstr);
  return 0;
}

/*****************************************************************************/
int APP_CC
set_string(char** in_str, const char* in)
{
  if (in_str == 0)
  {
    return 0;
  }
  g_free(*in_str);
  *in_str = g_strdup(in);
  return 0;
}

/*****************************************************************************/
int APP_CC
wchar_repeat(twchar* dest, int dest_size_in_wchars, twchar ch, int repeat)
{
  int index;

  for (index = 0; index < repeat; index++)
  {
    if (index >= dest_size_in_wchars)
    {
      break;
    }
    dest[index] = ch;
  }
  return 0;
}
