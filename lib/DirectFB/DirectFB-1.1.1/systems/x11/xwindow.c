/*
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "x11.h"


//
// Data to create an invisible cursor
//
const char null_cursor_bits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};



Bool
dfb_x11_open_window(XWindow** ppXW, int iXPos, int iYPos, int iWidth, int iHeight)
{
    XWindow* xw = (XWindow *)calloc(1, sizeof(XWindow));

	/* We set the structure as needed for our window */
	xw->width	= iWidth;
	xw->height	= iHeight;
	xw->visual  = DefaultVisualOfScreen(dfb_x11->screenptr);
    xw->depth   = DefaultDepth( dfb_x11->display, dfb_x11->screennum );

	xw->window = XCreateWindow(dfb_x11->display,
							 RootWindowOfScreen(dfb_x11->screenptr),
							 iXPos, iYPos, iWidth, iHeight, 0, xw->depth, InputOutput,
							 xw->visual, 0, NULL); 
							 
	if (!xw->window) {
         free( xw );
         return False;
    }


	XSizeHints Hints;

    /*
	* Here we inform the function of what we are going to change for the
	* window (there's also PPosition but it's obsolete)
	*/
	Hints.flags	=	PSize | PMinSize | PMaxSize;

    /*
	* Now we set the structure to the values we need for width & height.
	* For esthetic reasons we set Width=MinWidth=MaxWidth.
	* The same goes for Height. You can try whith differents values, or
	* let's use Hints.flags=Psize; and resize your window..
	*/
	Hints.min_width		=	Hints.max_width		=	Hints.base_width	=	xw->width;
	Hints.min_height	=	Hints.max_height	=	Hints.base_height	=	xw->height;

	/* Now we can set the size hints for the specified window */
	XSetWMNormalHints(dfb_x11->display,xw->window,&Hints);

	/* We change the title of the window (default:Untitled) */
	XStoreName(dfb_x11->display,xw->window,"DFB X11 system window");



	XSelectInput(dfb_x11->display, xw->window,
				 ExposureMask|KeyPressMask|KeyReleaseMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask);


	xw->gc=XCreateGC(dfb_x11->display, xw->window, 0, NULL);


	
	// Create a null cursor
	XColor  fore;
	XColor  back;
	xw->pixmp1 = XCreateBitmapFromData(	dfb_x11->display, xw->window, null_cursor_bits, 16, 16);
	xw->pixmp2 = XCreateBitmapFromData(	dfb_x11->display, xw->window, null_cursor_bits, 16, 16);
	xw->NullCursor = XCreatePixmapCursor(dfb_x11->display,xw->pixmp1,xw->pixmp2,&fore,&back,0,0);
	XDefineCursor(dfb_x11->display, xw->window, xw->NullCursor);
	
	
	/* maps the window and raises it to the top of the stack */
	XMapRaised(dfb_x11->display, xw->window);

	
	// Shared memory 	
	xw->shmseginfo=(XShmSegmentInfo *)malloc(sizeof(XShmSegmentInfo));
	if(!xw->shmseginfo) {
         XFreeGC(dfb_x11->display,xw->gc);
         XDestroyWindow(dfb_x11->display,xw->window);
         free( xw );
         return False;
    }

	memset(xw->shmseginfo,0, sizeof(XShmSegmentInfo));

	xw->ximage=XShmCreateImage(dfb_x11->display, xw->visual, xw->depth, ZPixmap,
							   NULL,xw->shmseginfo, xw->width, xw->height * 2);
	if(!xw->ximage) {
		printf("X11: Error creating shared image (XShmCreateImage) \n");
        free(xw->shmseginfo);
        XFreeGC(dfb_x11->display,xw->gc);
        XDestroyWindow(dfb_x11->display,xw->window);
        free( xw );
        return False;
	}
	
    xw->bpp = (xw->ximage->bits_per_pixel + 7) / 8;
	
    /* we firstly create our shared memory segment with the size we need, and
	correct permissions for the owner, the group and the world --> 0777 */
	xw->shmseginfo->shmid=shmget(IPC_PRIVATE, 
								 xw->ximage->bytes_per_line * xw->ximage->height * 2,
								 IPC_CREAT|0777);

	if(xw->shmseginfo->shmid<0) {
         XDestroyImage(xw->ximage);
         free(xw->shmseginfo);
         XFreeGC(dfb_x11->display,xw->gc);
         XDestroyWindow(dfb_x11->display,xw->window);
         free( xw );
         return False;
    }

    /* Then, we have to attach the segment to our process, and we let the
	function search the correct memory place --> NULL. It's safest ! */
	xw->shmseginfo->shmaddr = shmat( xw->shmseginfo->shmid, NULL, 0 );
	if(!xw->shmseginfo->shmaddr) {
         shmctl(xw->shmseginfo->shmid,IPC_RMID,NULL);
         XDestroyImage(xw->ximage);
         free(xw->shmseginfo);
         XFreeGC(dfb_x11->display,xw->gc);
         XDestroyWindow(dfb_x11->display,xw->window);
         free( xw );
         return False;
    }

	/* We set the buffer in Read and Write mode */
	xw->shmseginfo->readOnly=False;

	xw->virtualscreen= (unsigned char *) (xw->ximage->data = xw->shmseginfo->shmaddr );
	if(!XShmAttach(dfb_x11->display,xw->shmseginfo)) {
         shmdt(xw->shmseginfo->shmaddr);
         shmctl(xw->shmseginfo->shmid,IPC_RMID,NULL);
         XDestroyImage(xw->ximage);
         free(xw->shmseginfo);
         XFreeGC(dfb_x11->display,xw->gc);
         XDestroyWindow(dfb_x11->display,xw->window);
         free( xw );
         return False;
    }
	
    (*ppXW) = xw;

    return True;
}

void dfb_x11_close_window(XWindow* xw)
{
     XShmDetach(dfb_x11->display, xw->shmseginfo);
     shmdt(xw->shmseginfo->shmaddr);
     shmctl(xw->shmseginfo->shmid,IPC_RMID,NULL);
     XDestroyImage(xw->ximage);
     free(xw->shmseginfo);

     XFreeGC(dfb_x11->display,xw->gc);
     XDestroyWindow(dfb_x11->display,xw->window);
     free(xw);
}

