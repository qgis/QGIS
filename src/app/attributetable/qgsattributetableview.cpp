/***************************************************************************
     QgsAttributeTableView.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QKeyEvent>
#include <QSettings>
#include <QHeaderView>

#include "qgsattributetableview.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablememorymodel.h"
#include "qgsattributetabledelegate.h"
#include "qgsattributetablefiltermodel.h"

#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"


QgsAttributeTableView::QgsAttributeTableView( QWidget* parent )
    : QTableView( parent )
{
  QSettings settings;
  restoreGeometry( settings.value( "/BetterTable/geometry" ).toByteArray() );

  verticalHeader()->setDefaultSectionSize( 20 );
  horizontalHeader()->setHighlightSections( false );

  setItemDelegate( new QgsAttributeTableDelegate( this ) );

  setSelectionBehavior( QAbstractItemView::SelectRows );
  setSelectionMode( QAbstractItemView::NoSelection );
  setSortingEnabled( true );

}

void QgsAttributeTableView::setLayer( QgsVectorLayer* layer )
{
  // in case the provider allows fast access to features
  // we will use the model that calls featureAtId() to fetch only the
  // features in the current view. Otherwise we'll have to store
  // everything in the memory because using featureAtId() would be too slow
  if ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::SelectAtId )
    mModel = new QgsAttributeTableModel( layer );
  else
    mModel = new QgsAttributeTableMemoryModel( layer );

  mFilterModel = new QgsAttributeTableFilterModel( layer );
  mFilterModel->setSourceModel( mModel );
  setModel( mFilterModel );
}

QgsAttributeTableView::~QgsAttributeTableView()
{
  delete mModel;
  delete mFilterModel;
}

void QgsAttributeTableView::closeEvent( QCloseEvent *event )
{
  QSettings settings;
  settings.setValue( "/BetterAttributeTable/geometry", QVariant( saveGeometry() ) );
}
