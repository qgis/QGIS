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
#include "qgsfeedback.h"

// threshold for length intervals, to avoid division by 0
static const double eps = 1e-6;

QgsMesh3dAveragingMethod::QgsMesh3dAveragingMethod( Method method )
  : mMethod( method )
{
}

QgsMesh3dAveragingMethod *QgsMesh3dAveragingMethod::createFromXml( const QDomElement &elem )
{
  std::unique_ptr<QgsMesh3dAveragingMethod> ret;

  const QgsMesh3dAveragingMethod::Method method = static_cast<QgsMesh3dAveragingMethod::Method>(
        elem.attribute( QStringLiteral( "method" ) ).toInt() );
  switch ( method )
  {
    case QgsMesh3dAveragingMethod::MultiLevelsAveragingMethod:
      ret.reset( new QgsMeshMultiLevelsAveragingMethod() );
      break;
    case QgsMesh3dAveragingMethod::SigmaAveragingMethod:
      ret.reset( new QgsMeshSigmaAveragingMethod() );
      break;
    case QgsMesh3dAveragingMethod::RelativeHeightAveragingMethod:
      ret.reset( new QgsMeshRelativeHeightAveragingMethod() );
      break;
    case QgsMesh3dAveragingMethod::ElevationAveragingMethod:
      ret.reset( new QgsMeshElevationAveragingMethod() );
      break;
  }
  ret->readXml( elem );
  return ret.release();
}

QgsMeshDataBlock QgsMesh3dAveragingMethod::calculate( const QgsMesh3dDataBlock &block3d, QgsFeedback *feedback ) const
{
  if ( !block3d.isValid() )
    return QgsMeshDataBlock();

  if ( !hasValidInputs() )
    return QgsMeshDataBlock();

  const bool isVector = block3d.isVector();
  const int count = block3d.count();
  QgsMeshDataBlock result( isVector ? QgsMeshDataBlock::Vector2DDouble : QgsMeshDataBlock::ScalarDouble, count );
  QVector<double> valuesFaces( isVector ? 2 * count : count, std::numeric_limits<double>::quiet_NaN() );
  const QVector<int> verticalLevelsCount = block3d.verticalLevelsCount();
  const QVector<double> verticalLevels = block3d.verticalLevels();
  const QVector<double> volumeValues = block3d.values();

  int startVolumeIndex = 0;
  for ( int faceIndex = 0; faceIndex < count; ++faceIndex )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return QgsMeshDataBlock();
    }

    const int volumesBelowFaceCount = verticalLevelsCount[faceIndex];
    if ( volumesBelowFaceCount <= 0 )
      continue;

    const int startVerticalLevelIndex = startVolumeIndex + faceIndex;
    Q_ASSERT( verticalLevels.size() >= startVerticalLevelIndex + volumesBelowFaceCount + 1 );
    QVector<double> verticalLevelsForFace = verticalLevels.mid( startVerticalLevelIndex, volumesBelowFaceCount + 1 );
    double faceLevelTop = verticalLevelsForFace[0];
    double faceLevelBottom = verticalLevelsForFace[verticalLevelsForFace.size() - 1];

    // the level is value below surface, so top level (-0.1m) is usually higher number than bottom level (e.g. -1.2m)
    if ( faceLevelTop < faceLevelBottom )
    {
      std::swap( faceLevelTop, faceLevelBottom );
    }

    double methodLevelTop = std::numeric_limits<double>::quiet_NaN();
    double methodLevelBottom = std::numeric_limits<double>::quiet_NaN();

    int singleVerticalIndex = -1;
    volumeRangeForFace( methodLevelTop,
                        methodLevelBottom,
                        singleVerticalIndex,
                        verticalLevelsForFace );

    if ( singleVerticalIndex != -1 )
    {
      int volumeIndex = singleVerticalIndex + startVolumeIndex;
      if ( isVector )
      {
        valuesFaces[2 * faceIndex] = volumeValues.at( 2 * volumeIndex );
        valuesFaces[2 * faceIndex + 1 ] = volumeValues.at( 2 * volumeIndex + 1 );
      }
      else
      {
        valuesFaces[faceIndex] = volumeValues.at( volumeIndex );
      }
    }
    else if ( !std::isnan( methodLevelTop ) && !std::isnan( methodLevelBottom ) )
    {
      // the level is value below surface, so top level (-0.1m) is usually higher number than bottom level (e.g. -1.2m)
      if ( methodLevelTop < methodLevelBottom )
      {
        std::swap( methodLevelTop, methodLevelBottom );
      }

      // check if we are completely outside the limits
      if ( ( methodLevelTop >= faceLevelBottom ) && ( methodLevelBottom <= faceLevelTop ) )
      {
        averageVolumeValuesForFace(
          faceIndex,
          volumesBelowFaceCount,
          startVolumeIndex,
          methodLevelTop,
          methodLevelBottom,
          isVector,
          verticalLevelsForFace,
          volumeValues,
          valuesFaces
        );
      }
    }

    // move to next face and associated volumes
    startVolumeIndex += volumesBelowFaceCount;
  }
  result.setValues( valuesFaces );
  return result;
}

