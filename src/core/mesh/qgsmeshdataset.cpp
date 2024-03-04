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
    const QString uri,
    bool isScalar,
    DataType dataType,
    double minimum,
    double maximum,
    int maximumVerticalLevels,
    const QDateTime &referenceTime,
    bool isTemporal,
    const QMap<QString, QString> &extraOptions )
  : mName( name )
  , mUri( uri )
  , mIsScalar( isScalar )
  , mDataType( dataType )
  , mMinimumValue( minimum )
  , mMaximumValue( maximum )
  , mExtraOptions( extraOptions )
  , mMaximumVerticalLevelsCount( maximumVerticalLevels )
  , mReferenceTime( referenceTime )
  , mIsTemporal( isTemporal )
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

bool QgsMeshDatasetGroupMetadata::isTemporal() const
{
  return mIsTemporal;
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

QString QgsMeshDatasetGroupMetadata::uri() const
{
  return mUri;
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

QgsMesh3DDataBlock::QgsMesh3DDataBlock() = default;

QgsMesh3DDataBlock::~QgsMesh3DDataBlock() {};

QgsMesh3DDataBlock::QgsMesh3DDataBlock( int count, bool isVector )
  : mSize( count )
  , mIsVector( isVector )
{
}

bool QgsMesh3DDataBlock::isValid() const
{
  return mIsValid;
}

bool QgsMesh3DDataBlock::isVector() const
{
  return mIsVector;
}

int QgsMesh3DDataBlock::count() const
{
  return mSize;
}

int QgsMesh3DDataBlock::firstVolumeIndex() const
{
  if ( mFaceToVolumeIndex.empty() )
    return -1;
  return mFaceToVolumeIndex[0];
}

int QgsMesh3DDataBlock::lastVolumeIndex() const
{
  if ( mFaceToVolumeIndex.empty() || mVerticalLevelsCount.empty() )
    return -1;
  const int lastVolumeStartIndex = mFaceToVolumeIndex[mFaceToVolumeIndex.size() - 1];
  const int volumesCountInLastRow = mVerticalLevelsCount[mVerticalLevelsCount.size() - 1];
  return lastVolumeStartIndex + volumesCountInLastRow;
}

int QgsMesh3DDataBlock::volumesCount() const
{
  return lastVolumeIndex() - firstVolumeIndex();
}

QVector<int> QgsMesh3DDataBlock::verticalLevelsCount() const
{
  Q_ASSERT( isValid() );
  return mVerticalLevelsCount;
}

void QgsMesh3DDataBlock::setFaceToVolumeIndex( const QVector<int> &faceToVolumeIndex )
{
  Q_ASSERT( faceToVolumeIndex.size() == count() );
  mFaceToVolumeIndex = faceToVolumeIndex;
}

void QgsMesh3DDataBlock::setVerticalLevelsCount( const QVector<int> &verticalLevelsCount )
{
  Q_ASSERT( verticalLevelsCount.size() == count() );
  mVerticalLevelsCount = verticalLevelsCount;
}

QVector<double> QgsMesh3DDataBlock::verticalLevels() const
{
  Q_ASSERT( isValid() );
  return mVerticalLevels;
}

void QgsMesh3DDataBlock::setVerticalLevels( const QVector<double> &verticalLevels )
{
  Q_ASSERT( verticalLevels.size() == volumesCount() + count() );
  mVerticalLevels = verticalLevels;
}

QVector<int> QgsMesh3DDataBlock::faceToVolumeIndex() const
{
  Q_ASSERT( isValid() );
  return mFaceToVolumeIndex;
}

QVector<double> QgsMesh3DDataBlock::values() const
{
  Q_ASSERT( isValid() );
  return mDoubleBuffer;
}

QgsMeshDatasetValue QgsMesh3DDataBlock::value( int volumeIndex ) const
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

void QgsMesh3DDataBlock::setValues( const QVector<double> &doubleBuffer )
{
  Q_ASSERT( doubleBuffer.size() == ( isVector() ? 2 * volumesCount() : volumesCount() ) );
  mDoubleBuffer = doubleBuffer;
}

void QgsMesh3DDataBlock::setValid( bool valid )
{
  mIsValid = valid;
}

QgsMeshDatasetGroupTreeItem::QgsMeshDatasetGroupTreeItem() = default;

QgsMeshDatasetGroupTreeItem::QgsMeshDatasetGroupTreeItem( const QString &defaultName, const QString &sourceName,
    bool isVector,
    int index )
  : mOriginalName( defaultName )
  , mSourceName( sourceName )
  , mIsVector( isVector )
  , mDatasetGroupIndex( index )
{
}

QgsMeshDatasetGroupTreeItem::QgsMeshDatasetGroupTreeItem( const QDomElement &itemElement, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );
  if ( itemElement.hasAttribute( QStringLiteral( "display-name" ) ) )
    mUserName = itemElement.attribute( QStringLiteral( "display-name" ), mUserName );

  if ( itemElement.hasAttribute( QStringLiteral( "original-name" ) ) )
    mOriginalName = itemElement.attribute( QStringLiteral( "original-name" ), mOriginalName );

  if ( itemElement.hasAttribute( QStringLiteral( "source-name" ) ) )
    mSourceName = itemElement.attribute( QStringLiteral( "source-name" ), mSourceName );

  if ( itemElement.hasAttribute( QStringLiteral( "is-vector" ) ) )
    mIsVector = itemElement.attribute( QStringLiteral( "is-vector" ) ).toInt();

  if ( itemElement.hasAttribute( QStringLiteral( "dataset-index" ) ) )
    mDatasetGroupIndex = itemElement.attribute( QStringLiteral( "dataset-index" ) ).toInt();

  if ( itemElement.hasAttribute( QStringLiteral( "is-enabled" ) ) )
    mIsEnabled = itemElement.attribute( QStringLiteral( "is-enabled" ) ).toInt();

  if ( itemElement.hasAttribute( QStringLiteral( "dataset-group-type" ) ) )
    mDatasetGroupType = static_cast<QgsMeshDatasetGroup::Type>( itemElement.attribute( QStringLiteral( "dataset-group-type" ) ) .toInt() ) ;

  if ( itemElement.hasAttribute( QStringLiteral( "description" ) ) )
    mDescription = itemElement.attribute( QStringLiteral( "description" ) );

  QDomElement dependOnElement = itemElement.firstChildElement( QStringLiteral( "dependent-on-item" ) );
  while ( !dependOnElement.isNull() )
  {
    if ( dependOnElement.hasAttribute( QStringLiteral( "dataset-index" ) ) )
      mDatasetGroupDependentOn.append( dependOnElement.attribute( QStringLiteral( "dataset-index" ) ).toInt() );
    dependOnElement = dependOnElement.nextSiblingElement( QStringLiteral( "dependent-on-item" ) );
  }

  QDomElement dependencyElement = itemElement.firstChildElement( QStringLiteral( "dependency-item" ) );
  while ( !dependencyElement.isNull() )
  {
    if ( dependencyElement.hasAttribute( QStringLiteral( "dataset-index" ) ) )
      mDatasetGroupDependencies.append( dependencyElement.attribute( QStringLiteral( "dataset-index" ) ).toInt() );
    dependencyElement = dependencyElement.nextSiblingElement( QStringLiteral( "dependency-item" ) );
  }

  QDomElement childElement = itemElement.firstChildElement( QStringLiteral( "mesh-dataset-group-tree-item" ) );
  while ( !childElement.isNull() )
  {
    appendChild( new QgsMeshDatasetGroupTreeItem( childElement, context ) );
    childElement = childElement.nextSiblingElement( QStringLiteral( "mesh-dataset-group-tree-item" ) );
  }
}

QgsMeshDatasetGroupTreeItem::~QgsMeshDatasetGroupTreeItem()
{
  // Remove from where this item is linked

  freeAsDependency();
  freeFromDependencies();
  qDeleteAll( mChildren );
  if ( mParent )
  {
    mParent->mDatasetGroupIndexToChild.remove( mDatasetGroupIndex );
    mParent->mChildren.removeOne( this );
  }
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::clone() const
{
  QgsMeshDatasetGroupTreeItem *other = new QgsMeshDatasetGroupTreeItem( mOriginalName, mSourceName, mIsVector, mDatasetGroupIndex );
  *other = *this;

  other->mChildren.clear();
  other->mDatasetGroupIndexToChild.clear();
  if ( !mChildren.empty() )
    for ( int i = 0; i < mChildren.count(); ++i )
      other->appendChild( mChildren.at( i )->clone() );

  return other;
}

void QgsMeshDatasetGroupTreeItem::appendChild( QgsMeshDatasetGroupTreeItem *item )
{
  mChildren.append( item );
  item->mParent = this;
  mDatasetGroupIndexToChild[item->datasetGroupIndex()] = item;
}

void QgsMeshDatasetGroupTreeItem::removeChild( QgsMeshDatasetGroupTreeItem *item )
{
  delete item;
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::child( int row ) const
{
  if ( row < mChildren.count() )
    return mChildren.at( row );
  else
    return nullptr;
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::childFromDatasetGroupIndex( int index )
{
  if ( mDatasetGroupIndexToChild.empty() )
    return nullptr;

  const QMap<int, QgsMeshDatasetGroupTreeItem *>::iterator it = mDatasetGroupIndexToChild.find( index );

  if ( it != mDatasetGroupIndexToChild.end() )
    return it.value();
  else
  {
    QgsMeshDatasetGroupTreeItem *item = nullptr;
    for ( int i = 0; i < mChildren.count(); ++i )
    {
      item = mChildren.at( i )->childFromDatasetGroupIndex( index );
      if ( item )
        break;
    }
    return item;
  }
}

int QgsMeshDatasetGroupTreeItem::childCount() const
{
  return mChildren.count();
}

int QgsMeshDatasetGroupTreeItem::totalChildCount() const
{
  int count = 0;
  for ( int i = 0; i < mChildren.count(); ++i )
  {
    count++;
    count += mChildren.at( i )->totalChildCount();
  }
  return count;
}

QList<int> QgsMeshDatasetGroupTreeItem::enabledDatasetGroupIndexes() const
{
  QList<int> indexesList;

  for ( int i = 0; i < mChildren.count(); ++i )
  {
    if ( mChildren.at( i )->isEnabled() )
      indexesList.append( mChildren.at( i )->datasetGroupIndex() );
    indexesList.append( mChildren.at( i )->enabledDatasetGroupIndexes() );
  }

  return indexesList;
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::parentItem() const
{
  return mParent;
}

int QgsMeshDatasetGroupTreeItem::row() const
{
  if ( mParent )
    return mParent->mChildren.indexOf( const_cast<QgsMeshDatasetGroupTreeItem *>( this ) );

  return 0;
}

QString QgsMeshDatasetGroupTreeItem::name() const
{
  if ( mUserName.isEmpty() )
    return mOriginalName;
  else
    return mUserName;
}

bool QgsMeshDatasetGroupTreeItem::isVector() const
{
  return mIsVector;
}

int QgsMeshDatasetGroupTreeItem::datasetGroupIndex() const
{
  return mDatasetGroupIndex;
}

bool QgsMeshDatasetGroupTreeItem::isEnabled() const
{
  return mIsEnabled;
}

void QgsMeshDatasetGroupTreeItem::setIsEnabled( bool enabled )
{
  mIsEnabled = enabled;
}

QString QgsMeshDatasetGroupTreeItem::defaultName() const
{
  return mOriginalName;
}

QgsMeshDatasetGroup::Type QgsMeshDatasetGroupTreeItem::datasetGroupType() const
{
  return mDatasetGroupType;
}

QString QgsMeshDatasetGroupTreeItem::description() const
{
  return mDescription;
}

void QgsMeshDatasetGroupTreeItem::setDatasetGroup( QgsMeshDatasetGroup *datasetGroup )
{
  if ( datasetGroup )
  {
    mDescription = datasetGroup->description();
    mDatasetGroupType = datasetGroup->type();
    const QStringList &datasetGroupNames = datasetGroup->datasetGroupNamesDependentOn();
    for ( const QString &varName : datasetGroupNames )
    {
      QgsMeshDatasetGroupTreeItem *varItem = searchItemBySourceName( varName );
      if ( varItem )
      {
        varItem->mDatasetGroupDependencies.append( this->datasetGroupIndex() );
        mDatasetGroupDependentOn.append( varItem->datasetGroupIndex() );
      }
    }
  }
}

void QgsMeshDatasetGroupTreeItem::setPersistentDatasetGroup( const QString &uri )
{
  mDatasetGroupType = QgsMeshDatasetGroup::Persistent;
  mDatasetGroupDependentOn.clear();
  mDescription = uri;
}

QDomElement QgsMeshDatasetGroupTreeItem::writeXml( QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement itemElement = doc.createElement( QStringLiteral( "mesh-dataset-group-tree-item" ) );
  itemElement.setAttribute( QStringLiteral( "display-name" ), mUserName );
  itemElement.setAttribute( QStringLiteral( "source-name" ), mSourceName );
  itemElement.setAttribute( QStringLiteral( "original-name" ), mOriginalName );
  itemElement.setAttribute( QStringLiteral( "is-vector" ), mIsVector ? true : false );
  itemElement.setAttribute( QStringLiteral( "dataset-index" ), mDatasetGroupIndex );
  itemElement.setAttribute( QStringLiteral( "is-enabled" ), mIsEnabled ? true : false );
  itemElement.setAttribute( QStringLiteral( "dataset-group-type" ), mDatasetGroupType );
  itemElement.setAttribute( QStringLiteral( "description" ), mDescription );

  for ( const int index : mDatasetGroupDependentOn )
  {
    QDomElement dependOnElement = doc.createElement( QStringLiteral( "dependent-on-item" ) );
    dependOnElement.setAttribute( QStringLiteral( "dataset-index" ), index );
    itemElement.appendChild( dependOnElement );
  }

  for ( const int index : mDatasetGroupDependencies )
  {
    QDomElement dependencyElement = doc.createElement( QStringLiteral( "dependency-item" ) );
    dependencyElement.setAttribute( QStringLiteral( "dataset-index" ), index );
    itemElement.appendChild( dependencyElement );
  }

  for ( int i = 0; i < mChildren.count(); ++i )
    itemElement.appendChild( mChildren.at( i )->writeXml( doc, context ) );

  return itemElement;
}

QList<int> QgsMeshDatasetGroupTreeItem::groupIndexDependencies() const
{
  QList<int> dependencies;
  QgsMeshDatasetGroupTreeItem *root = rootItem();
  for ( const int index : mDatasetGroupDependencies )
  {
    if ( !dependencies.contains( index ) )
      dependencies.append( index );
    QgsMeshDatasetGroupTreeItem *item = root->childFromDatasetGroupIndex( index );
    if ( item )
      dependencies.append( item->groupIndexDependencies() );
  }

  for ( int i = 0; i < childCount(); ++i )
  {
    dependencies.append( child( i )->groupIndexDependencies() );
  }

  return dependencies;
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::searchItemBySourceName( const QString &sourceName ) const
{
  QgsMeshDatasetGroupTreeItem *baseItem = rootItem();

  QList<QgsMeshDatasetGroupTreeItem *> itemToCheck;
  itemToCheck.append( baseItem );
  while ( baseItem && baseItem->providerName() != sourceName && !itemToCheck.isEmpty() )
  {
    for ( int i = 0; i < baseItem->childCount(); ++i )
      itemToCheck.append( baseItem->child( i ) );
    itemToCheck.removeOne( baseItem );
    if ( !itemToCheck.empty() )
      baseItem = itemToCheck.first();
    else
      baseItem = nullptr;
  }

  return baseItem;
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::rootItem() const
{
  const QgsMeshDatasetGroupTreeItem *baseItem = this;
  while ( baseItem->parentItem() != nullptr )
    baseItem = baseItem->parentItem();

  return const_cast<QgsMeshDatasetGroupTreeItem *>( baseItem );
}

void QgsMeshDatasetGroupTreeItem::freeAsDependency()
{
  QgsMeshDatasetGroupTreeItem *root = rootItem();
  for ( const int index : mDatasetGroupDependentOn )
  {
    QgsMeshDatasetGroupTreeItem *item = root->childFromDatasetGroupIndex( index );
    if ( item )
      item->mDatasetGroupDependencies.removeOne( this->datasetGroupIndex() );
  }
}

void QgsMeshDatasetGroupTreeItem::freeFromDependencies()
{
  QgsMeshDatasetGroupTreeItem *root = rootItem();
  for ( const int index : mDatasetGroupDependencies )
  {
    QgsMeshDatasetGroupTreeItem *item = root->childFromDatasetGroupIndex( index );
    if ( item )
      item->mDatasetGroupDependentOn.removeOne( this->datasetGroupIndex() );
  }
}

QString QgsMeshDatasetGroupTreeItem::providerName() const
{
  return mSourceName;
}

void QgsMeshDatasetGroupTreeItem::setName( const QString &name )
{
  mUserName = name;
}


QgsMeshDatasetValue QgsMeshMemoryDataset::datasetValue( int valueIndex ) const
{
  if ( valueIndex >= 0 && valueIndex < values.count() )
    return values[valueIndex];
  else
    return QgsMeshDatasetValue();
}

QgsMeshDataBlock QgsMeshMemoryDataset::datasetValues( bool isScalar, int valueIndex, int count ) const
{
  QgsMeshDataBlock ret( isScalar ? QgsMeshDataBlock::ScalarDouble : QgsMeshDataBlock::Vector2DDouble, count );
  QVector<double> buf( isScalar ? count : 2 * count );
  for ( int i = 0; i < count; ++i )
  {
    const int idx = valueIndex + i;
    if ( ( idx < 0 ) || ( idx >= values.size() ) )
      return ret;

    const QgsMeshDatasetValue val = values[ valueIndex + i ];
    if ( isScalar )
      buf[i] = val.x();
    else
    {
      buf[2 * i] = val.x();
      buf[2 * i + 1] = val.y();
    }
  }
  ret.setValues( buf );
  return ret;
}

QgsMeshDataBlock QgsMeshMemoryDataset::areFacesActive( int faceIndex, int count ) const
{
  QgsMeshDataBlock ret( QgsMeshDataBlock::ActiveFlagInteger, count );
  if ( active.isEmpty() ||
       ( faceIndex < 0 ) ||
       ( faceIndex + count > active.size() )
     )
    ret.setValid( true );
  else
    ret.setActive( active );
  return ret;
}

QgsMeshDatasetMetadata QgsMeshMemoryDataset::metadata() const
{
  return QgsMeshDatasetMetadata( time, valid, minimum, maximum, 0 );
}

void QgsMeshMemoryDataset::calculateMinMax()
{
  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::lowest();

  if ( !valid )
    return;


  bool firstIteration = true;
  for ( int i = 0; i < values.size(); ++i )
  {
    const double v = values[i].scalar();

    if ( std::isnan( v ) )
      continue;
    if ( firstIteration )
    {
      firstIteration = false;
      min = v;
      max = v;
    }
    else
    {
      if ( v < min )
        min = v;
      if ( v > max )
        max = v;
    }
  }

  minimum = min;
  maximum = max;
}

bool QgsMeshMemoryDataset::isActive( int faceIndex ) const
{
  if ( active.isEmpty() || faceIndex >= active.count() )
    return true;
  else
    return active.at( faceIndex );
}

int QgsMeshMemoryDataset::valuesCount() const
{
  return values.count();
}

QgsMeshMemoryDatasetGroup::QgsMeshMemoryDatasetGroup( const QString &name, QgsMeshDatasetGroupMetadata::DataType dataType )
  : QgsMeshDatasetGroup( name, dataType )
{
}

QgsMeshMemoryDatasetGroup::QgsMeshMemoryDatasetGroup( const QString &name )
  : QgsMeshDatasetGroup( name )
{
}

QgsMeshDatasetGroupMetadata QgsMeshDatasetGroup::groupMetadata() const
{
  return QgsMeshDatasetGroupMetadata(
           name(),
           QString(),
           isScalar(),
           dataType(),
           minimum(),
           maximum(),
           0,
           mReferenceTime,
           datasetCount() > 1,
           extraMetadata()
         );
}

int QgsMeshMemoryDatasetGroup::datasetCount() const
{
  return memoryDatasets.size();
}

QgsMeshDatasetMetadata QgsMeshMemoryDatasetGroup::datasetMetadata( int datasetIndex ) const
{
  if ( datasetIndex >= 0 && datasetIndex < memoryDatasets.count() )
    return memoryDatasets[datasetIndex]->metadata();
  else
    return QgsMeshDatasetMetadata();
}

QgsMeshDataset *QgsMeshMemoryDatasetGroup::dataset( int index ) const
{
  return memoryDatasets[index].get();
}

void QgsMeshMemoryDatasetGroup::addDataset( std::shared_ptr<QgsMeshMemoryDataset> dataset )
{
  dataset->calculateMinMax();
  memoryDatasets.push_back( dataset );
}

void QgsMeshMemoryDatasetGroup::clearDatasets()
{
  memoryDatasets.clear();
}

void QgsMeshMemoryDatasetGroup::initialize()
{
  calculateStatistic();
}

std::shared_ptr<const QgsMeshMemoryDataset> QgsMeshMemoryDatasetGroup::constDataset( int index ) const
{
  return memoryDatasets[index];
}

QDomElement QgsMeshMemoryDatasetGroup::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( doc )
  Q_UNUSED( context )
  return QDomElement();
}

void QgsMeshDatasetGroup::calculateStatistic() const
{
  updateStatistic();
}

void QgsMeshDatasetGroup::setStatisticObsolete() const
{
  mIsStatisticObsolete = true;
}

QStringList QgsMeshDatasetGroup::datasetGroupNamesDependentOn() const
{
  return QStringList();
}

QString QgsMeshDatasetGroup::description() const
{
  return QString();
}

void QgsMeshDatasetGroup::setReferenceTime( const QDateTime &referenceTime )
{
  mReferenceTime = referenceTime;
}

void QgsMeshDatasetGroup::updateStatistic() const
{
  if ( !mIsStatisticObsolete )
    return;

  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::lowest();

  const int count = datasetCount();
  for ( int i = 0; i < count; ++i )
  {
    const QgsMeshDatasetMetadata &meta = datasetMetadata( i );
    min = std::min( min,  meta.minimum() );
    max = std::max( max, meta.maximum() );
  }
  mMinimum = min;
  mMaximum = max;

  mIsStatisticObsolete = false;
}

bool QgsMeshDatasetGroup::checkValueCountPerDataset( int count ) const
{
  for ( int i = 0; i < datasetCount(); ++i )
    if ( dataset( i )->valuesCount() != count )
      return false;
  return true;
}

QgsMeshDatasetGroup::QgsMeshDatasetGroup( const QString &name, QgsMeshDatasetGroupMetadata::DataType dataType )
  : mName( name ), mDataType( dataType ) {}

QgsMeshDatasetGroup::~QgsMeshDatasetGroup() = default;

QgsMeshDatasetGroup::QgsMeshDatasetGroup( const QString &name ): mName( name ) {}

double QgsMeshDatasetGroup::minimum() const
{
  updateStatistic();
  return mMinimum;
}

double QgsMeshDatasetGroup::maximum() const
{
  updateStatistic();
  return mMaximum;
}

void QgsMeshDatasetGroup::setMinimumMaximum( double min, double max ) const
{
  mMinimum = min;
  mMaximum = max;
}

QString QgsMeshDatasetGroup::name() const
{
  return mName;
}

void QgsMeshDatasetGroup::setName( const QString &name )
{
  mName = name;
}

QgsMeshDatasetGroupMetadata::DataType QgsMeshDatasetGroup::dataType() const
{
  return mDataType;
}

void QgsMeshDatasetGroup::setDataType( const QgsMeshDatasetGroupMetadata::DataType &type )
{
  mDataType = type;
}

void QgsMeshDatasetGroup::addExtraMetadata( QString key, QString value )
{
  mMetadata.insert( key, value );
}

QMap<QString, QString> QgsMeshDatasetGroup::extraMetadata() const
{
  return mMetadata;
}

bool QgsMeshDatasetGroup::isScalar() const
{
  return mIsScalar;
}

void QgsMeshDatasetGroup::setIsScalar( bool isScalar )
{
  mIsScalar = isScalar;
}


QgsMeshVerticesElevationDataset::QgsMeshVerticesElevationDataset( QgsMesh *mesh )
  : mMesh( mesh )
{}

QgsMeshDatasetValue QgsMeshVerticesElevationDataset::datasetValue( int valueIndex ) const
{
  if ( mMesh && valueIndex >= 0 && valueIndex < mMesh->vertexCount() )
    return mMesh->vertex( valueIndex ).z();

  return std::numeric_limits<double>::quiet_NaN();
}

QgsMeshDataBlock QgsMeshVerticesElevationDataset::datasetValues( bool isScalar, int valueIndex, int count ) const
{
  if ( !isScalar || !mMesh )
    return QgsMeshDataBlock();

  QgsMeshDataBlock block( QgsMeshDataBlock::ScalarDouble, count );
  int effectiveValueCount = std::min( count, ( mMesh->vertexCount() - valueIndex ) );
  QVector<double> values( effectiveValueCount );
  for ( int i = valueIndex; i < effectiveValueCount; ++i )
    values[i] = mMesh->vertex( i - valueIndex ).z();
  block.setValues( values );
  block.setValid( true );
  return block;
}

QgsMeshDataBlock QgsMeshVerticesElevationDataset::areFacesActive( int faceIndex, int count ) const
{
  QgsMeshDataBlock block( QgsMeshDataBlock::ActiveFlagInteger, std::min( count, ( mMesh->faceCount() - faceIndex ) ) );
  block.setValid( true );
  return block;
}

QgsMeshDatasetMetadata QgsMeshVerticesElevationDataset::metadata() const
{
  double min = std::numeric_limits<double>::max();
  double max = -std::numeric_limits<double>::max();
  if ( mMesh )
    for ( int i = 0; i < mMesh->vertexCount(); ++i )
    {
      const double z = mMesh->vertex( i ).z();
      if ( min > z )
        min = z;
      if ( max < z )
        max = z;
    }

  return QgsMeshDatasetMetadata( 0, true, min, max, 0 );
}

int QgsMeshVerticesElevationDataset::valuesCount() const
{
  if ( mMesh )
    return mMesh->vertexCount();

  return 0;
}

QgsMeshVerticesElevationDatasetGroup::QgsMeshVerticesElevationDatasetGroup( QString name, QgsMesh *mesh )
  : mDataset( new QgsMeshVerticesElevationDataset( mesh ) )
{
  mName = name ;
  initialize();
}

void QgsMeshVerticesElevationDatasetGroup::initialize()
{
  calculateStatistic();
}

QgsMeshDatasetMetadata QgsMeshVerticesElevationDatasetGroup::datasetMetadata( int datasetIndex ) const
{
  if ( datasetIndex != 0 )
    return QgsMeshDatasetMetadata();

  return mDataset->metadata();
}

int QgsMeshVerticesElevationDatasetGroup::datasetCount() const {return 1;}

QgsMeshDataset *QgsMeshVerticesElevationDatasetGroup::dataset( int index ) const
{
  if ( index != 0 )
    return nullptr;

  return mDataset.get();
}

QgsMeshDatasetGroup::Type QgsMeshVerticesElevationDatasetGroup::type() const
{
  return QgsMeshDatasetGroup::Memory; //maybe create a new type ?
}

