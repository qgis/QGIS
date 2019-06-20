/***************************************************************************
                         qgslayoutitempolyline.h
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
     email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEMPOLYLINE_H
#define QGSLAYOUTITEMPOLYLINE_H

#include "qgis_core.h"
#include "qgslayoutitemnodeitem.h"
#include "qgssymbol.h"
#include <QGraphicsPathItem>
#include "qgslogger.h"
#include "qgslayout.h"

/**
 * \ingroup core
 * Layout item for node based polyline shapes.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemPolyline: public QgsLayoutNodesItem
{
    Q_OBJECT

  public:

    //! Vertex marker mode
    enum MarkerMode
    {
      NoMarker, //!< Don't show marker
      ArrowHead, //!< Show arrow marker
      SvgMarker, //!< Show SVG marker
    };

    /**
     * Constructor for QgsLayoutItemPolyline for the specified \a layout.
     */
    QgsLayoutItemPolyline( QgsLayout *layout );

    /**
     * Constructor for QgsLayoutItemPolyline for the specified \a polyline
     * and \a layout.
     */
    QgsLayoutItemPolyline( const QPolygonF &polyline, QgsLayout *layout );

    /**
     * Returns a new polyline item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemPolyline *create( QgsLayout *layout ) SIP_FACTORY;

    int type() const override;
    QIcon icon() const override;
    QString displayName() const override;
    QPainterPath shape() const override;

    /**
     * Returns the line symbol used to draw the shape.
     * \see setSymbol()
     */
    QgsLineSymbol *symbol() { return mPolylineStyleSymbol.get(); }

    /**
     * Sets the \a symbol used to draw the shape.
     * Ownership of \a symbol is not transferred.
     * \see symbol()
     */
    void setSymbol( QgsLineSymbol *symbol );

    /**
     * Returns the start marker mode, which controls what marker is drawn at the start of the line.
     * \see setStartMarker()
     * \see endMarker()
     */
    MarkerMode startMarker() const { return mStartMarker; }

    /**
     * Sets the start marker \a mode, which controls what marker is drawn at the start of the line.
     * \see startMarker()
     * \see setEndMarker()
     */
    void setStartMarker( MarkerMode mode );

    /**
     * Returns the end marker mode, which controls what marker is drawn at the end of the line.
     * \see setEndMarker()
     * \see startMarker()
     */
    MarkerMode endMarker() const { return mEndMarker; }

    /**
     * Sets the end marker \a mode, which controls what marker is drawn at the end of the line.
     * \see endMarker()
     * \see setStartMarker()
     */
    void setEndMarker( MarkerMode mode );

    /**
     * Sets the \a width of line arrow heads in mm.
     * \see arrowHeadWidth()
     */
    void setArrowHeadWidth( double width );

    /**
     * Returns the width of line arrow heads in mm.
     * \see setArrowHeadWidth()
     */
    double arrowHeadWidth() const { return mArrowHeadWidth; }

    /**
     * Sets the \a path to a SVG marker to draw at the start of the line.
     * \see startSvgMarkerPath()
     * \see setEndSvgMarkerPath()
     */
    void setStartSvgMarkerPath( const QString &path );

    /**
     * Returns the path the an SVG marker drawn at the start of the line.
     * \see setStartSvgMarkerPath()
     * \see endSvgMarkerPath
     */
    QString startSvgMarkerPath() const { return mStartMarkerFile; }

    /**
     * Sets the \a path to a SVG marker to draw at the end of the line.
     * \see endSvgMarkerPath()
     * \see setStartSvgMarkerPath()
     */
    void setEndSvgMarkerPath( const QString &path );

    /**
     * Returns the path the an SVG marker drawn at the end of the line.
     * \see setEndSvgMarkerPath()
     * \see startSvgMarkerPath
     */
    QString endSvgMarkerPath() const { return mEndMarkerFile; }

    /**
     * Returns the color used to draw the stroke around the the arrow head.
     * \see arrowHeadFillColor()
     * \see setArrowHeadStrokeColor()
     */
    QColor arrowHeadStrokeColor() const { return mArrowHeadStrokeColor; }

    /**
     * Sets the \a color used to draw the stroke around the arrow head.
     * \see setArrowHeadFillColor()
     * \see arrowHeadStrokeColor()
     */
    void setArrowHeadStrokeColor( const QColor &color );

    /**
     * Returns the color used to fill the arrow head.
     * \see arrowHeadStrokeColor()
     * \see setArrowHeadFillColor()
     */
    QColor arrowHeadFillColor() const { return mArrowHeadFillColor; }

    /**
     * Sets the \a color used to fill the arrow head.
     * \see arrowHeadFillColor()
     * \see setArrowHeadStrokeColor()
     */
    void setArrowHeadFillColor( const QColor &color );

    /**
     * Sets the pen \a width in millimeters for the stroke of the arrow head
     * \see arrowHeadStrokeWidth()
     * \see setArrowHeadStrokeColor()
     */
    void setArrowHeadStrokeWidth( double width );

    /**
     * Returns the pen width in millimeters for the stroke of the arrow head.
     * \see setArrowHeadStrokeWidth()
     * \see arrowHeadStrokeColor()
     */
    double arrowHeadStrokeWidth() const { return mArrowHeadStrokeWidth; }

  protected:

    bool _addNode( int indexPoint, QPointF newPoint, double radius ) override;
    bool _removeNode( int nodeIndex ) override;
    void _draw( QgsLayoutItemRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;
    void _readXmlStyle( const QDomElement &elmt, const QgsReadWriteContext &context ) override;
    void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const override;
    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  protected slots:

    void updateBoundingRect() override;

  private:

    //! QgsSymbol use to draw the shape.
    std::unique_ptr<QgsLineSymbol> mPolylineStyleSymbol;

    //! Marker to show at start of line
    MarkerMode mStartMarker = NoMarker;
    //! Marker to show at end of line
    MarkerMode mEndMarker = NoMarker;

    //! Width of the arrow marker in mm. The height is automatically adapted.
    double mArrowHeadWidth = 4.0;

    //! Height of the arrow marker in mm. Is calculated from arrow marker width and aspect ratio of svg
    double mStartArrowHeadHeight = 0.0;

    //! Height of the arrow marker in mm. Is calculated from arrow marker width and aspect ratio of svg
    double mEndArrowHeadHeight = 0.0;

    //! Path to the start marker file
    QString mStartMarkerFile;

    //! Path to the end marker file
    QString mEndMarkerFile;

    //! Arrow head stroke width, in mm
    double mArrowHeadStrokeWidth = 1.0;

    QColor mArrowHeadStrokeColor = Qt::black;
    QColor mArrowHeadFillColor = Qt::black;

    //! Create a default symbol.
    void createDefaultPolylineStyleSymbol();

    /**
     * Should be called after the shape's symbol is changed. Redraws the shape and recalculates
     * its selection bounds.
    */
    void refreshSymbol();

    void drawStartMarker( QPainter *painter );
    void drawEndMarker( QPainter *painter );

    void drawArrow( QPainter *painter, QPointF center, double angle );

    void updateMarkerSvgSizes();

    /**
     * Draws an arrow head on to a QPainter.
     * \param p destination painter
     * \param x x-coordinate of arrow center
     * \param y y-coordinate of arrow center
     * \param angle angle in degrees which arrow should point toward, measured
     * clockwise from pointing vertical upward
     * \param arrowHeadWidth size of arrow head
     */
    static void drawArrowHead( QPainter *p, double x, double y, double angle, double arrowHeadWidth );
    void drawSvgMarker( QPainter *p, QPointF point, double angle, const QString &markerPath, double height ) const;

    double computeMarkerMargin() const;

    friend class TestQgsLayoutPolyline;
    friend class QgsCompositionConverter;

};

#endif // QGSLAYOUTITEMPOLYLINE_H

