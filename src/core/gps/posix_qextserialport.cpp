
/*!
\class Posix_QextSerialPort
\version 1.0.0
\author Stefan Sander

A cross-platform serial port class.
This class encapsulates the POSIX portion of QextSerialPort.  The user will be notified of errors
and possible portability conflicts at run-time by default - this behavior can be turned off by
defining _TTY_NOWARN_ (to turn off all warnings) or _TTY_NOWARN_PORT_ (to turn off portability
warnings) in the project.  Note that _TTY_NOWARN_ will also turn off portability warnings.
*/

#include <stdio.h>
#include "posix_qextserialport.h"
#include "qgslogger.h"

/*!
\fn Posix_QextSerialPort::Posix_QextSerialPort()
Default constructor.  Note that the name of the device used by a QextSerialPort constructed with
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

This constructor assigns the device name to the name of the first port on the specified system.
See the other constructors if you need to open a different port.
*/
Posix_QextSerialPort::Posix_QextSerialPort()
    : QextSerialBase()
{
  Posix_File = new QFile();
}

/*!
\fn Posix_QextSerialPort::Posix_QextSerialPort(const Posix_QextSerialPort&)
Copy constructor.
*/
Posix_QextSerialPort::Posix_QextSerialPort( const Posix_QextSerialPort& s )
    : QextSerialBase( s.port )
{
  setOpenMode( s.openMode() );
  port = s.port;
  Settings.BaudRate = s.Settings.BaudRate;
  Settings.DataBits = s.Settings.DataBits;
  Settings.Parity = s.Settings.Parity;
  Settings.StopBits = s.Settings.StopBits;
  Settings.FlowControl = s.Settings.FlowControl;
  lastErr = s.lastErr;

  Posix_File = new QFile();
  Posix_File = s.Posix_File;
  memcpy( &Posix_Timeout, &s.Posix_Timeout, sizeof( struct timeval ) );
  memcpy( &Posix_Copy_Timeout, &s.Posix_Copy_Timeout, sizeof( struct timeval ) );
  memcpy( &Posix_CommConfig, &s.Posix_CommConfig, sizeof( struct termios ) );
}

/*!
\fn Posix_QextSerialPort::Posix_QextSerialPort(const QString & name)
Constructs a serial port attached to the port specified by name.
name is the name of the device, which is windowsystem-specific,
e.g."COM1" or "/dev/ttyS0".
*/
Posix_QextSerialPort::Posix_QextSerialPort( const QString & name )
    : QextSerialBase( name )
{
  Posix_File = new QFile();
}

/*!
\fn Posix_QextSerialPort::Posix_QextSerialPort(const PortSettings& settings)
Constructs a port with default name and specified settings.
*/
Posix_QextSerialPort::Posix_QextSerialPort( const PortSettings& settings )
    : QextSerialBase()
{
  setBaudRate( settings.BaudRate );
  setDataBits( settings.DataBits );
  setParity( settings.Parity );
  setStopBits( settings.StopBits );
  setFlowControl( settings.FlowControl );

  Posix_File = new QFile();
  setTimeout( settings.Timeout_Sec, settings.Timeout_Millisec );
}

/*!
\fn Posix_QextSerialPort::Posix_QextSerialPort(const QString & name, const PortSettings& settings)
Constructs a port with specified name and settings.
*/
Posix_QextSerialPort::Posix_QextSerialPort( const QString & name, const PortSettings& settings )
    : QextSerialBase( name )
{
  setBaudRate( settings.BaudRate );
  setDataBits( settings.DataBits );
  setParity( settings.Parity );
  setStopBits( settings.StopBits );
  setFlowControl( settings.FlowControl );

  Posix_File = new QFile();
  setTimeout( settings.Timeout_Sec, settings.Timeout_Millisec );
}

/*!
\fn Posix_QextSerialPort& Posix_QextSerialPort::operator=(const Posix_QextSerialPort& s)
Override the = operator.
*/
Posix_QextSerialPort& Posix_QextSerialPort::operator=( const Posix_QextSerialPort & s )
{
  setOpenMode( s.openMode() );
  port = s.port;
  Settings.BaudRate = s.Settings.BaudRate;
  Settings.DataBits = s.Settings.DataBits;
  Settings.Parity = s.Settings.Parity;
  Settings.StopBits = s.Settings.StopBits;
  Settings.FlowControl = s.Settings.FlowControl;
  lastErr = s.lastErr;

  Posix_File = s.Posix_File;
  memcpy( &Posix_Timeout, &( s.Posix_Timeout ), sizeof( struct timeval ) );
  memcpy( &Posix_Copy_Timeout, &( s.Posix_Copy_Timeout ), sizeof( struct timeval ) );
  memcpy( &Posix_CommConfig, &( s.Posix_CommConfig ), sizeof( struct termios ) );
  return *this;
}

