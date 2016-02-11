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
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/ComposerManager/geometry" ).toByteArray() );

  mComposerListWidget->setItemDelegate( new QgsComposerNameDelegate( mComposerListWidget ) );

  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
  connect( QgisApp::instance(), SIGNAL( composerAdded( QgsComposerView* ) ), this, SLOT( refreshComposers() ) );
  connect( QgisApp::instance(), SIGNAL( composerRemoved( QgsComposerView* ) ), this, SLOT( refreshComposers() ) );

  connect( mComposerListWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( toggleButtons() ) );

  mShowButton = mButtonBox->addButton( tr( "&Show" ), QDialogButtonBox::ActionRole );
  connect( mShowButton, SIGNAL( clicked() ), this, SLOT( show_clicked() ) );

  mDuplicateButton = mButtonBox->addButton( tr( "&Duplicate" ), QDialogButtonBox::ActionRole );
  connect( mDuplicateButton, SIGNAL( clicked() ), this, SLOT( duplicate_clicked() ) );

  mRemoveButton = mButtonBox->addButton( tr( "&Remove" ), QDialogButtonBox::ActionRole );
  connect( mRemoveButton, SIGNAL( clicked() ), this, SLOT( remove_clicked() ) );

  mRenameButton = mButtonBox->addButton( tr( "Re&name" ), QDialogButtonBox::ActionRole );
  connect( mRenameButton, SIGNAL( clicked() ), this, SLOT( rename_clicked() ) );

#ifdef Q_OS_MAC
  // Create action to select this window
  mWindowAction = new QAction( windowTitle(), this );
  connect( mWindowAction, SIGNAL( triggered() ), this, SLOT( activate() ) );
#endif

  mTemplate->addItem( tr( "Empty composer" ) );
  mTemplate->addItem( tr( "Specific" ) );

  mUserTemplatesDir = QgsApplication::qgisSettingsDirPath() + "/composer_templates";
  QMap<QString, QString> userTemplateMap = defaultTemplates( true );
  this->addTemplates( userTemplateMap );

  mDefaultTemplatesDir = QgsApplication::pkgDataPath() + "/composer_templates";
  QMap<QString, QString> defaultTemplateMap = defaultTemplates( false );
  this->addTemplates( defaultTemplateMap );
  this->addTemplates( this->otherTemplates() );

  mTemplatePathLineEdit->setText( settings.value( "/UI/ComposerManager/templatePath", QString() ).toString() );

  refreshComposers();
}

QgsComposerManager::~QgsComposerManager()
{
  QSettings settings;
  settings.setValue( "/Windows/ComposerManager/geometry", saveGeometry() );
}

void QgsComposerManager::refreshComposers()
{
  // Backup selection
  QSet<QgsComposer *> selectedComposers;
  Q_FOREACH ( QListWidgetItem* item, mComposerListWidget->selectedItems() )
  {
    QMap<QListWidgetItem*, QgsComposer*>::const_iterator it = mItemComposerMap.constFind( item );
    if ( it != mItemComposerMap.constEnd() )
    {
      selectedComposers << it.value();
    }
  }

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
  bool selectionRestored = false;
  if ( !selectedComposers.isEmpty() )
  {
    QMap<QListWidgetItem*, QgsComposer*>::const_iterator i = mItemComposerMap.constBegin();
    while ( i != mItemComposerMap.constEnd() )
    {
      // This composer was selected: reselect it !
      if ( selectedComposers.contains( i.value() ) )
      {
        selectionRestored = true;
        int index = mComposerListWidget->row( i.key() );
        QModelIndex selectLine = mComposerListWidget->model()->index( index, 0, QModelIndex() );
        mComposerListWidget->selectionModel()->select( selectLine, QItemSelectionModel::Select );
      }
      ++i;
    }
  }
  // Select the first item by default
  if ( !selectionRestored && mComposerListWidget->count() > 0 )
  {
    QModelIndex firstLine = mComposerListWidget->model()->index( 0, 0, QModelIndex() );
    mComposerListWidget->selectionModel()->select( firstLine, QItemSelectionModel::Select );
  }

  // Update buttons
  toggleButtons();
}

