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

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckersetuptab.h"
#include "qgsgeometrycheckerresulttab.h"
#include "qgsgeometrychecker.h"
#include "qgsgeometrycheckfactory.h"
#include "qgsgeometrycheck.h"
#include "qgsfeaturepool.h"
#include "qgsvectordataproviderfeaturepool.h"

#include "qgsfeatureiterator.h"
#include "qgisinterface.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectordataprovider.h"
#include "qgsapplication.h"
#include "qgsiconutils.h"

#include <QAction>
#include <QEventLoop>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QPushButton>
#include <QtConcurrentMap>

static const int LayerIdRole = Qt::UserRole + 1;

QgsGeometryCheckerSetupTab::QgsGeometryCheckerSetupTab( QgisInterface *iface, QDialog *checkerDialog, QWidget *parent )
  : QWidget( parent )
  , mIface( iface )
  , mCheckerDialog( checkerDialog )

{
  ui.setupUi( this );
  ui.progressBar->hide();
  ui.labelStatus->hide();
  mRunButton = ui.buttonBox->addButton( tr( "Run" ), QDialogButtonBox::ActionRole );
  mAbortButton = new QPushButton( tr( "Abort" ) );
  mRunButton->setEnabled( false );

  const auto drivers = QgsVectorFileWriter::ogrDriverList( QgsVectorFileWriter::SortRecommended | QgsVectorFileWriter::SkipNonSpatialFormats );
  for ( const QgsVectorFileWriter::DriverDetails &driver : drivers )
  {
    ui.comboBoxOutputFormat->addItem( driver.longName, driver.driverName );
  }
  ui.listWidgetInputLayers->setIconSize( QSize( 16, 16 ) );

  connect( mRunButton, &QAbstractButton::clicked, this, &QgsGeometryCheckerSetupTab::runChecks );
  connect( ui.listWidgetInputLayers, &QListWidget::itemChanged, this, &QgsGeometryCheckerSetupTab::validateInput );
  connect( QgsProject::instance(), &QgsProject::layersAdded, this, &QgsGeometryCheckerSetupTab::updateLayers );
  connect( QgsProject::instance(), static_cast<void ( QgsProject::* )( const QStringList & )>( &QgsProject::layersRemoved ), this, &QgsGeometryCheckerSetupTab::updateLayers );
  connect( ui.pushButtonSelectAllLayers, &QAbstractButton::clicked, this, &QgsGeometryCheckerSetupTab::selectAllLayers );
  connect( ui.pushButtonDeselectAllLayers, &QAbstractButton::clicked, this, &QgsGeometryCheckerSetupTab::deselectAllLayers );
  connect( ui.radioButtonOutputNew, &QAbstractButton::toggled, ui.frameOutput, &QWidget::setEnabled );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  connect( ui.buttonGroupOutput, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ), this, &QgsGeometryCheckerSetupTab::validateInput );
#else
  connect( ui.buttonGroupOutput, &QButtonGroup::idClicked, this, &QgsGeometryCheckerSetupTab::validateInput );
#endif
  connect( ui.pushButtonOutputDirectory, &QAbstractButton::clicked, this, &QgsGeometryCheckerSetupTab::selectOutputDirectory );
  connect( ui.lineEditOutputDirectory, &QLineEdit::textChanged, this, &QgsGeometryCheckerSetupTab::validateInput );
  connect( ui.checkBoxSliverPolygons, &QAbstractButton::toggled, ui.widgetSliverThreshold, &QWidget::setEnabled );
  connect( ui.checkBoxSliverArea, &QAbstractButton::toggled, ui.doubleSpinBoxSliverArea, &QWidget::setEnabled );
  connect( ui.checkLineLayerIntersection, &QAbstractButton::toggled, ui.comboLineLayerIntersection, &QComboBox::setEnabled );
  connect( ui.checkBoxFollowBoundaries, &QAbstractButton::toggled, ui.comboBoxFollowBoundaries, &QComboBox::setEnabled );

  ui.lineEditFilenamePrefix->setText( QgsSettings().value( "/geometry_checker/previous_values/filename_prefix", tr( "checked_" ) ).toString() );
  ui.spinBoxTolerance->setValue( QgsSettings().value( "/geometry_checker/previous_values/toleranceDigits", 8 ).toInt() );
  if ( QgsSettings().value( "/geometry_checker/previous_values/createNewLayers", true ).toBool() )
  {
    ui.radioButtonOutputNew->setChecked( true );
  }
  else
  {
    ui.radioButtonOutputModifyInput->setChecked( true );
  }

  for ( const QgsGeometryCheckFactory *factory : QgsGeometryCheckFactoryRegistry::getCheckFactories() )
  {
    factory->restorePrevious( ui );
  }
  updateLayers();
}

