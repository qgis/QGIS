/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
** Oracle Spatial support: (C) 2012-2013 Juergen E. Fischer < jef at norbit dot de >, norBIT GmbH
**
****************************************************************************/
// #define QOCISPATIAL_DEBUG
#ifndef QOCISPATIAL_DEBUG
#define QT_NO_DEBUG_OUTPUT
#endif

#if __cplusplus >= 201500
#define FALLTHROUGH [[fallthrough]];
#elif defined(__clang__)
#define FALLTHROUGH [[clang::fallthrough]];
#elif defined(__GNUC__) && __GNUC__ >= 7
#define FALLTHROUGH [[gnu::fallthrough]];
#else
#define FALLTHROUGH
#endif

#include "qsql_ocispatial.h"
#include "wkbptr.h"

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qmetatype.h>
#include <qregularexpression.h>
#include <qshareddata.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <QtSql/private/qsqlcachedresult_p.h>
#include <QtSql/private/qsqldriver_p.h>
#include <qstringlist.h>
#include <qvarlengtharray.h>
#include <qvector.h>
#include <qdebug.h>
#include <cmath>

#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

// This is needed for oracle oci when compiling with mingw-w64 headers
#if defined(__MINGW64_VERSION_MAJOR) && defined(_WIN64)
#define _int64 __int64
#endif


#include <oci.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include <cstdlib>

#define QOCISPATIAL_DYNAMIC_CHUNK_SIZE 65535
#define QOCISPATIAL_PREFETCH_ROWS 10000
#define QOCISPATIAL_PREFETCH_MEM 8388608 // 8MB

// setting this define will allow using a query from a different
// thread than its database connection.
// warning - this is not fully tested and can lead to race conditions
#define QOCISPATIAL_THREADED


Q_DECLARE_OPAQUE_POINTER( OCIEnv * )
Q_DECLARE_OPAQUE_POINTER( OCIStmt * )
Q_DECLARE_METATYPE( OCIEnv * )
Q_DECLARE_METATYPE( OCIStmt * )

QT_BEGIN_NAMESPACE

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
enum { QOCISpatialEncoding = 2002 }; // AL16UTF16LE
#else
enum { QOCISpatialEncoding = 2000 }; // AL16UTF16
#endif

#ifdef OCI_ATTR_CHARSET_FORM
// Always set the OCI_ATTR_CHARSET_FORM to SQLCS_NCHAR is safe
// because Oracle server will deal with the implicit Conversion
// Between CHAR and NCHAR.
// see: http://download.oracle.com/docs/cd/A91202_01/901_doc/appdev.901/a89857/oci05bnd.htm#422705
static const ub1 qOraCharsetForm = SQLCS_NCHAR;
#endif

#if defined (OCI_UTF16ID)
static const ub2 qOraCharset = OCI_UTF16ID;
#else
static const ub2 qOraCharset = OCI_UCS2ID;
#endif

typedef QVarLengthArray<sb2, 32> IndicatorArray;
typedef QVarLengthArray<ub2, 32> SizeArray;

static QByteArray qMakeOraDate( const QDateTime &dt );
static QDateTime qMakeDate( const char *oraDate );

static QByteArray qMakeOCINumber( const qlonglong &ll, OCIError *err );
static QByteArray qMakeOCINumber( const qulonglong &ull, OCIError *err );

static qlonglong qMakeLongLong( const char *ociNumber, OCIError *err );
static qulonglong qMakeULongLong( const char *ociNumber, OCIError *err );

static QString qOraWarn( OCIError *err, int *errorCode = nullptr );

#ifndef Q_CC_SUN
static // for some reason, Sun CC can't use qOraWarning when it's declared static
#endif
void qOraWarningAt( const char *msg, OCIError *err, const char *function, const char *file, int line );
static QSqlError qMakeError( const QString &errString, QSqlError::ErrorType type, OCIError *err );

#ifndef _MSC_VER
#define qOraWarning(msg,err) qOraWarningAt(msg,err,__PRETTY_FUNCTION__,__FILE__,__LINE__)
#define OCI_VERIFY(x) do { oci_verify(__PRETTY_FUNCTION__, __FILE__, __LINE__, x, #x); } while(0)
#define OCI_VERIFY_E(e,x) do { oci_verify(e,__PRETTY_FUNCTION__, __FILE__, __LINE__, x, #x); } while(0)
#else
#define qOraWarning(msg,err) qOraWarningAt(msg,err,__FUNCTION__,__FILE__,__LINE__)
#define OCI_VERIFY(x) do { oci_verify(__FUNCTION__, __FILE__, __LINE__, x, #x); } while(0)
#define OCI_VERIFY_E(e,x) do { oci_verify(e,__FUNCTION__, __FILE__, __LINE__, x, #x); } while(0)
#endif

void oci_verify( OCIError *err, const char *function, const char *file, int line, int result, const char *expression )
{
  if ( result == OCI_SUCCESS || result == OCI_SUCCESS_WITH_INFO )
    return;

  qWarning( "%s:%d (%s) OCI error %s = %d [%s]", file, line, function, expression, result, qPrintable( qOraWarn( err ) ) );
  throw result;
}

void oci_verify( const char *function, const char *file, int line, int result, const char *expression )
{
  if ( result == OCI_SUCCESS || result == OCI_SUCCESS_WITH_INFO )
    return;

  qWarning( "%s:%d (%s) OCI error %s = %d", file, line, function, expression, result );
  throw result;
}

#ifdef QOCISPATIAL_DEBUG
class enter
{
    const char *mFunction = nullptr;
    const char *mFile = nullptr;
    int mLine;
    static int level;

  public:
    enter( const char *function, const char *file, int line )
      : mFunction( function ), mFile( file ), mLine( line )
    {
      qDebug( "+%*sEntering %s at %s:%d", level, "", mFunction, mFile, mLine );
      level++;
    }

    ~enter()
    {
      level--;
      qDebug( "-%*sLeaving %s", level, "", mFunction );
    }
};

#ifdef _MSC_VER
#define ENTER enter here(__FUNCTION__,__FILE__,__LINE__);
#else
#define ENTER enter here(__PRETTY_FUNCTION__,__FILE__,__LINE__);
#endif

int enter::level = 0;
#else
#define ENTER
#endif

enum WKBType
{
  WKBUnknown = 0,
  WKBPoint = 1,
  WKBLineString,
  WKBPolygon,
  WKBMultiPoint,
  WKBMultiLineString,
  WKBMultiPolygon,

  WKBCircularString = 8,
  WKBCompoundCurve = 9,
  WKBCurvePolygon = 10,
  WKBMultiCurve = 11,
  WKBMultiSurface = 12,

  WKBNoGeometry = 100, //attributes only
  WKBCircularStringZ = 1008,
  WKBCompoundCurveZ = 1009,
  WKBCurvePolygonZ = 1010,
  WKBMultiCurveZ = 1011,
  WKBMultiSurfaceZ = 1012,

  WKBPoint25D = 0x80000001,
  WKBLineString25D,
  WKBPolygon25D,
  WKBMultiPoint25D,
  WKBMultiLineString25D,
  WKBMultiPolygon25D,

#if 0
  WKBGeometry = 0,
  WKBPoint = 1,
  WKBLineString = 2,
  WKBPolygon = 3,
  WKBMultiPoint = 4,
  WKBMultiLineString = 5,
  WKBMultiPolygon = 6,
  WKBGeometryCollection = 7,
  WKBPolyhedralSurface = 15,
  WKBTIN = 16,
  WKBTriangle = 17,

  WKBGeometryZ = 1000,
  WKBPointZ = 1001,
  WKBLineStringZ = 1002,
  WKBPolygonZ = 1003,
  WKBMultiPointZ = 1004,
  WKBMultiLineStringZ = 1005,
  WKBMultiPolygonZ = 1006,
  WKBGeometryCollectionZ = 1007,
  WKBPolyhedralSurfaceZ = 1015,
  WKBTINZ = 1016,
  WKBTriangleZ = 1017,

  WKBGeometryM = 2000,
  WKBPointM = 2001,
  WKBLineStringM = 2002,
  WKBPolygonM = 2003,
  WKBMultiPointM = 2004,
  WKBMultiLineStringM = 2005,
  WKBMultiPolygonM = 2006,
  WKBGeometryCollectionM = 2007,
  WKBPolyhedralSurfaceM = 2015,
  WKBTINM = 2016,
  WKBTriangleM = 2017,

  WKBGeometryZM = 3000,
  WKBPointZM = 3001,
  WKBLineStringZM = 3002,
  WKBPolygonZM = 3003,
  WKBMultiPointZM = 3004,
  WKBMultiLineStringZM = 3005,
  WKBMultiPolygonZM = 3006,
  WKBGeometryCollectionZM = 3007,
  WKBPolyhedralSurfaceZM = 3015,
  WKBTINZM = 3016,
  WKBTriangleZM = 3017,
#endif
};


class QOCISpatialRowId: public QSharedData
{
  public:
    explicit QOCISpatialRowId( OCIEnv *env );
    ~QOCISpatialRowId();

    OCIRowid *id = nullptr;

  private:
    QOCISpatialRowId( const QOCISpatialRowId &other ): QSharedData( other ), id( nullptr ) { Q_ASSERT( false ); }
    QOCISpatialRowId &operator= ( const QOCISpatialRowId & ) = delete;
};

QOCISpatialRowId::QOCISpatialRowId( OCIEnv *env )
{
  OCIDescriptorAlloc( env, reinterpret_cast<dvoid **>( &id ),
                      OCI_DTYPE_ROWID, 0, nullptr );
}

QOCISpatialRowId::~QOCISpatialRowId()
{
  if ( id )
    OCIDescriptorFree( id, OCI_DTYPE_ROWID );
}

typedef QSharedDataPointer<QOCISpatialRowId> QOCISpatialRowIdPointer;
QT_BEGIN_INCLUDE_NAMESPACE
Q_DECLARE_METATYPE( QOCISpatialRowIdPointer )
QT_END_INCLUDE_NAMESPACE

class QOCISpatialDriverPrivate : public QSqlDriverPrivate
{
    Q_DECLARE_PUBLIC( QOCISpatialDriver )

  public:
    QOCISpatialDriverPrivate();

    OCIEnv *env = nullptr;
    OCISvcCtx *svc = nullptr;
    OCIServer *srvhp = nullptr;
    OCISession *authp = nullptr;
    OCIError *err = nullptr;
    bool transaction = false;
    int serverVersion = -1;
    ub4 prefetchRows = QOCISPATIAL_PREFETCH_ROWS;
    ub4 prefetchMem = QOCISPATIAL_PREFETCH_MEM;
    bool commitOnSuccess = true;
    QString user;

    OCIType *geometryTDO = nullptr;

    void allocErrorHandle();
    OCIType *tdo( QString type );
};

class QOCISpatialCols;
class QOCISpatialResultPrivate;

class QOCISpatialResult: public QSqlCachedResult
{
    Q_DECLARE_PRIVATE( QOCISpatialResult )
    friend class QOCISpatialDriver;
    friend class QOCISpatialCols;
  public:
    QOCISpatialResult( const QOCISpatialDriver *db );
    ~QOCISpatialResult() override;
    bool prepare( const QString &query ) override;
    bool exec() override;
    QVariant handle() const override;

  protected:
    bool gotoNext( ValueCache &values, int index ) override;
    bool reset( const QString &query ) override;
    int size() override;
    int numRowsAffected() override;
    QSqlRecord record() const override;
    QVariant lastInsertId() const override;
    bool execBatch( bool arrayBind = false ) override;
    void virtual_hook( int id, void *data ) override;
};

struct QOCISDOPointObj
{
  OCINumber x;
  OCINumber y;
  OCINumber z;
};

struct QOCISDOGeometryObj
{
  OCINumber gtype;
  OCINumber srid;
  QOCISDOPointObj point;
  OCIArray *elem_info = nullptr;
  OCIArray *ordinates = nullptr;
};

struct QOCISDOPointInd
{
  OCIInd _atomic;
  OCIInd x;
  OCIInd y;
  OCIInd z;
};

struct QOCISDOGeometryInd
{
  OCIInd _atomic;
  OCIInd gtype;
  OCIInd srid;
  QOCISDOPointInd point;
  OCIInd elem_info;
  OCIInd ordinates;
};


class QOCISpatialResultPrivate: public QSqlCachedResultPrivate
{
  public:
    Q_DECLARE_PUBLIC( QOCISpatialResult )
    Q_DECLARE_SQLDRIVER_PRIVATE( QOCISpatialDriver )
    QOCISpatialResultPrivate( QOCISpatialResult *q, const QOCISpatialDriver *drv );
    ~QOCISpatialResultPrivate() override;

    QOCISpatialCols *cols = nullptr;
    OCIEnv *env = nullptr;
    OCIError *err = nullptr;
    OCISvcCtx *&svc;
    OCIStmt *sql = nullptr;
    QList<QOCISDOGeometryObj *> sdoobj;
    QList<QOCISDOGeometryInd *> sdoind;
    bool transaction;
    bool commitOnSuccess = true;
    int serverVersion;
    ub4 prefetchRows, prefetchMem;
    OCIType *geometryTDO = nullptr;
    QOCISDOGeometryObj *geometryObj = nullptr;
    QOCISDOGeometryInd *geometryInd = nullptr;

    void setStatementAttributes();
    int bindValue( OCIStmt *sql, OCIBind **hbnd, OCIError *err, int pos,
                   const QVariant &val, dvoid *indPtr, ub2 *tmpSize, QList<QByteArray> &tmpStorage );
    int bindValues( QVector<QVariant> &values, IndicatorArray &indicators, SizeArray &tmpSizes,
                    QList<QByteArray> &tmpStorage );
    void outValues( QVector<QVariant> &values, IndicatorArray &indicators,
                    QList<QByteArray> &tmpStorage );
    inline bool isOutValue( int i ) const
    { Q_Q( const QOCISpatialResult ); return q->bindValueType( i ) & QSql::Out; }
    inline bool isBinaryValue( int i ) const
    { Q_Q( const QOCISpatialResult ); return q->bindValueType( i ) & QSql::Binary; }

    void setCharset( dvoid *handle, ub4 type ) const
    {
      int r = OCI_SUCCESS;
      Q_ASSERT( handle );

#ifdef OCI_ATTR_CHARSET_FORM
      r = OCIAttrSet( handle,
                      type,
                      // this const cast is safe since OCI doesn't touch
                      // the charset.
                      const_cast<void *>( static_cast<const void *>( &qOraCharsetForm ) ),
                      0,
                      OCI_ATTR_CHARSET_FORM,
                      //Strange Oracle bug: some Oracle servers crash the server process with non-zero error handle (mostly for 10g).
                      //So ignore the error message here.
                      nullptr );
#ifdef QOCISPATIAL_DEBUG
      if ( r != OCI_SUCCESS )
        qWarning( "QOCISpatialResultPrivate::setCharset: Couldn't set OCI_ATTR_CHARSET_FORM." );
#endif
#endif

      r = OCIAttrSet( handle,
                      type,
                      // this const cast is safe since OCI doesn't touch
                      // the charset.
                      const_cast<void *>( static_cast<const void *>( &qOraCharset ) ),
                      0,
                      OCI_ATTR_CHARSET_ID,
                      err );
      if ( r != OCI_SUCCESS )
        qOraWarning( "Couldn't set OCI_ATTR_CHARSET_ID: ", err );

    }
};

void QOCISpatialResultPrivate::setStatementAttributes()
{
  ENTER
  Q_ASSERT( sql );

  int r = OCI_SUCCESS;

  r = OCIAttrSet( sql,
                  OCI_HTYPE_STMT,
                  &prefetchRows,
                  0,
                  OCI_ATTR_PREFETCH_ROWS,
                  err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "Couldn't set OCI_ATTR_PREFETCH_ROWS: ", err );
  r = OCIAttrSet( sql,
                  OCI_HTYPE_STMT,
                  &prefetchMem,
                  0,
                  OCI_ATTR_PREFETCH_MEMORY,
                  err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "QOCISpatialResultPrivate::setStatementAttributes:"
                 " Couldn't set OCI_ATTR_PREFETCH_MEMORY: ", err );
}

