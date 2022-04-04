/***************************************************************************
                      qgsgeometryvalidationmodel.cpp
                     --------------------------------------
Date                 : 6.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryvalidationmodel.h"

#include "qgsvectorlayer.h"
#include "qgssinglegeometrycheck.h"
#include "qgsfeatureid.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"

#include <QIcon>

QgsGeometryValidationModel::QgsGeometryValidationModel( QgsGeometryValidationService *geometryValidationService, QObject *parent )
  : QAbstractItemModel( parent )
  , mGeometryValidationService( geometryValidationService )
{
  connect( mGeometryValidationService, &QgsGeometryValidationService::singleGeometryCheckCleared, this, &QgsGeometryValidationModel::onSingleGeometryCheckCleared );
  connect( mGeometryValidationService, &QgsGeometryValidationService::geometryCheckCompleted, this, &QgsGeometryValidationModel::onGeometryCheckCompleted );
  connect( mGeometryValidationService, &QgsGeometryValidationService::geometryCheckStarted, this, &QgsGeometryValidationModel::onGeometryCheckStarted );
  connect( mGeometryValidationService, &QgsGeometryValidationService::topologyChecksUpdated, this, &QgsGeometryValidationModel::onTopologyChecksUpdated );
  connect( mGeometryValidationService, &QgsGeometryValidationService::topologyChecksCleared, this, &QgsGeometryValidationModel::onTopologyChecksCleared );
  connect( mGeometryValidationService, &QgsGeometryValidationService::topologyErrorUpdated, this, &QgsGeometryValidationModel::onTopologyErrorUpdated );
}

QModelIndex QgsGeometryValidationModel::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return createIndex( row, column );
}

QModelIndex QgsGeometryValidationModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsGeometryValidationModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mErrorStorage.value( mCurrentLayer ).size() + mTopologyErrorStorage.value( mCurrentLayer ).size();
}

int QgsGeometryValidationModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsGeometryValidationModel::data( const QModelIndex &index, int role ) const
{
  const QList< FeatureErrors > layerErrors = mErrorStorage.value( mCurrentLayer );

  if ( index.row() >= layerErrors.size() )
  {
    // Topology error
    const QList< std::shared_ptr< QgsGeometryCheckError > > topologyErrors = mTopologyErrorStorage.value( mCurrentLayer );
    auto topologyError = topologyErrors.at( index.row() - layerErrors.size() );

    switch ( role )
    {
      case Qt::DecorationRole:
        return topologyError->icon();

      case Qt::DisplayRole:
      case DetailsRole:
      {
        const QgsFeatureId fid = topologyError->featureId();
        const QgsFeature feature = getFeature( fid );
        mExpressionContext.setFeature( feature );
        const QVariant featureTitle = mDisplayExpression.evaluate( &mExpressionContext );

        if ( featureTitle.isNull() )
        {
          return topologyError->description();
        }
        else
        {
          return tr( "%1: %2" ).arg( featureTitle.toString(), topologyError->description() );
        }
      }

      case FeatureExtentRole:
      {
        const QgsRectangle contextBoundingBox = topologyError->contextBoundingBox();
        if ( !contextBoundingBox.isNull() )
        {
          return contextBoundingBox;
        }
        else
        {
          const QgsFeatureId fid = topologyError->featureId();
          if ( FID_IS_NULL( fid ) )
            return QgsRectangle();
          const QgsFeature feature = getFeature( fid );
          return feature.geometry().boundingBox();
        }
      }

      case ProblemExtentRole:
      {
        return topologyError->affectedAreaBBox();
      }

      case ErrorGeometryRole:
      {
        return topologyError->geometry();
      }

      case ErrorFeatureIdRole:
      {
        return topologyError->featureId();
      }

      case FeatureGeometryRole:
      {
        const QgsFeatureId fid = topologyError->featureId();
        if ( !FID_IS_NULL( fid ) )
        {
          const QgsFeature feature = getFeature( fid );
          return feature.geometry();
        }
        return QgsGeometry();
      }

      case ErrorLocationGeometryRole:
      {
        return topologyError->location();
      }

      case GeometryCheckErrorRole:
      {
        return QVariant::fromValue<QgsGeometryCheckError *>( topologyError.get() );
      }
    }
  }
  else
  {
    // Geometry error
    const FeatureErrors &featureItem = layerErrors.at( index.row() );

    switch ( role )
    {
      case Qt::DisplayRole:
      {
        QgsFeature feature = getFeature( featureItem.fid );
        mExpressionContext.setFeature( feature );
        QString featureTitle = mDisplayExpression.evaluate( &mExpressionContext ).toString();
        if ( featureTitle.isEmpty() )
          featureTitle = FID_TO_STRING( featureItem.fid );

        if ( featureItem.errors.count() > 1 )
          return tr( "%1: %n Error(s)", "", featureItem.errors.count() ).arg( featureTitle );
        else if ( featureItem.errors.count() == 1 )
          return tr( "%1: %2" ).arg( featureTitle, featureItem.errors.at( 0 )->description() );
        else
          return QVariant();
      }

      case Qt::DecorationRole:
      {
        return QgsApplication::getThemeIcon( "/checks/InvalidGeometry.svg" );
      }

      case GeometryCheckErrorRole:
      {
        // Not (yet?) used
        break;
      }

      case ErrorFeatureIdRole:
      {
        return featureItem.fid;
      }

      case FeatureExtentRole:
      {
        return getFeature( featureItem.fid ).geometry().boundingBox();
      }

      case ErrorLocationGeometryRole:
      {
        if ( featureItem.errors.empty() )
          return QVariant();

        QgsSingleGeometryCheckError *error = featureItem.errors.first().get();
        return error->errorLocation();
      }

      case ProblemExtentRole:
      {
        if ( featureItem.errors.empty() )
          return QVariant();

        QgsSingleGeometryCheckError *error = featureItem.errors.first().get();
        return error->errorLocation().boundingBox();
      }

      case DetailsRole:
      {
        QStringList details;
        for ( const std::shared_ptr<QgsSingleGeometryCheckError> &error : std::as_const( featureItem.errors ) )
        {
          details << error->description();
        }

        return details.join( '\n' );
      }

      default:
        return QVariant();
    }
  }


  return QVariant();
}

QgsVectorLayer *QgsGeometryValidationModel::currentLayer() const
{
  return mCurrentLayer;
}

void QgsGeometryValidationModel::setCurrentLayer( QgsVectorLayer *currentLayer )
{
  if ( mCurrentLayer == currentLayer )
    return;

  beginResetModel();
  mCurrentLayer = currentLayer;
  mCachedFeature.setValid( false );
  if ( mCurrentLayer )
  {
    mDisplayExpression = mCurrentLayer ? mCurrentLayer->displayExpression() : QString();
    mExpressionContext = QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( mCurrentLayer ) );
    mDisplayExpression.prepare( &mExpressionContext );
    mRequiredAttributes = qgis::setToList( mDisplayExpression.referencedColumns() );
  }
  else
  {
    mDisplayExpression = QString();
    mExpressionContext = QgsExpressionContext();
  }
  endResetModel();
}

void QgsGeometryValidationModel::onSingleGeometryCheckCleared( QgsVectorLayer *layer )
{
  auto &layerErrors = mErrorStorage[layer];

  if ( mCurrentLayer == layer && !layerErrors.empty() )
  {
    emit aboutToRemoveSingleGeometryCheck();
    beginRemoveRows( QModelIndex(), 0, layerErrors.size() - 1 );
  }

  layerErrors.clear();

  if ( mCurrentLayer == layer && !layerErrors.empty() )
  {
    endRemoveRows();
  }

}

void QgsGeometryValidationModel::onGeometryCheckCompleted( QgsVectorLayer *layer, QgsFeatureId fid, const QList<std::shared_ptr<QgsSingleGeometryCheckError>> &errors )
{
  auto &layerErrors = mErrorStorage[layer];

  int featureIdx = errorsForFeature( layer, fid );

  // The last check for this feature finished: remove
  if ( featureIdx > -1 && errors.empty() ) // && !mGeometryValidationService->validationActive( layer, fid ) )
  {
    if ( mCurrentLayer == layer )
    {
      emit aboutToRemoveSingleGeometryCheck();
      beginRemoveRows( QModelIndex(), featureIdx, featureIdx );
    }

    layerErrors.removeAt( featureIdx );

    if ( mCurrentLayer == layer )
      endRemoveRows();
  }
  else if ( !errors.empty() )
  {
    // a new or updated feature
    if ( featureIdx == -1 )
    {
      featureIdx = layerErrors.count();
      if ( mCurrentLayer == layer )
        beginInsertRows( QModelIndex(), featureIdx, featureIdx );

      layerErrors << FeatureErrors( fid );

      if ( mCurrentLayer == layer )
        endInsertRows();
    }

    auto &featureItem = layerErrors[featureIdx];
    featureItem.errors.append( errors );
    if ( mCurrentLayer == layer )
    {
      QModelIndex modelIndex = index( featureIdx, 0, QModelIndex() );
      emit dataChanged( modelIndex, modelIndex );
    }
  }
}

void QgsGeometryValidationModel::onGeometryCheckStarted( QgsVectorLayer *layer, QgsFeatureId fid )
{
  auto &layerErrors = mErrorStorage[layer];
  int featureIdx = errorsForFeature( layer, fid );
  if ( featureIdx != -1 )
  {
    auto &featureItem = layerErrors[featureIdx];

    featureItem.errors.clear();

    if ( mCurrentLayer == layer )
    {
      QModelIndex modelIndex = index( featureIdx, 0, QModelIndex() );
      emit dataChanged( modelIndex, modelIndex );
    }
  }
}

void QgsGeometryValidationModel::onTopologyChecksUpdated( QgsVectorLayer *layer, const QList<std::shared_ptr<QgsGeometryCheckError> > &errors )
{
  if ( errors.empty() )
    return;

  auto &topologyLayerErrors = mTopologyErrorStorage[layer];

  if ( layer == currentLayer() )
  {
    const int oldRowCount = rowCount();
    beginInsertRows( QModelIndex(), oldRowCount, oldRowCount + errors.size() );
  }

  topologyLayerErrors.append( errors );

  if ( layer == currentLayer() )
  {
    endInsertRows();
  }
}

void QgsGeometryValidationModel::onTopologyChecksCleared( QgsVectorLayer *layer )
{
  auto &topologyLayerErrors = mTopologyErrorStorage[layer];
  if ( topologyLayerErrors.empty() )
    return;

  if ( layer == currentLayer() )
  {
    beginRemoveRows( QModelIndex(), mErrorStorage.size(), rowCount() - 1 );
  }
  topologyLayerErrors.clear();
  if ( layer == currentLayer() )
  {
    endRemoveRows();
  }
}

void QgsGeometryValidationModel::onTopologyErrorUpdated( QgsVectorLayer *layer, QgsGeometryCheckError *error )
{
  if ( layer == mCurrentLayer )
  {
    int i = 0;
    const QList< std::shared_ptr< QgsGeometryCheckError > > errors = mTopologyErrorStorage[layer];
    for ( const std::shared_ptr< QgsGeometryCheckError > &currentError : errors )
    {
      if ( currentError.get() == error )
      {
        QModelIndex idx = index( i, 0, QModelIndex() );
        emit dataChanged( idx, idx );
      }
      ++i;
    }
  }
}

int QgsGeometryValidationModel::errorsForFeature( QgsVectorLayer *layer, QgsFeatureId fid )
{
  const QList< FeatureErrors > layerErrors = mErrorStorage[layer];
  int idx = 0;

  for ( const FeatureErrors &feature : layerErrors )
  {
    if ( feature.fid == fid )
      return idx;
    idx++;
  }
  return -1;
}

QgsFeature QgsGeometryValidationModel::getFeature( QgsFeatureId fid ) const
{
  if ( fid != mCachedFeature.id() || !mCachedFeature.isValid() )
  {
    QgsFeatureRequest request;
    request.setFilterFid( fid );
    request.setSubsetOfAttributes( mRequiredAttributes, mCurrentLayer->fields() );
    mCachedFeature = mCurrentLayer->getFeature( fid );
  }
  return mCachedFeature;
}
