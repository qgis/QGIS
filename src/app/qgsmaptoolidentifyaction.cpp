/***************************************************************************
    qgsmaptoolidentify.cpp  -  map tool for identifying features
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscursors.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgsrasterlayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmaplayerregistry.h"
#include "qgisapp.h"
#include "qgsrendererv2.h"

#include <QSettings>
#include <QMessageBox>
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QVariant>

QgsMapToolIdentifyAction::QgsMapToolIdentifyAction( QgsMapCanvas * canvas )
    : QgsMapToolIdentify( canvas )
{
  // set cursor
  QPixmap myIdentifyQPixmap = QPixmap(( const char ** ) identify_cursor );
  mCursor = QCursor( myIdentifyQPixmap, 1, 1 );

  connect( this, SIGNAL( changedRasterResults( QList<IdentifyResult>& ) ), this, SLOT( handleChangedRasterResults( QList<IdentifyResult>& ) ) );
}

QgsMapToolIdentifyAction::~QgsMapToolIdentifyAction()
{
  if ( mResultsDialog )
  {
    mResultsDialog->done( 0 );
  }
}

QgsIdentifyResultsDialog *QgsMapToolIdentifyAction::resultsDialog()
{
  if ( !mResultsDialog )
  {
    mResultsDialog = new QgsIdentifyResultsDialog( mCanvas, mCanvas->window() );

    connect( mResultsDialog, SIGNAL( formatChanged( QgsRasterLayer * ) ), this, SLOT( formatChanged( QgsRasterLayer * ) ) );
    connect( mResultsDialog, SIGNAL( copyToClipboard( QgsFeatureStore & ) ), this, SLOT( handleCopyToClipboard( QgsFeatureStore & ) ) );
  }

  return mResultsDialog;
}

void QgsMapToolIdentifyAction::canvasMoveEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentifyAction::canvasPressEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentifyAction::canvasReleaseEvent( QMouseEvent *e )
{
  if ( !mCanvas || mCanvas->isDrawing() )
  {
    return;
  }

  resultsDialog()->clear();
  connect( this, SIGNAL( identifyProgress( int, int ) ), QgisApp::instance(), SLOT( showProgress( int, int ) ) );
  connect( this, SIGNAL( identifyMessage( QString ) ), QgisApp::instance(), SLOT( showStatusMessage( QString ) ) );

  QList<IdentifyResult> results;
  QSettings settings;
  IdentifyMode mode = static_cast<IdentifyMode>( settings.value( "/Map/identifyMode", 0 ).toInt() );
  if ( mode == LayerSelection )
  {
    QMap< QgsMapLayer*, QList<IdentifyResult> > layerIdResults;

    QList<QgsMapLayer*> canvasLayers = mCanvas->layers();
    QList<QgsMapLayer*>::iterator it = canvasLayers.begin();
    for ( ; it != canvasLayers.end(); ++it )
    {
      QList<IdentifyResult> idResult = QgsMapToolIdentify::identify( e->x(), e->y(), QList<QgsMapLayer*>() << *it );
      if ( !idResult.isEmpty() )
      {
        layerIdResults.insert( *it, idResult );
      }
    }

    //show QMenu with available layers
    QMenu layerSelectionMenu;
    QMap< QgsMapLayer*, QList<IdentifyResult> >::const_iterator resultIt = layerIdResults.constBegin();
    for ( ; resultIt != layerIdResults.constEnd(); ++resultIt )
    {
      QAction* action = new QAction( resultIt.key()->name(), 0 );
      action->setData( resultIt.key()->id() );
      //QObject::connect( action, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
      layerSelectionMenu.addAction( action );
    }

    QAction* selectedAction = layerSelectionMenu.exec( e->globalPos() );
    if ( selectedAction )
    {
      QgsMapLayer* selectedLayer = QgsMapLayerRegistry::instance()->mapLayer( selectedAction->data().toString() );
      results = layerIdResults[ selectedLayer ];
    }
  }
  else
  {
    results = QgsMapToolIdentify::identify( e->x(), e->y() );
  }

  disconnect( this, SIGNAL( identifyProgress( int, int ) ), QgisApp::instance(), SLOT( showProgress( int, int ) ) );
  disconnect( this, SIGNAL( identifyMessage( QString ) ), QgisApp::instance(), SLOT( showStatusMessage( QString ) ) );


  QList<IdentifyResult>::const_iterator result;
  for ( result = results.begin(); result != results.end(); ++result )
  {
    resultsDialog()->addFeature( *result );
  }

  if ( !results.isEmpty() )
  {
    resultsDialog()->show();
  }
  else
  {
    QSettings mySettings;
    bool myDockFlag = mySettings.value( "/qgis/dockIdentifyResults", false ).toBool();
    if ( !myDockFlag )
    {
      resultsDialog()->hide();
    }
    else
    {
      resultsDialog()->clear();
    }
    QgisApp::instance()->statusBar()->showMessage( tr( "No features at this position found." ) );
  }
}

void QgsMapToolIdentifyAction::handleChangedRasterResults( QList<IdentifyResult> &results )
{
  // Add new result after raster format change
  QgsDebugMsg( QString( "%1 raster results" ).arg( results.size() ) );
  QList<IdentifyResult>::const_iterator rresult;
  for ( rresult = results.begin(); rresult != results.end(); ++rresult )
  {
    if ( rresult->mLayer->type() == QgsMapLayer::RasterLayer )
    {
      resultsDialog()->addFeature( *rresult );
    }
  }
}

void QgsMapToolIdentifyAction::activate()
{
  resultsDialog()->activate();
  QgsMapTool::activate();
}

void QgsMapToolIdentifyAction::deactivate()
{
  resultsDialog()->deactivate();
  QgsMapTool::deactivate();
}

QGis::UnitType QgsMapToolIdentifyAction::displayUnits()
{
  // Get the units for display
  QSettings settings;
  return QGis::fromLiteral( settings.value( "/qgis/measure/displayunits", QGis::toLiteral( QGis::Meters ) ).toString() );
}

void QgsMapToolIdentifyAction::handleCopyToClipboard( QgsFeatureStore & featureStore )
{
  QgsDebugMsg( QString( "features count = %1" ).arg( featureStore.features().size() ) );
  emit copyToClipboard( featureStore );
}

