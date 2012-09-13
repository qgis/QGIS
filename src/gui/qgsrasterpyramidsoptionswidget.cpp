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
#include "qgslogger.h"
#include "qgsdialog.h"

#include "gdal.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "cpl_minixml.h"

#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QCheckBox>


QgsRasterPyramidsOptionsWidget::QgsRasterPyramidsOptionsWidget( QWidget* parent, QString provider )
    : QWidget( parent ), mProvider( provider )
{
  setupUi( this );

  mPyramidsOptionsWidget->setProvider( provider );
  mPyramidsOptionsWidget->setFormat( "_pyramids" );
  // mPyramidsOptionsWidget->swapOptionsUI( 1 );

  updateUi();
}

QgsRasterPyramidsOptionsWidget::~QgsRasterPyramidsOptionsWidget()
{
}


void QgsRasterPyramidsOptionsWidget::updateUi()
{
  QSettings mySettings;
  QString prefix = mProvider + "/driverOptions/_pyramids/";
  QString tmpStr;

  // cbxPyramidsInternal->setChecked( mySettings.value( prefix + "internal", false ).toBool() );
  tmpStr = mySettings.value( prefix + "format", "gtiff" ).toString();
  if ( tmpStr == "internal" )
    cbxPyramidsFormat->setCurrentIndex( 1 );
  else if ( tmpStr == "external_erdas" )
    cbxPyramidsFormat->setCurrentIndex( 2 );
  else
    cbxPyramidsFormat->setCurrentIndex( 0 );

  // initialize resampling methods
  cboResamplingMethod->clear();
  foreach ( QString method, QgsRasterDataProvider::pyramidResamplingMethods( mProvider ) )
    cboResamplingMethod->addItem( method );
  cboResamplingMethod->setCurrentIndex( cboResamplingMethod->findText(
                                          mySettings.value( prefix + "resampling", "Average" ).toString() ) );

  // validate string, only space-separated positive integers are allowed
  lePyramidsLevels->setValidator( new QRegExpValidator( QRegExp( "(\\d*)(\\s\\d*)*" ), lePyramidsLevels ) );
  connect( lePyramidsLevels, SIGNAL( editingFinished() ),
           this, SLOT( setOverviewList() ) );

  // overview list
  if ( mOverviewCheckBoxes.isEmpty() )
  {
    QList<int> overviewList;
    overviewList << 2 << 4 << 8 << 16 << 32 << 64;
    mOverviewCheckBoxes.clear();
    foreach ( int i, overviewList )
    {
      mOverviewCheckBoxes[ i ] = new QCheckBox( QString::number( i ), this );
      connect( mOverviewCheckBoxes[ i ], SIGNAL( toggled( bool ) ),
               this, SLOT( setOverviewList() ) );
      layoutPyramidsLevels->addWidget( mOverviewCheckBoxes[ i ] );
    }
  }
  else
  {
    foreach ( int i, mOverviewCheckBoxes.keys() )
      mOverviewCheckBoxes[ i ]->setChecked( false );
  }
  tmpStr = mySettings.value( prefix + "overviewList", "" ).toString();
  foreach ( QString lev, tmpStr.split( " ", QString::SkipEmptyParts ) )
  {
    if ( mOverviewCheckBoxes.contains( lev.toInt() ) )
      mOverviewCheckBoxes[ lev.toInt()]->setChecked( true );
  }
  setOverviewList();

  mPyramidsOptionsWidget->updateProfiles();
}



QString QgsRasterPyramidsOptionsWidget::resamplingMethod() const
{
  return cboResamplingMethod->currentText().trimmed();
}

void QgsRasterPyramidsOptionsWidget::apply()
{
  QSettings mySettings;
  QString prefix = mProvider + "/driverOptions/_pyramids/";
  QString tmpStr;

  // mySettings.setValue( prefix + "internal", cbxPyramidsInternal->isChecked() );
  if ( cbxPyramidsFormat->currentIndex() == 1 )
    tmpStr = "internal";
  else if ( cbxPyramidsFormat->currentIndex() == 2 )
    tmpStr = "external_erdas";
  else
    tmpStr = "external";
  mySettings.setValue( prefix + "format", tmpStr );
  mySettings.setValue( prefix + "resampling", cboResamplingMethod->currentText().trimmed() );
  mySettings.setValue( prefix + "overviewStr", lePyramidsLevels->text().trimmed() );

  // overview list
  tmpStr = "";
  foreach ( int i, mOverviewCheckBoxes.keys() )
  {
    if ( mOverviewCheckBoxes[ i ]->isChecked() )
      tmpStr += QString::number( i ) + " ";
  }
  mySettings.setValue( prefix + "overviewList", tmpStr.trimmed() );

  mPyramidsOptionsWidget->apply();
}

void QgsRasterPyramidsOptionsWidget::checkAllLevels( bool checked )
{
  foreach ( int i, mOverviewCheckBoxes.keys() )
    mOverviewCheckBoxes[ i ]->setChecked( checked );
}

void QgsRasterPyramidsOptionsWidget::on_cbxPyramidsLevelsCustom_toggled( bool toggled )
{
  // if toggled, disable checkboxes and enable line edit
  lePyramidsLevels->setEnabled( toggled );
  foreach ( int i, mOverviewCheckBoxes.keys() )
    mOverviewCheckBoxes[ i ]->setEnabled( ! toggled );
  setOverviewList();
}

void QgsRasterPyramidsOptionsWidget::setOverviewList()
{
  QgsDebugMsg( "Entered" );

  mOverviewList.clear();

  // if custum levels is toggled, get selection from line edit
  if ( cbxPyramidsLevelsCustom->isChecked() )
  {
    // should we also validate that numbers are increasing?
    foreach ( QString lev, lePyramidsLevels->text().trimmed().split( " ", QString::SkipEmptyParts ) )
    {
      QgsDebugMsg( "lev= " + lev );
      int tmpInt = lev.toInt();
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
    foreach ( int i, mOverviewCheckBoxes.keys() )
    {
      if ( mOverviewCheckBoxes[ i ]->isChecked() )
        mOverviewList << i;
    }
  }

  emit overviewListChanged();
}
