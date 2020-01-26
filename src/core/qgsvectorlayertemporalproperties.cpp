/***************************************************************************
                         qgsvectorlayertemporalproperties.cpp
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayertemporalproperties.h"

QgsVectorLayerTemporalProperties::QgsVectorLayerTemporalProperties( bool enabled )
  : QgsMapLayerTemporalProperties( enabled )
{
}

QgsVectorLayerTemporalProperties::TemporalMode QgsVectorLayerTemporalProperties::mode() const
{
  return mMode;
}

void QgsVectorLayerTemporalProperties::setMode( QgsVectorLayerTemporalProperties::TemporalMode mode )
{
  if ( mMode == mode )
    return;
  mMode = mode;
}

void  QgsVectorLayerTemporalProperties::setFixedTemporalRange( const QgsDateTimeRange &range )
{
  if ( range == mRange )
    return;

  mRange = range;
}

const QgsDateTimeRange &QgsVectorLayerTemporalProperties::fixedTemporalRange() const
{
  return mRange;
}

void  QgsVectorLayerTemporalProperties::setStartTimeField( const QString &field )
{
  if ( field == mStartTimeField )
    return;

  mStartTimeField = field;
}

QString QgsVectorLayerTemporalProperties::startTimeField() const
{
  return mStartTimeField;
}