QgsMesh3dAveragingMethod::Method QgsMesh3dAveragingMethod::method() const
{
  return mMethod;
}

void QgsMesh3dAveragingMethod::averageVolumeValuesForFace(
  int faceIndex,
  int volumesBelowFaceCount,
  int startVolumeIndex,
  double methodLevelTop,
  double methodLevelBottom,
  bool isVector,
  const QVector<double> &verticalLevelsForFace,
  const QVector<double> &volumeValues,
  QVector<double> &valuesFaces
) const
{
  double totalAveragedHeight = 0;
  double nSumX = 0.0;
  double nSumY = 0.0;

  // Now go through all volumes below face and check if we need to take that volume into consideration
  for ( int relativeVolumeIndex = 0; relativeVolumeIndex < volumesBelowFaceCount; ++relativeVolumeIndex )
  {
    const int volumeIndex = startVolumeIndex + relativeVolumeIndex;
    double volumeLevelTop = verticalLevelsForFace[relativeVolumeIndex];
    double volumeLevelBottom = verticalLevelsForFace[relativeVolumeIndex + 1];
    if ( volumeLevelTop < volumeLevelBottom )
    {
      std::swap( volumeLevelTop, volumeLevelBottom );
    }

    const double intersectionLevelTop = std::min( methodLevelTop, volumeLevelTop );
    const double intersectionLevelBottom = std::max( methodLevelBottom, volumeLevelBottom );
    const double effectiveInterval = intersectionLevelTop - intersectionLevelBottom;

    if ( effectiveInterval > eps )
    {
      if ( isVector )
      {
        const double x = volumeValues[2 * volumeIndex ];
        const double y = volumeValues[ 2 * volumeIndex + 1 ];
        if ( ! std::isnan( x ) &&
             ! std::isnan( y )
           )
        {
          nSumX += x * effectiveInterval;
          nSumY += y * effectiveInterval;
          totalAveragedHeight += effectiveInterval;
        }
      }
      else
      {
        const double x = volumeValues[ volumeIndex ];
        if ( ! std::isnan( x ) )
        {
          nSumX += x * effectiveInterval;
          totalAveragedHeight += effectiveInterval;
        }
      }
    }
  }

  // calculate average
  if ( totalAveragedHeight > eps )
  {
    if ( isVector )
    {
      valuesFaces[2 * faceIndex] = nSumX / totalAveragedHeight;
      valuesFaces[2 * faceIndex + 1 ] = nSumY / totalAveragedHeight;
    }
    else
    {
      valuesFaces[faceIndex] = nSumX / totalAveragedHeight;
    }
  }
}

bool QgsMesh3dAveragingMethod::equals( const QgsMesh3dAveragingMethod *a, const QgsMesh3dAveragingMethod *b )
{
  if ( a )
    return a->equals( b );
  else
    return !b;
}

QgsMeshMultiLevelsAveragingMethod::QgsMeshMultiLevelsAveragingMethod( int startLevel, int endLevel, bool countedFromTop )
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::MultiLevelsAveragingMethod )
  , mStartVerticalLevel( startLevel )
  , mEndVerticalLevel( endLevel )
  , mCountedFromTop( countedFromTop )
{
  if ( mStartVerticalLevel > mEndVerticalLevel )
  {
    std::swap( mStartVerticalLevel, mEndVerticalLevel );
  }
}

QgsMeshMultiLevelsAveragingMethod::QgsMeshMultiLevelsAveragingMethod()
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::MultiLevelsAveragingMethod )
{
}

QgsMeshMultiLevelsAveragingMethod::QgsMeshMultiLevelsAveragingMethod( int verticalLevel, bool countedFromTop )
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::MultiLevelsAveragingMethod )
  , mStartVerticalLevel( verticalLevel )
  , mEndVerticalLevel( verticalLevel )
  , mCountedFromTop( countedFromTop )
{
}

QgsMeshMultiLevelsAveragingMethod::~QgsMeshMultiLevelsAveragingMethod() = default;

