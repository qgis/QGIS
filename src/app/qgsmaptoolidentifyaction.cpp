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

#include "qgsapplication.h"
#include "qgsattributetabledialog.h"
#include "qgscursors.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsidentifymenu.h"
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
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QVariant>

QgsMapToolIdentifyAction::QgsMapToolIdentifyAction( QgsMapCanvas * canvas )
    : QgsMapToolIdentify( canvas )
{
  mToolName = tr( "Identify" );
  // set cursor
  QPixmap myIdentifyQPixmap = QPixmap(( const char ** ) identify_cursor );
  mCursor = QCursor( myIdentifyQPixmap, 1, 1 );

  connect( this, SIGNAL( changedRasterResults( QList<IdentifyResult>& ) ), this, SLOT( handleChangedRasterResults( QList<IdentifyResult>& ) ) );

  mIdentifyMenu->setAllowMultipleReturn( true );

  QgsMapLayerAction* attrTableAction = new QgsMapLayerAction( tr( "Show attribute table" ), mIdentifyMenu, QgsMapLayer::VectorLayer, QgsMapLayerAction::MultipleFeatures );
  connect( attrTableAction, SIGNAL( triggeredForFeatures( QgsMapLayer*, const QList<QgsFeature> ) ), this, SLOT( showAttributeTable( QgsMapLayer*, const QList<QgsFeature> ) ) );
  identifyMenu()->addCustomAction( attrTableAction );
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

void QgsMapToolIdentifyAction::showAttributeTable( QgsMapLayer* layer, const QList<QgsFeature> featureList )
{
  resultsDialog()->clear();

  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( layer );
  if ( !vl )
    return;

  QString filter = "$id IN (";
  Q_FOREACH ( const QgsFeature &feature, featureList )
  {
    filter.append( QString( "%1," ).arg( feature.id() ) );
  }
  filter = filter.replace( QRegExp( ",$" ), ")" );

  QgsAttributeTableDialog* tableDialog = new QgsAttributeTableDialog( vl );
  tableDialog->setFilterExpression( filter );
  tableDialog->show();
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
  resultsDialog()->clear();
  connect( this, SIGNAL( identifyProgress( int, int ) ), QgisApp::instance(), SLOT( showProgress( int, int ) ) );
  connect( this, SIGNAL( identifyMessage( QString ) ), QgisApp::instance(), SLOT( showStatusMessage( QString ) ) );

  identifyMenu()->setResultsIfExternalAction( false );

  // enable the right click for extended menu so it behaves as a contextual menu
  // this would be removed when a true contextual menu is brought in QGIS
  bool extendedMenu = e->modifiers() == Qt::ShiftModifier || e->button() == Qt::RightButton;
  identifyMenu()->setExecWithSingleResult( extendedMenu );
  identifyMenu()->setShowFeatureActions( extendedMenu );
  IdentifyMode mode = extendedMenu ? LayerSelection : DefaultQgsSetting;

  QList<IdentifyResult> results = QgsMapToolIdentify::identify( e->x(), e->y(), mode );

  disconnect( this, SIGNAL( identifyProgress( int, int ) ), QgisApp::instance(), SLOT( showProgress( int, int ) ) );
  disconnect( this, SIGNAL( identifyMessage( QString ) ), QgisApp::instance(), SLOT( showStatusMessage( QString ) ) );

  if ( results.isEmpty() )
  {
    resultsDialog()->clear();
    QgisApp::instance()->statusBar()->showMessage( tr( "No features at this position found." ) );
  }
  else
  {
    // Show the dialog before items are inserted so that items can resize themselves
    // according to dialog size also the first time, see also #9377
    if ( results.size() != 1 || !QSettings().value( "/Map/identifyAutoFeatureForm", false ).toBool() )
      resultsDialog()->QDialog::show();

    QList<IdentifyResult>::const_iterator result;
    for ( result = results.begin(); result != results.end(); ++result )
    {
      resultsDialog()->addFeature( *result );
    }

    // Call QgsIdentifyResultsDialog::show() to adjust with items
    resultsDialog()->show();
  }

  // update possible view modes
  resultsDialog()->updateViewModes();
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


