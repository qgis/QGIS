/***************************************************************************
  qgsdatetimefieldkit.h - QgsDateTimeFieldKit

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

#include "qgsfieldkit.h"

#define QGSDATETIMEEDIT_DATEFORMAT QStringLiteral( "yyyy-MM-dd" )
#define QGSDATETIMEEDIT_TIMEFORMAT QStringLiteral( "HH:mm:ss" )
#define QGSDATETIMEEDIT_DATETIMEFORMAT QStringLiteral( "yyyy-MM-dd HH:mm:ss" )

class QgsDateTimeFieldKit : public QgsFieldKit
{
  public:
    QgsDateTimeFieldKit();

    virtual bool supportsField( QgsVectorLayer* layer, int fieldIdx ) override;
    virtual QString representValue( QgsVectorLayer* layer, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const override;

    /**
     * Get the default format in function of the type
     * @param type the field type
     * @return the date/time format
     */
    static QString defaultFormat( const QVariant::Type type );
};

#endif // QGSDATETIMEFIELDKIT_H
