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

  connect( mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::activeScalarGroupChanged,
           this, &QgsMeshRendererActiveDatasetWidget::onActiveScalarGroupChanged );
  connect( mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::activeVectorGroupChanged,
           this, &QgsMeshRendererActiveDatasetWidget::onActiveVectorGroupChanged );
}

QgsMeshRendererActiveDatasetWidget::~QgsMeshRendererActiveDatasetWidget() = default;


void QgsMeshRendererActiveDatasetWidget::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
  mDatasetGroupTreeView->setLayer( layer );
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

  if ( !mMeshLayer ||
       !mMeshLayer->dataProvider() )
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
          msg += QStringLiteral( "</p>" );
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
  msg += QStringLiteral( "<table>" );

  QString definedOnMesh;
  if ( mMeshLayer->dataProvider()->contains( QgsMesh::ElementType::Face ) )
  {
    if ( mMeshLayer->dataProvider()->contains( QgsMesh::ElementType::Edge ) )
    {
      definedOnMesh = tr( "faces and edges" );
    }
    else
    {
      definedOnMesh = tr( "faces" );
    }
  }
  else if ( mMeshLayer->dataProvider()->contains( QgsMesh::ElementType::Edge ) )
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

  const QgsMeshDatasetGroupMetadata gmeta = mMeshLayer->dataProvider()->datasetGroupMetadata( datasetIndex );
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
    msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" ).arg( it.key() ).arg( it.value() );
  }

  msg += QStringLiteral( "</table>" );

  return msg;
}

void QgsMeshRendererActiveDatasetWidget::syncToLayer()
{
  setEnabled( mMeshLayer );

  whileBlocking( mDatasetGroupTreeView )->syncToLayer();

  if ( mMeshLayer )
  {
    const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
    mActiveScalarDatasetGroup = mDatasetGroupTreeView->activeScalarGroup();
    mActiveVectorDatasetGroup = mDatasetGroupTreeView->activeVectorGroup();
  }
  else
  {
    mActiveScalarDatasetGroup = -1;
    mActiveVectorDatasetGroup = -1;
  }

  updateMetadata();
}
