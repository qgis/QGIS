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
#include "qgisapp.h"
#include "qgsattributetabledialog.h"
#include "qgscursors.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfields.h"
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
#include "qgsrenderer.h"
#include "qgsunittypes.h"
#include "qgsstatusbar.h"
#include "qgsactionscoperegistry.h"

#include "qgssettings.h"
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QVariant>

QgsMapToolIdentifyAction::QgsMapToolIdentifyAction( QgsMapCanvas *canvas )
  : QgsMapToolIdentify( canvas )
{
  mToolName = tr( "Identify" );
  // set cursor
  QPixmap myIdentifyQPixmap = QPixmap( ( const char ** ) identify_cursor );
  mCursor = QCursor( myIdentifyQPixmap, 1, 1 );

  connect( this, &QgsMapToolIdentify::changedRasterResults, this, &QgsMapToolIdentifyAction::handleChangedRasterResults );

  mIdentifyMenu->setAllowMultipleReturn( true );

  QgsMapLayerAction *attrTableAction = new QgsMapLayerAction( tr( "Show attribute table" ), mIdentifyMenu, QgsMapLayer::VectorLayer, QgsMapLayerAction::MultipleFeatures );
  connect( attrTableAction, &QgsMapLayerAction::triggeredForFeatures, this, &QgsMapToolIdentifyAction::showAttributeTable );
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

    connect( mResultsDialog.data(), static_cast<void ( QgsIdentifyResultsDialog::* )( QgsRasterLayer * )>( &QgsIdentifyResultsDialog::formatChanged ), this, &QgsMapToolIdentify::formatChanged );
    connect( mResultsDialog.data(), &QgsIdentifyResultsDialog::copyToClipboard, this, &QgsMapToolIdentifyAction::handleCopyToClipboard );
  }

  return mResultsDialog;
}

void QgsMapToolIdentifyAction::showAttributeTable( QgsMapLayer *layer, const QList<QgsFeature> &featureList )
{
  resultsDialog()->clear();

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vl )
    return;

  QString filter = QStringLiteral( "$id IN (" );
  Q_FOREACH ( const QgsFeature &feature, featureList )
  {
    filter.append( QStringLiteral( "%1," ).arg( feature.id() ) );
  }
  filter = filter.replace( QRegExp( ",$" ), QStringLiteral( ")" ) );

  QgsAttributeTableDialog *tableDialog = new QgsAttributeTableDialog( vl );
  tableDialog->setFilterExpression( filter );
  tableDialog->show();
}

void QgsMapToolIdentifyAction::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentifyAction::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentifyAction::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  resultsDialog()->clear();
  connect( this, &QgsMapToolIdentifyAction::identifyProgress, QgisApp::instance(), &QgisApp::showProgress );
  connect( this, &QgsMapToolIdentifyAction::identifyMessage, QgisApp::instance(), &QgisApp::showStatusMessage );

  setClickContextScope( toMapCoordinates( e->pos() ) );

  identifyMenu()->setResultsIfExternalAction( false );

  // enable the right click for extended menu so it behaves as a contextual menu
  // this would be removed when a true contextual menu is brought in QGIS
  bool extendedMenu = e->modifiers() == Qt::ShiftModifier || e->button() == Qt::RightButton;
  identifyMenu()->setExecWithSingleResult( extendedMenu );
  identifyMenu()->setShowFeatureActions( extendedMenu );
  IdentifyMode mode = extendedMenu ? LayerSelection : DefaultQgsSetting;

  QList<IdentifyResult> results = QgsMapToolIdentify::identify( e->x(), e->y(), mode );

  disconnect( this, &QgsMapToolIdentifyAction::identifyProgress, QgisApp::instance(), &QgisApp::showProgress );
  disconnect( this, &QgsMapToolIdentifyAction::identifyMessage, QgisApp::instance(), &QgisApp::showStatusMessage );

  if ( results.isEmpty() )
  {
    resultsDialog()->clear();
    QgisApp::instance()->statusBarIface()->showMessage( tr( "No features at this position found." ) );
  }
  else
  {
    // Show the dialog before items are inserted so that items can resize themselves
    // according to dialog size also the first time, see also #9377
    if ( results.size() != 1 || !QgsSettings().value( QStringLiteral( "/Map/identifyAutoFeatureForm" ), false ).toBool() )
      resultsDialog()->QDialog::show();

    QList<IdentifyResult>::const_iterator result;
    for ( result = results.constBegin(); result != results.constEnd(); ++result )
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
  for ( rresult = results.constBegin(); rresult != results.constEnd(); ++rresult )
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

QgsUnitTypes::DistanceUnit QgsMapToolIdentifyAction::displayDistanceUnits() const
{
  return QgsProject::instance()->distanceUnits();
}

QgsUnitTypes::AreaUnit QgsMapToolIdentifyAction::displayAreaUnits() const
{
  return QgsProject::instance()->areaUnits();
}

void QgsMapToolIdentifyAction::handleCopyToClipboard( QgsFeatureStore &featureStore )
{
  QgsDebugMsg( QString( "features count = %1" ).arg( featureStore.features().size() ) );
  emit copyToClipboard( featureStore );
}

void QgsMapToolIdentifyAction::setClickContextScope( const QgsPointXY &point )
{
  QgsExpressionContextScope clickScope;
  clickScope.addVariable( QgsExpressionContextScope::StaticVariable( QString( "click_x" ), point.x(), true ) );
  clickScope.addVariable( QgsExpressionContextScope::StaticVariable( QString( "click_y" ), point.y(), true ) );

  resultsDialog()->setExpressionContextScope( clickScope );

  if ( mIdentifyMenu )
  {
    mIdentifyMenu->setExpressionContextScope( clickScope );
  }
}
