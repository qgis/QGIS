/***************************************************************************
                         qgscomposerarrow.h
                         ----------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERARROW_H
#define QGSCOMPOSERARROW_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgscomposeritem.h"
#include <QBrush>
#include <QPen>

class QgsLineSymbol;

/**
 * \ingroup core
 * An item that draws an arrow between two points.
*/
class CORE_EXPORT QgsComposerArrow: public QgsComposerItem
{
    Q_OBJECT

  public:

    enum MarkerMode
    {
      DefaultMarker,
      NoMarker,
      SVGMarker
    };

    /**
     * Constructor
     * \param c parent composition
     */
    QgsComposerArrow( QgsComposition *c SIP_TRANSFERTHIS );

    /**
     * Constructor
     * \param startPoint start point for line
     * \param stopPoint end point for line
     * \param c parent composition
     */
    QgsComposerArrow( QPointF startPoint, QPointF stopPoint, QgsComposition *c SIP_TRANSFERTHIS );

    ~QgsComposerArrow();

    //! Return composer item type.
    virtual int type() const override { return ComposerArrow; }

    //! \brief Reimplementation of QCanvasItem::paint - draw on canvas
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

    /**
     * Modifies position of start and endpoint and calls QgsComposerItem::setSceneRect
     */
    void setSceneRect( const QRectF &rectangle ) override;

    /**
     * Sets the width of the arrow head in mm
     * \param width width of arrow head
     * \see arrowHeadWidth
     */
    void setArrowHeadWidth( double width );

    /**
     * Returns the width of the arrow head in mm
     * \returns width of arrow head
     * \see setArrowHeadWidth
     */
    double arrowHeadWidth() const { return mArrowHeadWidth; }

    /**
     * Sets the marker to draw at the start of the line
     * \param svgPath file path for svg marker graphic to draw
     * \see startMarker
     * \see setEndMarker
     */
    void setStartMarker( const QString &svgPath );

    /**
     * Returns the marker drawn at the start of the line
     * \returns file path for svg marker graphic
     * \see setStartMarker
     * \see endMarker
     */
    QString startMarker() const { return mStartMarkerFile; }

    /**
     * Sets the marker to draw at the end of the line
     * \param svgPath file path for svg marker graphic to draw
     * \see endMarker
     * \see setStartMarker
     */
    void setEndMarker( const QString &svgPath );

    /**
     * Returns the marker drawn at the end of the line
     * \returns file path for svg marker graphic
     * \see setEndMarker
     * \see startMarker
     */
    QString endMarker() const { return mEndMarkerFile; }

    /**
     * Returns the color used to draw stroke around the the arrow head.
     * \returns arrow head stroke color
     * \see arrowHeadFillColor
     * \see setArrowHeadStrokeColor
     * \since QGIS 2.5
     */
    QColor arrowHeadStrokeColor() const { return mArrowHeadStrokeColor; }

    /**
     * Sets the color used to draw the stroke around the arrow head.
     * \param color arrow head stroke color
     * \see setArrowHeadFillColor
     * \see arrowHeadStrokeColor
     * \since QGIS 2.5
     */
    void setArrowHeadStrokeColor( const QColor &color );

    /**
     * Returns the color used to fill the arrow head.
     * \returns arrow head fill color
     * \see arrowHeadStrokeColor
     * \see setArrowHeadFillColor
     * \since QGIS 2.5
     */
    QColor arrowHeadFillColor() const { return mArrowHeadFillColor; }

    /**
     * Sets the color used to fill the arrow head.
     * \param color arrow head fill color
     * \see arrowHeadFillColor
     * \see setArrowHeadStrokeColor
     * \since QGIS 2.5
     */
    void setArrowHeadFillColor( const QColor &color );

    /**
     * Sets the pen width for the stroke of the arrow head
     * \param width pen width for arrow head stroke
     * \see arrowHeadStrokeWidth
     * \see setArrowHeadStrokeColor
     * \since QGIS 2.5
     */
    void setArrowHeadStrokeWidth( const double width );

