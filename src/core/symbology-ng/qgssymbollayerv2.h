/***************************************************************************
    qgssymbollayerv2.h
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
#ifndef QGSSYMBOLLAYERV2_H
#define QGSSYMBOLLAYERV2_H



#include <QColor>
#include <QMap>
#include <QPointF>
#include <QSet>
#include <QDomDocument>
#include <QDomElement>

#include "qgssymbolv2.h"

#include "qgssymbollayerv2utils.h" // QgsStringMap

class QPainter;
class QSize;
class QPolygonF;

class QgsExpression;
class QgsRenderContext;

class CORE_EXPORT QgsSymbolLayerV2
{
  public:

    // not necessarily supported by all symbol layers...
    virtual void setColor( const QColor& color ) { mColor = color; }
    virtual QColor color() const { return mColor; }

    virtual ~QgsSymbolLayerV2() { removeDataDefinedProperties(); }

    virtual QString layerType() const = 0;

    virtual void startRender( QgsSymbolV2RenderContext& context ) = 0;
    virtual void stopRender( QgsSymbolV2RenderContext& context ) = 0;

    virtual QgsSymbolLayerV2* clone() const = 0;

    virtual void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
    { Q_UNUSED( props ); element.appendChild( doc.createComment( QString( "SymbolLayerV2 %1 not implemented yet" ).arg( layerType() ) ) ); }

    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const { Q_UNUSED( mmScaleFactor ); Q_UNUSED( mapUnitScaleFactor ); return QString(); }

    virtual QgsStringMap properties() const = 0;

    virtual void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) = 0;

    virtual QgsSymbolV2* subSymbol() { return NULL; }
    // set layer's subsymbol. takes ownership of the passed symbol
    virtual bool setSubSymbol( QgsSymbolV2* symbol ) { delete symbol; return false; }

    QgsSymbolV2::SymbolType type() const { return mType; }

    void setLocked( bool locked ) { mLocked = locked; }
    bool isLocked() const { return mLocked; }

    virtual void setOutputUnit( QgsSymbolV2::OutputUnit unit ) { Q_UNUSED( unit ); } //= 0;
    virtual QgsSymbolV2::OutputUnit outputUnit() const { return QgsSymbolV2::Mixed; } //= 0;

    // used only with rending with symbol levels is turned on (0 = first pass, 1 = second, ...)
    void setRenderingPass( int renderingPass ) { mRenderingPass = renderingPass; }
    int renderingPass() const { return mRenderingPass; }

    // symbol layers normally only use additional attributes to provide data defined settings
    virtual QSet<QString> usedAttributes() const;

    virtual const QgsExpression* dataDefinedProperty( const QString& property ) const;
    virtual QString dataDefinedPropertyString( const QString& property ) const;
    virtual void setDataDefinedProperty( const QString& property, const QString& expressionString );
    virtual void removeDataDefinedProperty( const QString& property );
    virtual void removeDataDefinedProperties();

  protected:
    QgsSymbolLayerV2( QgsSymbolV2::SymbolType type, bool locked = false )
        : mType( type ), mLocked( locked ), mRenderingPass( 0 ) {}

    QgsSymbolV2::SymbolType mType;
    bool mLocked;
    QColor mColor;
    int mRenderingPass;

    QMap< QString, QgsExpression* > mDataDefinedProperties;

    // Configuration of selected symbology implementation
    static const bool selectionIsOpaque = true;  // Selection ignores symbol alpha
    static const bool selectFillBorder = false;  // Fill symbol layer also selects border symbology
    static const bool selectFillStyle = false;   // Fill symbol uses symbol layer style..

    virtual void prepareExpressions( const QgsVectorLayer* vl );
    virtual QgsExpression* expression( const QString& property );
    /**Saves data defined properties to string map*/
    void saveDataDefinedProperties( QgsStringMap& stringMap ) const;
    /**Copies data defined properties of this layer to another symbol layer*/
    void copyDataDefinedProperties( QgsSymbolLayerV2* destLayer ) const;
};

//////////////////////

class CORE_EXPORT QgsMarkerSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    void setAngle( double angle ) { mAngle = angle; }
    double angle() const { return mAngle; }

    void setSize( double size ) { mSize = size; }
    double size() const { return mSize; }

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod ) { mScaleMethod = scaleMethod; }
    QgsSymbolV2::ScaleMethod scaleMethod() const { return mScaleMethod; }

    void setOffset( QPointF offset ) { mOffset = offset; }
    QPointF offset() { return mOffset; }

    virtual void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
    { Q_UNUSED( props ); element.appendChild( doc.createComment( QString( "QgsMarkerSymbolLayerV2 %1 not implemented yet" ).arg( layerType() ) ) ); }

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setSizeUnit( QgsSymbolV2::OutputUnit unit ) { mSizeUnit = unit; }
    QgsSymbolV2::OutputUnit sizeUnit() const { return mSizeUnit; }

    virtual void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    virtual QgsSymbolV2::OutputUnit outputUnit() const;

  protected:
    QgsMarkerSymbolLayerV2( bool locked = false );

    double mAngle;
    double mSize;
    QgsSymbolV2::OutputUnit mSizeUnit;
    QPointF mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsSymbolV2::ScaleMethod mScaleMethod;
};

class CORE_EXPORT QgsLineSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context ) = 0;

    //! @note added in v1.7
    virtual void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    virtual void setWidth( double width ) { mWidth = width; }
    virtual double width() const { return mWidth; }

    void setWidthUnit( QgsSymbolV2::OutputUnit unit ) { mWidthUnit = unit; }
    QgsSymbolV2::OutputUnit widthUnit() const { return mWidthUnit; }

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

  protected:
    QgsLineSymbolLayerV2( bool locked = false );

    double mWidth;
    QgsSymbolV2::OutputUnit mWidthUnit;
};

class CORE_EXPORT QgsFillSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    void setAngle( double angle ) { mAngle = angle; }
    double angle() const { return mAngle; }

  protected:
    QgsFillSymbolLayerV2( bool locked = false );
    /**Default method to render polygon*/
    void _renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings );

    double mAngle;
};

class QgsSymbolLayerV2Widget;  // why does SIP fail, when this isn't here

#endif
