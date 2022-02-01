/***************************************************************************
                         qgsmapsavedialog.cpp
                         -------------------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QClipboard>
#include <QCheckBox>
#include <QFileDialog>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QPrinter>
#include <QSpinBox>
#include <QUrl>

#include "qgsmapsavedialog.h"
#include "qgsabstractgeopdfexporter.h"
#include "qgsguiutils.h"
#include "qgis.h"
#include "qgisapp.h"
#include "qgsscalecalculator.h"
#include "qgsdecorationitem.h"
#include "qgsexpressioncontext.h"
#include "qgsextentgroupbox.h"
#include "qgsmapsettings.h"
#include "qgsmapsettingsutils.h"
#include "qgsmaprenderertask.h"
#include "qgsmessageviewer.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfileutils.h"
#include "qgsannotationlayer.h"

Q_GUI_EXPORT extern int qt_defaultDpiX();

QgsMapSaveDialog::QgsMapSaveDialog( QWidget *parent, QgsMapCanvas *mapCanvas, const QList<QgsMapDecoration *> &decorations, const QList< QgsAnnotation *> &annotations, DialogType type )
  : QDialog( parent )
  , mDialogType( type )
  , mMapCanvas( mapCanvas )
  , mDecorations( decorations )
  , mAnnotations( annotations )
{
  setupUi( this );

  // Use unrotated visible extent to insure output size and scale matches canvas
  QgsMapSettings ms = mMapCanvas->mapSettings();
  ms.setRotation( 0 );
  mExtent = ms.visibleExtent();
  mDpi = ms.outputDpi();
  mSize = ms.outputSize();

  mResolutionSpinBox->setValue( static_cast< int >( std::round( mDpi ) ) );

  mExtentGroupBox->setOutputCrs( ms.destinationCrs() );
  mExtentGroupBox->setCurrentExtent( mExtent, ms.destinationCrs() );
  mExtentGroupBox->setOutputExtentFromCurrent();
  mExtentGroupBox->setMapCanvas( mapCanvas );

  mScaleWidget->setScale( ms.scale() );
  mScaleWidget->setMapCanvas( mMapCanvas );
  mScaleWidget->setShowCurrentScaleButton( true );

  QString activeDecorations;
  const auto constDecorations = mDecorations;
  for ( QgsMapDecoration *decoration : constDecorations )
  {
    if ( activeDecorations.isEmpty() )
      activeDecorations = decoration->displayName().toLower();
    else
      activeDecorations += QStringLiteral( ", %1" ).arg( decoration->displayName().toLower() );
  }
  mDrawDecorations->setText( tr( "Draw active decorations: %1" ).arg( !activeDecorations.isEmpty() ? activeDecorations : tr( "none" ) ) );

  connect( mResolutionSpinBox, &QSpinBox::editingFinished, this, [ = ] { updateDpi( mResolutionSpinBox->value() ); } );
  connect( mOutputWidthSpinBox, &QSpinBox::editingFinished, this, [ = ] { updateOutputWidth( mOutputWidthSpinBox->value() );} );
  connect( mOutputHeightSpinBox, &QSpinBox::editingFinished, this, [ = ] { updateOutputHeight( mOutputHeightSpinBox->value() );} );
  connect( mExtentGroupBox, &QgsExtentGroupBox::extentChanged, this, &QgsMapSaveDialog::updateExtent );
  connect( mScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsMapSaveDialog::updateScale );
  connect( mLockAspectRatio, &QgsRatioLockButton::lockChanged, this, &QgsMapSaveDialog::lockChanged );

  updateOutputSize();

  switch ( mDialogType )
  {
    case Pdf:
    {
      connect( mInfo, &QLabel::linkActivated, this, [this]( const QString & )
      {
        QgsMessageViewer *viewer = new QgsMessageViewer( this );
        viewer->setWindowTitle( tr( "Advanced effects warning" ) );
        viewer->setMessageAsPlainText( mInfoDetails );
        viewer->exec();
      } );

      this->setWindowTitle( tr( "Save Map as PDF" ) );

      mTextRenderFormatComboBox->addItem( tr( "Always Export Text as Paths (Recommended)" ), static_cast< int>( Qgis::TextRenderFormat::AlwaysOutlines ) );
      mTextRenderFormatComboBox->addItem( tr( "Always Export Text as Text Objects" ), static_cast< int >( Qgis::TextRenderFormat::AlwaysText ) );

      const bool geoPdfAvailable = QgsAbstractGeoPdfExporter::geoPDFCreationAvailable();
      mGeoPDFGroupBox->setEnabled( geoPdfAvailable );
      mGeoPDFGroupBox->setChecked( false );
      if ( !geoPdfAvailable )
      {
        mGeoPDFOptionsStackedWidget->setCurrentIndex( 0 );
        mGeoPdfUnavailableReason->setText( QgsAbstractGeoPdfExporter::geoPDFAvailabilityExplanation() );
        // avoid showing reason in disabled text color - we want it to stand out
        QPalette p = mGeoPdfUnavailableReason->palette();
        p.setColor( QPalette::Disabled, QPalette::WindowText, QPalette::WindowText );
        mGeoPdfUnavailableReason->setPalette( p );
        mGeoPDFOptionsStackedWidget->removeWidget( mGeoPDFOptionsStackedWidget->widget( 1 ) );
      }
      else
      {
        mGeoPDFOptionsStackedWidget->setCurrentIndex( 1 );
        mGeoPdfFormatComboBox->addItem( tr( "ISO 32000 Extension (recommended)" ) );
        mGeoPdfFormatComboBox->addItem( tr( "OGC Best Practice" ) );
      }

      connect( mGeoPDFGroupBox, &QGroupBox::toggled, this, &QgsMapSaveDialog::updatePdfExportWarning );
      updatePdfExportWarning();
      break;
    }

    case Image:
    {
      mExportMetadataCheckBox->hide();
      mGeoPDFGroupBox->hide();
      mAdvancedPdfSettings->hide();
      mTextExportLabel->hide();
      QPushButton *button = new QPushButton( tr( "Copy to Clipboard" ) );
      buttonBox->addButton( button, QDialogButtonBox::ResetRole );
      connect( button, &QPushButton::clicked, this, &QgsMapSaveDialog::copyToClipboard );
      break;
    }
  }

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsMapSaveDialog::onAccepted );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMapSaveDialog::showHelp );
}

void QgsMapSaveDialog::updateDpi( int dpi )
{
  mSize *= static_cast<double>( dpi ) / mDpi;
  mDpi = dpi;

  updateOutputSize();
}

void QgsMapSaveDialog::updateOutputWidth( int width )
{
  const double scale = static_cast<double>( width ) / mSize.width();
  const double adjustment = ( ( mExtent.width() * scale ) - mExtent.width() ) / 2;

  mSize.setWidth( width );

  mExtent.setXMinimum( mExtent.xMinimum() - adjustment );
  mExtent.setXMaximum( mExtent.xMaximum() + adjustment );

  if ( mLockAspectRatio->locked() )
  {
    const int height = width * mExtentGroupBox->ratio().height() / mExtentGroupBox->ratio().width();
    const double scale = static_cast<double>( height ) / mSize.height();
    const double adjustment = ( ( mExtent.height() * scale ) - mExtent.height() ) / 2;

    whileBlocking( mOutputHeightSpinBox )->setValue( height );
    mSize.setHeight( height );

    mExtent.setYMinimum( mExtent.yMinimum() - adjustment );
    mExtent.setYMaximum( mExtent.yMaximum() + adjustment );
  }

  whileBlocking( mExtentGroupBox )->setOutputExtentFromUser( mExtent, mExtentGroupBox->currentCrs() );
}

void QgsMapSaveDialog::updateOutputHeight( int height )
{
  const double scale = static_cast<double>( height ) / mSize.height();
  const double adjustment = ( ( mExtent.height() * scale ) - mExtent.height() ) / 2;

  mSize.setHeight( height );

  mExtent.setYMinimum( mExtent.yMinimum() - adjustment );
  mExtent.setYMaximum( mExtent.yMaximum() + adjustment );

  if ( mLockAspectRatio->locked() )
  {
    const int width = height * mExtentGroupBox->ratio().width() / mExtentGroupBox->ratio().height();
    const double scale = static_cast<double>( width ) / mSize.width();
    const double adjustment = ( ( mExtent.width() * scale ) - mExtent.width() ) / 2;

    whileBlocking( mOutputWidthSpinBox )->setValue( width );
    mSize.setWidth( width );

    mExtent.setXMinimum( mExtent.xMinimum() - adjustment );
    mExtent.setXMaximum( mExtent.xMaximum() + adjustment );
  }

  whileBlocking( mExtentGroupBox )->setOutputExtentFromUser( mExtent, mExtentGroupBox->currentCrs() );
}

void QgsMapSaveDialog::updateExtent( const QgsRectangle &extent )
{
  int currentDpi = 0;

  // reset scale to properly sync output width and height when extent set using
  // current map view, layer extent, or drawn on canvas buttons
  if ( mExtentGroupBox->extentState() != QgsExtentGroupBox::UserExtent )
  {
    currentDpi = mDpi;

    QgsMapSettings ms = mMapCanvas->mapSettings();
    ms.setRotation( 0 );
    mDpi = static_cast< int>( std::round( ms.outputDpi() ) ) ;
    mSize.setWidth( ms.outputSize().width() * extent.width() / ms.visibleExtent().width() );
    mSize.setHeight( ms.outputSize().height() * extent.height() / ms.visibleExtent().height() );

    whileBlocking( mScaleWidget )->setScale( ms.scale() );

    if ( currentDpi != mDpi )
    {
      updateDpi( currentDpi );
    }
  }
  else
  {
    mSize.setWidth( mSize.width() * extent.width() / mExtent.width() );
    mSize.setHeight( mSize.height() * extent.height() / mExtent.height() );
  }
  updateOutputSize();

  mExtent = extent;
  if ( mLockAspectRatio->locked() )
  {
    mExtentGroupBox->setRatio( QSize( mSize.width(), mSize.height() ) );
  }
}

void QgsMapSaveDialog::updateScale( double scale )
{
  QgsScaleCalculator calculator;
  calculator.setMapUnits( mExtentGroupBox->currentCrs().mapUnits() );
  calculator.setDpi( mDpi );

  const double oldScale = calculator.calculate( mExtent, mSize.width() );
  const double scaleRatio = scale / oldScale;
  mExtent.scale( scaleRatio );
  mExtentGroupBox->setOutputExtentFromUser( mExtent, mExtentGroupBox->currentCrs() );
}

void QgsMapSaveDialog::updateOutputSize()
{
  whileBlocking( mOutputWidthSpinBox )->setValue( mSize.width() );
  whileBlocking( mOutputHeightSpinBox )->setValue( mSize.height() );
}

QgsRectangle QgsMapSaveDialog::extent() const
{
  return mExtentGroupBox->outputExtent();
}

int QgsMapSaveDialog::dpi() const
{
  return mResolutionSpinBox->value();
}

QSize QgsMapSaveDialog::size() const
{
  return mSize;
}

bool QgsMapSaveDialog::drawAnnotations() const
{
  return mDrawAnnotations->isChecked();
}

bool QgsMapSaveDialog::drawDecorations() const
{
  return mDrawDecorations->isChecked();
}

bool QgsMapSaveDialog::saveWorldFile() const
{
  return mSaveWorldFile->isChecked();
}

bool QgsMapSaveDialog::exportMetadata() const
{
  return mExportMetadataCheckBox->isChecked();
}

bool QgsMapSaveDialog::saveAsRaster() const
{
  return mSaveAsRaster->isChecked();
}

void QgsMapSaveDialog::applyMapSettings( QgsMapSettings &mapSettings )
{
  const QgsSettings settings;

  switch ( mDialogType )
  {
    case Pdf:
      mapSettings.setFlag( Qgis::MapSettingsFlag::Antialiasing, true ); // hardcode antialiasing when saving as PDF
      mapSettings.setFlag( Qgis::MapSettingsFlag::HighQualityImageTransforms, true ); // hardcode antialiasing when saving as PDF
      break;

    case Image:
      mapSettings.setFlag( Qgis::MapSettingsFlag::Antialiasing, settings.value( QStringLiteral( "qgis/enable_anti_aliasing" ), true ).toBool() );
      mapSettings.setFlag( Qgis::MapSettingsFlag::HighQualityImageTransforms, settings.value( QStringLiteral( "qgis/enable_anti_aliasing" ), true ).toBool() );
      break;
  }
  mapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true ); // force vector output (no caching of marker images etc.)
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawEditingInfo, false );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawSelection, true );
  mapSettings.setSelectionColor( mMapCanvas->mapSettings().selectionColor() );
  mapSettings.setDestinationCrs( mMapCanvas->mapSettings().destinationCrs() );
  mapSettings.setExtent( extent() );
  mapSettings.setOutputSize( size() );
  mapSettings.setOutputDpi( dpi() );
  mapSettings.setBackgroundColor( mMapCanvas->canvasColor() );
  mapSettings.setRotation( mMapCanvas->rotation() );
  mapSettings.setEllipsoid( QgsProject::instance()->ellipsoid() );

  QList< QgsMapLayer * > layers = mMapCanvas->layers();
  if ( !QgsProject::instance()->mainAnnotationLayer()->isEmpty() )
  {
    layers.insert( 0, QgsProject::instance()->mainAnnotationLayer() );
  }
  mapSettings.setLayers( layers );
  mapSettings.setLabelingEngineSettings( mMapCanvas->mapSettings().labelingEngineSettings() );
  mapSettings.setTransformContext( QgsProject::instance()->transformContext() );
  mapSettings.setPathResolver( QgsProject::instance()->pathResolver() );
  mapSettings.setTemporalRange( mMapCanvas->mapSettings().temporalRange() );
  mapSettings.setIsTemporal( mMapCanvas->mapSettings().isTemporal() );


  //build the expression context
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
                    << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
                    << QgsExpressionContextUtils::mapSettingsScope( mapSettings );

  mapSettings.setExpressionContext( expressionContext );

  mapSettings.setRendererUsage( Qgis::RendererUsage::Export );
}

void QgsMapSaveDialog::lockChanged( const bool locked )
{
  if ( locked )
  {
    mExtentGroupBox->setRatio( QSize( mOutputWidthSpinBox->value(), mOutputHeightSpinBox->value() ) );
  }
  else
  {
    mExtentGroupBox->setRatio( QSize( 0, 0 ) );
  }
}

void QgsMapSaveDialog::copyToClipboard()
{
  QgsMapSettings ms = QgsMapSettings();
  applyMapSettings( ms );

  QPainter *p = nullptr;
  QImage *img = nullptr;

  img = new QImage( ms.outputSize(), QImage::Format_ARGB32 );
  if ( img->isNull() )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Save as image" ), tr( "Could not allocate required memory for image" ) );
    return;
  }

  img->setDotsPerMeterX( 1000 * ms.outputDpi() / 25.4 );
  img->setDotsPerMeterY( 1000 * ms.outputDpi() / 25.4 );

  p = new QPainter( img );

  QgsMapRendererTask *mapRendererTask = new QgsMapRendererTask( ms, p );

  if ( drawAnnotations() )
  {
    mapRendererTask->addAnnotations( mAnnotations );
  }

  if ( drawDecorations() )
  {
    mapRendererTask->addDecorations( mDecorations );
  }

  connect( mapRendererTask, &QgsMapRendererTask::renderingComplete, this, [ = ]
  {
    QApplication::clipboard()->setImage( *img, QClipboard::Clipboard );
    QApplication::restoreOverrideCursor();
    QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as image" ), tr( "Successfully copied map to clipboard" ) );

    delete p;
    delete img;
    setEnabled( true );
  } );
  connect( mapRendererTask, &QgsMapRendererTask::errorOccurred, this, [ = ]( int )
  {
    QApplication::restoreOverrideCursor();
    QgisApp::instance()->messageBar()->pushWarning( tr( "Save as image" ), tr( "Could not copy the map to clipboard" ) );

    delete p;
    delete img;
    setEnabled( true );
  } );

  setEnabled( false );

  QApplication::setOverrideCursor( Qt::WaitCursor );
  QgsApplication::taskManager()->addTask( mapRendererTask );
}

void QgsMapSaveDialog::onAccepted()
{
  switch ( mDialogType )
  {
    case Image:
    {
      const QPair< QString, QString> fileNameAndFilter = QgsGuiUtils::getSaveAsImageName( QgisApp::instance(), tr( "Choose a file name to save the map image as" ) );
      if ( !fileNameAndFilter.first.isEmpty() )
      {
        QgsMapSettings ms = QgsMapSettings();
        applyMapSettings( ms );

        QgsMapRendererTask *mapRendererTask = new QgsMapRendererTask( ms, fileNameAndFilter.first, fileNameAndFilter.second );

        if ( drawAnnotations() )
        {
          mapRendererTask->addAnnotations( mAnnotations );
        }

        if ( drawDecorations() )
        {
          mapRendererTask->addDecorations( mDecorations );
        }

        mapRendererTask->setSaveWorldFile( saveWorldFile() );

        connect( mapRendererTask, &QgsMapRendererTask::renderingComplete, [ = ]
        {
          QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as image" ), tr( "Successfully saved map to <a href=\"%1\">%2</a>" )
              .arg( QUrl::fromLocalFile( fileNameAndFilter.first ).toString(), QDir::toNativeSeparators( fileNameAndFilter.first ) ) );
        } );
        connect( mapRendererTask, &QgsMapRendererTask::errorOccurred, [ = ]( int error )
        {
          switch ( error )
          {
            case QgsMapRendererTask::ImageAllocationFail:
            {
              QgisApp::instance()->messageBar()->pushWarning( tr( "Save as image" ), tr( "Could not allocate required memory for image" ) );
              break;
            }
            case QgsMapRendererTask::ImageSaveFail:
            {
              QgisApp::instance()->messageBar()->pushWarning( tr( "Save as image" ), tr( "Could not save the map to file" ) );
              break;
            }
          }
        } );

        QgsApplication::taskManager()->addTask( mapRendererTask );
      }
      break;
    }

    case Pdf:
    {
      QgsSettings settings;
      const QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSaveAsImageDir" ), QDir::homePath() ).toString();
      QString fileName = QFileDialog::getSaveFileName( QgisApp::instance(), tr( "Save Map As" ), lastUsedDir, tr( "PDF Format" ) + " (*.pdf *.PDF)" );
      if ( !fileName.isEmpty() )
      {
        fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, QStringList() << QStringLiteral( "pdf" ) );

        settings.setValue( QStringLiteral( "UI/lastSaveAsImageDir" ), QFileInfo( fileName ).absolutePath() );

        QgsMapSettings ms = QgsMapSettings();
        applyMapSettings( ms );

        if ( mSimplifyGeometriesCheckbox->isChecked() )
        {
          QgsVectorSimplifyMethod simplifyMethod;
          simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::GeometrySimplification );
          simplifyMethod.setForceLocalOptimization( true );
          // we use SnappedToGridGlobal, because it avoids gaps and slivers between previously adjacent polygons
          simplifyMethod.setSimplifyAlgorithm( QgsVectorSimplifyMethod::SnappedToGridGlobal );
          simplifyMethod.setThreshold( 0.1f ); // (pixels). We are quite conservative here. This could possibly be bumped all the way up to 1. But let's play it safe.
          ms.setSimplifyMethod( simplifyMethod );
        }

        ms.setTextRenderFormat( static_cast< Qgis::TextRenderFormat >( mTextRenderFormatComboBox->currentData().toInt() ) );

        QgsAbstractGeoPdfExporter::ExportDetails geoPdfExportDetails;
        if ( mExportMetadataCheckBox->isChecked() )
        {
          // These details will be used on non-GeoPDF exports is the export metadata checkbox is checked
          geoPdfExportDetails.author = QgsProject::instance()->metadata().author();
          geoPdfExportDetails.producer = QStringLiteral( "QGIS %1" ).arg( Qgis::version() );
          geoPdfExportDetails.creator = QStringLiteral( "QGIS %1" ).arg( Qgis::version() );
          geoPdfExportDetails.creationDateTime = QDateTime::currentDateTime();
          geoPdfExportDetails.subject = QgsProject::instance()->metadata().abstract();
          geoPdfExportDetails.title = QgsProject::instance()->metadata().title();
          geoPdfExportDetails.keywords = QgsProject::instance()->metadata().keywords();
        }

        if ( mGeoPDFGroupBox->isChecked() )
        {
          if ( mGeoPdfFormatComboBox->currentIndex() == 0 )
          {
            geoPdfExportDetails.useIso32000ExtensionFormatGeoreferencing = true;
            geoPdfExportDetails.useOgcBestPracticeFormatGeoreferencing = false;
          }
          else
          {
            geoPdfExportDetails.useIso32000ExtensionFormatGeoreferencing = false;
            geoPdfExportDetails.useOgcBestPracticeFormatGeoreferencing = true;
          }

          geoPdfExportDetails.includeFeatures = mExportGeoPdfFeaturesCheckBox->isChecked();
        }
        QgsMapRendererTask *mapRendererTask = new QgsMapRendererTask( ms, fileName, QStringLiteral( "PDF" ), saveAsRaster(), mGeoPDFGroupBox->isChecked(), geoPdfExportDetails );

        if ( drawAnnotations() )
        {
          mapRendererTask->addAnnotations( mAnnotations );
        }

        if ( drawDecorations() )
        {
          mapRendererTask->addDecorations( mDecorations );
        }

        mapRendererTask->setSaveWorldFile( saveWorldFile() );

        if ( exportMetadata() )
        {
          mapRendererTask->setExportMetadata( exportMetadata() );
        }

        connect( mapRendererTask, &QgsMapRendererTask::renderingComplete, [ = ]
        {
          QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as PDF" ), tr( "Successfully saved map to <a href=\"%1\">%2</a>" )
              .arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
        } );
        connect( mapRendererTask, &QgsMapRendererTask::errorOccurred, [ = ]( int )
        {
          QgisApp::instance()->messageBar()->pushWarning( tr( "Save as PDF" ), tr( "Could not save the map to PDF" ) );
        } );

        QgsApplication::taskManager()->addTask( mapRendererTask );
      }
      break;
    }
  }
}

void QgsMapSaveDialog::updatePdfExportWarning()
{
  const QStringList layers = QgsMapSettingsUtils::containsAdvancedEffects( mMapCanvas->mapSettings(), mGeoPDFGroupBox->isChecked() ? QgsMapSettingsUtils::EffectsCheckFlags( QgsMapSettingsUtils::EffectsCheckFlag::IgnoreGeoPdfSupportedEffects ) : QgsMapSettingsUtils::EffectsCheckFlags() );
  if ( !layers.isEmpty() )
  {
    mInfoDetails = tr( "The following layer(s) use advanced effects:\n\n%1\n\nRasterizing map is recommended for proper rendering." ).arg(
                     QChar( 0x2022 ) + QStringLiteral( " " ) + layers.join( QStringLiteral( "\n" ) + QChar( 0x2022 ) + QStringLiteral( " " ) ) );
    mInfo->setText( tr( "%1A number of layers%2 use advanced effects, rasterizing map is recommended for proper rendering." ).arg( QStringLiteral( "<a href='#'>" ), QStringLiteral( "</a>" ) ) );
    mSaveAsRaster->setChecked( true );
  }
  else
  {
    mSaveAsRaster->setChecked( false );
    mInfo->clear();
  }
}

void QgsMapSaveDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/qgis_gui.html#exporting-the-map-view" ) );
}
