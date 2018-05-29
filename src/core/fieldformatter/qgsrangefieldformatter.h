/***************************************************************************
  qgsrangefieldformatter.h - QgsRangeFieldFormatter

 ---------------------
 begin                : 01/02/2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRANGEFIELDFORMATTER_H
#define QGSRANGEFIELDFORMATTER_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"

/**
 * \ingroup core
 * Field formatter for a range (double) field with precision and locale
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsRangeFieldFormatter : public QgsFieldFormatter
{
  public:

    /**
      * Default constructor of field formatter for a range (double)field.
      */
    QgsRangeFieldFormatter() = default;

    QString id() const override;

    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

};

#endif // QGSRANGEFIELDFORMATTER_H
