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
#include "qgsunittypes.h"
#include <QFontMetricsF>
#include <QPixmap>
#include "qgis_app.h"

//! An item that shows a rotated point symbol (e.g. arrow) centered to a map location together with a text displaying the rotation value
class APP_EXPORT QgsPointRotationItem: public QgsMapCanvasItem
{
  public:

    enum Orientation
    {
      Clockwise = 0,
      Counterclockwise
    };

    QgsPointRotationItem( QgsMapCanvas *canvas );

    void paint( QPainter *painter ) override;

    //! Sets the center point of the rotation symbol (in map coordinates)
    void setPointLocation( const QgsPointXY &p );

    /**
     * Sets the rotation of the symbol.
     * Units are degrees, starting from north direction, clockwise direction.
    */
    void setSymbolRotation( int r ) {mRotation = r;}

    /**
     * Sets the rotation unit.
     * \since QGIS 3.22
     */
    void setRotationUnit( const QgsUnitTypes::AngleUnit &rotationUnit );

    //! Sets rotation symbol from image (takes ownership)
    void setSymbol( const QImage &symbolImage );

    void setOrientation( Orientation o ) { mOrientation = o; }
    Orientation orientation() const { return mOrientation; }

  private:

    //! Converts rotation into QPainter rotation considering mOrientation
    int painterRotation( int rotation ) const;
    //! Clockwise (default) or counterclockwise
    Orientation mOrientation = Clockwise;
    //! Font to display the numerical rotation values
    QFont mFont;
    //! Symbol pixmap
    QPixmap mPixmap;
    int mRotation = 0.0;
    QgsUnitTypes::AngleUnit mRotationUnit = QgsUnitTypes::AngleDegrees;
    QPainterPath mArrowPath;
};

#endif // QGSPOINTROTATIONITEM_H
