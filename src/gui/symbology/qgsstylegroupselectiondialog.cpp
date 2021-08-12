/***************************************************************************
    qgsstylegroupselectiondialog.h
    ---------------------
    begin                : Oct 2015
    copyright            : (C) 2015 by Alessandro Pasotti
    email                : elpaso at itopen dot it

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsstylegroupselectiondialog.h"
#include "qgsstyle.h"
#include "qgsgui.h"

#include <QStandardItemModel>
#include <QStandardItem>


QgsStyleGroupSelectionDialog::QgsStyleGroupSelectionDialog( QgsStyle *style, QWidget *parent )
  : QDialog( parent )
  , mStyle( style )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  QStandardItemModel *model = new QStandardItemModel( groupTree );
  groupTree->setModel( model );

  QStandardItem *favSymbols = new QStandardItem( tr( "Favorites" ) );
  favSymbols->setData( "favorites", Qt::UserRole + 2 );
  favSymbols->setEditable( false );
  setBold( favSymbols );
  model->appendRow( favSymbols );

  QStandardItem *allSymbols = new QStandardItem( tr( "All" ) );
  allSymbols->setData( "all", Qt::UserRole + 2 );
  allSymbols->setEditable( false );
  setBold( allSymbols );
  model->appendRow( allSymbols );

  QStandardItem *tags = new QStandardItem( QString() ); //require empty name to get first order groups
  tags->setData( "tagsheader", Qt::UserRole + 2 );
  tags->setEditable( false );
  tags->setFlags( tags->flags() & ~Qt::ItemIsSelectable );
  buildTagTree( tags );
  tags->setText( tr( "Tags" ) );//set title later
  setBold( tags );
  model->appendRow( tags );

  QStandardItem *tag = new QStandardItem( tr( "Smart Groups" ) );
  tag->setData( "smartgroupsheader", Qt::UserRole + 2 );
  tag->setEditable( false );
  tag->setFlags( tag->flags() & ~Qt::ItemIsSelectable );
  setBold( tag );
  const QgsSymbolGroupMap sgMap = mStyle->smartgroupsListMap();
  QgsSymbolGroupMap::const_iterator i = sgMap.constBegin();
  while ( i != sgMap.constEnd() )
  {
    QStandardItem *item = new QStandardItem( i.value() );
    item->setEditable( false );
    item->setData( i.key() );
    item->setData( "smartgroup", Qt::UserRole + 2 );
    tag->appendRow( item );
    ++i;
  }
  model->appendRow( tag );

  // expand things in the group tree
  const int rows = model->rowCount( model->indexFromItem( model->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    groupTree->setExpanded( model->indexFromItem( model->item( i ) ), true );
  }
  connect( groupTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsStyleGroupSelectionDialog::groupTreeSelectionChanged );
}

void QgsStyleGroupSelectionDialog::setBold( QStandardItem *item )
{
  QFont font = item->font();
  font.setBold( true );
  item->setFont( font );
}

void QgsStyleGroupSelectionDialog::groupTreeSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  const QModelIndexList selectedItems = selected.indexes();
  const QModelIndexList deselectedItems = deselected.indexes();

  for ( const QModelIndex &index : deselectedItems )
  {
    if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "tagssheader" ) )
    {
      // Ignore: it's the group header
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "favorites" ) )
    {
      emit favoritesDeselected();
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "all" ) )
    {
      emit allDeselected();
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "smartgroupsheader" ) )
    {
      // Ignore: it's the smartgroups header
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "smartgroup" ) )
    {
      emit smartgroupDeselected( index.data().toString() );
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "tag" ) )
    {
      // It's a tag
      emit tagDeselected( index.data().toString() );
    }
  }
  const auto constSelectedItems = selectedItems;
  for ( const QModelIndex &index : constSelectedItems )
  {
    if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "tagssheader" ) )
    {
      // Ignore: it's the group header
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "favorites" ) )
    {
      emit favoritesSelected();
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "all" ) )
    {
      emit allSelected();
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "smartgroupsheader" ) )
    {
      // Ignore: it's the smartgroups header
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "smartgroup" ) )
    {
      emit smartgroupSelected( index.data().toString() );
    }
    else if ( index.data( Qt::UserRole + 2 ).toString() == QLatin1String( "tag" ) )
    {
      // It's a tag
      emit tagSelected( index.data().toString() );
    }
  }
}

void QgsStyleGroupSelectionDialog::buildTagTree( QStandardItem *&parent )
{
  QStringList tags = mStyle->tags();
  tags.sort();
  const auto constTags = tags;
  for ( const QString &tag : constTags )
  {
    QStandardItem *item = new QStandardItem( tag );
    item->setData( mStyle->tagId( tag ) );
    item->setData( "tag", Qt::UserRole + 2 );
    item->setEditable( false );
    parent->appendRow( item );
  }
}