void QgsComposerManager::toggleButtons()
{
  // Nothing selected: no button.
  if ( mComposerListWidget->selectedItems().isEmpty() )
  {
    mShowButton->setEnabled( false );
    mRemoveButton->setEnabled( false );
    mRenameButton->setEnabled( false );
    mDuplicateButton->setEnabled( false );
  }
  // toggle everything if one composer is selected
  else if ( mComposerListWidget->selectedItems().count() == 1 )
  {
    mShowButton->setEnabled( true );
    mRemoveButton->setEnabled( true );
    mRenameButton->setEnabled( true );
    mDuplicateButton->setEnabled( true );
  }
  // toggle only show and remove buttons in other cases
  else
  {
    mShowButton->setEnabled( true );
    mRemoveButton->setEnabled( true );
    mRenameButton->setEnabled( false );
    mDuplicateButton->setEnabled( false );
  }
}

void QgsComposerManager::addTemplates( const QMap<QString, QString>& templates )
{
  if ( !templates.isEmpty() )
  {
    mTemplate->insertSeparator( mTemplate->count() );
    QMap<QString, QString>::const_iterator templateIt = templates.constBegin();
    for ( ; templateIt != templates.constEnd(); ++templateIt )
    {
      mTemplate->addItem( templateIt.key(), templateIt.value() );
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
  //search for default templates in $pkgDataPath/composer_templates
  // user templates in $qgisSettingsDirPath/composer_templates
  return templatesFromPath( fromUser ? mUserTemplatesDir : mDefaultTemplatesDir );
}

QMap<QString, QString> QgsComposerManager::otherTemplates() const
{
  QMap<QString, QString> templateMap;
  QStringList paths = QgsApplication::composerTemplatePaths();
  Q_FOREACH ( const QString& path, paths )
  {
    QMap<QString, QString> templates = templatesFromPath( path );
    QMap<QString, QString>::const_iterator templateIt = templates.constBegin();
    for ( ; templateIt != templates.constEnd(); ++templateIt )
    {
      templateMap.insert( templateIt.key(), templateIt.value() );
    }
  }
  return templateMap;
}

QMap<QString, QString> QgsComposerManager::templatesFromPath( const QString& path ) const
{
  QMap<QString, QString> templateMap;

  QDir templateDir( path );
  if ( !templateDir.exists() )
  {
    return templateMap;
  }

  QFileInfoList fileInfoList = templateDir.entryInfoList( QDir::Files );
  QFileInfoList::const_iterator infoIt = fileInfoList.constBegin();
  for ( ; infoIt != fileInfoList.constEnd(); ++infoIt )
  {
    if ( infoIt->suffix().toLower() == "qpt" )
    {
      templateMap.insert( infoIt->baseName(), infoIt->absoluteFilePath() );
    }
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

  QgsComposer* newComposer = nullptr;
  bool loadedOK = false;

  QString title;
  if ( !QgisApp::instance()->uniqueComposerTitle( this, title, true ) )
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
      loadedOK = newComposer->composition()->loadFromTemplate( templateDoc, nullptr, false );
      newComposer->activate();

      dlg->close();
      delete dlg;
      dlg = nullptr;
    }
  }

  if ( !loadedOK )
  {
    newComposer->close();
    QgisApp::instance()->deleteComposer( newComposer );
    newComposer = nullptr;
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
  QString lastTmplDir = settings.value( "/UI/lastComposerTemplateDir", QDir::homePath() ).toString();
  QString tmplPath = QFileDialog::getOpenFileName( this,
                     tr( "Choose template" ),
                     lastTmplDir,
                     tr( "Composer templates" ) + " (*.qpt)" );
  if ( !tmplPath.isEmpty() )
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

#ifdef Q_OS_MAC
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
  QList<QgsComposer *> composerList;
  QList<QListWidgetItem *> composerItems = mComposerListWidget->selectedItems();
  QString title = tr( "Remove composers" );
  QString message = tr( "Do you really want to remove all selected map composers?" );

  if ( composerItems.isEmpty() )
  {
    return;
  }

  // Ask for confirmation
  if ( composerItems.count() == 1 )
  {
    title = tr( "Remove composer" );
    QListWidgetItem* uniqItem = composerItems.at( 0 );
    message = tr( "Do you really want to remove the map composer '%1'?" ).arg( uniqItem->text() );
  }

  if ( QMessageBox::warning( this, title, message, QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
  {
    return;
  }

  // Find the QgsComposers that need to be deleted
  Q_FOREACH ( QListWidgetItem* item, composerItems )
  {
    QMap<QListWidgetItem*, QgsComposer*>::const_iterator it = mItemComposerMap.constFind( item );
    if ( it != mItemComposerMap.constEnd() )
    {
      composerList << it.value();
    }
  }

  // Once we have the composer list, we can delete all of them !
  Q_FOREACH ( QgsComposer* c, composerList )
  {
    QgisApp::instance()->deleteComposer( c );
  }
}

void QgsComposerManager::show_clicked()
{
  Q_FOREACH ( QListWidgetItem* item, mComposerListWidget->selectedItems() )
  {
    QMap<QListWidgetItem*, QgsComposer*>::const_iterator it = mItemComposerMap.constFind( item );
    if ( it != mItemComposerMap.constEnd() )
    {
      QgsComposer* c = nullptr;
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
  }
}

void QgsComposerManager::duplicate_clicked()
{
  if ( mComposerListWidget->selectedItems().isEmpty() )
  {
    return;
  }

  QgsComposer* currentComposer = nullptr;
  QString currentTitle;

  QListWidgetItem* item = mComposerListWidget->selectedItems().at( 0 );
  QMap<QListWidgetItem*, QgsComposer*>::const_iterator it = mItemComposerMap.constFind( item );
  if ( it != mItemComposerMap.constEnd() )
  {
    currentComposer = it.value();
    currentTitle = it.value()->title();
  }
  else
  {
    return;
  }

  QString newTitle;
  if ( !QgisApp::instance()->uniqueComposerTitle( this, newTitle, false, currentTitle + tr( " copy" ) ) )
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
  dlg = nullptr;

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
  if ( mComposerListWidget->selectedItems().isEmpty() )
  {
    return;
  }

  QString currentTitle;
  QgsComposer* currentComposer = nullptr;

  QListWidgetItem* item = mComposerListWidget->selectedItems().at( 0 );
  QMap<QListWidgetItem*, QgsComposer*>::const_iterator it = mItemComposerMap.constFind( item );
  if ( it != mItemComposerMap.constEnd() )
  {
    currentComposer = it.value();
    currentTitle = it.value()->title();
  }
  else
  {
    return;
  }

  QString newTitle;
  if ( !QgisApp::instance()->uniqueComposerTitle( this, newTitle, false, currentTitle ) )
  {
    return;
  }
  currentComposer->setTitle( newTitle );
  item->setText( newTitle );

  mComposerListWidget->sortItems();
}

void QgsComposerManager::on_mComposerListWidget_itemChanged( QListWidgetItem * item )
{
  QMap<QListWidgetItem*, QgsComposer*>::const_iterator it = mItemComposerMap.constFind( item );
  if ( it != mItemComposerMap.constEnd() )
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
  Q_FOREACH ( QgsComposer* c, QgisApp::instance()->printComposers() )
  {
    cNames << c->title();
  }
  if ( changed && cNames.contains( value ) )
  {
    //name exists!
    QMessageBox::warning( nullptr, tr( "Rename composer" ), tr( "There is already a composer named \"%1\"" ).arg( value ) );
    return;
  }

  model->setData( index, QVariant( value ), Qt::EditRole );
}

void QgsComposerNameDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}
