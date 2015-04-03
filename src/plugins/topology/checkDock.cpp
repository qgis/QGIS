/***************************************************************************
  checkDock.cpp
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include <QProgressDialog>

#include "checkDock.h"

#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayer.h>
#include <qgsmaplayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsgeometry.h>
#include <qgsvertexmarker.h>
#include <qgsfeature.h>
#include <qgsmapcanvas.h>
#include <qgsrubberband.h>
#include <qgsproviderregistry.h>
#include <qgslogger.h>
#include <qgsspatialindex.h>
#include <qgisinterface.h>
#include <qgsmessagelog.h>

#include "topolTest.h"
#include "rulesDialog.h"
#include "dockModel.h"

//class QgisInterface;

checkDock::checkDock( QgisInterface* qIface, QWidget* parent )
    : QDockWidget( parent ), Ui::checkDock()
{
  mTest = new topolTest( qIface );

  setupUi( this );

  // hide the fix-related stuff, needs more work
  qgsInterface = qIface;
  mFixButton->hide();
  mFixBox->hide();

  mErrorListModel = new DockModel( mErrorList, parent );
  mErrorTableView->setModel( mErrorListModel );
  mErrorTableView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mErrorTableView->verticalHeader()->setDefaultSectionSize( 20 );

  mLayerRegistry = QgsMapLayerRegistry::instance();
  mConfigureDialog = new rulesDialog( mTest->testMap(), qIface, parent );
  mTestTable = mConfigureDialog->rulesTable();

  mValidateExtentButton->setIcon( QIcon( ":/topology/validateExtent.png" ) );
  mValidateAllButton->setIcon( QIcon( ":/topology/validateAll.png" ) );
  mConfigureButton->setIcon( QIcon( ":/topology/configureRules.png" ) );

  QgsMapCanvas* canvas = qIface->mapCanvas();// mQgisApp->mapCanvas();
  mRBFeature1 = new QgsRubberBand( canvas );
  mRBFeature2 = new QgsRubberBand( canvas );
  mRBConflict = new QgsRubberBand( canvas );

  mRBFeature1->setColor( QColor( 0, 0, 255, 65 ) );
  mRBFeature2->setColor( QColor( 0, 255, 0, 65 ) );
  mRBConflict->setColor( QColor( 255, 0, 0, 65 ) );

  mRBFeature1->setWidth( 5 );
  mRBFeature2->setWidth( 5 );
  mRBConflict->setWidth( 5 );

  mVMConflict = 0;
  mVMFeature1 = 0;
  mVMFeature2 = 0;

  connect( mConfigureButton, SIGNAL( clicked() ), this, SLOT( configure() ) );
  connect( mValidateAllButton, SIGNAL( clicked() ), this, SLOT( validateAll() ) );
  //connect( mValidateSelectedButton, SIGNAL( clicked() ), this, SLOT( validateSelected() ) );
  connect( mValidateExtentButton, SIGNAL( clicked() ), this, SLOT( validateExtent() ) );
  connect( mToggleRubberband, SIGNAL( clicked() ), this, SLOT( toggleErrorMarker() ) );

  connect( mFixButton, SIGNAL( clicked() ), this, SLOT( fix() ) );
  connect( mErrorTableView, SIGNAL( clicked( const QModelIndex & ) ), this, SLOT( errorListClicked( const QModelIndex & ) ) );

  connect( mLayerRegistry, SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( parseErrorListByLayer( QString ) ) );

  connect( this, SIGNAL( visibilityChanged( bool ) ), this, SLOT( updateRubberBands( bool ) ) );
  connect( qgsInterface, SIGNAL( newProjectCreated() ), mConfigureDialog, SLOT( clearRules() ) );
  connect( qgsInterface, SIGNAL( newProjectCreated() ), this, SLOT( deleteErrors() ) );

}

checkDock::~checkDock()
{
  delete mConfigureDialog;
  mRbErrorMarkers.clear();
  clearVertexMarkers();
  // delete errors in list
  deleteErrors();
  delete mErrorListModel;
}

void checkDock::clearVertexMarkers()
{
  if ( mVMConflict )
  {
    delete mVMConflict;
    mVMConflict = 0;
  }
  if ( mVMFeature1 )
  {
    delete mVMFeature1;
    mVMFeature1 = 0;
  }
  if ( mVMFeature2 )
  {
    delete mVMFeature2;
    mVMFeature2 = 0;
  }
}

void checkDock::updateRubberBands( bool visible )
{
  if ( !visible )
  {
    mRBConflict->reset();
    mRBFeature1->reset();
    mRBFeature2->reset();

    clearVertexMarkers();
  }
}

void checkDock::deleteErrors()
{
  QList<TopolError*>::Iterator it = mErrorList.begin();
  for ( ; it != mErrorList.end(); ++it )
    delete *it;

  mErrorList.clear();
  mErrorListModel->resetModel();

  qDeleteAll( mRbErrorMarkers );
  mRbErrorMarkers.clear();
}

void checkDock::parseErrorListByLayer( QString layerId )
{
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer*>( mLayerRegistry->mapLayers()[layerId] );
  QList<TopolError*>::Iterator it = mErrorList.begin();

  while ( it != mErrorList.end() )
  {
    FeatureLayer fl1 = ( *it )->featurePairs().first();
    FeatureLayer fl2 = ( *it )->featurePairs()[1];
    if ( fl1.layer == layer || fl2.layer == layer )
    {
      it = mErrorList.erase( it );
    }
    else
      ++it;
  }

  mErrorListModel->resetModel();
  mComment->setText( tr( "No errors were found" ) );
}

void checkDock::parseErrorListByFeature( int featureId )
{
  QList<TopolError*>::Iterator it = mErrorList.begin();

  while ( it != mErrorList.end() )
  {
    FeatureLayer fl1 = ( *it )->featurePairs().first();
    FeatureLayer fl2 = ( *it )->featurePairs()[1];
    if ( fl1.feature.id() == featureId || fl2.feature.id() == featureId )
    {
      it = mErrorList.erase( it );
    }
    else
      ++it;
  }

  mComment->setText( tr( "No errors were found" ) );
  mErrorListModel->resetModel();
}

void checkDock::configure()
{
  mConfigureDialog->initGui();
  mConfigureDialog->show();
}

void checkDock::errorListClicked( const QModelIndex& index )
{
  int row = index.row();
  QgsRectangle r = mErrorList[row]->boundingBox();
  r.scale( 1.5 );
  QgsMapCanvas* canvas = qgsInterface->mapCanvas();
  canvas->setExtent( r );
  canvas->refresh();

  mFixBox->clear();
  mFixBox->addItems( mErrorList[row]->fixNames() );
  mFixBox->setCurrentIndex( mFixBox->findText( tr( "Select automatic fix" ) ) );

  QgsFeature f;
  QgsGeometry* g;
  FeatureLayer fl = mErrorList[row]->featurePairs().first();
  if ( !fl.layer )
  {
    QgsMessageLog::logMessage( tr( "Invalid first layer" ), tr( "Topology plugin" ) );
    return;
  }

  //fl1.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl1.feature.id() ) ).nextFeature( f1 );

  fl.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl.feature.id() ) ).nextFeature( f );
  g = f.geometry();
  if ( !g )
  {
    QgsMessageLog::logMessage( tr( "Invalid first geometry" ), tr( "Topology plugin" ) );
    QMessageBox::information( this, tr( "Topology test" ), tr( "Feature not found in the layer.\nThe layer has probably changed.\nRun topology check again." ) );
    return;
  }

  clearVertexMarkers();

  // use vertex marker when highlighting a point
  // and rubber band otherwise
  if ( g->type() == QGis::Point )
  {
    mVMFeature1 = new QgsVertexMarker( canvas );
    mVMFeature1->setIconType( QgsVertexMarker::ICON_X );
    mVMFeature1->setPenWidth( 5 );
    mVMFeature1->setIconSize( 5 );
    mVMFeature1->setColor( "blue" );
    mVMFeature1->setCenter( g->asPoint() );
  }
  else
    mRBFeature1->setToGeometry( g, fl.layer );

  fl = mErrorList[row]->featurePairs()[1];
  if ( !fl.layer )
  {
    QgsMessageLog::logMessage( tr( "Invalid second layer" ), tr( "Topology plugin" ) );
    return;
  }


  fl.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl.feature.id() ) ).nextFeature( f );
  g = f.geometry();
  if ( !g )
  {
    QgsMessageLog::logMessage( tr( "Invalid second geometry" ), tr( "Topology plugin" ) );
    QMessageBox::information( this, tr( "Topology test" ), tr( "Feature not found in the layer.\nThe layer has probably changed.\nRun topology check again." ) );
    return;
  }

  if ( g->type() == QGis::Point )
  {
    mVMFeature2 = new QgsVertexMarker( canvas );
    mVMFeature2->setIconType( QgsVertexMarker::ICON_BOX );
    mVMFeature2->setPenWidth( 5 );
    mVMFeature2->setIconSize( 5 );
    mVMFeature2->setColor( "green" );
    mVMFeature2->setCenter( g->asPoint() );
  }
  else
    mRBFeature2->setToGeometry( g, fl.layer );

  if ( !mErrorList[row]->conflict() )
  {
    QgsMessageLog::logMessage( tr( "Invalid conflict" ), tr( "Topology plugin" ) );
    return;
  }

  if ( mErrorList[row]->conflict()->type() == QGis::Point )
  {
    mVMConflict = new QgsVertexMarker( canvas );
    mVMConflict->setIconType( QgsVertexMarker::ICON_BOX );
    mVMConflict->setPenWidth( 5 );
    mVMConflict->setIconSize( 5 );
    mVMConflict->setColor( "red" );
    mVMConflict->setCenter( mErrorList[row]->conflict()->asPoint() );
  }
  else
    mRBConflict->setToGeometry( mErrorList[row]->conflict(), fl.layer );
}

void checkDock::fix()
{
  int row = mErrorTableView->currentIndex().row();
  QString fixName = mFixBox->currentText();

  if ( row == -1 )
    return;

  mRBFeature1->reset();
  mRBFeature2->reset();
  mRBConflict->reset();

  clearVertexMarkers();

  if ( mErrorList[row]->fix( fixName ) )
  {
    mErrorList.removeAt( row );
    mErrorListModel->resetModel();
    //parseErrorListByFeature();
    mComment->setText( tr( "%1 errors were found" ).arg( mErrorList.count() ) );
    qgsInterface->mapCanvas()->refresh();
  }
  else
    QMessageBox::information( this, tr( "Topology fix error" ), tr( "Fixing failed!" ) );
}

void checkDock::runTests( ValidateType type )
{
  for ( int i = 0; i < mTestTable->rowCount(); ++i )
  {
    QString testName = mTestTable->item( i, 0 )->text();
    QString toleranceStr = mTestTable->item( i, 3 )->text();
    QString layer1Str = mTestTable->item( i, 4 )->text();
    QString layer2Str = mTestTable->item( i, 5 )->text();

    // test if layer1 is in the registry
    if ( !(( QgsVectorLayer* )mLayerRegistry->mapLayers().contains( layer1Str ) ) )
    {
      QgsMessageLog::logMessage( tr( "Layer %1 not found in registry." ).arg( layer1Str ), tr( "Topology plugin" ) );
      return;
    }

    QgsVectorLayer* layer1 = ( QgsVectorLayer* )mLayerRegistry->mapLayers()[layer1Str];
    QgsVectorLayer* layer2 = 0;

    if (( QgsVectorLayer* )mLayerRegistry->mapLayers().contains( layer2Str ) )
      layer2 = ( QgsVectorLayer* )mLayerRegistry->mapLayers()[layer2Str];

    QProgressDialog progress( testName, tr( "Abort" ), 0, layer1->featureCount(), this );
    progress.setWindowModality( Qt::WindowModal );

    connect( &progress, SIGNAL( canceled() ), mTest, SLOT( setTestCancelled() ) );
    connect( mTest, SIGNAL( progress( int ) ), &progress, SLOT( setValue( int ) ) );
    // run the test

    ErrorList errors = mTest->runTest( testName, layer1, layer2, type, toleranceStr.toDouble() );

    QList<TopolError*>::Iterator it;

    QgsRubberBand* rb = 0;
    for ( it = errors.begin(); it != errors.end(); ++it )
    {
      TopolError* te = *it;
      te->conflict();

      QSettings settings;
      if ( te->conflict()->type() == QGis::Polygon )
      {
        rb = new QgsRubberBand( qgsInterface->mapCanvas(), true );
      }
      else
      {
        rb = new QgsRubberBand( qgsInterface->mapCanvas(), te->conflict()->type() );
      }
      rb->setColor( "red" );
      rb->setWidth( 4 );
      rb->setToGeometry( te->conflict(), layer1 );
      rb->show();
      mRbErrorMarkers << rb;
    }
    disconnect( &progress, SIGNAL( canceled() ), mTest, SLOT( setTestCancelled() ) );
    disconnect( mTest, SIGNAL( progress( int ) ), &progress, SLOT( setValue( int ) ) );
    mErrorList << errors;
  }
  mToggleRubberband->setChecked( true );
  mErrorListModel->resetModel();
}

void checkDock::validate( ValidateType type )
{
  mErrorList.clear();

  qDeleteAll( mRbErrorMarkers );
  mRbErrorMarkers.clear();

  runTests( type );
  mComment->setText( tr( "%1 errors were found" ).arg( mErrorList.count() ) );

  mRBFeature1->reset();
  mRBFeature2->reset();
  mRBConflict->reset();
  clearVertexMarkers();

  mErrorTableView->resizeColumnsToContents();
  mToggleRubberband->setChecked( true );
}

void checkDock::validateExtent()
{
  validate( ValidateExtent );
}

void checkDock::validateAll()
{
  validate( ValidateAll );
}

void checkDock::validateSelected()
{
  validate( ValidateSelected );
}

void checkDock::toggleErrorMarker()
{
  QList<QgsRubberBand*>::const_iterator it;
  for ( it = mRbErrorMarkers.begin(); it != mRbErrorMarkers.end(); ++it )
  {
    QgsRubberBand* rb = *it;
    if ( mToggleRubberband->isChecked() )
    {
      rb->show();
    }
    else
    {
      rb->hide();
    }
  }
}
