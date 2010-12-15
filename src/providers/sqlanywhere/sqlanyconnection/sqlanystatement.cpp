/***************************************************************************
    sqlanystatement.cpp - Class for representing a SQL Anywhere statement
    --------------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author		 : David DeHaan
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


#include <qgslogger.h>

#include "sqlanystatement.h"
#include "sqlanyconnection.h"


/** Statement constructor
 * Called by a connection prepare or execute direct
 */
SqlAnyStatement::SqlAnyStatement( SqlAnyConnection *conn )
    : mConn( conn )
    , mHndl( NULL )
    , mErrCode( 0 )
{
}

SqlAnyStatement::~SqlAnyStatement()
{
    if( isValid() ) { api()->sqlany_free_stmt( mHndl );  }
    mConn->release();
}

bool	    
SqlAnyStatement::reset( )
{
    if( !isValid() ) return false; 
    SaDebugMsg( "Resetting statement." );
    if( !api()->sqlany_reset( mHndl ) ) {
	getErrMsg();
	SaDebugMsg( QString( "Error code %1: %2" ) .arg( errCode() ) .arg( errMsg() ) );
	return false;
    }
    SaDebugMsg( "Reset successful." );
    return true;
}

sacapi_i32	
SqlAnyStatement::numParams( )
{ 
    if( !isValid() ) return -1; 
    sacapi_i32 retval = api()->sqlany_num_params( mHndl ); 
    if( retval < 0 ) { getErrMsg(); }
    return retval;
}

bool	
SqlAnyStatement::describe_bind_param( sacapi_i32 index, a_sqlany_bind_param &param )
{
    if( !isValid() ) return false; 
    if( !api()->sqlany_describe_bind_param( mHndl, index, &param ) ) {
	getErrMsg();
	return false;
    }
    return true;
}

bool	
SqlAnyStatement::bind_param( sacapi_i32 index, a_sqlany_bind_param &param )
{
    if( !isValid() ) return false; 
    if( !api()->sqlany_bind_param( mHndl, index, &param ) ) {
	getErrMsg();
	return false;
    }
    return true;
}

bool	    
SqlAnyStatement::execute( )
{
    if( !isValid() ) return false; 
    SaDebugMsg( "Executing prepared statement." );
    if( !api()->sqlany_execute( mHndl ) ) {
	getErrMsg();
	SaDebugMsg( QString( "Error code %1: %2" ) .arg( errCode() ) .arg( errMsg() ) );
	return false;
    }
    SaDebugMsg( "Executed successfully." );
    return true;
}


bool	    
SqlAnyStatement::fetchAbsolute( sacapi_i32 rownum )
{
    if( !isValid() ) return false; 
    if( !api()->sqlany_fetch_absolute( mHndl, rownum ) ) {
	getErrMsg();
	return false;
    }
    return true;
}

bool	    
SqlAnyStatement::fetchNext( )
{
    if( !isValid() ) return false; 
    if( !api()->sqlany_fetch_next( mHndl ) ) {
	getErrMsg();
	return false;
    }
    return true;
}

bool	    
SqlAnyStatement::getColumn( sacapi_u32 col_index, a_sqlany_data_value *buffer )
{
    if( !isValid() ) return false; 
    if( !api()->sqlany_get_column( mHndl, col_index, buffer ) ) {
	getErrMsg();
	return false;
    }
    return true;
}

bool	    
SqlAnyStatement::getColumnInfo( sacapi_u32 col_index, a_sqlany_column_info *buffer )
{
    if( !isValid() ) return false; 
    if( !api()->sqlany_get_column_info( mHndl, col_index, buffer ) ) {
	getErrMsg();
	return false;
    }
#if 0
    SaDebugMsg( QString( "Column info for index %1: name %2, type %3, native type %4, precision %5, scale %6, max size %7, nullable %8" )
	    .arg( col_index )
	    .arg( buffer->name )
	    .arg( buffer->type )
	    .arg( buffer->native_type )
	    .arg( buffer->precision )
	    .arg( buffer->scale )
	    .arg( buffer->max_size )
	    .arg( buffer->nullable ) );
#endif
    return true;
}

sacapi_i32	    
SqlAnyStatement::getData( sacapi_u32 col_index, size_t offset, void *buffer, size_t size )
{
    if( !isValid() ) return -1; 
    sacapi_i32 nbytes = api()->sqlany_get_data( mHndl, col_index, offset, buffer, size );
    if( nbytes < 0 ) { getErrMsg(); }
    return nbytes;
}

