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
#include "qgsmaterial3dhandler.h"

#include <QVBoxLayout>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QCamera>

QgsMaterialPreviewWidget::QgsMaterialPreviewWidget( QWidget *parent )
  : QWidget( parent )
{
  mView = new Qt3DExtras::Qt3DWindow();
  mView->defaultFrameGraph()->setClearColor( QColor( 40, 40, 40 ) );

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

void QgsMaterialPreviewWidget::setupCamera( Qt3DRender::QCamera *camera )
{
  camera->lens()->setPerspectiveProjection( 45.0f, 1.0f, 0.1f, 100.0f );
  camera->setPosition( { 0, 0, 4 } );
  camera->setViewCenter( { 0, 0, 0 } );
}

void QgsMaterialPreviewWidget::updatePreview( const QgsAbstractMaterialSettings *settings )
{
  const QgsAbstractMaterial3DHandler *handler = Qgs3D::handlerForMaterialSettings( settings );
  if ( !handler )
    return;

  QgsMaterialContext context;
  if ( !mPreviewScene )
  {
    mPreviewScene = handler->createPreviewScene( settings, context, mView, mSceneRoot );
  }
  else
  {
    handler->updatePreviewScene( mPreviewScene, settings, context );
  }
}
