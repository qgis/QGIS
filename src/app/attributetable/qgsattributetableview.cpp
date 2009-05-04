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

  shiftPressed = false;
  ctrlPressed = false;
}

void QgsAttributeTableView::setLayer( QgsVectorLayer* layer )
{
  QgsAttributeTableModel *bModel;

  if ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::RandomSelectGeometryAtId )
    bModel = new QgsAttributeTableModel( layer );
  else
    bModel = new QgsAttributeTableMemoryModel( layer );

  QgsAttributeTableFilterModel* bfModel = new QgsAttributeTableFilterModel( layer );
  bfModel->setSourceModel( bModel );

  setModel( bfModel );
}

QgsAttributeTableView::~QgsAttributeTableView()
{
}

void QgsAttributeTableView::closeEvent( QCloseEvent *event )
{
  QSettings settings;
  settings.setValue( "/BetterAttributeTable/geometry", QVariant( saveGeometry() ) );
}

void QgsAttributeTableView::keyPressEvent( QKeyEvent *event )
{
  // shift pressed
  if ( event->key() == Qt::Key_Shift )// && event->modifiers() & Qt::ShiftModifier)
    shiftPressed = true;
  else if ( event->key() == Qt::Key_Control )
    ctrlPressed = true;
  else
    QTableView::keyPressEvent( event );
}

void QgsAttributeTableView::keyReleaseEvent( QKeyEvent *event )
{
  // workaround for some Qt bug
  if ( event->key() == Qt::Key_Shift || event->key() == -1 )
    shiftPressed = false;
  else if ( event->key() == Qt::Key_Control )
    ctrlPressed = false;
  else
    QTableView::keyReleaseEvent( event );
}