bool	    
SqlAnyStatement::getDataInfo( sacapi_u32 col_index, a_sqlany_data_info *buffer )
{
    if( !isValid() ) return false; 
    if( !api()->sqlany_get_data_info( mHndl, col_index, buffer ) ) {
	getErrMsg();
	return false;
    }
    return true;
}

sacapi_i32	    
SqlAnyStatement::numCols( ) 
{   // n<0 means estimated to be |n| 
    if( !isValid() ) return 0; 
    return api()->sqlany_num_cols( mHndl ); 
}

sacapi_i32	    
SqlAnyStatement::numRows( ) 
{   // n<0 means estimated to be |n| 
    if( !isValid() ) return 0; 
    return api()->sqlany_num_rows( mHndl ); 
}

bool
SqlAnyStatement::nextResultSet( )
{ 
    if( !isValid() ) return false; 
    if( !api()->sqlany_get_next_result( mHndl ) ) {
	getErrMsg();
	return false;
    }
    return true;
}

bool
SqlAnyStatement::getByteArray( sacapi_u32 col_index, QByteArray &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( value.type == A_BINARY )
	&& ( ! *(value.is_null) );
    if( ok ) {
	// this makes a deep copy of the buffer
	val = QByteArray( (char*)value.buffer, (int) *(value.length) );
    }
    return ok;
}

bool
SqlAnyStatement::getString( sacapi_u32 col_index, QString &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( value.type == A_STRING )
	&& ( ! *(value.is_null) );
    if( ok ) {
	// this makes a deep copy of the buffer
	val = QString::fromUtf8( (char*)value.buffer, (int) *(value.length) );
    }
    return ok;
}

bool
SqlAnyStatement::getDouble( sacapi_u32 col_index, double &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( value.type == A_DOUBLE )
	&& ( ! *(value.is_null) );
    if( ok ) {
	val = *(double*)value.buffer;
    }
    return ok;
}

