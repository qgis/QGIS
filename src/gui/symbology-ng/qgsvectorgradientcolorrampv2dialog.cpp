/***************************************************************************
    qgsvectorgradientcolorrampv2dialog.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorgradientcolorrampv2dialog.h"

#include "qgsvectorcolorrampv2.h"
#include "qgsdialog.h"
#include "qgscolordialog.h"
#include "qgscptcityarchive.h"

#include <QColorDialog>
#include <QInputDialog>
#include <QPainter>
#include <QSettings>
#include <QTableWidget>
#include <QTextEdit>

QgsVectorGradientColorRampV2Dialog::QgsVectorGradientColorRampV2Dialog( QgsVectorGradientColorRampV2* ramp, QWidget* parent )
    : QDialog( parent ), mRamp( ramp ), mCurrentItem( 0 )
{
  setupUi( this );
#ifdef Q_WS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  connect( btnColor1, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor1( const QColor& ) ) );
  connect( btnColor2, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor2( const QColor& ) ) );

  // handle stops
  updateStops();
  connect( groupStops, SIGNAL( toggled( bool ) ), this, SLOT( toggledStops( bool ) ) );
  connect( btnAddStop, SIGNAL( clicked() ), this, SLOT( addStop() ) );
  connect( btnRemoveStop, SIGNAL( clicked() ), this, SLOT( removeStop() ) );
  connect( treeStops, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( stopDoubleClicked( QTreeWidgetItem*, int ) ) );

  // fill type combobox
  cboType->blockSignals( true );
  cboType->addItem( tr( "Discrete" ) );
  cboType->addItem( tr( "Continous" ) );
  if ( mRamp->isDiscrete() )
    cboType->setCurrentIndex( 0 );
  else
    cboType->setCurrentIndex( 1 );
  cboType->blockSignals( false );

  // information button if needed
  // QPushButton* button = buttonBox->addButton( tr( "Information" ), QDialogButtonBox::ActionRole );
  if ( mRamp->info().isEmpty() )
    btnInformation->setEnabled( false );
  // else
  //   connect( button, SIGNAL( pressed() ), this, SLOT( showInformation() ) );

  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::on_cboType_currentIndexChanged( int index )
{
  if (( index == 0 && mRamp->isDiscrete() ) ||
      ( index == 1 && !mRamp->isDiscrete() ) )
    return;
  mRamp->convertToDiscrete( index == 0 );
  updateStops();
  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::on_btnInformation_pressed()
{
  if ( mRamp->info().isEmpty() )
    return;

  QgsDialog *dlg = new QgsDialog( this );
  QLabel *label = 0;

  // information table
  QTableWidget *tableInfo = new QTableWidget( dlg );
  tableInfo->verticalHeader()->hide();
  tableInfo->horizontalHeader()->hide();
  tableInfo->setRowCount( mRamp->info().count() );
  tableInfo->setColumnCount( 2 );
  int i = 0;
  for ( QgsStringMap::const_iterator it = mRamp->info().constBegin();
        it != mRamp->info().constEnd(); ++it )
  {
    if ( it.key().startsWith( "cpt-city" ) )
      continue;
    tableInfo->setItem( i, 0, new QTableWidgetItem( it.key() ) );
    tableInfo->setItem( i, 1, new QTableWidgetItem( it.value() ) );
    tableInfo->resizeRowToContents( i );
    i++;
  }
  tableInfo->resizeColumnToContents( 0 );
  tableInfo->horizontalHeader()->setStretchLastSection( true );
  tableInfo->setRowCount( i );
  tableInfo->setFixedHeight( tableInfo->rowHeight( 0 ) * i + 5 );
  dlg->layout()->addWidget( tableInfo );
  dlg->resize( 600, 250 );

  dlg->layout()->addSpacing( 5 );

  // gradient file
  QString gradientFile = mRamp->info().value( "cpt-city-gradient" );
  if ( ! gradientFile.isNull() )
  {
    QString fileName = gradientFile;
    fileName.replace( "<cpt-city>", QgsCptCityArchive::defaultBaseDir() );
    if ( ! QFile::exists( fileName ) )
    {
      fileName = gradientFile;
      fileName.replace( "<cpt-city>", "http://soliton.vm.bytemark.co.uk/pub/cpt-city" );
    }
    label = new QLabel( tr( "Gradient file : %1" ).arg( fileName ), dlg );
    label->setTextInteractionFlags( Qt::TextBrowserInteraction );
    dlg->layout()->addSpacing( 5 );
    dlg->layout()->addWidget( label );
  }

  // license file
  QString licenseFile = mRamp->info().value( "cpt-city-license" );
  if ( !licenseFile.isNull() )
  {
    QString fileName = licenseFile;
    fileName.replace( "<cpt-city>", QgsCptCityArchive::defaultBaseDir() );
    if ( ! QFile::exists( fileName ) )
    {
      fileName = licenseFile;
      fileName.replace( "<cpt-city>", "http://soliton.vm.bytemark.co.uk/pub/cpt-city" );
    }
    label = new QLabel( tr( "License file : %1" ).arg( fileName ), dlg );
    label->setTextInteractionFlags( Qt::TextBrowserInteraction );
    dlg->layout()->addSpacing( 5 );
    dlg->layout()->addWidget( label );
    if ( QFile::exists( fileName ) )
    {
      QTextEdit *textEdit = new QTextEdit( dlg );
      textEdit->setReadOnly( true );
      QFile file( fileName );
      if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        textEdit->setText( file.readAll() );
        file.close();
        dlg->layout()->addSpacing( 5 );
        dlg->layout()->addWidget( textEdit );
        dlg->resize( 600, 500 );
      }
    }
  }

  dlg->show(); //non modal
}

void QgsVectorGradientColorRampV2Dialog::updateStops()
{
  QgsGradientStopsList stops = mRamp->stops();
  groupStops->setChecked( !stops.isEmpty() );

  QList<QTreeWidgetItem *> items;
  for ( QgsGradientStopsList::iterator it = stops.begin();
        it != stops.end(); ++it )
  {
    double val = it->offset * 100.0;
    QStringList lst;
    lst << "." << QString(( val < 10 ) ? '0' + QString::number( val ) : QString::number( val ) );
    QTreeWidgetItem* item = new QTreeWidgetItem( lst );

    setStopColor( item, it->color );
    item->setData( 0, StopOffsetRole, it->offset );

    items.append( item );
  }
  treeStops->clear();
  treeStops->insertTopLevelItems( 0, items );
  treeStops->resizeColumnToContents( 0 );
  treeStops->setColumnWidth( 0, treeStops->columnWidth( 0 ) + 20 );
  treeStops->sortByColumn( 1, Qt::AscendingOrder );
  treeStops->setSortingEnabled( true );
}

void QgsVectorGradientColorRampV2Dialog::updatePreview()
{
  // update ramp stops from the tree widget
  QgsGradientStopsList stops;
  if ( groupStops->isChecked() )
  {
    int count = treeStops->topLevelItemCount();
    for ( int i = 0; i < count; i++ )
    {
      QTreeWidgetItem* item = treeStops->topLevelItem( i );
      double offset = item->data( 0, StopOffsetRole ).toDouble();
      QColor color = item->data( 0, StopColorRole ).value<QColor>();
      stops.append( QgsGradientStop( offset, color ) );
    }
  }
  mRamp->setStops( stops );

  // generate the preview
  QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerV2Utils::colorRampPreviewPixmap( mRamp, size ) );

  btnColor1->setColor( mRamp->color1() );
  btnColor2->setColor( mRamp->color2() );
}

void QgsVectorGradientColorRampV2Dialog::setColor1( const QColor& color )
{
  mRamp->setColor1( color );
  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::setColor2( const QColor& color )
{
  mRamp->setColor2( color );
  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::setStopColor( QTreeWidgetItem* item, QColor color )
{
  QSize iconSize( 16, 16 );
  QPixmap pixmap( iconSize );
  pixmap.fill( QColor( 0, 0, 0, 0 ) );
  QRect rect( 1, 1, iconSize.width() - 2, iconSize.height() - 2 );

  // draw a slightly rounded rectangle
  QPainter p;
  p.begin( &pixmap );
  p.setPen( Qt::NoPen );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( color );
  p.drawRoundedRect( rect, 2, 2 );
  p.end();

  item->setIcon( 0, QIcon( pixmap ) );
  item->setData( 0, StopColorRole, color );
  item->setText( 0, color.name() );
}

void QgsVectorGradientColorRampV2Dialog::setItemStopColor( const QColor& newColor )
{
  if ( mCurrentItem )
  {
    setStopColor( mCurrentItem, newColor );
    updatePreview();
  }
}

void QgsVectorGradientColorRampV2Dialog::stopDoubleClicked( QTreeWidgetItem* item, int column )
{
  if ( column == 0 )
  {
    QColor color;

    QSettings settings;
    if ( settings.value( "/qgis/live_color_dialogs", false ).toBool() )
    {
      mCurrentItem = item;
      color = QgsColorDialog::getLiveColor(
                item->data( 0, StopColorRole ).value<QColor>(),
                this, SLOT( setItemStopColor( const QColor& ) ), this );
      mCurrentItem = 0;
    }
    else
    {
      color = QColorDialog::getColor( item->data( 0, StopColorRole ).value<QColor>(), this );
    }
    if ( !color.isValid() )
      return;
    setStopColor( item, color );

    updatePreview();
  }
  else
  {
    bool ok;
    double key = item->data( 0, StopOffsetRole ).toDouble();
    // allow for floating-point values
    double val = key * 100;
#if QT_VERSION >= 0x40500
    val = QInputDialog::getDouble( this, tr( "Offset of the stop" ),
                                   tr( "Please enter offset in percents (%) of the new stop" ),
                                   val, 0, 100, 2, &ok );
#else
    QString res = QInputDialog::getText( this, tr( "Offset of the stop" ),
                                         tr( "Please enter offset in percents (%) of the new stop" ),
                                         QLineEdit::Normal, QString::number( val ), &ok );
    if ( ok )
      val = res.toDouble( &ok );
    if ( ok )
      ok = val >= 0 && val <= 100;
#endif
    if ( !ok )
      return;

    double newkey = val / 100.0;
    item->setText( 1, ( val < 10 ) ? '0' + QString::number( val ) : QString::number( val ) );
    item->setData( 0, StopOffsetRole, newkey );

    updatePreview();
  }
}

void QgsVectorGradientColorRampV2Dialog::addStop()
{
// Native Mac dialog works only for Qt Carbon
// Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
// Qt 4.7 Mac Cocoa bug: calling QInputDialog::getInt after QColorDialog::getColor will freeze app
// workaround: call QColorDialog::getColor below instead of here,
// but not needed at this time because of the other Qt bug
// FIXME need to also check max QT_VERSION when Qt bug(s) fixed
#ifndef Q_WS_MAC
  QColor color = QColorDialog::getColor( QColor(), this );

  if ( !color.isValid() )
    return;
  activateWindow();
#endif

  bool ok;
  double val = 50.0;
#if QT_VERSION >= 0x40500
  val = QInputDialog::getDouble( this, tr( "Offset of the stop" ),
                                 tr( "Please enter offset in percents (%) of the new stop" ),
                                 val, 0, 100, 2, &ok );
#else
  QString res = QInputDialog::getText( this, tr( "Offset of the stop" ),
                                       tr( "Please enter offset in percents (%) of the new stop" ),
                                       QLineEdit::Normal, QString::number( val ), &ok );
  if ( ok )
    val = res.toDouble( &ok );
  if ( ok )
    ok = val >= 0 && val <= 100;
#endif
  if ( !ok )
    return;
  activateWindow();

  double key = val / 100.0;
  QStringList lst;
  lst << "." << QString(( val < 10 ) ? '0' + QString::number( val ) : QString::number( val ) );

#ifdef Q_WS_MAC
  QColor color = QColorDialog::getColor( QColor(), this );

  if ( !color.isValid() )
    return;
  activateWindow();
#endif

  QTreeWidgetItem* item = new QTreeWidgetItem( lst );

  setStopColor( item, color );
  item->setData( 0, StopOffsetRole, key );

  treeStops->addTopLevelItem( item );

  treeStops->resizeColumnToContents( 0 );
  treeStops->setColumnWidth( 0, treeStops->columnWidth( 0 ) + 20 );

  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::removeStop()
{
  QTreeWidgetItem* item = treeStops->currentItem();
  if ( !item )
    return;
  int index = treeStops->indexOfTopLevelItem( item );
  treeStops->takeTopLevelItem( index );

  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::toggledStops( bool on )
{
  Q_UNUSED( on );
  updatePreview();
}
