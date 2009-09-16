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
class QgsPointRotationItem: public QgsMapCanvasItem
{
  public:
    QgsPointRotationItem( QgsMapCanvas* canvas );
    ~QgsPointRotationItem();

    void paint( QPainter * painter );

    /**Sets the center point of the rotation symbol (in map coordinates)*/
    void setPointLocation( const QgsPoint& p );

    /**Sets the rotation of the symbol and displays the new rotation number. \
    Units are degrees, starting from north direction, clockwise direction*/
    void setSymbolRotation( double r ) {mRotation = r;}

    /**Sets a symbol from image file*/
    void setSymbol( const QString& symbolPath );

  private:
    QgsPointRotationItem();
    /**Font to display the numerical rotation values*/
    QFont mFont;
    /**Symboll pixmap*/
    QPixmap mPixmap;
    double mRotation;
};

#endif // QGSPOINTROTATIONITEM_H
