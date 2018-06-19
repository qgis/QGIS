/***************************************************************************
  qgsvaluemapfieldformatter.h - QgsValueMapFieldFormatter

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
#ifndef QGSVALUEMAPFIELDKIT_H
#define QGSVALUEMAPFIELDKIT_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"

/**
 * \ingroup core
 * Field formatter for a ValueMap field.
 * A value relation field formatter looks up the values a map.
 *
 * The map is defined in the configuration as dictionary under the key "map".
 *
 * { "map": { 1: "one", 2: "two", 3: "three" } }
 *
 * Values that are not on the map will be wrapped in parentheses. So with the above
 * configuration:
 *
 * - 3 => "three"
 * - 5 => "(5)"
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsValueMapFieldFormatter : public QgsFieldFormatter
{
  public:

    /**
     * Will be saved in the configuration when a value is NULL.
     * It's the magic UUID {2839923C-8B7D-419E-B84B-CA2FE9B80EC7}
     */
    static const QString NULL_VALUE;

    /**
      * Default constructor of field formatter for a value map field.
      */
    QgsValueMapFieldFormatter() = default;

    QString id() const override;

    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;
};

#endif // QGSVALUEMAPFIELDKIT_H
