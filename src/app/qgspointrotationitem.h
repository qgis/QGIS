/***************************************************************************
    qgspointrotationitem.h
    ----------------------
    begin                : September 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTROTATIONITEM_H
#define QGSPOINTROTATIONITEM_H

#include "qgsmapcanvasitem.h"
#include <QFontMetricsF>
#include <QPixmap>

/**An item that shows a rotated point symbol (e.g. arrow) centered to a map location together with a text displaying the rotation value*/
class APP_EXPORT QgsPointRotationItem: public QgsMapCanvasItem
{
  public:

    enum Orientation
    {
      Clockwise = 0,
      Counterclockwise
    };

    QgsPointRotationItem( QgsMapCanvas* canvas );
    ~QgsPointRotationItem();

    void paint( QPainter * painter );

    /**Sets the center point of the rotation symbol (in map coordinates)*/
    void setPointLocation( const QgsPoint& p );

    /**Sets the rotation of the symbol and displays the new rotation number.
    Units are degrees, starting from north direction, clockwise direction*/
    void setSymbolRotation( int r ) {mRotation = r;}

    /**Sets rotation symbol from image (takes ownership)*/
    void setSymbol( const QImage& symbolImage );

    void setOrientation( Orientation o ) { mOrientation = o; }
    Orientation orientation() const { return mOrientation; }

  private:
    QgsPointRotationItem();
    /**Converts rotation into QPainter rotation considering mOrientation*/
    int painterRotation( int rotation ) const;
    /**Clockwise (default) or counterclockwise*/
    Orientation mOrientation;
    /**Font to display the numerical rotation values*/
    QFont mFont;
    /**Symboll pixmap*/
    QPixmap mPixmap;
    int mRotation;
};

#endif // QGSPOINTROTATIONITEM_H
