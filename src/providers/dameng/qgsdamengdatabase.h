/***************************************************************************
    qgsdamengdatabase.h
    ---------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGDATABASE_H
#define QGSDAMENGDATABASE_H

#include <qqueue.h>
#include <qsqldatabase.h>
#include <qsqlrecord.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QtSql/qsqldriver.h>
#include <QtSql/private/qsqldriver_p.h>
#include <QtSql/private/qsqlresult_p.h>

#if defined( Q_OS_WIN32 )
#include <QtCore/qt_windows.h>
#endif

extern "C"
{
#include <DPIext.h>
}

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_DM
#else
#define Q_EXPORT_SQLDRIVER_DM Q_SQL_EXPORT
#endif

typedef short sint;
typedef int lint;
typedef unsigned int ulint;
typedef unsigned short usint;
typedef signed char schar;
typedef unsigned char byte;

typedef ulint dmbool;
typedef lint dmcode_t;

#define UNDETERMINED_SRID -2
#define NDCT_IDCLS_PACKAGE 14
#define NDCT_PKGID_DMGEO2 ( NDCT_IDCLS_PACKAGE << 24 | 112 )
#define NDCT_CLSID_GEO2_ST_GEOMETRY ( NDCT_IDCLS_PACKAGE << 24 | 113 )
#define NDCT_CLSID_GEO2_ST_POINT ( NDCT_IDCLS_PACKAGE << 24 | 114 )
#define NDCT_CLSID_GEO2_ST_LINE ( NDCT_IDCLS_PACKAGE << 24 | 115 )
#define NDCT_CLSID_GEO2_ST_POLYGON ( NDCT_IDCLS_PACKAGE << 24 | 116 )
#define NDCT_CLSID_GEO2_ST_MULTIPOINT ( NDCT_IDCLS_PACKAGE << 24 | 117 )
#define NDCT_CLSID_GEO2_ST_MULTILINE ( NDCT_IDCLS_PACKAGE << 24 | 118 )
#define NDCT_CLSID_GEO2_ST_MULTIPOLYGON ( NDCT_IDCLS_PACKAGE << 24 | 119 )
#define NDCT_CLSID_GEO2_ST_COLLECTION ( NDCT_IDCLS_PACKAGE << 24 | 120 )
#define NDCT_CLSID_GEO2_ST_CIRCSTRING ( NDCT_IDCLS_PACKAGE << 24 | 121 )
#define NDCT_CLSID_GEO2_ST_COMPOUND ( NDCT_IDCLS_PACKAGE << 24 | 122 )
#define NDCT_CLSID_GEO2_ST_CURVEPOLY ( NDCT_IDCLS_PACKAGE << 24 | 123 )
#define NDCT_CLSID_GEO2_ST_MULTICURVE ( NDCT_IDCLS_PACKAGE << 24 | 124 )
#define NDCT_CLSID_GEO2_ST_MULTISURFACE ( NDCT_IDCLS_PACKAGE << 24 | 125 )
#define NDCT_CLSID_GEO2_ST_POLYHEDRALSURFACE ( NDCT_IDCLS_PACKAGE << 24 | 126 )
#define NDCT_CLSID_GEO2_ST_TRIANGLE ( NDCT_IDCLS_PACKAGE << 24 | 127 )
#define NDCT_CLSID_GEO2_ST_TIN ( NDCT_IDCLS_PACKAGE << 24 | 128 )
#define NDCT_CLSID_GEO2_ST_GEOGRAPHY ( NDCT_IDCLS_PACKAGE << 24 | 129 )


QT_BEGIN_NAMESPACE
struct DMBatchColumn
{
    inline DMBatchColumn()
      : maxLen( 0 ), recordCount( 0 ), data( 0 ), indicators( 0 ) {}

    slength maxLen;
    udint4 recordCount;
    char *data;
    slength *indicators;
};

struct DMBatchCleanupHandler
{
    inline DMBatchCleanupHandler( QVector<DMBatchColumn> &columns )
      : col( columns ) {}

    ~DMBatchCleanupHandler()
    {
      // deleting storage, length and indicator arrays
      for ( int j = 0; j < col.count(); ++j )
      {
        delete[] col[j].indicators;
        delete[] col[j].data;
      }
    }
    QVector<DMBatchColumn> &col;
};

class QgsDMDriverPrivate;
class QSqlDatabase;
class QSqlQuery;
class QSqlRecord;

//*********************************************
//! Spatial column types
enum QgsDamengGeometryColumnType
{
  SctNone,
  SctGeometry,
  SctGeography,
  SctTopoGeometry,
  //SctPcPatch,
  SctRaster
};

enum QgsDamengPrimaryKeyType
{
  PktUnknown,
  PktInt,
  PktInt64,
  PktUint64,
  PktRowId,
  PktFidMap
};

typedef enum
{
  DmResEmptyQuery = 0, /* empty query string was executed */
  DmResCommandOk,      /* a query command that doesn't return
                             * anything was executed properly by the
                             * backend */
  DmResSuccessInfo,    /* a query command with a warning message */
  DmResFatalError,     /* query failed */
  DmResInvalidHandle,  /* a query command with invalid handle */
  DmResNeedData        /* a query command that need data */
} ExecStatusType;

