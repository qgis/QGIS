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
 * \brief A symbol layer for rendering objects with major and minor axis (e.g. ellipse, rectangle, etc).
*/
class CORE_EXPORT QgsEllipseSymbolLayer: public QgsMarkerSymbolLayer
{
  public:

    //! Marker symbol shapes
    enum Shape
    {
      Circle, //!< Circle
      Rectangle, //!< Rectangle
      Diamond, //!< Diamond
      Cross, //!< Stroke-only cross
      Arrow, //!< Stroke-only arrow (since QGIS 3.20)
      HalfArc, //!< Stroke-only half arc (since QGIS 3.20)
      Triangle, //!< Triangle
      RightHalfTriangle, //!< Right half of a triangle
      LeftHalfTriangle, //!< Left half of a triangle
      SemiCircle, //!< Semi circle
      ThirdCircle, //!< Third Circle (since QGIS 3.28)
      QuarterCircle, //!< Quarter Circle (since QGIS 3.28)
      Pentagon, //!< Pentagon (since QGIS 3.28)
      Hexagon, //!< Hexagon (since QGIS 3.28)
      Octagon, //!< Octagon (since QGIS 3.28)
      Star, //!< Star (since QGIS 3.28)
    };

    //! Returns a list of all available shape types.
    static QList< QgsEllipseSymbolLayer::Shape > availableShapes();

    /**
     * Returns TRUE if a \a shape has a fill.
     * \returns TRUE if shape uses a fill, or FALSE if shape uses lines only
     * \since QGIS 3.20
     */
    static bool shapeIsFilled( const QgsEllipseSymbolLayer::Shape &shape );

    QgsEllipseSymbolLayer();
    ~QgsEllipseSymbolLayer() override;

    //! Creates the symbol layer
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    void renderPoint( QPointF point, QgsSymbolRenderContext &context ) override;
    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsEllipseSymbolLayer *clone() const override SIP_FACTORY;
    QVariantMap properties() const override;

    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;
    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;

    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const override;

    /**
     * Sets the rendered ellipse marker shape using a symbol \a name.
     * \see setShape()
     * \see shape()
     * \deprecated since QGIS 3.20
     */
    Q_DECL_DEPRECATED void setSymbolName( const QString &name ) SIP_DEPRECATED { mShape = decodeShape( name ); }

    /**
     * Returns the shape name for the rendered ellipse marker symbol.
     * \see shape()
     * \see setShape()
     * \deprecated since QGIS 3.20
     */
    Q_DECL_DEPRECATED QString symbolName() const SIP_DEPRECATED { return encodeShape( mShape ); }

    /**
     * Returns the shape for the rendered ellipse marker symbol.
     * \see setShape()
     * \since QGIS 3.20
     */
    QgsEllipseSymbolLayer::Shape shape() const { return mShape; }

    /**
     * Sets the rendered ellipse marker shape.
     * \param shape new ellipse marker shape
     * \see shape()
     * \since QGIS 3.20
     */
    void setShape( QgsEllipseSymbolLayer::Shape shape ) { mShape = shape; }

    /**
     * Attempts to decode a string representation of a shape name to the corresponding
     * shape.
     * \param name encoded shape name
     * \param ok if specified, will be set to TRUE if shape was successfully decoded
     * \returns decoded name
     * \see encodeShape()
     * \since QGIS 3.20
     */
    static QgsEllipseSymbolLayer::Shape decodeShape( const QString &name, bool *ok = nullptr );

    /**
     * Encodes a shape to its string representation.
     * \param shape shape to encode
     * \returns encoded string
     * \see decodeShape()
     * \since QGIS 3.20
     */
    static QString encodeShape( QgsEllipseSymbolLayer::Shape shape );

    void setSize( double size ) override;

    void setSymbolWidth( double w );
    double symbolWidth() const { return mSymbolWidth; }

    void setSymbolHeight( double h );
    double symbolHeight() const { return mSymbolHeight; }

    Qt::PenStyle strokeStyle() const { return mStrokeStyle; }
    void setStrokeStyle( Qt::PenStyle strokeStyle ) { mStrokeStyle = strokeStyle; }

    /**
     * Gets stroke join style.
     * \since QGIS 2.16
     */
    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }

    /**
     * Set stroke join style.
     * \since QGIS 2.16
    */
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    /**
     * Returns the marker's stroke cap style (e.g., flat, round, etc).
     * \see setPenCapStyle()
     * \see penJoinStyle()
     * \see strokeColor()
     * \see strokeStyle()
     * \since QGIS 3.20
    */
    Qt::PenCapStyle penCapStyle() const { return mPenCapStyle; }

    /**
     * Sets the marker's stroke cap \a style (e.g., flat, round, etc).
     * \see penCapStyle()
     * \see penJoinStyle()
     * \see setStrokeColor()
     * \see setStrokeStyle()
     * \since QGIS 3.20
    */
    void setPenCapStyle( Qt::PenCapStyle style ) { mPenCapStyle = style; }

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
    bool usesMapUnits() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) override;

  private:
    Shape mShape = Circle;
    double mSymbolWidth = 4;
    QgsUnitTypes::RenderUnit mSymbolWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mSymbolWidthMapUnitScale;
    double mSymbolHeight = 3;
    QgsUnitTypes::RenderUnit mSymbolHeightUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mSymbolHeightMapUnitScale;
    QColor mStrokeColor;
    Qt::PenStyle mStrokeStyle = Qt::SolidLine;
    Qt::PenJoinStyle mPenJoinStyle = DEFAULT_ELLIPSE_JOINSTYLE;
    Qt::PenCapStyle mPenCapStyle = Qt::SquareCap;
    double mStrokeWidth = 0;
    QgsUnitTypes::RenderUnit mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mStrokeWidthMapUnitScale;

    QPainterPath mPainterPath;

    QPen mPen;
    QBrush mBrush;
    //! QPen to use as stroke of selected symbols
    QPen mSelPen;
    //! QBrush to use as fill of selected symbols
    QBrush mSelBrush;

    /**
     * Setup mPainterPath
     * \param shape name of symbol
     * \param context render context
     * \param scaledWidth optional width
     * \param scaledHeight optional height
     * \param f optional feature to render (0 if no data defined rendering)
     */
    void preparePath( const QgsEllipseSymbolLayer::Shape &shape, QgsSymbolRenderContext &context, double *scaledWidth = nullptr, double *scaledHeight = nullptr, const QgsFeature *f = nullptr );
    QSizeF calculateSize( QgsSymbolRenderContext &context, double *scaledWidth = nullptr, double *scaledHeight = nullptr );
    void calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledWidth, double scaledHeight, bool &hasDataDefinedRotation, QPointF &offset, double &angle ) const;
};

// clazy:excludeall=qstring-allocations

#endif // QGSELLIPSESYMBOLLAYER_H