QDomElement QgsMeshMultiLevelsAveragingMethod::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "multi-vertical-layers-settings" ) );
  elem.setAttribute( QStringLiteral( "start-layer-index" ), startVerticalLevel() );
  elem.setAttribute( QStringLiteral( "end-layer-index" ), endVerticalLevel() );
  return elem;
}

void QgsMeshMultiLevelsAveragingMethod::readXml( const QDomElement &elem )
{
  const QDomElement settings = elem.firstChildElement( QStringLiteral( "multi-vertical-layers-settings" ) );
  if ( !settings.isNull() )
  {
    mStartVerticalLevel = settings.attribute( QStringLiteral( "start-layer-index" ) ).toInt();
    mEndVerticalLevel = settings.attribute( QStringLiteral( "end-layer-index" ) ).toInt();
    if ( mStartVerticalLevel > mEndVerticalLevel )
    {
      std::swap( mStartVerticalLevel, mEndVerticalLevel );
    }
  }
}

bool QgsMeshMultiLevelsAveragingMethod::equals( const QgsMesh3dAveragingMethod *other ) const
{
  if ( !other || other->method() != method() )
    return false;

  const QgsMeshMultiLevelsAveragingMethod *otherMethod = static_cast<const QgsMeshMultiLevelsAveragingMethod *>( other );

  return ( otherMethod->startVerticalLevel() == startVerticalLevel() ) &&
         ( otherMethod->endVerticalLevel() == endVerticalLevel() ) &&
         ( otherMethod->countedFromTop() == countedFromTop() );
}

QgsMesh3dAveragingMethod *QgsMeshMultiLevelsAveragingMethod::clone() const
{
  return new QgsMeshMultiLevelsAveragingMethod( startVerticalLevel(), endVerticalLevel(), countedFromTop() );
}


int QgsMeshMultiLevelsAveragingMethod::startVerticalLevel() const
{
  return mStartVerticalLevel;
}

int QgsMeshMultiLevelsAveragingMethod::endVerticalLevel() const
{
  return mEndVerticalLevel;
}

bool QgsMeshMultiLevelsAveragingMethod::hasValidInputs() const
{
  return mStartVerticalLevel >= 1 && mEndVerticalLevel >= mStartVerticalLevel;
}

void QgsMeshMultiLevelsAveragingMethod::volumeRangeForFace( double &startVerticalLevel,
    double &endVerticalLevel,
    int &singleVerticalIndex,
    const QVector<double> &verticalLevels ) const
{
  Q_ASSERT( mStartVerticalLevel <= mEndVerticalLevel );

  if ( countedFromTop() )
  {
    const int startIndex = mStartVerticalLevel - 1;
    if ( mStartVerticalLevel == mEndVerticalLevel )
    {
      if ( startIndex >= 0 && startIndex < verticalLevels.size() - 1 )
        singleVerticalIndex = startIndex;
    }
    else
    {
      if ( startIndex >= 0 && startIndex < verticalLevels.size() )
      {
        startVerticalLevel = verticalLevels[ startIndex ];
      }

      if ( mEndVerticalLevel >= 0 && mEndVerticalLevel < verticalLevels.size() )
      {
        endVerticalLevel = verticalLevels[ mEndVerticalLevel ];
      }
      else
      {
        endVerticalLevel = verticalLevels[ verticalLevels.size() - 1 ];
      }
    }
  }
  else
  {
    const int volumesBelowFaceCount = verticalLevels.size() - 1;
    const int startIndex = volumesBelowFaceCount - mEndVerticalLevel;
    if ( mStartVerticalLevel == mEndVerticalLevel )
    {
      if ( startIndex >= 0 && startIndex < verticalLevels.size() - 1 )
        singleVerticalIndex = startIndex;
    }
    else
    {
      if ( startIndex >= 0 && startIndex < verticalLevels.size() )
      {
        startVerticalLevel = verticalLevels[ startIndex ];
      }
      else
      {
        startVerticalLevel = verticalLevels[ 0 ];
      }

      const int endIndex = volumesBelowFaceCount - mStartVerticalLevel + 1;
      if ( endIndex >= 0 && endIndex < verticalLevels.size() )
      {
        endVerticalLevel = verticalLevels[ endIndex ];
      }
    }
  }
}

QgsMeshSigmaAveragingMethod::QgsMeshSigmaAveragingMethod()
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::SigmaAveragingMethod )
{
}

QgsMeshSigmaAveragingMethod::QgsMeshSigmaAveragingMethod( double startFraction, double endFraction )
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::SigmaAveragingMethod )
  , mStartFraction( startFraction )
  , mEndFraction( endFraction )
{
  if ( mStartFraction > mEndFraction )
  {
    std::swap( mStartFraction, mEndFraction );
  }
}