typedef struct DmObj
{
    dhobj obj;
    dhobjdesc objDesc;
    dhdesc hdescCol;
    slength length = 0;
} DmObj_t;

//*********************************************

class QgsDMDriver : public QSqlDriver
{
    Q_DECLARE_PRIVATE( QgsDMDriver )
    Q_OBJECT
    friend class QgsDMResultPrivate;

  public:
    explicit QgsDMDriver( QObject *parent = 0 );
    QgsDMDriver( dhandle env, dhandle con, QObject *parent = 0 );
    virtual ~QgsDMDriver();
    bool hasFeature( DriverFeature f ) const override;
    void close() override;
    QSqlResult *createResult() const override;
    QStringList tables( QSql::TableType ) const override;
    QSqlRecord record( const QString &tablename ) const override;
    QSqlIndex primaryIndex( const QString &tablename ) const override;
    QVariant handle() const override;
    QString formatValue( const QSqlField &field, bool trimStrings ) const override;
    bool open( const QString &dbTrans, const QString &user, const QString &password, const QString &host, int port, const QString &connOpts ) override;
    QString &getConnMsg() { return connMsg; }
    void setConnMsg( const QString &msg ) { connMsg = msg; }
    bool isConnect();

    QString escapeIdentifier( const QString &identifier, IdentifierType type ) const override;

    bool isIdentifierEscaped( const QString &identifier, IdentifierType type ) const override;

    bool beginTransaction() override;
    bool commitTransaction() override;
    bool rollbackTransaction() override;

  private:
    bool endTrans();
    void cleanup();
    QString connMsg;
};


static const int COLNAME_SIZE = 256;
static const sdint2 TABLENAME_SIZE = 256;
static const sdint2 qParamType[4] = { DSQL_PARAM_INPUT, DSQL_PARAM_INPUT, DSQL_PARAM_OUTPUT, DSQL_PARAM_INPUT_OUTPUT };

class QgsDMDriverPrivate : public QSqlDriverPrivate
{
    Q_DECLARE_PUBLIC( QgsDMDriver )

  public:
    QgsDMDriverPrivate()
      : QSqlDriverPrivate(), hEnv( 0 ), hDbc( 0 ), disconnectCount( 0 )
    {
    }

    dhenv hEnv;
    dhcon hDbc;

    int disconnectCount;

    bool setConnectionOptions( const QString &connOpts );
    void splitTableQualifier( const QString &qualifier, QString &schema, QString &table );
};

class QgsDMResultPrivate;

class QgsDMResult : public QSqlResult
{
    Q_DECLARE_PRIVATE( QgsDMResult )

  public:
    QgsDMResult( const QgsDMDriver *db );
    virtual ~QgsDMResult();

