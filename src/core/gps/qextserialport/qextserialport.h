
#ifndef QEXTSERIALPORT_H
#define QEXTSERIALPORT_H


/*if all warning messages are turned off, flag portability warnings to be turned off as well*/
#ifdef _TTY_NOWARN_
#define _TTY_NOWARN_PORT_
#endif

/*macros for warning and debug messages*/
#ifdef _TTY_NOWARN_PORT_
#define TTY_PORTABILITY_WARNING(s)
#else
#define TTY_PORTABILITY_WARNING(s) qWarning(s)
#endif /*_TTY_NOWARN_PORT_*/
#ifdef _TTY_NOWARN_
#define TTY_WARNING(s)
#else
#define TTY_WARNING(s) qWarning(s)
#endif /*_TTY_NOWARN_*/


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

/**
 * structure to contain port settings
 */
struct PortSettings
{
    BaudRateType BaudRate;
    DataBitsType DataBits;
    ParityType Parity;
    StopBitsType StopBits;
    FlowType FlowControl;
    long Timeout_Millisec;
};

#include <QIODevice>
#include <QMutex>
#ifdef Q_OS_UNIX
#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <QSocketNotifier>
typedef int HANDLE; // unused
#elif defined (Q_OS_WIN)
#include <windows.h>
#include <QThread>
#include <QReadWriteLock>
#endif

/*!
Encapsulates a serial port on both POSIX and Windows systems.

\note
Be sure to check the full list of members, as QIODevice provides quite a lot of
functionality for QextSerialPort.

\section Usage
QextSerialPort offers both a polling and event driven API.  Event driven is typically easier
to use, since you never have to worry about checking for new data.

\b Example
\code
QextSerialPort* port = new QextSerialPort("COM1", QextSerialPort::EventDriven);
connect(port, SIGNAL(readyRead()), myClass, SLOT(onDataAvailable()));
port->open();

void MyClass::onDataAvailable() {
    int avail = port->bytesAvailable();
    if( avail > 0 ) {
        QByteArray usbdata;
        usbdata.resize(avail);
        int read = port->read(usbdata.data(), usbdata.size());
        if( read > 0 ) {
            processNewData(usbdata);
        }
    }
}
\endcode

\section Compatibility
The user will be notified of errors and possible portability conflicts at run-time
by default - this behavior can be turned off by defining _TTY_NOWARN_
(to turn off all warnings) or _TTY_NOWARN_PORT_ (to turn off portability warnings) in the project.

On Windows NT/2000/XP this class uses Win32 serial port functions by default.  The user may
select POSIX behavior under NT, 2000, or XP ONLY by defining Q_OS_UNIX in the project.
No guarantees are made as to the quality of POSIX support under NT/2000 however.

\author Stefan Sander, Michal Policht, Brandon Fosdick, Liam Staskawicz
*/

class QWinEventNotifier;

class QextSerialPort: public QIODevice
{
    Q_OBJECT
    public:
        enum QueryMode {
            Polling,
            EventDriven
        };

        explicit QextSerialPort(QueryMode mode = EventDriven);
        QextSerialPort(const QString & name, QueryMode mode = EventDriven);
        QextSerialPort(PortSettings const& s, QueryMode mode = EventDriven);
        QextSerialPort(const QString & name, PortSettings const& s, QueryMode mode = EventDriven);
        ~QextSerialPort();

        void setPortName(const QString & name);
        QString portName() const;

        /**!
         * Get query mode.
         * \return query mode.
         */
        inline QueryMode queryMode() const { return _queryMode; }

        /*!
         * Set desired serial communication handling style. You may choose from polling
         * or event driven approach. This function does nothing when port is open; to
         * apply changes port must be reopened.
         *
         * In event driven approach read() and write() functions are acting
         * asynchronously. They return immediately and the operation is performed in
         * the background, so they doesn't freeze the calling thread.
         * To determine when operation is finished, QextSerialPort runs separate thread
         * and monitors serial port events. Whenever the event occurs, adequate signal
         * is emitted.
         *
         * When polling is set, read() and write() are acting synchronously. Signals are
         * not working in this mode and some functions may not be available. The advantage
         * of polling is that it generates less overhead due to lack of signals emissions
         * and it doesn't start separate thread to monitor events.
         *
         * Generally event driven approach is more capable and friendly, although some
         * applications may need as low overhead as possible and then polling comes.
         *
         * \param mode query mode.
         */
        void setQueryMode(QueryMode mode);

        void setBaudRate(BaudRateType);
        BaudRateType baudRate() const;

        void setDataBits(DataBitsType);
        DataBitsType dataBits() const;

        void setParity(ParityType);
        ParityType parity() const;

        void setStopBits(StopBitsType);
        StopBitsType stopBits() const;

        void setFlowControl(FlowType);
        FlowType flowControl() const;

        void setTimeout(long);

        bool open(OpenMode mode) override;
        bool isSequential() const override;
        void close() override;
        void flush();

        qint64 size() const override;
        qint64 bytesAvailable() const override;
        QByteArray readAll();

        void ungetChar(char c);

        ulong lastError() const;
        void translateError(ulong error);

        void setDtr(bool set=true);
        void setRts(bool set=true);
        ulong lineStatus();
        QString errorString();

#ifdef Q_OS_WIN
        virtual bool waitForReadyRead(int msecs);  ///< @todo implement.
        virtual qint64 bytesToWrite() const;
        static QString fullPortNameWin(const QString & name);
#endif

    protected:
        QMutex* mutex;
        QString port;
        PortSettings Settings;
        ulong lastErr;
        QueryMode _queryMode;

        // platform specific members
#ifdef Q_OS_UNIX
        int fd;
        QSocketNotifier *readNotifier;
        struct termios Posix_CommConfig;
        struct termios old_termios;
        struct timeval Posix_Timeout;
        struct timeval Posix_Copy_Timeout;
#elif (defined Q_OS_WIN)
        HANDLE Win_Handle;
        OVERLAPPED overlap;
        COMMCONFIG Win_CommConfig;
        COMMTIMEOUTS Win_CommTimeouts;
        QWinEventNotifier *winEventNotifier;
        DWORD eventMask;
        QList<OVERLAPPED*> pendingWrites;
        QReadWriteLock* bytesToWriteLock;
        qint64 _bytesToWrite;
#endif

        void construct(); // common construction
        void platformSpecificDestruct();
        void platformSpecificInit();
        qint64 readData(char * data, qint64 maxSize) override;
        qint64 writeData(const char * data, qint64 maxSize) override;

    private slots:
        void onWinEvent(HANDLE h);

    private:
        Q_DISABLE_COPY(QextSerialPort)

    signals:
//        /**
//         * This signal is emitted whenever port settings are updated.
//         * 	\param valid \p true if settings are valid, \p false otherwise.
//         *
//         * 	@todo implement.
//         */
//        // void validSettings(bool valid);

        /*!
         * This signal is emitted whenever dsr line has changed its state. You may
         * use this signal to check if device is connected.
         * 	\param status \p true when DSR signal is on, \p false otherwise.
         *
         * 	\see lineStatus().
         */
        void dsrChanged(bool status);

};

#endif
