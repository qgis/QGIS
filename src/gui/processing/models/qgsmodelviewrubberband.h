/***************************************************************************
                             qgsmodelviewrubberband.h
                             -------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELVIEWRUBBERBAND_H
#define QGSMODELVIEWRUBBERBAND_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QBrush>
#include <QPen>
#include <QObject>

#define SIP_NO_FILE

class QgsModelGraphicsView;
class QGraphicsRectItem;
class QGraphicsEllipseItem;
class QGraphicsPolygonItem;

/**
 * \ingroup gui
 * \brief QgsModelViewRubberBand is an abstract base class for temporary rubber band items
 * in various shapes, for use within QgsModelGraphicsView widgets.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelViewRubberBand : public QObject
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelViewRubberBand.
     */
    QgsModelViewRubberBand( QgsModelGraphicsView *view = nullptr );

    ~QgsModelViewRubberBand() override = default;

    /**
     * Creates a new instance of the QgsModelViewRubberBand subclass.
     */
    virtual QgsModelViewRubberBand *create( QgsModelGraphicsView *view ) const = 0 SIP_FACTORY;

    /**
     * Called when a rubber band should be created at the specified
     * starting \a position (in model coordinate space).
     */
    virtual void start( QPointF position, Qt::KeyboardModifiers modifiers ) = 0;

    /**
     * Called when a rubber band should be updated to reflect a temporary
     * ending \a position (in model coordinate space).
     */
    virtual void update( QPointF position, Qt::KeyboardModifiers modifiers ) = 0;

    /**
     * Called when a rubber band use has finished and the rubber
     * band is no longer required.
     * Returns the final bounding box of the rubber band.
     */
    virtual QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers() ) = 0;

    /**
     * Returns the view associated with the rubber band.
     */
    QgsModelGraphicsView *view() const;

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

  signals:

    /**
     * Emitted when the size of the rubber band is changed. The \a size
     * argument gives a translated string describing the new rubber band size,
     * with a format which differs per subclass (e.g. rectangles may describe
     * a size using width and height, while circles may describe a size by radius).
     */
    void sizeChanged( const QString &size );

  protected:
    /**
     * Calculates an updated bounding box rectangle from a original \a start position
     * and new \a position. If \a constrainSquare is TRUE then the bounding box will be
     * forced to a square shape. If \a fromCenter is TRUE then the original \a start
     * position will form the center point of the returned rectangle.
     */
    QRectF updateRect( QPointF start, QPointF position, bool constrainSquare, bool fromCenter );

  private:
    QgsModelGraphicsView *mView = nullptr;

    QBrush mBrush = Qt::NoBrush;
    QPen mPen = QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 );
};


/**
 * \ingroup gui
 * \brief QgsModelViewRectangularRubberBand is rectangular rubber band for use within QgsModelGraphicsView widgets.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelViewRectangularRubberBand : public QgsModelViewRubberBand
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelViewRectangularRubberBand.
     */
    QgsModelViewRectangularRubberBand( QgsModelGraphicsView *view = nullptr );
    QgsModelViewRectangularRubberBand *create( QgsModelGraphicsView *view ) const override SIP_FACTORY;

    ~QgsModelViewRectangularRubberBand() override;

    void start( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    void update( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers() ) override;

  private:
    //! Rubber band item
    QGraphicsRectItem *mRubberBandItem = nullptr;

    //! Start of rubber band creation
    QPointF mRubberBandStartPos;
};

#endif // QGSMODELVIEWRUBBERBAND_H