bool
SqlAnyStatement::getLongLong( sacapi_u32 col_index, long long &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (long long) *(long long*)value.buffer; break;
	case A_UVAL64: val = (long long) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (long long) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (long long) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (long long) *(short*)value.buffer; break;
	case A_UVAL16: val = (long long) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (long long) *(char*)value.buffer; break;
	case A_UVAL8: val = (long long) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getULongLong( sacapi_u32 col_index, unsigned long long &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (unsigned long long) *(long long*)value.buffer; break;
	case A_UVAL64: val = (unsigned long long) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (unsigned long long) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (unsigned long long) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (unsigned long long) *(short*)value.buffer; break;
	case A_UVAL16: val = (unsigned long long) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (unsigned long long) *(char*)value.buffer; break;
	case A_UVAL8: val = (unsigned long long) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getLong( sacapi_u32 col_index, long &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (long) *(long long*)value.buffer; break;
	case A_UVAL64: val = (long) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (long) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (long) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (long) *(short*)value.buffer; break;
	case A_UVAL16: val = (long) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (long) *(char*)value.buffer; break;
	case A_UVAL8: val = (long) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getULong( sacapi_u32 col_index, unsigned long &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (unsigned long) *(long long*)value.buffer; break;
	case A_UVAL64: val = (unsigned long) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (unsigned long) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (unsigned long) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (unsigned long) *(short*)value.buffer; break;
	case A_UVAL16: val = (unsigned long) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (unsigned long) *(char*)value.buffer; break;
	case A_UVAL8: val = (unsigned long) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getInt( sacapi_u32 col_index, int &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (int) *(long long*)value.buffer; break;
	case A_UVAL64: val = (int) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (int) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (int) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (int) *(short*)value.buffer; break;
	case A_UVAL16: val = (int) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (int) *(char*)value.buffer; break;
	case A_UVAL8: val = (int) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getUInt( sacapi_u32 col_index, unsigned int &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (unsigned int) *(long long*)value.buffer; break;
	case A_UVAL64: val = (unsigned int) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (unsigned int) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (unsigned int) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (unsigned int) *(short*)value.buffer; break;
	case A_UVAL16: val = (unsigned int) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (unsigned int) *(char*)value.buffer; break;
	case A_UVAL8: val = (unsigned int) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getShort( sacapi_u32 col_index, short &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (short) *(long long*)value.buffer; break;
	case A_UVAL64: val = (short) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (short) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (short) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (short) *(short*)value.buffer; break;
	case A_UVAL16: val = (short) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (short) *(char*)value.buffer; break;
	case A_UVAL8: val = (short) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getUShort( sacapi_u32 col_index, unsigned short &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (unsigned short) *(long long*)value.buffer; break;
	case A_UVAL64: val = (unsigned short) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (unsigned short) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (unsigned short) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (unsigned short) *(short*)value.buffer; break;
	case A_UVAL16: val = (unsigned short) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (unsigned short) *(char*)value.buffer; break;
	case A_UVAL8: val = (unsigned short) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getChar( sacapi_u32 col_index, char &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (char) *(long long*)value.buffer; break;
	case A_UVAL64: val = (char) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (char) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (char) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (char) *(short*)value.buffer; break;
	case A_UVAL16: val = (char) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (char) *(char*)value.buffer; break;
	case A_UVAL8: val = (char) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getUChar( sacapi_u32 col_index, unsigned char &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (unsigned char) *(long long*)value.buffer; break;
	case A_UVAL64: val = (unsigned char) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (unsigned char) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (unsigned char) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (unsigned char) *(short*)value.buffer; break;
	case A_UVAL16: val = (unsigned char) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (unsigned char) *(char*)value.buffer; break;
	case A_UVAL8: val = (unsigned char) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getBool( sacapi_u32 col_index, bool &val )
{
    a_sqlany_data_value	value;
    bool		ok;

    ok = getColumn( col_index, &value ) 
	&& ( ! *(value.is_null) );
    if( ok ) {
	switch( value.type ) {
	case A_VAL64: val = (bool) *(long long*)value.buffer; break;
	case A_UVAL64: val = (bool) *(unsigned long long*)value.buffer; break;
	case A_VAL32: val = (bool) *(sacapi_i32*)value.buffer; break;
	case A_UVAL32: val = (bool) *(sacapi_u32*)value.buffer; break;
	case A_VAL16: val = (bool) *(short*)value.buffer; break;
	case A_UVAL16: val = (bool) *(unsigned short*)value.buffer; break;
	case A_VAL8: val = (bool) *(char*)value.buffer; break;
	case A_UVAL8: val = (bool) *(unsigned char*)value.buffer; break;
	default:
	    ok = false;
	}
    }
    return ok;
}

bool
SqlAnyStatement::getQVariant( sacapi_u32 col_index, QVariant &val )
{
    a_sqlany_column_info    cinfo;
    bool		ok;

    // map column's native type to QVariant::Type
    ok = getColumnInfo( col_index, &cinfo ) 
	    && containsType( cinfo.native_type );
    if( ok ) {
	switch( mapType( cinfo.native_type ).mType ) {

	case QVariant::ByteArray :
	    {
		QByteArray mybytearray;
		ok = getByteArray( col_index, mybytearray );
		val = QVariant( mybytearray );
	    }
	    break;

	case QVariant::String : 
	    {
		QString mystring;
		ok = getString( col_index, mystring );
		val = QVariant( mystring );
	    }
	    break;

	case QVariant::Double :
	    {
		double mydouble;
		ok = getDouble( col_index, mydouble );
		val = QVariant( mydouble );
	    }
	    break;

	case QVariant::Bool :
	    {
		bool mybool;
		ok = getBool( col_index, mybool );
		val = QVariant( mybool );
	    }
	    break;

	case QVariant::Int :
	    {
		int myint;
		ok = getInt( col_index, myint );
		val = QVariant( myint );
	    }
	    break;

	case QVariant::UInt :
	    {
		unsigned int myuint;
		ok = getUInt( col_index, myuint );
		val = QVariant( myuint );
	    }
	    break;

	case QVariant::LongLong :
	    {
		long long mylonglong;
		ok = getLongLong( col_index, mylonglong );
		val = QVariant( mylonglong );
	    }
	    break;

	case QVariant::ULongLong :
	    {
		unsigned long long myulonglong;
		ok = getULongLong( col_index, myulonglong );
		val = QVariant( myulonglong );
	    }
	    break;

	default:
	    ok = false;
	}
    }

    if( !ok ) {
	val = QVariant( QString::null );
    }

    return ok;
}

