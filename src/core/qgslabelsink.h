/***************************************************************************
                         qgslabelsink.h
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

#ifndef QGSLABELSINK_H
#define QGSLABELSINK_H

#define SIP_NO_FILE

#include "qgsvectorlayerlabelprovider.h"
#include "qgsrulebasedlabeling.h"

class QgsPalLayerSettings;
class QgsRuleBasedLabeling;

class QgsLabelSink
{
  public:
    virtual ~QgsLabelSink() = default;

    virtual void drawLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings ) = 0;
};

/**
 * \ingroup core
 * Implements a derived label provider internally used for DXF export
 *
 * Internal class, not in public API. Added in QGIS 2.12
 * \note not available in Python bindings
 */
class QgsLabelSinkProvider : public QgsVectorLayerLabelProvider
{
  public:
    //! construct the provider
    explicit QgsLabelSinkProvider( QgsVectorLayer *layer, const QString &providerId, QgsLabelSink *dxf, const QgsPalLayerSettings *settings );

    /**
     * Re-implementation that writes to DXF file instead of drawing with QPainter
     * \param context render context
     * \param label label
     */
    void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;

  private:
    //! pointer to parent DXF export where this instance is used
    QgsLabelSink *mLabelSink = nullptr;
};

/**
 * \ingroup core
 * Implements a derived label provider for rule based labels internally used
 * for DXF export
 *
 * Internal class, not in public API. Added in QGIS 2.15
 * \note not available in Python bindings
 */
class QgsRuleBasedLabelSinkProvider : public QgsRuleBasedLabelProvider
{
  public:
    //! construct the provider
    explicit QgsRuleBasedLabelSinkProvider( const QgsRuleBasedLabeling &rules, QgsVectorLayer *layer, QgsLabelSink *destination );

    /**
     * Reinitialize the subproviders with QgsLabelSinkProviders
     * \param layer layer
     * \deprecated since QGIS 3.12
     */
    Q_DECL_DEPRECATED void reinit( QgsVectorLayer *layer );

    /**
     * Re-implementation that writes to DXF file instead of drawing with QPainter
     * \param context render context
     * \param label label
     */
    void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;

    //! create QgsRuleBasedLabelSinkProvider
    QgsVectorLayerLabelProvider *createProvider( QgsVectorLayer *layer, const QString &providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings ) override;

  private:
    //! pointer to parent DXF export where this instance is used
    QgsLabelSink *mLabelSink = nullptr;
};



#endif // QGSLABELSINK_H
