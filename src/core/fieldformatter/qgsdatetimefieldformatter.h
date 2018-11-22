/***************************************************************************
  qgsdatetimefieldformatter.h - QgsDateTimeFieldFormatter

 ---------------------
 begin                : 2.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATETIMEFIELDKIT_H
#define QGSDATETIMEFIELDKIT_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"

/**
 * \ingroup core
 * Field formatter for a date time field.
 * This represents a date, time or datetime value based on
 * the field configuration.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsDateTimeFieldFormatter : public QgsFieldFormatter
{
  public:
    static const QString DATE_FORMAT;
    static const QString TIME_FORMAT;
    static const QString DATETIME_FORMAT;
    static const QString QT_ISO_FORMAT;
    static const QString DISPLAY_FOR_ISO_FORMAT;

    /**
      * Default constructor of field formatter for a date time field.
      */
    QgsDateTimeFieldFormatter() = default;

    QString id() const override;

    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    /**
     * Gets the default format in function of the type.
     * The type is expected to be one of
     *
     * - QVariant::DateTime
     * - QVariant::Date
     * - QVariant::Time
     */
    static QString defaultFormat( QVariant::Type type );
};

#endif // QGSDATETIMEFIELDKIT_H