QgsGeometryCheckerSetupTab::~QgsGeometryCheckerSetupTab()
{
  delete mAbortButton;
}

void QgsGeometryCheckerSetupTab::updateLayers()
{
  QStringList prevCheckedLayers;
  for ( int row = 0, nRows = ui.listWidgetInputLayers->count(); row < nRows; ++row )
  {
    QListWidgetItem *item = ui.listWidgetInputLayers->item( row );
    if ( item->checkState() == Qt::Checked )
    {
      prevCheckedLayers.append( item->data( LayerIdRole ).toString() );
    }
  }
  ui.listWidgetInputLayers->clear();
  ui.comboLineLayerIntersection->clear();
  ui.comboBoxFollowBoundaries->clear();

  // Collect layers
  for ( QgsVectorLayer *layer : QgsProject::instance()->layers<QgsVectorLayer *>() )
  {
    if ( !layer->isValid() || !layer->dataProvider() )
      continue;

    QListWidgetItem *item = new QListWidgetItem( layer->name() );
    bool supportedGeometryType = true;
    if ( layer->geometryType() == QgsWkbTypes::PointGeometry )
    {
      item->setIcon( QgsIconUtils::iconPoint() );
    }
    else if ( layer->geometryType() == QgsWkbTypes::LineGeometry )
    {
      item->setIcon( QgsIconUtils::iconLine() );
      ui.comboLineLayerIntersection->addItem( layer->name(), layer->id() );
    }
    else if ( layer->geometryType() == QgsWkbTypes::PolygonGeometry )
    {
      item->setIcon( QgsIconUtils::iconPolygon() );
      ui.comboLineLayerIntersection->addItem( layer->name(), layer->id() );
      ui.comboBoxFollowBoundaries->addItem( layer->name(), layer->id() );
    }
    else
    {
      supportedGeometryType = false;
    }
    item->setToolTip( layer->dataProvider()->dataSourceUri() );
    item->setData( LayerIdRole, layer->id() );
    if ( supportedGeometryType )
    {
      // Only set item to checked if it previously was
      item->setCheckState( prevCheckedLayers.contains( layer->id() ) ? Qt::Checked : Qt::Unchecked );
    }
    else
    {
      item->setCheckState( Qt::Unchecked );
      item->setFlags( item->flags() & ~( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable ) );
    }
    ui.listWidgetInputLayers->addItem( item );
  }
  validateInput();
}

void QgsGeometryCheckerSetupTab::selectAllLayers()
{
  for ( int row = 0, nRows = ui.listWidgetInputLayers->count(); row < nRows; ++row )
  {
    QListWidgetItem *item = ui.listWidgetInputLayers->item( row );
    if ( item->flags().testFlag( Qt::ItemIsEnabled ) )
    {
      item->setCheckState( Qt::Checked );
    }
  }
}

void QgsGeometryCheckerSetupTab::deselectAllLayers()
{
  for ( int row = 0, nRows = ui.listWidgetInputLayers->count(); row < nRows; ++row )
  {
    QListWidgetItem *item = ui.listWidgetInputLayers->item( row );
    if ( item->flags().testFlag( Qt::ItemIsEnabled ) )
    {
      item->setCheckState( Qt::Unchecked );
    }
  }
}

QList<QgsVectorLayer *> QgsGeometryCheckerSetupTab::getSelectedLayers()
{
  QList<QgsVectorLayer *> layers;
  for ( int row = 0, nRows = ui.listWidgetInputLayers->count(); row < nRows; ++row )
  {
    QListWidgetItem *item = ui.listWidgetInputLayers->item( row );
    if ( item->checkState() == Qt::Checked )
    {
      QString layerId = item->data( LayerIdRole ).toString();
      QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
      if ( layer )
      {
        layers.append( layer );
      }
    }
  }
  return layers;
}

