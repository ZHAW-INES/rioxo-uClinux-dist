#include "nxlib.h"
#include <string.h>
#include "Xutil.h"

void
XSetWMName(Display * display, Window w, XTextProperty * name)
{
	GR_WM_PROPERTIES props;

	if (!name || !name->value)
		return;

	props.flags = GR_WM_FLAGS_TITLE;
	props.title = name->value;
	GrSetWMProperties(w, &props);
}

/* Not used */
void
XSetWMIconName(Display * display, Window w, XTextProperty * name)
{
}

/* Not used */
void
XSetWMNormalHints(Display * display, Window w, XSizeHints * hints)
{
}

/* Not used */
int
XSetWMHints(Display * display, Window w, XWMHints * hints)
{
	return 1;
}

/* Not used */
void
XSetWMClassHints(Display * display, Window w, XClassHint * hints)
{
}

XSizeHints *
XAllocSizeHints(void)
{
	return (XSizeHints *)Xcalloc(1, (unsigned)sizeof(XSizeHints));
}

XWMHints *
XAllocWMHints(void)
{
	return (XWMHints *)Xcalloc(1, (unsigned)sizeof(XWMHints));
}

void
XSetWMProperties(Display * display, Window w, XTextProperty * window_name,
	XTextProperty * icon_name, char **argv, int argc,
	XSizeHints * normal_hints, XWMHints * wm_hints, XClassHint *class_hints)
{
	if (window_name)
		XSetWMName(display, w, window_name);
	if (icon_name)
		XSetWMIconName(display, w, icon_name);
	if (normal_hints)
		XSetWMNormalHints(display, w, normal_hints);
	if (wm_hints)
		XSetWMHints(display, w, wm_hints);
	if (class_hints)
		XSetWMClassHints(display, w, class_hints);
}

void
XmbSetWMProperties(Display *dpy, Window w, _Xconst char *windowName,
	_Xconst char *iconName, char **argv, int argc, XSizeHints * sizeHints,
	XWMHints * wmHints, XClassHint * classHints)
{
	XTextProperty wname, iname;
	XTextProperty *wprop = NULL;
	XTextProperty *iprop = NULL;

	/* fake up XTextProperty struct members for XSetWMName*/
	if (windowName) {
		wname.value = (char *)windowName;
		wprop = &wname;
	}
	if (iconName) {
		iname.value = (char *)iconName;
		iprop = &iname;
	}
	XSetWMProperties(dpy, w, wprop, iprop, argv, argc, sizeHints, wmHints,
		classHints);
}
