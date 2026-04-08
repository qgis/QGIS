/***************************************************************************
  qgsmaterialpreviewwidget.cpp
  --------------------------------------
  Date                 : March 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaterialpreviewwidget.h"

#include "qgs3d.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsapplication.h"
#include "qgsmaterialregistry.h"

#include <QActionGroup>
#include <QMenu>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QCamera>

#include "moc_qgsmaterialpreviewwidget.cpp"

QgsMaterialPreviewWidget::QgsMaterialPreviewWidget( QWidget *parent )
  : QWidget( parent )
{
  mView = new Qt3DExtras::Qt3DWindow();
  mView->defaultFrameGraph()->setClearColor( palette().color( QPalette::ColorGroup::Active, QPalette::ColorRole::Window ) );

  mView->installEventFilter( this );

  QWidget *container = QWidget::createWindowContainer( mView, this );
  container->setMinimumSize( 200, 200 );
  container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  auto *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( container );
  setLayout( layout );

  mSceneRoot = new Qt3DCore::QEntity;
  setupCamera( mView->camera() );
  mView->setRootEntity( mSceneRoot );
}

void QgsMaterialPreviewWidget::setMaterialType( const QString &type )
{
  if ( const QgsMaterialSettingsMetadata *metadata = dynamic_cast< const QgsMaterialSettingsMetadata * >( QgsApplication::materialRegistry()->materialSettingsMetadata( type ) ) )
  {
    if ( const QgsAbstractMaterial3DHandler *handler = metadata->handler() )
    {
      mMeshTypes = handler->previewMeshTypes();
      mPreviewSceneType = mMeshTypes.at( 0 ).type;
    }
  }
}

void QgsMaterialPreviewWidget::setupCamera( Qt3DRender::QCamera *camera )
{
  camera->lens()->setPerspectiveProjection( 45.0f, 1.0f, 0.1f, 100.0f );
  camera->setPosition( { 0, 0, 4 } );
  camera->setViewCenter( { 0, 0, 0 } );
}

void QgsMaterialPreviewWidget::updatePreview( const QgsAbstractMaterialSettings *settings )
{
  mLastPreviewSettings.reset( settings->clone() );
  const QgsAbstractMaterial3DHandler *handler = Qgs3D::handlerForMaterialSettings( mLastPreviewSettings.get() );
  if ( !handler )
    return;

  QgsMaterialContext context;
  if ( !mPreviewScene )
  {
    delete mPreviewScene;
    mPreviewScene = handler->createPreviewScene( mLastPreviewSettings.get(), mPreviewSceneType, context, mView, mSceneRoot );
  }
  else
  {
    if ( !handler->updatePreviewScene( mPreviewScene, mLastPreviewSettings.get(), context ) )
    {
      delete mPreviewScene;
      mPreviewScene = handler->createPreviewScene( mLastPreviewSettings.get(), mPreviewSceneType, context, mView, mSceneRoot );
    }
  }
}

bool QgsMaterialPreviewWidget::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mView && event->type() == QEvent::MouseButtonPress )
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
    if ( mouseEvent->button() == Qt::RightButton )
    {
      QMenu menu( this );
      auto actionGroup = new QActionGroup( &menu );
      actionGroup->setExclusive( true );
      for ( const QgsAbstractMaterial3DHandler::PreviewMeshType &type : mMeshTypes )
      {
        QAction *action = new QAction( type.displayName, &menu );
        action->setCheckable( true );
        action->setChecked( type.type == mPreviewSceneType );
        connect( action, &QAction::toggled, this, [this, type]( bool checked ) {
          if ( checked )
          {
            mPreviewSceneType = type.type;
            mPreviewScene->deleteLater();
            mPreviewScene = nullptr;
            updatePreview( mLastPreviewSettings.get() );
          }
        } );
        menu.addAction( action );
        actionGroup->addAction( action );
      }
      menu.exec( mouseEvent->globalPosition().toPoint() );
      return true;
    }
  }
  return QWidget::eventFilter( watched, event );
}
