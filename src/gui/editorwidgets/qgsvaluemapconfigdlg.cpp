/***************************************************************************
    qgsvaluemapconfigdlg.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
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

  connect( removeSelectedButton, SIGNAL( clicked() ), this, SLOT( removeSelectedButtonPushed() ) );
  connect( loadFromLayerButton, SIGNAL( clicked() ), this, SLOT( loadFromLayerButtonPushed() ) );
  connect( loadFromCSVButton, SIGNAL( clicked() ), this, SLOT( loadFromCSVButtonPushed() ) );
  connect( tableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( vCellChanged( int, int ) ) );
}

QgsEditorWidgetConfig QgsValueMapConfigDlg::config()
{
  QgsEditorWidgetConfig cfg;

  //store data to map
  for ( int i = 0; i < tableWidget->rowCount() - 1; i++ )
  {
    QTableWidgetItem *ki = tableWidget->item( i, 0 );
    QTableWidgetItem *vi = tableWidget->item( i, 1 );

    if ( !ki )
      continue;

    if ( !vi || vi->text().isNull() )
    {
      cfg.insert( ki->text(), ki->text() );
    }
    else
    {
      cfg.insert( vi->text(), ki->text() );
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
    tableWidget->insertRow( row );
    if ( mit.value().isNull() )
    {
      tableWidget->setItem( row, 0, new QTableWidgetItem( mit.key() ) );
    }
    else
    {
      tableWidget->setItem( row, 0, new QTableWidgetItem( mit.value().toString() ) );
      tableWidget->setItem( row, 1, new QTableWidgetItem( mit.key() ) );
    }
  }
}

void QgsValueMapConfigDlg::vCellChanged( int row, int column )
{
  Q_UNUSED( column );
  if ( row == tableWidget->rowCount() - 1 )
  {
    tableWidget->insertRow( row + 1 );
  } //else check type
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
  for ( i = 0; i < rowsToRemove.values().size(); i++ )
  {
    tableWidget->removeRow( rowsToRemove.values()[i] - removed );
    removed++;
  }
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
    QSettings settings;
    tableWidget->setItem( row, 0, new QTableWidgetItem( settings.value( "qgis/nullValue", "NULL" ).toString() ) );
    tableWidget->setItem( row, 1, new QTableWidgetItem( "<NULL>" ) );
    ++row;
  }

  for ( QMap<QString, QVariant>::const_iterator mit = map.begin(); mit != map.end(); mit++, row++ )
  {
    tableWidget->insertRow( row );
    if ( mit.value().isNull() )
    {
      tableWidget->setItem( row, 0, new QTableWidgetItem( mit.key() ) );
    }
    else
    {
      tableWidget->setItem( row, 0, new QTableWidgetItem( mit.key() ) );
      tableWidget->setItem( row, 1, new QTableWidgetItem( mit.value().toString() ) );
    }
  }
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
  QString fileName = QFileDialog::getOpenFileName( 0, tr( "Select a file" ) );
  if ( fileName.isNull() )
    return;

  QFile f( fileName );

  if ( !f.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::information( NULL,
                              tr( "Error" ),
                              tr( "Could not open file %1\nError was:%2" ).arg( fileName ).arg( f.errorString() ),
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

    if (( key.startsWith( "\"" ) && key.endsWith( "\"" ) ) ||
        ( key.startsWith( "'" ) && key.endsWith( "'" ) ) )
    {
      key = key.mid( 1, key.length() - 2 );
    }

    if (( val.startsWith( "\"" ) && val.endsWith( "\"" ) ) ||
        ( val.startsWith( "'" ) && val.endsWith( "'" ) ) )
    {
      val = val.mid( 1, val.length() - 2 );
    }

    map[ key ] = val;
  }

  updateMap( map, false );
}
