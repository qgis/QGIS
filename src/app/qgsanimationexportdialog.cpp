/***************************************************************************
                         qgsanimationexportdialog.cpp
                         -------------------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsanimationexportdialog.h"
#include "qgsmapcanvas.h"
#include "qgsexpressioncontextutils.h"
#include "qgshelp.h"
#include "qgstemporalnavigationobject.h"
#include "qgsprojecttimesettings.h"
#include "qgstemporalutils.h"
#include "qgsmapdecoration.h"

#include <QRegularExpression>
#include <QRegularExpressionValidator>

Q_GUI_EXPORT extern int qt_defaultDpiX();

QgsAnimationExportDialog::QgsAnimationExportDialog( QWidget *parent, QgsMapCanvas *mapCanvas, const QList< QgsMapDecoration * > &decorations )
  : QDialog( parent )
  , mMapCanvas( mapCanvas )
{
  setupUi( this );

  mStartDateTime->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mStartDateTime->maximumDateTime() );
  mEndDateTime->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mStartDateTime->maximumDateTime() );

  // Use unrotated visible extent to insure output size and scale matches canvas
  QgsMapSettings ms = mMapCanvas->mapSettings();
  ms.setRotation( 0 );
  mExtent = ms.visibleExtent();
  mSize = ms.outputSize();

  mExtentGroupBox->setOutputCrs( ms.destinationCrs() );
  mExtentGroupBox->setCurrentExtent( mExtent, ms.destinationCrs() );
  mExtentGroupBox->setOutputExtentFromCurrent();
  mExtentGroupBox->setMapCanvas( mapCanvas );

  mStartDateTime->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mEndDateTime->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  QString activeDecorations;
  const auto constDecorations = decorations;
  for ( QgsMapDecoration *decoration : constDecorations )
  {
    if ( activeDecorations.isEmpty() )
      activeDecorations = decoration->displayName().toLower();
    else
      activeDecorations += QStringLiteral( ", %1" ).arg( decoration->displayName().toLower() );
  }
  mDrawDecorations->setText( tr( "Draw active decorations: %1" ).arg( !activeDecorations.isEmpty() ? activeDecorations : tr( "none" ) ) );

  const QgsSettings settings;

  const QString templateText = settings.value( QStringLiteral( "ExportAnimation/fileNameTemplate" ),
                               QStringLiteral( "%1####.png" ).arg( QgsProject::instance()->baseName() )
                               , QgsSettings::App ).toString();
  mTemplateLineEdit->setText( templateText );
  const QRegularExpression rx( QStringLiteral( "^\\w+#+\\.{1}\\w+$" ) ); //e.g. anyprefix#####.png
  QValidator *validator = new QRegularExpressionValidator( rx, this );
  mTemplateLineEdit->setValidator( validator );

  connect( mTemplateLineEdit, &QLineEdit::textChanged, this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "ExportAnimation/fileNameTemplate" ), mTemplateLineEdit->text() );
  } );

  mOutputDirFileWidget->setStorageMode( QgsFileWidget::GetDirectory );
  mOutputDirFileWidget->setDialogTitle( tr( "Select Directory for Animation Frames" ) );
  mOutputDirFileWidget->lineEdit()->setShowClearButton( false );
  mOutputDirFileWidget->setDefaultRoot( settings.value( QStringLiteral( "ExportAnimation/lastDir" ), QString(), QgsSettings::App ).toString() );
  mOutputDirFileWidget->setFilePath( settings.value( QStringLiteral( "ExportAnimation/lastDir" ), QString(), QgsSettings::App ).toString() );

  connect( mOutputDirFileWidget, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "ExportAnimation/lastDir" ), mOutputDirFileWidget->filePath(), QgsSettings::App );
  } );

  for ( const QgsUnitTypes::TemporalUnit u :
        {
          QgsUnitTypes::TemporalMilliseconds,
          QgsUnitTypes::TemporalSeconds,
          QgsUnitTypes::TemporalMinutes,
          QgsUnitTypes::TemporalHours,
          QgsUnitTypes::TemporalDays,
          QgsUnitTypes::TemporalWeeks,
          QgsUnitTypes::TemporalMonths,
          QgsUnitTypes::TemporalYears,
          QgsUnitTypes::TemporalDecades,
          QgsUnitTypes::TemporalCenturies
        } )
  {
    mTimeStepsComboBox->addItem( QgsUnitTypes::toString( u ), u );
  }

  if ( const QgsTemporalNavigationObject *controller = qobject_cast< const QgsTemporalNavigationObject * >( mMapCanvas->temporalController() ) )
  {
    mStartDateTime->setDateTime( controller->temporalExtents().begin() );
    mEndDateTime->setDateTime( controller->temporalExtents().end() );
  }
  mFrameDurationSpinBox->setClearValue( 1 );
  mFrameDurationSpinBox->setValue( QgsProject::instance()->timeSettings()->timeStep() );
  mTimeStepsComboBox->setCurrentIndex( QgsProject::instance()->timeSettings()->timeStepUnit() );

  connect( mOutputWidthSpinBox, &QSpinBox::editingFinished, this, [ = ] { updateOutputWidth( mOutputWidthSpinBox->value() );} );
  connect( mOutputHeightSpinBox, &QSpinBox::editingFinished, this, [ = ] { updateOutputHeight( mOutputHeightSpinBox->value() );} );
  connect( mExtentGroupBox, &QgsExtentGroupBox::extentChanged, this, &QgsAnimationExportDialog::updateExtent );
  connect( mLockAspectRatio, &QgsRatioLockButton::lockChanged, this, &QgsAnimationExportDialog::lockChanged );

  connect( mSetToProjectTimeButton, &QPushButton::clicked, this, &QgsAnimationExportDialog::setToProjectTime );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "introduction/qgis_gui.html#time-based-control-on-the-map-canvas" ) );
  } );

  connect( buttonBox, &QDialogButtonBox::accepted, this, [ = ]
  {
    emit startExport();
    accept();
  } );

  updateOutputSize();
}

void QgsAnimationExportDialog::updateOutputWidth( int width )
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

void QgsAnimationExportDialog::updateOutputHeight( int height )
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

void QgsAnimationExportDialog::updateExtent( const QgsRectangle &extent )
{
  // leave width as is, update height
  mSize.setHeight( mSize.width() * extent.height() / extent.width() );
  updateOutputSize();

  mExtent = extent;
  if ( mLockAspectRatio->locked() )
  {
    mExtentGroupBox->setRatio( QSize( mSize.width(), mSize.height() ) );
  }
}

void QgsAnimationExportDialog::updateOutputSize()
{
  whileBlocking( mOutputWidthSpinBox )->setValue( mSize.width() );
  whileBlocking( mOutputHeightSpinBox )->setValue( mSize.height() );
}

QgsRectangle QgsAnimationExportDialog::extent() const
{
  return mExtentGroupBox->outputExtent();
}

QSize QgsAnimationExportDialog::size() const
{
  return mSize;
}

QString QgsAnimationExportDialog::outputDirectory() const
{
  return mOutputDirFileWidget->filePath();
}

QString QgsAnimationExportDialog::fileNameExpression() const
{
  return mTemplateLineEdit->text();
}

QgsDateTimeRange QgsAnimationExportDialog::animationRange() const
{
  return QgsDateTimeRange( mStartDateTime->dateTime(), mEndDateTime->dateTime() );
}

QgsInterval QgsAnimationExportDialog::frameInterval() const
{
  return QgsInterval( mFrameDurationSpinBox->value(), static_cast< QgsUnitTypes::TemporalUnit>( mTimeStepsComboBox->currentData().toInt() ) );
}

void QgsAnimationExportDialog::applyMapSettings( QgsMapSettings &mapSettings )
{
  const QgsSettings settings;

  mapSettings.setFlag( Qgis::MapSettingsFlag::Antialiasing, settings.value( QStringLiteral( "qgis/enable_anti_aliasing" ), true ).toBool() );
  mapSettings.setFlag( Qgis::MapSettingsFlag::HighQualityImageTransforms, settings.value( QStringLiteral( "qgis/enable_anti_aliasing" ), true ).toBool() );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawEditingInfo, false );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawSelection, false );
  mapSettings.setSelectionColor( mMapCanvas->mapSettings().selectionColor() );
  mapSettings.setDestinationCrs( mMapCanvas->mapSettings().destinationCrs() );
  mapSettings.setExtent( extent() );
  mapSettings.setOutputSize( size() );
  mapSettings.setBackgroundColor( mMapCanvas->canvasColor() );
  mapSettings.setRotation( mMapCanvas->rotation() );
  mapSettings.setEllipsoid( QgsProject::instance()->ellipsoid() );
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

void QgsAnimationExportDialog::setToProjectTime()
{
  QgsDateTimeRange range;

  // by default try taking the project's fixed temporal extent
  if ( QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();

  // if that's not set, calculate the extent from the project's layers
  if ( !range.begin().isValid() || !range.end().isValid() )
  {
    range = QgsTemporalUtils::calculateTemporalRangeForProject( QgsProject::instance() );
  }

  if ( range.begin().isValid() && range.end().isValid() )
  {
    whileBlocking( mStartDateTime )->setDateTime( range.begin() );
    whileBlocking( mEndDateTime )->setDateTime( range.end() );
  }
}

void QgsAnimationExportDialog::lockChanged( const bool locked )
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

bool QgsAnimationExportDialog::drawDecorations() const
{
  return mDrawDecorations->isChecked();
}
