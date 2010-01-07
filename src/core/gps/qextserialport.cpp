
/*!
\class QextSerialPort
\version 1.0.0
\author Stefan Sander

A cross-platform serial port class.
This class encapsulates a serial port on both POSIX and Windows systems.  The user will be
notified of errors and possible portability conflicts at run-time by default - this behavior can
be turned off by defining _TTY_NOWARN_ (to turn off all warnings) or _TTY_NOWARN_PORT_ (to turn
off portability warnings) in the project.

\note
On Windows NT/2000/XP this class uses Win32 serial port functions by default.  The user may
select POSIX behavior under NT, 2000, or XP ONLY by defining _TTY_POSIX_ in the project. I can
make no guarantees as to the quality of POSIX support under NT/2000 however.

*/

#include <stdio.h>
#include "qextserialport.h"

/*!
\fn QextSerialPort::QextSerialPort()
Default constructor.  Note that the naming convention used by a QextSerialPort constructed with
this constructor will be determined by #defined constants, or lack thereof - the default behavior
is the same as _TTY_LINUX_.  Possible naming conventions and their associated constants are:

\verbatim

Constant         Used By         Naming Convention
----------       -------------   ------------------------
_TTY_WIN_        Windows         COM1, COM2
_TTY_IRIX_       SGI/IRIX        /dev/ttyf1, /dev/ttyf2
_TTY_HPUX_       HP-UX           /dev/tty1p0, /dev/tty2p0
_TTY_SUN_        SunOS/Solaris   /dev/ttya, /dev/ttyb
_TTY_DIGITAL_    Digital UNIX    /dev/tty01, /dev/tty02
_TTY_FREEBSD_    FreeBSD         /dev/ttyd0, /dev/ttyd1
_TTY_LINUX_      Linux           /dev/ttyS0, /dev/ttyS1
<none>           Linux           /dev/ttyS0, /dev/ttyS1
\endverbatim

The object will be associated with the first port in the system, e.g. COM1 on Windows systems.
See the other constructors if you need to use a port other than the first.
*/
QextSerialPort::QextSerialPort()
    : QextBaseType()
{}

/*!
\fn QextSerialPort::QextSerialPort(const QString & name)
Constructs a serial port attached to the port specified by name.
name is the name of the device, which is windowsystem-specific,
e.g."COM1" or "/dev/ttyS0".
*/
QextSerialPort::QextSerialPort( const QString & name )
    : QextBaseType( name )
{}

/*!
\fn QextSerialPort::QextSerialPort(PortSettings const& settings)
Constructs a port with default name and settings specified by the settings parameter.
*/
QextSerialPort::QextSerialPort( PortSettings const& settings )
    : QextBaseType( settings )
{}

/*!
\fn QextSerialPort::QextSerialPort(const QString & name, PortSettings const& settings)
Constructs a port with the name and settings specified.
*/
QextSerialPort::QextSerialPort( const QString & name, PortSettings const& settings )
    : QextBaseType( name, settings )
{}

/*!
\fn QextSerialPort::QextSerialPort(const QextSerialPort& s)
Copy constructor.
*/
QextSerialPort::QextSerialPort( const QextSerialPort& s )
    : QextBaseType( s )
{}

/*!
\fn QextSerialPort& QextSerialPort::operator=(const QextSerialPort& s)
Overrides the = operator.
*/
QextSerialPort& QextSerialPort::operator=( const QextSerialPort & s )
{
  return ( QextSerialPort& )QextBaseType::operator=( s );
}

/*!
\fn QextSerialPort::~QextSerialPort()
Standard destructor.
*/
QextSerialPort::~QextSerialPort()
{}
