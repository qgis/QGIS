/***************************************************************************
   qgshanautils.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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
#include "qgsfields.h"
#include <qgswkbtypes.h>
#include <qstring.h>
#include <QVariant>
#include <QVersionNumber>
#include <string.h>

#include "odbc/Types.h"

class QgsHanaUtils
{
  public:
    QgsHanaUtils() = delete;

    static QString connectionInfo(const QgsDataSourceUri& uri);

    static QString quotedIdentifier(const QString& str);
    static QString quotedString(const QString &str);
    static QString quotedValue(const QVariant &value);

    static QString toQString(const odbc::String& str);
    static QVariant toVariant(const odbc::Byte& value);
    static QVariant toVariant(const odbc::UByte& value);
    static QVariant toVariant(const odbc::Short& value);
    static QVariant toVariant(const odbc::UShort& value);
    static QVariant toVariant(const odbc::Long& value);
    static QVariant toVariant(const odbc::ULong& value);
    static QVariant toVariant(const odbc::Date& value);
    static QVariant toVariant(const odbc::Time& value);
    static QVariant toVariant(const odbc::Timestamp& value);
    static QVariant toVariant(const odbc::String& value);
    static QVariant toVariant(const odbc::String& value, int type);
    static QVariant toVariant(const odbc::NString& value);
    static QVariant toVariant(const odbc::Binary& value);
    template<typename T>
    static QVariant toVariant(const odbc::Nullable<T>& value, QVariant::Type nullType)
    {
      if (value.isNull())
        return QVariant(nullType);
      else
        return QVariant(*value);
    }
    static QByteArray toQByteArray(const odbc::Binary& value);

    static QgsWkbTypes::Type toWkbType(const QString& hanaType);
    static QVersionNumber toHANAVersion(const QString& dbVersion);
    static int toPlanarSRID(int srid);
    static bool convertField(QgsField &field);
    static int countFieldsInUppercase(const QgsFields &fields);
    static QString formatErrorMessage(const char* message, bool withPrefix = true);
};

#endif // QGSHANAUTILS_H
