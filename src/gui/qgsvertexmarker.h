/***************************************************************************
    qgsvertexmarker.h  - canvas item which shows a simple vertex marker
    ---------------------
    begin                : February 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVERTEXMARKER_H
#define QGSVERTEXMARKER_H

#include "qgsmapcanvasitem.h"
#include "qgspoint.h"
#include "qgis_gui.h"

class QPainter;

/** \ingroup gui
 * A class for marking vertices of features using e.g. circles or 'x'.
 */
class GUI_EXPORT QgsVertexMarker : public QgsMapCanvasItem
{
  public:

    //! Icons
    enum IconType
    {
      ICON_NONE,
      ICON_CROSS,
      ICON_X,
      ICON_BOX,
      ICON_CIRCLE
    };

    QgsVertexMarker( QgsMapCanvas *mapCanvas );

    void setCenter( const QgsPoint &point );

    void setIconType( int iconType );

    void setIconSize( int iconSize );

    /**
     * Sets the stroke \a color for the marker.
     * @see color()
     * @see setFillColor()
     */
    void setColor( const QColor &color );

    /**
     * Returns the stroke color for the marker.
     * @see setColor()
     * @see fillColor()
     * @note added in QGIS 3.0
     */
    QColor color() const { return mColor; }

    /**
     * Sets the fill \a color for the marker. This setting only
     * applies to some icon types.
     * @note added in QGIS 3.0
     * @see fillColor()
     * @see setColor()
     */
    void setFillColor( const QColor &color );

    /**
     * Returns the fill \a color for the marker. This setting only
     * applies to some icon types.
     * @note added in QGIS 3.0
     * @see setFillColor()
     * @see color()
     */
    QColor fillColor() const { return mFillColor; }

    void setPenWidth( int width );

    void paint( QPainter *p ) override;

    QRectF boundingRect() const override;

    virtual void updatePosition() override;

  private:

    //! icon to be shown
    int mIconType = ICON_X;

    //! size
    int mIconSize = 10;

    //! coordinates of the point in the center
    QgsPoint mCenter;

    //! color of the marker
    QColor mColor = QColor( 255, 0, 0 );

    //! pen width
    int mPenWidth = 1;

    //! Fill color
    QColor mFillColor = QColor( 0, 0, 0, 0 );

};

#endif
