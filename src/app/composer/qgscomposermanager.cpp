/***************************************************************************
                              qgscomposermanager.cpp
                             ------------------------
    begin                : September 11 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposermanager.h"
#include "qgisapp.h"
#include "qgscomposer.h"
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>

QgsComposerManager::QgsComposerManager( QgisApp* app, QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f ), mQgisApp( app )
{
  setupUi( this );
  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
  initialize();
}

QgsComposerManager::~QgsComposerManager()
{

}

void QgsComposerManager::initialize()
{
  if ( !mQgisApp )
  {
    return;
  }

  QSet<QgsComposer*> composers = mQgisApp->printComposers();
  QSet<QgsComposer*>::const_iterator it = composers.constBegin();
  for ( ; it != composers.constEnd(); ++it )
  {
    QListWidgetItem* item = new QListWidgetItem(( *it )->title(), mComposerListWidget );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
    mItemComposerMap.insert( item, *it );
  }
}

void QgsComposerManager::on_mAddButton_clicked()
{
  if ( !mQgisApp )
  {
    return;
  }
  QgsComposer* newComposer = mQgisApp->createNewComposer();
  if ( !newComposer )
  {
    return;
  }
  QListWidgetItem* item = new QListWidgetItem( newComposer->title(), mComposerListWidget );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mItemComposerMap.insert( item, newComposer );
}

void QgsComposerManager::on_mRemoveButton_clicked()
{
  if ( !mQgisApp )
  {
    return;
  }

  QListWidgetItem* item = mComposerListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  //ask for confirmation
  if ( QMessageBox::warning( 0, tr( "Remove composer" ), tr( "Do you really want to remove the map composer '%1'?" ).arg( item->text() ), QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
  {
    return;
  }

  //delete composer
  QMap<QListWidgetItem*, QgsComposer*>::iterator it = mItemComposerMap.find( item );
  if ( it != mItemComposerMap.end() )
  {
    mQgisApp->deleteComposer( it.value() );
  }
  mItemComposerMap.remove( item );
  mComposerListWidget->removeItemWidget( item );
  //and remove the list widget row
  delete( mComposerListWidget->takeItem( mComposerListWidget->row( item ) ) );
}

void QgsComposerManager::on_mShowPushButton_clicked()
{
  if ( !mQgisApp )
  {
    return;
  }

  QListWidgetItem* item = mComposerListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  //delete composer
  QMap<QListWidgetItem*, QgsComposer*>::iterator it = mItemComposerMap.find( item );
  if ( it != mItemComposerMap.end() )
  {
    it.value()->show();
    it.value()->activate();
    it.value()->stackUnder( this );
  }
}

void QgsComposerManager::on_mRenamePushButton_clicked()
{
  QListWidgetItem* item = mComposerListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  QString currentTitle;
  QgsComposer* currentComposer = 0;
  QMap<QListWidgetItem*, QgsComposer*>::iterator it = mItemComposerMap.find( item );
  if ( it != mItemComposerMap.end() )
  {
    currentComposer = it.value();
    currentTitle = it.value()->title();
  }
  else
  {
    return;
  }
  QString newTitle = QInputDialog::getText( 0, tr( "Change title" ), tr( "Title" ), QLineEdit::Normal, currentTitle );
  if ( newTitle.isNull() )
  {
    return;
  }
  currentComposer->setTitle( newTitle );
  item->setText( newTitle );
}

void QgsComposerManager::on_mComposerListWidget_itemChanged( QListWidgetItem * item )
{
  QMap<QListWidgetItem*, QgsComposer*>::iterator it = mItemComposerMap.find( item );
  if ( it != mItemComposerMap.end() )
  {
    it.value()->setTitle( item->text() );
  }
}
