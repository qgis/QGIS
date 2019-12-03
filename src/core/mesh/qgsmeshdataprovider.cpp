/***************************************************************************
                         qgsmeshdataprovider.cpp
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

#include "qgsmeshdataprovider.h"
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

bool QgsMeshDatasetIndex::operator ==( const QgsMeshDatasetIndex &other ) const
{
  if ( isValid() && other.isValid() )
    return other.group() == group() && other.dataset() == dataset();
  else
    return isValid() == other.isValid();
}

bool QgsMeshDatasetIndex::operator !=( const QgsMeshDatasetIndex &other ) const
{
  return !( operator==( other ) );
}

QgsMeshDataProvider::QgsMeshDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options )
  : QgsDataProvider( uri, options )
{
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

bool QgsMeshDatasetValue::operator==( const QgsMeshDatasetValue &other ) const
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
    const QMap<QString, QString> &extraOptions )
  : mName( name )
  , mIsScalar( isScalar )
  , mDataType( dataType )
  , mMinimumValue( minimum )
  , mMaximumValue( maximum )
  , mExtraOptions( extraOptions )
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

int QgsMeshDatasetSourceInterface::datasetCount( QgsMeshDatasetIndex index ) const
{
  return datasetCount( index.group() );
}

QgsMeshDatasetGroupMetadata QgsMeshDatasetSourceInterface::datasetGroupMetadata( QgsMeshDatasetIndex index ) const
{
  return datasetGroupMetadata( index.group() );
}

QgsMeshDatasetMetadata::QgsMeshDatasetMetadata(
  double time,
  bool isValid,
  double minimum,
  double maximum,
  int maximumVerticalLevelsCount )
  : mTime( time )
  , mIsValid( isValid )
  , mMinimumValue( minimum )
  , mMaximumValue( maximum )
  , mMaximumVerticalLevelsCount( maximumVerticalLevelsCount )
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
  : mType( type )
{
  switch ( type )
  {
    case ActiveFlagInteger:
      mIntegerBuffer.resize( count );
      break;
    case ScalarDouble:
      mDoubleBuffer.resize( count );
      break;
    case Vector2DDouble:
      mDoubleBuffer.resize( 2 * count );
      break;
  }
}

QgsMeshDataBlock::DataType QgsMeshDataBlock::type() const
{
  return mType;
}

int QgsMeshDataBlock::count() const
{
  switch ( mType )
  {
    case ActiveFlagInteger:
      return mIntegerBuffer.size();
    case ScalarDouble:
      return mDoubleBuffer.size();
    case Vector2DDouble:
      return static_cast<int>( mDoubleBuffer.size() / 2.0 );
  }
  return 0; // no warnings
}

bool QgsMeshDataBlock::isValid() const
{
  return count() > 0;
}

QgsMeshDatasetValue QgsMeshDataBlock::value( int index ) const
{
  switch ( mType )
  {
    case ActiveFlagInteger:
      return QgsMeshDatasetValue();
    case ScalarDouble:
      return QgsMeshDatasetValue( mDoubleBuffer[index] );
    case Vector2DDouble:
      return QgsMeshDatasetValue(
               mDoubleBuffer[2 * index],
               mDoubleBuffer[2 * index + 1]
             );
  }
  return QgsMeshDatasetValue(); // no warnings
}

bool QgsMeshDataBlock::active( int index ) const
{
  if ( ActiveFlagInteger == mType )
    return bool( mIntegerBuffer[index] );
  else
    return false;
}

void *QgsMeshDataBlock::buffer()
{
  if ( ActiveFlagInteger == mType )
  {
    return mIntegerBuffer.data();
  }
  else
  {
    return mDoubleBuffer.data();
  }
}

const void *QgsMeshDataBlock::constBuffer() const
{
  if ( ActiveFlagInteger == mType )
  {
    return mIntegerBuffer.constData();
  }
  else
  {
    return mDoubleBuffer.constData();
  }
}

QgsMeshVertex QgsMesh::vertex( int index ) const
{
  if ( index < vertices.size() && index >= 0 )
    return vertices[index];
  return QgsMeshVertex();
}

QgsMeshFace QgsMesh::face( int index ) const
{
  if ( index < faces.size() && index >= 0 )
    return faces[index];
  return QgsMeshFace();
}

int QgsMesh::vertexCount() const
{
  return vertices.size();
}

int QgsMesh::faceCount() const
{
  return faces.size();
}

QgsMesh3dDataBlock::QgsMesh3dDataBlock() = default;

QgsMesh3dDataBlock::~QgsMesh3dDataBlock() {};

QgsMesh3dDataBlock::QgsMesh3dDataBlock( int count, int maximumVerticalLevels, bool isVector )
  : mIsVector( isVector )
  , mMaximumVerticalLevels( maximumVerticalLevels )
{
  if ( mMaximumVerticalLevels > 0 )
  {
    mVerticalLevelsCount.resize( count );
    mVerticalLevels.resize( count * mMaximumVerticalLevels );
    mFaceToVolumeIndex.resize( count );
    mIntegerBuffer.resize( count * mMaximumVerticalLevels );

    int doubleBufSize = count * mMaximumVerticalLevels;
    if ( isVector )
      doubleBufSize *= 2;
    mDoubleBuffer.resize( doubleBufSize );
  }
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
  return mVerticalLevelsCount.size();
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

void *QgsMesh3dDataBlock::buffer( QgsMesh3dDataBlock::DataType type )
{
  switch ( type )
  {
    case ScalarDouble:
      return mDoubleBuffer.data();
    case VectorDouble:
      return mDoubleBuffer.data();
    case ActiveFlagInteger:
      return mIntegerBuffer.data();
    case VerticalLevels:
      return mVerticalLevels.data();
    case VerticalLevelsCount:
      return mVerticalLevelsCount.data();
    case FaceToVolumeIndex:
      return mFaceToVolumeIndex.data();
  }
}

const void *QgsMesh3dDataBlock::constBuffer( QgsMesh3dDataBlock::DataType type ) const
{
  switch ( type )
  {
    case ScalarDouble:
      return mDoubleBuffer.constData();
    case VectorDouble:
      return mDoubleBuffer.constData();
    case ActiveFlagInteger:
      return mIntegerBuffer.constData();
    case VerticalLevels:
      return mVerticalLevels.constData();
    case VerticalLevelsCount:
      return mVerticalLevelsCount.constData();
    case FaceToVolumeIndex:
      return mFaceToVolumeIndex.constData();
  }
}

void QgsMesh3dDataBlock::setIsValid( bool valid )
{
  mIsValid = valid;
}
