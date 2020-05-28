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
#ifndef QGSELLIPSESYMBOLLAYER_H
#define QGSELLIPSESYMBOLLAYER_H

#define DEFAULT_ELLIPSE_JOINSTYLE    Qt::MiterJoin

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmarkersymbollayer.h"
#include <QPainterPath>

class QgsExpression;

/**
 * \ingroup core
 * A symbol layer for rendering objects with major and minor axis (e.g. ellipse, rectangle )*/
class CORE_EXPORT QgsEllipseSymbolLayer: public QgsMarkerSymbolLayer
{
  public:
    QgsEllipseSymbolLayer();

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    void renderPoint( QPointF point, QgsSymbolRenderContext &context ) override;
    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsEllipseSymbolLayer *clone() const override SIP_FACTORY;
    QgsStringMap properties() const override;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;
    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;

    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const override;

    void setSymbolName( const QString &name ) { mSymbolName = name; }
    QString symbolName() const { return mSymbolName; }

    void setSize( double size ) override;

    void setSymbolWidth( double w );
    double symbolWidth() const { return mSymbolWidth; }

    void setSymbolHeight( double h );
    double symbolHeight() const { return mSymbolHeight; }

    Qt::PenStyle strokeStyle() const { return mStrokeStyle; }
    void setStrokeStyle( Qt::PenStyle strokeStyle ) { mStrokeStyle = strokeStyle; }

    /**
     * Gets stroke join style.
     * \since QGIS 2.16 */
    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }

    /**
     * Set stroke join style.
     * \since QGIS 2.16 */
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    void setStrokeWidth( double w ) { mStrokeWidth = w; }
    double strokeWidth() const { return mStrokeWidth; }

    void setFillColor( const QColor &c ) override { setColor( c ); }
    QColor fillColor() const override { return color(); }

    void setStrokeColor( const QColor &c ) override { mStrokeColor = c; }
    QColor strokeColor() const override { return mStrokeColor; }

    /**
     * Sets the units for the symbol's width.
     * \param unit symbol units
     * \see symbolWidthUnit()
     * \see setSymbolHeightUnit()
    */
    void setSymbolWidthUnit( QgsUnitTypes::RenderUnit unit ) { mSymbolWidthUnit = unit; }

    /**
     * Returns the units for the symbol's width.
     * \see setSymbolWidthUnit()
     * \see symbolHeightUnit()
    */
    QgsUnitTypes::RenderUnit symbolWidthUnit() const { return mSymbolWidthUnit; }

    void setSymbolWidthMapUnitScale( const QgsMapUnitScale &scale ) { mSymbolWidthMapUnitScale = scale; }
    const QgsMapUnitScale &symbolWidthMapUnitScale() const { return mSymbolWidthMapUnitScale; }

    /**
     * Sets the units for the symbol's height.
     * \param unit symbol units
     * \see symbolHeightUnit()
     * \see setSymbolWidthUnit()
    */
    void setSymbolHeightUnit( QgsUnitTypes::RenderUnit unit ) { mSymbolHeightUnit = unit; }

    /**
     * Returns the units for the symbol's height.
     * \see setSymbolHeightUnit()
     * \see symbolWidthUnit()
    */
    QgsUnitTypes::RenderUnit symbolHeightUnit() const { return mSymbolHeightUnit; }

    void setSymbolHeightMapUnitScale( const QgsMapUnitScale &scale ) { mSymbolHeightMapUnitScale = scale; }
    const QgsMapUnitScale &symbolHeightMapUnitScale() const { return mSymbolHeightMapUnitScale; }

    /**
     * Sets the units for the symbol's stroke width.
     * \param unit symbol units
     * \see strokeWidthUnit()
    */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit ) { mStrokeWidthUnit = unit; }

    /**
     * Returns the units for the symbol's stroke width.
     * \see setStrokeWidthUnit()
    */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const { return mStrokeWidthUnit; }

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mStrokeWidthMapUnitScale = scale; }
    const QgsMapUnitScale &strokeWidthMapUnitScale() const { return mStrokeWidthMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) override;

  private:
    QString mSymbolName;
    double mSymbolWidth = 4;
    QgsUnitTypes::RenderUnit mSymbolWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mSymbolWidthMapUnitScale;
    double mSymbolHeight = 3;
    QgsUnitTypes::RenderUnit mSymbolHeightUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mSymbolHeightMapUnitScale;
    QColor mStrokeColor;
    Qt::PenStyle mStrokeStyle = Qt::SolidLine;
    Qt::PenJoinStyle mPenJoinStyle = DEFAULT_ELLIPSE_JOINSTYLE;
    double mStrokeWidth = 0;
    QgsUnitTypes::RenderUnit mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mStrokeWidthMapUnitScale;

    QPainterPath mPainterPath;

    QPen mPen;
    QBrush mBrush;

    /**
     * Setup mPainterPath
      \param symbolName name of symbol
      \param context render context
      \param scaledWidth optional width
      \param scaledHeight optional height
      \param f optional feature to render (0 if no data defined rendering)
     */
    void preparePath( const QString &symbolName, QgsSymbolRenderContext &context, double *scaledWidth = nullptr, double *scaledHeight = nullptr, const QgsFeature *f = nullptr );
    QSizeF calculateSize( QgsSymbolRenderContext &context, double *scaledWidth = nullptr, double *scaledHeight = nullptr );
    void calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledWidth, double scaledHeight, bool &hasDataDefinedRotation, QPointF &offset, double &angle ) const;
};

// clazy:excludeall=qstring-allocations

#endif // QGSELLIPSESYMBOLLAYER_H


