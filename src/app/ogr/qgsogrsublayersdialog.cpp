/***************************************************************************
    qgsogrsublayersdialog.cpp  - dialog for selecting ogr sublayers
    ---------------------
    begin                : January 2009
    copyright            : (C) 2009 by Florian El Ahdab
    email                : felahdab at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrsublayersdialog.h"

#include <QSettings>
#include <QTableWidgetItem>


QgsOGRSublayersDialog::QgsOGRSublayersDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  layersTable->setHeaderLabels( QStringList() << tr( "Layer ID" ) << tr( "Layer name" ) << tr( "Nb of features" ) << tr( "Geometry type" ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/OGRSubLayers/geometry" ).toByteArray() );
}

QgsOGRSublayersDialog::~QgsOGRSublayersDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/OGRSubLayers/geometry", saveGeometry() );
}

QStringList QgsOGRSublayersDialog::getSelection()
{
  QStringList list;
  for ( int i = 0; i < layersTable->selectedItems().size(); i++ )
  {
    list << layersTable->selectedItems().at( i )->text( 1 );
  }
  return list;
}

QList<int> QgsOGRSublayersDialog::getSelectionIndexes()
{
  QList<int> list;
  for ( int i = 0; i < layersTable->selectedItems().size(); i++ )
  {
    list << layersTable->selectedItems().at( i )->text( 0 ).toInt();
  }
  return list;
}

void QgsOGRSublayersDialog::populateLayerTable( QStringList theList, QString delim )
{
  foreach( QString item, theList )
  {
    layersTable->addTopLevelItem( new QTreeWidgetItem( item.split( delim ) ) );
  }

  // resize columns
  for ( int i = 0; i < layersTable->columnCount(); i++ )
    layersTable->resizeColumnToContents( i );

  layersTable->setColumnWidth( 1, layersTable->columnWidth( 1 ) + 10 );
}