int QOCISpatialResultPrivate::bindValue( OCIStmt *sql, OCIBind **hbnd, OCIError *err, int pos,
    const QVariant &val, dvoid *indPtr, ub2 *tmpSize, QList<QByteArray> &tmpStorage )
{
  ENTER
  int r = OCI_SUCCESS;
  void *data = const_cast<void *>( val.constData() );

  switch ( val.userType() )
  {
    case QMetaType::Type::QByteArray:
      r = OCIBindByPos( sql, hbnd, err,
                        pos + 1,
                        isOutValue( pos )
                        ? const_cast<char *>( reinterpret_cast<QByteArray *>( data )->constData() )
                        : reinterpret_cast<QByteArray *>( data )->data(),
                        reinterpret_cast<QByteArray *>( data )->size(),
                        SQLT_BIN, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
      qDebug() << "inout" << isOutValue( pos ) << "bytearray size" << reinterpret_cast<QByteArray *>( data )->size() << "r" << r;
      break;
    case QMetaType::Type::QTime:
    case QMetaType::Type::QDate:
    case QMetaType::Type::QDateTime:
    {
      QByteArray ba = qMakeOraDate( val.toDateTime() );
      r = OCIBindByPos( sql, hbnd, err,
                        pos + 1,
                        ba.data(),
                        ba.size(),
                        SQLT_DAT, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
      tmpStorage.append( ba );
      break;
    }
    case QMetaType::Type::Int:
      r = OCIBindByPos( sql, hbnd, err,
                        pos + 1,
                        // if it's an out value, the data is already detached
                        // so the const cast is safe.
                        const_cast<void *>( data ),
                        sizeof( int ),
                        SQLT_INT, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
      break;
    case QMetaType::Type::UInt:
      r = OCIBindByPos( sql, hbnd, err,
                        pos + 1,
                        // if it's an out value, the data is already detached
                        // so the const cast is safe.
                        const_cast<void *>( data ),
                        sizeof( uint ),
                        SQLT_UIN, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
      break;
    case QMetaType::Type::LongLong:
    {
      QByteArray ba = qMakeOCINumber( val.toLongLong(), err );
      r = OCIBindByPos( sql, hbnd, err,
                        pos + 1,
                        ba.data(),
                        ba.size(),
                        SQLT_VNU, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
      tmpStorage.append( ba );
      break;
    }
    case QMetaType::Type::ULongLong:
    {
      QByteArray ba = qMakeOCINumber( val.toULongLong(), err );
      r = OCIBindByPos( sql, hbnd, err,
                        pos + 1,
                        ba.data(),
                        ba.size(),
                        SQLT_VNU, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
      tmpStorage.append( ba );
      break;
    }
    case QMetaType::Type::Double:
      r = OCIBindByPos( sql, hbnd, err,
                        pos + 1,
                        // if it's an out value, the data is already detached
                        // so the const cast is safe.
                        const_cast<void *>( data ),
                        sizeof( double ),
                        SQLT_FLT, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
      break;
    case QMetaType::Type::QString:
    {
      const QString s = val.toString();
      if ( isBinaryValue( pos ) )
      {
        r = OCIBindByPos( sql, hbnd, err,
                          pos + 1,
                          const_cast<ushort *>( s.utf16() ),
                          static_cast<sb4>( s.length() * sizeof( QChar ) ),
                          SQLT_LNG, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
        break;
      }
      else if ( !isOutValue( pos ) )
      {
        // don't detach the string
        r = OCIBindByPos( sql, hbnd, err,
                          pos + 1,
                          // safe since oracle doesn't touch OUT values
                          const_cast<ushort *>( s.utf16() ),
                          static_cast<sb4>( ( s.length() + 1 ) * sizeof( QChar ) ),
                          SQLT_STR, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
        if ( r == OCI_SUCCESS )
          setCharset( *hbnd, OCI_HTYPE_BIND );
        break;
      }
    }
    FALLTHROUGH

    default:
    {
      if ( val.canConvert<QOCISpatialGeometry>() && !isOutValue( pos ) )
      {
        try
        {
          if ( !geometryObj )
          {
            OCI_VERIFY_E( err, OCIObjectNew( env, err, svc, OCI_TYPECODE_OBJECT, geometryTDO, ( dvoid * ) nullptr, OCI_DURATION_SESSION, 1, ( dvoid ** ) &geometryObj ) );
            if ( !geometryObj )
            {
              throw OCI_ERROR;
            }

            OCI_VERIFY_E( err, OCIObjectGetInd( env, err, geometryObj, ( void ** ) &geometryInd ) );
            if ( !geometryInd )
            {
              throw OCI_ERROR;
            }
          }

          OCI_VERIFY_E( err, OCIBindByPos( sql, hbnd, err, pos + 1, nullptr, 0, SQLT_NTY, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT ) );
          OCI_VERIFY_E( err, OCIBindObject( *hbnd, err, geometryTDO, ( dvoid ** )&geometryObj, nullptr, ( dvoid ** ) &geometryInd, nullptr ) );

          const QOCISpatialGeometry &g = qvariant_cast<QOCISpatialGeometry>( val );

          int n;
          OCI_VERIFY_E( err, OCICollSize( env, err, geometryObj->elem_info, &n ) );
          OCI_VERIFY_E( err, OCICollTrim( env, err, n, geometryObj->elem_info ) );

          OCI_VERIFY_E( err, OCICollSize( env, err, geometryObj->ordinates, &n ) );
          OCI_VERIFY_E( err, OCICollTrim( env, err, n, geometryObj->ordinates ) );

          if ( g.isNull )
          {
            geometryInd->_atomic = OCI_IND_NULL;
          }
          else
          {
            geometryInd->_atomic = OCI_IND_NOTNULL;
            geometryInd->gtype   = g.gtype < 0 ? OCI_IND_NULL : OCI_IND_NOTNULL;
            geometryInd->srid    = g.srid  < 0 ? OCI_IND_NULL : OCI_IND_NOTNULL;

            OCI_VERIFY_E( err, OCINumberFromInt( err, &g.gtype, sizeof( int ), OCI_NUMBER_SIGNED, &geometryObj->gtype ) );
            OCI_VERIFY_E( err, OCINumberFromInt( err, &g.srid, sizeof( int ), OCI_NUMBER_SIGNED, &geometryObj->srid ) );

            if ( SDO_GTYPE_TT( g.gtype ) == GtPoint )
            {
              geometryInd->point._atomic = OCI_IND_NOTNULL;
              geometryInd->point.x       = OCI_IND_NOTNULL;
              geometryInd->point.y       = OCI_IND_NOTNULL;
              geometryInd->point.z       = OCI_IND_NOTNULL;
              geometryInd->elem_info     = OCI_IND_NULL;
              geometryInd->ordinates     = OCI_IND_NULL;

              OCI_VERIFY_E( err, OCINumberFromReal( err, &g.x, sizeof( double ), &geometryObj->point.x ) );
              OCI_VERIFY_E( err, OCINumberFromReal( err, &g.y, sizeof( double ), &geometryObj->point.y ) );
              OCI_VERIFY_E( err, OCINumberFromReal( err, &g.z, sizeof( double ), &geometryObj->point.z ) );
            }
            else
            {
              geometryInd->point._atomic = OCI_IND_NULL;
              geometryInd->elem_info = g.eleminfo.size() == 0  ? OCI_IND_NULL : OCI_IND_NOTNULL;
              geometryInd->ordinates = g.ordinates.size() == 0 ? OCI_IND_NULL : OCI_IND_NOTNULL;

              for ( int e : g.eleminfo )
              {
                OCINumber n;
                OCI_VERIFY_E( err, OCINumberFromInt( err, &e, sizeof( int ), OCI_NUMBER_SIGNED, &n ) );
                OCI_VERIFY_E( err, OCICollAppend( env, err, &n, nullptr, geometryObj->elem_info ) );
              }

              for ( double o : g.ordinates )
              {
                OCINumber n;
                OCI_VERIFY_E( err, OCINumberFromReal( err, &o, sizeof( double ), &n ) );
                OCI_VERIFY_E( err, OCICollAppend( env, err, &n, nullptr, geometryObj->ordinates ) );
              }
            }
          }
        }
        catch ( int e )
        {
          r = e;
        }
      }
      else if ( val.canConvert<QOCISpatialRowIdPointer>() && !isOutValue( pos ) )
      {
        // use a const pointer to prevent a detach
        const QOCISpatialRowIdPointer rptr = qvariant_cast<QOCISpatialRowIdPointer>( val );
        r = OCIBindByPos( sql, hbnd, err,
                          pos + 1,
                          // it's an IN value, so const_cast is OK
                          const_cast<OCIRowid **>( &rptr->id ),
                          -1,
                          SQLT_RDD, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
      }
      else if ( val.userType() >= QMetaType::Type::User )
      {
        qWarning( "Unknown bind variable" );
        r = OCI_ERROR;
      }
      else
      {
        const QString s = val.toString();
        // create a deep-copy
        QByteArray ba( reinterpret_cast<const char *>( s.utf16() ), static_cast<int>( ( s.length() + 1 ) * sizeof( QChar ) ) );
        if ( isOutValue( pos ) )
        {
          ba.reserve( static_cast<int>( ( s.capacity() + 1 ) * sizeof( QChar ) ) );
          *tmpSize = ba.size();
          r = OCIBindByPos( sql, hbnd, err,
                            pos + 1,
                            ba.data(),
                            ba.capacity(),
                            SQLT_STR, indPtr, tmpSize, nullptr, 0, nullptr, OCI_DEFAULT );
          tmpStorage.append( ba );
        }
        else
        {
          r = OCIBindByPos( sql, hbnd, err,
                            pos + 1,
                            ba.data(),
                            ba.size(),
                            SQLT_STR, indPtr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT );
        }
        if ( r == OCI_SUCCESS )
          setCharset( *hbnd, OCI_HTYPE_BIND );
        break;
      }
    } // default case
  } // switch
  if ( r != OCI_SUCCESS )
    qOraWarning( "QOCISpatialResultPrivate::bindValue:", err );
  return r;
}

int QOCISpatialResultPrivate::bindValues( QVector<QVariant> &values, IndicatorArray &indicators,
    SizeArray &tmpSizes, QList<QByteArray> &tmpStorage )
{
  ENTER
  int r = OCI_SUCCESS;
  for ( int i = 0; i < values.count(); ++i )
  {
    if ( isOutValue( i ) )
      values[i].detach();
    const QVariant &val = values.at( i );

    OCIBind *hbnd = nullptr;  // Oracle handles these automatically
    sb2 *indPtr = &indicators[i];
    *indPtr = val.isNull() ? -1 : 0;

    bindValue( sql, &hbnd, err, i, val, indPtr, &tmpSizes[i], tmpStorage );
  }
  return r;
}

// will assign out value and remove its temp storage.
static void qOraOutValue( QVariant &value, QList<QByteArray> &storage, OCIError *err )
{
  ENTER
  switch ( value.userType() )
  {
    case QMetaType::Type::QTime:
      value = qMakeDate( storage.takeFirst() ).time();
      break;
    case QMetaType::Type::QDate:
      value = qMakeDate( storage.takeFirst() ).date();
      break;
    case QMetaType::Type::QDateTime:
      value = qMakeDate( storage.takeFirst() );
      break;
    case QMetaType::Type::LongLong:
      value = qMakeLongLong( storage.takeFirst(), err );
      break;
    case QMetaType::Type::ULongLong:
      value = qMakeULongLong( storage.takeFirst(), err );
      break;
    case QMetaType::Type::QString:
      value = QString(
                reinterpret_cast<const QChar *>( storage.takeFirst().constData() ) );
      break;
    default:
      break; //nothing
  }
}

void QOCISpatialResultPrivate::outValues( QVector<QVariant> &values, IndicatorArray &indicators,
    QList<QByteArray> &tmpStorage )
{
  ENTER
  for ( int i = 0; i < values.count(); ++i )
  {

    if ( !isOutValue( i ) )
      continue;

    qOraOutValue( values[i], tmpStorage, err );

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMetaType::Type typ = static_cast<QMetaType::Type>( values.at( i ).userType() );
#else
    QMetaType typ = values.at( i ).metaType();
#endif
    if ( indicators[i] == -1 ) // NULL
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      values[i] = static_cast< QVariant::Type >( typ );
#else
      values[i] = QVariant( typ );
#endif
  }
}


QOCISpatialDriverPrivate::QOCISpatialDriverPrivate()
{
  ENTER
}

void QOCISpatialDriverPrivate::allocErrorHandle()
{
  ENTER
  int r = OCIHandleAlloc( env,
                          reinterpret_cast<void **>( &err ),
                          OCI_HTYPE_ERROR,
                          0,
                          nullptr );
  if ( r != OCI_SUCCESS )
    qWarning( "QOCISpatialDriver: unable to allocate error handle" );
}

OCIType *QOCISpatialDriverPrivate::tdo( QString type )
{
  OCIParam *paramp = nullptr;
  OCIRef *type_ref = nullptr;
  OCIType *tdo = nullptr;
  OCIDescribe *dschp = nullptr;

  try
  {
    OCI_VERIFY( OCIHandleAlloc( env, ( void ** ) & dschp, OCI_HTYPE_DESCRIBE, 0, nullptr ) );
    OCI_VERIFY_E( err, OCIDescribeAny( svc, err, ( dvoid * ) type.utf16(), type.length() * sizeof( QChar ), OCI_OTYPE_NAME, OCI_DEFAULT, OCI_PTYPE_TYPE, dschp ) );
    OCI_VERIFY_E( err, OCIAttrGet( dschp, OCI_HTYPE_DESCRIBE, &paramp, nullptr, OCI_ATTR_PARAM, err ) );
    OCI_VERIFY_E( err, OCIAttrGet( paramp, OCI_DTYPE_PARAM, &type_ref, nullptr, OCI_ATTR_REF_TDO, err ) );
    OCI_VERIFY_E( err, OCIObjectPin( env, err, type_ref, nullptr, OCI_PIN_ANY, OCI_DURATION_SESSION, OCI_LOCK_NONE, ( dvoid ** ) &tdo ) );
  }
  catch ( int r )
  {
    Q_UNUSED( r )
    return nullptr;
  }

  return tdo;
}

struct OraFieldInfo
{
  QString name;
  QMetaType::Type type;
  ub1 oraIsNull;
  ub4 oraType;
  sb1 oraScale;
  ub4 oraLength; // size in bytes
  ub4 oraFieldLength; // amount of characters
  sb2 oraPrecision;
  QString oraTypeName;
  OCIType *oraOCIType = nullptr;
};

QString qOraWarn( OCIError *err, int *errorCode )
{
  sb4 errcode;
  text errbuf[1024];
  errbuf[0] = 0;
  errbuf[1] = 0;

  OCIErrorGet( err,
               1,
               nullptr,
               &errcode,
               errbuf,
               sizeof( errbuf ),
               OCI_HTYPE_ERROR );
  if ( errorCode )
    *errorCode = errcode;
  return QString( reinterpret_cast<const QChar *>( errbuf ) );
}

void qOraWarningAt( const char *msg, OCIError *err, const char *function, const char *file, int line )
{
  qWarning( "%s: %d: (%s) %s [%s]", file, line, function, msg, qPrintable( qOraWarn( err ) ) );
}

static int qOraErrorNumber( OCIError *err )
{
  ENTER
  sb4 errcode;
  OCIErrorGet( err,
               1,
               nullptr,
               &errcode,
               nullptr,
               0,
               OCI_HTYPE_ERROR );
  return errcode;
}

QSqlError qMakeError( const QString &errString, QSqlError::ErrorType type, OCIError *err )
{
  ENTER
  int errorCode = 0;
  const QString oraErrorString = qOraWarn( err, &errorCode );
  return QSqlError( errString, oraErrorString, type, QString::number( errorCode ) );
}

QMetaType::Type qDecodeOCIType( const QString &ocitype, QSql::NumericalPrecisionPolicy precisionPolicy )
{
  ENTER
  QMetaType::Type type = QMetaType::Type::UnknownType;
  qDebug( "qDecodeOCIType(ocitype=%s, precisionPolicy=%d)\n", ocitype.toLocal8Bit().constData(), precisionPolicy );
  if ( ocitype == QLatin1String( "VARCHAR2" ) || ocitype == QLatin1String( "VARCHAR" )
       || ocitype.startsWith( QLatin1String( "INTERVAL" ) )
       || ocitype == QLatin1String( "CHAR" ) || ocitype == QLatin1String( "NVARCHAR2" )
       || ocitype == QLatin1String( "NCHAR" ) )
    type = QMetaType::Type::QString;
  else if ( ocitype == QLatin1String( "NUMBER" )
            || ocitype == QLatin1String( "FLOAT" )
            || ocitype == QLatin1String( "BINARY_FLOAT" )
            || ocitype == QLatin1String( "BINARY_DOUBLE" ) )
  {
    switch ( precisionPolicy )
    {
      case QSql::LowPrecisionInt32:
        type = QMetaType::Type::Int;
        break;
      case QSql::LowPrecisionInt64:
        type = QMetaType::Type::LongLong;
        break;
      case QSql::LowPrecisionDouble:
        type = QMetaType::Type::Double;
        break;
      case QSql::HighPrecision:
      default:
        type = QMetaType::Type::QString;
        break;
    }
  }
  else if ( ocitype == QLatin1String( "LONG" ) || ocitype == QLatin1String( "NCLOB" )
            || ocitype == QLatin1String( "CLOB" ) || ocitype == QLatin1String( "RAW" ) || ocitype == QLatin1String( "LONG RAW" )
            || ocitype == QLatin1String( "ROWID" ) || ocitype == QLatin1String( "BLOB" )
            || ocitype == QLatin1String( "CFILE" ) || ocitype == QLatin1String( "BFILE" ) )
    type = QMetaType::Type::QByteArray;
  else if ( ocitype == QLatin1String( "DATE" ) ||  ocitype.startsWith( QLatin1String( "TIME" ) ) )
    type = QMetaType::Type::QDateTime;
  else if ( ocitype == QLatin1String( "UNDEFINED" ) )
    type = QMetaType::Type::UnknownType;
  if ( type == QMetaType::Type::UnknownType )
    qWarning( "qDecodeOCIType: unknown type: %s", ocitype.toLocal8Bit().constData() );
  return type;
}

QMetaType::Type qDecodeOCIType( int ocitype, QSql::NumericalPrecisionPolicy precisionPolicy )
{
  ENTER
  QMetaType::Type type = QMetaType::Type::UnknownType;
  qDebug( "qDecodeOCIType(ocitype=%d, precisionPolicy=%d)\n", ocitype, precisionPolicy );
  switch ( ocitype )
  {
    case SQLT_STR:
    case SQLT_VST:
    case SQLT_CHR:
    case SQLT_AFC:
    case SQLT_VCS:
    case SQLT_AVC:
    case SQLT_RDD:
    case SQLT_LNG:
#ifdef SQLT_INTERVAL_YM
    case SQLT_INTERVAL_YM:
#endif
#ifdef SQLT_INTERVAL_DS
    case SQLT_INTERVAL_DS:
#endif
      type = QMetaType::Type::QString;
      break;
    case SQLT_INT:
      type = QMetaType::Type::Int;
      break;
    case SQLT_FLT:
    case SQLT_NUM:
    case SQLT_VNU:
    case SQLT_UIN:
    case SQLT_IBDOUBLE:
    case SQLT_IBFLOAT:
      switch ( precisionPolicy )
      {
        case QSql::LowPrecisionInt32:
          type = QMetaType::Type::Int;
          break;
        case QSql::LowPrecisionInt64:
          type = QMetaType::Type::LongLong;
          break;
        case QSql::LowPrecisionDouble:
          type = QMetaType::Type::Double;
          break;
        case QSql::HighPrecision:
        default:
          type = QMetaType::Type::QString;
          break;
      }
      break;
    case SQLT_VBI:
    case SQLT_BIN:
    case SQLT_LBI:
    case SQLT_LVC:
    case SQLT_LVB:
    case SQLT_BLOB:
    case SQLT_CLOB:
    case SQLT_FILE:
    case SQLT_NTY:
    case SQLT_REF:
    case SQLT_RID:
      type = QMetaType::Type::QByteArray;
      break;
    case SQLT_DAT:
    case SQLT_ODT:
#ifdef SQLT_TIMESTAMP
    case SQLT_TIMESTAMP:
    case SQLT_TIMESTAMP_TZ:
    case SQLT_TIMESTAMP_LTZ:
#endif
      type = QMetaType::Type::QDateTime;
      break;
    default:
      type = QMetaType::Type::UnknownType;
      qWarning( "qDecodeOCIType: unknown OCI datatype: %d", ocitype );
      break;
  }
  return type;
}

static QSqlField qFromOraInf( const OraFieldInfo &ofi )
{
  ENTER
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QSqlField f( ofi.name, static_cast<QVariant::Type>( ofi.type ) );
#else
  QSqlField f( ofi.name, QMetaType( ofi.type ) );
#endif

  f.setRequired( ofi.oraIsNull == 0 );

  if ( ofi.type == QMetaType::Type::QString && ofi.oraType != SQLT_NUM && ofi.oraType != SQLT_VNU )
    f.setLength( ofi.oraFieldLength );
  else
    f.setLength( ofi.oraPrecision == 0 ? 38 : int( ofi.oraPrecision ) );

  f.setPrecision( ofi.oraScale );
  f.setSqlType( int( ofi.oraType ) );
  return f;
}

/*!
    \internal

    Convert QDateTime to the internal Oracle DATE format NB!
    It does not handle BCE dates.
*/
QByteArray qMakeOraDate( const QDateTime &dt )
{
  ENTER
  QByteArray ba;
  ba.resize( 7 );
  int year = dt.date().year();
  ba[0] = ( year / 100 ) + 100; // century
  ba[1] = ( year % 100 ) + 100; // year
  ba[2] = dt.date().month();
  ba[3] = dt.date().day();
  ba[4] = dt.time().hour() + 1;
  ba[5] = dt.time().minute() + 1;
  ba[6] = dt.time().second() + 1;
  return ba;
}

/*!
  \internal

   Convert qlonglong to the internal Oracle OCINumber format.
  */
QByteArray qMakeOCINumber( const qlonglong &ll, OCIError *err )
{
  ENTER
  QByteArray ba( sizeof( OCINumber ), 0 );

  OCINumberFromInt( err,
                    &ll,
                    sizeof( qlonglong ),
                    OCI_NUMBER_SIGNED,
                    reinterpret_cast<OCINumber *>( ba.data() ) );
  return ba;
}

/*!
  \internal

   Convert qulonglong to the internal Oracle OCINumber format.
  */
QByteArray qMakeOCINumber( const qulonglong &ull, OCIError *err )
{
  ENTER
  QByteArray ba( sizeof( OCINumber ), 0 );

  OCINumberFromInt( err,
                    &ull,
                    sizeof( qlonglong ),
                    OCI_NUMBER_UNSIGNED,
                    reinterpret_cast<OCINumber *>( ba.data() ) );
  return ba;
}

qlonglong qMakeLongLong( const char *ociNumber, OCIError *err )
{
  ENTER
  qlonglong qll = 0;
  OCINumberToInt( err, reinterpret_cast<const OCINumber *>( ociNumber ), sizeof( qlonglong ),
                  OCI_NUMBER_SIGNED, &qll );
  return qll;
}

qulonglong qMakeULongLong( const char *ociNumber, OCIError *err )
{
  ENTER
  qulonglong qull = 0;
  OCINumberToInt( err, reinterpret_cast<const OCINumber *>( ociNumber ), sizeof( qulonglong ),
                  OCI_NUMBER_UNSIGNED, &qull );
  return qull;
}

QDateTime qMakeDate( const char *oraDate )
{
  ENTER
  int century = uchar( oraDate[0] );
  if ( century >= 100 )
  {
    int year    = uchar( oraDate[1] );
    year = ( ( century - 100 ) * 100 ) + ( year - 100 );
    int month = oraDate[2];
    int day   = oraDate[3];
    int hour  = oraDate[4] - 1;
    int min   = oraDate[5] - 1;
    int sec   = oraDate[6] - 1;
    return QDateTime( QDate( year, month, day ), QTime( hour, min, sec ) );
  }
  return QDateTime();
}

class QOCISpatialCols
{
  public:
    QOCISpatialCols( int size, QOCISpatialResultPrivate *dp );
    ~QOCISpatialCols();
    int readPiecewise( QVector<QVariant> &values, int index = 0 );
    int readLOBs( QVector<QVariant> &values, int index = 0 );
    int fieldFromDefine( OCIDefine *d );
    void getValues( QVector<QVariant> &v, int index );
    inline int size() { return fieldInf.size(); }
    static bool execBatch( QOCISpatialResultPrivate *d, QVector<QVariant> &boundValues, bool arrayBind );

    QSqlRecord rec;

    struct Point
    {
      Point( double x = 0, double y = 0, double z = 0 )
        : x( x )
        , y( y )
        , z( z )
      {}

      double x = 0;
      double y = 0;
      double z = 0;
    };
    typedef QVector< Point > PointSequence;
    typedef QPair< WKBType, PointSequence > CurvePart;
    typedef QVector< CurvePart > CurveParts;
    typedef QVector< QPair< WKBType, CurveParts > > SurfaceRings;

  private:
    char *create( int position, int size );
    OCILobLocator **createLobLocator( int position, OCIEnv *env );
    OraFieldInfo qMakeOraField( const QOCISpatialResultPrivate *p, OCIParam *param ) const;

    class OraFieldInf
    {
      public:
        OraFieldInf() = default;
        ~OraFieldInf();
        char *data = nullptr;
        int len = 0;
        sb2 ind = 0;
        QMetaType::Type typ = QMetaType::Type::UnknownType;
        ub4 oraType = 0;
        OCIDefine *def = nullptr;
        OCILobLocator *lob = nullptr;
        QString oraTypeName;
    };

    bool convertToWkb( QVariant &v, int index );

    PointSequence circlePoints( double x1, double y1, double x2, double y2, double x3, double y3 );

    QOCISpatialCols::CurveParts getCurveParts( int &iElem, const QVector<int> &vElems, int nOrds, const QVector<double> &ordinates, int nDims, WKBType &baseType, bool &ok );
    bool getValue( OCINumber *num, unsigned int &value );
    bool getValue( OCINumber *num, int &value );
    bool getValue( OCINumber *num, double &value );
    bool getArraySize( OCIColl *coll, int &nSize );
    bool getElemInfoElem( int elem, const QVector<int> &vElem, int nOrds, int &startOffset, int &endOffset, int &etype, int &interpretation );
    static int byteorder() { static char littleEndian = htonl( 1 ) != 1; return littleEndian; }

#ifdef QOCISPATIAL_DEBUG
    void dumpArrays( int nElems, int nOrds );
#endif

    QVector<OraFieldInf> fieldInf;
    const QOCISpatialResultPrivate *const d;
};


Q_DECLARE_TYPEINFO( QOCISpatialCols::Point, Q_PRIMITIVE_TYPE );


QOCISpatialCols::OraFieldInf::~OraFieldInf()
{
  ENTER
  delete [] data;
  if ( lob )
  {
    int r = OCIDescriptorFree( lob, OCI_DTYPE_LOB );
    if ( r != OCI_SUCCESS )
      qWarning( "QOCISpatialCols: Cannot free LOB descriptor" );
  }
}

QOCISpatialCols::QOCISpatialCols( int size, QOCISpatialResultPrivate *dp )
  : fieldInf( size ), d( dp )
{
  ENTER
  ub4 dataSize = 0;
  OCIDefine *dfn = nullptr;
  int r;

  OCIParam *param = nullptr;
  sb4 parmStatus = 0;
  ub4 count = 1;
  int idx = 0;
  parmStatus = OCIParamGet( d->sql,
                            OCI_HTYPE_STMT,
                            d->err,
                            reinterpret_cast<void **>( &param ),
                            count );

  if ( parmStatus != OCI_SUCCESS )
  {
    qOraWarning( "OCIParamGet failed: ", d->err );
  }

  while ( parmStatus == OCI_SUCCESS )
  {
    OraFieldInfo ofi = qMakeOraField( d, param );
    if ( ofi.oraType == SQLT_RDD )
      dataSize = 50;
#ifdef SQLT_INTERVAL_YM
#ifdef SQLT_INTERVAL_DS
    else if ( ofi.oraType == SQLT_INTERVAL_YM || ofi.oraType == SQLT_INTERVAL_DS )
      // since we are binding interval datatype as string,
      // we are not interested in the number of bytes but characters.
      dataSize = 50;  // magic number
#endif //SQLT_INTERVAL_DS
#endif //SQLT_INTERVAL_YM
    else if ( ofi.oraType == SQLT_NUM || ofi.oraType == SQLT_VNU )
    {
      if ( ofi.oraPrecision > 0 )
        dataSize = ( ofi.oraPrecision + 1 ) * sizeof( utext );
      else
        dataSize = ( 38 + 1 ) * sizeof( utext );
    }
    else
      dataSize = ofi.oraLength;

    fieldInf[idx].typ = ofi.type;
    fieldInf[idx].oraType = ofi.oraType;
    fieldInf[idx].oraTypeName = ofi.oraTypeName;
    rec.append( qFromOraInf( ofi ) );

    qDebug() << "ofi.type:" << QVariant::typeToName( ofi.type );
    switch ( ofi.type )
    {
      case QMetaType::Type::QDateTime:
        r = OCIDefineByPos( d->sql,
                            &dfn,
                            d->err,
                            count,
                            create( idx, dataSize + 1 ),
                            dataSize + 1,
                            SQLT_DAT,
                            &( fieldInf[idx].ind ),
                            nullptr, nullptr, OCI_DEFAULT );
        break;
      case QMetaType::Type::Double:
        r = OCIDefineByPos( d->sql,
                            &dfn,
                            d->err,
                            count,
                            create( idx, sizeof( double ) - 1 ),
                            sizeof( double ),
                            SQLT_FLT,
                            &( fieldInf[idx].ind ),
                            nullptr, nullptr, OCI_DEFAULT );
        break;
      case QMetaType::Type::Int:
        r = OCIDefineByPos( d->sql,
                            &dfn,
                            d->err,
                            count,
                            create( idx, sizeof( qint32 ) - 1 ),
                            sizeof( qint32 ),
                            SQLT_INT,
                            &( fieldInf[idx].ind ),
                            nullptr, nullptr, OCI_DEFAULT );
        break;
      case QMetaType::Type::LongLong:
        r = OCIDefineByPos( d->sql,
                            &dfn,
                            d->err,
                            count,
                            create( idx, sizeof( OCINumber ) ),
                            sizeof( OCINumber ),
                            SQLT_VNU,
                            &( fieldInf[idx].ind ),
                            nullptr, nullptr, OCI_DEFAULT );
        break;
      case QMetaType::Type::QByteArray:
        // RAW and LONG RAW fields can't be bound to LOB locators
        qDebug() << "ofi.oraType:" << ofi.oraType;
        if ( ofi.oraType == SQLT_BIN )
        {
          qDebug( "binding SQLT_BIN" );
          r = OCIDefineByPos( d->sql,
                              &dfn,
                              d->err,
                              count,
                              create( idx, dataSize ),
                              dataSize,
                              SQLT_BIN,
                              &( fieldInf[idx].ind ),
                              nullptr, nullptr, OCI_DYNAMIC_FETCH );
        }
        else if ( ofi.oraType == SQLT_LBI )
        {
          qDebug( "binding SQLT_LBI" );
          r = OCIDefineByPos( d->sql,
                              &dfn,
                              d->err,
                              count,
                              nullptr,
                              SB4MAXVAL,
                              SQLT_LBI,
                              &( fieldInf[idx].ind ),
                              nullptr, nullptr, OCI_DYNAMIC_FETCH );
        }
        else if ( ofi.oraType == SQLT_CLOB )
        {
          qDebug( "binding SQLT_CLOB" );
          r = OCIDefineByPos( d->sql,
                              &dfn,
                              d->err,
                              count,
                              createLobLocator( idx, d->env ),
                              -1,
                              SQLT_CLOB,
                              &( fieldInf[idx].ind ),
                              nullptr, nullptr, OCI_DEFAULT );
        }
        else if ( ofi.oraType == SQLT_NTY && ofi.oraTypeName == "SDO_GEOMETRY" )
        {
          qDebug( "binding SQLT_NTY SDO_GEOMETRY" );
          r = OCIDefineByPos( d->sql,
                              &dfn,
                              d->err,
                              count,
                              nullptr,
                              0,
                              SQLT_NTY,
                              nullptr,
                              nullptr,
                              nullptr,
                              OCI_DEFAULT );

          if ( r == OCI_SUCCESS )
          {
            dp->sdoobj.push_back( 0 );
            dp->sdoind.push_back( 0 );

            qDebug( "define object" );
            r = OCIDefineObject( dfn,
                                 d->err,
                                 ofi.oraOCIType,
                                 ( void ** ) & dp->sdoobj.last(), nullptr,
                                 ( void ** ) & dp->sdoind.last(), nullptr );
          }
          else
          {
            qOraWarning( "OCIDefineByPos failed: ", d->err );
          }
        }
        else
        {
          qDebug( "binding SQLT_BLOB" );
          r = OCIDefineByPos( d->sql,
                              &dfn,
                              d->err,
                              count,
                              createLobLocator( idx, d->env ),
                              -1,
                              SQLT_BLOB,
                              &( fieldInf[idx].ind ),
                              nullptr, nullptr, OCI_DEFAULT );
        }
        break;
      case QMetaType::Type::QString:
        if ( ofi.oraType == SQLT_LNG )
        {
          qDebug( "binding SQLT_LNG" );
          r = OCIDefineByPos( d->sql,
                              &dfn,
                              d->err,
                              count,
                              nullptr,
                              SB4MAXVAL,
                              SQLT_LNG,
                              &( fieldInf[idx].ind ),
                              nullptr, nullptr, OCI_DYNAMIC_FETCH );
        }
        else
        {
          dataSize += dataSize + sizeof( QChar );
          qDebug( "OCIDefineByPosStr(%d): %d", count, dataSize );
          r = OCIDefineByPos( d->sql,
                              &dfn,
                              d->err,
                              count,
                              create( idx, dataSize ),
                              dataSize,
                              SQLT_STR,
                              &( fieldInf[idx].ind ),
                              nullptr, nullptr, OCI_DEFAULT );
          if ( r == OCI_SUCCESS )
            d->setCharset( dfn, OCI_HTYPE_DEFINE );
        }
        break;
      default:
        // this should make enough space even with character encoding
        dataSize = ( dataSize + 1 ) * sizeof( utext );
        qDebug( "OCIDefineByPosDef(%d): %d", count, dataSize );
        r = OCIDefineByPos( d->sql,
                            &dfn,
                            d->err,
                            count,
                            create( idx, dataSize ),
                            dataSize + 1,
                            SQLT_STR,
                            &( fieldInf[idx].ind ),
                            nullptr, nullptr, OCI_DEFAULT );
        break;
    }

    if ( r != OCI_SUCCESS )
      qOraWarning( "QOCISpatialCols::bind:", d->err );

    fieldInf[idx].def = dfn;
    ++count;
    ++idx;
    parmStatus = OCIParamGet( d->sql,
                              OCI_HTYPE_STMT,
                              d->err,
                              reinterpret_cast<void **>( &param ),
                              count );
  }
}

QOCISpatialCols::~QOCISpatialCols()
{
  ENTER
}

char *QOCISpatialCols::create( int position, int size )
{
  ENTER
  char *c = new char[size + 1];
  // Oracle may not fill fixed width fields
  memset( c, 0, size + 1 );
  fieldInf[position].data = c;
  fieldInf[position].len = size;
  return c;
}

OCILobLocator **QOCISpatialCols::createLobLocator( int position, OCIEnv *env )
{
  ENTER
  OCILobLocator *&lob = fieldInf[position].lob;
  int r = OCIDescriptorAlloc( env,
                              reinterpret_cast<void **>( &lob ),
                              OCI_DTYPE_LOB,
                              0,
                              nullptr );
  if ( r != OCI_SUCCESS )
  {
    qWarning( "QOCISpatialCols: Cannot create LOB locator" );
    lob = nullptr;
  }
  return &lob;
}

int QOCISpatialCols::readPiecewise( QVector<QVariant> &values, int index )
{
  ENTER
  qDebug() << "readPiecewise( index =" << index << " )";
  OCIDefine     *dfn = nullptr;
  ub4            typep;
  ub1            in_outp;
  ub4            iterp;
  ub4            idxp;
  ub1            piecep;
  sword          status;
  text           col [QOCISPATIAL_DYNAMIC_CHUNK_SIZE + 1];
  int            fieldNum = -1;
  int            r = 0;
  bool           nullField;

  bool firstPiece = true;
  do
  {
    r = OCIStmtGetPieceInfo( d->sql, d->err, reinterpret_cast<void **>( &dfn ), &typep,
                             &in_outp, &iterp, &idxp, &piecep );
    if ( r != OCI_SUCCESS )
      qOraWarning( "QOCISpatialResultPrivate::readPiecewise: unable to get piece info:", d->err );
    fieldNum = fieldFromDefine( dfn );
    bool isStringField = fieldInf.at( fieldNum ).oraType == SQLT_LNG;
    ub4 chunkSize = QOCISPATIAL_DYNAMIC_CHUNK_SIZE;
    nullField = false;
    r  = OCIStmtSetPieceInfo( dfn, OCI_HTYPE_DEFINE,
                              d->err, col,
                              &chunkSize, piecep, nullptr, nullptr );
    if ( r != OCI_SUCCESS )
      qOraWarning( "QOCISpatialResultPrivate::readPiecewise: unable to set piece info:", d->err );
    status = OCIStmtFetch( d->sql, d->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT );
    if ( status == -1 )
    {
      sb4 errcode;
      OCIErrorGet( d->err, 1, nullptr, &errcode, nullptr, 0, OCI_HTYPE_ERROR );
      switch ( errcode )
      {
        case 1405: /* NULL */
          nullField = true;
          break;
        default:
          qOraWarning( "QOCISpatialResultPrivate::readPiecewise: unable to fetch next:", d->err );
          break;
      }
    }
    if ( status == OCI_NO_DATA )
      break;
    if ( nullField || !chunkSize )
    {
      fieldInf[fieldNum].ind = -1;
    }
    else
    {
      if ( isStringField )
      {
        QString str = firstPiece ? QString() : values.at( fieldNum + index ).toString();
        str += QString( reinterpret_cast<const QChar *>( col ), chunkSize / 2 );
        values[fieldNum + index] = str;
        fieldInf[fieldNum].ind = 0;
      }
      else
      {
        QByteArray ba = firstPiece ? QByteArray() : values.at( fieldNum + index ).toByteArray();
        int sz = ba.size();
        ba.resize( sz + chunkSize );
        memcpy( ba.data() + sz, reinterpret_cast<char *>( col ), chunkSize );
        values[fieldNum + index] = ba;
        fieldInf[fieldNum].ind = 0;
      }
      firstPiece = false;
    }
  }
  while ( status == OCI_SUCCESS_WITH_INFO || status == OCI_NEED_DATA );
  return r;
}

OraFieldInfo QOCISpatialCols::qMakeOraField( const QOCISpatialResultPrivate *p, OCIParam *param ) const
{
  ENTER

  OraFieldInfo ofi;
  ub2 colType( 0 );
  text *colName = nullptr;
  ub4 colNameLen( 0 );
  sb1 colScale( 0 );
  ub2 colLength( 0 );
  ub2 colFieldLength( 0 );
  sb2 colPrecision( 0 );
  ub1 colIsNull( 0 );
  text *colTypeName = nullptr;
  ub4 colTypeNameLen( 0 );
  OCIType *colOCIType = nullptr;
  int r( 0 );
  QMetaType::Type type( QMetaType::Type::UnknownType );

  r = OCIAttrGet( param,
                  OCI_DTYPE_PARAM,
                  &colType,
                  nullptr,
                  OCI_ATTR_DATA_TYPE,
                  p->err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "qMakeOraField:", p->err );

  r = OCIAttrGet( param,
                  OCI_DTYPE_PARAM,
                  &colName,
                  &colNameLen,
                  OCI_ATTR_NAME,
                  p->err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "qMakeOraField:", p->err );

  r = OCIAttrGet( param,
                  OCI_DTYPE_PARAM,
                  &colLength,
                  nullptr,
                  OCI_ATTR_DATA_SIZE, /* in bytes */
                  p->err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "qMakeOraField:", p->err );

#ifdef OCI_ATTR_CHAR_SIZE
  r = OCIAttrGet( param,
                  OCI_DTYPE_PARAM,
                  &colFieldLength,
                  nullptr,
                  OCI_ATTR_CHAR_SIZE,
                  p->err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "qMakeOraField:", p->err );
#else
  // for Oracle8.
  colFieldLength = colLength;
#endif

  r = OCIAttrGet( param,
                  OCI_DTYPE_PARAM,
                  &colPrecision,
                  nullptr,
                  OCI_ATTR_PRECISION,
                  p->err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "qMakeOraField:", p->err );

  r = OCIAttrGet( param,
                  OCI_DTYPE_PARAM,
                  &colScale,
                  nullptr,
                  OCI_ATTR_SCALE,
                  p->err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "qMakeOraField:", p->err );

  r = OCIAttrGet( param,
                  OCI_DTYPE_PARAM,
                  &colType,
                  nullptr,
                  OCI_ATTR_DATA_TYPE,
                  p->err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "qMakeOraField:", p->err );

  qDebug() << "colType:" << colLength;

  r = OCIAttrGet( param,
                  OCI_DTYPE_PARAM,
                  &colIsNull,
                  nullptr,
                  OCI_ATTR_IS_NULL,
                  p->err );
  if ( r != OCI_SUCCESS )
    qOraWarning( "qMakeOraField:", p->err );

  if ( colType == SQLT_NTY )
  {
    qDebug() << "NTY!";
    r = OCIAttrGet( param,
                    OCI_DTYPE_PARAM,
                    &colTypeName,
                    &colTypeNameLen,
                    OCI_ATTR_TYPE_NAME,
                    p->err );
    if ( r != OCI_SUCCESS )
      qOraWarning( "qMakeOraField:", p->err );

    qDebug() << "typename: " << QString( reinterpret_cast<const QChar *>( colTypeName ), colTypeNameLen / 2 );

    OCIRef *typeRef = nullptr;

    r = OCIAttrGet( param,
                    OCI_DTYPE_PARAM,
                    &typeRef,
                    nullptr,
                    OCI_ATTR_REF_TDO,
                    p->err );
    if ( r != OCI_SUCCESS )
      qOraWarning( "qMakeOraField:", p->err );

    r = OCIObjectPin( d->env, d->err, typeRef, nullptr, OCI_PIN_ANY, OCI_DURATION_SESSION, OCI_LOCK_NONE, ( void ** ) & colOCIType );
    if ( r != OCI_SUCCESS )
      qOraWarning( "qMakeOraField:", d->err );
  }

  type = qDecodeOCIType( colType, p->q_func()->numericalPrecisionPolicy() );

  if ( type == QMetaType::Type::Int )
  {
    if ( colLength == 22 && colPrecision == 0 && colScale == 0 )
      type = QMetaType::Type::QString;
    if ( colScale > 0 )
      type = QMetaType::Type::QString;
  }

  // bind as double if the precision policy asks for it
  if ( ( ( colType == SQLT_FLT ) || ( colType == SQLT_NUM ) )
       && ( p->q_func()->numericalPrecisionPolicy() == QSql::LowPrecisionDouble ) )
  {
    type = QMetaType::Type::Double;
  }

  // bind as int32 or int64 if the precision policy asks for it
  if ( ( colType == SQLT_NUM ) || ( colType == SQLT_VNU ) || ( colType == SQLT_UIN )
       || ( colType == SQLT_INT ) )
  {
    if ( p->q_func()->numericalPrecisionPolicy() == QSql::LowPrecisionInt64 )
      type = QMetaType::Type::LongLong;
    else if ( p->q_func()->numericalPrecisionPolicy() == QSql::LowPrecisionInt32 )
      type = QMetaType::Type::Int;
  }

  if ( colType == SQLT_BLOB )
    colLength = 0;

  // colNameLen is length in bytes
  ofi.name = QString( reinterpret_cast<const QChar *>( colName ), colNameLen / 2 );
  ofi.type = type;
  ofi.oraType = colType;
  ofi.oraFieldLength = colFieldLength;
  ofi.oraLength = colLength;
  ofi.oraScale = colScale;
  ofi.oraPrecision = colPrecision;
  ofi.oraIsNull = colIsNull;
  ofi.oraTypeName = QString( reinterpret_cast<const QChar *>( colTypeName ), colTypeNameLen / 2 );
  ofi.oraOCIType = colOCIType;

#ifdef QOCISPATIAL_DEBUG
  qDebug() << "name: " << ofi.name
           << "\ntype:" << ofi.type
           << "\noraType:" << ofi.oraType
           << "\noraFieldLength:" << ofi.oraFieldLength
           << "\noraLength:" << ofi.oraLength
           << "\noraScale:" << ofi.oraScale
           << "\noraPrecision:" << ofi.oraPrecision
           << "\noraIsNull:" << ofi.oraIsNull
           << "\noraTypeName:" << ofi.oraTypeName
           << "\n----------------------\n"
           ;
#endif

  return ofi;
}

struct QOCISpatialBatchColumn
{
  inline QOCISpatialBatchColumn() = default;

  OCIBind *bindh = nullptr;
  ub2 bindAs = 0;
  ub4 maxLen = 0;
  ub4 recordCount = 0;
  std::vector<char> data;
  std::vector<ub2> lengths;
  std::vector<sb2> indicators;
  ub4 maxarr_len = 0;
  ub4 curelep = 0;
};

bool QOCISpatialCols::execBatch( QOCISpatialResultPrivate *d, QVector<QVariant> &boundValues, bool arrayBind )
{
  ENTER

  int columnCount = boundValues.count();
  if ( boundValues.isEmpty() || columnCount == 0 )
    return false;

#ifdef QOCISPATIAL_DEBUG
  qDebug() << "columnCount:" << columnCount << boundValues;
#endif

  int i;
  sword r;

  QVarLengthArray<QMetaType::Type> fieldTypes;
  for ( i = 0; i < columnCount; ++i )
  {
    QMetaType::Type tp = static_cast<QMetaType::Type>( boundValues.at( i ).userType() );
    fieldTypes.append( tp == QMetaType::Type::QVariantList ? static_cast<QMetaType::Type>( boundValues.at( i ).toList().value( 0 ).userType() )
                       : tp );
  }

  QList<QByteArray> tmpStorage;
  SizeArray tmpSizes( columnCount );
  QVector<QOCISpatialBatchColumn> columns( columnCount );

  // figuring out buffer sizes
  for ( i = 0; i < columnCount; ++i )
  {

    if ( boundValues.at( i ).userType() != QMetaType::Type::QVariantList )
    {

      // not a list - create a deep-copy of the single value
      QOCISpatialBatchColumn &singleCol = columns[i];
      singleCol.indicators.resize( 1 );
      singleCol.indicators[0] = boundValues.at( i ).isNull() ? -1 : 0;

      r = d->bindValue( d->sql, &singleCol.bindh, d->err, i,
                        boundValues.at( i ), &singleCol.indicators[0], &tmpSizes[i], tmpStorage );

      if ( r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO )
      {
        qOraWarning( "QOCISpatialPrivate::execBatch: unable to bind column:", d->err );
        d->q_func()->setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult",
                                               "Unable to bind column for batch execute" ),
                                               QSqlError::StatementError, d->err ) );
        return false;
      }
      continue;
    }

    QOCISpatialBatchColumn &col = columns[i];
    col.recordCount = boundValues.at( i ).toList().count();

    col.lengths.resize( col.recordCount );
    col.indicators.resize( col.recordCount );
    col.maxarr_len = col.recordCount;
    col.curelep = col.recordCount;

    switch ( fieldTypes[i] )
    {
      case QMetaType::Type::QTime:
      case QMetaType::Type::QDate:
      case QMetaType::Type::QDateTime:
        col.bindAs = SQLT_DAT;
        col.maxLen = 7;
        break;

      case QMetaType::Type::Int:
        col.bindAs = SQLT_INT;
        col.maxLen = sizeof( int );
        break;

      case QMetaType::Type::UInt:
        col.bindAs = SQLT_UIN;
        col.maxLen = sizeof( uint );
        break;

      case QMetaType::Type::LongLong:
      case QMetaType::Type::ULongLong:
        col.bindAs = SQLT_VNU;
        col.maxLen = sizeof( OCINumber );
        break;

      case QMetaType::Type::Double:
        col.bindAs = SQLT_FLT;
        col.maxLen = sizeof( double );
        break;

      case QMetaType::Type::QString:
      {
        col.bindAs = SQLT_STR;
        for ( uint j = 0; j < col.recordCount; ++j )
        {
          uint len;
          if ( d->isOutValue( i ) )
            len = boundValues.at( i ).toList().at( j ).toString().capacity() + 1;
          else
            len = boundValues.at( i ).toList().at( j ).toString().length() + 1;
          if ( len > col.maxLen )
            col.maxLen = len;
        }
        col.maxLen *= sizeof( QChar );
        break;
      }

      case QMetaType::Type::QByteArray:
      default:
      {
        if ( fieldTypes[i] >= QMetaType::Type::User )
        {
          col.bindAs = SQLT_RDD;
          col.maxLen = sizeof( OCIRowid * );
        }
        else
        {
          col.bindAs = SQLT_LBI;
          for ( uint j = 0; j < col.recordCount; ++j )
          {
            if ( d->isOutValue( i ) )
              col.lengths[j] = boundValues.at( i ).toList().at( static_cast<int>( j ) ).toByteArray().capacity();
            else
              col.lengths[j] = boundValues.at( i ).toList().at( static_cast<int>( j ) ).toByteArray().size();
            if ( col.lengths[j] > col.maxLen )
              col.maxLen = col.lengths[j];
          }
        }
        break;
      }
    }

    col.data.resize( static_cast<std::size_t>( col.maxLen ) * col.recordCount );

    // we may now populate column with data
    for ( uint row = 0; row < col.recordCount; ++row )
    {
      const QVariant &val = boundValues.at( i ).toList().at( static_cast<int>( row ) );

      if ( val.isNull() )
      {
        columns[i].indicators[row] = -1;
        columns[i].lengths[row] = 0;
      }
      else
      {
        columns[i].indicators[row] = 0;
        char *dataPtr = &columns[i].data[0] + ( columns[i].maxLen * row );
        switch ( fieldTypes[i] )
        {
          case QMetaType::Type::QTime:
          case QMetaType::Type::QDate:
          case QMetaType::Type::QDateTime:
          {
            columns[i].lengths[row] = columns[i].maxLen;
            const QByteArray ba = qMakeOraDate( val.toDateTime() );
            Q_ASSERT( ba.size() == int( columns[i].maxLen ) );
            memcpy( dataPtr, ba.constData(), columns[i].maxLen );
            break;
          }
          case QMetaType::Type::Int:
            columns[i].lengths[row] = columns[i].maxLen;
            *reinterpret_cast<int *>( dataPtr ) = val.toInt();
            break;

          case QMetaType::Type::UInt:
            columns[i].lengths[row] = columns[i].maxLen;
            *reinterpret_cast<uint *>( dataPtr ) = val.toUInt();
            break;

          case QMetaType::Type::LongLong:
          {
            columns[i].lengths[row] = columns[i].maxLen;
            const QByteArray ba = qMakeOCINumber( val.toLongLong(), d->err );
            Q_ASSERT( ba.size() == int( columns[i].maxLen ) );
            memcpy( dataPtr, ba.constData(), columns[i].maxLen );
            break;
          }
          case QMetaType::Type::ULongLong:
          {
            columns[i].lengths[row] = columns[i].maxLen;
            const QByteArray ba = qMakeOCINumber( val.toULongLong(), d->err );
            Q_ASSERT( ba.size() == int( columns[i].maxLen ) );
            memcpy( dataPtr, ba.constData(), columns[i].maxLen );
            break;
          }
          case QMetaType::Type::Double:
            columns[i].lengths[row] = columns[i].maxLen;
            *reinterpret_cast<double *>( dataPtr ) = val.toDouble();
            break;

          case QMetaType::Type::QString:
          {
            const QString s = val.toString();
            columns[i].lengths[row] = ( ub2 )( s.length() + 1 ) * sizeof( QChar );
            memcpy( dataPtr, s.utf16(), columns[i].lengths[row] );
            break;
          }

          case QMetaType::Type::QByteArray:
          default:
          {
            if ( val.canConvert<QOCISpatialRowIdPointer>() )
            {
              const QOCISpatialRowIdPointer rptr = qvariant_cast<QOCISpatialRowIdPointer>( val );
              *reinterpret_cast<OCIRowid **>( dataPtr ) = rptr->id;
              columns[i].lengths[row] = 0;
            }
            else
            {
              const QByteArray ba = val.toByteArray();
              columns[i].lengths[row] = ba.size();
              memcpy( dataPtr, ba.constData(), ba.size() );
            }
            break;
          }
        }
      }
    }

    QOCISpatialBatchColumn &bindColumn = columns[i];

#ifdef QOCISPATIAL_DEBUG
    qDebug( "OCIBindByPos(%p, %p, %p, %d, %p, %d, %d, %p, %p, 0, %d, %p, OCI_DEFAULT)",
            d->sql, &bindColumn.bindh, d->err, i + 1, bindColumn.data,
            bindColumn.maxLen, bindColumn.bindAs, bindColumn.indicators, bindColumn.lengths,
            arrayBind ? bindColumn.maxarr_len : 0, arrayBind ? &bindColumn.curelep : 0 );

    for ( int ii = 0; ii < static_cast<int>( bindColumn.recordCount ); ++ii )
    {
      qDebug( " record %d: indicator %d, length %d", ii, bindColumn.indicators[ii],
              bindColumn.lengths[ii] );
    }
#endif


    // binding the column
    r = OCIBindByPos(
          d->sql, &bindColumn.bindh, d->err, i + 1,
          &bindColumn.data[0],
          bindColumn.maxLen,
          bindColumn.bindAs,
          &bindColumn.indicators[0],
          &bindColumn.lengths[0],
          nullptr,
          arrayBind ? bindColumn.maxarr_len : 0,
          arrayBind ? &bindColumn.curelep : nullptr,
          OCI_DEFAULT );

#ifdef QOCISPATIAL_DEBUG
    qDebug( "After OCIBindByPos: r = %d, bindh = %p", r, bindColumn.bindh );
#endif

    if ( r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO )
    {
      qOraWarning( "QOCISpatialPrivate::execBatch: unable to bind column:", d->err );
      d->q_func()->setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult",
                                             "Unable to bind column for batch execute" ),
                                             QSqlError::StatementError, d->err ) );
      return false;
    }

    r = OCIBindArrayOfStruct(
          columns[i].bindh, d->err,
          columns[i].maxLen,
          sizeof( columns[i].indicators[0] ),
          sizeof( columns[i].lengths[0] ),
          0 );

    if ( r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO )
    {
      qOraWarning( "QOCISpatialPrivate::execBatch: unable to bind column:", d->err );
      d->q_func()->setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult",
                                             "Unable to bind column for batch execute" ),
                                             QSqlError::StatementError, d->err ) );
      return false;
    }
  }

  //finally we can execute
  r = OCIStmtExecute( d->svc, d->sql, d->err,
                      arrayBind ? 1 : columns[0].recordCount,
                      0, nullptr, nullptr,
                      d->transaction || !d->commitOnSuccess ? OCI_DEFAULT : OCI_COMMIT_ON_SUCCESS );

  if ( r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO )
  {
    qOraWarning( "QOCISpatialPrivate::execBatch: unable to execute batch statement:", d->err );
    d->q_func()->setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult",
                                           "Unable to execute batch statement" ),
                                           QSqlError::StatementError, d->err ) );
    return false;
  }

  // for out parameters we copy data back to value vector
  for ( i = 0; i < columnCount; ++i )
  {

    if ( !d->isOutValue( i ) )
      continue;

    QMetaType::Type tp = static_cast<QMetaType::Type>( boundValues.at( i ).userType() );
    if ( tp != QMetaType::Type::QVariantList )
    {
      qOraOutValue( boundValues[i], tmpStorage, d->err );
      if ( columns[i].indicators[0] == -1 )
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        boundValues[i] = static_cast< QVariant::Type >( tp );
#else
        boundValues[i] = QVariant( QMetaType( tp ) );
#endif
      continue;
    }

    QVariantList *list = static_cast<QVariantList *>( const_cast<void *>( boundValues.at( i ).data() ) );

    const char *data = columns[i].data.data();
    for ( uint r = 0; r < columns[i].recordCount; ++r )
    {

      if ( columns[i].indicators[r] == -1 )
      {
        ( *list )[r] = QVariant();
        continue;
      }

      switch ( columns[i].bindAs )
      {

        case SQLT_DAT:
          ( *list )[r] =  qMakeDate( data + r * columns[i].maxLen );
          break;

        case SQLT_INT:
          ( *list )[r] =  *reinterpret_cast<const int *>( data + r * columns[i].maxLen );
          break;

        case SQLT_UIN:
          ( *list )[r] =  *reinterpret_cast<const uint *>( data + r * columns[i].maxLen );
          break;

        case SQLT_VNU:
        {
          switch ( boundValues.at( i ).userType() )
          {
            case QMetaType::Type::LongLong:
              ( *list )[r] =  qMakeLongLong( data + r * columns[i].maxLen, d->err );
              break;
            case QMetaType::Type::ULongLong:
              ( *list )[r] =  qMakeULongLong( data + r * columns[i].maxLen, d->err );
              break;
            default:
              break;
          }
          break;
        }

        case SQLT_FLT:
          ( *list )[r] =  *reinterpret_cast<const double *>( data + r * columns[i].maxLen );
          break;

        case SQLT_STR:
          ( *list )[r] =  QString( reinterpret_cast<const QChar *>( data
                                   + r * columns[i].maxLen ) );
          break;

        default:
          ( *list )[r] =  QByteArray( data + r * columns[i].maxLen, columns[i].maxLen );
          break;
      }
    }
  }

  d->q_func()->setSelect( false );
  d->q_func()->setAt( QSql::BeforeFirstRow );
  d->q_func()->setActive( true );

  return true;
}

