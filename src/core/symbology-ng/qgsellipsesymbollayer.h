/***************************************************************************
 qgsellipsesymbollayer.h
 ---------------------
 begin                : June 2011
 copyright            : (C) 2011 by Marco Hugentobler
 email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSELLIPSESYMBOLLAYERV2_H
#define QGSELLIPSESYMBOLLAYERV2_H

#define DEFAULT_ELLIPSE_JOINSTYLE    Qt::MiterJoin

#include "qgsmarkersymbollayer.h"
#include <QPainterPath>

class QgsExpression;

/** \ingroup core
 * A symbol layer for rendering objects with major and minor axis (e.g. ellipse, rectangle )*/
class CORE_EXPORT QgsEllipseSymbolLayer: public QgsMarkerSymbolLayer
{
  public:
    QgsEllipseSymbolLayer();
    ~QgsEllipseSymbolLayer();

    static QgsSymbolLayer* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayer* createFromSld( QDomElement &element );

    void renderPoint( QPointF point, QgsSymbolRenderContext& context ) override;
    QString layerType() const override;
    void startRender( QgsSymbolRenderContext& context ) override;
    void stopRender( QgsSymbolRenderContext& context ) override;
    QgsEllipseSymbolLayer* clone() const override;
    QgsStringMap properties() const override;

    void toSld( QDomDocument& doc, QDomElement &element, const QgsStringMap& props ) const override;
    void writeSldMarker( QDomDocument& doc, QDomElement &element, const QgsStringMap& props ) const override;

    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const override;

    void setSymbolName( const QString& name ) { mSymbolName = name; }
    QString symbolName() const { return mSymbolName; }

    void setSymbolWidth( double w ) { mSymbolWidth = w; }
    double symbolWidth() const { return mSymbolWidth; }

    void setSymbolHeight( double h ) { mSymbolHeight = h; }
    double symbolHeight() const { return mSymbolHeight; }

    Qt::PenStyle outlineStyle() const { return mOutlineStyle; }
    void setOutlineStyle( Qt::PenStyle outlineStyle ) { mOutlineStyle = outlineStyle; }

    /** Get outline join style.
     * @note added in 2.16 */
    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }
    /** Set outline join style.
     * @note added in 2.16 */
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    void setOutlineWidth( double w ) { mOutlineWidth = w; }
    double outlineWidth() const { return mOutlineWidth; }

    void setFillColor( const QColor& c ) override { setColor( c ); }
    QColor fillColor() const override { return color(); }

    void setOutlineColor( const QColor& c ) override { mOutlineColor = c; }
    QColor outlineColor() const override { return mOutlineColor; }

    /** Sets the units for the symbol's width.
     * @param unit symbol units
     * @see symbolWidthUnit()
     * @see setSymbolHeightUnit()
    */
    void setSymbolWidthUnit( QgsUnitTypes::RenderUnit unit ) { mSymbolWidthUnit = unit; }

    /** Returns the units for the symbol's width.
     * @see setSymbolWidthUnit()
     * @see symbolHeightUnit()
    */
    QgsUnitTypes::RenderUnit symbolWidthUnit() const { return mSymbolWidthUnit; }

    void setSymbolWidthMapUnitScale( const QgsMapUnitScale& scale ) { mSymbolWidthMapUnitScale = scale; }
    const QgsMapUnitScale& symbolWidthMapUnitScale() const { return mSymbolWidthMapUnitScale; }

    /** Sets the units for the symbol's height.
     * @param unit symbol units
     * @see symbolHeightUnit()
     * @see setSymbolWidthUnit()
    */
    void setSymbolHeightUnit( QgsUnitTypes::RenderUnit unit ) { mSymbolHeightUnit = unit; }

    /** Returns the units for the symbol's height.
     * @see setSymbolHeightUnit()
     * @see symbolWidthUnit()
    */
    QgsUnitTypes::RenderUnit symbolHeightUnit() const { return mSymbolHeightUnit; }

    void setSymbolHeightMapUnitScale( const QgsMapUnitScale& scale ) { mSymbolHeightMapUnitScale = scale; }
    const QgsMapUnitScale& symbolHeightMapUnitScale() const { return mSymbolHeightMapUnitScale; }

    /** Sets the units for the symbol's outline width.
     * @param unit symbol units
     * @see outlineWidthUnit()
    */
    void setOutlineWidthUnit( QgsUnitTypes::RenderUnit unit ) { mOutlineWidthUnit = unit; }

    /** Returns the units for the symbol's outline width.
     * @see setOutlineWidthUnit()
    */
    QgsUnitTypes::RenderUnit outlineWidthUnit() const { return mOutlineWidthUnit; }

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale ) { mOutlineWidthMapUnitScale = scale; }
    const QgsMapUnitScale& outlineWidthMapUnitScale() const { return mOutlineWidthMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale& scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    QRectF bounds( QPointF point, QgsSymbolRenderContext& context ) override;

  private:
    QString mSymbolName;
    double mSymbolWidth;
    QgsUnitTypes::RenderUnit mSymbolWidthUnit;
    QgsMapUnitScale mSymbolWidthMapUnitScale;
    double mSymbolHeight;
    QgsUnitTypes::RenderUnit mSymbolHeightUnit;
    QgsMapUnitScale mSymbolHeightMapUnitScale;
    QColor mOutlineColor;
    Qt::PenStyle mOutlineStyle;
    Qt::PenJoinStyle mPenJoinStyle;
    double mOutlineWidth;
    QgsUnitTypes::RenderUnit mOutlineWidthUnit;
    QgsMapUnitScale mOutlineWidthMapUnitScale;

    QPainterPath mPainterPath;

    QPen mPen;
    QBrush mBrush;

    /** Setup mPainterPath
      @param symbolName name of symbol
      @param context render context
      @param scaledWidth optional width
      @param scaledHeight optional height
      @param f optional feature to render (0 if no data defined rendering)
     */
    void preparePath( const QString& symbolName, QgsSymbolRenderContext& context, double* scaledWidth = nullptr, double* scaledHeight = nullptr, const QgsFeature* f = nullptr );
    QSizeF calculateSize( QgsSymbolRenderContext& context, double* scaledWidth = nullptr, double* scaledHeight = nullptr );
    void calculateOffsetAndRotation( QgsSymbolRenderContext& context, double scaledWidth, double scaledHeight, bool& hasDataDefinedRotation, QPointF& offset, double& angle ) const;
};

#endif // QGSELLIPSESYMBOLLAYERV2_H


