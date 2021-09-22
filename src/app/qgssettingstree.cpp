/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QHeaderView>
#include <QEvent>

#include "qgssettingstree.h"
#include "qgsvariantdelegate.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgssettingsentry.h"
#include "qgssettingsregistrycore.h"
#include "qgsapplication.h"
#include "qgsguiutils.h"

#include <QMenu>
#include <QMessageBox>

QgsSettingsTree::QgsSettingsTree( QWidget *parent )
  : QTreeWidget( parent )
{
  setItemDelegate( new QgsVariantDelegate( this ) );

  QStringList labels;
  labels << tr( "Setting" ) << tr( "Type" ) << tr( "Value" ) << tr( "Description" );
  setHeaderLabels( labels );
  header()->resizeSection( ColumnSettings, 250 );
  header()->resizeSection( ColumnType, 100 );
  header()->resizeSection( ColumnValue, 250 );

  setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

  mRefreshTimer.setInterval( 2000 );

  mGroupIcon = QgsApplication::getThemeIcon( QStringLiteral( "mIconFolderOpen.svg" ) );
  mKeyIcon = QgsApplication::getThemeIcon( QStringLiteral( "mIconDeselected.svg" ) );

  setEditTriggers( QAbstractItemView::AllEditTriggers );

  connect( &mRefreshTimer, &QTimer::timeout, this, &QgsSettingsTree::maybeRefresh );

  setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, &QTreeWidget::customContextMenuRequested, this, &QgsSettingsTree::showContextMenu );
  mContextMenu = new QMenu( this );
}

void QgsSettingsTree::setSettingsObject( QgsSettings *settings )
{
  mSettings = settings;
  clear();

  if ( mSettings )
  {
    mSettings->setParent( this );
    if ( isVisible() )
      refresh();
    if ( mAutoRefresh )
      mRefreshTimer.start();
  }
  else
  {
    mRefreshTimer.stop();
  }
}

QSize QgsSettingsTree::sizeHint() const
{
  return QSize( 800, 600 );
}

void QgsSettingsTree::setAutoRefresh( bool autoRefresh )
{
  mAutoRefresh = autoRefresh;
  if ( mAutoRefresh )
  {
    maybeRefresh();
    if ( mSettings )
      mRefreshTimer.start();
  }
  else
  {
    mRefreshTimer.stop();
  }
}

void QgsSettingsTree::maybeRefresh()
{
  if ( state() != EditingState )
    refresh();
}

void QgsSettingsTree::refresh()
{
  if ( !mSettings )
    return;

  disconnect( this, &QTreeWidget::itemChanged,
              this, &QgsSettingsTree::updateSetting );

  mSettings->sync();

  // add any settings not in QgsSettings object, so it will show up in the tree view
  QMap<QString, QStringList>::const_iterator it = mSettingsMap.constBegin();
  while ( it != mSettingsMap.constEnd() )
  {
    if ( ! mSettings->contains( it.key() ) )
    {
      mSettings->setValue( it.key(), it.value().at( 3 ) );
    }
    ++it;
  }

  updateChildItems( nullptr );

  connect( this, &QTreeWidget::itemChanged,
           this, &QgsSettingsTree::updateSetting );
}

bool QgsSettingsTree::event( QEvent *event )
{
  if ( event->type() == QEvent::WindowActivate )
  {
    if ( isActiveWindow() && mAutoRefresh )
      maybeRefresh();
  }
  return QTreeWidget::event( event );
}

void QgsSettingsTree::showEvent( QShowEvent * )
{
  const QgsTemporaryCursorOverride waitCursor( Qt::BusyCursor );
  refresh();
}

void QgsSettingsTree::updateSetting( QTreeWidgetItem *item )
{
  const QString key = itemKey( item );
  if ( key.isNull() )
    return;

  mSettings->setValue( key, item->data( ColumnValue, Qt::UserRole ) );
  if ( mAutoRefresh )
    refresh();
}