template<class T, int sz>
int qReadLob( T &buf, const QOCISpatialResultPrivate *d, OCILobLocator *lob )
{
  ENTER
  ub1 csfrm;
  ub4 amount;
  int r;

  // Read this from the database, don't assume we know what it is set to
  r = OCILobCharSetForm( d->env, d->err, lob, &csfrm );
  if ( r != OCI_SUCCESS )
  {
    qOraWarning( "OCIResultPrivate::readLobs: Couldn't get LOB char set form: ", d->err );
    csfrm = 0;
  }

  // Get the length of the LOB (this is in characters)
  r = OCILobGetLength( d->svc, d->err, lob, &amount );
  if ( r == OCI_SUCCESS )
  {
    if ( amount == 0 )
    {
      // Short cut for null LOBs
      buf.resize( 0 );
      return OCI_SUCCESS;
    }
  }
  else
  {
    qOraWarning( "OCIResultPrivate::readLobs: Couldn't get LOB length: ", d->err );
    return r;
  }

  // Resize the buffer to hold the LOB contents
  buf.resize( amount );

  // Read the LOB into the buffer
  r = OCILobRead( d->svc,
                  d->err,
                  lob,
                  &amount,
                  1,
                  buf.data(),
                  buf.size() * sz, // this argument is in bytes, not characters
                  nullptr,
                  nullptr,
                  // Extract the data from a CLOB in UTF-16 (ie. what QString uses internally)
                  sz == 1 ? ub2( 0 ) : ub2( QOCISpatialEncoding ),
                  csfrm );

  if ( r != OCI_SUCCESS )
    qOraWarning( "OCIResultPrivate::readLOBs: Cannot read LOB: ", d->err );

  return r;
}

