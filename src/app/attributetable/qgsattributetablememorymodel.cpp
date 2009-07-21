/***************************************************************************
     QgsAttributeTableMemoryModel.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributetablememorymodel.h"
#include "qgsattributetablefiltermodel.h"

#include "qgsfield.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QtGui>
#include <QVariant>

/////////////////////
// In-Memory model //
/////////////////////

void QgsAttributeTableMemoryModel::loadLayer()
{
  QgsAttributeTableModel::loadLayer();
  mLayer->select( mLayer->pendingAllAttributesList(), QgsRectangle(), false );

  mFeatureMap.reserve( mLayer->pendingFeatureCount() + 50 );

  QgsFeature f;
  while ( mLayer->nextFeature( f ) )
    mFeatureMap.insert( f.id(), f );
}

QgsAttributeTableMemoryModel::QgsAttributeTableMemoryModel
( QgsVectorLayer *theLayer )
    : QgsAttributeTableModel( theLayer )
{
  loadLayer();
}

QVariant QgsAttributeTableMemoryModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || ( role != Qt::TextAlignmentRole && role != Qt::DisplayRole && role != Qt::EditRole ) )
    return QVariant();

  QVariant::Type fldType = mLayer->pendingFields()[ mAttributes[index.column()] ].type();
  bool fldNumeric = ( fldType == QVariant::Int || fldType == QVariant::Double );

  if ( role == Qt::TextAlignmentRole )
  {
    if ( fldNumeric )
      return QVariant( Qt::AlignRight );
    else
      return QVariant( Qt::AlignLeft );
  }

  // if we don't have the row in current cache, load it from layer first
  if ( mLastRowId != rowToId( index.row() ) )
  {
    //bool res = mLayer->featureAtId(rowToId(index.row()), mFeat, false, true);
    bool res = mFeatureMap.contains( rowToId( index.row() ) );

    if ( !res )
      return QVariant( "ERROR" );

    mLastRowId = rowToId( index.row() );
    mFeat = mFeatureMap[rowToId( index.row() )];
    mLastRow = ( QgsAttributeMap * ) & mFeat.attributeMap();
  }

  if ( !mLastRow )
    return QVariant( "ERROR" );

  QVariant &val = ( *mLastRow )[ mAttributes[index.column()] ];

  if ( val.isNull() )
  {
    // if the value is NULL, show that in table, but don't show "NULL" text in editor
    if ( role == Qt::EditRole )
      return QVariant();
    else
      return QVariant( "NULL" );
  }

  // force also numeric data for EditRole to be strings
  // otherwise it creates spinboxes instead of line edits
  // (probably not what we do want)
  if ( fldNumeric && role == Qt::EditRole )
    return val.toString();

  // convert to QString from some other representation
  // this prevents displaying greater numbers in exponential format
  return val.toString();
}

bool QgsAttributeTableMemoryModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
    return false;

  if ( !mLayer->isEditable() )
    return false;

  //bool res = mLayer->featureAtId(rowToId(index.row()), mFeat, false, true);
  bool res = mFeatureMap.contains( rowToId( index.row() ) );

  if ( res )
  {
    mLastRowId = rowToId( index.row() );
    mFeat = mFeatureMap[rowToId( index.row() )];
    mLastRow = ( QgsAttributeMap * ) & mFeat.attributeMap();


// QgsDebugMsg(mFeatureMap[rowToId(index.row())].id());
    mFeatureMap[rowToId( index.row() )].changeAttribute( mAttributes[ index.column()], value );
    // propagate back to the layer
    mLayer->beginEditCommand( tr( "Attribute changed" ) );
    mLayer->changeAttributeValue( rowToId( index.row() ), mAttributes[ index.column()], value, true );
    mLayer->endEditCommand();
  }

  if ( !mLayer->isModified() )
    return false;

  emit dataChanged( index, index );
  return true;
}

void QgsAttributeTableMemoryModel::featureDeleted( int fid )
{
  QgsDebugMsg( "entered." );
  mFeatureMap.remove( fid );
  QgsAttributeTableModel::featureDeleted( fid );
}

void QgsAttributeTableMemoryModel::featureAdded( int fid )
{
  QgsDebugMsg( "entered." );
  QgsFeature f;
  mLayer->featureAtId( fid, f, false, true );
  mFeatureMap.insert( fid, f );
  QgsAttributeTableModel::featureAdded( fid );
}

#if 0
void QgsAttributeTableMemoryModel::attributeAdded( int idx )
{
  QgsDebugMsg( "entered." );
  loadLayer();
  reload( index( 0, 0 ), index( rowCount(), columnCount() ) );
}

void QgsAttributeTableMemoryModel::attributeDeleted( int idx )
{
  QgsDebugMsg( "entered." );
  loadLayer();
  reload( index( 0, 0 ), index( rowCount(), columnCount() ) );
}
#endif

void QgsAttributeTableMemoryModel::layerDeleted()
{
  QgsDebugMsg( "entered." );
  mFeatureMap.clear();
  QgsAttributeTableModel::layerDeleted();
}

void QgsAttributeTableMemoryModel::attributeValueChanged( int fid, int idx, const QVariant &value )
{
  QgsDebugMsg( "entered." );
  mFeatureMap[fid].changeAttribute( idx, value );
  reload( index( 0, 0 ), index( rowCount(), columnCount() ) );
}
