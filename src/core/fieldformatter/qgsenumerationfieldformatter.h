/***************************************************************************
    qgsenumerationfieldformatter.h
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSENUMERATIONFIELDFORMATTER_H
#define QGSENUMERATIONFIELDFORMATTER_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"

/**
 * \ingroup core
 * \brief Field formatter for an Enumeration field.
 *
 * Enumeration values can be either list of values (eg. postgres ENUM type)
 * or code-value pairs (eg. gpkg field domains)
 *
 * A map of code-value pairs is used in both cases, with values==codes in case only
 * values are provided (eg. postgres ENUM type)
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEnumerationFieldFormatter : public QgsFieldFormatter
{
  public:
    /**
      * Default constructor of field formatter provider supplied enumerated values.
      */
    QgsEnumerationFieldFormatter();

    QString id() const override;

    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const override;
};

#endif // QGSENUMERATIONFIELDFORMATTER_H
