
#ifndef _QEXTSERIALPORT_H_
#define _QEXTSERIALPORT_H_

#ifdef WIN32
#include "win_qextserialport.h"
#define QextBaseType Win_QextSerialPort
#else
#include "posix_qextserialport.h"
#define QextBaseType Posix_QextSerialPort
#endif

class CORE_EXPORT QextSerialPort: public QextBaseType
{
  public:
    QextSerialPort();
    QextSerialPort( const QString & name );
    QextSerialPort( PortSettings const& s );
    QextSerialPort( const QString & name, PortSettings const& s );
    QextSerialPort( const QextSerialPort& s );
    QextSerialPort& operator=( const QextSerialPort& );
    virtual ~QextSerialPort();
};

#endif
