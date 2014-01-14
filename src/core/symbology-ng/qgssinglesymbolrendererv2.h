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
#include <memory>

class CORE_EXPORT QgsSingleSymbolRendererV2 : public QgsFeatureRendererV2
{
  public:

    QgsSingleSymbolRendererV2( QgsSymbolV2* symbol );
    QgsSingleSymbolRendererV2( const QgsSingleSymbolRendererV2 & );
    QgsSingleSymbolRendererV2 & operator=( QgsSingleSymbolRendererV2 other );


    virtual ~QgsSingleSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    QgsSymbolV2* symbol() const;
    void setSymbol( QgsSymbolV2* s );

    //! @note added in 1.5
    void setRotationField( QString expression )
    {
      mRotation.reset( expression.isEmpty() ? NULL : new QgsExpression( expression ) );
      Q_ASSERT( !mRotation.get() || !mRotation->hasParserError() );
    }
    //! @note added in 1.5
    QString rotationField() const { return mRotation.get() ? mRotation->expression() : ""; }

    //! @note added in 1.5
    void setSizeScaleField( QString expression )
    {
      mSizeScale.reset( expression.isEmpty() ? NULL : new QgsExpression( expression ) );
      Q_ASSERT( !mSizeScale.get() || !mSizeScale->hasParserError() );
    }
    //! @note added in 1.5
    QString sizeScaleField() const { return mSizeScale.get() ? mSizeScale->expression() : ""; }

    //! @note added in 2.0
    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    //! @note added in 2.0
    QgsSymbolV2::ScaleMethod scaleMethod() const { return mScaleMethod; }

    virtual QString dump() const;

    virtual QgsFeatureRendererV2* clone();

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;
    static QgsFeatureRendererV2* createFromSld( QDomElement& element, QGis::GeometryType geomType );

    //! returns bitwise OR-ed capabilities of the renderer
    //! \note added in 2.0
    virtual int capabilities() { return SymbolLevels | RotationField; }

    virtual QgsSymbolV2List symbols();

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    //! @note not available in python bindings
    virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, QString rule = "" );

  protected:
    std::auto_ptr<QgsSymbolV2> mSymbol;
    std::auto_ptr<QgsExpression> mRotation;
    std::auto_ptr<QgsExpression> mSizeScale;
    QgsSymbolV2::ScaleMethod mScaleMethod;

    // temporary stuff for rendering
    std::auto_ptr<QgsSymbolV2> mTempSymbol;
    double mOrigSize;

    // for copy and swap idiom for assignment operator
    void swap( QgsSingleSymbolRendererV2 & other );
};


#endif // QGSSINGLESYMBOLRENDERERV2_H
