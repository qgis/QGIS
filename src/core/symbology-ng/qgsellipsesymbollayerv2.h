/***************************************************************************
 qgsellipsesymbollayerv2.h
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

#include "qgsmarkersymbollayerv2.h"
#include <QPainterPath>

class QgsExpression;

/**A symbol layer for rendering objects with major and minor axis (e.g. ellipse, rectangle )*/
class CORE_EXPORT QgsEllipseSymbolLayerV2: public QgsMarkerSymbolLayerV2
{
  public:
    QgsEllipseSymbolLayerV2();
    ~QgsEllipseSymbolLayerV2();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context ) override;
    QString layerType() const override;
    void startRender( QgsSymbolV2RenderContext& context ) override;
    void stopRender( QgsSymbolV2RenderContext& context ) override;
    QgsSymbolLayerV2* clone() const override;
    QgsStringMap properties() const override;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const override;
    void writeSldMarker( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const override;

    bool writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, const QgsSymbolV2RenderContext* context, const QgsFeature* f, const QPointF& shift = QPointF( 0.0, 0.0 ) ) const override;

    void setSymbolName( const QString& name ) { mSymbolName = name; }
    QString symbolName() const { return mSymbolName; }

    void setSymbolWidth( double w ) { mSymbolWidth = w; }
    double symbolWidth() const { return mSymbolWidth; }

    void setSymbolHeight( double h ) { mSymbolHeight = h; }
    double symbolHeight() const { return mSymbolHeight; }

    Qt::PenStyle outlineStyle() const { return mOutlineStyle; }
    void setOutlineStyle( Qt::PenStyle outlineStyle ) { mOutlineStyle = outlineStyle; }

    void setOutlineWidth( double w ) { mOutlineWidth = w; }
    double outlineWidth() const { return mOutlineWidth; }

    void setFillColor( const QColor& c ) override { mFillColor = c;}
    QColor fillColor() const override { return mFillColor; }

    void setOutlineColor( const QColor& c ) override { mOutlineColor = c; }
    QColor outlineColor() const override { return mOutlineColor; }

    void setSymbolWidthUnit( QgsSymbolV2::OutputUnit unit ) { mSymbolWidthUnit = unit; }
    QgsSymbolV2::OutputUnit symbolWidthUnit() const { return mSymbolWidthUnit; }

    void setSymbolWidthMapUnitScale( const QgsMapUnitScale& scale ) { mSymbolWidthMapUnitScale = scale; }
    const QgsMapUnitScale& symbolWidthMapUnitScale() const { return mSymbolWidthMapUnitScale; }

    void setSymbolHeightUnit( QgsSymbolV2::OutputUnit unit ) { mSymbolHeightUnit = unit; }
    QgsSymbolV2::OutputUnit symbolHeightUnit() const { return mSymbolHeightUnit; }

    void setSymbolHeightMapUnitScale( const QgsMapUnitScale& scale ) { mSymbolHeightMapUnitScale = scale; }
    const QgsMapUnitScale& symbolHeightMapUnitScale() const { return mSymbolHeightMapUnitScale; }

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mOutlineWidthUnit = unit; }
    QgsSymbolV2::OutputUnit outlineWidthUnit() const { return mOutlineWidthUnit; }

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale ) { mOutlineWidthMapUnitScale = scale; }
    const QgsMapUnitScale& outlineWidthMapUnitScale() const { return mOutlineWidthMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit ) override;
    QgsSymbolV2::OutputUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale& scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

  private:
    QString mSymbolName;
    double mSymbolWidth;
    QgsSymbolV2::OutputUnit mSymbolWidthUnit;
    QgsMapUnitScale mSymbolWidthMapUnitScale;
    double mSymbolHeight;
    QgsSymbolV2::OutputUnit mSymbolHeightUnit;
    QgsMapUnitScale mSymbolHeightMapUnitScale;
    QColor mFillColor;
    QColor mOutlineColor;
    Qt::PenStyle mOutlineStyle;
    double mOutlineWidth;
    QgsSymbolV2::OutputUnit mOutlineWidthUnit;
    QgsMapUnitScale mOutlineWidthMapUnitScale;

    QPainterPath mPainterPath;

    QPen mPen;
    QBrush mBrush;

    /**Setup mPainterPath
      @param symbolName name of symbol
      @param context render context
      @param scaledWidth optional width
      @param scaledHeight optional height
      @param f optional feature to render (0 if no data defined rendering)
     */
    void preparePath( const QString& symbolName, QgsSymbolV2RenderContext& context, double* scaledWidth = 0, double* scaledHeight = 0, const QgsFeature* f = 0 );
};

#endif // QGSELLIPSESYMBOLLAYERV2_H


