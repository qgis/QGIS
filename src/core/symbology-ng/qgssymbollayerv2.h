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

// MSVC compiler doesn't have defined M_PI in math.h
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#define DEG2RAD(x)    ((x)*M_PI/180)
#define DEFAULT_SCALE_METHOD              QgsSymbolV2::ScaleArea

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

class QgsDxfExport;
class QgsExpression;
class QgsRenderContext;

class CORE_EXPORT QgsSymbolLayerV2
{
  public:

    // not necessarily supported by all symbol layers...
    virtual QColor color() const { return mColor; }
    virtual void setColor( const QColor& color ) { mColor = color; }
    /** Set outline color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual void setOutlineColor( const QColor& color ) { Q_UNUSED( color ); }
    /** Get outline color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual QColor outlineColor() const { return QColor(); }
    /** Set fill color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual void setFillColor( const QColor& color ) { Q_UNUSED( color ); }
    /** Get fill color. Supported by marker and fill layers.
     * @note added in 2.1 */
    virtual QColor fillColor() const { return QColor(); }

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

    /**Returns the estimated maximum distance which the layer style will bleed outside
      the drawn shape. Eg, polygons drawn with an outline will draw half the width
      of the outline outside of the polygon. This amount is estimated, since it may
      be affected by data defined symbology rules.*/
    virtual double estimateMaxBleed() const { return 0; }

    virtual void setOutputUnit( QgsSymbolV2::OutputUnit unit ) { Q_UNUSED( unit ); } //= 0;
    virtual QgsSymbolV2::OutputUnit outputUnit() const { return QgsSymbolV2::Mixed; } //= 0;

    virtual void setMapUnitScale( const QgsMapUnitScale& scale ) { Q_UNUSED( scale ); } //= 0;
    virtual QgsMapUnitScale mapUnitScale() const { return QgsMapUnitScale(); } //= 0;

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
    bool hasDataDefinedProperties() const { return mDataDefinedProperties.size() > 0; }

    virtual bool writeDxf( QgsDxfExport& e,
                           double mmMapUnitScaleFactor,
                           const QString& layerName,
                           const QgsSymbolV2RenderContext* context,
                           const QgsFeature* f,
                           const QPointF& shift = QPointF( 0.0, 0.0 ) ) const;

    virtual double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;

    virtual QColor dxfColor( const QgsSymbolV2RenderContext& context ) const;

    virtual QVector<qreal> dxfCustomDashPattern( QgsSymbolV2::OutputUnit& unit ) const;
    virtual Qt::PenStyle dxfPenStyle() const;
    virtual QColor dxfBrushColor( const QgsSymbolV2RenderContext& context ) const;
    virtual Qt::BrushStyle dxfBrushStyle() const;

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

    virtual void prepareExpressions( const QgsFields* fields, double scale = -1.0 );
    virtual QgsExpression* expression( const QString& property ) const;
    /**Saves data defined properties to string map*/
    void saveDataDefinedProperties( QgsStringMap& stringMap ) const;
    /**Copies data defined properties of this layer to another symbol layer*/
    void copyDataDefinedProperties( QgsSymbolLayerV2* destLayer ) const;
};

//////////////////////

