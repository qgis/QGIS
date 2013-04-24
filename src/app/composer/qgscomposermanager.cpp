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
#include "qgsbusyindicatordialog.h"
#include "qgscomposer.h"
#include "qgscomposition.h"
#include "qgslogger.h"

#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QUrl>
#include <QSettings>

QgsComposerManager::QgsComposerManager( QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  QPushButton *pb;

  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/ComposerManager/geometry" ).toByteArray() );

  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( close() ) );

  pb = new QPushButton( tr( "&Show" ) );
  mButtonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( show_clicked() ) );

  pb = new QPushButton( tr( "&Duplicate" ) );
  mButtonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( duplicate_clicked() ) );

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
  QSettings settings;
  settings.setValue( "/Windows/ComposerManager/geometry", saveGeometry() );
}

void QgsComposerManager::initialize()
{
  QSettings settings;
  QSet<QgsComposer*> composers = QgisApp::instance()->printComposers();
  QSet<QgsComposer*>::const_iterator it = composers.constBegin();
  for ( ; it != composers.constEnd(); ++it )
  {
    QListWidgetItem* item = new QListWidgetItem(( *it )->title(), mComposerListWidget );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
    mItemComposerMap.insert( item, *it );
  }
  mComposerListWidget->sortItems();

  mTemplate->addItem( tr( "Empty composer" ) );
  mTemplate->addItem( tr( "Specific" ) );

  mUserTemplatesDir = QgsApplication::qgisSettingsDirPath() + "/composer_templates";
  QMap<QString, QString> userTemplateMap = defaultTemplates( true );
  if ( userTemplateMap.size() > 0 )
  {
    mTemplate->insertSeparator( mTemplate->count() );
    QMap<QString, QString>::const_iterator templateIt = userTemplateMap.constBegin();
    for ( ; templateIt != userTemplateMap.constEnd(); ++templateIt )
    {
      mTemplate->addItem( templateIt.key(), templateIt.value() );
    }
  }

  mDefaultTemplatesDir = QgsApplication::pkgDataPath() + "/composer_templates";
  QMap<QString, QString> defaultTemplateMap = defaultTemplates( false );
  if ( defaultTemplateMap.size() > 0 )
  {
    mTemplate->insertSeparator( mTemplate->count() );
    QMap<QString, QString>::const_iterator templateIt = defaultTemplateMap.constBegin();
    for ( ; templateIt != defaultTemplateMap.constEnd(); ++templateIt )
    {
      mTemplate->addItem( templateIt.key(), templateIt.value() );
    }
  }

  mTemplatePathLineEdit->setText( settings.value( "/UI/ComposerManager/templatePath", QString( "" ) ).toString() );
}

QMap<QString, QString> QgsComposerManager::defaultTemplates( bool fromUser ) const
{
  QMap<QString, QString> templateMap;

  //search for default templates in $pkgDataPath/composer_templates
  // user templates in $qgisSettingsDirPath/composer_templates
  QDir defaultTemplateDir( fromUser ? mUserTemplatesDir : mDefaultTemplatesDir );
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
  QFile templateFile;
  bool loadingTemplate = ( mTemplate->currentIndex() > 0 );
  if ( loadingTemplate )
  {
    if ( mTemplate->currentIndex() == 1 )
    {
      templateFile.setFileName( mTemplatePathLineEdit->text() );
    }
    else
    {
      templateFile.setFileName( mTemplate->itemData( mTemplate->currentIndex() ).toString() );
    }

    if ( !templateFile.exists() )
    {
      QMessageBox::warning( this, tr( "Template error" ), tr( "Error, template file not found" ) );
      return;
    }
    if ( !templateFile.open( QIODevice::ReadOnly ) )
    {
      QMessageBox::warning( this, tr( "Template error" ), tr( "Error, could not read file" ) );
      return;
    }
  }

  QgsComposer* newComposer = 0;
  bool loadedOK = false;

  QString title = QgisApp::instance()->uniqueComposerTitle( this, true );
  if ( title.isNull() )
  {
    return;
  }

  newComposer = QgisApp::instance()->createNewComposer( title );
  if ( !newComposer )
  {
    QMessageBox::warning( this, tr( "Composer error" ), tr( "Error, could not create composer" ) );
    return;
  }
  else
  {
    loadedOK = true;
  }

  if ( loadingTemplate )
  {
    QDomDocument templateDoc;
    if ( templateDoc.setContent( &templateFile, false ) )
    {
      // provide feedback, since composer will be hidden when loading template (much faster)
      // (not needed for empty composer)
      QDialog* dlg = new QgsBusyIndicatorDialog( tr( "Loading template into composer..." ) );
      dlg->setStyleSheet( QgisApp::instance()->styleSheet() );
      dlg->show();

      newComposer->hide();
      loadedOK = newComposer->composition()->loadFromTemplate( templateDoc, 0, false );
      newComposer->activate();

      dlg->close();
      delete dlg;
      dlg = 0;
    }
  }

  if ( loadedOK )
  {
    // do not close on Add, since user may want to add multiple composers from templates
    QListWidgetItem* item = new QListWidgetItem( newComposer->title(), mComposerListWidget );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
    mItemComposerMap.insert( item, newComposer );

    mComposerListWidget->sortItems();
    mComposerListWidget->setCurrentItem( item );
    mComposerListWidget->setFocus();
  }
  else
  {
    if ( newComposer )
    {
      newComposer->close();
      QgisApp::instance()->deleteComposer( newComposer );
      newComposer = 0;
    }
    QMessageBox::warning( this, tr( "Template error" ), tr( "Error, could not load template file" ) );
  }
}

