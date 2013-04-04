/***************************************************************************
    sqlanyconnection.h - Class for pooling connections to a SQL Anywhere DBMS
    ------------------------
    begin                : Dec 2010
    copyright            : (C) 2013 by SAP AG or an SAP affiliate company.
    author               : David DeHaan
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SQLANYCONNECTION_H
#define SQLANYCONNECTION_H

// #define DEBUG_RELEASES_VIA_WARNINGS
#ifdef DEBUG_RELEASES_VIA_WARNINGS
#define SaDebugMsg(str) qDebug(QString("%1(%2) : (%3) %4").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__).arg(str).toLocal8Bit().constData());
#else
#define SaDebugMsg(str) QgsDebugMsg(str)
#endif


extern "C"
{
#define _SACAPI_VERSION 2
#include "sacapi/sacapidll.h"
}

#include <QDebug>
#include <QString>
#include <QMap>
#include <QMutex>
#include <QMap>

#include <qgsvectordataprovider.h>
#include <qgsdatasourceuri.h>

class QgsDataSourceURI;
class SqlAnyStatement;

typedef QMap<a_sqlany_native_type, QgsVectorDataProvider::NativeType> SqlAnyTypeMap;

/**
 * SqlAnyConnection
 * A class wrapping a a_sqlany_connection object to allow it to
 * be used by multiple layers and to make it thread-safe.
 */
class SACONN_EXPORT SqlAnyConnection
{
    friend class SqlAnyStatement;

  public:
    // initializing API
    static bool   initApi();
    static void   releaseApi();
    static const char *  failedInitMsg();

    // connecting
    static SqlAnyConnection * connect( QString connName
                                       , QString host, QString port, QString server
                                       , QString database, QString parameters, QString username
                                       , QString password, bool simpleEncrypt, bool estimateMeta
                                       , bool readOnly );
    static SqlAnyConnection * connect( QString connName
                                       , QString host, QString port, QString server
                                       , QString database, QString parameters, QString username
                                       , QString password, bool simpleEncrypt, bool estimateMeta
                                       , bool readOnly, sacapi_i32 &code, char *errbuf, size_t size );
    static SqlAnyConnection * connect( const QString uri, bool readOnly );
    static SqlAnyConnection * connect( const QString uri, bool readOnly, sacapi_i32 &code, char *errbuf, size_t size );

    // misc
    QString   uri() { return mUri; }
    static QString  makeUri( QString connName
                             , QString host, QString port, QString server
                             , QString database, QString parameters, QString username
                             , QString password, bool simpleEncrypt, bool estimateMeta );
    bool   isAlive();
    SqlAnyConnection *  addRef();
    void   release();
    QString   serverVersion();

    /**
     * function for opening database statements
     */
    SqlAnyStatement *  prepare( QString sql ); // grabs+releases mMutex
    SqlAnyStatement *  execute_direct( QString sql ); // grabs+releases mMutex
    bool   execute_immediate( QString sql ); // grabs+releases mMutex
    bool   execute_immediate( QString sql, sacapi_i32 &code, char *errbuf, size_t size ); // grabs+releases mMutex
    void   getError( sacapi_i32 &code, char *errbuf, size_t size );

    /**
      * functions for transaction management
      */
    void    begin( ); // grabs mMutex
    bool    commit( ); // releases mMutex
    bool    commit( sacapi_i32 &code, char *errbuf, size_t size ); // releases mMutex
    bool    rollback( ); // releases mMutex
    bool    rollback( sacapi_i32 &code, char *errbuf, size_t size ); // releases mMutex

    /**
     * functions for mapping native types
     */
    const SqlAnyTypeMap     *typeMap() const { return & mNativeMap; }
    bool      containsType( a_sqlany_native_type t ) const { return mNativeMap.contains( t ); }
    QgsVectorDataProvider::NativeType mapType( a_sqlany_native_type t ) const;

  protected:
    // methods accessible to SqlAnyStatement.
    SQLAnywhereInterface    *api();

  protected:
    // members accessible to SqlAnyStatement.
    static QgsVectorDataProvider::NativeType sInvalidType;
    SqlAnyTypeMap mNativeMap;

  private:
    ~SqlAnyConnection();
    SqlAnyConnection( QString mapKey, const QString uri, a_sqlany_connection * handle, bool readOnly );

    void populateNativeMap();
    static QString makeConnectString( QgsDataSourceURI uri, bool includeUidPwd );

  private:
    QString  mMapKey;
    const QString mUri;
    const bool  mReadOnly;
    unsigned int mRefCount;
    a_sqlany_connection *mHandle;
    QMutex  mMutex;  // used to serialize statement preparations and read-write transactions

    static bool  sApiInit;
    static unsigned int connCount;
    static QMap< QString, SqlAnyConnection * > connCache;
    static QMutex gMutex; // used to serialize construction of connections
    static const char *sNotFoundMsg;
};

#endif // SQLANYCONNECTION_H
