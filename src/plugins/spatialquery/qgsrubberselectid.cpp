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
/*  $Id$ */

#include "qgsrubberselectid.h"

#include "qgsfeature.h"

QgsRubberSelectId::QgsRubberSelectId( QgsMapCanvas* mapCanvas )
{
  mIsPolygon = true;
  mMapCanvas = mapCanvas;
  mRubberBand = new QgsRubberBand( mMapCanvas, mIsPolygon );

} // QgsRubberSelectId::QgsRubberSelectId( QgsMapCanvas* mapCanvas, bool isPolygon = true )

QgsRubberSelectId::~QgsRubberSelectId()
{
  reset();
  delete mRubberBand;

} // QgsRubberSelectId::~QgsRubberSelectId()

void QgsRubberSelectId::isGeometryNotPolygon( bool isPolygon = false )
{
  reset();
  delete mRubberBand;
  mIsPolygon = isPolygon;
  mRubberBand = new QgsRubberBand( mMapCanvas, mIsPolygon );

} // void QgsRubberSelectId::isGeometryNotPolygon(bool isPolygon)

void QgsRubberSelectId::reset()
{
  mRubberBand->reset( mIsPolygon );

} // void QgsRubberSelectId::reset()

void QgsRubberSelectId::setColor( int colorRed, int colorGreen, int colorBlue, int width, float alfa = 0 )
{
  QColor color = QColor( colorRed, colorGreen, colorBlue );
  color.setAlpha( alfa );
  mRubberBand->setColor( color );
  mRubberBand->setWidth( width );

} // void QgsRubberSelectId::setColor(int colorRed, int colorGreen, int colorBlue, float alfa, width)

void QgsRubberSelectId::addFeature( QgsVectorLayer* mLayer, int fid )
{
  QgsFeature feat;
  if ( !mLayer->featureAtId( fid, feat, true, false ) )
  {
    return;
  }
  if ( !feat.geometry() )
  {
    return;
  }

  mRubberBand->setToGeometry( feat.geometry(), mLayer );

} // void QgsRubberSelectId::addFeature( QgsVectorLayer* mLayer, int Id )

void QgsRubberSelectId::show()
{
  mRubberBand->show();
} // QgsRubberSelectId::show()
