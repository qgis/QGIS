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

#include "qgsmapsavedialog.h"
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
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsapplication.h"


Q_GUI_EXPORT extern int qt_defaultDpiX();

QgsMapSaveDialog::QgsMapSaveDialog( QWidget *parent, QgsMapCanvas *mapCanvas, const QList<QgsDecorationItem *> &decorations, const QList< QgsAnnotation *> &annotations, DialogType type )
  : QDialog( parent )
  , mDialogType( type )
  , mMapCanvas( mapCanvas )
  , mAnnotations( annotations )
{
  setupUi( this );

  // Use unrotated visible extent to insure output size and scale matches canvas
  QgsMapSettings ms = mMapCanvas->mapSettings();
  ms.setRotation( 0 );
  mExtent = ms.visibleExtent();
  mDpi = ms.outputDpi();
  mSize = ms.outputSize();

  mResolutionSpinBox->setValue( mDpi );

  mExtentGroupBox->setOutputCrs( ms.destinationCrs() );
  mExtentGroupBox->setCurrentExtent( mExtent, ms.destinationCrs() );
  mExtentGroupBox->setOutputExtentFromCurrent();
  mExtentGroupBox->setMapCanvas( mapCanvas );

  mScaleWidget->setScale( ms.scale() );
  mScaleWidget->setMapCanvas( mMapCanvas );
  mScaleWidget->setShowCurrentScaleButton( true );

  QString activeDecorations;
  Q_FOREACH ( QgsDecorationItem *decoration, decorations )
  {
    mDecorations << decoration;
    if ( activeDecorations.isEmpty() )
      activeDecorations = decoration->name().toLower();
    else
      activeDecorations += QStringLiteral( ", %1" ).arg( decoration->name().toLower() );
  }
  mDrawDecorations->setText( tr( "Draw active decorations: %1" ).arg( !activeDecorations.isEmpty() ? activeDecorations : tr( "none" ) ) );

  connect( mResolutionSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsMapSaveDialog::updateDpi );
  connect( mOutputWidthSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsMapSaveDialog::updateOutputWidth );
  connect( mOutputHeightSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsMapSaveDialog::updateOutputHeight );
  connect( mExtentGroupBox, &QgsExtentGroupBox::extentChanged, this, &QgsMapSaveDialog::updateExtent );
  connect( mScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsMapSaveDialog::updateScale );
  connect( mLockAspectRatio, &QgsRatioLockButton::lockChanged, this, &QgsMapSaveDialog::lockChanged );

  updateOutputSize();

  if ( mDialogType == QgsMapSaveDialog::Pdf )
  {
    mSaveWorldFile->setVisible( false );

    QStringList layers = QgsMapSettingsUtils::containsAdvancedEffects( mMapCanvas->mapSettings() );
    if ( !layers.isEmpty() )
    {
      // Limit number of items to avoid extreme dialog height
      if ( layers.count() >= 10 )
      {
        layers = layers.mid( 0, 9 );
        layers << QChar( 0x2026 );
      }
      mInfo->setText( tr( "The following layer(s) use advanced effects:\n%1\nRasterizing map is recommended for proper rendering." ).arg(
                        QChar( 0x2022 ) + QStringLiteral( " " ) + layers.join( QStringLiteral( "\n" ) + QChar( 0x2022 ) + QStringLiteral( " " ) ) ) );
      mSaveAsRaster->setChecked( true );
    }
    else
    {
      mSaveAsRaster->setChecked( false );
    }
    mSaveAsRaster->setVisible( true );

    this->setWindowTitle( tr( "Save Map as PDF" ) );
  }
  else
  {
    QPushButton *button = new QPushButton( tr( "Copy to Clipboard" ) );
    buttonBox->addButton( button, QDialogButtonBox::ResetRole );
    connect( button, &QPushButton::clicked, this, &QgsMapSaveDialog::copyToClipboard );
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
  double scale = static_cast<double>( width ) / mSize.width();
  double adjustment = ( ( mExtent.width() * scale ) - mExtent.width() ) / 2;

  mSize.setWidth( width );

  mExtent.setXMinimum( mExtent.xMinimum() - adjustment );
  mExtent.setXMaximum( mExtent.xMaximum() + adjustment );

  if ( mLockAspectRatio->locked() )
  {
    int height = width * mExtentGroupBox->ratio().height() / mExtentGroupBox->ratio().width();
    double scale = static_cast<double>( height ) / mSize.height();
    double adjustment = ( ( mExtent.height() * scale ) - mExtent.height() ) / 2;

    whileBlocking( mOutputHeightSpinBox )->setValue( height );
    mSize.setHeight( height );

    mExtent.setYMinimum( mExtent.yMinimum() - adjustment );
    mExtent.setYMaximum( mExtent.yMaximum() + adjustment );
  }

  whileBlocking( mExtentGroupBox )->setOutputExtentFromUser( mExtent, mExtentGroupBox->currentCrs() );
}

void QgsMapSaveDialog::updateOutputHeight( int height )
{
  double scale = static_cast<double>( height ) / mSize.height();
  double adjustment = ( ( mExtent.height() * scale ) - mExtent.height() ) / 2;

  mSize.setHeight( height );

  mExtent.setYMinimum( mExtent.yMinimum() - adjustment );
  mExtent.setYMaximum( mExtent.yMaximum() + adjustment );

  if ( mLockAspectRatio->locked() )
  {
    int width = height * mExtentGroupBox->ratio().width() / mExtentGroupBox->ratio().height();
    double scale = static_cast<double>( width ) / mSize.width();
    double adjustment = ( ( mExtent.width() * scale ) - mExtent.width() ) / 2;

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
    mDpi = ms.outputDpi();
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

  double oldScale = 1 / ( calculator.calculate( mExtent, mSize.width() ) );
  double scaleRatio = scale / oldScale;
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

bool QgsMapSaveDialog::saveAsRaster() const
{
  return mSaveAsRaster->isChecked();
}

void QgsMapSaveDialog::applyMapSettings( QgsMapSettings &mapSettings )
{
  QgsSettings settings;

  if ( mDialogType == QgsMapSaveDialog::Pdf )
  {
    mapSettings.setFlag( QgsMapSettings::Antialiasing, true ); // hardcode antialising when saving as PDF
  }
  else
  {
    mapSettings.setFlag( QgsMapSettings::Antialiasing, settings.value( QStringLiteral( "qgis/enable_anti_aliasing" ), true ).toBool() );
  }
  mapSettings.setFlag( QgsMapSettings::ForceVectorOutput, true ); // force vector output (no caching of marker images etc.)
  mapSettings.setFlag( QgsMapSettings::DrawEditingInfo, false );
  mapSettings.setFlag( QgsMapSettings::DrawSelection, true );
  mapSettings.setSelectionColor( mMapCanvas->mapSettings().selectionColor() );
  mapSettings.setDestinationCrs( mMapCanvas->mapSettings().destinationCrs() );
  mapSettings.setExtent( extent() );
  mapSettings.setOutputSize( size() );
  mapSettings.setOutputDpi( dpi() );
  mapSettings.setBackgroundColor( mMapCanvas->canvasColor() );
  mapSettings.setRotation( mMapCanvas->rotation() );
  mapSettings.setLayers( mMapCanvas->layers() );
  mapSettings.setLabelingEngineSettings( mMapCanvas->mapSettings().labelingEngineSettings() );
  mapSettings.setTransformContext( QgsProject::instance()->transformContext() );
  mapSettings.setPathResolver( QgsProject::instance()->pathResolver() );

  //build the expression context
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
                    << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
                    << QgsExpressionContextUtils::mapSettingsScope( mapSettings );

  mapSettings.setExpressionContext( expressionContext );
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
    QgisApp::instance()->messageBar()->pushWarning( tr( "Save as PDF" ), tr( "Could not copy the map to clipboard" ) );

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
  if ( mDialogType == Image )
  {
    QPair< QString, QString> fileNameAndFilter = QgsGuiUtils::getSaveAsImageName( QgisApp::instance(), tr( "Choose a file name to save the map image as" ) );
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
  }
  else
  {
    QgsSettings settings;
    QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSaveAsImageDir" ), QDir::homePath() ).toString();
    QString fileName = QFileDialog::getSaveFileName( QgisApp::instance(), tr( "Save Map As" ), lastUsedDir, tr( "PDF Format" ) + " (*.pdf *.PDF)" );
    if ( !fileName.isEmpty() )
    {
      settings.setValue( QStringLiteral( "UI/lastSaveAsImageDir" ), QFileInfo( fileName ).absolutePath() );

      QgsMapSettings ms = QgsMapSettings();
      applyMapSettings( ms );

      QgsMapRendererTask *mapRendererTask = new QgsMapRendererTask( ms, fileName, QStringLiteral( "PDF" ), saveAsRaster() );

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
        QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as PDF" ), tr( "Successfully saved map to <a href=\"%1\">%2</a>" )
            .arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
      } );
      connect( mapRendererTask, &QgsMapRendererTask::errorOccurred, [ = ]( int )
      {
        QgisApp::instance()->messageBar()->pushWarning( tr( "Save as PDF" ), tr( "Could not save the map to PDF" ) );
      } );

      QgsApplication::taskManager()->addTask( mapRendererTask );
    }
  }
}

void QgsMapSaveDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/getting_started.html#output" ) );
}
