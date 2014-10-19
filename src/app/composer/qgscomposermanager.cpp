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

  mComposerListWidget->setItemDelegate( new QgsComposerNameDelegate( mComposerListWidget ) );

  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
  connect( QgisApp::instance(), SIGNAL( composerAdded( QgsComposerView* ) ), this, SLOT( refreshComposers() ) );
  connect( QgisApp::instance(), SIGNAL( composerRemoved( QgsComposerView* ) ), this, SLOT( refreshComposers() ) );

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

#ifdef Q_WS_MAC
  // Create action to select this window
  mWindowAction = new QAction( windowTitle(), this );
  connect( mWindowAction, SIGNAL( triggered() ), this, SLOT( activate() ) );
#endif

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

  refreshComposers();
}

QgsComposerManager::~QgsComposerManager()
{
  QSettings settings;
  settings.setValue( "/Windows/ComposerManager/geometry", saveGeometry() );
}

void QgsComposerManager::refreshComposers()
{
  QString selName = mComposerListWidget->currentItem() ? mComposerListWidget->currentItem()->text() : "";

  mItemComposerMap.clear();
  mComposerListWidget->clear();

  QSet<QgsComposer*> composers = QgisApp::instance()->printComposers();
  QSet<QgsComposer*>::const_iterator it = composers.constBegin();
  for ( ; it != composers.constEnd(); ++it )
  {
    QListWidgetItem* item = new QListWidgetItem(( *it )->title(), mComposerListWidget );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
    mItemComposerMap.insert( item, *it );
  }
  mComposerListWidget->sortItems();

  // Restore selection
  if ( !selName.isEmpty() )
  {
    QList<QListWidgetItem*> items = mComposerListWidget->findItems( selName, Qt::MatchExactly );
    if ( !items.isEmpty() )
    {
      mComposerListWidget->setCurrentItem( items.first() );
    }
  }
}

void QgsComposerManager::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
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

  if ( !loadedOK )
  {
    newComposer->close();
    QgisApp::instance()->deleteComposer( newComposer );
    newComposer = 0;
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

#ifdef Q_WS_MAC
void QgsComposerManager::showEvent( QShowEvent* event )
{
  if ( !event->spontaneous() )
  {
    QgisApp::instance()->addWindow( mWindowAction );
  }
}

void QgsComposerManager::changeEvent( QEvent* event )
{
  QDialog::changeEvent( event );
  switch ( event->type() )
  {
    case QEvent::ActivationChange:
      if ( QApplication::activeWindow() == this )
      {
        mWindowAction->setChecked( true );
      }
      break;

    default:
      break;
  }
}
#endif

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
    }
  }

  if ( c )
  {
    c->activate();
  }
#endif //0
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
    newComposer->activate();
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


//
// QgsComposerNameDelegate
//

QgsComposerNameDelegate::QgsComposerNameDelegate( QObject *parent )
    : QItemDelegate( parent )
{

}

QWidget *QgsComposerNameDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  //create a line edit
  QLineEdit *lineEdit = new QLineEdit( parent );
  return lineEdit;
}

void QgsComposerNameDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString text = index.model()->data( index, Qt::EditRole ).toString();
  QLineEdit *lineEdit = static_cast<QLineEdit*>( editor );
  lineEdit->setText( text );
}

void QgsComposerNameDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QLineEdit *lineEdit = static_cast<QLineEdit*>( editor );
  QString value = lineEdit->text();

  //has name changed?
  bool changed = model->data( index, Qt::EditRole ).toString() != value;

  //check if name already exists
  QStringList cNames;
  foreach ( QgsComposer* c, QgisApp::instance()->printComposers() )
  {
    cNames << c->title();
  }
  if ( changed && cNames.contains( value ) )
  {
    //name exists!
    QMessageBox::warning( 0, tr( "Rename composer" ), tr( "There is already a composer named \"%1\"" ).arg( value ) );
    return;
  }

  model->setData( index, QVariant( value ), Qt::EditRole );
}

void QgsComposerNameDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}
