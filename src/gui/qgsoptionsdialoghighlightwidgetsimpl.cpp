/***************************************************************************
    qgsoptionsdialoghighlightwidgetsimpl.cpp
     -------------------------------
    Date                 : February 2018
    Copyright            : (C) 2018 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QTreeView>
#include <QTreeWidget>
#include <QAbstractItemModel>
#include <QTableView>
#include <QTextDocumentFragment>

#include "qgsoptionsdialoghighlightwidget.h"
#include "qgsmessagebaritem.h"
#include "qgslogger.h"

#include "qgsoptionsdialoghighlightwidgetsimpl.h"

#include <functional>

const int HIGHLIGHT_BACKGROUND_RED = 255;
const int HIGHLIGHT_BACKGROUND_GREEN = 251;
const int HIGHLIGHT_BACKGROUND_BLUE = 190;
const int HIGHLIGHT_TEXT_RED = 0;
const int HIGHLIGHT_TEXT_GREEN = 0;
const int HIGHLIGHT_TEXT_BLUE = 0;

// ****************
// QLabel
QgsOptionsDialogHighlightLabel::QgsOptionsDialogHighlightLabel( QLabel *label )
  : QgsOptionsDialogHighlightWidget( label )
  , mLabel( label )
  , mStyleSheet( QStringLiteral( /*!search!*/"QLabel { background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6 );}/*!search!*/" ).arg( HIGHLIGHT_BACKGROUND_RED )
                 .arg( HIGHLIGHT_BACKGROUND_GREEN )
                 .arg( HIGHLIGHT_BACKGROUND_BLUE )
                 .arg( HIGHLIGHT_TEXT_RED )
                 .arg( HIGHLIGHT_TEXT_GREEN )
                 .arg( HIGHLIGHT_TEXT_BLUE ) )
{}

bool QgsOptionsDialogHighlightLabel::searchText( const QString &text )
{
  if ( !mLabel )
    return false;

  const QString labelText = QTextDocumentFragment::fromHtml( mLabel->text() ).toPlainText();
  return labelText.contains( text, Qt::CaseInsensitive );
}

bool QgsOptionsDialogHighlightLabel::highlightText( const QString &text )
{
  if ( !mWidget )
    return false;
  Q_UNUSED( text )
  mWidget->setStyleSheet( mWidget->styleSheet() + mStyleSheet );
  return true;
}

void QgsOptionsDialogHighlightLabel::reset()
{
  if ( !mWidget )
    return;
  QString ss = mWidget->styleSheet();
  ss.remove( mStyleSheet );
  mWidget->setStyleSheet( ss );
}

// ****************
// QCheckBox
QgsOptionsDialogHighlightCheckBox::QgsOptionsDialogHighlightCheckBox( QCheckBox *checkBox )
  : QgsOptionsDialogHighlightWidget( checkBox )
  , mCheckBox( checkBox )
  , mStyleSheet( QStringLiteral( "/*!search!*/QCheckBox { background-color: rgb(%1, %2, %3); color: rgb( %4, %5, %6);}/*!search!*/" ).arg( HIGHLIGHT_BACKGROUND_RED )
                 .arg( HIGHLIGHT_BACKGROUND_GREEN )
                 .arg( HIGHLIGHT_BACKGROUND_BLUE )
                 .arg( HIGHLIGHT_TEXT_RED )
                 .arg( HIGHLIGHT_TEXT_GREEN )
                 .arg( HIGHLIGHT_TEXT_BLUE ) )
{
}

bool QgsOptionsDialogHighlightCheckBox::searchText( const QString &text )
{
  if ( !mCheckBox )
    return false;

  return mCheckBox->text().contains( text, Qt::CaseInsensitive );

}

bool QgsOptionsDialogHighlightCheckBox::highlightText( const QString &text )
{
  if ( !mWidget )
    return false;
  Q_UNUSED( text )
  mWidget->setStyleSheet( mWidget->styleSheet() + mStyleSheet );
  return true;
}

