/***************************************************************************
 qgsmasksymbollayer.h
 ---------------------
 begin                : July 2019
 copyright            : (C) 2019 by Hugo Mercier
 email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMASKSYMBOLLAYER_H
#define QGSMASKSYMBOLLAYER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbollayer.h"

class QgsPaintEffect;
class QgsMarkerSymbol;
class QgsSymbolLayerReference;

/**
 * \ingroup core
 * \class QgsMaskMarkerSymbolLayer
 * \brief Special symbol layer that uses its sub symbol as a selective mask
 * \since QGIS 3.12
 */

class CORE_EXPORT QgsMaskMarkerSymbolLayer : public QgsMarkerSymbolLayer
{
  public:
    //! Simple constructor
    QgsMaskMarkerSymbolLayer();

    ~QgsMaskMarkerSymbolLayer() override;

    /**
     * Create a new QgsMaskMarkerSymbolLayer
     *
     * \param properties A property map to deserialize saved information from properties()
     *
     * \returns A new QgsMaskMarkerSymbolLayer
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    QgsMaskMarkerSymbolLayer *clone() const override SIP_FACTORY;
    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;

    QVariantMap properties() const override;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPoint( QPointF point, QgsSymbolRenderContext &context ) override;
    QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) override;
    bool usesMapUnits() const override;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QColor color() const override;

    virtual void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size ) override;

    //! Whether some masked symbol layers are defined
    bool enabled() const;

    /**
     * Returns a list of references to symbol layers that are masked by the sub symbol's shape.
     * \returns a list of references to masked symbol layers
     * \see setMasks
     */
    QList<QgsSymbolLayerReference> masks() const override;

    /**
     * Sets the symbol layers that will be masked by the sub symbol's shape.
     * \param maskedLayers list of references to symbol layers
     * \see masks
     */
    void setMasks( const QList<QgsSymbolLayerReference> &maskedLayers );

  private:
#ifdef SIP_RUN
    QgsMaskMarkerSymbolLayer( const QgsMaskMarkerSymbolLayer & );
#endif

    //! Marker sub symbol
    std::unique_ptr<QgsMarkerSymbol> mSymbol;

    //! List of symbol layers that will be masked
    QList<QgsSymbolLayerReference> mMaskedSymbolLayers;

    std::unique_ptr<QgsPaintEffect> mEffect;
};

#endif


