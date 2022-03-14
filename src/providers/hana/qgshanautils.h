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

    static QString toQString( const qgs::odbc::NString &str );
    static QString toQString( const qgs::odbc::String &str );
    static QVariant toVariant( const qgs::odbc::Boolean &value );
    static QVariant toVariant( const qgs::odbc::Byte &value );
    static QVariant toVariant( const qgs::odbc::UByte &value );
    static QVariant toVariant( const qgs::odbc::Short &value );
    static QVariant toVariant( const qgs::odbc::UShort &value );
    static QVariant toVariant( const qgs::odbc::Int &value );
    static QVariant toVariant( const qgs::odbc::UInt &value );
    static QVariant toVariant( const qgs::odbc::Long &value );
    static QVariant toVariant( const qgs::odbc::ULong &value );
    static QVariant toVariant( const qgs::odbc::Float &value );
    static QVariant toVariant( const qgs::odbc::Double &value );
    static QVariant toVariant( const qgs::odbc::Date &value );
    static QVariant toVariant( const qgs::odbc::Time &value );
    static QVariant toVariant( const qgs::odbc::Timestamp &value );
    static QVariant toVariant( const qgs::odbc::String &value );
    static QVariant toVariant( const qgs::odbc::NString &value );
    static QVariant toVariant( const qgs::odbc::Binary &value );

    static const char16_t *toUtf16( const QString &sql );
    static QgsWkbTypes::Type toWkbType( const qgs::odbc::String &type, const qgs::odbc::Int &hasZ, const qgs::odbc::Int &hasM );
    static QVersionNumber toHANAVersion( const QString &dbVersion );
    static int toPlanarSRID( int srid );
    static bool convertField( QgsField &field );
    static int countFieldsWithFirstLetterInUppercase( const QgsFields &fields );
    static QString formatErrorMessage( const char *message, bool withPrefix = false );
};

#endif // QGSHANAUTILS_H
