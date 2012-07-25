/***************************************************************************
    qgswmtsdimensions.cpp  -  selector for WMTS dimensions
                             -------------------
    begin     : 2. May 2012
    copyright : (C) 2012 Juergen E. Fischer < jef at norbit dot de >, norBIT GmbH

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsprovider.h"
#include "qgswmtsdimensions.h"

#include <QSettings>
#include <QComboBox>

QgsWmtsDimensions::QgsWmtsDimensions( const QgsWmtsTileLayer &layer, QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  QStringList dims = layer.dimensions.keys();
  qSort( dims );

  mDimensions->setRowCount( dims.size() );

  for ( int i = 0; i < mDimensions->rowCount(); i++ )
  {
    const QgsWmtsDimension &d = layer.dimensions[ dims[i] ];

    mDimensions->setItem( i, 0, new QTableWidgetItem( d.identifier ) );
    mDimensions->setItem( i, 1, new QTableWidgetItem( d.title ) );
    mDimensions->setItem( i, 2, new QTableWidgetItem( d.abstract ) );
    mDimensions->setItem( i, 3, new QTableWidgetItem( d.defaultValue ) );

    QComboBox *cb = new QComboBox( mDimensions );
    cb->addItems( d.values );
    int idx = cb->findText( d.defaultValue );
    cb->setCurrentIndex( idx < 0 ? 0 : idx );
    mDimensions->setCellWidget( i, 4, cb );

    i++;
  }

  QSettings settings;
  QgsDebugMsg( "restoring geometry" );
  restoreGeometry( settings.value( "/Windows/WMTSDimensions/geometry" ).toByteArray() );
}

QgsWmtsDimensions::~QgsWmtsDimensions()
{
  QSettings settings;
  QgsDebugMsg( "saving geometry" );
  settings.setValue( "/Windows/WmtsDimensions/geometry", saveGeometry() );
}

void QgsWmtsDimensions::selectedDimensions( QHash<QString, QString> &selected )
{
  selected.clear();

  for ( int i = 0; i < mDimensions->rowCount(); i++ )
  {
    QComboBox *cb = qobject_cast< QComboBox * >( mDimensions->cellWidget( i, 4 ) );
    Q_ASSERT( cb );
    selected.insert( mDimensions->item( i, 0 )->text(),  cb->currentText() );
  }
}
