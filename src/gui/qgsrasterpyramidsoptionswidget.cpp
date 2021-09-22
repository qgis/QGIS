/***************************************************************************
                          qgsrasterpyramidsoptionswidget.cpp
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterpyramidsoptionswidget.h"
#include "qgsrasterdataprovider.h"
#include "qgslogger.h"
#include "qgsdialog.h"
#include "qgssettings.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QCheckBox>
#include <QRegularExpressionValidator>
#include <QRegularExpression>

QgsRasterPyramidsOptionsWidget::QgsRasterPyramidsOptionsWidget( QWidget *parent, const QString &provider )
  : QWidget( parent )
  , mProvider( provider )
{
  setupUi( this );
  connect( cbxPyramidsLevelsCustom, &QCheckBox::toggled, this, &QgsRasterPyramidsOptionsWidget::cbxPyramidsLevelsCustom_toggled );
  connect( cbxPyramidsFormat, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterPyramidsOptionsWidget::cbxPyramidsFormat_currentIndexChanged );

  mSaveOptionsWidget->setProvider( provider );
  mSaveOptionsWidget->setPyramidsFormat( QgsRaster::PyramidsGTiff );
  mSaveOptionsWidget->setType( QgsRasterFormatSaveOptionsWidget::ProfileLineEdit );

  updateUi();
}

void QgsRasterPyramidsOptionsWidget::updateUi()
{
  const QgsSettings mySettings;
  const QString prefix = mProvider + "/driverOptions/_pyramids/";
  QString tmpStr;

  // keep it in sync with qgsrasterlayerproperties.cpp
  tmpStr = mySettings.value( prefix + "format", "external" ).toString();
  if ( tmpStr == QLatin1String( "internal" ) )
    cbxPyramidsFormat->setCurrentIndex( INTERNAL );
  else if ( tmpStr == QLatin1String( "external_erdas" ) )
    cbxPyramidsFormat->setCurrentIndex( ERDAS );
  else
    cbxPyramidsFormat->setCurrentIndex( GTIFF );

  // initialize resampling methods
  cboResamplingMethod->clear();
  const auto methods {QgsRasterDataProvider::pyramidResamplingMethods( mProvider )};
  for ( const QPair<QString, QString> &method : methods )
  {
    cboResamplingMethod->addItem( method.second, method.first );
  }
  const QString defaultMethod = mySettings.value( prefix + "resampling", "AVERAGE" ).toString();
  const int idx = cboResamplingMethod->findData( defaultMethod );
  cboResamplingMethod->setCurrentIndex( idx );

  // validate string, only space-separated positive integers are allowed
  lePyramidsLevels->setEnabled( cbxPyramidsLevelsCustom->isChecked() );
  lePyramidsLevels->setValidator( new QRegularExpressionValidator( QRegularExpression( "(\\d*)(\\s\\d*)*" ), lePyramidsLevels ) );
  connect( lePyramidsLevels, &QLineEdit::textEdited,
           this, &QgsRasterPyramidsOptionsWidget::setOverviewList );

  // overview list
  if ( mOverviewCheckBoxes.isEmpty() )
  {
    QList<int> overviewList;
    overviewList << 2 << 4 << 8 << 16 << 32 << 64;
    mOverviewCheckBoxes.clear();
    const auto constOverviewList = overviewList;
    for ( const int i : constOverviewList )
    {
      mOverviewCheckBoxes[ i ] = new QCheckBox( QString::number( i ), this );
      connect( mOverviewCheckBoxes[ i ], &QCheckBox::toggled,
               this, &QgsRasterPyramidsOptionsWidget::setOverviewList );
      layoutPyramidsLevels->addWidget( mOverviewCheckBoxes[ i ] );
    }
  }
  else
  {
    for ( auto it = mOverviewCheckBoxes.constBegin(); it != mOverviewCheckBoxes.constEnd(); ++it )
      it.value()->setChecked( false );
  }
  tmpStr = mySettings.value( prefix + "overviewList", "" ).toString();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  const QStringList constSplit = tmpStr.split( ' ', QString::SkipEmptyParts );
#else
  const QStringList constSplit = tmpStr.split( ' ', Qt::SkipEmptyParts );
#endif
  for ( const QString &lev : constSplit )
  {
    if ( mOverviewCheckBoxes.contains( lev.toInt() ) )
      mOverviewCheckBoxes[ lev.toInt()]->setChecked( true );
  }
  setOverviewList();

  mSaveOptionsWidget->updateProfiles();

  connect( cbxPyramidsFormat, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsRasterPyramidsOptionsWidget::someValueChanged );
  connect( cboResamplingMethod, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsRasterPyramidsOptionsWidget::someValueChanged );
  connect( mSaveOptionsWidget, &QgsRasterFormatSaveOptionsWidget::optionsChanged,
           this, &QgsRasterPyramidsOptionsWidget::someValueChanged );
}

QString QgsRasterPyramidsOptionsWidget::resamplingMethod() const
{
  return cboResamplingMethod->currentData().toString();
}

void QgsRasterPyramidsOptionsWidget::apply()
{
  QgsSettings mySettings;
  const QString prefix = mProvider + "/driverOptions/_pyramids/";
  QString tmpStr;

  // mySettings.setValue( prefix + "internal", cbxPyramidsInternal->isChecked() );
  if ( cbxPyramidsFormat->currentIndex() == INTERNAL )
    tmpStr = QStringLiteral( "internal" );
  else if ( cbxPyramidsFormat->currentIndex() == ERDAS )
    tmpStr = QStringLiteral( "external_erdas" );
  else
    tmpStr = QStringLiteral( "external" );
  mySettings.setValue( prefix + "format", tmpStr );
  mySettings.setValue( prefix + "resampling", resamplingMethod() );
  mySettings.setValue( prefix + "overviewStr", lePyramidsLevels->text().trimmed() );

  // overview list
  tmpStr.clear();
  for ( auto it = mOverviewCheckBoxes.constBegin(); it != mOverviewCheckBoxes.constEnd(); ++it )
  {
    if ( it.value()->isChecked() )
      tmpStr += QString::number( it.key() ) + ' ';
  }
  mySettings.setValue( prefix + "overviewList", tmpStr.trimmed() );

  mSaveOptionsWidget->apply();
}

void QgsRasterPyramidsOptionsWidget::checkAllLevels( bool checked )
{
  for ( auto it = mOverviewCheckBoxes.constBegin(); it != mOverviewCheckBoxes.constEnd(); ++it )
    it.value()->setChecked( checked );
}

void QgsRasterPyramidsOptionsWidget::cbxPyramidsLevelsCustom_toggled( bool toggled )
{
  // if toggled, disable checkboxes and enable line edit
  lePyramidsLevels->setEnabled( toggled );
  for ( auto it = mOverviewCheckBoxes.constBegin(); it != mOverviewCheckBoxes.constEnd(); ++it )
    it.value()->setEnabled( ! toggled );
  setOverviewList();
}

void QgsRasterPyramidsOptionsWidget::cbxPyramidsFormat_currentIndexChanged( int index )
{
  mSaveOptionsWidget->setEnabled( index != ERDAS );
  QgsRaster::RasterPyramidsFormat format;
  switch ( index )
  {
    case GTIFF:
      format = QgsRaster::PyramidsGTiff;
      break;
    case INTERNAL:
      format = QgsRaster::PyramidsInternal;
      break;
    case ERDAS:
      format = QgsRaster::PyramidsErdas;
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Should not happen !" ) );
      format = QgsRaster::PyramidsGTiff;
      break;
  }
  mSaveOptionsWidget->setPyramidsFormat( format );
}

void QgsRasterPyramidsOptionsWidget::setOverviewList()
{

  mOverviewList.clear();

  // if custom levels is toggled, get selection from line edit
  if ( cbxPyramidsLevelsCustom->isChecked() )
  {
    // should we also validate that numbers are increasing?
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const QStringList constSplit = lePyramidsLevels->text().trimmed().split( ' ', QString::SkipEmptyParts );
#else
    const QStringList constSplit = lePyramidsLevels->text().trimmed().split( ' ', Qt::SkipEmptyParts );
#endif
    for ( const QString &lev : constSplit )
    {
      QgsDebugMsg( "lev= " + lev );
      const int tmpInt = lev.toInt();
      if ( tmpInt > 0 )
      {
        QgsDebugMsg( "tmpInt= " + QString::number( tmpInt ) );
        // if number is valid, add to overview list
        mOverviewList << tmpInt;
      }
    }
  }
  else
  {
    for ( auto it = mOverviewCheckBoxes.constBegin(); it != mOverviewCheckBoxes.constEnd(); ++it )
    {
      if ( it.value()->isChecked() )
        mOverviewList << it.key();
    }
  }

  emit overviewListChanged();
}
