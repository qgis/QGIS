/***************************************************************************
    qgssinglesymbolrendererv2.h
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
#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgsexpression.h"
#include <QScopedPointer>

Q_NOWARN_DEPRECATED_PUSH
/** \ingroup core
 * \class QgsSingleSymbolRendererV2
 */
class CORE_EXPORT QgsSingleSymbolRendererV2 : public QgsFeatureRendererV2
{
  public:

    QgsSingleSymbolRendererV2( QgsSymbolV2* symbol );

    virtual ~QgsSingleSymbolRendererV2();

    //! @note available in python as symbolForFeature2
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    //! @note available in python as originalSymbolForFeature2
    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;

    virtual void stopRender( QgsRenderContext& context ) override;

    virtual QList<QString> usedAttributes() override;

    QgsSymbolV2* symbol() const;
    void setSymbol( QgsSymbolV2* s );

    Q_DECL_DEPRECATED void setRotationField( const QString& fieldOrExpression ) override;
    Q_DECL_DEPRECATED QString rotationField() const override;

    void setSizeScaleField( const QString& fieldOrExpression );
    QString sizeScaleField() const;

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    QgsSymbolV2::ScaleMethod scaleMethod() const { return mScaleMethod; }

    virtual QString dump() const override;

    virtual QgsSingleSymbolRendererV2* clone() const override;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const override;
    static QgsFeatureRendererV2* createFromSld( QDomElement& element, QGis::GeometryType geomType );

    //! returns bitwise OR-ed capabilities of the renderer
    virtual int capabilities() override { return SymbolLevels | RotationField; }

    //! @note available in python as symbol2
    virtual QgsSymbolV2List symbols( QgsRenderContext& context ) override;

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element );

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

    virtual void setLegendSymbolItem( const QString& key, QgsSymbolV2* symbol ) override;

    //! creates a QgsSingleSymbolRendererV2 from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsSingleSymbolRendererV2* convertFromRenderer( const QgsFeatureRendererV2 *renderer );

  protected:
    QScopedPointer<QgsSymbolV2> mSymbol;
    QScopedPointer<QgsExpression> mRotation;
    QScopedPointer<QgsExpression> mSizeScale;
    QgsSymbolV2::ScaleMethod mScaleMethod;

    // temporary stuff for rendering
    QScopedPointer<QgsSymbolV2> mTempSymbol;
    double mOrigSize;
};
Q_NOWARN_DEPRECATED_POP


#endif // QGSSINGLESYMBOLRENDERERV2_H
