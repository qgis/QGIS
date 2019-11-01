/***************************************************************************
                         qgsmesh3daveraging.cpp
                         ----------------------
    begin                : November 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "qgsmesh3daveraging.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshrenderersettings.h"

///@cond PRIVATE

QgsMesh3dAveragingMethod::QgsMesh3dAveragingMethod( Method method )
  : mMethod( method )
{
}

QgsMesh3dAveragingMethod::Method QgsMesh3dAveragingMethod::method() const
{
  return mMethod;
}

QgsMesh3dAveragingMethod *QgsMesh3dAveragingMethod::create( const QgsMeshRenderer3dAveragingSettings &settings )
{
  std::unique_ptr<QgsMesh3dAveragingMethod> ret;
  switch ( settings.averagingMethod() )
  {
    case QgsMeshRenderer3dAveragingSettings::SingleVerticalLayer:
      ret.reset( new QgsMeshSingleLevelAverageMethod( settings.singleVerticalLayerSettings().verticalLayer() ) );
      break;
  }
  return ret.release();
}

bool QgsMesh3dAveragingMethod::equals( QgsMesh3dAveragingMethod *a, QgsMesh3dAveragingMethod *b )
{
  if ( !a )
    return bool( b );

  if ( !b )
    return true;

  if ( a->method() != b->method() )
    return false;

  switch ( a->method() )
  {
    case QgsMesh3dAveragingMethod::SingleLevelAverageMethod:
      return *dynamic_cast<QgsMeshSingleLevelAverageMethod *>( a ) == *dynamic_cast<QgsMeshSingleLevelAverageMethod *>( b );
  }
  return false;
}

QgsMeshSingleLevelAverageMethod::QgsMeshSingleLevelAverageMethod( int verticalLevel )
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::SingleLevelAverageMethod )
  , mVerticalLevel( verticalLevel )
{
}

QgsMeshSingleLevelAverageMethod::~QgsMeshSingleLevelAverageMethod() = default;

QgsMeshDataBlock QgsMeshSingleLevelAverageMethod::calculate( const QgsMesh3dDataBlock &block3d, QgsFeedback *feedback ) const
{
  if ( !block3d.isValid() )
    return QgsMeshDataBlock();

  bool isVector = block3d.isVector();
  int count = block3d.count();
  QgsMeshDataBlock result( isVector ? QgsMeshDataBlock::Vector2DDouble : QgsMeshDataBlock::ScalarDouble, count );
  double *buffer = static_cast<double *>( result.buffer() );
  const int *verticalLevelsCount = static_cast<const int *>( block3d.constBuffer( QgsMesh3dDataBlock::VerticalLevelsCount ) );
  const double *values = static_cast<const double *>( block3d.constBuffer( isVector ? QgsMesh3dDataBlock::VectorDouble : QgsMesh3dDataBlock::ScalarDouble ) );

  int volumeIndex = 0;
  for ( int i = 0; i < count; ++i )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return QgsMeshDataBlock();
    }
    int verticalLevels = verticalLevelsCount[i];
    if ( isVector )
    {
      if ( mVerticalLevel <= verticalLevels )
      {
        const size_t index = volumeIndex + mVerticalLevel;
        buffer[ 2 * i ] = values[ 2 * index ];
        buffer[ 2 * i + 1 ] = values[ 2 * index + 1 ];
      }
      else
      {
        buffer[ 2 * i ] = std::numeric_limits<double>::quiet_NaN();
        buffer[ 2 * i + 1 ] = std::numeric_limits<double>::quiet_NaN();
      }
    }
    else
    {
      if ( mVerticalLevel <= verticalLevels )
      {
        const size_t index = volumeIndex + mVerticalLevel;
        buffer[i] = values[index];
      }
      else
      {
        buffer[i] = std::numeric_limits<double>::quiet_NaN();
      }
    }
    volumeIndex += verticalLevels;
  }
  return result;
}

bool QgsMeshSingleLevelAverageMethod::operator==( const QgsMeshSingleLevelAverageMethod &rhs ) const
{
  return mVerticalLevel == rhs.verticalLevel();
}

int QgsMeshSingleLevelAverageMethod::verticalLevel() const
{
  return mVerticalLevel;
}

///@endcond
