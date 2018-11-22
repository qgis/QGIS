/***************************************************************************
  qgslistfieldformatter.h - QgsListFieldFormatter

 ---------------------
 begin                : 3.12.2016
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
#ifndef QGSLISTFIELDKIT_H
#define QGSLISTFIELDKIT_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"

/**
 * \ingroup core
 * Field formatter for a list field.
 * This represents a list type value.
 * Values will be represented as a comma-separated list.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsListFieldFormatter : public QgsFieldFormatter
{
  public:

    /**
      * Default constructor of field formatter for a list field.
      */
    QgsListFieldFormatter() = default;
    QString id() const override;

    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;
};

#endif // QGSLISTFIELDKIT_H