int QOCISpatialCols::readLOBs( QVector<QVariant> &values, int index )
{
  ENTER
  OCILobLocator *lob = nullptr;
  int r = OCI_SUCCESS;

  for ( int i = 0; i < size(); ++i )
  {
    const OraFieldInf &fi = fieldInf.at( i );
    if ( fi.ind == -1 || !( lob = fi.lob ) )
      continue;

    bool isClob = fi.oraType == SQLT_CLOB;
    QVariant var;

    if ( isClob )
    {
      QString str;
      r = qReadLob < QString, sizeof( QChar ) > ( str, d, lob );
      var = str;
    }
    else
    {
      QByteArray buf;
      r = qReadLob < QByteArray, sizeof( char ) > ( buf, d, lob );
      var = buf;
    }
    if ( r == OCI_SUCCESS )
      values[index + i] = var;
    else
      break;
  }
  return r;
}

int QOCISpatialCols::fieldFromDefine( OCIDefine *d )
{
  ENTER
  for ( int i = 0; i < fieldInf.count(); ++i )
  {
    if ( fieldInf.at( i ).def == d )
      return i;
  }
  return -1;
}

bool QOCISpatialCols::getValue( OCINumber *num, unsigned int &value )
{
  if ( OCINumberToInt( d->err, num, sizeof( unsigned int ), OCI_NUMBER_UNSIGNED, &value ) == OCI_SUCCESS )
    return true;

  qOraWarning( "Couldn't convert number to uint: ", d->err );
  return false;
}