void QgsOptionsDialogHighlightCheckBox::reset()
{
  if ( !mWidget )
    return;
  QString ss = mWidget->styleSheet();
  ss.remove( mStyleSheet );
  mWidget->setStyleSheet( ss );
}

// ****************
// QAbstractButton
QgsOptionsDialogHighlightButton::QgsOptionsDialogHighlightButton( QAbstractButton *button )
  : QgsOptionsDialogHighlightWidget( button )
  , mButton( button )
  , mStyleSheet( QStringLiteral( "/*!search!*/QAbstractButton { background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);}/*!search!*/" ).arg( HIGHLIGHT_BACKGROUND_RED )
                 .arg( HIGHLIGHT_BACKGROUND_GREEN )
                 .arg( HIGHLIGHT_BACKGROUND_BLUE )
                 .arg( HIGHLIGHT_TEXT_RED )
                 .arg( HIGHLIGHT_TEXT_GREEN )
                 .arg( HIGHLIGHT_TEXT_BLUE ) )
{
}

bool QgsOptionsDialogHighlightButton::searchText( const QString &text )
{
  if ( !mButton )
    return false;

  return mButton->text().contains( text, Qt::CaseInsensitive );

}

bool QgsOptionsDialogHighlightButton::highlightText( const QString &text )
{
  if ( !mWidget )
    return false;
  Q_UNUSED( text )
  mWidget->setStyleSheet( mWidget->styleSheet() + mStyleSheet );
  return true;
}

void QgsOptionsDialogHighlightButton::reset()
{
  if ( !mWidget )
    return;
  QString ss = mWidget->styleSheet();
  ss.remove( mStyleSheet );
  mWidget->setStyleSheet( ss );
}

// ****************
// QGroupBox
QgsOptionsDialogHighlightGroupBox::QgsOptionsDialogHighlightGroupBox( QGroupBox *groupBox )
  : QgsOptionsDialogHighlightWidget( groupBox )
  , mGroupBox( groupBox )
  , mStyleSheet( QStringLiteral( "/*!search!*/QGroupBox::title { background-color: rgb(%1, %2, %3); color: rgb(%4, %5, %6);}/*!search!*/" ).arg( HIGHLIGHT_BACKGROUND_RED )
                 .arg( HIGHLIGHT_BACKGROUND_GREEN )
                 .arg( HIGHLIGHT_BACKGROUND_BLUE )
                 .arg( HIGHLIGHT_TEXT_RED )
                 .arg( HIGHLIGHT_TEXT_GREEN )
                 .arg( HIGHLIGHT_TEXT_BLUE ) )
{
}

bool QgsOptionsDialogHighlightGroupBox::searchText( const QString &text )
{
  if ( !mGroupBox )
    return false;

  return mGroupBox->title().contains( text, Qt::CaseInsensitive );
}

bool QgsOptionsDialogHighlightGroupBox::highlightText( const QString &text )
{
  Q_UNUSED( text )
  if ( !mWidget )
    return false;

  mWidget->setStyleSheet( mWidget->styleSheet() + mStyleSheet );
  return true;
}

void QgsOptionsDialogHighlightGroupBox::reset()
{
  if ( !mWidget )
    return;
  QString ss = mWidget->styleSheet();
  ss.remove( mStyleSheet );
  mWidget->setStyleSheet( ss );
}

// ****************
// QTreeView
QgsOptionsDialogHighlightTree::QgsOptionsDialogHighlightTree( QTreeView *treeView )
  : QgsOptionsDialogHighlightWidget( treeView )
  , mTreeView( treeView )
{
}

