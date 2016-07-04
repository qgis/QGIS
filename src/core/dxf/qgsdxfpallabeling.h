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
#include "qgsrulebasedlabeling.h"

class QgsDxfExport;
class QgsPalLayerSettings;
class QgsRuleBasedLabeling;


/** \ingroup core
 * Implements a derived label provider internally used for DXF export
 *
 * Internal class, not in public API. Added in QGIS 2.12
 * @note not available in Python bindings
 */
class QgsDxfLabelProvider : public QgsVectorLayerLabelProvider
{
  public:
    //! construct the provider
    explicit QgsDxfLabelProvider( QgsVectorLayer* layer, const QString& providerId, QgsDxfExport* dxf, const QgsPalLayerSettings *settings );

    /** Re-implementation that writes to DXF file instead of drawing with QPainter
     * @param context render context
     * @param label label
     */
    void drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const override;

    /** Registration method that keeps track of DXF layer names of individual features
     * @param feature feature
     * @param context render context
     * @param dxfLayerName name of dxf layer
     */
    void registerDxfFeature( QgsFeature& feature, QgsRenderContext &context, const QString& dxfLayerName );

  protected:
    //! pointer to parent DXF export where this instance is used
    QgsDxfExport* mDxfExport;
};

/** \ingroup core
 * Implements a derived label provider for rule based labels internally used
 * for DXF export
 *
 * Internal class, not in public API. Added in QGIS 2.15
 * @note not available in Python bindings
 */
class QgsDxfRuleBasedLabelProvider : public QgsRuleBasedLabelProvider
{
  public:
    //! construct the provider
    explicit QgsDxfRuleBasedLabelProvider( const QgsRuleBasedLabeling &rules, QgsVectorLayer* layer, QgsDxfExport* dxf );

    /** Reinitialize the subproviders with QgsDxfLabelProviders
     * @param layer layer
     */
    void reinit( QgsVectorLayer* layer );

    /** Re-implementation that writes to DXF file instead of drawing with QPainter
     * @param context render context
     * @param label label
     */
    void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;

    /** Registration method that keeps track of DXF layer names of individual features
     * @param feature feature
     * @param context render context
     * @param dxfLayerName name of dxf layer
     */
    void registerDxfFeature( QgsFeature& feature, QgsRenderContext &context, const QString& dxfLayerName );

    //! create QgsDxfLabelProvider
    virtual QgsVectorLayerLabelProvider *createProvider( QgsVectorLayer *layer, const QString& providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings ) override;

  protected:
    //! pointer to parent DXF export where this instance is used
    QgsDxfExport* mDxfExport;
};



#endif // QGSDXFPALLABELING_H
