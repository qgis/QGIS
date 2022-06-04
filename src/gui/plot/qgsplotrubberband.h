/***************************************************************************
                          qgsplotrubberband.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPLOTRUBBERBAND_H
#define QGSPLOTRUBBERBAND_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QPointF>
#include <QObject>
#include <QBrush>
#include <QPen>

class QgsPlotCanvas;
class QGraphicsRectItem;

/**
 * \ingroup gui
 * \brief QgsPlotRubberBand is an abstract base class for temporary rubber band items
 * in various shapes, for use within QgsPlotCanvas widgets.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotRubberBand : public QObject
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPlotRubberBand.
     */
    QgsPlotRubberBand( QgsPlotCanvas *canvas = nullptr );

    ~QgsPlotRubberBand() override = default;

    /**
     * Called when a rubber band should be created at the specified
     * starting \a position (in canvas coordinate space).
     */
    virtual void start( QPointF position, Qt::KeyboardModifiers modifiers ) = 0;

    /**
     * Called when a rubber band should be updated to reflect a temporary
     * ending \a position (in canvas coordinate space).
     */
    virtual void update( QPointF position, Qt::KeyboardModifiers modifiers ) = 0;

    /**
     * Called when a rubber band use has finished and the rubber
     * band is no longer required.
     * Returns the final bounding box of the rubber band.
     */
    virtual QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers() ) = 0;

    /**
     * Returns the canvas associated with the rubber band.
     */
    QgsPlotCanvas *canvas() const;

    /**
     * Returns the brush used for drawing the rubber band.
     * \see setBrush()
     * \see pen()
     */
    QBrush brush() const;

    /**
     * Sets the \a brush used for drawing the rubber band.
     * \see brush()
     * \see setPen()
     */
    void setBrush( const QBrush &brush );

    /**
     * Returns the pen used for drawing the rubber band.
     * \see setPen()
     * \see brush()
     */
    QPen pen() const;

    /**
     * Sets the \a pen used for drawing the rubber band.
     * \see pen()
     * \see setBrush()
     */
    void setPen( const QPen &pen );

  protected:

    /**
     * Calculates an updated bounding box rectangle from a original \a start position
     * and new \a position. If \a constrainSquare is TRUE then the bounding box will be
     * forced to a square shape. If \a fromCenter is TRUE then the original \a start
     * position will form the center point of the returned rectangle.
     */
    QRectF updateRect( QPointF start, QPointF position, bool constrainSquare, bool fromCenter );

  private:

    QgsPlotCanvas *mCanvas = nullptr;

    QBrush mBrush = Qt::NoBrush;
    QPen mPen = QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 );

};

/**
 * \ingroup gui
 * \brief QgsPlotRectangularRubberBand is rectangular rubber band for use within QgsPlotCanvas widgets.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotRectangularRubberBand : public QgsPlotRubberBand
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsPlotRectangularRubberBand.
     */
    QgsPlotRectangularRubberBand( QgsPlotCanvas *canvas = nullptr );

    ~QgsPlotRectangularRubberBand() override;

    void start( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    void update( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers() ) override;

  private:

    //! Rubber band item
    QGraphicsRectItem *mRubberBandItem = nullptr;

    //! Start of rubber band creation
    QPointF mRubberBandStartPos;

};

#endif // QGSPLOTRUBBERBAND_H