void QgsGeometryCheckerSetupTab::validateInput()
{
  QStringList layerCrs = QStringList() << QgsProject::instance()->crs().authid();
  QList<QgsVectorLayer *> layers = getSelectedLayers();
  int nApplicable = 0;
  int nPoint = 0;
  int nLineString = 0;
  int nPolygon = 0;
  if ( !layers.isEmpty() )
  {
    for ( QgsVectorLayer *layer : layers )
    {
      QgsWkbTypes::GeometryType geomType = layer->geometryType();
      if ( geomType == QgsWkbTypes::PointGeometry )
      {
        ++nPoint;
      }
      else if ( geomType == QgsWkbTypes::LineGeometry )
      {
        ++nLineString;
      }
      else if ( geomType == QgsWkbTypes::PolygonGeometry )
      {
        ++nPolygon;
      }
      layerCrs.append( layer->crs().authid() );
    }
  }
  for ( const QgsGeometryCheckFactory *factory : QgsGeometryCheckFactoryRegistry::getCheckFactories() )
  {
    nApplicable += factory->checkApplicability( ui, nPoint, nLineString, nPolygon );
  }

  bool outputOk = ui.radioButtonOutputModifyInput->isChecked() || !ui.lineEditOutputDirectory->text().isEmpty();
  mRunButton->setEnabled( !layers.isEmpty() && nApplicable > 0 && outputOk );
}

void QgsGeometryCheckerSetupTab::selectOutputDirectory()
{
  QString initialdir = ui.lineEditOutputDirectory->text();
  if ( initialdir.isEmpty() || !QDir( initialdir ).exists() )
  {
    for ( const QgsVectorLayer *layer : getSelectedLayers() )
    {
      QDir dir = QFileInfo( layer->dataProvider()->dataSourceUri() ).dir();
      if ( dir.exists() )
      {
        initialdir = dir.absolutePath();
        break;
      }
    }
  }
  if ( initialdir.isEmpty() || !QDir( initialdir ).exists() )
  {
    initialdir = QDir::homePath();
  }
  QString dir = QFileDialog::getExistingDirectory( this, tr( "Select Output Directory" ), initialdir );
  if ( !dir.isEmpty() )
  {
    ui.lineEditOutputDirectory->setText( dir );
  }
}