/*!
\fn Posix_QextSerialPort::~Posix_QextSerialPort()
Standard destructor.
*/
Posix_QextSerialPort::~Posix_QextSerialPort()
{
  if ( isOpen() )
  {
    close();
  }
  Posix_File->close();
  delete Posix_File;
}

/*!
\fn void Posix_QextSerialPort::setBaudRate(BaudRateType baudRate)
Sets the baud rate of the serial port.  Note that not all rates are applicable on
all platforms.  The following table shows translations of the various baud rate
constants on Windows(including NT/2000) and POSIX platforms.  Speeds marked with an *
are speeds that are usable on both Windows and POSIX.

\note
BAUD76800 may not be supported on all POSIX systems.  SGI/IRIX systems do not support
BAUD1800.

\verbatim

  RATE          Windows Speed   POSIX Speed
  -----------   -------------   -----------
   BAUD50                 110          50
   BAUD75                 110          75
  *BAUD110                110         110
   BAUD134                110         134.5
   BAUD150                110         150
   BAUD200                110         200
  *BAUD300                300         300
  *BAUD600                600         600
  *BAUD1200              1200        1200
   BAUD1800              1200        1800
  *BAUD2400              2400        2400
  *BAUD4800              4800        4800
  *BAUD9600              9600        9600
   BAUD14400            14400        9600
  *BAUD19200            19200       19200
  *BAUD38400            38400       38400
   BAUD56000            56000       38400
  *BAUD57600            57600       57600
   BAUD76800            57600       76800
  *BAUD115200          115200      115200
   BAUD128000          128000      115200
   BAUD256000          256000      115200
\endverbatim
*/
void Posix_QextSerialPort::setBaudRate( BaudRateType baudRate )
{
  LOCK_MUTEX();
  if ( Settings.BaudRate != baudRate )
  {
    switch ( baudRate )
    {
      case BAUD14400:
        Settings.BaudRate = BAUD9600;
        break;

      case BAUD56000:
        Settings.BaudRate = BAUD38400;
        break;

      case BAUD76800:

#ifndef B76800
        Settings.BaudRate = BAUD57600;
#else
        Settings.BaudRate = baudRate;
#endif
        break;

      case BAUD128000:
      case BAUD256000:
        Settings.BaudRate = BAUD115200;
        break;

      default:
        Settings.BaudRate = baudRate;
        break;
    }
  }
  if ( isOpen() )
  {
    switch ( baudRate )
    {

        /*50 baud*/
      case BAUD50:
        TTY_PORTABILITY_WARNING( "Posix_QextSerialPort Portability Warning: Windows does not support 50 baud operation." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B50;
#else
        cfsetispeed( &Posix_CommConfig, B50 );
        cfsetospeed( &Posix_CommConfig, B50 );
#endif
        break;

        /*75 baud*/
      case BAUD75:
        TTY_PORTABILITY_WARNING( "Posix_QextSerialPort Portability Warning: Windows does not support 75 baud operation." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B75;
#else
        cfsetispeed( &Posix_CommConfig, B75 );
        cfsetospeed( &Posix_CommConfig, B75 );
#endif
        break;

        /*110 baud*/
      case BAUD110:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B110;
#else
        cfsetispeed( &Posix_CommConfig, B110 );
        cfsetospeed( &Posix_CommConfig, B110 );
#endif
        break;

        /*134.5 baud*/
      case BAUD134:
        TTY_PORTABILITY_WARNING( "Posix_QextSerialPort Portability Warning: Windows does not support 134.5 baud operation." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B134;
#else
        cfsetispeed( &Posix_CommConfig, B134 );
        cfsetospeed( &Posix_CommConfig, B134 );
#endif
        break;

        /*150 baud*/
      case BAUD150:
        TTY_PORTABILITY_WARNING( "Posix_QextSerialPort Portability Warning: Windows does not support 150 baud operation." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B150;
#else
        cfsetispeed( &Posix_CommConfig, B150 );
        cfsetospeed( &Posix_CommConfig, B150 );
#endif
        break;

        /*200 baud*/
      case BAUD200:
        TTY_PORTABILITY_WARNING( "Posix_QextSerialPort Portability Warning: Windows does not support 200 baud operation." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B200;
#else
        cfsetispeed( &Posix_CommConfig, B200 );
        cfsetospeed( &Posix_CommConfig, B200 );
#endif
        break;

        /*300 baud*/
      case BAUD300:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B300;
#else
        cfsetispeed( &Posix_CommConfig, B300 );
        cfsetospeed( &Posix_CommConfig, B300 );
#endif
        break;

        /*600 baud*/
      case BAUD600:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B600;
#else
        cfsetispeed( &Posix_CommConfig, B600 );
        cfsetospeed( &Posix_CommConfig, B600 );
#endif
        break;

        /*1200 baud*/
      case BAUD1200:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B1200;
#else
        cfsetispeed( &Posix_CommConfig, B1200 );
        cfsetospeed( &Posix_CommConfig, B1200 );
#endif
        break;

        /*1800 baud*/
      case BAUD1800:
        TTY_PORTABILITY_WARNING( "Posix_QextSerialPort Portability Warning: Windows and IRIX do not support 1800 baud operation." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B1800;
#else
        cfsetispeed( &Posix_CommConfig, B1800 );
        cfsetospeed( &Posix_CommConfig, B1800 );
#endif
        break;

        /*2400 baud*/
      case BAUD2400:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B2400;
#else
        cfsetispeed( &Posix_CommConfig, B2400 );
        cfsetospeed( &Posix_CommConfig, B2400 );
#endif
        break;

        /*4800 baud*/
      case BAUD4800:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B4800;
#else
        cfsetispeed( &Posix_CommConfig, B4800 );
        cfsetospeed( &Posix_CommConfig, B4800 );
#endif
        break;

        /*9600 baud*/
      case BAUD9600:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B9600;
#else
        cfsetispeed( &Posix_CommConfig, B9600 );
        cfsetospeed( &Posix_CommConfig, B9600 );
#endif
        break;

        /*14400 baud*/
      case BAUD14400:
        TTY_WARNING( "Posix_QextSerialPort: POSIX does not support 14400 baud operation.  Switching to 9600 baud." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B9600;
#else
        cfsetispeed( &Posix_CommConfig, B9600 );
        cfsetospeed( &Posix_CommConfig, B9600 );
#endif
        break;

        /*19200 baud*/
      case BAUD19200:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B19200;
#else
        cfsetispeed( &Posix_CommConfig, B19200 );
        cfsetospeed( &Posix_CommConfig, B19200 );
#endif
        break;

        /*38400 baud*/
      case BAUD38400:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B38400;
#else
        cfsetispeed( &Posix_CommConfig, B38400 );
        cfsetospeed( &Posix_CommConfig, B38400 );
#endif
        break;

        /*56000 baud*/
      case BAUD56000:
        TTY_WARNING( "Posix_QextSerialPort: POSIX does not support 56000 baud operation.  Switching to 38400 baud." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B38400;
#else
        cfsetispeed( &Posix_CommConfig, B38400 );
        cfsetospeed( &Posix_CommConfig, B38400 );
#endif
        break;

        /*57600 baud*/
      case BAUD57600:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B57600;
#else
        cfsetispeed( &Posix_CommConfig, B57600 );
        cfsetospeed( &Posix_CommConfig, B57600 );
#endif
        break;

        /*76800 baud*/
      case BAUD76800:
        TTY_PORTABILITY_WARNING( "Posix_QextSerialPort Portability Warning: Windows and some POSIX systems do not support 76800 baud operation." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );

#ifdef B76800
        Posix_CommConfig.c_cflag |= B76800;
#else
        TTY_WARNING( "Posix_QextSerialPort: Posix_QextSerialPort was compiled without 76800 baud support.  Switching to 57600 baud." );
        Posix_CommConfig.c_cflag |= B57600;
#endif //B76800
#else  //CBAUD
#ifdef B76800
        cfsetispeed( &Posix_CommConfig, B76800 );
        cfsetospeed( &Posix_CommConfig, B76800 );
#else
        TTY_WARNING( "Posix_QextSerialPort: Posix_QextSerialPort was compiled without 76800 baud support.  Switching to 57600 baud." );
        cfsetispeed( &Posix_CommConfig, B57600 );
        cfsetospeed( &Posix_CommConfig, B57600 );
#endif //B76800
#endif //CBAUD
        break;

        /*115200 baud*/
      case BAUD115200:
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B115200;
#else
        cfsetispeed( &Posix_CommConfig, B115200 );
        cfsetospeed( &Posix_CommConfig, B115200 );
#endif
        break;

        /*128000 baud*/
      case BAUD128000:
        TTY_WARNING( "Posix_QextSerialPort: POSIX does not support 128000 baud operation.  Switching to 115200 baud." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B115200;
#else
        cfsetispeed( &Posix_CommConfig, B115200 );
        cfsetospeed( &Posix_CommConfig, B115200 );
#endif
        break;

        /*256000 baud*/
      case BAUD256000:
        TTY_WARNING( "Posix_QextSerialPort: POSIX does not support 256000 baud operation.  Switching to 115200 baud." );
#ifdef CBAUD
        Posix_CommConfig.c_cflag &= ( ~CBAUD );
        Posix_CommConfig.c_cflag |= B115200;
#else
        cfsetispeed( &Posix_CommConfig, B115200 );
        cfsetospeed( &Posix_CommConfig, B115200 );
#endif
        break;
    }
    tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
  }
  UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextSerialPort::setDataBits(DataBitsType dataBits)
Sets the number of data bits used by the serial port.  Possible values of dataBits are:
\verbatim
    DATA_5      5 data bits
    DATA_6      6 data bits
    DATA_7      7 data bits
    DATA_8      8 data bits
\endverbatim

\note
This function is subject to the following restrictions:
\par
    5 data bits cannot be used with 2 stop bits.
\par
    8 data bits cannot be used with space parity on POSIX systems.

*/
void Posix_QextSerialPort::setDataBits( DataBitsType dataBits )
{
  LOCK_MUTEX();
  if ( Settings.DataBits != dataBits )
  {
    if (( Settings.StopBits == STOP_2 && dataBits == DATA_5 ) ||
        ( Settings.StopBits == STOP_1_5 && dataBits != DATA_5 ) ||
        ( Settings.Parity == PAR_SPACE && dataBits == DATA_8 ) )
    {
    }
    else
    {
      Settings.DataBits = dataBits;
    }
  }
  if ( isOpen() )
  {
    switch ( dataBits )
    {

        /*5 data bits*/
      case DATA_5:
        if ( Settings.StopBits == STOP_2 )
        {
          TTY_WARNING( "Posix_QextSerialPort: 5 Data bits cannot be used with 2 stop bits." );
        }
        else
        {
          Settings.DataBits = dataBits;
          Posix_CommConfig.c_cflag &= ( ~CSIZE );
          Posix_CommConfig.c_cflag |= CS5;
          tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        }
        break;

        /*6 data bits*/
      case DATA_6:
        if ( Settings.StopBits == STOP_1_5 )
        {
          TTY_WARNING( "Posix_QextSerialPort: 6 Data bits cannot be used with 1.5 stop bits." );
        }
        else
        {
          Settings.DataBits = dataBits;
          Posix_CommConfig.c_cflag &= ( ~CSIZE );
          Posix_CommConfig.c_cflag |= CS6;
          tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        }
        break;

        /*7 data bits*/
      case DATA_7:
        if ( Settings.StopBits == STOP_1_5 )
        {
          TTY_WARNING( "Posix_QextSerialPort: 7 Data bits cannot be used with 1.5 stop bits." );
        }
        else
        {
          Settings.DataBits = dataBits;
          Posix_CommConfig.c_cflag &= ( ~CSIZE );
          Posix_CommConfig.c_cflag |= CS7;
          tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        }
        break;

        /*8 data bits*/
      case DATA_8:
        if ( Settings.StopBits == STOP_1_5 )
        {
          TTY_WARNING( "Posix_QextSerialPort: 8 Data bits cannot be used with 1.5 stop bits." );
        }
        else
        {
          Settings.DataBits = dataBits;
          Posix_CommConfig.c_cflag &= ( ~CSIZE );
          Posix_CommConfig.c_cflag |= CS8;
          tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        }
        break;
    }
  }
  UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextSerialPort::setParity(ParityType parity)
Sets the parity associated with the serial port.  The possible values of parity are:
\verbatim
    PAR_SPACE       Space Parity
    PAR_MARK        Mark Parity
    PAR_NONE        No Parity
    PAR_EVEN        Even Parity
    PAR_ODD         Odd Parity
\endverbatim

\note
This function is subject to the following limitations:
\par
POSIX systems do not support mark parity.
\par
POSIX systems support space parity only if tricked into doing so, and only with
   fewer than 8 data bits.  Use space parity very carefully with POSIX systems.

*/
void Posix_QextSerialPort::setParity( ParityType parity )
{
  LOCK_MUTEX();
  if ( Settings.Parity != parity )
  {
    if ( parity == PAR_MARK || ( parity == PAR_SPACE && Settings.DataBits == DATA_8 ) )
    {
    }
    else
    {
      Settings.Parity = parity;
    }
  }
  if ( isOpen() )
  {
    switch ( parity )
    {

        /*space parity*/
      case PAR_SPACE:
        if ( Settings.DataBits == DATA_8 )
        {
          TTY_PORTABILITY_WARNING( "Posix_QextSerialPort:  Space parity is only supported in POSIX with 7 or fewer data bits" );
        }
        else
        {

          /*space parity not directly supported - add an extra data bit to simulate it*/
          Posix_CommConfig.c_cflag &= ~( PARENB | CSIZE );
          switch ( Settings.DataBits )
          {
            case DATA_5:
              Settings.DataBits = DATA_6;
              Posix_CommConfig.c_cflag |= CS6;
              break;

            case DATA_6:
              Settings.DataBits = DATA_7;
              Posix_CommConfig.c_cflag |= CS7;
              break;

            case DATA_7:
              Settings.DataBits = DATA_8;
              Posix_CommConfig.c_cflag |= CS8;
              break;

            case DATA_8:
              break;
          }
          tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        }
        break;

        /*mark parity - WINDOWS ONLY*/
      case PAR_MARK:
        TTY_WARNING( "Posix_QextSerialPort: Mark parity is not supported by POSIX." );
        break;

        /*no parity*/
      case PAR_NONE:
        Posix_CommConfig.c_cflag &= ( ~PARENB );
        tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        break;

        /*even parity*/
      case PAR_EVEN:
        Posix_CommConfig.c_cflag &= ( ~PARODD );
        Posix_CommConfig.c_cflag |= PARENB;
        tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        break;

        /*odd parity*/
      case PAR_ODD:
        Posix_CommConfig.c_cflag |= ( PARENB | PARODD );
        tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        break;
    }
  }
  UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextSerialPort::setStopBits(StopBitsType stopBits)
Sets the number of stop bits used by the serial port.  Possible values of stopBits are:
\verbatim
    STOP_1      1 stop bit
    STOP_1_5    1.5 stop bits
    STOP_2      2 stop bits
\endverbatim
\note
This function is subject to the following restrictions:
\par
    2 stop bits cannot be used with 5 data bits.
\par
    POSIX does not support 1.5 stop bits.

*/
void Posix_QextSerialPort::setStopBits( StopBitsType stopBits )
{
  LOCK_MUTEX();
  if ( Settings.StopBits != stopBits )
  {
    if (( Settings.DataBits == DATA_5 && stopBits == STOP_2 ) || stopBits == STOP_1_5 ) {}
    else
    {
      Settings.StopBits = stopBits;
    }
  }
  if ( isOpen() )
  {
    switch ( stopBits )
    {

        /*one stop bit*/
      case STOP_1:
        Settings.StopBits = stopBits;
        Posix_CommConfig.c_cflag &= ( ~CSTOPB );
        tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        break;

        /*1.5 stop bits*/
      case STOP_1_5:
        TTY_WARNING( "Posix_QextSerialPort: 1.5 stop bit operation is not supported by POSIX." );
        break;

        /*two stop bits*/
      case STOP_2:
        if ( Settings.DataBits == DATA_5 )
        {
          TTY_WARNING( "Posix_QextSerialPort: 2 stop bits cannot be used with 5 data bits" );
        }
        else
        {
          Settings.StopBits = stopBits;
          Posix_CommConfig.c_cflag |= CSTOPB;
          tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        }
        break;
    }
  }
  UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextSerialPort::setFlowControl(FlowType flow)
Sets the flow control used by the port.  Possible values of flow are:
\verbatim
    FLOW_OFF            No flow control
    FLOW_HARDWARE       Hardware (RTS/CTS) flow control
    FLOW_XONXOFF        Software (XON/XOFF) flow control
\endverbatim
\note
FLOW_HARDWARE may not be supported on all versions of UNIX.  In cases where it is
unsupported, FLOW_HARDWARE is the same as FLOW_OFF.

*/
void Posix_QextSerialPort::setFlowControl( FlowType flow )
{
  LOCK_MUTEX();
  if ( Settings.FlowControl != flow )
  {
    Settings.FlowControl = flow;
  }
  if ( isOpen() )
  {
    switch ( flow )
    {

        /*no flow control*/
      case FLOW_OFF:
        Posix_CommConfig.c_cflag &= ( ~CRTSCTS );
        Posix_CommConfig.c_iflag &= ( ~( IXON | IXOFF | IXANY ) );
        tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        break;

        /*software (XON/XOFF) flow control*/
      case FLOW_XONXOFF:
        Posix_CommConfig.c_cflag &= ( ~CRTSCTS );
        Posix_CommConfig.c_iflag |= ( IXON | IXOFF | IXANY );
        tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        break;

      case FLOW_HARDWARE:
        Posix_CommConfig.c_cflag |= CRTSCTS;
        Posix_CommConfig.c_iflag &= ( ~( IXON | IXOFF | IXANY ) );
        tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
        break;
    }
  }
  UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextSerialPort::setTimeout(ulong sec, ulong millisec);
Sets the read and write timeouts for the port to sec seconds and millisec milliseconds.
Note that this is a per-character timeout, i.e. the port will wait this long for each
individual character, not for the whole read operation.  This timeout also applies to the
bytesWaiting() function.

\note
POSIX does not support millisecond-level control for I/O timeout values.  Any
timeout set using this function will be set to the next lowest tenth of a second for
the purposes of detecting read or write timeouts.  For example a timeout of 550 milliseconds
will be seen by the class as a timeout of 500 milliseconds for the purposes of reading and
writing the port.  However millisecond-level control is allowed by the select() system call,
so for example a 550-millisecond timeout will be seen as 550 milliseconds on POSIX systems for
the purpose of detecting available bytes in the read buffer.

*/
void Posix_QextSerialPort::setTimeout( ulong sec, ulong millisec )
{
  LOCK_MUTEX();
  Settings.Timeout_Sec = sec;
  Settings.Timeout_Millisec = millisec;
  Posix_Copy_Timeout.tv_sec = sec;
  Posix_Copy_Timeout.tv_usec = millisec;
  if ( isOpen() )
  {
    tcgetattr( Posix_File->handle(), &Posix_CommConfig );
    Posix_CommConfig.c_cc[VTIME] = sec * 10 + millisec / 100;
    tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
  }
  UNLOCK_MUTEX();
}

/*!
\fn bool Posix_QextSerialPort::open(OpenMode mode)
Opens the serial port associated to this class.
This function has no effect if the port associated with the class is already open.
The port is also configured to the current settings, as stored in the Settings structure.
*/
bool Posix_QextSerialPort::open( OpenMode mode )
{
  LOCK_MUTEX();
  if ( mode == QIODevice::NotOpen )
    return isOpen();
  if ( !isOpen() )
  {
    /*open the port*/
    Posix_File->setFileName( port );
    QgsDebugMsg( "Trying to open File" );
    if ( Posix_File->open( QIODevice::ReadWrite | QIODevice::Unbuffered ) )
    {
      QgsDebugMsg( "Opened File successfully" );
      /*set open mode*/
      QIODevice::open( mode );

      /*configure port settings*/
      tcgetattr( Posix_File->handle(), &Posix_CommConfig );

      /*set up other port settings*/
      Posix_CommConfig.c_cflag |= CREAD | CLOCAL;
      Posix_CommConfig.c_lflag &= ( ~( ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG ) );
      Posix_CommConfig.c_iflag &= ( ~( INPCK | IGNPAR | PARMRK | ISTRIP | ICRNL | IXANY ) );
      Posix_CommConfig.c_oflag &= ( ~OPOST );
      Posix_CommConfig.c_cc[VMIN] = 0;
      Posix_CommConfig.c_cc[VINTR] = _POSIX_VDISABLE;
      Posix_CommConfig.c_cc[VQUIT] = _POSIX_VDISABLE;
      Posix_CommConfig.c_cc[VSTART] = _POSIX_VDISABLE;
      Posix_CommConfig.c_cc[VSTOP] = _POSIX_VDISABLE;
      Posix_CommConfig.c_cc[VSUSP] = _POSIX_VDISABLE;
      setBaudRate( Settings.BaudRate );
      setDataBits( Settings.DataBits );
      setParity( Settings.Parity );
      setStopBits( Settings.StopBits );
      setFlowControl( Settings.FlowControl );
      setTimeout( Settings.Timeout_Sec, Settings.Timeout_Millisec );
      tcsetattr( Posix_File->handle(), TCSAFLUSH, &Posix_CommConfig );
    }
    else
    {
      QgsDebugMsg( QString( "Could not open File! Error code: %1" ).arg( Posix_File->error() ) );
    }
  }
  UNLOCK_MUTEX();
  return isOpen();
}

/*!
\fn void Posix_QextSerialPort::close()
Closes a serial port.  This function has no effect if the serial port associated with the class
is not currently open.
*/
void Posix_QextSerialPort::close()
{
  LOCK_MUTEX();
  Posix_File->close();
  QIODevice::close();
  UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextSerialPort::flush()
Flushes all pending I/O to the serial port.  This function has no effect if the serial port
associated with the class is not currently open.
*/
void Posix_QextSerialPort::flush()
{
  LOCK_MUTEX();
  if ( isOpen() )
  {
    Posix_File->flush();
  }
  UNLOCK_MUTEX();
}

/*!
\fn qint64 Posix_QextSerialPort::size() const
This function will return the number of bytes waiting in the receive queue of the serial port.
It is included primarily to provide a complete QIODevice interface, and will not record errors
in the lastErr member (because it is const).  This function is also not thread-safe - in
multithreading situations, use Posix_QextSerialPort::bytesWaiting() instead.
*/
qint64 Posix_QextSerialPort::size() const
{
  int numBytes;
  if ( ioctl( Posix_File->handle(), FIONREAD, &numBytes ) < 0 )
  {
    numBytes = 0;
  }
  return ( qint64 )numBytes;
}

/*!
\fn qint64 Posix_QextSerialPort::bytesAvailable()
Returns the number of bytes waiting in the port's receive queue.  This function will return 0 if
the port is not currently open, or -1 on error.  Error information can be retrieved by calling
Posix_QextSerialPort::getLastError().
*/
qint64 Posix_QextSerialPort::bytesAvailable()
{
  LOCK_MUTEX();
  if ( isOpen() )
  {
    int bytesQueued;
    fd_set fileSet;
    FD_ZERO( &fileSet );
    FD_SET( Posix_File->handle(), &fileSet );

    /*on Linux systems the Posix_Timeout structure will be altered by the select() call.
      Make sure we use the right timeout values*/
    //memcpy(&Posix_Timeout, &Posix_Copy_Timeout, sizeof(struct timeval));
    Posix_Timeout = Posix_Copy_Timeout;
    int n = select( Posix_File->handle() + 1, &fileSet, NULL, &fileSet, &Posix_Timeout );
    if ( !n )
    {
      lastErr = E_PORT_TIMEOUT;
      UNLOCK_MUTEX();
      return -1;
    }
    if ( n == -1 || ioctl( Posix_File->handle(), FIONREAD, &bytesQueued ) == -1 )
    {
      translateError( errno );
      UNLOCK_MUTEX();
      return -1;
    }
    lastErr = E_NO_ERROR;
    UNLOCK_MUTEX();
    return bytesQueued + QIODevice::bytesAvailable();
  }
  UNLOCK_MUTEX();
  return 0;
}

/*!
\fn void Posix_QextSerialPort::ungetChar(char)
This function is included to implement the full QIODevice interface, and currently has no
purpose within this class.  This function is meaningless on an unbuffered device and currently
only prints a warning message to that effect.
*/
void Posix_QextSerialPort::ungetChar( char )
{
  /*meaningless on unbuffered sequential device - return error and print a warning*/
  TTY_WARNING( "Posix_QextSerialPort: ungetChar() called on an unbuffered sequential device - operation is meaningless" );
}

/*!
\fn void Posix_QextSerialPort::translateError(ulong error)
Translates a system-specific error code to a QextSerialPort error code.  Used internally.
*/
void Posix_QextSerialPort::translateError( ulong error )
{
  switch ( error )
  {
    case EBADF:
    case ENOTTY:
      lastErr = E_INVALID_FD;
      break;

    case EINTR:
      lastErr = E_CAUGHT_NON_BLOCKED_SIGNAL;
      break;

    case ENOMEM:
      lastErr = E_NO_MEMORY;
      break;
  }
}

/*!
\fn void Posix_QextSerialPort::setDtr(bool set)
Sets DTR line to the requested state (high by default).  This function will have no effect if
the port associated with the class is not currently open.
*/
void Posix_QextSerialPort::setDtr( bool set )
{
  LOCK_MUTEX();
  if ( isOpen() )
  {
    int status;
    ioctl( Posix_File->handle(), TIOCMGET, &status );
    if ( set )
    {
      status |= TIOCM_DTR;
    }
    else
    {
      status &= ~TIOCM_DTR;
    }
    ioctl( Posix_File->handle(), TIOCMSET, &status );
  }
  UNLOCK_MUTEX();
}

/*!
\fn void Posix_QextSerialPort::setRts(bool set)
Sets RTS line to the requested state (high by default).  This function will have no effect if
the port associated with the class is not currently open.
*/
void Posix_QextSerialPort::setRts( bool set )
{
  LOCK_MUTEX();
  if ( isOpen() )
  {
    int status;
    ioctl( Posix_File->handle(), TIOCMGET, &status );
    if ( set )
    {
      status |= TIOCM_RTS;
    }
    else
    {
      status &= ~TIOCM_RTS;
    }
    ioctl( Posix_File->handle(), TIOCMSET, &status );
  }
  UNLOCK_MUTEX();
}

/*!
\fn unsigned long Posix_QextSerialPort::lineStatus()
returns the line status as stored by the port function.  This function will retrieve the states
of the following lines: DCD, CTS, DSR, and RI.  On POSIX systems, the following additional lines
can be monitored: DTR, RTS, Secondary TXD, and Secondary RXD.  The value returned is an unsigned
long with specific bits indicating which lines are high.  The following constants should be used
to examine the states of individual lines:

\verbatim
Mask        Line
------      ----
LS_CTS      CTS
LS_DSR      DSR
LS_DCD      DCD
LS_RI       RI
LS_RTS      RTS (POSIX only)
LS_DTR      DTR (POSIX only)
LS_ST       Secondary TXD (POSIX only)
LS_SR       Secondary RXD (POSIX only)
\endverbatim

This function will return 0 if the port associated with the class is not currently open.
*/
unsigned long Posix_QextSerialPort::lineStatus()
{
  unsigned long Status = 0, Temp = 0;
  LOCK_MUTEX();
  if ( isOpen() )
  {
    ioctl( Posix_File->handle(), TIOCMGET, &Temp );
    if ( Temp&TIOCM_CTS )
    {
      Status |= LS_CTS;
    }
    if ( Temp&TIOCM_DSR )
    {
      Status |= LS_DSR;
    }
    if ( Temp&TIOCM_RI )
    {
      Status |= LS_RI;
    }
    if ( Temp&TIOCM_CD )
    {
      Status |= LS_DCD;
    }
    if ( Temp&TIOCM_DTR )
    {
      Status |= LS_DTR;
    }
    if ( Temp&TIOCM_RTS )
    {
      Status |= LS_RTS;
    }
    if ( Temp&TIOCM_ST )
    {
      Status |= LS_ST;
    }
    if ( Temp&TIOCM_SR )
    {
      Status |= LS_SR;
    }
  }
  UNLOCK_MUTEX();
  return Status;
}

/*!
\fn qint64 Posix_QextSerialPort::readData(char * data, qint64 maxSize)
Reads a block of data from the serial port.  This function will read at most maxSize bytes from
the serial port and place them in the buffer pointed to by data.  Return value is the number of
bytes actually read, or -1 on error.

\warning before calling this function ensure that serial port associated with this class
is currently open (use isOpen() function to check if port is open).
*/
qint64 Posix_QextSerialPort::readData( char * data, qint64 maxSize )
{
  LOCK_MUTEX();
  int retVal = 0;
  retVal = Posix_File->read( data, maxSize );
  if ( retVal == -1 )
    lastErr = E_READ_FAILED;
  UNLOCK_MUTEX();

  return retVal;
}

/*!
\fn qint64 Posix_QextSerialPort::writeData(const char * data, qint64 maxSize)
Writes a block of data to the serial port.  This function will write maxSize bytes
from the buffer pointed to by data to the serial port.  Return value is the number
of bytes actually written, or -1 on error.

\warning before calling this function ensure that serial port associated with this class
is currently open (use isOpen() function to check if port is open).
*/
qint64 Posix_QextSerialPort::writeData( const char * data, qint64 maxSize )
{
  LOCK_MUTEX();
  int retVal = 0;
  retVal = Posix_File->write( data, maxSize );
  if ( retVal == -1 )
    lastErr = E_WRITE_FAILED;
  UNLOCK_MUTEX();

  flush();
  return retVal;
}
