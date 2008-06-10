/* Portions Copyright 2003, Jordan Crouse (jordan@cosmicpenguin.net) */

#define MWINCLUDECOLORS
#include "nxlib.h"
#include <stdlib.h>
#include <string.h>

/*
 * X Image routines.  Messy.  Not done yet.
 */

#define READ_BIT(image, x, y)	\
	(image->data[(y * image->bytes_per_line) + (x >> 3) ] & (1 << (x & 7)))

static int
destroy_image(XImage *image)
{
	if (image->data)
		Xfree(image->data);
	Xfree(image);
	return 1;
}

static unsigned long
get_pixel1(XImage *image, unsigned int x, unsigned int y)
{
	return READ_BIT(image, x, y) != 0;
}

static int
put_pixel1(XImage *image, unsigned int x, unsigned int y, unsigned long pixel)
{
	static unsigned char mask[] =
		{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

	if (pixel)
		image->data[(y * image->bytes_per_line) + (x >> 3) ] |= mask[x&7];
	else image->data[(y * image->bytes_per_line) + (x >> 3) ] &= ~mask[x&7];
	return 1;
}

static unsigned long
get_pixel8(XImage *image, unsigned int x, unsigned int y)
{
	unsigned char *src = image->data + (y * image->bytes_per_line) + x;

	return *src;
}

static int
put_pixel8(XImage *image, unsigned int x, unsigned int y, unsigned long pixel)
{
	unsigned char *src = image->data + (y * image->bytes_per_line) + x;

	*src = pixel;
	return 1;
}

static unsigned long
get_pixel16(XImage *image, unsigned int x, unsigned int y)
{
	unsigned short *src = (unsigned short *)
		(image->data + (y * image->bytes_per_line) + (x << 1));

	return *src;
}

static int
put_pixel16(XImage *image, int x, int y, unsigned long pixel)
{
	unsigned short *src = (unsigned short *)
		(image->data + (y * image->bytes_per_line) + (x << 1));

	*src = pixel;
	return 1;
}

static unsigned long
get_pixel32(XImage *image, unsigned int x, unsigned int y)
{
	unsigned long *src = (unsigned long *)
		(image->data + (y * image->bytes_per_line) + (x << 2));

	return *src;
}

static int
put_pixel32(XImage *image, unsigned int x, unsigned int y, unsigned long pixel)
{
	unsigned long *src = (unsigned long *)
		(image->data + (y * image->bytes_per_line) + (x << 2));

	*src = pixel;
	return 1;
}

/*
 * compute image line size and bytes per pixel
 * from bits per pixel and width
 */
#define PIX2BYTES(n)    (((n)+7)/8)
static void
computePitch(int bpp, int width, int *pitch, int *bytesperpixel)
{
	int linesize;
	int bytespp = 1;

	if (bpp == 1)
		linesize = PIX2BYTES(width);
	else if (bpp <= 4)
		linesize = PIX2BYTES(width << 2);
	else if (bpp <= 8)
		linesize = width;
	else if (bpp <= 16) {
		linesize = width * 2;
		bytespp = 2;
	} else if (bpp <= 24) {
		linesize = width * 3;
		bytespp = 3;
	} else {
		linesize = width * 4;
		bytespp = 4;
	}

	/* rows are DWORD right aligned */
	*pitch = (linesize + 3) & ~3;

	*bytesperpixel = bytespp;
}

static XImage *
createImageStruct(unsigned int width, unsigned int height, unsigned int depth,
	int format, int bytes_per_line, int bitmap_pad, unsigned long red_mask,
	unsigned long green_mask, unsigned long blue_mask)
{
	XImage *image = (XImage *) Xcalloc(sizeof(XImage), 1);
	if (!image)
		return 0;

	image->width = width;
	image->height = height;
	image->format = format;
	image->depth = depth;

	/* note: if these are changed by application, we'll likely fail*/
	image->byte_order = LSBFirst;
	image->bitmap_bit_order = LSBFirst;
	image->bitmap_unit = bitmap_pad;
	image->bitmap_pad = bitmap_pad;

	if (bytes_per_line)
		image->bytes_per_line = bytes_per_line;
	else {
		int bytespp, pitch;

		computePitch(depth, image->width, &pitch, &bytespp);
		image->bytes_per_line = pitch;
	}

	image->bits_per_pixel = depth;

#ifdef DEBUG
	printf("createImage: %d,%d format %d depth %d bytespl %d\n", width, height, format, depth, image->bytes_per_line);
#endif

	// FIXME check
	image->red_mask = red_mask;
	image->green_mask = green_mask;
	image->blue_mask = blue_mask;

	image->f.create_image = XCreateImage;
	image->f.destroy_image = destroy_image;
	image->f.sub_image = 0;		//FIXME
	image->f.add_pixel = 0;
	switch (image->bits_per_pixel) {
	case 1:
		image->f.get_pixel = get_pixel1;
		image->f.put_pixel = put_pixel1;
		break;
	case 8:
		image->f.get_pixel = get_pixel8;
		image->f.put_pixel = put_pixel8;
		break;
	case 16:
		image->f.get_pixel = get_pixel16;
		image->f.put_pixel = put_pixel16;
		break;
	case 32:
		image->f.get_pixel = get_pixel32;
		image->f.put_pixel = put_pixel32;
		break;
	default:
		printf("createImageStruct: unsupported bpp\n");
	}

	return image;
}

/*
 * Create an image in either, preferably in hw format.
 */
XImage *
XCreateImage(Display * display, Visual * visual, unsigned int depth,
	int format, int offset, char *data, unsigned int width,
	unsigned int height, int bitmap_pad, int bytes_per_line)
{
	XImage *image;
	unsigned long red_mask = 0, green_mask = 0, blue_mask = 0;
	
	if (depth == 24) {
		printf("XCreateImage: changing depth to GR_PIXELVAL\n");
		depth = sizeof(GR_PIXELVAL) * 8;
	}
	if (depth != display->screens[0].root_depth) {
		printf("XCreateImage: depth != hw_format\n");
		//if (depth == 1)	//FIXME
			//depth = sizeof(GR_PIXELVAL) * 8;
	}

	if (visual) {
		red_mask = visual->red_mask;
		green_mask = visual->green_mask;
		blue_mask = visual->blue_mask;
	}
	image = createImageStruct(width, height, depth, format,
			   bytes_per_line, bitmap_pad, red_mask,
			   blue_mask, green_mask);
	if (image)
		image->data = data;

	return image;
}

/*
 * Create an image, (always in GR_PIXELVAL format), and initialize
 * using GrReadArea.
 */
XImage *
XGetImage(Display * display, Drawable d, int x, int y,
	unsigned int width, unsigned int height,
	unsigned long plane_mask, int format)
{
	int depth = 0, drawsize;
	Visual *visual;
	XImage *image = 0;
	GR_WINDOW_INFO winfo;

	/* Ensure that the block is entirely within the drawable */
	GrGetWindowInfo(d, &winfo);
	if (x < 0 || (x + width) > winfo.width ||
	    y < 0 || (y + height) > winfo.height) {
		/* Error - BadMatch */
		printf("XGetImage: Image out of bounds\n");
		printf("    %d %d - %d %d is out of bounds on %d, %d - %d %d\n",
		       x, y, width, height, 0, 0, winfo.width, winfo.height);
		return NULL;
	}

	if (format == XYPixmap)
		printf("XGetImage warning: broken for XYPixmap\n");

	/* 
	 * create XImage in GrReadArea compatible format,
	 * which is always sizeof(GR_PIXELVAL), not hw display format
	 */
	depth = sizeof(GR_PIXELVAL) * 8;

#if 0
	if (depth <= 8)
		drawsize = 1;
	if (depth > 8 && depth <= 16)
		drawsize = 2;
	else
		drawsize = 4;
#endif
	drawsize = sizeof(GR_PIXELVAL);

	visual = XDefaultVisual(display, 0);
	image = createImageStruct(width, height, depth, format, 0, 0,
		visual->red_mask, visual->green_mask, visual->blue_mask);
	if (!image)
		return NULL;

	image->data = (char *) Xcalloc(width * height * drawsize, 1);
	GrReadArea(d, x, y, width, height, (void *) image->data);

	if (format == XYPixmap && plane_mask != 0xFFFFFFFF)
		printf("XGetImage: plane_mask ignored\n");

	return image;
}

/*
 * Output a non-palettized image.  Never uses colormap, but instead
 * uses GrArea with image depth parameter for server conversion.
 */

/* This takes a portion of the image buffer and rearranges it to keep from 
   freaking out GrArea. This will take into account a shifted src_x 
   (or a width that is not as large as the declared image width).
*/

static void
showPartialImage(GR_WINDOW_ID d, GR_GC_ID gc, GR_RECT *srect, GR_RECT *drect, char *src, int mode)
{
	char *dst = 0, *local = 0;
	char *ptr = src;  /* This will already be adjusted to the inital X and Y of the image */
	int r, size = 0;

	switch(mode) {
	case MWPF_TRUECOLOR332: 
		size = 1; break;
	case MWPF_TRUECOLOR555:
	case MWPF_TRUECOLOR565:
		size = 2; break;
	case MWPF_TRUECOLOR888:
		size = 3; break;
	case MWPF_TRUECOLOR0888:
		size = 4; break;
	}

	/* Allocate a local buffer - this is much faster than doing N GrArea calls */
	dst = local = (char *) malloc( (drect->width * size) * drect->height);

	if (!local) return;

	/* Copy each row to the buffer */

	for(r = 0; r < drect->height; r++) {
		memcpy(dst, ptr, drect->width * size); 
		dst += (drect->width * size);

		ptr += (srect->width - srect->x) * size;  /* Move to the end of the line on the real buffer */
		ptr += (srect->x * size);              /* And then offset ourselves accordingly          */
	}
	       
	GrArea(d, gc, drect->x, drect->y, drect->width, drect->height, local, mode);
	free(local);
}

static int
putTrueColorImage(Display * display, Drawable d, GC gc, XImage *image,
	int src_x, int src_y, int dest_x, int dest_y,
	unsigned int width, unsigned int height)
{
	int mode = MWPF_PIXELVAL;
	unsigned char *src = image->data;

#ifdef DEBUG
	printf("putTrueColorImage depth %d  src %d,%d wxh %d,%d dst %d,%d\n",
	       image->depth, src_x, src_y, width, height, dest_x, dest_y);
#endif

	switch (image->bits_per_pixel) {
	case 1:	// major FIXME
		printf("putTruecolorImage bpp 1 FIXME\n");
		src = image->data;
		return 1; // must return, will crash server
		break;
	case 8:
		mode = MWPF_TRUECOLOR332;
		src = image->data + (src_y * image->bytes_per_line) + src_x;
		break;
	case 16:
		if (display->screens[0].root_visual->bits_per_rgb == 5)
			mode = MWPF_TRUECOLOR555;
		else
			mode = MWPF_TRUECOLOR565;
		src = image->data + (src_y * image->bytes_per_line) + (src_x << 1);
		break;
	case 24:
		mode = MWPF_TRUECOLOR888;
		src = image->data + (src_y * image->bytes_per_line) + (src_x * 3);
		break;
	case 32:
		mode = MWPF_TRUECOLOR0888;
		src = image->data + (src_y * image->bytes_per_line) + (src_x << 2);
		break;
	default:
		printf("XPutImage: unsupported bpp %d\n", image->bits_per_pixel);
	}

#ifdef DEBUG
	printf("putTrueColorImage nxmode = %d\n", mode);
#endif

	/* We can only do a direct GrArea if the width is the same as the width
	   of our image buffer.  Otherwise, we need to move to a slower path that
	   handles the situation better.
	*/

	if (!src_x && width == image->width) 
		GrArea((GR_WINDOW_ID)d, (GR_GC_ID)gc->gid, dest_x, dest_y,
		       width, height, src, mode);
	else {
		GR_RECT srect, drect;
		srect.x = src_x;  srect.y = src_y;
		srect.width = image->width;  srect.height = image->height;
		
		drect.x = dest_x;  drect.y = dest_y;
		drect.width = width;  drect.height = height;
		
		showPartialImage((GR_WINDOW_ID) d, (GR_GC_ID) gc->gid, &srect, &drect, src, mode);
	}

	return 1;
}

/*
 * Output a palette-oriented image.  Must have properly defined colormap.
 * These images are defined in any bpp but contain colormap indices.
 */
static int
putImage(Display * display, Drawable d, GC gc, XImage * image,
	int src_x, int src_y, int dest_x, int dest_y,
	unsigned int width, unsigned int height)
{
	unsigned int x, y;
	unsigned long *buffer, *dst;
	unsigned char *src = image->data +
		((src_y * (image->bytes_per_line)) + src_x);
	nxColormap *colormap = _nxFindColormap(XDefaultColormap(display, 0));

printf("putImage: bpp %d\n", image->depth);
	if (!colormap)
		return 0;

	dst = buffer = ALLOCA(width * height * sizeof(unsigned long));

	for (y = src_y; y < src_y + height; y++) {
		for (x = src_x; x < src_x + width; x++, dst++) {
			unsigned short cl;

			/* get colormap index bits from image*/
			switch (image->bits_per_pixel) {
			case 32:
				cl = (unsigned short) *((unsigned long *) src);
				src += 4;
				continue;

			//case 24: FIXME
			case 16:
				cl = (unsigned short) *((unsigned short *) src);
				src += 2;
				break;

			case 8:
				cl = (unsigned short) *((unsigned char *) src);
				src += 1;
				break;

			case 1:
				if (READ_BIT(image, x, y))
					//cl = colormap->colorval[1].value;
					cl = 1;
				else
					//cl = colormap->colorval[0].value;
					cl = 0;
				break;
			}

			if (cl < colormap->cur_color)
				*dst = (unsigned long) colormap->colorval[cl].
					value;
			else {
				// FIXME colors kluged as if truecolor here...
				// (no colormap entries...)
				if (image->depth == 1) {
					//cl = ((XGCValues *)gc->ext_data)->foreground;
					*dst = cl? WHITE: BLACK;
				} else {
					printf("XPutImage: unknown color index %x\n", cl);
					*dst = 0;
				}
			}
		}
	}

	GrArea((GR_WINDOW_ID) d, (GR_GC_ID) gc->gid, dest_x, dest_y,
	       width, height, buffer, MWPF_RGB);

	FREEA(buffer);
	return 1;
}

int
XPutImage(Display * display, Drawable d, GC gc, XImage * image,
	int src_x, int src_y, int dest_x, int dest_y, unsigned int width,
	unsigned int height)
{
	// FIXME bpp 1
	if (XDefaultVisual(display, 0)->class == TrueColor && image->depth != 1)
		return putTrueColorImage(display, d, gc, image, src_x, src_y,
			dest_x, dest_y, width, height);
	return putImage(display, d, gc, image, src_x, src_y, dest_x, dest_y,
			width, height);
}