void QgsComposerManager::on_mTemplate_currentIndexChanged( int indx )
{
  bool specific = ( indx == 1 ); // comes just after empty template
  mTemplatePathLineEdit->setEnabled( specific );
  mTemplatePathBtn->setEnabled( specific );
}

void QgsComposerManager::on_mTemplatePathBtn_pressed()
{
  QSettings settings;
  QString lastTmplDir = settings.value( "/UI/lastComposerTemplateDir", "." ).toString();
  QString tmplPath = QFileDialog::getOpenFileName( this,
                     tr( "Choose template" ),
                     lastTmplDir,
                     tr( "Composer templates" ) + " (*.qpt)" );
  if ( !tmplPath.isNull() )
  {
    mTemplatePathLineEdit->setText( tmplPath );
    settings.setValue( "UI/ComposerManager/templatePath", tmplPath );
    QFileInfo tmplFileInfo( tmplPath );
    settings.setValue( "UI/lastComposerTemplateDir", tmplFileInfo.absolutePath() );
  }
}

void QgsComposerManager::on_mTemplatesDefaultDirBtn_pressed()
{
  openLocalDirectory( mDefaultTemplatesDir );
}

void QgsComposerManager::on_mTemplatesUserDirBtn_pressed()
{
  openLocalDirectory( mUserTemplatesDir );
}

void QgsComposerManager::openLocalDirectory( const QString& localDirPath )
{
  QDir localDir;
  if ( !localDir.mkpath( localDirPath ) )
  {
    QMessageBox::warning( this, tr( "File system error" ), tr( "Error, could not open or create local directory" ) );
    return;
  }
  QDesktopServices::openUrl( QUrl::fromLocalFile( localDirPath ) );
}

void QgsComposerManager::remove_clicked()
{
  QListWidgetItem* item = mComposerListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  //ask for confirmation
  if ( QMessageBox::warning( this, tr( "Remove composer" ), tr( "Do you really want to remove the map composer '%1'?" ).arg( item->text() ), QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
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
        // extra activation steps for Windows
        bool shown = c->isVisible();
        hide();

        c->activate();

        // extra activation steps for Windows
        if ( !shown )
        {
          c->on_mActionZoomAll_triggered();
        }
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
    c->activate();
  }
#endif //0
  close();
}

void QgsComposerManager::duplicate_clicked()
{
  QListWidgetItem* item = mComposerListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  QgsComposer* currentComposer = 0;
  QString currentTitle;
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

  QString newTitle = QgisApp::instance()->uniqueComposerTitle( this, false, currentTitle + tr( " copy" ) );
  if ( newTitle.isNull() )
  {
    return;
  }

  // provide feedback, since loading of template into duplicate composer will be hidden
  QDialog* dlg = new QgsBusyIndicatorDialog( tr( "Duplicating composer..." ) );
  dlg->setStyleSheet( QgisApp::instance()->styleSheet() );
  dlg->show();

  QgsComposer* newComposer = QgisApp::instance()->duplicateComposer( currentComposer, newTitle );

  dlg->close();
  delete dlg;
  dlg = 0;

  if ( newComposer )
  {
    // extra activation steps for Windows
    hide();
    newComposer->activate();

    // no need to add new composer to list widget, if just closing this->exec();
    close();
  }
  else
  {
    QMessageBox::warning( this, tr( "Duplicate Composer" ),
                          tr( "Composer duplication failed." ) );
  }
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
  QString newTitle = QgisApp::instance()->uniqueComposerTitle( this, false, currentTitle );
  if ( newTitle.isNull() )
  {
    return;
  }
  currentComposer->setTitle( newTitle );
  item->setText( newTitle );

  mComposerListWidget->sortItems();
}

void QgsComposerManager::on_mComposerListWidget_itemChanged( QListWidgetItem * item )
{
  QMap<QListWidgetItem*, QgsComposer*>::iterator it = mItemComposerMap.find( item );
  if ( it != mItemComposerMap.end() )
  {
    it.value()->setTitle( item->text() );
  }
  mComposerListWidget->sortItems();
}