QgsMeshSigmaAveragingMethod::~QgsMeshSigmaAveragingMethod() = default;

QDomElement QgsMeshSigmaAveragingMethod::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "sigma-settings" ) );
  elem.setAttribute( QStringLiteral( "start-fraction" ), startFraction() );
  elem.setAttribute( QStringLiteral( "end-fraction" ), endFraction() );
  return elem;
}

void QgsMeshSigmaAveragingMethod::readXml( const QDomElement &elem )
{
  const QDomElement settings = elem.firstChildElement( QStringLiteral( "sigma-settings" ) );
  if ( !settings.isNull() )
  {
    mStartFraction = settings.attribute( QStringLiteral( "start-fraction" ) ).toDouble();
    mEndFraction = settings.attribute( QStringLiteral( "end-fraction" ) ).toDouble();
    if ( mStartFraction > mEndFraction )
    {
      std::swap( mStartFraction, mEndFraction );
    }
  }
}

bool QgsMeshSigmaAveragingMethod::equals( const QgsMesh3dAveragingMethod *other ) const
{
  if ( !other || other->method() != method() )
    return false;

  const QgsMeshSigmaAveragingMethod *otherMethod = static_cast<const QgsMeshSigmaAveragingMethod *>( other );

  return qgsDoubleNear( otherMethod->startFraction(), startFraction() ) && qgsDoubleNear( otherMethod->endFraction(), endFraction() ) ;
}

QgsMesh3dAveragingMethod *QgsMeshSigmaAveragingMethod::clone() const
{
  return new QgsMeshSigmaAveragingMethod( startFraction(), endFraction() );
}

double QgsMeshSigmaAveragingMethod::startFraction() const
{
  return mStartFraction;
}

double QgsMeshSigmaAveragingMethod::endFraction() const
{
  return mEndFraction;
}

bool QgsMeshSigmaAveragingMethod::hasValidInputs() const
{
  return mStartFraction >= 0 && mEndFraction >= mStartFraction && mEndFraction <= 1;
}

void QgsMeshSigmaAveragingMethod::volumeRangeForFace( double &startVerticalLevel,
    double &endVerticalLevel,
    int &,
    const QVector<double> &verticalLevels ) const
{
  const double top = verticalLevels[ 0 ];
  const double bot = verticalLevels[ verticalLevels.size() - 1 ];
  const double diff = top - bot;

  if ( mStartFraction < 0 )
    startVerticalLevel = bot;
  else
    startVerticalLevel = bot + diff * mStartFraction;

  if ( mEndFraction > 1 )
    endVerticalLevel = top;
  else
    endVerticalLevel = bot + diff * mEndFraction;
}

bool QgsMeshMultiLevelsAveragingMethod::countedFromTop() const
{
  return mCountedFromTop;
}

bool QgsMeshMultiLevelsAveragingMethod::isSingleLevel() const
{
  return mStartVerticalLevel == mEndVerticalLevel;
}


QgsMeshRelativeHeightAveragingMethod::QgsMeshRelativeHeightAveragingMethod()
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::RelativeHeightAveragingMethod )
{
}

QgsMeshRelativeHeightAveragingMethod::QgsMeshRelativeHeightAveragingMethod( double startDepth, double endDepth, bool countedFromTop )
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::RelativeHeightAveragingMethod )
  , mStartHeight( startDepth )
  , mEndHeight( endDepth )
  , mCountedFromTop( countedFromTop )
{
  if ( mStartHeight > mEndHeight )
  {
    std::swap( mStartHeight, mEndHeight );
  }
}

QgsMeshRelativeHeightAveragingMethod::~QgsMeshRelativeHeightAveragingMethod() = default;

QDomElement QgsMeshRelativeHeightAveragingMethod::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "relative-height-settings" ) );
  elem.setAttribute( QStringLiteral( "start-height" ), startHeight() );
  elem.setAttribute( QStringLiteral( "end-height" ), endHeight() );
  return elem;
}

void QgsMeshRelativeHeightAveragingMethod::readXml( const QDomElement &elem )
{
  const QDomElement settings = elem.firstChildElement( QStringLiteral( "relative-height-settings" ) );
  if ( !settings.isNull() )
  {
    mStartHeight = settings.attribute( QStringLiteral( "start-height" ) ).toDouble();
    mEndHeight = settings.attribute( QStringLiteral( "end-height" ) ).toDouble();
    if ( mStartHeight > mEndHeight )
    {
      std::swap( mStartHeight, mEndHeight );
    }
  }
}

