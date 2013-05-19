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

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    QgsSymbolLayerV2* clone() const;
    QgsStringMap properties() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;
    void writeSldMarker( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

    void setSymbolName( const QString& name ) { mSymbolName = name; }
    QString symbolName() const { return mSymbolName; }

    void setSymbolWidth( double w ) { mSymbolWidth = w; }
    double symbolWidth() const { return mSymbolWidth; }

    void setSymbolHeight( double h ) { mSymbolHeight = h; }
    double symbolHeight() const { return mSymbolHeight; }

    void setOutlineWidth( double w ) { mOutlineWidth = w; }
    double outlineWidth() const { return mOutlineWidth; }

    void setFillColor( const QColor& c ) { mFillColor = c;}
    QColor fillColor() const { return mFillColor; }

    void setOutlineColor( const QColor& c ) { mOutlineColor = c; }
    QColor outlineColor() const { return mOutlineColor; }

    void setSymbolWidthUnit( QgsSymbolV2::OutputUnit unit ) { mSymbolWidthUnit = unit; }
    QgsSymbolV2::OutputUnit symbolWidthUnit() const { return mSymbolWidthUnit; }

    void setSymbolHeightUnit( QgsSymbolV2::OutputUnit unit ) { mSymbolHeightUnit = unit; }
    QgsSymbolV2::OutputUnit symbolHeightUnit() const { return mSymbolHeightUnit; }

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mOutlineWidthUnit = unit; }
    QgsSymbolV2::OutputUnit outlineWidthUnit() const { return mOutlineWidthUnit; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

  private:
    QString mSymbolName;
    double mSymbolWidth;
    QgsSymbolV2::OutputUnit mSymbolWidthUnit;
    double mSymbolHeight;
    QgsSymbolV2::OutputUnit mSymbolHeightUnit;
    QColor mFillColor;
    QColor mOutlineColor;
    double mOutlineWidth;
    QgsSymbolV2::OutputUnit mOutlineWidthUnit;

    QPainterPath mPainterPath;

    QPen mPen;
    QBrush mBrush;

    /**Setup mPainterPath
      @param symbolName name of symbol
      @param context render context
      @param f feature f to render (0 if no data defined rendering)*/
    void preparePath( const QString& symbolName, QgsSymbolV2RenderContext& context, const QgsFeature* f = 0 );

    /**True if this symbol layer uses a data defined property*/
    bool hasDataDefinedProperty() const;
};

#endif // QGSELLIPSESYMBOLLAYERV2_H
