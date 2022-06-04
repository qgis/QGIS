/***************************************************************************
   qgshanautils.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANAUTILS_H
#define QGSHANAUTILS_H

#include "qgsdatasourceuri.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include <qgsunittypes.h>
#include <qgswkbtypes.h>
#include <qstring.h>

#include <QVariant>
#include <QVersionNumber>

#include "odbc/Types.h"

class QgsHanaUtils
{
  public:
    QgsHanaUtils() = delete;

    static QString connectionInfo( const QgsDataSourceUri &uri );

    static QString quotedIdentifier( const QString &str );
    static QString quotedString( const QString &str );
    static QString quotedValue( const QVariant &value );

    static QString toConstant( const QVariant &value, QVariant::Type type );

    static QString toString( QgsUnitTypes::DistanceUnit unit );

    static QString toQString( const NS_ODBC::NString &str );
    static QString toQString( const NS_ODBC::String &str );
    static QVariant toVariant( const NS_ODBC::Boolean &value );
    static QVariant toVariant( const NS_ODBC::Byte &value );
    static QVariant toVariant( const NS_ODBC::UByte &value );
    static QVariant toVariant( const NS_ODBC::Short &value );
    static QVariant toVariant( const NS_ODBC::UShort &value );
    static QVariant toVariant( const NS_ODBC::Int &value );
    static QVariant toVariant( const NS_ODBC::UInt &value );
    static QVariant toVariant( const NS_ODBC::Long &value );
    static QVariant toVariant( const NS_ODBC::ULong &value );
    static QVariant toVariant( const NS_ODBC::Float &value );
    static QVariant toVariant( const NS_ODBC::Double &value );
    static QVariant toVariant( const NS_ODBC::Date &value );
    static QVariant toVariant( const NS_ODBC::Time &value );
    static QVariant toVariant( const NS_ODBC::Timestamp &value );
    static QVariant toVariant( const NS_ODBC::String &value );
    static QVariant toVariant( const NS_ODBC::NString &value );
    static QVariant toVariant( const NS_ODBC::Binary &value );

    static const char16_t *toUtf16( const QString &sql );
    static bool isGeometryTypeSupported( QgsWkbTypes::Type wkbType );
    static QgsWkbTypes::Type toWkbType( const NS_ODBC::String &type, const NS_ODBC::Int &hasZ, const NS_ODBC::Int &hasM );
    static QVersionNumber toHANAVersion( const QString &dbVersion );
    static int toPlanarSRID( int srid );
    static bool convertField( QgsField &field );
    static int countFieldsWithFirstLetterInUppercase( const QgsFields &fields );
    static QString formatErrorMessage( const char *message, bool withPrefix = false );
};

#endif // QGSHANAUTILS_H
