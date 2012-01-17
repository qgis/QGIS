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

QgsAttributeTableMemoryModel::QgsAttributeTableMemoryModel( QgsMapCanvas *theCanvas, QgsVectorLayer *theLayer )
    : QgsAttributeTableModel( theCanvas, theLayer )
{
  QgsDebugMsg( "entered." );
}

void QgsAttributeTableMemoryModel::loadLayer()
{
  QgsDebugMsg( "entered." );

  QSettings settings;
  int behaviour = settings.value( "/qgis/attributeTableBehaviour", 0 ).toInt();

  QgsRectangle rect;
  if ( behaviour == 2 )
  {
    // current canvas only
    rect = mCurrentExtent;
  }

  mLayer->select( mLayer->pendingAllAttributesList(), rect, false );

  if ( behaviour != 1 )
    mFeatureMap.reserve( mLayer->pendingFeatureCount() + 50 );
  else
    mFeatureMap.reserve( mLayer->selectedFeatureCount() );

  int i = 0;

  QTime t;
  t.start();

  QgsFeature f;
  while ( mLayer->nextFeature( f ) )
  {
    if ( behaviour == 1 && !mLayer->selectedFeaturesIds().contains( f.id() ) )
      continue;

    mIdRowMap.insert( f.id(), i );
    mRowIdMap.insert( i, f.id() );
    mFeatureMap.insert( f.id(), f );

    i++;

    if ( t.elapsed() > 5000 )
    {
      bool cancel = false;
      emit progress( i, cancel );
      if ( cancel )
        break;

      t.restart();
    }
  }

  emit finished();

  mFieldCount = mAttributes.size();
}

int QgsAttributeTableMemoryModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mFeatureMap.size();
}

bool QgsAttributeTableMemoryModel::featureAtId( QgsFeatureId fid ) const
{
  QgsDebugMsg( QString( "entered featureAtId: %1." ).arg( fid ) );
  if ( mFeatureMap.contains( fid ) )
  {
    mFeat = mFeatureMap[ fid ];
    return true;
  }
  else
  {
    QgsDebugMsg( QString( "feature %1 not loaded" ).arg( fid ) );
    return false;
  }
}

void QgsAttributeTableMemoryModel::featureDeleted( QgsFeatureId fid )
{
  QgsDebugMsg( "entered." );
  mFeatureMap.remove( fid );
  QgsAttributeTableModel::featureDeleted( fid );
}

void QgsAttributeTableMemoryModel::featureAdded( QgsFeatureId fid )
{
  QgsDebugMsg( "entered." );
  Q_UNUSED( fid );
  loadLayer();
}

void QgsAttributeTableMemoryModel::layerDeleted()
{
  QgsDebugMsg( "entered." );
  loadLayer();
}

void QgsAttributeTableMemoryModel::attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  QgsDebugMsg( "entered." );
  mFeatureMap[fid].changeAttribute( idx, value );
  QgsAttributeTableModel::attributeValueChanged( fid, idx, value );
}

bool QgsAttributeTableMemoryModel::removeRows( int row, int count, const QModelIndex &parent )
{
  QgsDebugMsg( "entered." );
  for ( int i = row; i < row + count; i++ )
  {
    mFeatureMap.remove( mRowIdMap[ i ] );
  }
  return QgsAttributeTableModel::removeRows( row, count, parent );
}