void QgsSettingsTree::showContextMenu( QPoint pos )
{
  QTreeWidgetItem *item = itemAt( pos );
  if ( !item )
    return;

  const Type itemType = item->data( ColumnSettings, TypeRole ).value< Type >();
  const QString itemText = item->data( ColumnSettings, Qt::DisplayRole ).toString();
  const QString itemPath = item->data( ColumnSettings, PathRole ).toString();
  mContextMenu->clear();

  switch ( itemType )
  {
    case Group:
    {
      QAction *deleteAction = new QAction( tr( "Delete Group…" ), mContextMenu );
      connect( deleteAction, &QAction::triggered, this, [ = ]
      {
        if ( QMessageBox::question( nullptr, tr( "Delete Group" ),
                                    tr( "Are you sure you want to delete the %1 group?" ).arg( itemText ),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
          return;


        mSettings->remove( itemPath );
        refresh();

      } );
      mContextMenu->addAction( deleteAction );
      break;
    }

    case Setting:
    {
      QAction *deleteSetting = new QAction( tr( "Delete Setting…" ), mContextMenu );
      connect( deleteSetting, &QAction::triggered, this, [ = ]
      {
        if ( QMessageBox::question( nullptr, tr( "Delete Setting" ),
                                    tr( "Are you sure you want to delete the %1 setting?" ).arg( itemPath ),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
          return;

        mSettings->remove( itemPath );
        refresh();
      } );

      mContextMenu->addAction( deleteSetting );
      break;
    }

  }

  mContextMenu->exec( mapToGlobal( pos ) );
}

void QgsSettingsTree::updateChildItems( QTreeWidgetItem *parent )
{
  int dividerIndex = 0;

  const auto constChildGroups = mSettings->childGroups();
  for ( const QString &group : constChildGroups )
  {
    QTreeWidgetItem *child = nullptr;
    const int childIndex = findChild( parent, group, dividerIndex );
    if ( childIndex != -1 )
    {
      child = childAt( parent, childIndex );
      child->setText( ColumnType, QString() );
      child->setText( ColumnValue, QString() );
      child->setData( ColumnValue, Qt::UserRole, QVariant() );
      moveItemForward( parent, childIndex, dividerIndex );
    }
    else
    {
      child = createItem( group, parent, dividerIndex, true );
    }
    child->setIcon( ColumnSettings, mGroupIcon );
    ++dividerIndex;

    mSettings->beginGroup( group );
    updateChildItems( child );
    mSettings->endGroup();
  }

  const auto constChildKeys = mSettings->childKeys();
  for ( const QString &key : constChildKeys )
  {
    QTreeWidgetItem *child = nullptr;
    const int childIndex = findChild( parent, key, 0 );

    if ( childIndex == -1 || childIndex >= dividerIndex )
    {
      if ( childIndex != -1 )
      {
        child = childAt( parent, childIndex );
        for ( int i = 0; i < child->childCount(); ++i )
          delete childAt( child, i );
        moveItemForward( parent, childIndex, dividerIndex );
      }
      else
      {
        child = createItem( key, parent, dividerIndex, false );
      }
      child->setIcon( ColumnSettings, mKeyIcon );
      ++dividerIndex;
    }
    else
    {
      child = childAt( parent, childIndex );
    }

    const QVariant value = mSettings->value( key );
    if ( value.type() == QVariant::Invalid )
    {
      child->setText( ColumnType, QStringLiteral( "Invalid" ) );
    }
    else
    {
      child->setText( ColumnType, QVariant::typeToName( QgsVariantDelegate::type( value ) ) );
    }
    child->setText( ColumnValue, QgsVariantDelegate::displayText( value ) );
    child->setData( ColumnValue, Qt::UserRole, value );
  }

  while ( dividerIndex < childCount( parent ) )
    delete childAt( parent, dividerIndex );
}

QTreeWidgetItem *QgsSettingsTree::createItem( const QString &text,
    QTreeWidgetItem *parent, int index, const bool isGroup )
{
  QTreeWidgetItem *after = nullptr;
  if ( index != 0 )
    after = childAt( parent, index - 1 );

  QTreeWidgetItem *item = nullptr;
  if ( parent )
    item = new QTreeWidgetItem( parent, after );
  else
    item = new QTreeWidgetItem( this, after );

  item->setText( ColumnSettings, text );
  if ( !isGroup )
    item->setFlags( item->flags() | Qt::ItemIsEditable );

  item->setData( ColumnSettings, TypeRole, isGroup ? Group : Setting );

  const QString completeSettingsPath = mSettings->group().isEmpty() ? text : mSettings->group() + '/' + text;
  item->setData( ColumnSettings, PathRole, completeSettingsPath );

  // If settings registered add description
  if ( !isGroup )
  {
    const QgsSettingsEntryBase *settingsEntry = QgsApplication::settingsRegistryCore()->settingsEntry( completeSettingsPath, true );
    if ( settingsEntry )
    {
      item->setText( ColumnDescription, settingsEntry->description() );
      item->setToolTip( ColumnDescription, settingsEntry->description() );
    }
  }

  const QString key = itemKey( item );
  QgsDebugMsgLevel( key, 4 );
  if ( mSettingsMap.contains( key ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "contains!!!!" ), 4 );
    const QStringList values = mSettingsMap[ key ];
    item->setText( ColumnDescription, values.at( 0 ) );
    item->setToolTip( ColumnDescription, values.at( 0 ) );
    item->setToolTip( ColumnSettings, values.at( 1 ) );
    item->setToolTip( ColumnValue, values.at( 1 ) );
  }

  return item;
}

QString QgsSettingsTree::itemKey( QTreeWidgetItem *item )
{
  if ( ! item )
    return QString();

  QString key = item->text( ColumnSettings );
  QTreeWidgetItem *ancestor = item->parent();
  while ( ancestor )
  {
    key.prepend( ancestor->text( ColumnSettings ) + '/' );
    ancestor = ancestor->parent();
  }

  return key;
}

QTreeWidgetItem *QgsSettingsTree::childAt( QTreeWidgetItem *parent, int index )
{
  if ( parent )
    return parent->child( index );
  else
    return topLevelItem( index );
}

int QgsSettingsTree::childCount( QTreeWidgetItem *parent )
{
  if ( parent )
    return parent->childCount();
  else
    return topLevelItemCount();
}

int QgsSettingsTree::findChild( QTreeWidgetItem *parent, const QString &text,
                                int startIndex )
{
  for ( int i = startIndex; i < childCount( parent ); ++i )
  {
    if ( childAt( parent, i )->text( ColumnSettings ) == text )
      return i;
  }
  return -1;
}

void QgsSettingsTree::moveItemForward( QTreeWidgetItem *parent, int oldIndex,
                                       int newIndex )
{
  for ( int i = 0; i < oldIndex - newIndex; ++i )
    delete childAt( parent, newIndex );
}
