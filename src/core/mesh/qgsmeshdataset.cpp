/***************************************************************************
                         qgsmeshdataset.cpp
                         -----------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
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

#include "qgsmeshdataset.h"
#include "qgsrectangle.h"
#include "qgis.h"

QgsMeshDatasetIndex::QgsMeshDatasetIndex( int group, int dataset )
  : mGroupIndex( group ), mDatasetIndex( dataset )
{}

int QgsMeshDatasetIndex::group() const
{
  return mGroupIndex;
}

int QgsMeshDatasetIndex::dataset() const
{
  return mDatasetIndex;
}

bool QgsMeshDatasetIndex::isValid() const
{
  return ( group() > -1 ) && ( dataset() > -1 );
}

bool QgsMeshDatasetIndex::operator ==( QgsMeshDatasetIndex other ) const
{
  if ( isValid() && other.isValid() )
    return other.group() == group() && other.dataset() == dataset();
  else
    return isValid() == other.isValid();
}

bool QgsMeshDatasetIndex::operator !=( QgsMeshDatasetIndex other ) const
{
  return !( operator==( other ) );
}

QgsMeshDatasetValue::QgsMeshDatasetValue( double x, double y )
  : mX( x ), mY( y )
{}

QgsMeshDatasetValue::QgsMeshDatasetValue( double scalar )
  : mX( scalar )
{}

double QgsMeshDatasetValue::scalar() const
{
  if ( std::isnan( mY ) )
  {
    return mX;
  }
  else if ( std::isnan( mX ) )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else
  {
    return std::sqrt( ( mX ) * ( mX ) + ( mY ) * ( mY ) );
  }
}

void QgsMeshDatasetValue::set( double scalar )
{
  setX( scalar );
}

void QgsMeshDatasetValue::setX( double x )
{
  mX = x;
}

void QgsMeshDatasetValue::setY( double y )
{
  mY = y;
}

double QgsMeshDatasetValue::x() const
{
  return mX;
}

double QgsMeshDatasetValue::y() const
{
  return mY;
}

bool QgsMeshDatasetValue::operator==( const QgsMeshDatasetValue other ) const
{
  bool equal = std::isnan( mX ) == std::isnan( other.x() );
  equal &= std::isnan( mY ) == std::isnan( other.y() );

  if ( equal )
  {
    if ( std::isnan( mY ) )
    {
      equal &= qgsDoubleNear( other.x(), mX, 1E-8 );
    }
    else
    {
      equal &= qgsDoubleNear( other.x(), mX, 1E-8 );
      equal &= qgsDoubleNear( other.y(), mY, 1E-8 );
    }
  }
  return equal;
}

QgsMeshDatasetGroupMetadata::QgsMeshDatasetGroupMetadata( const QString &name,
    bool isScalar,
    DataType dataType,
    double minimum,
    double maximum,
    int maximumVerticalLevels,
    const QDateTime &referenceTime,
    const QMap<QString, QString> &extraOptions )
  : mName( name )
  , mIsScalar( isScalar )
  , mDataType( dataType )
  , mMinimumValue( minimum )
  , mMaximumValue( maximum )
  , mExtraOptions( extraOptions )
  , mMaximumVerticalLevelsCount( maximumVerticalLevels )
  , mReferenceTime( referenceTime )
{
}

QMap<QString, QString> QgsMeshDatasetGroupMetadata::extraOptions() const
{
  return mExtraOptions;
}

bool QgsMeshDatasetGroupMetadata::isVector() const
{
  return !mIsScalar;
}

bool QgsMeshDatasetGroupMetadata::isScalar() const
{
  return mIsScalar;
}

QString QgsMeshDatasetGroupMetadata::name() const
{
  return mName;
}

QgsMeshDatasetGroupMetadata::DataType QgsMeshDatasetGroupMetadata::dataType() const
{
  return mDataType;
}

double QgsMeshDatasetGroupMetadata::minimum() const
{
  return mMinimumValue;
}

double QgsMeshDatasetGroupMetadata::maximum() const
{
  return mMaximumValue;
}

int QgsMeshDatasetGroupMetadata::maximumVerticalLevelsCount() const
{
  return mMaximumVerticalLevelsCount;
}

QDateTime QgsMeshDatasetGroupMetadata::referenceTime() const
{
  return mReferenceTime;
}

QgsMeshDatasetMetadata::QgsMeshDatasetMetadata(
  double time,
  bool isValid,
  double minimum,
  double maximum,
  int maximumVerticalLevels )
  : mTime( time )
  , mIsValid( isValid )
  , mMinimumValue( minimum )
  , mMaximumValue( maximum )
  , mMaximumVerticalLevelsCount( maximumVerticalLevels )
{
}

double QgsMeshDatasetMetadata::time() const
{
  return mTime;
}

bool QgsMeshDatasetMetadata::isValid() const
{
  return mIsValid;
}

double QgsMeshDatasetMetadata::minimum() const
{
  return mMinimumValue;
}

double QgsMeshDatasetMetadata::maximum() const
{
  return mMaximumValue;
}

int QgsMeshDatasetMetadata::maximumVerticalLevelsCount() const
{
  return mMaximumVerticalLevelsCount;
}

QgsMeshDataBlock::QgsMeshDataBlock()
  : mType( ActiveFlagInteger )
{
}

QgsMeshDataBlock::QgsMeshDataBlock( QgsMeshDataBlock::DataType type, int count )
  : mType( type ),
    mSize( count )
{
}

QgsMeshDataBlock::DataType QgsMeshDataBlock::type() const
{
  return mType;
}

int QgsMeshDataBlock::count() const
{
  return mSize;
}

bool QgsMeshDataBlock::isValid() const
{
  return ( count() > 0 ) && ( mIsValid );
}

QgsMeshDatasetValue QgsMeshDataBlock::value( int index ) const
{
  if ( !isValid() )
    return QgsMeshDatasetValue();

  Q_ASSERT( mType != ActiveFlagInteger );

  if ( mType == ScalarDouble )
    return QgsMeshDatasetValue( mDoubleBuffer[index] );

  return QgsMeshDatasetValue(
           mDoubleBuffer[2 * index],
           mDoubleBuffer[2 * index + 1]
         );
}

bool QgsMeshDataBlock::active( int index ) const
{
  if ( !isValid() )
    return false;

  Q_ASSERT( mType == ActiveFlagInteger );

  if ( mIntegerBuffer.empty() )
    return true;
  else
    return bool( mIntegerBuffer[index] );
}

void QgsMeshDataBlock::setActive( const QVector<int> &vals )
{
  Q_ASSERT( mType == ActiveFlagInteger );
  Q_ASSERT( vals.size() == count() );

  mIntegerBuffer = vals;
  setValid( true );
}

QVector<int> QgsMeshDataBlock::active() const
{
  Q_ASSERT( mType == ActiveFlagInteger );
  return mIntegerBuffer;
}

QVector<double> QgsMeshDataBlock::values() const
{
  Q_ASSERT( mType != ActiveFlagInteger );

  return mDoubleBuffer;
}

void QgsMeshDataBlock::setValues( const QVector<double> &vals )
{
  Q_ASSERT( mType != ActiveFlagInteger );
  Q_ASSERT( mType == ScalarDouble ? vals.size() == count() : vals.size() == 2 * count() );

  mDoubleBuffer = vals;
  setValid( true );
}

void QgsMeshDataBlock::setValid( bool valid )
{
  mIsValid = valid;
}

QgsMesh3dDataBlock::QgsMesh3dDataBlock() = default;

QgsMesh3dDataBlock::~QgsMesh3dDataBlock() {};

QgsMesh3dDataBlock::QgsMesh3dDataBlock( int count, bool isVector )
  : mSize( count )
  , mIsVector( isVector )
{
}

bool QgsMesh3dDataBlock::isValid() const
{
  return mIsValid;
}

bool QgsMesh3dDataBlock::isVector() const
{
  return mIsVector;
}

int QgsMesh3dDataBlock::count() const
{
  return mSize;
}

int QgsMesh3dDataBlock::firstVolumeIndex() const
{
  if ( mFaceToVolumeIndex.empty() )
    return -1;
  return mFaceToVolumeIndex[0];
}

int QgsMesh3dDataBlock::lastVolumeIndex() const
{
  if ( mFaceToVolumeIndex.empty() || mVerticalLevelsCount.empty() )
    return -1;
  const int lastVolumeStartIndex = mFaceToVolumeIndex[mFaceToVolumeIndex.size() - 1];
  const int volumesCountInLastRow = mVerticalLevelsCount[mVerticalLevelsCount.size() - 1];
  return lastVolumeStartIndex + volumesCountInLastRow;
}

int QgsMesh3dDataBlock::volumesCount() const
{
  return lastVolumeIndex() - firstVolumeIndex();
}

QVector<int> QgsMesh3dDataBlock::verticalLevelsCount() const
{
  Q_ASSERT( isValid() );
  return mVerticalLevelsCount;
}

void QgsMesh3dDataBlock::setFaceToVolumeIndex( const QVector<int> &faceToVolumeIndex )
{
  Q_ASSERT( faceToVolumeIndex.size() == count() );
  mFaceToVolumeIndex = faceToVolumeIndex;
}

void QgsMesh3dDataBlock::setVerticalLevelsCount( const QVector<int> &verticalLevelsCount )
{
  Q_ASSERT( verticalLevelsCount.size() == count() );
  mVerticalLevelsCount = verticalLevelsCount;
}

QVector<double> QgsMesh3dDataBlock::verticalLevels() const
{
  Q_ASSERT( isValid() );
  return mVerticalLevels;
}

void QgsMesh3dDataBlock::setVerticalLevels( const QVector<double> &verticalLevels )
{
  Q_ASSERT( verticalLevels.size() == volumesCount() + count() );
  mVerticalLevels = verticalLevels;
}

QVector<int> QgsMesh3dDataBlock::faceToVolumeIndex() const
{
  Q_ASSERT( isValid() );
  return mFaceToVolumeIndex;
}

QVector<double> QgsMesh3dDataBlock::values() const
{
  Q_ASSERT( isValid() );
  return mDoubleBuffer;
}

QgsMeshDatasetValue QgsMesh3dDataBlock::value( int volumeIndex ) const
{
  if ( !isValid() )
    return QgsMeshDatasetValue();

  if ( !mIsVector )
    return QgsMeshDatasetValue( mDoubleBuffer[volumeIndex] );

  return QgsMeshDatasetValue(
           mDoubleBuffer[2 * volumeIndex],
           mDoubleBuffer[2 * volumeIndex + 1]
         );
}

void QgsMesh3dDataBlock::setValues( const QVector<double> &doubleBuffer )
{
  Q_ASSERT( doubleBuffer.size() == ( isVector() ? 2 * volumesCount() : volumesCount() ) );
  mDoubleBuffer = doubleBuffer;
}

void QgsMesh3dDataBlock::setValid( bool valid )
{
  mIsValid = valid;
}

