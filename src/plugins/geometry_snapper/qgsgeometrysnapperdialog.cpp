/***************************************************************************
 *  qgsgeometrysnapperdialog.cpp                                           *
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

#include "qgsgeometrysnapperdialog.h"
#include "qgsgeometrysnapper.h"

#include "qgisinterface.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectordataprovider.h"
#include "qgsspatialindex.h"

#include <QAction>
#include <QEventLoop>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QPushButton>
#include <QtConcurrentMap>


QgsGeometrySnapperDialog::QgsGeometrySnapperDialog( QgisInterface* iface ):
    mIface( iface )
{
  setupUi( this );
  mRunButton = buttonBox->addButton( tr( "Run" ), QDialogButtonBox::ActionRole );
  buttonBox->button( QDialogButtonBox::Abort )->hide();
  mRunButton->setEnabled( false );
  progressBar->hide();
  setFixedSize( sizeHint() );
  setWindowModality( Qt::ApplicationModal );

  connect( mRunButton, SIGNAL( clicked() ), this, SLOT( run() ) );
  connect( comboBoxInputLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( validateInput() ) );
  connect( comboBoxReferenceLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( validateInput() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( updateLayers() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( updateLayers() ) );
  connect( radioButtonOutputNew, SIGNAL( toggled( bool ) ), lineEditOutput, SLOT( setEnabled( bool ) ) );
  connect( radioButtonOutputNew, SIGNAL( toggled( bool ) ), pushButtonOutputBrowse, SLOT( setEnabled( bool ) ) );
  connect( buttonGroupOutput, SIGNAL( buttonClicked( int ) ), this, SLOT( validateInput() ) );
  connect( pushButtonOutputBrowse, SIGNAL( clicked() ), this, SLOT( selectOutputFile() ) );
  connect( lineEditOutput, SIGNAL( textChanged( QString ) ), this, SLOT( validateInput() ) );

  updateLayers();
}

void QgsGeometrySnapperDialog::updateLayers()
{
  QString curInput = comboBoxInputLayer->currentText();
  QString curReference = comboBoxReferenceLayer->currentText();

  comboBoxInputLayer->clear();
  comboBoxReferenceLayer->clear();

  // Collect layers
  // Don't switch current layer if dialog is visible to avoid confusing the user
  QgsMapLayer* currentLayer = isVisible() ? 0 : mIface->mapCanvas()->currentLayer();
  int curInputIdx = -1;
  int curReferenceIdx = -1;
  int idx = 0;
  Q_FOREACH ( QgsMapLayer* layer, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    if ( qobject_cast<QgsVectorLayer*>( layer ) )
    {
      QGis::WkbType type = QGis::flatType( QGis::singleType( static_cast<QgsVectorLayer*>( layer )->wkbType() ) );
      if ( type == QGis::WKBPolygon || type == QGis::WKBLineString )
      {
        comboBoxInputLayer->addItem( layer->name(), layer->id() );
        comboBoxReferenceLayer->addItem( layer->name(), layer->id() );
        if ( layer->name() == curInput )
        {
          curInputIdx = idx;
        }
        else if ( curInputIdx == -1 && layer == currentLayer )
        {
          curInputIdx = idx;
        }

        if ( layer->name() == curReference )
        {
          curReferenceIdx = idx;
        }
        ++idx;
      }
    }
  }
  if ( curInputIdx == -1 )
  {
    curInputIdx = 0;
  }
  if ( curReferenceIdx == -1 )
  {
    curReferenceIdx = curInputIdx + 1 >= comboBoxReferenceLayer->count() ? curInputIdx - 1 : curInputIdx + 1;
  }
  comboBoxInputLayer->setCurrentIndex( curInputIdx );
  comboBoxReferenceLayer->setCurrentIndex( curReferenceIdx );
}

QgsVectorLayer* QgsGeometrySnapperDialog::getInputLayer()
{
  int idx = comboBoxInputLayer->currentIndex();
  if ( idx < 0 )
    return nullptr;

  QString inputLayerId = comboBoxInputLayer->itemData( idx ).toString();
  return static_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( inputLayerId ) );
}

QgsVectorLayer* QgsGeometrySnapperDialog::getReferenceLayer()
{
  int idx = comboBoxReferenceLayer->currentIndex();
  if ( idx < 0 )
    return nullptr;

  QString inputLayerId = comboBoxReferenceLayer->itemData( idx ).toString();
  return static_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( inputLayerId ) );
}

void QgsGeometrySnapperDialog::validateInput()
{
  QgsVectorLayer* inLayer = getInputLayer();
  QgsVectorLayer* refLayer = getReferenceLayer();
  bool outputOk = radioButtonOuputModifyInput->isChecked() || !lineEditOutput->text().isEmpty();
  mRunButton->setEnabled( inLayer && refLayer && inLayer != refLayer &&
                          refLayer->geometryType() == inLayer->geometryType() && outputOk );
}

void QgsGeometrySnapperDialog::selectOutputFile()
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
  QgsVectorLayer* layer = getInputLayer();
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
    lineEditOutput->setText( filename );
  }
}

void QgsGeometrySnapperDialog::run()
{
  /** Get layers **/
  QgsVectorLayer* layer = getInputLayer();
  QgsVectorLayer* referenceLayer = getReferenceLayer();
  if ( !layer || !referenceLayer )
  {
    return;
  }

  if ( radioButtonOutputNew->isChecked() &&
       ( layer->dataProvider()->dataSourceUri().startsWith( lineEditOutput->text() ) ||
         referenceLayer->dataProvider()->dataSourceUri().startsWith( lineEditOutput->text() ) ) )
  {
    QMessageBox::critical( this, tr( "Invalid Output Layer" ), tr( "The chosen output layer is the same as an input layer." ) );
    return;
  }

  bool selectedOnly = checkBoxInputSelectedOnly->isChecked();

  /** Duplicate if necessary **/
  if ( radioButtonOutputNew->isChecked() )
  {
    QString filename = lineEditOutput->text();

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
    QMessageBox::critical( this, tr( "Non-editable Output Format" ), tr( "The output file format does not support editing features. Please select another output file format." ) );
    if ( radioButtonOutputNew->isChecked() )
    {
      QString outputFileName = lineEditOutput->text();
      QFile( outputFileName ).remove();
      if ( mOutputDriverName == "ESRI Shapefile" )
      {
        QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "dbf" ) ).remove();
        QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "prj" ) ).remove();
        QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "qpj" ) ).remove();
        QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "shx" ) ).remove();
      }
      return;
    }
  }

  layer->setReadOnly( true );
  if ( radioButtonOutputNew->isChecked() )
  {
    QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << layer );
  }

  /** Run **/
  QEventLoop evLoop;
  QFutureWatcher<void> futureWatcher;
  connect( &futureWatcher, SIGNAL( finished() ), &evLoop, SLOT( quit() ) );
  connect( buttonBox->button( QDialogButtonBox::Abort ), SIGNAL( clicked() ), &futureWatcher, SLOT( cancel() ) );

  setCursor( Qt::WaitCursor );
  buttonBox->button( QDialogButtonBox::Abort )->show();
  mRunButton->hide();
  progressBar->setRange( 0, 0 );
  progressBar->setValue( 0 );
  progressBar->show();
  widgetInputs->setEnabled( false );

  QgsGeometrySnapper snapper( layer, referenceLayer, selectedOnly, doubleSpinBoxMaxDistance->value(), &mIface->mapCanvas()->mapSettings() );
  connect( &snapper, SIGNAL( progressRangeChanged( int, int ) ), progressBar, SLOT( setRange( int, int ) ) );
  connect( &snapper, SIGNAL( progressStep() ), this, SLOT( progressStep() ) );
  futureWatcher.setFuture( snapper.processFeatures() );
  evLoop.exec();

  /** Restore window **/
  unsetCursor();
  buttonBox->button( QDialogButtonBox::Abort )->hide();
  mRunButton->show();
  progressBar->hide();
  widgetInputs->setEnabled( true );

  layer->setReadOnly( false );

  /** Refresh canvas **/
  mIface->mapCanvas()->refresh();

  /** Show errors **/
  if ( !snapper.getErrors().isEmpty() )
  {
    QMessageBox::warning( this, tr( "Errors occurred" ), tr( "<p>The following errors occurred:</p><ul><li>%1</li></ul>" ).arg( snapper.getErrors().join( "</li><li>" ) ) );
  }
  hide() ;
}

void QgsGeometrySnapperDialog::progressStep()
{
  progressBar->setValue( progressBar->value() + 1 );
}

