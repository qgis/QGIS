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

QgsMesh3dAveragingMethod::QgsMesh3dAveragingMethod( Method method )
  : mMethod( method )
{
}

QgsMesh3dAveragingMethod *QgsMesh3dAveragingMethod::createFromXml( const QDomElement &elem )
{
  std::unique_ptr<QgsMesh3dAveragingMethod> ret;

  QgsMesh3dAveragingMethod::Method method = static_cast<QgsMesh3dAveragingMethod::Method>(
        elem.attribute( QStringLiteral( "method" ) ).toInt() );
  switch ( method )
  {
    case QgsMesh3dAveragingMethod::SingleLevelAverageMethod:
      ret.reset( new QgsMeshSingleLevelAveragingMethod() );
      ret->readXml( elem );
  }

  return ret.release();
}

QgsMesh3dAveragingMethod::Method QgsMesh3dAveragingMethod::method() const
{
  return mMethod;
}

bool QgsMesh3dAveragingMethod::equals( const QgsMesh3dAveragingMethod *a, const QgsMesh3dAveragingMethod *b )
{
  if ( a )
    return a->equals( b );
  else
    return !b;
}

QgsMeshSingleLevelAveragingMethod::QgsMeshSingleLevelAveragingMethod()
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::SingleLevelAverageMethod )
{
}

QgsMeshSingleLevelAveragingMethod::QgsMeshSingleLevelAveragingMethod( int verticalLevel )
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::SingleLevelAverageMethod )
  , mVerticalLevel( verticalLevel )
{
}

QgsMeshSingleLevelAveragingMethod::~QgsMeshSingleLevelAveragingMethod() = default;

bool QgsMeshSingleLevelAveragingMethod::equals( const QgsMesh3dAveragingMethod *other ) const
{
  if ( !other || other->method() != method() )
    return false;

  const QgsMeshSingleLevelAveragingMethod *otherMethod = static_cast<const QgsMeshSingleLevelAveragingMethod *>( other );

  return otherMethod->verticalLevel() == verticalLevel();
}

QgsMesh3dAveragingMethod *QgsMeshSingleLevelAveragingMethod::clone() const
{
  return new QgsMeshSingleLevelAveragingMethod( verticalLevel() );
}

QgsMeshDataBlock QgsMeshSingleLevelAveragingMethod::calculate( const QgsMesh3dDataBlock &block3d, QgsFeedback *feedback ) const
{
  if ( !block3d.isValid() )
    return QgsMeshDataBlock();

  bool isVector = block3d.isVector();
  int count = block3d.count();
  QgsMeshDataBlock result( isVector ? QgsMeshDataBlock::Vector2DDouble : QgsMeshDataBlock::ScalarDouble, count );
  QVector<double> valuesFaces( isVector ? 2 * count : count );
  const QVector<int> verticalLevelsCount = block3d.verticalLevelsCount();
  const QVector<double> volumeValues = block3d.values();

  int startVolumeIndex = 0;
  for ( int faceIndex = 0; faceIndex < count; ++faceIndex )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return QgsMeshDataBlock();
    }
    int volumesBelowFaceCount = verticalLevelsCount[faceIndex];
    if ( isVector )
    {
      if ( mVerticalLevel < volumesBelowFaceCount )
      {
        const int volumeIndex = startVolumeIndex + mVerticalLevel;
        valuesFaces[ 2 * faceIndex ] = volumeValues[ 2 * volumeIndex ];
        valuesFaces[ 2 * faceIndex + 1 ] = volumeValues[ 2 * volumeIndex + 1 ];
      }
      else
      {
        valuesFaces[ 2 * faceIndex ] = std::numeric_limits<double>::quiet_NaN();
        valuesFaces[ 2 * faceIndex + 1 ] = std::numeric_limits<double>::quiet_NaN();
      }
    }
    else
    {
      if ( mVerticalLevel < volumesBelowFaceCount )
      {
        const int volumeIndex = startVolumeIndex + mVerticalLevel;
        valuesFaces[faceIndex] = volumeValues[volumeIndex];
      }
      else
      {
        valuesFaces[faceIndex] = std::numeric_limits<double>::quiet_NaN();
      }
    }
    startVolumeIndex += volumesBelowFaceCount;
  }
  result.setValues( valuesFaces );
  return result;
}

QDomElement QgsMeshSingleLevelAveragingMethod::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "single-vertical-layer-settings" ) );
  elem.setAttribute( QStringLiteral( "layer-index" ), verticalLevel() );
  return elem;
}

void QgsMeshSingleLevelAveragingMethod::readXml( const QDomElement &elem )
{
  mVerticalLevel = elem.attribute( QStringLiteral( "layer-index" ) ).toInt();
}

int QgsMeshSingleLevelAveragingMethod::verticalLevel() const
{
  return mVerticalLevel;
}
