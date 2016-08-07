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

Q_NOWARN_DEPRECATED_PUSH
/** \ingroup core
 * \class QgsSingleSymbolRenderer
 */
class CORE_EXPORT QgsSingleSymbolRenderer : public QgsFeatureRenderer
{
  public:

    QgsSingleSymbolRenderer( QgsSymbol* symbol );

    virtual ~QgsSingleSymbolRenderer();

    //! @note available in python as symbolForFeature2
    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    //! @note available in python as originalSymbolForFeature2
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;

    virtual void stopRender( QgsRenderContext& context ) override;

    virtual QList<QString> usedAttributes() override;

    QgsSymbol* symbol() const;
    void setSymbol( QgsSymbol* s );

    Q_DECL_DEPRECATED void setRotationField( const QString& fieldOrExpression ) override;
    Q_DECL_DEPRECATED QString rotationField() const override;

    void setSizeScaleField( const QString& fieldOrExpression );
    QString sizeScaleField() const;

    void setScaleMethod( QgsSymbol::ScaleMethod scaleMethod );
    QgsSymbol::ScaleMethod scaleMethod() const { return mScaleMethod; }

    virtual QString dump() const override;

    virtual QgsSingleSymbolRenderer* clone() const override;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const override;
    static QgsFeatureRenderer* createFromSld( QDomElement& element, QgsWkbTypes::GeometryType geomType );

    //! returns bitwise OR-ed capabilities of the renderer
    virtual Capabilities capabilities() override { return SymbolLevels | RotationField; }

    //! @note available in python as symbol2
    virtual QgsSymbolList symbols( QgsRenderContext& context ) override;

    //! create renderer from XML element
    static QgsFeatureRenderer* create( QDomElement& element );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc ) override;

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize ) override;

    //! return a list of item text / symbol
    //! @note not available in python bindings
    virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = QString() ) override;

    //! Return a list of symbology items for the legend. Better choice than legendSymbolItems().
    //! @note added in 2.6
    virtual QgsLegendSymbolListV2 legendSymbolItemsV2() const override;

    virtual QSet< QString > legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    virtual void setLegendSymbolItem( const QString& key, QgsSymbol* symbol ) override;

    //! creates a QgsSingleSymbolRenderer from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsSingleSymbolRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer );

  protected:
    QScopedPointer<QgsSymbol> mSymbol;
    QScopedPointer<QgsExpression> mRotation;
    QScopedPointer<QgsExpression> mSizeScale;
    QgsSymbol::ScaleMethod mScaleMethod;

    // temporary stuff for rendering
    QScopedPointer<QgsSymbol> mTempSymbol;
    double mOrigSize;
};
Q_NOWARN_DEPRECATED_POP


#endif // QGSSINGLESYMBOLRENDERERV2_H
