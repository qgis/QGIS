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
 * \brief Field formatter for a date time field.
 *
 * This represents a date, time or datetime value based on
 * the field configuration.
 *
 */
class CORE_EXPORT QgsDateTimeFieldFormatter : public QgsFieldFormatter
{
  public:
    static const QString DATE_FORMAT; //! Date format was localized by applyLocaleChange before QGIS 3.30
    static const QString TIME_FORMAT;
    static const QString DATETIME_FORMAT; //! Date time format was localized by applyLocaleChange before QGIS 3.30
    static const QString QT_ISO_FORMAT;
    static const QString DISPLAY_FOR_ISO_FORMAT;
    static QString DATE_DISPLAY_FORMAT; //! Date display format is localized by applyLocaleChange \see applyLocaleChange \since QGIS 3.30
    static QString DATETIME_DISPLAY_FORMAT; //! Date time display format is localized by applyLocaleChange \see applyLocaleChange \since QGIS 3.30

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
    static QString defaultFormat( QMetaType::Type type );

    /**
     * Gets the default format in function of the type.
     * The type is expected to be one of
     *
     * - QVariant::DateTime
     * - QVariant::Date
     * - QVariant::Time
     *
     * \deprecated QGIS 3.38. Use the method with a QMetaType::Type argument instead.
     */
    Q_DECL_DEPRECATED static QString defaultFormat( QVariant::Type type ) SIP_DEPRECATED;

    /**
     * Gets the default display format in function of the type.
     * The type is expected to be one of
     *
     * - QVariant::DateTime
     * - QVariant::Date
     * - QVariant::Time
     *
     * \since QGIS 3.30
     */
    static QString defaultDisplayFormat( QMetaType::Type type );

    /**
     * Gets the default display format in function of the type.
     * The type is expected to be one of
     *
     * - QVariant::DateTime
     * - QVariant::Date
     * - QVariant::Time
     *
     * \since QGIS 3.30
     * \deprecated QGIS 3.38. Use the method with a QMetaType::Type argument instead.
     */
    Q_DECL_DEPRECATED static QString defaultDisplayFormat( QVariant::Type type ) SIP_DEPRECATED;

    /**
     * Adjusts the date time display formats according to locale.
     * Before QGIS 3.30, the date time formats was adjusted.
     *
     * \since QGIS 3.22.2
     */
    static void applyLocaleChange(); SIP_SKIP;
};

#endif // QGSDATETIMEFIELDKIT_H
