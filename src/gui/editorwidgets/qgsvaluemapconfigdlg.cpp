/***************************************************************************
    qgsvaluemapconfigdlg.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaluemapconfigdlg.h"

#include "qgsattributetypeloaddialog.h"

#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

QgsValueMapConfigDlg::QgsValueMapConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  tableWidget->insertRow( 0 );

  connect( addNullButton, SIGNAL( clicked() ), this, SLOT( addNullButtonPushed() ) );
  connect( removeSelectedButton, SIGNAL( clicked() ), this, SLOT( removeSelectedButtonPushed() ) );
  connect( loadFromLayerButton, SIGNAL( clicked() ), this, SLOT( loadFromLayerButtonPushed() ) );
  connect( loadFromCSVButton, SIGNAL( clicked() ), this, SLOT( loadFromCSVButtonPushed() ) );
  connect( tableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( vCellChanged( int, int ) ) );
}

QgsEditorWidgetConfig QgsValueMapConfigDlg::config()
{
  QgsEditorWidgetConfig cfg;
  QSettings settings;

  //store data to map
  for ( int i = 0; i < tableWidget->rowCount() - 1; i++ )
  {
    QTableWidgetItem *ki = tableWidget->item( i, 0 );
    QTableWidgetItem *vi = tableWidget->item( i, 1 );

    if ( !ki )
      continue;

    QString ks = ki->text();
    if (( ks == settings.value( "qgis/nullValue", "NULL" ).toString() ) && !( ki->flags() & Qt::ItemIsEditable ) )
      ks = VALUEMAP_NULL_TEXT;

    if ( !vi || vi->text().isNull() )
    {
      cfg.insert( ks, ks );
    }
    else
    {
      cfg.insert( vi->text(), ks );
    }
  }

  return cfg;
}

void QgsValueMapConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  tableWidget->clearContents();
  for ( int i = tableWidget->rowCount() - 1; i > 0; i-- )
  {
    tableWidget->removeRow( i );
  }

  int row = 0;
  for ( QgsEditorWidgetConfig::ConstIterator mit = config.begin(); mit != config.end(); mit++, row++ )
  {
    if ( mit.value().isNull() )
      setRow( row, mit.key(), QString() );
    else
      setRow( row, mit.value().toString(), mit.key() );
  }
}

void QgsValueMapConfigDlg::vCellChanged( int row, int column )
{
  Q_UNUSED( column );
  if ( row == tableWidget->rowCount() - 1 )
  {
    tableWidget->insertRow( row + 1 );
  } //else check type

  emit changed();
}

void QgsValueMapConfigDlg::removeSelectedButtonPushed()
{
  QList<QTableWidgetItem *> list = tableWidget->selectedItems();
  QSet<int> rowsToRemove;
  int removed = 0;
  int i;
  for ( i = 0; i < list.size(); i++ )
  {
    if ( list[i]->column() == 0 )
    {
      int row = list[i]->row();
      if ( !rowsToRemove.contains( row ) )
      {
        rowsToRemove.insert( row );
      }
    }
  }
  for ( i = 0; i < rowsToRemove.size(); i++ )
  {
    tableWidget->removeRow( rowsToRemove.values().at( i ) - removed );
    removed++;
  }
  emit changed();
}

void QgsValueMapConfigDlg::updateMap( const QMap<QString, QVariant> &map, bool insertNull )
{
  tableWidget->clearContents();
  for ( int i = tableWidget->rowCount() - 1; i > 0; i-- )
  {
    tableWidget->removeRow( i );
  }
  int row = 0;

  if ( insertNull )
  {
    setRow( row, VALUEMAP_NULL_TEXT, "<NULL>" );
    ++row;
  }

  for ( QMap<QString, QVariant>::const_iterator mit = map.begin(); mit != map.end(); ++mit, ++row )
  {
    if ( mit.value().isNull() )
      setRow( row, mit.key(), QString() );
    else
      setRow( row, mit.key(), mit.value().toString() );
  }
}

void QgsValueMapConfigDlg::setRow( int row, const QString value, const QString description )
{
  QSettings settings;
  QTableWidgetItem* valueCell;
  QTableWidgetItem* descriptionCell = new QTableWidgetItem( description );
  tableWidget->insertRow( row );
  if ( value == QString( VALUEMAP_NULL_TEXT ) )
  {
    QFont cellFont;
    cellFont.setItalic( true );
    valueCell = new QTableWidgetItem( settings.value( "qgis/nullValue", "NULL" ).toString() );
    valueCell->setFont( cellFont );
    valueCell->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    descriptionCell->setFont( cellFont );
  }
  else
  {
    valueCell = new QTableWidgetItem( value );
  }
  tableWidget->setItem( row, 0, valueCell );
  tableWidget->setItem( row, 1, descriptionCell );
}

void QgsValueMapConfigDlg::addNullButtonPushed()
{
  setRow( tableWidget->rowCount() - 1, VALUEMAP_NULL_TEXT, "<NULL>" );
}

void QgsValueMapConfigDlg::loadFromLayerButtonPushed()
{
  QgsAttributeTypeLoadDialog layerDialog( layer() );
  if ( !layerDialog.exec() )
    return;

  updateMap( layerDialog.valueMap(), layerDialog.insertNull() );
}

void QgsValueMapConfigDlg::loadFromCSVButtonPushed()
{
  QSettings settings;

  QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Select a file" ), QDir::homePath() );
  if ( fileName.isNull() )
    return;

  QFile f( fileName );

  if ( !f.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::information( nullptr,
                              tr( "Error" ),
                              tr( "Could not open file %1\nError was:%2" ).arg( fileName, f.errorString() ),
                              QMessageBox::Cancel );
    return;
  }

  QTextStream s( &f );
  s.setAutoDetectUnicode( true );

  QRegExp re0( "^([^;]*);(.*)$" );
  re0.setMinimal( true );
  QRegExp re1( "^([^,]*),(.*)$" );
  re1.setMinimal( true );
  QMap<QString, QVariant> map;

  s.readLine();

  while ( !s.atEnd() )
  {
    QString l = s.readLine().trimmed();

    QString key, val;
    if ( re0.indexIn( l ) >= 0 && re0.captureCount() == 2 )
    {
      key = re0.cap( 1 ).trimmed();
      val = re0.cap( 2 ).trimmed();
    }
    else if ( re1.indexIn( l ) >= 0 && re1.captureCount() == 2 )
    {
      key = re1.cap( 1 ).trimmed();
      val = re1.cap( 2 ).trimmed();
    }
    else
      continue;

    if (( key.startsWith( '\"' ) && key.endsWith( '\"' ) ) ||
        ( key.startsWith( '\'' ) && key.endsWith( '\'' ) ) )
    {
      key = key.mid( 1, key.length() - 2 );
    }

    if (( val.startsWith( '\"' ) && val.endsWith( '\"' ) ) ||
        ( val.startsWith( '\'' ) && val.endsWith( '\'' ) ) )
    {
      val = val.mid( 1, val.length() - 2 );
    }

    if ( key == settings.value( "qgis/nullValue", "NULL" ).toString() )
      key = QString( VALUEMAP_NULL_TEXT );

    map[ key ] = val;
  }

  updateMap( map, false );
}
