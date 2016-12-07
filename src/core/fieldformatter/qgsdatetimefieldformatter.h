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

#include "qgsfieldformatter.h"

class CORE_EXPORT QgsDateTimeFieldFormatter : public QgsFieldFormatter
{
  public:
    static const QString DefaultDateFormat;
    static const QString DefaultTimeFormat;
    static const QString DefaultDateTimeFormat;

    virtual QString id() const override;

    virtual QString representValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const override;

    /**
     * Get the default format in function of the type.
     * The type is expected to be one of
     *
     * - QVariant::DateTime
     * - QVariant::Date
     * - QVariant::Time
     */
    static QString defaultFormat( const QVariant::Type type );
};

#endif // QGSDATETIMEFIELDKIT_H
