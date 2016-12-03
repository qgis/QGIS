/***************************************************************************
  qgsfieldkit.h - QgsFieldKit

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
#ifndef QGSFIELDKIT_H
#define QGSFIELDKIT_H

#include <QString>
#include <QVariant>

class QgsVectorLayer;

class QgsFieldKit
{
  public:
    QgsFieldKit();

    virtual ~QgsFieldKit();

    virtual bool supportsField( QgsVectorLayer* layer, int fieldIdx );

    /**
     * Create a pretty String representation of the value.
     *
     * @return By default the string representation of the provided value as implied by the field definition is returned.
     */
    virtual QString representValue( QgsVectorLayer* layer, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const;

    /**
     * If the default sort order should be overwritten for this widget, you can transform the value in here.
     *
     * @return By default the value is returned unmodified.
     *
     * @note Added in 2.16
     */
    virtual QVariant sortValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const;

    /**
     * Return the alignment for a particular field. By default this will consider the field type but can be overwritten if mapped
     * values are represented.
     */
    virtual Qt::AlignmentFlag alignmentFlag( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config ) const;

    /**
     * Create a cache for a given field.
     */
    virtual QVariant createCache( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config ) const;
};

#endif // QGSFIELDKIT_H