bool QgsOptionsDialogHighlightTree::searchText( const QString &text )
{
  if ( !mTreeView || !mTreeView->model() )
    return false;

  // search headers too!
  for ( int col = 0; col < mTreeView->model()->columnCount(); ++col )
  {
    const QString headerText = mTreeView->model()->headerData( col, Qt::Horizontal ).toString();
    if ( headerText.contains( text, Qt::CaseInsensitive ) )
      return true;
  }

  const QModelIndexList hits = mTreeView->model()->match( mTreeView->model()->index( 0, 0 ), Qt::DisplayRole, text, 1, Qt::MatchContains | Qt::MatchRecursive );
  return !hits.isEmpty();
}

bool QgsOptionsDialogHighlightTree::highlightText( const QString &text )
{
  bool success = false;
  QTreeWidget *treeWidget = qobject_cast<QTreeWidget *>( mTreeView );
  if ( treeWidget )
  {
    mTreeInitialVisible.clear();
    // initially hide everything
    std::function< void( QTreeWidgetItem *, bool ) > setChildrenVisible;
    setChildrenVisible = [this, &setChildrenVisible]( QTreeWidgetItem * item, bool visible )
    {
      for ( int i = 0; i < item->childCount(); ++i )
        setChildrenVisible( item->child( i ), visible );
      mTreeInitialVisible.insert( item, !item->isHidden() );
      item->setHidden( !visible );
    };
    setChildrenVisible( treeWidget->invisibleRootItem(), false );

    const QList<QTreeWidgetItem *> items = treeWidget->findItems( text, Qt::MatchContains | Qt::MatchRecursive, 0 );
    success = !items.empty();
    mTreeInitialExpand.clear();
    for ( QTreeWidgetItem *item : items )
    {
      setChildrenVisible( item, true );

      QTreeWidgetItem *parent = item;
      while ( parent )
      {
        if ( mTreeInitialExpand.contains( parent ) )
          break;
        mTreeInitialExpand.insert( parent, parent->isExpanded() );
        parent->setExpanded( true );
        parent->setHidden( false );
        parent = parent->parent();
      }
    }
  }

  return success;
}

void QgsOptionsDialogHighlightTree::reset()
{
  if ( !mTreeView )
    return;

  QTreeWidget *treeWidget = qobject_cast<QTreeWidget *>( mTreeView );
  if ( treeWidget )
  {
    // show everything
    std::function< void( QTreeWidgetItem * ) > showChildren;
    showChildren = [this, &showChildren]( QTreeWidgetItem * item )
    {
      for ( int i = 0; i < item->childCount(); ++i )
        showChildren( item->child( i ) );
      item->setHidden( !mTreeInitialVisible.value( item, true ) );
    };
    showChildren( treeWidget->invisibleRootItem() );
    for ( QTreeWidgetItem *item : mTreeInitialExpand.keys() )
    {
      if ( item )
      {
        item->setExpanded( mTreeInitialExpand.value( item ) );
      }
    }
    mTreeInitialExpand.clear();
  }
}


// ****************
// QTableView
QgsOptionsDialogHighlightTable::QgsOptionsDialogHighlightTable( QTableView *tableView )
  : QgsOptionsDialogHighlightWidget( tableView )
  , mTableView( tableView )
{
}

bool QgsOptionsDialogHighlightTable::searchText( const QString &text )
{
  if ( !mTableView || !mTableView->model() )
    return false;

  // search headers too!
  for ( int col = 0; col < mTableView->model()->columnCount(); ++col )
  {
    const QString headerText = mTableView->model()->headerData( col, Qt::Horizontal ).toString();
    if ( headerText.contains( text, Qt::CaseInsensitive ) )
      return true;
  }

  const QModelIndexList hits = mTableView->model()->match( mTableView->model()->index( 0, 0 ), Qt::DisplayRole, text, 1, Qt::MatchContains | Qt::MatchRecursive );
  return !hits.isEmpty();
}

bool QgsOptionsDialogHighlightTable::highlightText( const QString & )
{
  return false;
}

void QgsOptionsDialogHighlightTable::reset()
{
}