bool QOCISpatialCols::getValue( OCINumber *num, int &value )
{
  if ( OCINumberToInt( d->err, num, sizeof( int ), OCI_NUMBER_SIGNED, &value ) == OCI_SUCCESS )
    return true;

  qOraWarning( "Couldn't convert number to int: ", d->err );
  return false;
}

bool QOCISpatialCols::getValue( OCINumber *num, double &value )
{
  if ( OCINumberToReal( d->err, num, sizeof( double ), &value ) == OCI_SUCCESS )
    return true;

  qOraWarning( "Couldn't convert number to double: ", d->err );
  return false;
}

bool QOCISpatialCols::getArraySize( OCIColl *coll, int &nSize )
{
  if ( OCICollSize( d->env, d->err, coll, &nSize ) == OCI_SUCCESS )
    return true;

  qOraWarning( "Couldn't not get elem_info collection size: ", d->err );
  return false;
}

bool QOCISpatialCols::getElemInfoElem( int iElem, const QVector<int> &vElems, int nOrds,
                                       int &startOffset, int &endOffset,
                                       int &etype, int &interpretation )
{
  startOffset = vElems[ iElem + 0 ];
  etype = vElems[ iElem + 1 ];
  interpretation = vElems[ iElem + 2 ];

  if ( iElem + 3 >= vElems.size() )
  {
    endOffset = nOrds + 1;
  }
  else
  {
    endOffset = vElems[ iElem + 3 ];
  }

  --startOffset;
  --endOffset;

  return true;
}

QOCISpatialCols::CurveParts QOCISpatialCols::getCurveParts( int &iElem, const QVector<int> &vElems, int nOrds,
    const QVector<double> &ordinates, int nDims,
    WKBType &baseType, bool &ok )
{
  ok = true;
  int startOffset, endOffset, etype, n;
  if ( !getElemInfoElem( iElem, vElems, nOrds, startOffset, endOffset, etype, n ) )
  {
    qWarning() << "could not fetch element info" << iElem;
    ok = false;
    return CurveParts();
  }

  if ( etype == 2 && ( n == 1 || n == 2 ) )
  {
    // LineString (n == 1) or CircularString (n == 2)
    baseType = ( n == 1 ) ? ( nDims == 2 ? WKBLineString : WKBLineString25D ) : ( nDims == 2 ? WKBCircularString : WKBCircularStringZ );

    PointSequence points;
    points.reserve( 1 + ( endOffset - startOffset ) / nDims );
    for ( int j = startOffset; j < endOffset; j += nDims )
    {
      double x = ordinates[ j ];
      double y = ordinates[ j + 1 ];
      double z = nDims > 2 ? ordinates[ j + 2] : 0;
      points << Point( x, y, z );
    }
    return ( CurveParts() << qMakePair( baseType, points ) );
  }
  else if ( etype == 4 && n > 1 )
  {
    // CompoundCurve
    baseType = nDims == 2 ? WKBCompoundCurve : WKBCompoundCurveZ;
    int compoundParts = n;
    CurveParts parts;
    for ( int k = 0; k < compoundParts; k += 1 )
    {
      iElem += 3;
      if ( !getElemInfoElem( iElem, vElems, nOrds, startOffset, endOffset, etype, n ) )
      {
        qWarning() << "could not fetch element info" << iElem;
        return CurveParts();
      }

      if ( etype == 2 && ( n == 1 || n == 2 ) )
      {
        WKBType partType = ( n == 1 ) ? ( nDims == 2 ? WKBLineString : WKBLineString25D ) : ( nDims == 2 ? WKBCircularString : WKBCircularStringZ );
        PointSequence points;
        points.reserve( 1 + ( endOffset - startOffset ) / nDims );
        for ( int j = startOffset; j < endOffset; j += nDims )
        {
          double x = ordinates[ j ];
          double y = ordinates[ j + 1 ];
          double z = nDims > 2 ? ordinates[ j + 2] : 0;
          points << Point( x, y, z );
        }
        parts << qMakePair( partType, points );
      }
      else
      {
        qWarning( "skipped unsupported compound curve element: etype=%08x n=%d", etype, n );
      }
    }
    return parts;
  }
  else
  {
    qWarning( "skipped unsupported line element: etype=%08x n=%d", etype, n );
    return CurveParts();
  }
}

bool QOCISpatialCols::convertToWkb( QVariant &v, int index )
{
  ENTER

  Q_ASSERT( index < d->sdoobj.size() );
  Q_ASSERT( index < d->sdoind.size() );

  QOCISDOGeometryObj *sdoobj = d->sdoobj[index];
  QOCISDOGeometryInd *sdoind = d->sdoind[index];

  qDebug() << "sdoobj =" << sdoobj;
  qDebug() << "sdoinf =" << sdoind;
  if ( sdoind )
    qDebug() << "sdoind->_atomic =" << sdoind->_atomic;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  v = QVariant( QVariant::ByteArray );
#else
  v = QVariant( QMetaType( QMetaType::Type::QByteArray ) );
#endif

  if ( !sdoobj || !sdoind )
  {
    qDebug() << "sdoobj or sdoind not set";
    return false;
  }

  if ( sdoind->_atomic == OCI_IND_NULL )
  {
    qDebug() << "geometry is NULL";
    return true;
  }

  unsigned int iGType;
  if ( !getValue( &sdoobj->gtype, iGType ) )
    return false;

  int nDims = SDO_GTYPE_D( iGType );
  int iType = SDO_GTYPE_TT( iGType );
  qDebug() << " d =" << nDims;
  qDebug() << " tt =" << iType;

  if ( SDO_GTYPE_L( iGType ) != 0 )
  {
    qWarning() << "LRS" << SDO_GTYPE_L( iGType ) << "ignored";
  }

  int iSrid = 0;
  if ( sdoind->srid == OCI_IND_NOTNULL )
  {
    if ( !getValue( &sdoobj->srid, iSrid ) )
      return false;
  }

  qDebug() << " srid =" << iSrid;

  QByteArray ba;
  union wkbPtr ptr;

  int nElems = 0;
  if ( !getArraySize( sdoobj->elem_info, nElems ) )
  {
    qWarning() << "could not determine element info array size";
    return false;
  }

  int nOrds = 0;
  if ( !getArraySize( sdoobj->ordinates, nOrds ) )
  {
    qWarning() << "could not determine ordinate array size";
    return false;
  }

  Q_ASSERT( nElems % 3 == 0 );
  Q_ASSERT( nOrds % nDims == 0 );

  if ( iType == GtUnknown )
  {
    qWarning() << "unknown geometry";
    return false;
  }

  if ( iType == GtPoint &&
       sdoind->_atomic == OCI_IND_NOTNULL &&
       sdoind->point.x == OCI_IND_NOTNULL &&
       sdoind->point.y == OCI_IND_NOTNULL )
  {

    double x, y, z = 0.0;
    if ( !getValue( &sdoobj->point.x, x ) )
    {
      qWarning() << "could not convert x ordinate to real";
      return false;
    }

    if ( !getValue( &sdoobj->point.y, y ) )
    {
      qWarning() << "could not convert y ordinate to real";
      return false;
    }

    if ( nDims > 2 )
    {
      if ( sdoind->point.z != OCI_IND_NOTNULL )
      {
        qWarning() << "null value in z ordinate";
        return false;
      }

      if ( !getValue( &sdoobj->point.z, z ) )
      {
        qDebug() << "could not convert z ordinate to real";
        return false;
      }
    }

    ba.resize( 1 + sizeof( int ) + nDims * sizeof( double ) );
    ptr.cPtr = ba.data();
    *ptr.ucPtr++ = byteorder();
    *ptr.iPtr++ = nDims == 2 ? WKBPoint : WKBPoint25D;
    *ptr.dPtr++ = x;
    *ptr.dPtr++ = y;
    if ( nDims > 2 )
      *ptr.dPtr++ = z;

    qDebug() << "returning point";
    v = ba;
    return true;
  }

  if ( sdoind->_atomic != OCI_IND_NOTNULL )
  {
    qWarning() << "geometry with sdo_elem_info and non-null sdo_point found.";
    return false;
  }

  QVector<int> elems( nElems );

  try
  {
    QVector<boolean> exists( nElems );
    QVector<OCINumber *> numbers( nElems );
    uword nelems = nElems;
    OCI_VERIFY_E( d->err, OCICollGetElemArray( d->env, d->err, sdoobj->elem_info, 0, exists.data(), ( void ** ) numbers.data(), nullptr, &nelems ) );
    if ( !exists[0] )
    {
      qWarning() << "element info array does not exists";
      throw OCI_ERROR;
    }

    for ( unsigned int i = 0; i < nelems; i++ )
    {
      if ( !getValue( numbers[i], elems[i] ) )
      {
        qWarning() << "get value of element info item" << i << "failed";
        throw OCI_ERROR;
      }
    }
  }
  catch ( int )
  {
    return false;
  }


  QVector<double> ordinates( nOrds );

  try
  {
    QVector<boolean> exists( nOrds );
    QVector<OCINumber *> numbers( nOrds );
    uword nords = nOrds;
    OCI_VERIFY_E( d->err, OCICollGetElemArray( d->env, d->err, sdoobj->ordinates, 0, exists.data(), ( void ** ) numbers.data(), nullptr, &nords ) );
    if ( !exists[0] )
    {
      qWarning() << "ordinate array does not exists";
      throw OCI_ERROR;
    }
    OCI_VERIFY_E( d->err, OCINumberToRealArray( d->err, ( const OCINumber ** ) numbers.data(), nords, sizeof( double ), ordinates.data() ) );
  }
  catch ( int )
  {
    return false;
  }

  if ( iType == GtPoint || iType == GtMultiPoint )
  {
    int nPoints = 0;

    for ( int i = 0; i < nElems; i += 3 )
    {
      int startOffset, endOffset, etype, n;
      if ( !getElemInfoElem( i, elems, nOrds, startOffset, endOffset, etype, n ) )
      {
        qDebug() << "could not fetch element info" << i;
        return false;
      }

      if ( etype == 1 && n > 0 )
        nPoints += endOffset - startOffset;
    }

    Q_ASSERT( nPoints % nDims == 0 );
    Q_ASSERT( iType == GtMultiPoint || nPoints == nDims );

    int wkbSize = 0;

    if ( iType == GtMultiPoint )
      wkbSize += 1 + 2 * sizeof( int );

    wkbSize += ( nPoints / nDims ) * ( 1 + sizeof( int ) ) + nPoints * sizeof( double );
    qDebug() << "wkbSize" << wkbSize;

    ba.resize( wkbSize );
    ptr.cPtr = ba.data();

    if ( iType == GtMultiPoint )
    {
      *ptr.ucPtr++ = byteorder();
      *ptr.iPtr++  = nDims == 2 ? WKBMultiPoint : WKBMultiPoint25D;
      *ptr.iPtr++  = nPoints / nDims;
    }

    for ( int i = 0; i < nElems; i += 3 )
    {
      int startOffset, endOffset, etype, n;
      if ( !getElemInfoElem( i, elems, nOrds, startOffset, endOffset, etype, n ) )
      {
        qDebug() << "could not fetch element info" << i;
        return false;
      }

      if ( etype != 1 )
        continue;

      if ( n == 0 )
      {
        qDebug() << "point orientation skipped";
        continue;
      }

      Q_ASSERT( ( endOffset - startOffset ) % nDims == 0 );

      for ( int j = startOffset, k = 0; j < endOffset; j++, k++ )
      {
        if ( k % nDims == 0 )
        {
          *ptr.ucPtr++ = byteorder();
          *ptr.iPtr++  = nDims == 2 ? WKBPoint : WKBPoint25D;
        }

        Q_ASSERT( j < nOrds );
        *ptr.dPtr++ = ordinates[ j ];
      }
    }

    qDebug() << "returning (multi)point";
    v = ba;
    return true;
  }

  else if ( iType == GtLine || iType == GtMultiLine )
  {
    Q_ASSERT( nOrds % nDims == 0 );

    QVector< QPair<WKBType, CurveParts> > lines;

    bool isCurved = false;
    for ( int i = 0; i < nElems; i += 3 )
    {
      bool ok = false;
      WKBType baseType = WKBUnknown;
      const CurveParts parts = getCurveParts( i, elems, nOrds, ordinates, nDims, baseType, ok );
      if ( !ok )
        return false;

      if ( parts.empty() )
        continue;

      if ( baseType == WKBCompoundCurve || baseType == WKBCompoundCurveZ ||
           baseType == WKBCircularString || baseType == WKBCircularStringZ )
      {
        isCurved = true;
      }
      lines << qMakePair( baseType, parts );
    }

    int binarySize = 1 + sizeof( int ) ;
    if ( iType == GtMultiLine )
      binarySize += sizeof( int );
    for ( int partIndex = 0; partIndex < lines.size(); ++partIndex )
    {
      if ( iType == GtMultiLine )
        binarySize += 1 + sizeof( int );
      auto &line = lines[ partIndex ];

      if ( line.first == WKBCompoundCurve || line.first == WKBCompoundCurveZ )
      {
        binarySize += sizeof( int );
        for ( int partNum = 0; partNum < line.second.size() - 1; ++partNum )
        {
          line.second[ partNum ].second.append( line.second.at( partNum + 1 ).second.first() );
        }
      }

      for ( const CurvePart &part : qAsConst( line.second ) )
      {
        const PointSequence &pts = part.second;
        if ( line.first == WKBCompoundCurve || line.first == WKBCompoundCurveZ )
        {
          binarySize += 1 + sizeof( int );
        }
        binarySize += sizeof( int ) + pts.size() * ( nDims ) * sizeof( double );
      }
    }

    ba.resize( binarySize );
    ptr.cPtr = ba.data();

    Q_ASSERT( iType == GtMultiLine || lines.size() == 1 );
    if ( iType == GtMultiLine )
    {
      *ptr.ucPtr++ = byteorder();
      if ( isCurved )
        *ptr.iPtr++ = nDims == 2 ? WKBMultiCurve : WKBMultiCurveZ;
      else
        *ptr.iPtr++ = nDims == 2 ? WKBMultiLineString : WKBMultiLineString25D;
      *ptr.iPtr++ = lines.size();
    }

    for ( const auto &line : qAsConst( lines ) )
    {
      *ptr.ucPtr++ = byteorder();
      *ptr.iPtr++ = line.first;

      if ( line.first == WKBCompoundCurve || line.first == WKBCompoundCurveZ )
      {
        *ptr.iPtr++ = line.second.size();
      }
      for ( const CurvePart &part : line.second )
      {
        const PointSequence &pts = part.second;
        if ( line.first == WKBCompoundCurve || line.first == WKBCompoundCurveZ )
        {
          *ptr.ucPtr++ = byteorder();
          *ptr.iPtr++ = part.first;
        }
        *ptr.iPtr++ = pts.size();
        for ( const Point &point : pts )
        {
          *ptr.dPtr++ = point.x;
          *ptr.dPtr++ = point.y;
          if ( nDims > 2 )
            *ptr.dPtr++ = point.z;
        }
      }
    }

    Q_ASSERT( ptr.cPtr == ba.data() + ba.size() );

    v = ba;
    return true;
  }

  if ( iType == GtPolygon || iType == GtMultiPolygon )
  {
    QVector< QPair< WKBType, SurfaceRings > > parts;
    SurfaceRings currentPart;
    WKBType currentPartWkbType = WKBUnknown;

    bool isCurved = false;

    for ( int i = 0; i < nElems; i += 3 )
    {
      int startOffset, endOffset, etype, n;
      if ( !getElemInfoElem( i, elems, nOrds, startOffset, endOffset, etype, n ) )
      {
        qWarning() << "could not fetch element info" << i;
        return false;
      }

      if ( etype / 1000 == 1 && !currentPart.empty() )
      {
        // Exterior ring => new Polygon
        parts << qMakePair( currentPartWkbType, currentPart );
        currentPart.clear();
        currentPartWkbType = WKBUnknown;
      }

      if ( etype % 1000 == 3 && ( n == 1 || n == 2 ) )
      {
        // Polygon type or circular arc ring
        PointSequence points;
        points.reserve( 1 + ( endOffset - startOffset ) / nDims );
        for ( int j = startOffset; j < endOffset; j += nDims )
        {
          double x = ordinates[ j ];
          double y = ordinates[ j + 1 ];
          double z = nDims > 2 ? ordinates[ j + 2] : 0;
          points << Point( x, y, z );
        }
        WKBType type = WKBUnknown;
        if ( n == 1 )
        {
          // linear ring
          type = nDims == 2 ? WKBLineString
                 : WKBLineString25D;
          if ( currentPartWkbType == WKBUnknown )
            currentPartWkbType = nDims == 2 ? WKBPolygon : WKBPolygon25D;
        }
        else if ( n == 2 )
        {
          // circular arc ring
          isCurved = true;
          type = nDims == 2 ? WKBCircularString
                 : WKBCircularStringZ;
          currentPartWkbType = nDims == 2 ? WKBCurvePolygon : WKBCurvePolygonZ;
        }

        currentPart << qMakePair( type, CurveParts() << qMakePair( type, points ) );
      }
      else if ( etype % 1000 == 3 && n == 3 )
      {
        // Rectangle - expand to a polygon with 5 points
        double x0 = ordinates[ startOffset + 0 ];
        double y0 = ordinates[ startOffset + 1 ];
        double x1 = ordinates[ startOffset + nDims + 0 ];
        double y1 = ordinates[ startOffset + nDims + 1 ];

        PointSequence points;
        points.reserve( 5 );
        points << Point( x0, y0 );
        points << Point( x1, y0 );
        points << Point( x1, y1 );
        points << Point( x0, y1 );
        points << Point( x0, y0 );
        if ( currentPartWkbType == WKBUnknown )
          currentPartWkbType = WKBPolygon;
        currentPart << qMakePair( WKBLineString, CurveParts() << qMakePair( WKBLineString, points ) );
      }
      else if ( etype % 1000 == 3 && n == 4 )
      {
        // Circle
        isCurved = true;
        double x0 = ordinates[ startOffset + 0 ];
        double y0 = ordinates[ startOffset + 1 ];
        double x1 = ordinates[ startOffset + nDims + 0 ];
        double y1 = ordinates[ startOffset + nDims + 1 ];
        double x2 = ordinates[ startOffset + 2 * nDims + 0 ];
        double y2 = ordinates[ startOffset + 2 * nDims + 1 ];
        currentPartWkbType = WKBCurvePolygon;
        currentPart << qMakePair( WKBCircularString, CurveParts() << qMakePair( WKBCircularString, circlePoints( x0, y0, x1, y1, x2, y2 ) ) );
      }
      else if ( etype % 1000 == 5 && n > 1 )
      {
        // CompoundCurve ring
        isCurved = true;
        int compoundParts = n;
        currentPartWkbType = ( nDims == 2 ? WKBCurvePolygon : WKBCurvePolygonZ );
        CurveParts parts;
        for ( int k = 0; k < compoundParts; k += 1 )
        {
          i += 3;
          if ( !getElemInfoElem( i, elems, nOrds, startOffset, endOffset, etype, n ) )
          {
            qWarning() << "could not fetch element info" << i;
            continue;
          }

          if ( etype == 2 && ( n == 1 || n == 2 ) )
          {
            WKBType partType = ( n == 1 ) ?
                               ( nDims == 2 ? WKBLineString : WKBLineString25D ) :
                               ( nDims == 2 ? WKBCircularString : WKBCircularStringZ );
            PointSequence points;
            points.reserve( 1 + ( endOffset - startOffset ) / nDims );
            for ( int j = startOffset; j < endOffset; j += nDims )
            {
              double x = ordinates[ j ];
              double y = ordinates[ j + 1 ];
              double z = nDims > 2 ? ordinates[ j + 2] : 0;
              points << Point( x, y, z );
            }
            parts << qMakePair( partType, points );
          }
          else
          {
            qWarning( "skipped unsupported compound curve element: etype=%08x n=%d", etype, n );
          }
        }
        currentPart << qMakePair( nDims == 2 ? WKBCompoundCurve : WKBCompoundCurveZ, parts );
      }
      else
      {
        qWarning( "skipped unsupported polygon element: etype=%08x n=%d", etype, n );
      }
    }

    if ( parts.empty() && currentPart.empty() )
      return false;

    if ( !currentPart.empty() )
      parts << qMakePair( currentPartWkbType, currentPart );

    int wkbSize = 1 + sizeof( int );
    const int nPolygons = parts.size();
    const bool isMultiPolygon = iType == GtMultiPolygon;
    if ( isMultiPolygon )
      wkbSize += sizeof( int );
    for ( int part = 0; part < nPolygons; ++part )
    {
      SurfaceRings &rings = parts[ part ].second;
      if ( isMultiPolygon )
        wkbSize += 1 + sizeof( int );
      wkbSize += sizeof( int );
      for ( int ringIdx = 0; ringIdx < rings.size(); ++ringIdx )
      {
        CurveParts &ring = rings[ ringIdx ].second;

        if ( parts[ part ].first == WKBCurvePolygon || parts[ part ].first == WKBCurvePolygonZ )
        {
          wkbSize += 1 + sizeof( int );
        }
        if ( rings[ ringIdx ].first == WKBCompoundCurve || rings[ ringIdx ].first == WKBCompoundCurveZ )
        {
          wkbSize += sizeof( int );
          for ( int partNum = 0; partNum < ring.size() - 1; ++partNum )
          {
            ring[ partNum ].second.append( ring.at( partNum + 1 ).second.first() );
          }
        }

        for ( const CurvePart &curvePart : qAsConst( ring ) )
        {
          if ( rings[ ringIdx ].first == WKBCompoundCurve || rings[ ringIdx ].first == WKBCompoundCurveZ )
            wkbSize += 1 + sizeof( int );
          wkbSize += sizeof( int ) + curvePart.second.size() * nDims * sizeof( double );
        }
      }
    }

    qDebug() << "wkbSize" << wkbSize;

    ba.resize( wkbSize );

    ptr.cPtr = ba.data();
    *ptr.ucPtr++ = byteorder();

    if ( !isMultiPolygon )
    {
      if ( isCurved )
        *ptr.iPtr++ = nDims == 2 ? WKBCurvePolygon : WKBCurvePolygonZ;
      else
        *ptr.iPtr++ = nDims == 2 ? WKBPolygon : WKBPolygon25D;
    }
    else
    {
      if ( isCurved )
        *ptr.iPtr++ = nDims == 2 ? WKBMultiSurface : WKBMultiSurfaceZ;
      else
        *ptr.iPtr++ = nDims == 2 ? WKBMultiPolygon : WKBMultiPolygon25D;
      *ptr.iPtr++ = nPolygons;
    }

    for ( const QPair< WKBType, SurfaceRings > &rings : qAsConst( parts ) )
    {
      if ( isMultiPolygon )
      {
        *ptr.ucPtr++ = byteorder();
        *ptr.iPtr++ = rings.first;
      }

      *ptr.iPtr++ = rings.second.size();
      for ( const QPair< WKBType, CurveParts > &ring : rings.second )
      {
        if ( rings.first == WKBCurvePolygon || rings.first == WKBCurvePolygonZ )
        {
          *ptr.ucPtr++ = byteorder();
          *ptr.iPtr++ = ring.first;
        }
        if ( ring.first == WKBCompoundCurve || ring.first == WKBCompoundCurveZ )
        {
          *ptr.iPtr++ = ring.second.size();
        }
        for ( const CurvePart &curvePart : ring.second )
        {
          if ( ring.first == WKBCompoundCurve || ring.first == WKBCompoundCurveZ )
          {
            *ptr.ucPtr++ = byteorder();
            *ptr.iPtr++ = curvePart.first;
          }

          *ptr.iPtr++ = curvePart.second.size();
          for ( const Point &point : curvePart.second )
          {
            *ptr.dPtr++ = point.x;
            *ptr.dPtr++ = point.y;
            if ( nDims > 2 )
              *ptr.dPtr++ = point.z;
          }
        }
      }
    }

    Q_ASSERT( ptr.cPtr == ba.data() + ba.size() );

    qDebug() << "returning (multi)polygon size" << ba.size();
    v = ba;
    return true;
  }

  qWarning() << "geometry type" << iType << "not supported";
  return false;
}

