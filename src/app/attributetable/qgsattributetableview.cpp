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
#include <QMenu>

#include "qgsattributetableview.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablememorymodel.h"
#include "qgsattributetabledelegate.h"
#include "qgsattributetablefiltermodel.h"

#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsattributeaction.h"

QgsAttributeTableView::QgsAttributeTableView( QWidget* parent )
    : QTableView( parent ), mActionPopup( 0 )
{
  QSettings settings;
  restoreGeometry( settings.value( "/BetterAttributeTable/geometry" ).toByteArray() );

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
  delete mActionPopup;
}

void QgsAttributeTableView::closeEvent( QCloseEvent *event )
{
  QSettings settings;
  settings.setValue( "/BetterAttributeTable/geometry", QVariant( saveGeometry() ) );
}

void QgsAttributeTableView::contextMenuEvent( QContextMenuEvent *event )
{
  if ( mActionPopup )
  {
    delete mActionPopup;
    mActionPopup = 0;
  }

  QModelIndex idx = indexAt( event->pos() );
  if ( !idx.isValid() )
  {
    return;
  }

  QgsVectorLayer *vlayer = mModel->layer();
  if ( !vlayer )
    return;

  mActionPopup = new QMenu();

  if ( vlayer->actions()->size() != 0 )
  {

    QAction *a = mActionPopup->addAction( tr( "Run action" ) );
    a->setEnabled( false );

    for ( int i = 0; i < vlayer->actions()->size(); i++ )
    {
      const QgsAction &action = vlayer->actions()->at( i );

      if ( !action.runable() )
        continue;

      QgsAttributeTableAction *a = new QgsAttributeTableAction( action.name(), this, mModel, i, idx );
      mActionPopup->addAction( action.name(), a, SLOT( execute() ) );
    }
  }

  QgsAttributeTableAction *a = new QgsAttributeTableAction( tr( "Open form" ), this, mModel, -1, idx );
  mActionPopup->addAction( tr( "Open form" ), a, SLOT( featureForm() ) );

  mActionPopup->popup( event->globalPos() );
}

void QgsAttributeTableAction::execute()
{
  mModel->executeAction( mAction, mFieldIdx );
}

void QgsAttributeTableAction::featureForm()
{
  mModel->featureForm( mFieldIdx );
}
