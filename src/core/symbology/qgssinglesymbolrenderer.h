/***************************************************************************
    qgssinglesymbolrenderer.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSINGLESYMBOLRENDERERV2_H
#define QGSSINGLESYMBOLRENDERERV2_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"
#include "qgsexpression.h"
#include "qgsdatadefinedsizelegend.h"

/**
 * \ingroup core
 * \class QgsSingleSymbolRenderer
 */
class CORE_EXPORT QgsSingleSymbolRenderer : public QgsFeatureRenderer
{
  public:

    QgsSingleSymbolRenderer( QgsSymbol *symbol SIP_TRANSFER );

    virtual QgsSymbol *symbolForFeature( QgsFeature &feature, QgsRenderContext &context ) override;
    virtual QgsSymbol *originalSymbolForFeature( QgsFeature &feature, QgsRenderContext &context ) override;
    virtual void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    virtual void stopRender( QgsRenderContext &context ) override;
    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;

    QgsSymbol *symbol() const;
    void setSymbol( QgsSymbol *s SIP_TRANSFER );

    virtual QString dump() const override;

    virtual QgsSingleSymbolRenderer *clone() const override SIP_FACTORY;

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const override;
    static QgsFeatureRenderer *createFromSld( QDomElement &element, QgsWkbTypes::GeometryType geomType );

    virtual QgsFeatureRenderer::Capabilities capabilities() override { return SymbolLevels; }
    virtual QgsSymbolList symbols( QgsRenderContext &context ) override;

    //! create renderer from XML element
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;
    virtual QgsLegendSymbolList legendSymbolItems() const override;
    virtual QSet< QString > legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context ) override;
    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol SIP_TRANSFER ) override;

    /**
     * creates a QgsSingleSymbolRenderer from an existing renderer.
     * \since QGIS 2.5
     * \returns a new renderer if the conversion was possible, otherwise 0.
     */
    static QgsSingleSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Configures appearance of legend when renderer is configured to use data-defined size for marker symbols.
     * This allows configuring which values (symbol sizes) should be shown in the legend, whether to display
     * different symbol sizes collapsed in one legend node or separated across multiple legend nodes etc.
     *
     * When renderer does not use data-defined size or does not use marker symbols, these settings will be ignored.
     * Takes ownership of the passed settings objects. Null pointer is a valid input that disables data-defined
     * size legend.
     * \since QGIS 3.0
     */
    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings SIP_TRANSFER );

    /**
     * Returns configuration of appearance of legend when using data-defined size for marker symbols.
     * Will return null if the functionality is disabled.
     * \since QGIS 3.0
     */
    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;

  protected:
    std::unique_ptr<QgsSymbol> mSymbol;
    std::unique_ptr<QgsDataDefinedSizeLegend> mDataDefinedSizeLegend;

  private:
#ifdef SIP_RUN
    QgsSingleSymbolRenderer( const QgsSingleSymbolRenderer & );
    QgsSingleSymbolRenderer &operator=( const QgsSingleSymbolRenderer & );
#endif

};


#endif // QGSSINGLESYMBOLRENDERERV2_H