inline bool doubleNear( double a, double b, double epsilon )
{
  const double diff = a - b;
  return diff > -epsilon && diff <= epsilon;
}

QOCISpatialCols::PointSequence QOCISpatialCols::circlePoints( double x1, double y1, double x2, double y2, double x3, double y3 )
{
  auto isPerpendicular = []( double x1, double y1, double x2, double y2, double x3, double y3 )->bool
  {
    // check the given point are perpendicular to x or y axis

    double yDelta_a = y2 - y1;
    double xDelta_a = x2 - x1;
    double yDelta_b = y3 - y2;
    double xDelta_b = x3 - x2;

    if ( ( std::fabs( xDelta_a ) <= 1E-8 ) && ( std::fabs( yDelta_b ) <= 1E-8 ) )
    {
      return false;
    }

    if ( std::fabs( yDelta_a ) <= 1E-8 )
    {
      return true;
    }
    else if ( std::fabs( yDelta_b ) <= 1E-8 )
    {
      return true;
    }
    else if ( std::fabs( xDelta_a ) <= 1E-8 )
    {
      return true;
    }
    else if ( std::fabs( xDelta_b ) <= 1E-8 )
    {
      return true;
    }

    return false;
  };

  auto toCircularStringPoints = []( double centerX, double centerY, double radius ) -> PointSequence
  {
    PointSequence sequence;
    sequence.append( Point( centerX, centerY + radius ) );
    sequence.append( Point( centerX + radius, centerY ) );
    sequence.append( Point( centerX, centerY - radius ) );
    sequence.append( Point( centerX - radius, centerY ) );
    sequence.append( sequence.at( 0 ) );
    return sequence;
  };

  if ( !isPerpendicular( x1, y1, x2, y2, x3, y3 ) )
  {

  }
  else if ( !isPerpendicular( x1, y1, x3, y3, x2, y2 ) )
  {
    std::swap( x2, x3 );
    std::swap( y2, y3 );
  }
  else if ( !isPerpendicular( x2, y2, x1, y1, x3, y3 ) )
  {
    std::swap( x1, x2 );
    std::swap( y1, y2 );
  }
  else if ( !isPerpendicular( x2, y2, x3, y3, x1, y1 ) )
  {
    double ax1 = x1;
    double ay1 = y1;
    double ax2 = x2;
    double ay2 = y2;
    double ax3 = x3;
    double ay3 = y3;
    x1 = ax2;
    y1 = ay2;
    x2 = ax3;
    y2 = ay3;
    x3 = ax1;
    y3 = ay1;
  }
  else if ( !isPerpendicular( x3, y3, x2, y2, x1, y1 ) )
  {
    std::swap( x1, x3 );
    std::swap( y1, y3 );
  }
  else if ( !isPerpendicular( x2, y3, x1, y1, x2, y2 ) )
  {
    double ax1 = x1;
    double ay1 = y1;
    double ax2 = x2;
    double ay2 = y2;
    double ax3 = x3;
    double ay3 = y3;
    x1 = ax3;
    y1 = ay3;
    x2 = ax1;
    y2 = ay1;
    x3 = ax2;
    y3 = ay2;
  }
  else
  {
    return PointSequence();
  }

  double radius = -0.0;
  // Paul Bourke's algorithm
  double yDelta_a = y2 - y1;
  double xDelta_a = x2 - x1;
  double yDelta_b = y3 - y2;
  double xDelta_b = x3 - x2;

  if ( doubleNear( xDelta_a, 0.0, 1E-8 ) || doubleNear( xDelta_b, 0.0, 1E-8 ) )
  {
    return PointSequence();
  }

  double aSlope = yDelta_a / xDelta_a;
  double bSlope = yDelta_b / xDelta_b;
  double centerX = 0;
  double centerY = 0;

  if ( ( std::fabs( xDelta_a ) <= 1E-8 ) && ( std::fabs( yDelta_b ) <= 1E-8 ) )
  {
    centerX = ( 0.5 * ( x2 + x3 ) );
    centerY = ( 0.5 * ( y1 + y2 ) );
    radius = std::sqrt( ( centerX - x1 ) * ( centerX - x1 ) + ( centerY - y1 ) * ( centerY - y1 ) );
    return toCircularStringPoints( centerX, centerY, radius );
  }

  if ( std::fabs( aSlope - bSlope ) <= 1E-8 )
  {
    return PointSequence();
  }

  centerX = (
              ( aSlope * bSlope * ( y1 - y3 ) +
                bSlope * ( x1 + x2 ) -
                aSlope * ( x2 + x3 ) ) /
              ( 2.0 * ( bSlope - aSlope ) )
            );
  centerY = (
              -1.0 * ( centerX - ( x1 + x2 ) / 2.0 ) /
              aSlope + ( y1 + y2 ) / 2.0
            );

  radius = std::sqrt( ( centerX - x1 ) * ( centerX - x1 ) + ( centerY - y1 ) * ( centerY - y1 ) );
  return toCircularStringPoints( centerX, centerY, radius );
}

void QOCISpatialCols::getValues( QVector<QVariant> &v, int index )
{
  ENTER
  for ( int i = 0, gcindex = 0; i < fieldInf.size(); ++i )
  {
    qDebug() << "getValues( index =" << index << "i =" << i << " )";
    const OraFieldInf &fld = fieldInf.at( i );

    if ( fld.ind == -1 )
    {
      // got a NULL value
      qDebug() << "NULL";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      v[index + i] = static_cast< QVariant::Type >( fld.typ );
#else
      v[index + i] = QVariant( QMetaType( fld.typ ) );
#endif
      continue;
    }

    qDebug() << "oraType:" << fld.oraType;
    if ( fld.oraType == SQLT_BIN || fld.oraType == SQLT_LBI || fld.oraType == SQLT_LNG )
    {
      qDebug() << "fetching piecewise";
      continue; // already fetched piecewise
    }

    switch ( fld.typ )
    {
      case QMetaType::Type::QDateTime:
        qDebug() << "DateTime";
        v[index + i] = QVariant( qMakeDate( fld.data ) );
        break;
      case QMetaType::Type::Double:
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
        if ( d->q_func()->numericalPrecisionPolicy() != QSql::HighPrecision )
        {
          if ( ( d->q_func()->numericalPrecisionPolicy() == QSql::LowPrecisionDouble )
               && ( fld.typ == QMetaType::Type::Double ) )
          {
            v[index + i] = *reinterpret_cast<double *>( fld.data );
            qDebug() << "double" << v[index + i].toDouble();
            break;
          }
          else if ( ( d->q_func()->numericalPrecisionPolicy() == QSql::LowPrecisionInt64 )
                    && ( fld.typ == QMetaType::Type::LongLong ) )
          {
            qint64 qll = 0;
            int r = OCINumberToInt( d->err, reinterpret_cast<OCINumber *>( fld.data ), sizeof( qint64 ),
                                    OCI_NUMBER_SIGNED, &qll );
            if ( r == OCI_SUCCESS )
            {
              v[index + i] = qll;
              qDebug() << "qint64" << qll;
            }
            else
            {
              qDebug() << "qint64 invalid";
              v[index + i] = QVariant();
            }
            break;
          }
          else if ( ( d->q_func()->numericalPrecisionPolicy() == QSql::LowPrecisionInt32 )
                    && ( fld.typ == QMetaType::Type::Int ) )
          {
            v[index + i] = *reinterpret_cast<int *>( fld.data );
            qDebug() << "int" << v[index + i].toInt();
            break;
          }
        }

        FALLTHROUGH

      case QMetaType::Type::QString:
        qDebug() << "String";
        v[index + i] = QString( reinterpret_cast<const QChar *>( fld.data ) );
        qDebug() << "string" << v[index + i].toString();
        break;
      case QMetaType::Type::QByteArray:
        if ( fld.oraType == SQLT_NTY && fld.oraTypeName == "SDO_GEOMETRY" )
        {
          qDebug() << "SQLT_NTY SDO_GEOMETRY";
          convertToWkb( v[ index + i ], gcindex++ );
        }
        else
        {
          qDebug() << "ByteArray length =" << fld.len;
          if ( fld.len > 0 )
            v[index + i] = QByteArray( fld.data, fld.len );
          else
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            v[index + i] = QVariant( QVariant::ByteArray );
#else
            v[index + i] = QVariant( QMetaType( QMetaType::Type::QByteArray ) );
#endif
        }
        break;
      default:
        qWarning( "QOCISpatialCols::value: unknown data type %s", QVariant::typeToName( fld.typ ) );
        break;
    }
  }
}

QOCISpatialResultPrivate::QOCISpatialResultPrivate( QOCISpatialResult *q, const QOCISpatialDriver *drv )
  : QSqlCachedResultPrivate( q, drv )
  , env( drv_d_func()->env )
  , svc( const_cast<OCISvcCtx * &>( drv_d_func()->svc ) )
  , transaction( drv_d_func()->transaction )
  , commitOnSuccess( drv_d_func()->commitOnSuccess )
  , serverVersion( drv_d_func()->serverVersion )
  , prefetchRows( drv_d_func()->prefetchRows )
  , prefetchMem( drv_d_func()->prefetchMem )
  , geometryTDO( drv_d_func()->geometryTDO )
{
  ENTER
  int r = OCIHandleAlloc( env,
                          reinterpret_cast<void **>( &err ),
                          OCI_HTYPE_ERROR,
                          0,
                          nullptr );
  if ( r != OCI_SUCCESS )
    qWarning( "QOCISpatialResult: unable to alloc error handle" );
}

