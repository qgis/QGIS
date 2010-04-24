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
#include "qgsapplication.h"
#include "qgscomposer.h"
#include "qgslogger.h"
#include <QDir>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>

QgsComposerManager::QgsComposerManager( QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  QPushButton *pb;

  setupUi( this );
  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( close() ) );

  pb = new QPushButton( tr( "&Show" ) );
  mButtonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( show_clicked() ) );

  pb = new QPushButton( tr( "&Remove" ) );
  mButtonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( remove_clicked() ) );

  pb = new QPushButton( tr( "Re&name" ) );
  mButtonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( rename_clicked() ) );

  initialize();
}

QgsComposerManager::~QgsComposerManager()
{

}

void QgsComposerManager::initialize()
{
  QSet<QgsComposer*> composers = QgisApp::instance()->printComposers();
  QSet<QgsComposer*>::const_iterator it = composers.constBegin();
  for ( ; it != composers.constEnd(); ++it )
  {
    QListWidgetItem* item = new QListWidgetItem(( *it )->title(), mComposerListWidget );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
    mItemComposerMap.insert( item, *it );
  }

  mTemplate->addItem( tr( "Empty composer" ) );

  QMap<QString, QString> templateMap = defaultTemplates();
  if ( templateMap.size() > 0 )
  {
    QMap<QString, QString>::const_iterator templateIt = templateMap.constBegin();
    for ( ; templateIt != templateMap.constEnd(); ++templateIt )
    {
      mTemplate->addItem( templateIt.key(), templateIt.value() );
    }
  }
}

QMap<QString, QString> QgsComposerManager::defaultTemplates() const
{
  QMap<QString, QString> templateMap;

  //search for default templates in $pkgDataPath/composer_templates
  QDir defaultTemplateDir( QgsApplication::pkgDataPath() + "/composer_templates" );
  if ( !defaultTemplateDir.exists() )
  {
    return templateMap;
  }

  QFileInfoList fileInfoList = defaultTemplateDir.entryInfoList( QDir::Files );
  QFileInfoList::const_iterator infoIt = fileInfoList.constBegin();
  for ( ; infoIt != fileInfoList.constEnd(); ++infoIt )
  {
    templateMap.insert( infoIt->baseName(), infoIt->absoluteFilePath() );
  }
  return templateMap;
}

void QgsComposerManager::on_mAddButton_clicked()
{
  QgsComposer* newComposer = 0;

  newComposer = QgisApp::instance()->createNewComposer();
  if ( !newComposer )
  {
    return;
  }

  if ( mTemplate->currentIndex() > 0 )
  {
    QDomDocument templateDoc;
    QFile templateFile( mTemplate->itemData( mTemplate->currentIndex() ).toString() );
    if ( templateFile.open( QIODevice::ReadOnly ) )
    {
      if ( templateDoc.setContent( &templateFile, false ) )
      {
        newComposer->readXML( templateDoc );
      }
    }
  }

  QListWidgetItem* item = new QListWidgetItem( newComposer->title(), mComposerListWidget );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mItemComposerMap.insert( item, newComposer );
}

void QgsComposerManager::remove_clicked()
{
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
    QgisApp::instance()->deleteComposer( it.value() );
  }
  mItemComposerMap.remove( item );
  mComposerListWidget->removeItemWidget( item );
  //and remove the list widget row
  delete( mComposerListWidget->takeItem( mComposerListWidget->row( item ) ) );
}

void QgsComposerManager::show_clicked()
{
  QListWidgetItem* item = mComposerListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  QMap<QListWidgetItem*, QgsComposer*>::iterator it = mItemComposerMap.find( item );
  if ( it != mItemComposerMap.end() )
  {
    QgsComposer* c = 0;
    if ( it.value() ) //a normal composer
    {
      c = it.value();
      if ( c )
      {
        c->show();
        c->activate();
        c->stackUnder( this );
        raise();
        activateWindow();
      }
    }
  }
#if 0
  else //create composer from default template
  {
    QMap<QString, QString>::const_iterator templateIt = mDefaultTemplateMap.find( it.key()->text() );
    if ( templateIt == mDefaultTemplateMap.constEnd() )
    {
      return;
    }

    QDomDocument templateDoc;
    QFile templateFile( templateIt.value() );
    if ( !templateFile.open( QIODevice::ReadOnly ) )
    {
      return;
    }

    if ( !templateDoc.setContent( &templateFile, false ) )
    {
      return;
    }
    c = QgisApp::instance()->createNewComposer();
    c->setTitle( it.key()->text() );
    if ( c )
    {
      c->readXML( templateDoc );
      mItemComposerMap.insert( it.key(), c );
    }
  }

  if ( c )
  {
    c->show();
    c->activate();
    c->stackUnder( this );
    raise();
    activateWindow();
  }
#endif //0
}

void QgsComposerManager::rename_clicked()
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
