/***************************************************************************
                         qgsdxfpallabeling.h
                         -------------------
    begin                : January 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFPALLABELING_H
#define QGSDXFPALLABELING_H

#include "qgsmaprenderer.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayerlabelprovider.h"

class QgsDxfExport;


/** Implements a derived label provider internally used for DXF export
 *
 * Internal class, not in public API. Added in QGIS 2.12
 */
class QgsDxfLabelProvider : public QgsVectorLayerLabelProvider
{
  public:
    //! construct the provider
    explicit QgsDxfLabelProvider( QgsVectorLayer* layer, QgsDxfExport* dxf );

    //! re-implementation that writes to DXF file instead of drawing with QPainter
    virtual void drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const override;

    //! registration method that keeps track of DXF layer names of individual features
    void registerDxfFeature( QgsFeature& feature, const QgsRenderContext& context, const QString& dxfLayerName );

  protected:
    //! pointer to parent DXF export where this instance is used
    QgsDxfExport* mDxfExport;
    //! DXF layer name for each label feature
    QMap<QgsFeatureId, QString> mDxfLayerNames;
};

#endif // QGSDXFPALLABELING_H
