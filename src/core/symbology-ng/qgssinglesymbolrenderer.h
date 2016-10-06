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

#include "qgis.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"
#include "qgsexpression.h"
#include <QScopedPointer>

/** \ingroup core
 * \class QgsSingleSymbolRenderer
 */
class CORE_EXPORT QgsSingleSymbolRenderer : public QgsFeatureRenderer
{
  public:

    QgsSingleSymbolRenderer( QgsSymbol* symbol );

    virtual ~QgsSingleSymbolRenderer();

    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;
    virtual void stopRender( QgsRenderContext& context ) override;
    virtual QSet<QString> usedAttributes() const override;

    QgsSymbol* symbol() const;
    void setSymbol( QgsSymbol* s );

    virtual QString dump() const override;

    virtual QgsSingleSymbolRenderer* clone() const override;

    virtual void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props = QgsStringMap() ) const override;
    static QgsFeatureRenderer* createFromSld( QDomElement& element, QgsWkbTypes::GeometryType geomType );

    virtual Capabilities capabilities() override { return SymbolLevels; }
    virtual QgsSymbolList symbols( QgsRenderContext& context ) override;

    //! create renderer from XML element
    static QgsFeatureRenderer* create( QDomElement& element );
    virtual QDomElement save( QDomDocument& doc ) override;
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize ) override;
    virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = QString() ) override;
    virtual QgsLegendSymbolListV2 legendSymbolItemsV2() const override;
    virtual QSet< QString > legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context ) override;
    virtual void setLegendSymbolItem( const QString& key, QgsSymbol* symbol ) override;

    //! creates a QgsSingleSymbolRenderer from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsSingleSymbolRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer );

  protected:
    QScopedPointer<QgsSymbol> mSymbol;

};


#endif // QGSSINGLESYMBOLRENDERERV2_H
