
#include "qextserialbase.h"

/*!
\class QextSerialBase
\version 1.0.0
\author Stefan Sander

A common base class for Win_QextSerialBase, Posix_QextSerialBase and QextSerialPort.
*/
#ifdef QT_THREAD_SUPPORT
QMutex* QextSerialBase::mutex = NULL;
unsigned long QextSerialBase::refCount = 0;
#endif

/*!
\fn QextSerialBase::QextSerialBase()
Default constructor.
*/
QextSerialBase::QextSerialBase()
    : QIODevice()
{

#ifdef _TTY_WIN_
  setPortName( "COM1" );

#elif defined(_TTY_IRIX_)
  setPortName( "/dev/ttyf1" );

#elif defined(_TTY_HPUX_)
  setPortName( "/dev/tty1p0" );

#elif defined(_TTY_SUN_)
  setPortName( "/dev/ttya" );

#elif defined(_TTY_DIGITAL_)
  setPortName( "/dev/tty01" );

#elif defined(_TTY_FREEBSD_)
  setPortName( "/dev/ttyd1" );

#else
  setPortName( "/dev/ttyS0" );
#endif

  construct();
}

/*!
\fn QextSerialBase::QextSerialBase(const QString & name)
Construct a port and assign it to the device specified by the name parameter.
*/
QextSerialBase::QextSerialBase( const QString & name )
    : QIODevice()
{
  setPortName( name );
  construct();
}

/*!
\fn QextSerialBase::~QextSerialBase()
Standard destructor.
*/
QextSerialBase::~QextSerialBase()
{

#ifdef QT_THREAD_SUPPORT
  refCount--;
  if ( mutex && refCount == 0 )
  {
    delete mutex;
    mutex = NULL;
  }
#endif

}

/*!
\fn void QextSerialBase::construct()
Common constructor function for setting up default port settings.
(115200 Baud, 8N1, Hardware flow control where supported, otherwise no flow control, and 500 ms timeout).
*/
void QextSerialBase::construct()
{
  Settings.BaudRate = BAUD115200;
  Settings.DataBits = DATA_8;
  Settings.Parity = PAR_NONE;
  Settings.StopBits = STOP_1;
  Settings.FlowControl = FLOW_HARDWARE;
  Settings.Timeout_Sec = 0;
  Settings.Timeout_Millisec = 500;

#ifdef QT_THREAD_SUPPORT
  if ( !mutex )
  {
    mutex = new QMutex( QMutex::Recursive );
  }
  refCount++;
#endif

  setOpenMode( QIODevice::NotOpen );
}

/*!
\fn void QextSerialBase::setPortName(const QString & name)
Sets the name of the device associated with the object, e.g. "COM1", or "/dev/ttyS0".
*/
void QextSerialBase::setPortName( const QString & name )
{
  port = name;
}

/*!
\fn QString QextSerialBase::portName() const
Returns the name set by setPortName().
*/
QString QextSerialBase::portName() const
{
  return port;
}

/*!
\fn BaudRateType QextSerialBase::baudRate(void) const
Returns the baud rate of the serial port.  For a list of possible return values see
the definition of the enum BaudRateType.
*/
BaudRateType QextSerialBase::baudRate( void ) const
{
  return Settings.BaudRate;
}

/*!
\fn DataBitsType QextSerialBase::dataBits() const
Returns the number of data bits used by the port.  For a list of possible values returned by
this function, see the definition of the enum DataBitsType.
*/
DataBitsType QextSerialBase::dataBits() const
{
  return Settings.DataBits;
}

/*!
\fn ParityType QextSerialBase::parity() const
Returns the type of parity used by the port.  For a list of possible values returned by
this function, see the definition of the enum ParityType.
*/
ParityType QextSerialBase::parity() const
{
  return Settings.Parity;
}

/*!
\fn StopBitsType QextSerialBase::stopBits() const
Returns the number of stop bits used by the port.  For a list of possible return values, see
the definition of the enum StopBitsType.
*/
StopBitsType QextSerialBase::stopBits() const
{
  return Settings.StopBits;
}

/*!
\fn FlowType QextSerialBase::flowControl() const
Returns the type of flow control used by the port.  For a list of possible values returned
by this function, see the definition of the enum FlowType.
*/
FlowType QextSerialBase::flowControl() const
{
  return Settings.FlowControl;
}

/*!
\fn bool QextSerialBase::isSequential() const
Returns true if device is sequential, otherwise returns false. Serial port is sequential device
so this function always returns true. Check QIODevice::isSequential() documentation for more
information.
*/
bool QextSerialBase::isSequential() const
{
  return true;
}

/*!
\fn bool QextSerialBase::atEnd() const
This function will return true if the input buffer is empty (or on error), and false otherwise.
Call QextSerialBase::lastError() for error information.
*/
bool QextSerialBase::atEnd() const
{
  if ( size() )
  {
    return true;
  }
  return false;
}

/*!
\fn qint64 QextSerialBase::readLine(char * data, qint64 maxSize)
This function will read a line of buffered input from the port, stopping when either maxSize bytes
have been read, the port has no more data available, or a newline is encountered.
The value returned is the length of the string that was read.
*/
qint64 QextSerialBase::readLine( char * data, qint64 maxSize )
{
  qint64 numBytes = bytesAvailable();
  char* pData = data;

  if ( maxSize < 2 ) //maxSize must be larger than 1
    return -1;

  /*read a byte at a time for MIN(bytesAvail, maxSize - 1) iterations, or until a newline*/
  while ( pData < ( data + numBytes ) && --maxSize )
  {
    readData( pData, 1 );
    if ( *pData++ == '\n' )
    {
      break;
    }
  }
  *pData = '\0';

  /*return size of data read*/
  return ( pData -data );
}

/*!
\fn ulong QextSerialBase::lastError() const
Returns the code for the last error encountered by the port, or E_NO_ERROR if the last port
operation was successful.  Possible error codes are:

\verbatim
Error                           Explanation
---------------------------     -------------------------------------------------------------
E_NO_ERROR                      No Error has occured
E_INVALID_FD                    Invalid file descriptor (port was not opened correctly)
E_NO_MEMORY                     Unable to allocate memory tables (POSIX)
E_CAUGHT_NON_BLOCKED_SIGNAL     Caught a non-blocked signal (POSIX)
E_PORT_TIMEOUT                  Operation timed out (POSIX)
E_INVALID_DEVICE                The file opened by the port is not a character device (POSIX)
E_BREAK_CONDITION               The port detected a break condition
E_FRAMING_ERROR                 The port detected a framing error
                                (usually caused by incorrect baud rate settings)
E_IO_ERROR                      There was an I/O error while communicating with the port
E_BUFFER_OVERRUN                Character buffer overrun
E_RECEIVE_OVERFLOW              Receive buffer overflow
E_RECEIVE_PARITY_ERROR          The port detected a parity error in the received data
E_TRANSMIT_OVERFLOW             Transmit buffer overflow
E_READ_FAILED                   General read operation failure
E_WRITE_FAILED                  General write operation failure
\endverbatim
*/
ulong QextSerialBase::lastError() const
{
  return lastErr;
}
