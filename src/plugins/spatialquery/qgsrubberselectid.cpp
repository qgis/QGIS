/***************************************************************************
                          qgsrubberselectid.cpp
    A plugin that makes spatial queries on vector layers
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: qgsrubberselectid.cpp 13377 2010-04-25 01:07:36Z jef $ */

#include "qgsrubberselectid.h"

#include "qgsfeature.h"

QgsRubberSelectId::QgsRubberSelectId( QgsMapCanvas* mapCanvas )
{
  mIsPolygon = false;
  mMapCanvas = mapCanvas;
  mRubberBand = new QgsRubberBand( mMapCanvas, mIsPolygon );
  mColorRGB[0] = 255;
  mColorRGB[1] = 0;
  mColorRGB[2] = 0;
  mWidth  = 2;
  setStyle();
} // QgsRubberSelectId::QgsRubberSelectId( QgsMapCanvas* mapCanvas, bool isPolygon = true )

QgsRubberSelectId::~QgsRubberSelectId()
{
  reset();
  delete mRubberBand;

} // QgsRubberSelectId::~QgsRubberSelectId()

void QgsRubberSelectId::reset()
{
    mRubberBand->reset( mIsPolygon );
} // void QgsRubberSelectId::reset()

void QgsRubberSelectId::setStyle( int colorRed, int colorGreen, int colorBlue, int width )
{
  mColorRGB[0] = colorRed;
  mColorRGB[1] = colorGreen;
  mColorRGB[2] = colorBlue;
  mWidth  = width;
  setStyle();
} // void QgsRubberSelectId::setColor(int colorRed, int colorGreen, int colorBlue, float alfa, width)

void QgsRubberSelectId::addFeature( QgsVectorLayer* lyr, int fid )
{
  bool isPolygon = ( lyr->geometryType() == QGis::Polygon );
  if( mIsPolygon != isPolygon)
  {
    reset();
    delete mRubberBand;
    mIsPolygon = isPolygon;
    mRubberBand = new QgsRubberBand( mMapCanvas, mIsPolygon );
    setStyle();
  }
  QgsFeature feat;
  if ( !lyr->featureAtId( fid, feat, true, false ) )
  {
    return;
  }
  if ( !feat.geometry() )
  {
    return;
  }
  mRubberBand->setToGeometry( feat.geometry(), lyr );
} // void QgsRubberSelectId::addFeature( QgsVectorLayer* mLayer, int Id )

void QgsRubberSelectId::show()
{
  mRubberBand->show();
} // QgsRubberSelectId::show()

void QgsRubberSelectId::setStyle()
{
  QColor color = QColor( mColorRGB[0], mColorRGB[1], mColorRGB[2] );
  mRubberBand->setColor( color );
  mRubberBand->setWidth( mWidth );
}