    /**
     * Returns the pen width for the stroke of the arrow head
     * \returns pen width for arrow head stroke
     * \see setArrowHeadStrokeWidth
     * \see arrowHeadStrokeColor
     * \since QGIS 2.5
     */
    double arrowHeadStrokeWidth() const { return mArrowHeadStrokeWidth; }

    /**
     * Sets the line symbol used for drawing the line portion of the arrow
     * \param symbol line symbol
     * \see lineSymbol
     * \since QGIS 2.5
     */
    void setLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbol used for drawing the line portion of the arrow
     * \returns line symbol
     * \see setLineSymbol
     * \since QGIS 2.5
     */
    QgsLineSymbol *lineSymbol() { return mLineSymbol; }

    /**
     * Returns marker mode, which controls how the arrow endpoints are drawn
     * \returns marker mode
     * \see setMarkerMode
     */
    MarkerMode markerMode() const { return mMarkerMode; }

    /**
     * Sets the marker mode, which controls how the arrow endpoints are drawn
     * \param mode marker mode
     * \see setMarkerMode
     */
    void setMarkerMode( MarkerMode mode );

    /**
     * Stores state in DOM element
     * \param elem is DOM element corresponding to 'Composer' tag
     * \param doc document
     */
    bool writeXml( QDomElement &elem, QDomDocument &doc ) const override;

    /**
     * Sets state from DOM document
     * \param itemElem is DOM node corresponding to item tag
     * \param doc is the document to read
     */
    bool readXml( const QDomElement &itemElem, const QDomDocument &doc ) override;

  private:

    enum MarkerType
    {
      StartMarker,
      EndMarker
    };

    QPointF mStartPoint;
    QPointF mStopPoint;

    /**
     * Considering the rectangle as spanning [x[0], x[1]] x [y[0], y[1]], these
     * indices specify which index {0, 1} corresponds to the start point
     * coordinate of the respective dimension*/
    int mStartXIdx;
    int mStartYIdx;

    QPen mPen;
    QBrush mBrush;

    //! Width of the arrow marker in mm. May be specified by the user. The height is automatically adapted
    double mArrowHeadWidth;
    //! Height of the arrow marker in mm. Is calculated from arrow marker width and apsect ratio of svg
    double mStartArrowHeadHeight;
    double mStopArrowHeadHeight;

    //! Path to the start marker file
    QString mStartMarkerFile;
    //! Path to the end marker file
    QString mEndMarkerFile;
    //! Default marker, no marker or svg marker
    MarkerMode mMarkerMode;

    double mArrowHeadStrokeWidth;
    QColor mArrowHeadStrokeColor;
    QColor mArrowHeadFillColor;

    /**
     * Indicates QGIS version to mimic bounding box behavior for. The line placement changed in version 2.4, so a value
     * of 22 is used to indicate that the line should be drawn using the older placement routine.
     */
    int mBoundsBehavior;

    QgsLineSymbol *mLineSymbol = nullptr;

    /**
     * Adapts the item scene rect to contain the start point, the stop point including the arrow marker and the stroke.
     *  Needs to be called whenever the arrow width/height, the stroke with or the endpoints are changed
     */
    void adaptItemSceneRect();
    //! Computes the margin around the line necessary to include the markers
    double computeMarkerMargin() const;
    //! Draws the default marker at the line end
    void drawHardcodedMarker( QPainter *p, MarkerType type );
    //! Draws a user-defined marker (must be an svg file)
    void drawSVGMarker( QPainter *p, MarkerType type, const QString &markerPath );
    //! Apply default graphics settings
    void init();

    /**
     * Creates the default line symbol
     * \since QGIS 2.5
     */
    void createDefaultLineSymbol();

    /**
     * Draws the arrow line
     * \since QGIS 2.5
     */
    void drawLine( QPainter *painter );
};

#endif // QGSCOMPOSERARROW_H