void QgsGeometryCheckerSetupTab::runChecks()
{
  QgsSettings().setValue( "/geometry_checker/previous_values/createNewLayers", ui.radioButtonOutputNew->isChecked() );
  QgsSettings().setValue( "/geometry_checker/previous_values/toleranceDigits", ui.spinBoxTolerance->value() );
  // Get selected layer
  const QList<QgsVectorLayer *> layers = getSelectedLayers();
  if ( layers.isEmpty() )
    return;

  if ( ui.radioButtonOutputNew->isChecked() )
  {
    for ( QgsVectorLayer *layer : layers )
    {
      if ( layer->dataProvider()->dataSourceUri().startsWith( ui.lineEditOutputDirectory->text() ) )
      {
        QMessageBox::critical( this, tr( "Check Geometries" ), tr( "The chosen output directory contains one or more input layers." ) );
        return;
      }
    }
  }
  QgsVectorLayer *lineLayerCheckLayer = ui.comboLineLayerIntersection->isEnabled() ? QgsProject::instance()->mapLayer<QgsVectorLayer *>( ui.comboLineLayerIntersection->currentData().toString() ) : nullptr;
  QgsVectorLayer *followBoundaryCheckLayer = ui.comboBoxFollowBoundaries->isEnabled() ? QgsProject::instance()->mapLayer<QgsVectorLayer *>( ui.comboBoxFollowBoundaries->currentData().toString() ) : nullptr;
  if ( layers.contains( lineLayerCheckLayer ) || layers.contains( followBoundaryCheckLayer ) )
  {
    QMessageBox::critical( this, tr( "Check Geometries" ), tr( "The selected input layers cannot contain a layer also selected for a topology check." ) );
    return;
  }

  for ( QgsVectorLayer *layer : layers )
  {
    if ( layer->isEditable() )
    {
      QMessageBox::critical( this, tr( "Check Geometries" ), tr( "Input layer '%1' is not allowed to be in editing mode." ).arg( layer->name() ) );
      return;
    }
  }
  bool selectedOnly = ui.checkBoxInputSelectedOnly->isChecked();

  // Set window busy
  setCursor( Qt::WaitCursor );
  mRunButton->setEnabled( false );
  ui.labelStatus->setText( tr( "<b>Preparing output…</b>" ) );
  ui.labelStatus->show();
  QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );

  QList<QgsVectorLayer *> processLayers;
  if ( ui.radioButtonOutputNew->isChecked() )
  {
    // Get output directory and file extension
    QDir outputDir = QDir( ui.lineEditOutputDirectory->text() );
    QString outputDriverName = ui.comboBoxOutputFormat->currentData().toString();
    QgsVectorFileWriter::MetaData metadata;
    if ( !QgsVectorFileWriter::driverMetadata( outputDriverName, metadata ) )
    {
      QMessageBox::critical( this, tr( "Check Geometries" ), tr( "The specified output format cannot be recognized." ) );
      mRunButton->setEnabled( true );
      ui.labelStatus->hide();
      unsetCursor();
      return;
    }
    QString outputExtension = metadata.ext;

    // List over input layers, check which existing project layers need to be removed and create output layers
    QString filenamePrefix = ui.lineEditFilenamePrefix->text();
    QgsSettings().setValue( "/geometry_checker/previous_values/filename_prefix", filenamePrefix );
    QStringList toRemove;
    QStringList createErrors;
    for ( QgsVectorLayer *layer : layers )
    {
      QString outputPath = outputDir.absoluteFilePath( filenamePrefix + layer->name() + "." + outputExtension );

      // Remove existing layer with same uri from project
      for ( QgsVectorLayer *projectLayer : QgsProject::instance()->layers<QgsVectorLayer *>() )
      {
        if ( projectLayer->dataProvider()->dataSourceUri().startsWith( outputPath ) )
        {
          toRemove.append( projectLayer->id() );
        }
      }

      // Create output layer
      QString errMsg;
      QgsVectorFileWriter::SaveVectorOptions saveOptions;
      saveOptions.fileEncoding = layer->dataProvider()->encoding();
      saveOptions.driverName = outputDriverName;
      saveOptions.onlySelectedFeatures = selectedOnly;
      QgsVectorFileWriter::WriterError err =  QgsVectorFileWriter::writeAsVectorFormatV3( layer, outputPath, layer->transformContext(), saveOptions, &errMsg, nullptr, nullptr );
      if ( err != QgsVectorFileWriter::NoError )
      {
        createErrors.append( errMsg );
        continue;
      }
      const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
      QgsVectorLayer *newlayer = new QgsVectorLayer( outputPath, QFileInfo( outputPath ).completeBaseName(), QStringLiteral( "ogr" ), options );
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
          if ( !layer->selectedFeatureIds().contains( feature.id() ) )
          {
            features.append( feature );
          }
        }
        newlayer->dataProvider()->addFeatures( features );

        // Set selected features
        newlayer->selectByIds( selectedFeatures );
      }
      processLayers.append( newlayer );
    }

    //  Remove layers from project
    if ( !toRemove.isEmpty() )
    {
      QgsProject::instance()->removeMapLayers( toRemove );
    }

    // Error if an output layer could not be created
    if ( !createErrors.isEmpty() )
    {
      QMessageBox::critical( this, tr( "Check Geometries" ), tr( "Failed to create one or more output layers:\n%1" ).arg( createErrors.join( "\n" ) ) );
      mRunButton->setEnabled( true );
      ui.labelStatus->hide();
      unsetCursor();
      return;
    }
  }
  else
  {
    processLayers = layers;
  }

  // Check if output layers are editable
  QList<QgsVectorLayer *> nonEditableLayers;
  for ( QgsVectorLayer *layer : std::as_const( processLayers ) )
  {
    if ( ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeGeometries ) == 0 )
    {
      nonEditableLayers.append( layer );
    }
  }
  if ( !nonEditableLayers.isEmpty() )
  {
    QStringList nonEditableLayerNames;
    for ( QgsVectorLayer *layer : nonEditableLayers )
    {
      nonEditableLayerNames.append( layer->name() );
    }
    if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Check Geometries" ), tr( "The following output layers are in a format that does not support editing features:\n%1\n\nThe geometry check can be performed, but it will not be possible to fix any errors. Do you want to continue?" ).arg( nonEditableLayerNames.join( "\n" ) ), QMessageBox::Yes, QMessageBox::No ) )
    {
      if ( ui.radioButtonOutputNew->isChecked() )
      {
        for ( QgsVectorLayer *layer : std::as_const( processLayers ) )
        {
          QString layerPath = layer->dataProvider()->dataSourceUri();
          delete layer;
          if ( ui.comboBoxOutputFormat->currentText() == QLatin1String( "ESRI Shapefile" ) )
          {
            QgsVectorFileWriter::deleteShapeFile( layerPath );
          }
          else
          {
            QFile( layerPath ).remove();
          }
        }
        mRunButton->setEnabled( true );
        ui.labelStatus->hide();
        unsetCursor();
      }
      return;
    }
  }

  // Setup checker
  ui.labelStatus->setText( tr( "<b>Building spatial index…</b>" ) );
  QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
  QMap<QString, QgsFeaturePool *> featurePools;
  for ( QgsVectorLayer *layer : std::as_const( processLayers ) )
  {
    featurePools.insert( layer->id(), new QgsVectorDataProviderFeaturePool( layer, selectedOnly ) );
  }
  // LineLayerIntersection check is enabled, make sure there is also a feature pool for that layer
  if ( ui.checkLineLayerIntersection->isChecked() && !featurePools.contains( ui.comboLineLayerIntersection->currentData().toString() ) )
  {
    QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( ui.comboLineLayerIntersection->currentData().toString() );
    Q_ASSERT( layer );
    featurePools.insert( layer->id(), new QgsVectorDataProviderFeaturePool( layer, selectedOnly ) );
  }

  QgsGeometryCheckContext *context = new QgsGeometryCheckContext( ui.spinBoxTolerance->value(), QgsProject::instance()->crs(), QgsProject::instance()->transformContext(), QgsProject::instance() );

  QList<QgsGeometryCheck *> checks;
  for ( const QgsGeometryCheckFactory *factory : QgsGeometryCheckFactoryRegistry::getCheckFactories() )
  {
    QgsGeometryCheck *check = factory->createInstance( context, ui );
    if ( check )
    {
      checks.append( check );
    }
  }
  QgsGeometryChecker *checker = new QgsGeometryChecker( checks, context, featurePools );

  emit checkerStarted( checker );

  if ( ui.radioButtonOutputNew->isChecked() )
  {
    QList<QgsMapLayer *> addLayers;
    for ( QgsVectorLayer *layer : std::as_const( processLayers ) )
    {
      addLayers.append( layer );
    }
    QgsProject::instance()->addMapLayers( addLayers );
  }

  // Run
  ui.buttonBox->addButton( mAbortButton, QDialogButtonBox::ActionRole );
  mRunButton->hide();
  ui.progressBar->setRange( 0, 0 );
  ui.labelStatus->hide();
  ui.progressBar->show();
  ui.widgetInputs->setEnabled( false );
  QEventLoop evLoop;
  QFutureWatcher<void> futureWatcher;
  connect( checker, &QgsGeometryChecker::progressValue, ui.progressBar, &QProgressBar::setValue );
  connect( &futureWatcher, &QFutureWatcherBase::finished, &evLoop, &QEventLoop::quit );
  connect( mAbortButton, &QAbstractButton::clicked, &futureWatcher, &QFutureWatcherBase::cancel );
  connect( mAbortButton, &QAbstractButton::clicked, this, &QgsGeometryCheckerSetupTab::showCancelFeedback );

  mIsRunningInBackground = true;

  int maxSteps = 0;
  futureWatcher.setFuture( checker->execute( &maxSteps ) );
  ui.progressBar->setRange( 0, maxSteps );
  evLoop.exec();

  mIsRunningInBackground = false;

  // Restore window
  unsetCursor();
  mAbortButton->setEnabled( true );
  ui.buttonBox->removeButton( mAbortButton );
  mRunButton->setEnabled( true );
  mRunButton->show();
  ui.progressBar->hide();
  ui.labelStatus->hide();
  ui.widgetInputs->setEnabled( true );

  // Show result
  emit checkerFinished( !futureWatcher.isCanceled() );
}

void QgsGeometryCheckerSetupTab::showCancelFeedback()
{
  mAbortButton->setEnabled( false );
  ui.labelStatus->setText( tr( "<b>Waiting for running checks to finish…</b>" ) );
  ui.labelStatus->show();
  ui.progressBar->hide() ;
}
