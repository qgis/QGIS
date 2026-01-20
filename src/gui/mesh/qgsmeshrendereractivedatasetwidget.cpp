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

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmeshlayer.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmessagelog.h"

#include <QDateTime>
#include <QIcon>

#include "moc_qgsmeshrendereractivedatasetwidget.cpp"

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
          msg += u"<p> <h3> %1 </h3> "_s.arg( tr( "Scalar dataset" ) );
          msg += metadata( mActiveScalarDatasetGroup );
          msg += u"</p> <p> <h3> %1 </h3>"_s.arg( tr( "Vector dataset" ) );
          msg += metadata( mActiveVectorDatasetGroup );
          msg += "</p>"_L1;
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
  msg += "<table>"_L1;

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
  msg += u"<tr><td>%1</td><td>%2</td></tr>"_s
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
  msg += u"<tr><td>%1</td><td>%2</td></tr>"_s
           .arg( tr( "Data type" ) )
           .arg( definedOn );

  msg += u"<tr><td>%1</td><td>%2</td></tr>"_s
           .arg( tr( "Is vector" ) )
           .arg( gmeta.isVector() ? tr( "Yes" ) : tr( "No" ) );

  const auto options = gmeta.extraOptions();
  for ( auto it = options.constBegin(); it != options.constEnd(); ++it )
  {
    if ( it.key() == "classification"_L1 )
    {
      msg += u"<tr><td>%1</td></tr>"_s.arg( tr( "Classified values" ) );
      continue;
    }
    msg += u"<tr><td>%1</td><td>%2</td></tr>"_s.arg( it.key() ).arg( it.value() );
  }

  msg += "</table>"_L1;

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
