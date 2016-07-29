/***************************************************************************
    qgsgradientstopeditor.h
    -----------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRADIENTSTOPEDITOR_H
#define QGSGRADIENTSTOPEDITOR_H

#include "qgsvectorcolorrampv2.h"
#include <QWidget>


/** \ingroup gui
 * \class QgsGradientStopEditor
 * An interactive editor for previewing a gradient color ramp and modifying the position of color
 * stops along the gradient.
 * \note Added in version 2.16
 */

class GUI_EXPORT QgsGradientStopEditor : public QWidget
{
    Q_OBJECT

  public:

    /** Constructor for QgsGradientStopEditor.
     * @param parent parent widget
     * @param ramp optional initial gradient ramp
     */
    QgsGradientStopEditor( QWidget* parent = nullptr, QgsVectorGradientColorRampV2* ramp = nullptr );

    /** Sets the current ramp shown in the editor.
     * @param ramp color ramp
     * @see gradientRamp()
     */
    void setGradientRamp( const QgsVectorGradientColorRampV2& ramp );

    /** Returns the current ramp created by the editor.
     * @see setGradientRamp()
     */
    QgsVectorGradientColorRampV2 gradientRamp() const { return mGradient; }

    /** Sets the currently selected stop.
     * @param index index of stop, where 0 corresponds to the first stop
     * @see selectedStop()
     */
    void selectStop( int index );

    /** Returns details about the currently selected stop.
     * @see selectStop()
     */
    QgsGradientStop selectedStop() const;

    virtual QSize sizeHint() const override;
    void paintEvent( QPaintEvent* event ) override;

  public slots:

    /** Sets the color for the current selected stop.
     * @param color new stop color
     * @see setSelectedStopOffset()
     * @see setSelectedStopDetails()
     * @see setColor1()
     * @see setColor2()
     */
    void setSelectedStopColor( const QColor& color );

    /** Sets the offset for the current selected stop. This slot has no effect if either the
     * first or last stop is selected, as they cannot be repositioned.
     * @param offset new stop offset
     * @see setSelectedStopColor()
     * @see setSelectedStopDetails()
     */
    void setSelectedStopOffset( double offset );

    /** Sets the color and offset for the current selected stop.
     * @param color new stop color
     * @param offset new stop offset
     * @see setSelectedStopColor()
     * @see setSelectedStopOffset()
     */
    void setSelectedStopDetails( const QColor& color, double offset );

    /** Deletes the current selected stop. This slot has no effect if either the
     * first or last stop is selected, as they cannot be deleted.
     */
    void deleteSelectedStop();

    /** Sets the color for the first stop.
     * @param color new stop color
     * @see setColor2()
     * @see setSelectedStopColor()
     */
    void setColor1( const QColor& color );

    /** Sets the color for the last stop.
     * @param color new stop color
     * @see setColor1()
     * @see setSelectedStopColor()
     */
    void setColor2( const QColor& color );

  signals:

    //! Emitted when the gradient ramp is changed by a user
    void changed();

    /** Emitted when the current selected stop changes.
     * @param stop details about newly selected stop
     */
    void selectedStopChanged( const QgsGradientStop& stop );

  protected:

    virtual void mouseMoveEvent( QMouseEvent *event ) override;
    virtual void mousePressEvent( QMouseEvent *event ) override;
    virtual void mouseDoubleClickEvent( QMouseEvent * event ) override;
    virtual void keyPressEvent( QKeyEvent * event ) override;

    //Reimplemented to accept dragged colors
    void dragEnterEvent( QDragEnterEvent * e ) override;

    //Reimplemented to accept dropped colors
    void dropEvent( QDropEvent *e ) override;

  private:

    /** Generates a checkboard pattern pixmap for use as a background to transparent colors
     * @returns checkerboard pixmap
     */
    const QPixmap& transparentBackground();

    /** Draws a stop marker on the specified painter.
     * @param painter destination painter
     * @param topMiddle coordinate corresponding to top middle point of desired marker
     * @param color color of marker
     * @param selected set to true to draw the marker in a selected state
     */
    void drawStopMarker( QPainter& painter, QPoint topMiddle, const QColor& color, bool selected = false );

    //! Converts an x-coordinate in the widget's coordinate system to a relative ramp position
    double pointToRelativePosition( int x ) const;

    //! Converts a relative ramp position to a x-coordinate in the widget's coordinate system
    int relativePositionToPoint( double position ) const;

    //! Returns true if the selected stop is movable and deletable
    bool selectedStopIsMovable() const;

    //! Returns the closest stop to a mouse x position, or -1 if no stops within tolerance
    int findClosestStop( int x, int threshold = -1 ) const;

    QgsVectorGradientColorRampV2 mGradient;

    //! We keep a separate, unordered copy of the gradient stops so that the selected stop is not changed.
    QgsGradientStopsList mStops;

    //! Stop number of selected stop, where 0 = first stop
    int mSelectedStop;

    //! Polygon for stop triangle marker outer
    QPolygonF sOuterTriangle;

    //! Polygon for stop triangle marker inner
    QPolygonF sInnerTriangle;

};

#endif // QGSGRADIENTSTOPEDITOR_H
