/***************************************************************************
    sqlanystatement.h - Class for representing a SQL Anywhere statement
    ------------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
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
/* $Id$ */

#ifndef SQLANYSTATEMENT_H
#define SQLANYSTATEMENT_H

#include <QString>
#include <QVariant>
#include "qgsvectordataprovider.h"

extern "C"
{
#define _SACAPI_VERSION 2
#include "sacapi/sacapidll.h"
}

#include "sqlanyconnection.h"

/**
 * SqlAnyStatement
 * A class wrapping a a_sqlany_stmt object for convenient access
 */
class SACONN_EXPORT SqlAnyStatement
{
    friend class SqlAnyConnection; // allow it to call protected constructor

  public:
    ~SqlAnyStatement();

    bool isValid() const { return mHndl != NULL; }

    // Because errors are returned via a shared connection handle, multiple
    //  threads executing statements simultaneously may overwrite each
    //  other's error messages (avoiding this would require grabbing the
    //  connection mutex for every database access, which is too expensive).
    // As such, testing errCode() is not a safe method for identifying
    //  success; use the return value of the statement method instead.
    // errCode() and errMsg() provide only best-effort information
    //  in the case of a warning or error.
    sacapi_i32 errCode() const { return mErrCode; }
    QString errMsg() const { return QString::fromUtf8( mErrMsg ); }

    /**
     * functions for mapping types
     */
    const SqlAnyTypeMap     *typeMap() const { return mConn->typeMap(); }
    bool      containsType( a_sqlany_native_type t ) const { return mConn->containsType( t ); }
    QgsVectorDataProvider::NativeType mapType( a_sqlany_native_type t ) const { return mConn->mapType( t ); }

    /**
     * functions for manipulating the database statement
     */
    bool reset( );
    sacapi_i32 numParams( );
    bool describe_bind_param( sacapi_i32 index, a_sqlany_bind_param &param );
    bool bind_param( sacapi_i32 index, a_sqlany_bind_param &param );

    /**
     * functions for executing/fetching statements
     */
    bool execute( );
    bool fetchAbsolute( sacapi_i32 rownum );
    bool fetchNext( );

    /**
     * functions for retrieving result meta-data
     */
    sacapi_i32 numCols( );
    sacapi_i32 numRows( ); // n<0 means estimated to be |n|
    bool nextResultSet( );
    bool getColumnInfo( sacapi_u32 col_index, a_sqlany_column_info *buffer );
    bool getDataInfo( sacapi_u32 col_index, a_sqlany_data_info *buffer );

    /**
     * functions for retrieving result values
     */
    bool getColumn( sacapi_u32 col_index, a_sqlany_data_value *buffer );
    sacapi_i32 getData( sacapi_u32 col_index, size_t offset, void *buffer, size_t size );

    bool getByteArray( sacapi_u32 col_index, QByteArray &val );
    bool getString( sacapi_u32 col_index, QString &val );
    bool getDouble( sacapi_u32 col_index, double &val );
    bool getLongLong( sacapi_u32 col_index, long long &val );
    bool getULongLong( sacapi_u32 col_index, unsigned long long &val );
    bool getLong( sacapi_u32 col_index, long &val );
    bool getULong( sacapi_u32 col_index, unsigned long &val );
    bool getInt( sacapi_u32 col_index, int &val );
    bool getUInt( sacapi_u32 col_index, unsigned int &val );
    bool getShort( sacapi_u32 col_index, short &val );
    bool getUShort( sacapi_u32 col_index, unsigned short &val );
    bool getChar( sacapi_u32 col_index, char &val );
    bool getUChar( sacapi_u32 col_index, unsigned char &val );
    bool getBool( sacapi_u32 col_index, bool &val );
    bool getQVariant( sacapi_u32 col_index, QVariant &val );

  protected: // functions called by SqlAnyConnection
    SqlAnyStatement( SqlAnyConnection *conn );

  private:
    void getErrMsg() { mConn->getError( mErrCode, mErrMsg, sizeof( mErrMsg ) ); }
    SQLAnywhereInterface *api() { return mConn->api(); }

  protected: // members accessible to SqlAnyConnection
    a_sqlany_stmt * mHndl;
    sacapi_i32  mErrCode;
    char  mErrMsg[SACAPI_ERROR_SIZE];

  private:
    SqlAnyConnection * mConn;
};

#endif // SQLANYSTATEMENT_H
