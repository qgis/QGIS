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

    QgsVertexMarker( QgsMapCanvas* mapCanvas );

    void setCenter( const QgsPoint& point );

    void setIconType( int iconType );

    void setIconSize( int iconSize );

    void setColor( const QColor& color );

    void setPenWidth( int width );

    void paint( QPainter* p ) override;

    QRectF boundingRect() const override;

    virtual void updatePosition() override;

  protected:

    //! icon to be shown
    int mIconType;

    //! size
    int mIconSize;

    //! coordinates of the point in the center
    QgsPoint mCenter;

    //! color of the marker
    QColor mColor;

    //! pen width
    int mPenWidth;
};

#endif
