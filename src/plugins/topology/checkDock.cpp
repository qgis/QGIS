/***************************************************************************
  checkDock.cpp
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : (C) 2009 by Vita Cizek
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

#include "qgsfeatureiterator.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsgeometry.h"
#include "qgsvertexmarker.h"
#include "qgsfeature.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgsspatialindex.h"
#include "qgisinterface.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"

#include "topolTest.h"
#include "rulesDialog.h"
#include "dockModel.h"

//class QgisInterface;

checkDock::checkDock( QgisInterface *qIface, QWidget *parent )
  : QgsDockWidget( parent ), Ui::checkDock()
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

  mConfigureDialog = new rulesDialog( mTest->testMap(), qIface, parent );
  mTestTable = mConfigureDialog->rulesTable();

  QgsMapCanvas *canvas = qIface->mapCanvas();// mQgisApp->mapCanvas();
  mRBFeature1.reset( new QgsRubberBand( canvas ) );
  mRBFeature2.reset( new QgsRubberBand( canvas ) );
  mRBConflict.reset( new QgsRubberBand( canvas ) );

  mRBFeature1->setColor( QColor( 0, 0, 255, 65 ) );
  mRBFeature2->setColor( QColor( 0, 255, 0, 65 ) );
  mRBConflict->setColor( QColor( 255, 0, 0, 65 ) );

  mRBFeature1->setWidth( 5 );
  mRBFeature2->setWidth( 5 );
  mRBConflict->setWidth( 5 );

  mVMConflict = nullptr;
  mVMFeature1 = nullptr;
  mVMFeature2 = nullptr;

  connect( actionConfigure, &QAction::triggered, this, &checkDock::configure );
  connect( actionValidateAll, &QAction::triggered, this, &checkDock::validateAll );
  //connect( mValidateSelectedButton, SIGNAL( clicked() ), this, SLOT( validateSelected() ) );
  connect( actionValidateExtent, &QAction::triggered, this, &checkDock::validateExtent );
  connect( mToggleRubberband, &QAbstractButton::clicked, this, &checkDock::toggleErrorMarker );

  connect( mFixButton, &QAbstractButton::clicked, this, &checkDock::fix );
  connect( mErrorTableView, &QAbstractItemView::clicked, this, &checkDock::errorListClicked );

  connect( QgsProject::instance(), static_cast < void ( QgsProject::* )( const QString & ) >( &QgsProject::layerWillBeRemoved ), this, &checkDock::parseErrorListByLayer );

  connect( this, &QDockWidget::visibilityChanged, this, &checkDock::updateRubberBands );
  connect( qgsInterface, &QgisInterface::newProjectCreated, mConfigureDialog, &rulesDialog::clearRules );
  connect( qgsInterface, &QgisInterface::newProjectCreated, this, &checkDock::deleteErrors );

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
    mVMConflict = nullptr;
  }
  if ( mVMFeature1 )
  {
    delete mVMFeature1;
    mVMFeature1 = nullptr;
  }
  if ( mVMFeature2 )
  {
    delete mVMFeature2;
    mVMFeature2 = nullptr;
  }
}

void checkDock::updateRubberBands( bool visible )
{
  if ( !visible )
  {
    if ( mRBConflict )
      mRBConflict->reset();
    if ( mRBFeature1 )
      mRBFeature1->reset();
    if ( mRBFeature2 )
      mRBFeature2->reset();

    clearVertexMarkers();
  }
}

void checkDock::deleteErrors()
{
  qDeleteAll( mErrorList );

  mErrorList.clear();
  mErrorListModel->resetModel();

  qDeleteAll( mRbErrorMarkers );
  mRbErrorMarkers.clear();
}

void checkDock::parseErrorListByLayer( const QString &layerId )
{
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  QList<TopolError *>::Iterator it = mErrorList.begin();

  while ( it != mErrorList.end() )
  {
    const FeatureLayer fl1 = ( *it )->featurePairs().first();
    const FeatureLayer fl2 = ( *it )->featurePairs()[1];
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
  QList<TopolError *>::Iterator it = mErrorList.begin();

  while ( it != mErrorList.end() )
  {
    const FeatureLayer fl1 = ( *it )->featurePairs().first();
    const FeatureLayer fl2 = ( *it )->featurePairs()[1];
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

void checkDock::errorListClicked( const QModelIndex &index )
{
  const int row = index.row();
  QgsRectangle r = mErrorList.at( row )->boundingBox();
  r.scale( 1.5 );
  QgsMapCanvas *canvas = qgsInterface->mapCanvas();
  canvas->setExtent( r, true );
  canvas->refresh();

  mFixBox->clear();
  mFixBox->addItems( mErrorList.at( row )->fixNames() );
  mFixBox->setCurrentIndex( mFixBox->findText( tr( "Select automatic fix" ) ) );

  QgsFeature f;
  QgsGeometry g;
  FeatureLayer fl = mErrorList.at( row )->featurePairs().first();
  if ( !fl.layer )
  {
    QgsMessageLog::logMessage( tr( "Invalid first layer" ), tr( "Topology plugin" ) );
    return;
  }

  //fl1.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl1.feature.id() ) ).nextFeature( f1 );

  fl.layer->getFeatures( QgsFeatureRequest().setFilterFid( fl.feature.id() ) ).nextFeature( f );
  g = f.geometry();
  if ( g.isNull() )
  {
    QgsMessageLog::logMessage( tr( "Invalid first geometry" ), tr( "Topology plugin" ) );
    QMessageBox::information( this, tr( "Topology test" ), tr( "Feature not found in the layer.\nThe layer has probably changed.\nRun topology check again." ) );
    return;
  }

  clearVertexMarkers();

  // use vertex marker when highlighting a point
  // and rubber band otherwise
  if ( g.type() == QgsWkbTypes::PointGeometry )
  {
    mVMFeature1 = new QgsVertexMarker( canvas );
    mVMFeature1->setIconType( QgsVertexMarker::ICON_X );
    mVMFeature1->setPenWidth( 5 );
    mVMFeature1->setIconSize( 5 );
    mVMFeature1->setColor( "blue" );
    mVMFeature1->setCenter( g.asPoint() );
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
  if ( g.isNull() )
  {
    QgsMessageLog::logMessage( tr( "Invalid second geometry" ), tr( "Topology plugin" ) );
    QMessageBox::information( this, tr( "Topology test" ), tr( "Feature not found in the layer.\nThe layer has probably changed.\nRun topology check again." ) );
    return;
  }

  if ( g.type() == QgsWkbTypes::PointGeometry )
  {
    mVMFeature2 = new QgsVertexMarker( canvas );
    mVMFeature2->setIconType( QgsVertexMarker::ICON_BOX );
    mVMFeature2->setPenWidth( 5 );
    mVMFeature2->setIconSize( 5 );
    mVMFeature2->setColor( "green" );
    mVMFeature2->setCenter( g.asPoint() );
  }
  else
    mRBFeature2->setToGeometry( g, fl.layer );

  if ( mErrorList[row]->conflict().isNull() )
  {
    QgsMessageLog::logMessage( tr( "Invalid conflict" ), tr( "Topology plugin" ) );
    return;
  }

  if ( mErrorList.at( row )->conflict().type() == QgsWkbTypes::PointGeometry )
  {
    mVMConflict = new QgsVertexMarker( canvas );
    mVMConflict->setIconType( QgsVertexMarker::ICON_BOX );
    mVMConflict->setPenWidth( 5 );
    mVMConflict->setIconSize( 5 );
    mVMConflict->setColor( "red" );
    mVMConflict->setCenter( mErrorList.at( row )->conflict().asPoint() );
  }
  else
    mRBConflict->setToGeometry( mErrorList.at( row )->conflict(), fl.layer );
}

void checkDock::fix()
{
  const int row = mErrorTableView->currentIndex().row();
  const QString fixName = mFixBox->currentText();

  if ( row == -1 )
    return;

  mRBFeature1->reset();
  mRBFeature2->reset();
  mRBConflict->reset();

  clearVertexMarkers();

  if ( mErrorList.at( row )->fix( fixName ) )
  {
    mErrorList.removeAt( row );
    mErrorListModel->resetModel();
    //parseErrorListByFeature();
    mComment->setText( tr( "%n error(s) were found", nullptr, mErrorList.count() ) );
    qgsInterface->mapCanvas()->refresh();
  }
  else
    QMessageBox::information( this, tr( "Topology fix error" ), tr( "Fixing failed!" ) );
}

void checkDock::runTests( ValidateType type )
{
  mTest->resetCanceledFlag();

  for ( int i = 0; i < mTestTable->rowCount(); ++i )
  {
    if ( mTest->testCanceled() )
      break;

    const QString testName = mTestTable->item( i, 0 )->text();
    const QString layer1Str = mTestTable->item( i, 3 )->text();
    const QString layer2Str = mTestTable->item( i, 4 )->text();

    // test if layer1 is in the registry
    if ( !( ( QgsVectorLayer * )QgsProject::instance()->mapLayers().contains( layer1Str ) ) )
    {
      QgsMessageLog::logMessage( tr( "Layer %1 not found in registry." ).arg( layer1Str ), tr( "Topology plugin" ) );
      return;
    }

    QgsVectorLayer *layer1 = ( QgsVectorLayer * )QgsProject::instance()->mapLayer( layer1Str );
    QgsVectorLayer *layer2 = nullptr;

    if ( ( QgsVectorLayer * )QgsProject::instance()->mapLayers().contains( layer2Str ) )
      layer2 = ( QgsVectorLayer * )QgsProject::instance()->mapLayer( layer2Str );

    QProgressDialog progress( testName, tr( "Abort" ), 0, layer1->featureCount(), this );
    progress.setWindowModality( Qt::WindowModal );

    connect( &progress, &QProgressDialog::canceled, mTest, &topolTest::setTestCanceled );
    connect( mTest, &topolTest::progress, &progress, &QProgressDialog::setValue );
    // run the test

    ErrorList errors = mTest->runTest( testName, layer1, layer2, type );

    QList<TopolError *>::Iterator it;

    QgsRubberBand *rb = nullptr;
    for ( it = errors.begin(); it != errors.end(); ++it )
    {
      TopolError *te = *it;
      te->conflict();

      const QgsSettings settings;
      if ( te->conflict().type() == QgsWkbTypes::PolygonGeometry )
      {
        rb = new QgsRubberBand( qgsInterface->mapCanvas(), QgsWkbTypes::PolygonGeometry );
      }
      else
      {
        rb = new QgsRubberBand( qgsInterface->mapCanvas(), te->conflict().type() );
      }
      rb->setColor( "red" );
      rb->setWidth( 4 );
      rb->setToGeometry( te->conflict(), layer1 );
      rb->show();
      mRbErrorMarkers << rb;
    }
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
  mComment->setText( tr( "%n error(s) were found", nullptr, mErrorList.count() ) );

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
  QList<QgsRubberBand *>::const_iterator it;
  for ( it = mRbErrorMarkers.constBegin(); it != mRbErrorMarkers.constEnd(); ++it )
  {
    QgsRubberBand *rb = *it;
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