QOCISpatialResultPrivate::~QOCISpatialResultPrivate()
{
  ENTER
  delete cols;

  int r;
  if ( geometryObj )
  {
    r = OCIObjectFree( env, err, geometryObj, OCI_OBJECTFREE_FORCE );
    if ( r != OCI_SUCCESS )
      qOraWarning( "~QOCISpatialResult: unable to free geometry object", err );
  }

  r = OCIHandleFree( err, OCI_HTYPE_ERROR );
  if ( r != OCI_SUCCESS )
    qWarning( "~QOCISpatialResult: unable to free error handle" );

  if ( sql )
  {
    r = OCIHandleFree( sql, OCI_HTYPE_STMT );
    if ( r != OCI_SUCCESS )
      qWarning( "~QOCISpatialResult: unable to free statement handle" );
  }
}


////////////////////////////////////////////////////////////////////////////

QOCISpatialResult::QOCISpatialResult( const QOCISpatialDriver *db )
  : QSqlCachedResult( *new QOCISpatialResultPrivate( this, db ) )
{
  ENTER
}

QOCISpatialResult::~QOCISpatialResult()
{
  ENTER
}

QVariant QOCISpatialResult::handle() const
{
  ENTER
  return QVariant::fromValue( d_func()->sql );
}

bool QOCISpatialResult::reset( const QString &query )
{
  ENTER
  return prepare( query ) && exec();
}

bool QOCISpatialResult::gotoNext( QSqlCachedResult::ValueCache &values, int index )
{
  ENTER
  Q_D( QOCISpatialResult );
  qDebug() << "gotoNext( index =" << index << ")";
  if ( at() == QSql::AfterLastRow )
    return false;

  bool piecewise = false;
  int r = OCIStmtFetch2( d->sql, d->err, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT );

  if ( index < 0 ) //not interested in values
    return r == OCI_SUCCESS || r == OCI_SUCCESS_WITH_INFO;

  switch ( r )
  {
    case OCI_SUCCESS:
      break;
    case OCI_SUCCESS_WITH_INFO:
      qOraWarning( "QOCISpatialResult::gotoNext: SuccessWithInfo: ", d->err );
      qDebug() << "QOCISpatialResult::gotoNext: statement " << lastQuery();
      r = OCI_SUCCESS; //ignore it
      break;
    case OCI_NO_DATA:
      // end of rowset
      return false;
    case OCI_NEED_DATA:
      qDebug( "QOCISpatialResult::gotoNext: NEED DATA - fetching piecewise" );
      piecewise = true;
      r = OCI_SUCCESS;
      break;
    case OCI_ERROR:
      if ( qOraErrorNumber( d->err ) == 1406 )
      {
        qWarning( "QOCISpatial Warning: data truncated for %s", lastQuery().toLocal8Bit().constData() );
        r = OCI_SUCCESS; /* ignore it */
        break;
      }
      FALLTHROUGH

    default:
      qOraWarning( "QOCISpatialResult::gotoNext: ", d->err );
      setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult",
                                "Unable to goto next" ),
                                QSqlError::StatementError, d->err ) );
      break;
  }

  // need to read piecewise before assigning values
  if ( r == OCI_SUCCESS && piecewise )
  {
    r = d->cols->readPiecewise( values, index );
  }

  if ( r == OCI_SUCCESS )
    d->cols->getValues( values, index );

  if ( r == OCI_SUCCESS )
    r = d->cols->readLOBs( values, index );

  if ( r != OCI_SUCCESS )
    setAt( QSql::AfterLastRow );

  return r == OCI_SUCCESS || r == OCI_SUCCESS_WITH_INFO;
}

int QOCISpatialResult::size()
{
  ENTER
  Q_D( QOCISpatialResult );
  int rowCount;
  if ( OCIAttrGet( d->sql,
                   OCI_HTYPE_STMT,
                   &rowCount,
                   nullptr,
                   OCI_ATTR_ROWS_FETCHED,
                   d->err ) == OCI_SUCCESS )
  {
    return rowCount;
  }
  else
  {
    return -1;
  }
}

int QOCISpatialResult::numRowsAffected()
{
  ENTER
  Q_D( QOCISpatialResult );
  int rowCount;
  OCIAttrGet( d->sql,
              OCI_HTYPE_STMT,
              &rowCount,
              nullptr,
              OCI_ATTR_ROW_COUNT,
              d->err );
  return rowCount;
}

bool QOCISpatialResult::prepare( const QString &query )
{
  ENTER
  Q_D( QOCISpatialResult );

  static int sDebugLevel = -1;
  if ( sDebugLevel < 0 )
  {
    if ( getenv( "QGIS_DEBUG" ) )
      sDebugLevel = atoi( getenv( "QGIS_DEBUG" ) );
    else
      sDebugLevel = 0;
  }

  if ( sDebugLevel >= 4 )
    qDebug() << "prepare(" << query << ")";

  int r = 0;
  QSqlResult::prepare( query );

  delete d->cols;
  d->cols = nullptr;
  QSqlCachedResult::cleanup();

  if ( d->sql )
  {
    r = OCIHandleFree( d->sql, OCI_HTYPE_STMT );
    if ( r != OCI_SUCCESS )
      qOraWarning( "QOCISpatialResult::prepare: unable to free statement handle:", d->err );
    d->sql = nullptr;
  }
  if ( query.isEmpty() )
    return false;
  r = OCIHandleAlloc( d->env,
                      reinterpret_cast<void **>( &d->sql ),
                      OCI_HTYPE_STMT,
                      0,
                      nullptr );
  if ( r != OCI_SUCCESS )
  {
    qOraWarning( "QOCISpatialResult::prepare: unable to alloc statement:", d->err );
    setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult",
                              "Unable to alloc statement" ), QSqlError::StatementError, d->err ) );
    return false;
  }
  d->setStatementAttributes();
  const OraText *txt = reinterpret_cast<const OraText *>( query.utf16() );
  const int len = query.length() * sizeof( QChar );
  r = OCIStmtPrepare( d->sql,
                      d->err,
                      txt,
                      len,
                      OCI_NTV_SYNTAX,
                      OCI_DEFAULT );
  if ( r != OCI_SUCCESS )
  {
    qOraWarning( "QOCISpatialResult::prepare: unable to prepare statement:", d->err );
    setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult",
                              "Unable to prepare statement" ), QSqlError::StatementError, d->err ) );
    return false;
  }

  return true;
}

bool QOCISpatialResult::exec()
{
  ENTER
  Q_D( QOCISpatialResult );
  int r = 0;
  ub2 stmtType = 0;
  ub4 iters;
  ub4 mode;
  QList<QByteArray> tmpStorage;
  IndicatorArray indicators( boundValueCount() );
  SizeArray tmpSizes( boundValueCount() );

  r = OCIAttrGet( d->sql,
                  OCI_HTYPE_STMT,
                  &stmtType,
                  nullptr,
                  OCI_ATTR_STMT_TYPE,
                  d->err );

  if ( r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO )
  {
    qOraWarning( "QOCISpatialResult::exec: Unable to get statement type:", d->err );
    setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult",
                              "Unable to get statement type" ), QSqlError::StatementError, d->err ) );
    qWarning( "type retrieval failed with statement:%s", lastQuery().toLocal8Bit().constData() );
    return false;
  }

  iters = stmtType == OCI_STMT_SELECT ? 0 : 1;
  mode = d->transaction || !d->commitOnSuccess ? OCI_DEFAULT : OCI_COMMIT_ON_SUCCESS;

  qDebug() << "iters:" << iters;

  // bind placeholders
  if ( boundValueCount() > 0
       && d->bindValues( boundValues(), indicators, tmpSizes, tmpStorage ) != OCI_SUCCESS )
  {
    qOraWarning( "QOCISpatialResult::exec: unable to bind value: ", d->err );
    setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult", "Unable to bind value" ),
                              QSqlError::StatementError, d->err ) );
    qWarning( "bind failed with statement:%s", lastQuery().toLocal8Bit().constData() );
    return false;
  }

  // execute
  r = OCIStmtExecute( d->svc,
                      d->sql,
                      d->err,
                      iters,
                      0,
                      nullptr,
                      nullptr,
                      mode );
  if ( r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO )
  {
    qOraWarning( "QOCISpatialResult::exec: unable to execute statement:", d->err );
    setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialResult",
                              "Unable to execute statement" ), QSqlError::StatementError, d->err ) );
    qWarning( "execution failed with statement:%s", lastQuery().toLocal8Bit().constData() );
    return false;
  }

  if ( stmtType == OCI_STMT_SELECT )
  {
    ub4 parmCount = 0;
    int r = OCIAttrGet( d->sql, OCI_HTYPE_STMT, reinterpret_cast<void **>( &parmCount ),
                        nullptr, OCI_ATTR_PARAM_COUNT, d->err );
    if ( r == OCI_SUCCESS && !d->cols )
    {
      d->sdoobj.clear();
      d->sdoind.clear();
      d->cols = new QOCISpatialCols( parmCount, d );
    }
    else
      qOraWarning( "QOCISpatialResult::exec: get param count failed:", d->err );
    setSelect( true );
    QSqlCachedResult::init( parmCount );
  }
  else   /* non-SELECT */
  {
    setSelect( false );
  }
  setAt( QSql::BeforeFirstRow );
  setActive( true );

  if ( hasOutValues() )
    d->outValues( boundValues(), indicators, tmpStorage );

  return true;
}

QSqlRecord QOCISpatialResult::record() const
{
  ENTER
  Q_D( const QOCISpatialResult );
  QSqlRecord inf;
  if ( !isActive() || !isSelect() || !d->cols )
    return inf;
  return d->cols->rec;
}

QVariant QOCISpatialResult::lastInsertId() const
{
  ENTER
  Q_D( const QOCISpatialResult );
  if ( isActive() )
  {
    QOCISpatialRowIdPointer ptr( new QOCISpatialRowId( d->env ) );

    int r = OCIAttrGet( d->sql, OCI_HTYPE_STMT, ptr.constData()->id,
                        nullptr, OCI_ATTR_ROWID, d->err );
    if ( r == OCI_SUCCESS )
      return QVariant::fromValue( ptr );
  }
  return QVariant();
}

bool QOCISpatialResult::execBatch( bool arrayBind )
{
  Q_D( QOCISpatialResult );
  QOCISpatialCols::execBatch( d, boundValues(), arrayBind );
  resetBindCount();
  return lastError().type() == QSqlError::NoError;
}

void QOCISpatialResult::virtual_hook( int id, void *data )
{
  Q_ASSERT( data );

  QSqlCachedResult::virtual_hook( id, data );
}

////////////////////////////////////////////////////////////////////////////


QOCISpatialDriver::QOCISpatialDriver( QObject *parent )
  : QSqlDriver( *new QOCISpatialDriverPrivate, parent )
{
  ENTER
  Q_D( QOCISpatialDriver );

#ifdef QOCISPATIAL_THREADED
  const ub4 mode = OCI_UTF16 | OCI_OBJECT | OCI_THREADED;
#else
  const ub4 mode = OCI_UTF16 | OCI_OBJECT;
#endif
  int r = OCIEnvCreate( &d->env,
                        mode,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        0,
                        nullptr );
  if ( r != OCI_SUCCESS )
  {
    qWarning( "QOCISpatialDriver: unable to create environment" );
    setLastError( qMakeError( tr( "Unable to initialize", "QOCISpatialDriver" ),
                              QSqlError::ConnectionError, d->err ) );
    return;
  }

  d->allocErrorHandle();
}

QOCISpatialDriver::QOCISpatialDriver( OCIEnv *env, OCISvcCtx *ctx, QObject *parent )
  : QSqlDriver( *new QOCISpatialDriverPrivate, parent )
{
  ENTER
  Q_D( QOCISpatialDriver );
  d->env = env;
  d->svc = ctx;

  d->allocErrorHandle();

  if ( env && ctx )
  {
    setOpen( true );
    setOpenError( false );
  }
}

QOCISpatialDriver::~QOCISpatialDriver()
{
  ENTER
  Q_D( QOCISpatialDriver );
  if ( isOpen() )
    close();
  int r = OCIHandleFree( d->err, OCI_HTYPE_ERROR );
  if ( r != OCI_SUCCESS )
    qWarning( "Unable to free Error handle: %d", r );
  r = OCIHandleFree( d->env, OCI_HTYPE_ENV );
  if ( r != OCI_SUCCESS )
    qWarning( "Unable to free Environment handle: %d", r );

  // delete d;
}

bool QOCISpatialDriver::hasFeature( DriverFeature f ) const
{
  ENTER
  Q_D( const QOCISpatialDriver );
  switch ( f )
  {
    case Transactions:
    case LastInsertId:
    case BLOB:
    case PreparedQueries:
    case NamedPlaceholders:
    case BatchOperations:
    case LowPrecisionNumbers:
      return true;
    case QuerySize:
    case PositionalPlaceholders:
    case SimpleLocking:
    case EventNotifications:
    case FinishQuery:
    case CancelQuery:
    case MultipleResultSets:
      return false;
    case Unicode:
      return d->serverVersion >= 9;
  }
  return false;
}

static void qParseOpts( const QString &options, QOCISpatialDriverPrivate *d )
{
  ENTER
  const QStringList opts( options.split( QLatin1Char( ';' ), Qt::SkipEmptyParts ) );
  for ( int i = 0; i < opts.count(); ++i )
  {
    const QString tmp( opts.at( i ) );
    int idx;
    if ( ( idx = tmp.indexOf( QLatin1Char( '=' ) ) ) == -1 )
    {
      qWarning( "QOCISpatialDriver::parseArgs: Invalid parameter: '%s'",
                tmp.toLocal8Bit().constData() );
      continue;
    }
    const QString opt = tmp.left( idx );
    const QString val = tmp.mid( idx + 1 ).simplified();
    bool ok;
    if ( opt == QLatin1String( "OCI_ATTR_PREFETCH_ROWS" ) )
    {
      int intVal = val.toInt( &ok );
      if ( !ok )
        d->prefetchRows = QOCISPATIAL_PREFETCH_ROWS;
      else if ( intVal >= 0 )
        d->prefetchRows = static_cast<ub4>( intVal );
    }
    else if ( opt == QLatin1String( "OCI_ATTR_PREFETCH_MEMORY" ) )
    {
      int intVal = val.toInt( &ok );
      if ( !ok )
        d->prefetchMem = QOCISPATIAL_PREFETCH_MEM;
      else if ( intVal >= 0 )
        d->prefetchMem = static_cast<ub4>( intVal );
    }
    else if ( opt == QLatin1String( "COMMIT_ON_SUCCESS" ) )
    {
      d->commitOnSuccess = val.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;
    }
    else
    {
      qWarning( "QOCISpatialDriver::parseArgs: Invalid parameter: '%s'",
                opt.toLocal8Bit().constData() );
    }
  }
}

bool QOCISpatialDriver::open( const QString &db,
                              const QString &user,
                              const QString &password,
                              const QString &hostname,
                              int port,
                              const QString &opts )
{
  ENTER
  Q_D( QOCISpatialDriver );
  int r;

  if ( isOpen() )
    close();

  qParseOpts( opts, d );

  // Connect without tnsnames.ora if a hostname is given
  QString connectionString = db;
  if ( !hostname.isEmpty() )
    connectionString =
      QString::fromLatin1( "(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(Host=%1)(Port=%2))"
                           "(CONNECT_DATA=(SID=%3)))" ).arg( hostname ).arg( ( port > -1 ? port : 1521 ) ).arg( db );

  r = OCIHandleAlloc( d->env, reinterpret_cast<void **>( &d->srvhp ), OCI_HTYPE_SERVER, 0, nullptr );
  if ( r == OCI_SUCCESS )
    r = OCIServerAttach( d->srvhp, d->err, reinterpret_cast<const OraText *>( connectionString.utf16() ),
                         connectionString.length() * sizeof( QChar ), OCI_DEFAULT );
  if ( r == OCI_SUCCESS || r == OCI_SUCCESS_WITH_INFO )
    r = OCIHandleAlloc( d->env, reinterpret_cast<void **>( &d->svc ), OCI_HTYPE_SVCCTX, 0, nullptr );
  if ( r == OCI_SUCCESS )
    r = OCIAttrSet( d->svc, OCI_HTYPE_SVCCTX, d->srvhp, 0, OCI_ATTR_SERVER, d->err );
  if ( r == OCI_SUCCESS )
    r = OCIHandleAlloc( d->env, reinterpret_cast<void **>( &d->authp ), OCI_HTYPE_SESSION, 0, nullptr );
  if ( r == OCI_SUCCESS )
    r = OCIAttrSet( d->authp, OCI_HTYPE_SESSION, const_cast<ushort *>( user.utf16() ),
                    user.length() * sizeof( QChar ), OCI_ATTR_USERNAME, d->err );
  if ( r == OCI_SUCCESS )
    r = OCIAttrSet( d->authp, OCI_HTYPE_SESSION, const_cast<ushort *>( password.utf16() ),
                    password.length() * sizeof( QChar ), OCI_ATTR_PASSWORD, d->err );

  OCITrans *trans = nullptr;
  if ( r == OCI_SUCCESS )
    r = OCIHandleAlloc( d->env, reinterpret_cast<void **>( &trans ), OCI_HTYPE_TRANS, 0, nullptr );
  if ( r == OCI_SUCCESS )
    r = OCIAttrSet( d->svc, OCI_HTYPE_SVCCTX, trans, 0, OCI_ATTR_TRANS, d->err );

  if ( r == OCI_SUCCESS )
  {
    if ( user.isEmpty() && password.isEmpty() )
      r = OCISessionBegin( d->svc, d->err, d->authp, OCI_CRED_EXT, OCI_DEFAULT );
    else
      r = OCISessionBegin( d->svc, d->err, d->authp, OCI_CRED_RDBMS, OCI_DEFAULT );
  }
  if ( r == OCI_SUCCESS || r == OCI_SUCCESS_WITH_INFO )
    r = OCIAttrSet( d->svc, OCI_HTYPE_SVCCTX, d->authp, 0, OCI_ATTR_SESSION, d->err );

  if ( r == OCI_SUCCESS || r == OCI_SUCCESS_WITH_INFO )
  {
    d->geometryTDO = d->tdo( "MDSYS.SDO_GEOMETRY" );
    if ( !d->geometryTDO )
    {
      qDebug() << "MDSYS.SDO_GEOMETRY TDO not found";
      r = OCI_INVALID_HANDLE;
    }
  }

  if ( r != OCI_SUCCESS )
  {
    setLastError( qMakeError( tr( "Unable to logon" ), QSqlError::ConnectionError, d->err ) );
    setOpenError( true );
    if ( d->authp )
    {
      OCISessionEnd( d->svc, d->err, d->authp, OCI_DEFAULT );
      OCIHandleFree( d->authp, OCI_HTYPE_SESSION );
    }
    d->authp = nullptr;
    if ( d->srvhp )
    {
      OCIServerDetach( d->srvhp, d->err, OCI_DEFAULT );
      OCIHandleFree( d->srvhp, OCI_HTYPE_SERVER );
    }
    d->srvhp = nullptr;
    return false;
  }

  // get server version
  char vertxt[512];
  r = OCIServerVersion( d->svc,
                        d->err,
                        reinterpret_cast<OraText *>( vertxt ),
                        sizeof( vertxt ),
                        OCI_HTYPE_SVCCTX );
  if ( r != OCI_SUCCESS )
  {
    qWarning( "QOCISpatialDriver::open: could not get Oracle server version." );
  }
  else
  {
    QString versionStr;
    versionStr = QString( reinterpret_cast<const QChar *>( vertxt ) );
    QRegularExpression vers( QLatin1String( "([0-9]+)\\.[0-9\\.]+[0-9]" ) );
    QRegularExpressionMatch match = vers.match( versionStr );
    if ( match.hasMatch() )
      d->serverVersion = match.captured( 1 ).toInt();
    if ( d->serverVersion == 0 )
      d->serverVersion = -1;
  }

  setOpen( true );
  setOpenError( false );
  d->user = user;

  return true;
}

