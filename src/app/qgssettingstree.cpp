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
#include <QSettings>
#include <QEvent>

#include "qgssettingstree.h"
#include "qgsvariantdelegate.h"

#include "qgslogger.h"

QgsSettingsTree::QgsSettingsTree( QWidget *parent )
    : QTreeWidget( parent )
{
  setItemDelegate( new QgsVariantDelegate( this ) );

  QStringList labels;
  labels << tr( "Setting" ) << tr( "Type" ) << tr( "Value" ) << tr( "Description" );
  setHeaderLabels( labels );
  // header()->setResizeMode( 0, QHeaderView::Stretch );
  // header()->setResizeMode( 2, QHeaderView::Stretch );
  header()->resizeSection( 0, 250 );
  header()->resizeSection( 1, 100 );
  header()->resizeSection( 2, 100 );

  settings = nullptr;
  refreshTimer.setInterval( 2000 );
  autoRefresh = false;

  groupIcon.addPixmap( style()->standardPixmap( QStyle::SP_DirClosedIcon ),
                       QIcon::Normal, QIcon::Off );
  groupIcon.addPixmap( style()->standardPixmap( QStyle::SP_DirOpenIcon ),
                       QIcon::Normal, QIcon::On );
  keyIcon.addPixmap( style()->standardPixmap( QStyle::SP_FileIcon ) );

  connect( &refreshTimer, SIGNAL( timeout() ), this, SLOT( maybeRefresh() ) );
}

void QgsSettingsTree::setSettingsObject( QSettings *settings )
{
  delete this->settings;
  this->settings = settings;
  clear();

  if ( settings )
  {
    settings->setParent( this );
    refresh();
    if ( autoRefresh )
      refreshTimer.start();
  }
  else
  {
    refreshTimer.stop();
  }
}

QSize QgsSettingsTree::sizeHint() const
{
  return QSize( 800, 600 );
}

void QgsSettingsTree::setAutoRefresh( bool autoRefresh )
{
  this->autoRefresh = autoRefresh;
  if ( settings )
  {
    if ( autoRefresh )
    {
      maybeRefresh();
      refreshTimer.start();
    }
    else
    {
      refreshTimer.stop();
    }
  }
}

void QgsSettingsTree::setFallbacksEnabled( bool enabled )
{
  if ( settings )
  {
    settings->setFallbacksEnabled( enabled );
    refresh();
  }
}

void QgsSettingsTree::maybeRefresh()
{
  if ( state() != EditingState )
    refresh();
}

void QgsSettingsTree::refresh()
{
  if ( !settings )
    return;

  disconnect( this, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
              this, SLOT( updateSetting( QTreeWidgetItem* ) ) );

  settings->sync();

  // add any settings not in QSettings object, so it will show up in the tree view
  if ( settings )
  {
    QMap<QString, QStringList>::const_iterator it = settingsMap.constBegin();
    while ( it != settingsMap.constEnd() )
    {
      if ( ! settings->contains( it.key() ) )
      {
        settings->setValue( it.key(), it.value().at( 3 ) );
      }
      ++it;
    }
  }

  updateChildItems( nullptr );

  connect( this, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
           this, SLOT( updateSetting( QTreeWidgetItem* ) ) );
}

bool QgsSettingsTree::event( QEvent *event )
{
  if ( event->type() == QEvent::WindowActivate )
  {
    if ( isActiveWindow() && autoRefresh )
      maybeRefresh();
  }
  return QTreeWidget::event( event );
}

void QgsSettingsTree::updateSetting( QTreeWidgetItem *item )
{
  QString key = itemKey( item );
  if ( key.isNull() )
    return;

  settings->setValue( key, item->data( 2, Qt::UserRole ) );
  if ( autoRefresh )
    refresh();
}

void QgsSettingsTree::updateChildItems( QTreeWidgetItem *parent )
{
  int dividerIndex = 0;

  Q_FOREACH ( const QString& group, settings->childGroups() )
  {
    QTreeWidgetItem *child;
    int childIndex = findChild( parent, group, dividerIndex );
    if ( childIndex != -1 )
    {
      child = childAt( parent, childIndex );
      child->setText( 1, "" );
      child->setText( 2, "" );
      child->setData( 2, Qt::UserRole, QVariant() );
      moveItemForward( parent, childIndex, dividerIndex );
    }
    else
    {
      child = createItem( group, parent, dividerIndex );
    }
    child->setIcon( 0, groupIcon );
    ++dividerIndex;

    settings->beginGroup( group );
    updateChildItems( child );
    settings->endGroup();
  }

  Q_FOREACH ( const QString& key, settings->childKeys() )
  {
    QTreeWidgetItem *child;
    int childIndex = findChild( parent, key, 0 );

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
        child = createItem( key, parent, dividerIndex );
      }
      child->setIcon( 0, keyIcon );
      ++dividerIndex;
    }
    else
    {
      child = childAt( parent, childIndex );
    }

    QVariant value = settings->value( key );
    if ( value.type() == QVariant::Invalid )
    {
      child->setText( 1, "Invalid" );
    }
    else
    {
      child->setText( 1, QVariant::typeToName( QgsVariantDelegate::type( value ) ) );
    }
    child->setText( 2, QgsVariantDelegate::displayText( value ) );
    child->setData( 2, Qt::UserRole, value );
  }

  while ( dividerIndex < childCount( parent ) )
    delete childAt( parent, dividerIndex );
}

QTreeWidgetItem *QgsSettingsTree::createItem( const QString &text,
    QTreeWidgetItem *parent, int index )
{
  QTreeWidgetItem *after = nullptr;
  if ( index != 0 )
    after = childAt( parent, index - 1 );

  QTreeWidgetItem *item;
  if ( parent )
    item = new QTreeWidgetItem( parent, after );
  else
    item = new QTreeWidgetItem( this, after );

  item->setText( 0, text );
  item->setFlags( item->flags() | Qt::ItemIsEditable );

  QString key = itemKey( item );
  QgsDebugMsg( key );
  if ( settingsMap.contains( key ) )
  {
    QgsDebugMsg( "contains!!!!" );
    QStringList values = settingsMap[ key ];
    item->setText( 3, values.at( 0 ) );
    item->setToolTip( 0, values.at( 1 ) );
    item->setToolTip( 2, values.at( 1 ) );
  }

  // if ( settingsMap.contains(

  return item;
}

QString QgsSettingsTree::itemKey( QTreeWidgetItem *item )
{
  if ( ! item )
    return QString();

  QString key = item->text( 0 );
  QTreeWidgetItem *ancestor = item->parent();
  while ( ancestor )
  {
    key.prepend( ancestor->text( 0 ) + '/' );
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
    if ( childAt( parent, i )->text( 0 ) == text )
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
