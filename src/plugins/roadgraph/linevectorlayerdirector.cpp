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
 * \file linevectorlayerdirector.cpp
 * \brief implementation of RgLineVectorLayerDirector
 */

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

RgLineVectorLayerDirector::RgLineVectorLayerDirector( const QString& layerId,
    int directionFieldId,
    const QString& directDirectionValue,
    const QString& reverseDirectionValue,
    const QString& bothDirectionValue,
    int defaultDirection,
    const QString& speedUnitName,
    int speedFieldId,
    double defaultSpeed )
{
  mLayerId                = layerId;
  mDirectionFieldId       = directionFieldId;
  mDirectDirectionValue   = directDirectionValue;
  mReverseDirectionValue  = reverseDirectionValue;
  mDefaultDirection       = defaultDirection;
  mBothDirectionValue     = bothDirectionValue;
  mSpeedUnitName          = speedUnitName;
  mSpeedFieldId           = speedFieldId;
  mDefaultSpeed           = defaultSpeed;
}

RgLineVectorLayerDirector::~RgLineVectorLayerDirector()
{
}

QString RgLineVectorLayerDirector::name() const
{
  return QString( "Vector line" );
}

void RgLineVectorLayerDirector::makeGraph( RgGraphBuilder *builder ) const
{
  QgsVectorLayer *vl = myLayer();

  if ( vl == NULL )
    return;

  builder->setSourceCrs( vl->crs() );
  QgsAttributeList la;
  la.push_back( mDirectionFieldId );
  la.push_back( mSpeedFieldId );

  SpeedUnit su = SpeedUnit::byName( mSpeedUnitName );

  vl->select( la );
  QgsFeature feature;
  while ( vl->nextFeature( feature ) )
  {
    QgsAttributeMap attr = feature.attributeMap();
    int directionType = mDefaultDirection;
    QgsAttributeMap::const_iterator it;
    // What direction have feature?
    for ( it = attr.constBegin(); it != attr.constEnd(); ++it )
    {
      if ( it.key() != mDirectionFieldId )
      {
        continue;
      }
      QString str = it.value().toString();
      if ( str == mBothDirectionValue )
      {
        directionType = 3;
      }
      else if ( str == mDirectDirectionValue )
      {
        directionType = 1;
      }
      else if ( str == mReverseDirectionValue )
      {
        directionType = 2;
      }
    }
    // What speed have feature?
    double speed = 0.0;
    for ( it = attr.constBegin(); it != attr.constEnd(); ++it )
    {
      if ( it.key() != mSpeedFieldId )
      {
        continue;
      }
      speed = it.value().toDouble();
    }
    if ( speed <= 0.0 )
    {
      speed = mDefaultSpeed;
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
        if ( directionType == 1 ||
             directionType  == 3 )
        {
          builder->addArc( pt1, pt2, speed*su.multipler() );
        }
        if ( directionType == 2 ||
             directionType == 3 )
        {
          builder->addArc( pt2, pt1, speed*su.multipler() );
        }
      }
      pt1 = pt2;
      isFirstPoint = false;
    } // for (it = pl.begin(); it != pl.end(); ++it)

  } // while( vl->nextFeature(feature) )
} // makeGraph( RgGraphBuilder *builder, const QgsRectangle& rt )

QgsVectorLayer* RgLineVectorLayerDirector::myLayer() const
{
  QMap <QString, QgsMapLayer*> m = QgsMapLayerRegistry::instance()->mapLayers();
  QMap <QString, QgsMapLayer*>::const_iterator it = m.find( mLayerId );
  if ( it == m.end() )
  {
    return NULL;
  }
  // return NULL if it.value() isn't QgsVectorLayer()
  return dynamic_cast<QgsVectorLayer*>( it.value() );
}