void QOCISpatialDriver::close()
{
  ENTER
  Q_D( QOCISpatialDriver );
  if ( !isOpen() )
    return;

  OCISessionEnd( d->svc, d->err, d->authp, OCI_DEFAULT );
  OCIServerDetach( d->srvhp, d->err, OCI_DEFAULT );
  OCIHandleFree( d->authp, OCI_HTYPE_SESSION );
  d->authp = nullptr;
  OCIHandleFree( d->srvhp, OCI_HTYPE_SERVER );
  d->srvhp = nullptr;
  OCIHandleFree( d->svc, OCI_HTYPE_SVCCTX );
  d->svc = nullptr;
  setOpen( false );
  setOpenError( false );
}

QSqlResult *QOCISpatialDriver::createResult() const
{
  ENTER
  return new QOCISpatialResult( this );
}

bool QOCISpatialDriver::beginTransaction()
{
  ENTER
  Q_D( QOCISpatialDriver );
  if ( !isOpen() )
  {
    qWarning( "QOCISpatialDriver::beginTransaction: Database not open" );
    return false;
  }
  int r = OCITransStart( d->svc,
                         d->err,
                         2,
                         OCI_TRANS_READWRITE );
  if ( r == OCI_ERROR )
  {
    qOraWarning( "QOCISpatialDriver::beginTransaction: ", d->err );
    setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialDriver",
                              "Unable to begin transaction" ), QSqlError::TransactionError, d->err ) );
    return false;
  }
  d->transaction = true;
  return true;
}

bool QOCISpatialDriver::commitTransaction()
{
  ENTER
  Q_D( QOCISpatialDriver );
  if ( !isOpen() )
  {
    qWarning( "QOCISpatialDriver::commitTransaction: Database not open" );
    return false;
  }
  int r = OCITransCommit( d->svc,
                          d->err,
                          0 );
  if ( r == OCI_ERROR )
  {
    qOraWarning( "QOCISpatialDriver::commitTransaction:", d->err );
    setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialDriver",
                              "Unable to commit transaction" ), QSqlError::TransactionError, d->err ) );
    return false;
  }
  d->transaction = false;
  return true;
}

bool QOCISpatialDriver::rollbackTransaction()
{
  ENTER
  Q_D( QOCISpatialDriver );
  if ( !isOpen() )
  {
    qWarning( "QOCISpatialDriver::rollbackTransaction: Database not open" );
    return false;
  }
  int r = OCITransRollback( d->svc,
                            d->err,
                            0 );
  if ( r == OCI_ERROR )
  {
    qOraWarning( "QOCISpatialDriver::rollbackTransaction:", d->err );
    setLastError( qMakeError( QCoreApplication::translate( "QOCISpatialDriver",
                              "Unable to rollback transaction" ), QSqlError::TransactionError, d->err ) );
    return false;
  }
  d->transaction = false;
  return true;
}

enum Expression
{
  OrExpression,
  AndExpression
};

static QString make_where_clause( const QString &user, Expression e )
{
  static const char sysUsers[][8] =
  {
    "MDSYS",
    "LBACSYS",
    "SYS",
    "SYSTEM",
    "WKSYS",
    "CTXSYS",
    "WMSYS",
  };
  static const char joinC[][4] = { "or", "and" };
  static Q_CONSTEXPR QLatin1Char bang[] = { QLatin1Char( ' ' ), QLatin1Char( '!' ) };

  const QLatin1String join( joinC[e], -1 ); // -1: force strlen call

  QString result;
  result.reserve( sizeof sysUsers / sizeof * sysUsers *
                  // max-sizeof(owner != <sysuser> and )
                  ( 9 + sizeof * sysUsers + 5 ) );
  for ( const auto &sysUser : sysUsers )
  {
    const QLatin1String l1( sysUser, -1 ); // -1: force strlen call
    if ( l1 != user )
      result += QLatin1String( "owner " ) + bang[e] + QLatin1String( "= '" ) + l1 + QLatin1Char( ' ' ) + join + QLatin1Char( ' ' );
  }

  result.chop( join.size() + 2 ); // remove final " <join> "

  return result;
}

QStringList QOCISpatialDriver::tables( QSql::TableType type ) const
{
  ENTER
  Q_D( const QOCISpatialDriver );
  QStringList tl;

  QString user = d->user;
  if ( isIdentifierEscaped( user, QSqlDriver::TableName ) )
    user = stripDelimiters( user, QSqlDriver::TableName );
  else
    user = user.toUpper();

  if ( !isOpen() )
    return tl;

  QSqlQuery t( createResult() );
  t.setForwardOnly( true );
  if ( type & QSql::Tables )
  {
    const QLatin1String tableQuery( "select owner, table_name from all_tables where " );
    const QString where = make_where_clause( user, AndExpression );
    t.exec( tableQuery + where );
    while ( t.next() )
    {
      if ( t.value( 0 ).toString().toUpper() != user.toUpper() )
        tl.append( t.value( 0 ).toString() + QLatin1Char( '.' ) + t.value( 1 ).toString() );
      else
        tl.append( t.value( 1 ).toString() );
    }

    // list all table synonyms as well
    const QLatin1String synonymQuery( "select owner, synonym_name from all_synonyms where " );
    t.exec( synonymQuery + where );
    while ( t.next() )
    {
      if ( t.value( 0 ).toString() != d->user )
        tl.append( t.value( 0 ).toString() + QLatin1Char( '.' ) + t.value( 1 ).toString() );
      else
        tl.append( t.value( 1 ).toString() );
    }
  }
  if ( type & QSql::Views )
  {
    const QLatin1String query( "select owner, view_name from all_views where " );
    const QString where = make_where_clause( user, AndExpression );
    t.exec( query + where );
    while ( t.next() )
    {
      if ( t.value( 0 ).toString().toUpper() != d->user.toUpper() )
        tl.append( t.value( 0 ).toString() + QLatin1Char( '.' ) + t.value( 1 ).toString() );
      else
        tl.append( t.value( 1 ).toString() );
    }
  }
  if ( type & QSql::SystemTables )
  {
    t.exec( QLatin1String( "select table_name from dictionary" ) );
    while ( t.next() )
    {
      tl.append( t.value( 0 ).toString() );
    }
    const QLatin1String tableQuery( "select owner, table_name from all_tables where " );
    const QString where = make_where_clause( user, OrExpression );
    t.exec( tableQuery + where );
    while ( t.next() )
    {
      if ( t.value( 0 ).toString().toUpper() != user.toUpper() )
        tl.append( t.value( 0 ).toString() + QLatin1Char( '.' ) + t.value( 1 ).toString() );
      else
        tl.append( t.value( 1 ).toString() );
    }

    // list all table synonyms as well
    const QLatin1String synonymQuery( "select owner, synonym_name from all_synonyms where " );
    t.exec( synonymQuery + where );
    while ( t.next() )
    {
      if ( t.value( 0 ).toString() != d->user )
        tl.append( t.value( 0 ).toString() + QLatin1Char( '.' ) + t.value( 1 ).toString() );
      else
        tl.append( t.value( 1 ).toString() );
    }
  }
  return tl;
}

void qSplitTableAndOwner( const QString &tname, QString *tbl,
                          QString *owner )
{
  ENTER
  int i = tname.indexOf( QLatin1Char( '.' ) ); // prefixed with owner?
  if ( i != -1 )
  {
    *tbl = tname.right( tname.length() - i - 1 );
    *owner = tname.left( i );
  }
  else
  {
    *tbl = tname;
  }
}

QSqlRecord QOCISpatialDriver::record( const QString &tablename ) const
{
  ENTER
  Q_D( const QOCISpatialDriver );
  QSqlRecord fil;
  if ( !isOpen() )
    return fil;

  QSqlQuery t( createResult() );
  // using two separate queries for this is A LOT faster than using,
  // e.g., a sub-query on the sys.synonyms table
  QString stmt( QLatin1String( "select column_name, data_type, data_length, "
                               "data_precision, data_scale, nullable, data_default%1"
                               "from all_tab_columns a "
                               "where a.table_name=%2" ) );
  if ( d->serverVersion >= 9 )
    stmt = stmt.arg( QLatin1String( ", char_length " ) );
  else
    stmt = stmt.arg( QLatin1String( " " ) );
  bool buildRecordInfo = false;
  QString table, owner, tmpStmt;
  qSplitTableAndOwner( tablename, &table, &owner );

  if ( isIdentifierEscaped( table, QSqlDriver::TableName ) )
    table = stripDelimiters( table, QSqlDriver::TableName );
  else
    table = table.toUpper();

  tmpStmt = stmt.arg( QLatin1Char( '\'' ) + table + QLatin1Char( '\'' ) );
  if ( owner.isEmpty() )
  {
    owner = d->user;
  }

  if ( isIdentifierEscaped( owner, QSqlDriver::TableName ) )
    owner = stripDelimiters( owner, QSqlDriver::TableName );
  else
    owner = owner.toUpper();

  tmpStmt += QLatin1String( " and a.owner='" ) + owner + QLatin1Char( '\'' );
  t.setForwardOnly( true );
  t.exec( tmpStmt );
  if ( !t.next() ) // try and see if the tablename is a synonym
  {
    stmt = stmt + QLatin1String( " join all_synonyms b "
                                 "on a.owner=b.table_owner and a.table_name=b.table_name "
                                 "where b.owner='" ) + owner +
           QLatin1String( "' and b.synonym_name='" ) + table +
           QLatin1Char( '\'' );
    t.setForwardOnly( true );
    t.exec( stmt );
    if ( t.next() )
      buildRecordInfo = true;
  }
  else
  {
    buildRecordInfo = true;
  }
  QStringList keywords = QStringList()
                         << QLatin1String( "NUMBER" )
                         << QLatin1String( "FLOAT" )
                         << QLatin1String( "BINARY_FLOAT" )
                         << QLatin1String( "BINARY_DOUBLE" );
  if ( buildRecordInfo )
  {
    do
    {
      QMetaType::Type ty = qDecodeOCIType( t.value( 1 ).toString(), t.numericalPrecisionPolicy() );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      QSqlField f( t.value( 0 ).toString(), static_cast<QVariant::Type>( ty ) );
#else
      QSqlField f( t.value( 0 ).toString(), QMetaType( ty ) );
#endif
      f.setRequired( t.value( 5 ).toString() == QLatin1String( "N" ) );
      f.setPrecision( t.value( 4 ).toInt() );
      if ( d->serverVersion >= 9 && ( ty == QMetaType::Type::QString ) && !t.isNull( 3 ) && !keywords.contains( t.value( 1 ).toString() ) )
      {
        // Oracle9: data_length == size in bytes, char_length == amount of characters
        f.setLength( t.value( 7 ).toInt() );
      }
      else
      {
        f.setLength( t.value( t.isNull( 3 ) ? 2 : 3 ).toInt() );
      }
      f.setDefaultValue( t.value( 6 ) );
      fil.append( f );
    }
    while ( t.next() );
  }
  return fil;
}

QSqlIndex QOCISpatialDriver::primaryIndex( const QString &tablename ) const
{
  Q_D( const QOCISpatialDriver );
  QSqlIndex idx( tablename );
  if ( !isOpen() )
    return idx;
  QSqlQuery t( createResult() );
  QString stmt( QLatin1String( "select b.column_name, b.index_name, a.table_name, a.owner "
                               "from all_constraints a, all_ind_columns b "
                               "where a.constraint_type='P' "
                               "and b.index_name = a.constraint_name "
                               "and b.index_owner = a.owner" ) );

  bool buildIndex = false;
  QString table, owner, tmpStmt;
  qSplitTableAndOwner( tablename, &table, &owner );

  if ( isIdentifierEscaped( table, QSqlDriver::TableName ) )
    table = stripDelimiters( table, QSqlDriver::TableName );
  else
    table = table.toUpper();

  tmpStmt = stmt + QLatin1String( " and a.table_name='" ) + table + QLatin1Char( '\'' );
  if ( owner.isEmpty() )
  {
    owner = d->user;
  }

  if ( isIdentifierEscaped( owner, QSqlDriver::TableName ) )
    owner = stripDelimiters( owner, QSqlDriver::TableName );
  else
    owner = owner.toUpper();

  tmpStmt += QLatin1String( " and a.owner='" ) + owner + QLatin1Char( '\'' );
  t.setForwardOnly( true );
  t.exec( tmpStmt );

  if ( !t.next() )
  {
    stmt += QLatin1String( " and a.table_name=(select tname from sys.synonyms "
                           "where sname='" ) + table + QLatin1String( "' and creator=a.owner)" );
    t.setForwardOnly( true );
    t.exec( stmt );
    if ( t.next() )
    {
      owner = t.value( 3 ).toString();
      buildIndex = true;
    }
  }
  else
  {
    buildIndex = true;
  }
  if ( buildIndex )
  {
    QSqlQuery tt( createResult() );
    tt.setForwardOnly( true );
    idx.setName( t.value( 1 ).toString() );
    do
    {
      tt.exec( QLatin1String( "select data_type from all_tab_columns where table_name='" ) +
               t.value( 2 ).toString() + QLatin1String( "' and column_name='" ) +
               t.value( 0 ).toString() + QLatin1String( "' and owner='" ) +
               owner + QLatin1Char( '\'' ) );
      if ( !tt.next() )
      {
        return QSqlIndex();
      }
      QMetaType::Type ty = qDecodeOCIType( tt.value( 0 ).toString(), t.numericalPrecisionPolicy() );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      QSqlField f( t.value( 0 ).toString(), static_cast<QVariant::Type>( ty ) );
#else
      QSqlField f( t.value( 0 ).toString(), QMetaType( ty ) );
#endif
      idx.append( f );
    }
    while ( t.next() );
    return idx;
  }
  return QSqlIndex();
}

QString QOCISpatialDriver::formatValue( const QSqlField &field, bool trimStrings ) const
{
  ENTER
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  switch ( static_cast<QMetaType::Type>( field.type() ) )
#else
  switch ( field.metaType().id() )
#endif
  {
    case QMetaType::Type::QDateTime:
    {
      QDateTime datetime = field.value().toDateTime();
      QString datestring;
      if ( datetime.isValid() )
      {
        datestring = QLatin1String( "TO_DATE('" ) + QString::number( datetime.date().year() )
                     + QLatin1Char( '-' )
                     + QString::number( datetime.date().month() ) + QLatin1Char( '-' )
                     + QString::number( datetime.date().day() ) + QLatin1Char( ' ' )
                     + QString::number( datetime.time().hour() ) + QLatin1Char( ':' )
                     + QString::number( datetime.time().minute() ) + QLatin1Char( ':' )
                     + QString::number( datetime.time().second() )
                     + QLatin1String( "','YYYY-MM-DD HH24:MI:SS')" );
      }
      else
      {
        datestring = QLatin1String( "NULL" );
      }
      return datestring;
    }
    case QMetaType::Type::QTime:
    {
      QDateTime datetime = field.value().toDateTime();
      QString datestring;
      if ( datetime.isValid() )
      {
        datestring = QLatin1String( "TO_DATE('" )
                     + QString::number( datetime.time().hour() ) + QLatin1Char( ':' )
                     + QString::number( datetime.time().minute() ) + QLatin1Char( ':' )
                     + QString::number( datetime.time().second() )
                     + QLatin1String( "','HH24:MI:SS')" );
      }
      else
      {
        datestring = QLatin1String( "NULL" );
      }
      return datestring;
    }
    case QMetaType::Type::QDate:
    {
      QDate date = field.value().toDate();
      QString datestring;
      if ( date.isValid() )
      {
        datestring = QLatin1String( "TO_DATE('" ) + QString::number( date.year() ) +
                     QLatin1Char( '-' ) +
                     QString::number( date.month() ) + QLatin1Char( '-' ) +
                     QString::number( date.day() ) + QLatin1String( "','YYYY-MM-DD')" );
      }
      else
      {
        datestring = QLatin1String( "NULL" );
      }
      return datestring;
    }
    default:
      break;
  }
  return QSqlDriver::formatValue( field, trimStrings );
}

QVariant QOCISpatialDriver::handle() const
{
  ENTER
  Q_D( const QOCISpatialDriver );
  return QVariant::fromValue( d->env );
}

QString QOCISpatialDriver::escapeIdentifier( const QString &identifier, IdentifierType type ) const
{
  ENTER
  QString res = identifier;
  if ( !identifier.isEmpty() && !isIdentifierEscaped( identifier, type ) )
  {
    res.replace( QLatin1Char( '"' ), QLatin1String( "\"\"" ) );
    res.prepend( QLatin1Char( '"' ) ).append( QLatin1Char( '"' ) );
    res.replace( QLatin1Char( '.' ), QLatin1String( "\".\"" ) );
  }
  return res;
}

QT_END_NAMESPACE

// vim: set sw=4 expandtab :
