/***************************************************************************
  qgskeyvaluefieldformatter.h - QgsKeyValueFieldFormatter

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
#ifndef QGSKEYVALUEFIELDKIT_H
#define QGSKEYVALUEFIELDKIT_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"

/**
 * \ingroup core
 * Field formatter for a key value field.
 * This represents a list type value.
 * Values will be represented as a colon-delimited and
 * comma-separated list.
 *
 * E.g. "color: yellow, amount: 5"
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsKeyValueFieldFormatter : public QgsFieldFormatter
{
  public:

    /**
      * Default constructor of field formatter for a key value field.
      */
    QgsKeyValueFieldFormatter() = default;
    QString id() const override;
    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;
};

#endif // QGSKEYVALUEFIELDKIT_H
