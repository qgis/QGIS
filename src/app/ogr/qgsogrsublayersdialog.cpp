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

#include <QTableWidgetItem>


QgsOGRSublayersDialog::QgsOGRSublayersDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  QStringList labels = QStringList() << tr( "Layer ID" ) << tr( "Layer name" ) << tr( "Nb of features" ) << tr( "Geometry type" );
  layersTable->setHeaderLabels( labels );
}

QgsOGRSublayersDialog::~QgsOGRSublayersDialog()
{
}

QStringList QgsOGRSublayersDialog::getSelection()
{
  QStringList list = QStringList();
  for ( int i = 0; i < layersTable->selectedItems().size(); i++ )
  {
    QString theItem = layersTable->selectedItems().at( i )->text( 1 );
    list.append( theItem );
  }
  return list;
}

void QgsOGRSublayersDialog::populateLayerTable( QStringList theList, QString delim )
{
  for ( int i = 0; i < theList.size(); i++ )
  {
    QString line = theList.at( i );
    QStringList elements = line.split( delim );
    QStringList item = QStringList();
    item << elements.at( 0 ) << elements.at( 1 ) << elements.at( 2 ) << elements.at( 3 );
    layersTable->addTopLevelItem( new QTreeWidgetItem( item ) );
  }
}
