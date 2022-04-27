#include "LinuxErrorsDumper.h"

#include <pthread.h>

//used to retrieve network socket of Display structure
#define XLIB_ILLEGAL_ACCESS
#include <X11/Xlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <core/Logger.h>

namespace Gengine {
namespace Diagnostic {

LinuxErrorsDumper* LinuxErrorsDumper::m_instance = nullptr;

LinuxErrorsDumper::LinuxErrorsDumper()
{
    assert(m_instance == nullptr);
    m_instance = this;
    XSetErrorHandler ((XErrorHandler)LinuxErrorsDumper::ErrorHandler);
}

void LinuxErrorsDumper::WriteDump()
{}

int LinuxErrorsDumper::ErrorHandler(void* d, void *evt)
{
    XErrorEvent* event=(XErrorEvent*)evt;
    Display *dpy=(Display*)d;

    //////////////
    char buffer[BUFSIZ];
    char mesg[BUFSIZ];
    char number[32];
    GLOG_DEBUG("Current DISPLAY %s; XAUTHORITY %s",getenv("DISPLAY"),getenv("XAUTHORITY"));
    const char *mtype = "XlibMessage";
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
    GLOG_DEBUG("%s:  %s", mesg, buffer);
    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d",
    mesg, BUFSIZ);
    GLOG_DEBUG(mesg, event->request_code);
    if (event->request_code < 128)
    {
    sprintf(number, "%d", event->request_code);
    XGetErrorDatabaseText(dpy, "XRequest", number, "", buffer, BUFSIZ);
    }
    else
    {
        GLOG_DEBUG("Error from extension");
    }
    GLOG_DEBUG("(%s)", buffer);
    if (event->request_code >= 128)
    {
    XGetErrorDatabaseText(dpy, mtype, "MinorCode", "Request Minor code %d",
                  mesg, BUFSIZ);
    GLOG_DEBUG(mesg, event->minor_code);
    }
    if ((event->error_code == BadWindow) ||
           (event->error_code == BadPixmap) ||
           (event->error_code == BadCursor) ||
           (event->error_code == BadFont) ||
           (event->error_code == BadDrawable) ||
           (event->error_code == BadColor) ||
           (event->error_code == BadGC) ||
           (event->error_code == BadIDChoice) ||
           (event->error_code == BadValue) ||
           (event->error_code == BadAtom))
    {
    if (event->error_code == BadValue)
        XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x",
                  mesg, BUFSIZ);
    else if (event->error_code == BadAtom)
        XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x",
                  mesg, BUFSIZ);
    else
        XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
                      mesg, BUFSIZ);
    GLOG_DEBUG(mesg, event->resourceid);
    }
    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d",
              mesg, BUFSIZ);
    GLOG_DEBUG(mesg, event->serial);
    XGetErrorDatabaseText(dpy, mtype, "CurrentSerial", "Current Serial #%d",
              mesg, BUFSIZ);
    GLOG_DEBUG(mesg, -1);
    //////////////////
    return 0;
}

}
}