class CORE_EXPORT QgsMarkerSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:

    enum HorizontalAnchorPoint
    {
      Left,
      HCenter,
      Right
    };

    enum VerticalAnchorPoint
    {
      Top,
      VCenter,
      Bottom
    };

    void startRender( QgsSymbolV2RenderContext& context ) override;

    virtual void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) override;

    void setAngle( double angle ) { mAngle = angle; }
    double angle() const { return mAngle; }

    void setSize( double size ) { mSize = size; }
    double size() const { return mSize; }

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod ) { mScaleMethod = scaleMethod; }
    QgsSymbolV2::ScaleMethod scaleMethod() const { return mScaleMethod; }

    void setOffset( QPointF offset ) { mOffset = offset; }
    QPointF offset() { return mOffset; }

    virtual void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const override;

    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
    { Q_UNUSED( props ); element.appendChild( doc.createComment( QString( "QgsMarkerSymbolLayerV2 %1 not implemented yet" ).arg( layerType() ) ) ); }

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setSizeUnit( QgsSymbolV2::OutputUnit unit ) { mSizeUnit = unit; }
    QgsSymbolV2::OutputUnit sizeUnit() const { return mSizeUnit; }

    void setSizeMapUnitScale( const QgsMapUnitScale& scale ) { mSizeMapUnitScale = scale; }
    const QgsMapUnitScale& sizeMapUnitScale() const { return mSizeMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit ) override;
    QgsSymbolV2::OutputUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale& scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    void setHorizontalAnchorPoint( HorizontalAnchorPoint h ) { mHorizontalAnchorPoint = h; }
    HorizontalAnchorPoint horizontalAnchorPoint() const { return mHorizontalAnchorPoint; }

    void setVerticalAnchorPoint( VerticalAnchorPoint v ) { mVerticalAnchorPoint = v; }
    VerticalAnchorPoint verticalAnchorPoint() const { return mVerticalAnchorPoint; }

  protected:
    QgsMarkerSymbolLayerV2( bool locked = false );

    //handles marker offset and anchor point shift together
    void markerOffset( const QgsSymbolV2RenderContext& context, double& offsetX, double& offsetY ) const;

    void markerOffset( const QgsSymbolV2RenderContext& context, double width, double height, double& offsetX, double& offsetY ) const;

    //! @note available in python bindings as markerOffset2
    void markerOffset( const QgsSymbolV2RenderContext& context, double width, double height,
                       QgsSymbolV2::OutputUnit widthUnit, QgsSymbolV2::OutputUnit heightUnit,
                       double& offsetX, double& offsetY,
                       const QgsMapUnitScale &widthMapUnitScale, const QgsMapUnitScale &heightMapUnitScale ) const;

    static QPointF _rotatedOffset( const QPointF& offset, double angle );

    double mAngle;
    double mSize;
    QgsSymbolV2::OutputUnit mSizeUnit;
    QgsMapUnitScale mSizeMapUnitScale;
    QPointF mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;
    QgsSymbolV2::ScaleMethod mScaleMethod;
    HorizontalAnchorPoint mHorizontalAnchorPoint;
    VerticalAnchorPoint mVerticalAnchorPoint;

  private:
    static QgsMarkerSymbolLayerV2::HorizontalAnchorPoint decodeHorizontalAnchorPoint( const QString& str );
    static QgsMarkerSymbolLayerV2::VerticalAnchorPoint decodeVerticalAnchorPoint( const QString& str );

    QgsExpression* mOffsetExpression;
    QgsExpression* mHorizontalAnchorExpression;
    QgsExpression* mVerticalAnchorExpression;
};

class CORE_EXPORT QgsLineSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context ) = 0;

    virtual void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    virtual void setWidth( double width ) { mWidth = width; }
    virtual double width() const { return mWidth; }

    void setWidthUnit( QgsSymbolV2::OutputUnit unit ) { mWidthUnit = unit; }
    QgsSymbolV2::OutputUnit widthUnit() const { return mWidthUnit; }

    void setWidthMapUnitScale( const QgsMapUnitScale& scale ) { mWidthMapUnitScale = scale; }
    const QgsMapUnitScale& widthMapUnitScale() const { return mWidthMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit ) override;
    QgsSymbolV2::OutputUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale& scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) override;

    virtual double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const override;

  protected:
    QgsLineSymbolLayerV2( bool locked = false );

    double mWidth;
    QgsSymbolV2::OutputUnit mWidthUnit;
    QgsMapUnitScale mWidthMapUnitScale;
};

class CORE_EXPORT QgsFillSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) override;

    void setAngle( double angle ) { mAngle = angle; }
    double angle() const { return mAngle; }

  protected:
    QgsFillSymbolLayerV2( bool locked = false );
    /**Default method to render polygon*/
    void _renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    double mAngle;
};

class QgsSymbolLayerV2Widget;  // why does SIP fail, when this isn't here

#endif


