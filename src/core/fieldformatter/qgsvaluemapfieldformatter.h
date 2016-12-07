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

#include "qgsfieldformatter.h"

class CORE_EXPORT QgsValueMapFieldFormatter : public QgsFieldFormatter
{
  public:

    /**
     * Will be saved in the configuration when a value is NULL.
     * It's the magic UUID {2839923C-8B7D-419E-B84B-CA2FE9B80EC7}
     */
    static const QString NullValue;

    virtual QString id() const override;

    virtual QString representValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const override;

    virtual QVariant sortValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const override;
};

#endif // QGSVALUEMAPFIELDKIT_H
