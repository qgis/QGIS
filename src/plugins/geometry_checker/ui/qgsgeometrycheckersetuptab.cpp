/***************************************************************************
 *  qgsgeometrycheckersetuptab.cpp                                         *
 *  -------------------                                                    *
 *  begin                : Jun 10, 2014                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckersetuptab.h"
#include "qgsgeometrycheckerresulttab.h"
#include "../qgsgeometrychecker.h"
#include "../qgsgeometrycheckfactory.h"
#include "../checks/qgsgeometrycheck.h"
#include "../utils/qgsfeaturepool.h"

#include "qgisinterface.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectordataprovider.h"

#include <QAction>
#include <QEventLoop>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QPushButton>
#include <QtConcurrentMap>


QgsGeometryCheckerSetupTab::QgsGeometryCheckerSetupTab( QgisInterface* iface , QWidget *parent )
    : QWidget( parent )
    , mIface( iface )

{
  ui.setupUi( this );
  ui.progressBar->hide();
  ui.labelStatus->hide();
  ui.comboBoxInputLayer->setFilters( QgsMapLayerProxyModel::HasGeometry );
  mRunButton = ui.buttonBox->addButton( tr( "Run" ), QDialogButtonBox::ActionRole );
  mAbortButton = new QPushButton( tr( "Abort" ) );
  mRunButton->setEnabled( false );

  connect( mRunButton, SIGNAL( clicked() ), this, SLOT( runChecks() ) );
  connect( ui.comboBoxInputLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( validateInput() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( updateLayers() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( updateLayers() ) );
  connect( ui.radioButtonOutputNew, SIGNAL( toggled( bool ) ), ui.lineEditOutput, SLOT( setEnabled( bool ) ) );
  connect( ui.radioButtonOutputNew, SIGNAL( toggled( bool ) ), ui.pushButtonOutputBrowse, SLOT( setEnabled( bool ) ) );
  connect( ui.buttonGroupOutput, SIGNAL( buttonClicked( int ) ), this, SLOT( validateInput() ) );
  connect( ui.pushButtonOutputBrowse, SIGNAL( clicked() ), this, SLOT( selectOutputFile() ) );
  connect( ui.lineEditOutput, SIGNAL( textChanged( QString ) ), this, SLOT( validateInput() ) );
  connect( ui.checkBoxSliverPolygons, SIGNAL( toggled( bool ) ), ui.widgetSliverThreshold, SLOT( setEnabled( bool ) ) );
  connect( ui.checkBoxSliverArea, SIGNAL( toggled( bool ) ), ui.doubleSpinBoxSliverArea, SLOT( setEnabled( bool ) ) );

  updateLayers();

  Q_FOREACH ( const QgsGeometryCheckFactory* factory, QgsGeometryCheckFactoryRegistry::getCheckFactories() )
  {
    factory->restorePrevious( ui );
  }
}

QgsGeometryCheckerSetupTab::~QgsGeometryCheckerSetupTab()
{
  delete mAbortButton;
}

void QgsGeometryCheckerSetupTab::updateLayers()
{
  QString prevLayer = ui.comboBoxInputLayer->currentText();
  ui.comboBoxInputLayer->clear();

  // Collect layers
  // Don't switch current layer if dialog is visible to avoid confusing the user
  QgsMapLayer* currentLayer = isVisible() ? 0 : mIface->mapCanvas()->currentLayer();
  int currIdx = -1;
  int idx = 0;
  Q_FOREACH ( QgsMapLayer* layer, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    QgsDebugMsg( QString( "Adding layer, have %1 in list" ).arg( ui.comboBoxInputLayer->count() ) );
    if ( qobject_cast<QgsVectorLayer*>( layer ) )
    {
      ui.comboBoxInputLayer->addItem( layer->name(), layer->id() );
      if ( layer->name() == prevLayer )
      {
        currIdx = idx;
      }
      else if ( currIdx == -1 && layer == currentLayer )
      {
        currIdx = idx;
      }
      ++idx;
    }
  }
  ui.comboBoxInputLayer->setCurrentIndex( qMax( 0, currIdx ) );
}

QgsVectorLayer* QgsGeometryCheckerSetupTab::getSelectedLayer()
{
  int inputIdx = ui.comboBoxInputLayer->currentIndex();
  if ( inputIdx < 0 )
    return nullptr;

  QgsVectorLayer *layer = dynamic_cast<QgsVectorLayer*>( ui.comboBoxInputLayer->currentLayer() );
  return layer;
}

void QgsGeometryCheckerSetupTab::validateInput()
{
  QgsVectorLayer* layer = getSelectedLayer();
  int nApplicable = 0;
  if ( layer )
  {
    Q_FOREACH ( const QgsGeometryCheckFactory* factory, QgsGeometryCheckFactoryRegistry::getCheckFactories() )
    {
      nApplicable += factory->checkApplicability( ui, layer->geometryType() );
    }
  }
  bool outputOk = ui.radioButtonOuputModifyInput->isChecked() || !ui.lineEditOutput->text().isEmpty();
  mRunButton->setEnabled( layer && nApplicable > 0 && outputOk );
}

void QgsGeometryCheckerSetupTab::selectOutputFile()
{
  QString filterString = QgsVectorFileWriter::filterForDriver( "ESRI Shapefile" );
  QMap<QString, QString> filterFormatMap = QgsVectorFileWriter::supportedFiltersAndFormats();
  Q_FOREACH ( const QString& filter, filterFormatMap.keys() )
  {
    QString driverName = filterFormatMap.value( filter );
    if ( driverName != "ESRI Shapefile" ) // Default entry, first in list (see above)
    {
      filterString += ";;" + filter;
    }
  }
  QString initialdir;
  QgsVectorLayer* layer = getSelectedLayer();
  if ( layer )
  {
    QDir dir = QFileInfo( layer->dataProvider()->dataSourceUri() ).dir();
    if ( dir.exists() )
    {
      initialdir = dir.absolutePath();
    }
  }
  QString selectedFilter;
  QString filename = QFileDialog::getSaveFileName( this, tr( "Select Output File" ), initialdir, filterString, &selectedFilter );
  if ( !filename.isEmpty() )
  {
    mOutputDriverName = filterFormatMap.value( selectedFilter );
    QgsVectorFileWriter::MetaData mdata;
    if ( QgsVectorFileWriter::driverMetadata( mOutputDriverName, mdata ) )
    {
      if ( !filename.endsWith( QString( ".%1" ).arg( mdata.ext ), Qt::CaseInsensitive ) )
      {
        filename += QString( ".%1" ).arg( mdata.ext );
      }
    }
    ui.lineEditOutput->setText( filename );
  }
}

void QgsGeometryCheckerSetupTab::runChecks()
{
  /** Get selected layer **/
  QgsVectorLayer* layer = getSelectedLayer();
  if ( !layer )
    return;

  if ( ui.radioButtonOutputNew->isChecked() &&
       layer->dataProvider()->dataSourceUri().startsWith( ui.lineEditOutput->text() ) )
  {
    QMessageBox::critical( this, tr( "Invalid Output Layer" ), tr( "The chosen output layer is the same as the input layer." ) );
    return;
  }

  if ( layer->isEditable() )
  {
    QMessageBox::critical( this, tr( "Editable Input Layer" ), tr( "The input layer is not allowed to be in editing mode." ) );
    return;
  }
  bool selectedOnly = ui.checkBoxInputSelectedOnly->isChecked();

  /** Set window busy **/
  setCursor( Qt::WaitCursor );
  mRunButton->setEnabled( false );
  ui.labelStatus->setText( tr( "<b>Preparing output...</b>" ) );
  ui.labelStatus->show();
  QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );

  /** Duplicate if necessary **/
  if ( ui.radioButtonOutputNew->isChecked() )
  {
    // Write selected feature ids to new layer
    QString filename = ui.lineEditOutput->text();

    // Remove existing layer with same uri
    QStringList toRemove;
    Q_FOREACH ( QgsMapLayer* maplayer, QgsMapLayerRegistry::instance()->mapLayers() )
    {
      if ( dynamic_cast<QgsVectorLayer*>( maplayer ) &&
           static_cast<QgsVectorLayer*>( maplayer )->dataProvider()->dataSourceUri().startsWith( filename ) )
      {
        toRemove.append( maplayer->id() );
      }
    }
    if ( !toRemove.isEmpty() )
    {
      QgsMapLayerRegistry::instance()->removeMapLayers( toRemove );
    }

    QString errMsg;
    QgsVectorFileWriter::WriterError err =  QgsVectorFileWriter::writeAsVectorFormat( layer, filename, layer->dataProvider()->encoding(), &layer->crs(), mOutputDriverName, selectedOnly, &errMsg );
    if ( err != QgsVectorFileWriter::NoError )
    {
      QMessageBox::critical( this, tr( "Layer Creation Failed" ), tr( "Failed to create the output layer: %1" ).arg( errMsg ) );
      mRunButton->setEnabled( true );
      ui.labelStatus->hide();
      unsetCursor();
      return;
    }
    QgsVectorLayer* newlayer = new QgsVectorLayer( filename, QFileInfo( filename ).completeBaseName(), "ogr" );

    if ( selectedOnly )
    {
      QgsFeature feature;

      // Get features to select (only selected features were written up to this point)
      QgsFeatureIds selectedFeatures = newlayer->allFeatureIds();

      // Write non-selected feature ids
      QgsFeatureList features;
      QgsFeatureIterator it = layer->getFeatures();
      while ( it.nextFeature( feature ) )
      {
        if ( !layer->selectedFeaturesIds().contains( feature.id() ) )
        {
          features.append( feature );
        }
      }
      newlayer->dataProvider()->addFeatures( features );

      // Set selected features
      newlayer->selectByIds( selectedFeatures );
    }
    layer = newlayer;
  }
  if (( layer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeGeometries ) == 0 )
  {
    if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Non-editable Output Format" ), tr( "The output file format does not support editing features. The geometry check can be performed, but it will not be possible to fix any errors. Do you want to continue?" ), QMessageBox::Yes, QMessageBox::No ) )
    {
      if ( ui.radioButtonOutputNew->isChecked() )
      {
        QString outputFileName = ui.lineEditOutput->text();
        QFile( outputFileName ).remove();
        if ( mOutputDriverName == "ESRI Shapefile" )
        {
          QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "dbf" ) ).remove();
          QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "prj" ) ).remove();
          QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "qpj" ) ).remove();
          QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "shx" ) ).remove();
        }
      }
      return;
    }
  }

  /** Setup checker **/
  ui.labelStatus->setText( tr( "<b>Building spatial index...</b>" ) );
  QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
  QgsFeaturePool* featurePool = new QgsFeaturePool( layer, selectedOnly );

  QList<QgsGeometryCheck*> checks;
  double mapToLayer = 1. / mIface->mapCanvas()->mapSettings().layerToMapUnits( layer );
  Q_FOREACH ( const QgsGeometryCheckFactory* factory, QgsGeometryCheckFactoryRegistry::getCheckFactories() )
  {
    QgsGeometryCheck* check = factory->createInstance( featurePool, ui, mapToLayer );
    if ( check )
    {
      checks.append( check );
    }
  }
  QgsGeometryCheckPrecision::setPrecision( ui.spinBoxTolerance->value() );
  QgsGeometryChecker* checker = new QgsGeometryChecker( checks, featurePool );

  emit checkerStarted( checker, featurePool );

  /** Add result layer (do this after checkerStarted, otherwise warning about removing of result layer may appear) **/
  layer->setReadOnly( true );
  if ( ui.radioButtonOutputNew->isChecked() )
  {
    QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << layer );
  }

  /** Run **/
  ui.buttonBox->addButton( mAbortButton, QDialogButtonBox::ActionRole );
  mRunButton->hide();
  ui.progressBar->setRange( 0, 0 );
  ui.labelStatus->hide();
  ui.progressBar->show();
  ui.widgetInputs->setEnabled( false );
  QEventLoop evLoop;
  QFutureWatcher<void> futureWatcher;
  connect( checker, SIGNAL( progressValue( int ) ), ui.progressBar, SLOT( setValue( int ) ) );
  connect( &futureWatcher, SIGNAL( finished() ), &evLoop, SLOT( quit() ) );
  connect( mAbortButton, SIGNAL( clicked() ), &futureWatcher, SLOT( cancel() ) );
  connect( mAbortButton, SIGNAL( clicked() ), this, SLOT( showCancelFeedback() ) );

  int maxSteps = 0;
  futureWatcher.setFuture( checker->execute( &maxSteps ) );
  ui.progressBar->setRange( 0, maxSteps );
  evLoop.exec();

  /** Restore window **/
  unsetCursor();
  mAbortButton->setEnabled( true );
  ui.buttonBox->removeButton( mAbortButton );
  mRunButton->setEnabled( true );
  mRunButton->show();
  ui.progressBar->hide();
  ui.labelStatus->hide();
  ui.widgetInputs->setEnabled( true );

  /** Show result **/
  emit checkerFinished( !futureWatcher.isCanceled() );
}

void QgsGeometryCheckerSetupTab::showCancelFeedback()
{
  mAbortButton->setEnabled( false );
  ui.labelStatus->setText( tr( "<b>Waiting for running checks to finish...</b>" ) );
  ui.labelStatus->show();
  ui.progressBar->hide() ;
}
