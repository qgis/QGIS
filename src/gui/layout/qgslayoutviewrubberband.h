/***************************************************************************
                             qgslayoutviewrubberband.h
                             -------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTVIEWRUBBERBAND_H
#define QGSLAYOUTVIEWRUBBERBAND_H

#include <QMouseEvent>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QBrush>
#include <QPen>
#include <QObject>

class QgsLayoutView;
class QGraphicsRectItem;
class QGraphicsEllipseItem;
class QGraphicsPolygonItem;
class QgsLayout;

/**
 * \ingroup gui
 * QgsLayoutViewRubberBand is an abstract base class for temporary rubber band items
 * in various shapes, for use within QgsLayoutView widgets.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewRubberBand : public QObject
{

    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsLayoutViewMouseEvent *>( sipCpp ) )
      sipType = sipType_QgsLayoutViewMouseEvent;
    else
      sipType = 0;
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsLayoutViewRubberBand.
     */
    QgsLayoutViewRubberBand( QgsLayoutView *view = nullptr );

    virtual ~QgsLayoutViewRubberBand() = default;

    /**
     * Creates a new instance of the QgsLayoutViewRubberBand subclass.
     */
    virtual QgsLayoutViewRubberBand *create( QgsLayoutView *view ) const = 0 SIP_FACTORY;

    /**
     * Called when a rubber band should be created at the specified
     * starting \a position (in layout coordinate space).
     */
    virtual void start( QPointF position, Qt::KeyboardModifiers modifiers ) = 0;

    /**
     * Called when a rubber band should be updated to reflect a temporary
     * ending \a position (in layout coordinate space).
     */
    virtual void update( QPointF position, Qt::KeyboardModifiers modifiers ) = 0;

    /**
     * Called when a rubber band use has finished and the rubber
     * band is no longer required.
     * Returns the final bounding box of the rubber band.
     */
    virtual QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = nullptr ) = 0;

    /**
     * Returns the view associated with the rubber band.
     * \see layout()
     */
    QgsLayoutView *view() const;

    /**
     * Returns the layout associated with the rubber band.
     * \see view()
     */
    QgsLayout *layout() const;

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
     * and new \a position. If \a constrainSquare is true then the bounding box will be
     * forced to a square shape. If \a fromCenter is true then the original \a start
     * position will form the center point of the returned rectangle.
     */
    QRectF updateRect( QPointF start, QPointF position, bool constrainSquare, bool fromCenter );

  private:

    QgsLayoutView *mView = nullptr;

    QBrush mBrush = Qt::NoBrush;
    QPen mPen = QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 );

};


/**
 * \ingroup gui
 * QgsLayoutViewRectangularRubberBand is rectangular rubber band for use within QgsLayoutView widgets.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewRectangularRubberBand : public QgsLayoutViewRubberBand
{
  public:

    /**
     * Constructor for QgsLayoutViewRectangularRubberBand.
     */
    QgsLayoutViewRectangularRubberBand( QgsLayoutView *view = nullptr );
    QgsLayoutViewRectangularRubberBand *create( QgsLayoutView *view ) const override SIP_FACTORY;

    ~QgsLayoutViewRectangularRubberBand() override;

    void start( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    void update( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = nullptr ) override;

  private:

    //! Rubber band item
    QGraphicsRectItem *mRubberBandItem = nullptr;

    //! Start of rubber band creation
    QPointF mRubberBandStartPos;

};

/**
 * \ingroup gui
 * QgsLayoutViewEllipseRubberBand is elliptical rubber band for use within QgsLayoutView widgets.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewEllipticalRubberBand : public QgsLayoutViewRubberBand
{
  public:

    /**
     * Constructor for QgsLayoutViewEllipticalRubberBand.
     */
    QgsLayoutViewEllipticalRubberBand( QgsLayoutView *view = nullptr );
    QgsLayoutViewEllipticalRubberBand *create( QgsLayoutView *view ) const override SIP_FACTORY;

    ~QgsLayoutViewEllipticalRubberBand() override;

    void start( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    void update( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = nullptr ) override;

  private:

    //! Rubber band item
    QGraphicsEllipseItem *mRubberBandItem = nullptr;

    //! Start of rubber band creation
    QPointF mRubberBandStartPos;

};

/**
 * \ingroup gui
 * QgsLayoutViewTriangleRubberBand is triangular rubber band for use within QgsLayoutView widgets.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewTriangleRubberBand : public QgsLayoutViewRubberBand
{
  public:

    /**
     * Constructor for QgsLayoutViewTriangleRubberBand.
     */
    QgsLayoutViewTriangleRubberBand( QgsLayoutView *view = nullptr );
    QgsLayoutViewTriangleRubberBand *create( QgsLayoutView *view ) const override SIP_FACTORY;

    ~QgsLayoutViewTriangleRubberBand() override;

    void start( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    void update( QPointF position, Qt::KeyboardModifiers modifiers ) override;
    QRectF finish( QPointF position = QPointF(), Qt::KeyboardModifiers modifiers = nullptr ) override;

  private:

    //! Rubber band item
    QGraphicsPolygonItem *mRubberBandItem = nullptr;

    //! Start of rubber band creation
    QPointF mRubberBandStartPos;

};
#endif // QGSLAYOUTVIEWRUBBERBAND_H
