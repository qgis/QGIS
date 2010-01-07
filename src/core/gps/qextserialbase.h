
#ifndef _QEXTSERIALBASE_H_
#define _QEXTSERIALBASE_H_

#include <QIODevice>
#include <QFile>

#ifdef QT_THREAD_SUPPORT
#include <QThread>
#include <QMutex>
#endif

/*if all warning messages are turned off, flag portability warnings to be turned off as well*/
#ifdef _TTY_NOWARN_
#define _TTY_NOWARN_PORT_
#endif

/*macros for thread support*/
#ifdef QT_THREAD_SUPPORT
#define LOCK_MUTEX() mutex->lock()
#define UNLOCK_MUTEX() mutex->unlock()
#else
#define LOCK_MUTEX()
#define UNLOCK_MUTEX()
#endif

/*macros for warning messages*/
#ifdef _TTY_NOWARN_PORT_
#define TTY_PORTABILITY_WARNING(s)
#else
#define TTY_PORTABILITY_WARNING(s) qWarning(s)
#endif
#ifdef _TTY_NOWARN_
#define TTY_WARNING(s)
#else
#define TTY_WARNING(s) qWarning(s)
#endif


/*line status constants*/
#define LS_CTS  0x01
#define LS_DSR  0x02
#define LS_DCD  0x04
#define LS_RI   0x08
#define LS_RTS  0x10
#define LS_DTR  0x20
#define LS_ST   0x40
#define LS_SR   0x80

/*error constants*/
#define E_NO_ERROR                   0
#define E_INVALID_FD                 1
#define E_NO_MEMORY                  2
#define E_CAUGHT_NON_BLOCKED_SIGNAL  3
#define E_PORT_TIMEOUT               4
#define E_INVALID_DEVICE             5
#define E_BREAK_CONDITION            6
#define E_FRAMING_ERROR              7
#define E_IO_ERROR                   8
#define E_BUFFER_OVERRUN             9
#define E_RECEIVE_OVERFLOW          10
#define E_RECEIVE_PARITY_ERROR      11
#define E_TRANSMIT_OVERFLOW         12
#define E_READ_FAILED               13
#define E_WRITE_FAILED              14

/*enums for port settings*/
enum NamingConvention
{
  WIN_NAMES,
  IRIX_NAMES,
  HPUX_NAMES,
  SUN_NAMES,
  DIGITAL_NAMES,
  FREEBSD_NAMES,
  LINUX_NAMES
};

enum BaudRateType
{
  BAUD50,                //POSIX ONLY
  BAUD75,                //POSIX ONLY
  BAUD110,
  BAUD134,               //POSIX ONLY
  BAUD150,               //POSIX ONLY
  BAUD200,               //POSIX ONLY
  BAUD300,
  BAUD600,
  BAUD1200,
  BAUD1800,              //POSIX ONLY
  BAUD2400,
  BAUD4800,
  BAUD9600,
  BAUD14400,             //WINDOWS ONLY
  BAUD19200,
  BAUD38400,
  BAUD56000,             //WINDOWS ONLY
  BAUD57600,
  BAUD76800,             //POSIX ONLY
  BAUD115200,
  BAUD128000,            //WINDOWS ONLY
  BAUD256000             //WINDOWS ONLY
};

enum DataBitsType
{
  DATA_5,
  DATA_6,
  DATA_7,
  DATA_8
};

enum ParityType
{
  PAR_NONE,
  PAR_ODD,
  PAR_EVEN,
  PAR_MARK,               //WINDOWS ONLY
  PAR_SPACE
};

enum StopBitsType
{
  STOP_1,
  STOP_1_5,               //WINDOWS ONLY
  STOP_2
};

enum FlowType
{
  FLOW_OFF,
  FLOW_HARDWARE,
  FLOW_XONXOFF
};

/*structure to contain port settings*/
struct PortSettings
{
  BaudRateType BaudRate;
  DataBitsType DataBits;
  ParityType Parity;
  StopBitsType StopBits;
  FlowType FlowControl;
  ulong Timeout_Sec;
  ulong Timeout_Millisec;
};

class CORE_EXPORT QextSerialBase : public QIODevice
{
  public:
    QextSerialBase();
    QextSerialBase( const QString & name );
    virtual ~QextSerialBase();
    virtual void construct();
    virtual void setPortName( const QString & name );
    virtual QString portName() const;

    virtual void setBaudRate( BaudRateType ) = 0;
    virtual BaudRateType baudRate() const;
    virtual void setDataBits( DataBitsType ) = 0;
    virtual DataBitsType dataBits() const;
    virtual void setParity( ParityType ) = 0;
    virtual ParityType parity() const;
    virtual void setStopBits( StopBitsType ) = 0;
    virtual StopBitsType stopBits() const;
    virtual void setFlowControl( FlowType ) = 0;
    virtual FlowType flowControl() const;
    virtual void setTimeout( ulong, ulong ) = 0;

    virtual bool open( OpenMode mode = 0 ) = 0;
    virtual bool isSequential() const;
    virtual void close() = 0;
    virtual void flush() = 0;

    virtual qint64 size() const = 0;
    virtual qint64 bytesAvailable() = 0;
    virtual bool atEnd() const;

    virtual void ungetChar( char c ) = 0;
    virtual qint64 readLine( char * data, qint64 maxSize );

    virtual ulong lastError() const;
    virtual void translateError( ulong error ) = 0;

    virtual void setDtr( bool set = true ) = 0;
    virtual void setRts( bool set = true ) = 0;
    virtual ulong lineStatus() = 0;

  protected:
    QString port;
    PortSettings Settings;
    ulong lastErr;

#ifdef QT_THREAD_SUPPORT
    static QMutex* mutex;
    static ulong refCount;
#endif

    virtual qint64 readData( char * data, qint64 maxSize ) = 0;
    virtual qint64 writeData( const char * data, qint64 maxSize ) = 0;

};

#endif