    static QSqlField qMakeFieldInfo( const dhstmt hStmt, const QgsDMDriverPrivate *p );
    QSqlField qMakeFieldInfo( const QgsDMResultPrivate *p, int i );
    QSqlField qMakeFieldInfo( const dhstmt hStmt, int i, QString *errorMessage );

    void setForwardOnly( bool forward );
    bool prepare( const QString &query ) override;
    bool exec() override;
    int execstatus();

    dhstmt *getStmt();
    QString &getMsg() { return mResMsg; }
    void setResStatus();
    ExecStatusType &getResStatus() { return mResultStatus; }

    void qGetClassData( dhobj &obj, dhobjdesc &objDesc, QString &res, bool &isNull );
    QString qGetClassData( QQueue<DmObj *> &obj_data, int &field, int &sql_type );

    bool fetchBinary();
    void getBinarydata( int field, byte *&data, slength &size );

    QVariant lastInsertId() const override;
    QVariant handle() const override;
    slength getLength( int field, int type );
    QVariant data( int field ) override;
    QVariant value( int field );
    bool isNull( int field ) override;
    void finish();

    bool fetch( int i ) override;
    bool fetchFirst() override;
    bool fetchNext() override;
    bool fetchScroll();
    bool fetchLast() override;
    QSqlRecord record() const override;

    bool nextResult() override;
    int size() override;

    int nfields();
    int ntuples();
    void setNtuples();
    sdint4 ftype( int col );
    QString getSqlTypeName( int col, sdint4 type );
    QString fname( int col );
    QString ftableName( int col );
    sdint4 ftable( QString schemaName, QString tableName );
    sdint4 ftable( int col );

    QgsDamengGeometryColumnType getGeoType( int field );
    QString getGeoTypeName( sdint2 geoType );
    QString getGeoSubTypeName( int field );
    static QString getGeoName( const uint &obj_classid );

  protected:
    bool fetchPrevious() override;
    bool reset( const QString &query ) override;
    int numRowsAffected() override;
    void virtual_hook( int id, void *data ) override;
    void detachFromResultSet() override;
    bool execBatch( bool arrayBind = false ) override;

  private:
    ExecStatusType mResultStatus = DmResEmptyQuery;
    sdint2 mColNum = 0;
    long long mNtups = 0;
    QVector<sdint4> mSqlType;
    QVector<QString> mTypeName;

    QMap<sdint2, sdint2> mGeoType; //<colid, type>
    QQueue<DmObj *> mObjData;

    QString mResMsg = "";
};

class QgsDMResultPrivate : public QSqlResultPrivate
{
    Q_DECLARE_PUBLIC( QgsDMResult )

  public:
    Q_DECLARE_SQLDRIVER_PRIVATE( QgsDMDriver )
    QgsDMResultPrivate( QgsDMResult *q, const QgsDMDriver *db )
      : QSqlResultPrivate( q, db ), hStmt( 0 )
    {
      disconnectCount = drv_d_func()->disconnectCount;
    }

    inline void clearValues()
    {
      fieldCache.fill( QVariant() );
      fieldCacheIdx = 0;

      for ( int i = 0; i < fieldCached.size(); ++i )
      {
        fieldCached[i] = false;
      }
    }

    dhenv dpEnv() const { return drv_d_func() ? drv_d_func()->hEnv : 0; }
    dhcon dpDbc() const { return drv_d_func() ? drv_d_func()->hDbc : 0; }
    dhstmt hStmt;
    DPIRETURN r;

    QSqlRecord rInf;
    QVector<QVariant> fieldCache;
    QVector<bool> fieldCached;
    int fieldCacheIdx;
    int disconnectCount;

    bool isStmtHandleValid();
    void updateStmtHandleState();
};

typedef struct DmConn_t DmConn;
struct DmConn_t
{
    bool connStatus;

    QgsDMDriver *dmDriver;
    QgsDMResult *dmResult;
};


QT_END_NAMESPACE

#endif // QGSDAMENGDATABASE_H
