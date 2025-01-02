/***************************************************************************
    qgsmeshrendereractivedatasetwidget.cpp
    ---------------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrendereractivedatasetwidget.h"
#include "moc_qgsmeshrendereractivedatasetwidget.cpp"

#include <QDateTime>
#include <QIcon>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"
#include "qgsmeshrenderersettings.h"

QgsMeshRendererActiveDatasetWidget::QgsMeshRendererActiveDatasetWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mDatasetGroupTreeView, &QgsMeshActiveDatasetGroupTreeView::activeScalarGroupChanged, this, &QgsMeshRendererActiveDatasetWidget::onActiveScalarGroupChanged );
  connect( mDatasetGroupTreeView, &QgsMeshActiveDatasetGroupTreeView::activeVectorGroupChanged, this, &QgsMeshRendererActiveDatasetWidget::onActiveVectorGroupChanged );
}

QgsMeshRendererActiveDatasetWidget::~QgsMeshRendererActiveDatasetWidget() = default;


void QgsMeshRendererActiveDatasetWidget::setLayer( QgsMeshLayer *layer )
{
  if ( mMeshLayer )
  {
    disconnect( mMeshLayer, &QgsMeshLayer::activeScalarDatasetGroupChanged, mDatasetGroupTreeView, &QgsMeshActiveDatasetGroupTreeView::setActiveScalarGroup );
    disconnect( mMeshLayer, &QgsMeshLayer::activeVectorDatasetGroupChanged, mDatasetGroupTreeView, &QgsMeshActiveDatasetGroupTreeView::setActiveVectorGroup );
  }

  mMeshLayer = layer;

  mDatasetGroupTreeView->setLayer( layer );
  connect( layer, &QgsMeshLayer::activeScalarDatasetGroupChanged, mDatasetGroupTreeView, &QgsMeshActiveDatasetGroupTreeView::setActiveScalarGroup );
  connect( layer, &QgsMeshLayer::activeVectorDatasetGroupChanged, mDatasetGroupTreeView, &QgsMeshActiveDatasetGroupTreeView::setActiveVectorGroup );
}

int QgsMeshRendererActiveDatasetWidget::activeScalarDatasetGroup() const
{
  return mActiveScalarDatasetGroup;
}

int QgsMeshRendererActiveDatasetWidget::activeVectorDatasetGroup() const
{
  return mActiveVectorDatasetGroup;
}


void QgsMeshRendererActiveDatasetWidget::onActiveScalarGroupChanged( int groupIndex )
{
  if ( groupIndex == mActiveScalarDatasetGroup )
    return;

  mActiveScalarDatasetGroup = groupIndex;
  updateMetadata();
  emit activeScalarGroupChanged( mActiveScalarDatasetGroup );
  emit widgetChanged();
}

void QgsMeshRendererActiveDatasetWidget::onActiveVectorGroupChanged( int groupIndex )
{
  if ( groupIndex == mActiveVectorDatasetGroup )
    return;

  mActiveVectorDatasetGroup = groupIndex;
  updateMetadata();
  emit activeVectorGroupChanged( mActiveVectorDatasetGroup );
  emit widgetChanged();
}

void QgsMeshRendererActiveDatasetWidget::updateMetadata()
{
  QString msg;

  if ( !mMeshLayer || !mMeshLayer->dataProvider() )
  {
    msg += tr( "Invalid mesh layer selected" );
  }
  else
  {
    if ( mActiveScalarDatasetGroup > -1 )
    {
      if ( mActiveVectorDatasetGroup > -1 )
      {
        if ( mActiveScalarDatasetGroup == mActiveVectorDatasetGroup )
        {
          msg += metadata( mActiveScalarDatasetGroup );
        }
        else
        {
          msg += QStringLiteral( "<p> <h3> %1 </h3> " ).arg( tr( "Scalar dataset" ) );
          msg += metadata( mActiveScalarDatasetGroup );
          msg += QStringLiteral( "</p> <p> <h3> %1 </h3>" ).arg( tr( "Vector dataset" ) );
          msg += metadata( mActiveVectorDatasetGroup );
          msg += QLatin1String( "</p>" );
        }
      }
      else
      {
        msg += metadata( mActiveScalarDatasetGroup );
      }
    }
    else
    {
      if ( mActiveVectorDatasetGroup > -1 )
      {
        msg += metadata( mActiveVectorDatasetGroup );
      }
      else
      {
        msg += tr( "No mesh dataset selected" );
      }
    }
  }

  mActiveDatasetMetadata->setText( msg );
}


QString QgsMeshRendererActiveDatasetWidget::metadata( QgsMeshDatasetIndex datasetIndex )
{
  QString msg;
  msg += QLatin1String( "<table>" );

  QString definedOnMesh;
  if ( mMeshLayer->contains( QgsMesh::ElementType::Face ) )
  {
    if ( mMeshLayer->contains( QgsMesh::ElementType::Edge ) )
    {
      definedOnMesh = tr( "faces and edges" );
    }
    else
    {
      definedOnMesh = tr( "faces" );
    }
  }
  else if ( mMeshLayer->contains( QgsMesh::ElementType::Edge ) )
  {
    definedOnMesh = tr( "edges" );
  }
  else
  {
    definedOnMesh = tr( "invalid mesh" );
  }
  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
           .arg( tr( "Mesh type" ) )
           .arg( definedOnMesh );

  const QgsMeshDatasetGroupMetadata gmeta = mMeshLayer->datasetGroupMetadata( datasetIndex );
  QString definedOn;
  switch ( gmeta.dataType() )
  {
    case QgsMeshDatasetGroupMetadata::DataOnVertices:
      definedOn = tr( "vertices" );
      break;
    case QgsMeshDatasetGroupMetadata::DataOnFaces:
      definedOn = tr( "faces" );
      break;
    case QgsMeshDatasetGroupMetadata::DataOnVolumes:
      definedOn = tr( "volumes" );
      break;
    case QgsMeshDatasetGroupMetadata::DataOnEdges:
      definedOn = tr( "edges" );
      break;
  }
  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
           .arg( tr( "Data type" ) )
           .arg( definedOn );

  msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
           .arg( tr( "Is vector" ) )
           .arg( gmeta.isVector() ? tr( "Yes" ) : tr( "No" ) );

  const auto options = gmeta.extraOptions();
  for ( auto it = options.constBegin(); it != options.constEnd(); ++it )
  {
    if ( it.key() == QLatin1String( "classification" ) )
    {
      msg += QStringLiteral( "<tr><td>%1</td></tr>" ).arg( tr( "Classified values" ) );
      continue;
    }
    msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" ).arg( it.key() ).arg( it.value() );
  }

  msg += QLatin1String( "</table>" );

  return msg;
}

void QgsMeshRendererActiveDatasetWidget::syncToLayer()
{
  setEnabled( mMeshLayer );

  whileBlocking( mDatasetGroupTreeView )->syncToLayer();

  mActiveScalarDatasetGroup = mDatasetGroupTreeView->activeScalarGroup();
  mActiveVectorDatasetGroup = mDatasetGroupTreeView->activeVectorGroup();

  updateMetadata();
}
