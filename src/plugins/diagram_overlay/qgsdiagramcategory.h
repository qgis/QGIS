/***************************************************************************
                         qgsdiagramcategory.h  -  description
                         ----------------------
    begin                : December 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIAGRAMCATEGORY_H
#define QGSDIAGRAMCATEGORY_H

#include <QBrush>
#include <QPen>

/**Describes a diagram category that
can be displayed e.g. as a pie slice
or a bar pillar*/
class QgsDiagramCategory
{
  public:
    QgsDiagramCategory();
    ~QgsDiagramCategory();

    //getters
    const QBrush& brush() const {return mBrush;}
    const QPen& pen() const {return mPen;}
    int propertyIndex() const {return mPropertyIndex;}
    int gap() const {return mGap;}

    //setters
    void setBrush( const QBrush& b ) {mBrush = b;}
    void setPen( const QPen& p ) {mPen = p;}
    void setPropertyIndex( int index ) {mPropertyIndex = index;}
    void setGap( int g ) {mGap = g;}

  private:
    /**Outline to draw the category*/
    QPen mPen;
    /**Fill to draw the category*/
    QBrush mBrush;
    /**Index of the attribute represented by the category*/
    int mPropertyIndex;
    /**Gap to highlight the category (e.g. explode in pie)*/
    int mGap;
};

#endif
