/***************************************************************************
 *   Copyright (C) 2010 by Sergey Yakushev                                 *
 *   yakushevs <at> list.ru                                                *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file linevectorlayersettins.cpp
 * \brief implementation of RgLineVectorLayerDirector
 */

#include "linevectorlayersettings.h"
#include "linevectorlayerdirector.h"
#include "graphbuilder.h"
#include "units.h"

// Qgis includes
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgspoint.h>
#include <qgsgeometry.h>

// QT includes
#include <QString>

//standard includes
#include <iostream>

RgLineVectorLayerDirector::RgLineVectorLayerDirector()
{
}
RgLineVectorLayerDirector::~RgLineVectorLayerDirector()
{
}
RgSettings* RgLineVectorLayerDirector::settings()
{
  return &mSettings;
}

QString RgLineVectorLayerDirector::name() const
{
  return QString( "Vector line" );
}

void RgLineVectorLayerDirector::makeGraph( RgGraphBuilder *builder ) const
{
  QgsVectorLayer *vl = NULL;
  QMap< QString, QgsMapLayer*> m = QgsMapLayerRegistry::instance()->mapLayers();
  QMap< QString, QgsMapLayer*>::const_iterator it;
  for ( it = m.constBegin(); it != m.constEnd(); ++it )
  {
    if ( it.value()->name() == mSettings.mLayer )
    {
      vl = dynamic_cast<QgsVectorLayer*>( it.value() );
      break;
    }
  }
  if ( vl == NULL )
    return;

  QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider*>( vl->dataProvider() );
  if ( provider == NULL )
    return;

  int directionFieldId = provider->fieldNameIndex( mSettings.mDirection );
  int speedFieldId     = provider->fieldNameIndex( mSettings.mSpeed );

  builder->setSourceCrs( vl->crs() );
  QgsAttributeList la;
  if ( directionFieldId > -1 )
    la.push_back( directionFieldId );
  if ( speedFieldId > -1 )
    la.push_back( speedFieldId );

  SpeedUnit su = SpeedUnit::byName( mSettings.mSpeedUnitName );

  vl->select( la );
  QgsFeature feature;
  while ( vl->nextFeature( feature ) )
  {
    QgsAttributeMap attr = feature.attributeMap();
    RgLineVectorLayerSettings::DirectionType directionType = mSettings.mDefaultDirection;
    QgsAttributeMap::const_iterator it;
    // What direction have feature?
    for ( it = attr.constBegin(); it != attr.constEnd(); ++it )
    {
      if ( it.key() != directionFieldId )
      {
        continue;
      }
      QString str = it.value().toString();
      if ( str == mSettings.mBothDirectionVal )
      {
        directionType = RgLineVectorLayerSettings::Both;
      }
      else if ( str == mSettings.mFirstPointToLastPointDirectionVal )
      {
        directionType = RgLineVectorLayerSettings::FirstPointToLastPoint;
      }
      else if ( str == mSettings.mLastPointToFirstPointDirectionVal )
      {
        directionType = RgLineVectorLayerSettings::LastPointToFirstPoint;
      }
    }
    // What speed have feature?
    double speed = 0.0;
    for ( it = attr.constBegin(); it != attr.constEnd(); ++it )
    {
      if ( it.key() != speedFieldId )
      {
        continue;
      }
      speed = it.value().toDouble();
    }
    if ( speed <= 0.0 )
    {
      speed = mSettings.mDefaultSpeed;
    }

    // begin features segments and add arc to the Graph;
    QgsPoint pt1, pt2;

    bool isFirstPoint = true;
    QgsPolyline pl = feature.geometry()->asPolyline();
    QgsPolyline::iterator pointIt;
    for ( pointIt = pl.begin(); pointIt != pl.end(); ++pointIt )
    {
      pt2 = *pointIt;
      if ( !isFirstPoint )
      {
        if ( directionType == RgLineVectorLayerSettings::FirstPointToLastPoint ||
             directionType  == RgLineVectorLayerSettings::Both )
        {
          builder->addArc( pt1, pt2, speed*su.multipler() );
        }
        if ( directionType == RgLineVectorLayerSettings::LastPointToFirstPoint ||
             directionType == RgLineVectorLayerSettings::Both )
        {
          builder->addArc( pt2, pt1, speed*su.multipler() );
        }
      }
      pt1 = pt2;
      isFirstPoint = false;
    } // for (it = pl.begin(); it != pl.end(); ++it)

  } // while( vl->nextFeature(feature) )
} // makeGraph( RgGraphBuilder *builder, const QgsRectangle& rt )