bool QgsMeshRelativeHeightAveragingMethod::equals( const QgsMesh3dAveragingMethod *other ) const
{
  if ( !other || other->method() != method() )
    return false;

  const QgsMeshRelativeHeightAveragingMethod *otherMethod = static_cast<const QgsMeshRelativeHeightAveragingMethod *>( other );

  return qgsDoubleNear( otherMethod->startHeight(), startHeight() ) &&
         qgsDoubleNear( otherMethod->endHeight(), endHeight() ) &&
         otherMethod->countedFromTop() == countedFromTop();
}

QgsMesh3dAveragingMethod *QgsMeshRelativeHeightAveragingMethod::clone() const
{
  return new QgsMeshRelativeHeightAveragingMethod( startHeight(), endHeight(), countedFromTop() );
}

double QgsMeshRelativeHeightAveragingMethod::startHeight() const
{
  return mStartHeight;
}

double QgsMeshRelativeHeightAveragingMethod::endHeight() const
{
  return mEndHeight;
}

bool QgsMeshRelativeHeightAveragingMethod::hasValidInputs() const
{
  return mStartHeight >= 0 && mEndHeight >= mStartHeight;
}

void QgsMeshRelativeHeightAveragingMethod::volumeRangeForFace( double &startVerticalLevel,
    double &endVerticalLevel,
    int &,
    const QVector<double> &verticalLevels ) const
{
  if ( countedFromTop() )
  {
    const double top = verticalLevels[ 0 ];
    startVerticalLevel = top - mStartHeight;
    endVerticalLevel = top - mEndHeight;
  }
  else
  {
    const double bot = verticalLevels[verticalLevels.size() - 1];
    startVerticalLevel = bot + mStartHeight;
    endVerticalLevel = bot + mEndHeight;
  }
}

bool QgsMeshRelativeHeightAveragingMethod::countedFromTop() const
{
  return mCountedFromTop;
}

QgsMeshElevationAveragingMethod::QgsMeshElevationAveragingMethod()
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::ElevationAveragingMethod )
{
}

QgsMeshElevationAveragingMethod::QgsMeshElevationAveragingMethod( double startElevation, double endElevation )
  : QgsMesh3dAveragingMethod( QgsMesh3dAveragingMethod::ElevationAveragingMethod )
  , mStartElevation( startElevation )
  , mEndElevation( endElevation )
{
  if ( mEndElevation > mStartElevation )
  {
    std::swap( mEndElevation, mStartElevation );
  }
}

QgsMeshElevationAveragingMethod::~QgsMeshElevationAveragingMethod() = default;

QDomElement QgsMeshElevationAveragingMethod::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "elevation-settings" ) );
  elem.setAttribute( QStringLiteral( "start-elevation" ), startElevation() );
  elem.setAttribute( QStringLiteral( "end-elevation" ), endElevation() );
  return elem;
}

void QgsMeshElevationAveragingMethod::readXml( const QDomElement &elem )
{
  const QDomElement settings = elem.firstChildElement( QStringLiteral( "elevation-settings" ) );
  if ( !settings.isNull() )
  {
    mStartElevation = settings.attribute( QStringLiteral( "start-elevation" ) ).toDouble();
    mEndElevation = settings.attribute( QStringLiteral( "end-elevation" ) ).toDouble();
    if ( mEndElevation > mStartElevation )
    {
      std::swap( mEndElevation, mStartElevation );
    }
  }
}

bool QgsMeshElevationAveragingMethod::equals( const QgsMesh3dAveragingMethod *other ) const
{
  if ( !other || other->method() != method() )
    return false;

  const QgsMeshElevationAveragingMethod *otherMethod = static_cast<const QgsMeshElevationAveragingMethod *>( other );

  return qgsDoubleNear( otherMethod->startElevation(), startElevation() ) && qgsDoubleNear( otherMethod->endElevation(), endElevation() ) ;
}

QgsMesh3dAveragingMethod *QgsMeshElevationAveragingMethod::clone() const
{
  return new QgsMeshElevationAveragingMethod( startElevation(), endElevation() );
}

double QgsMeshElevationAveragingMethod::startElevation() const
{
  return mStartElevation;
}

double QgsMeshElevationAveragingMethod::endElevation() const
{
  return mEndElevation;
}

bool QgsMeshElevationAveragingMethod::hasValidInputs() const
{
  return mStartElevation <= 0.0 && mEndElevation <= mStartElevation;
}

void QgsMeshElevationAveragingMethod::volumeRangeForFace( double &startVerticalLevel,
    double &endVerticalLevel,
    int &,
    const QVector<double> &verticalLevels ) const
{
  Q_UNUSED( verticalLevels )
  startVerticalLevel = mStartElevation;
  endVerticalLevel = mEndElevation;
}
