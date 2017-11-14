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
#include "qgssettings.h"
#include "qgscomposerview.h"
#include "qgslayoutmanager.h"
#include "qgsproject.h"

#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QUrl>

QgsComposerManager::QgsComposerManager( QWidget *parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  connect( mAddButton, &QPushButton::clicked, this, &QgsComposerManager::mAddButton_clicked );
  connect( mTemplate, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerManager::mTemplate_currentIndexChanged );
  connect( mTemplatePathBtn, &QPushButton::pressed, this, &QgsComposerManager::mTemplatePathBtn_pressed );
  connect( mTemplatesDefaultDirBtn, &QPushButton::pressed, this, &QgsComposerManager::mTemplatesDefaultDirBtn_pressed );
  connect( mTemplatesUserDirBtn, &QPushButton::pressed, this, &QgsComposerManager::mTemplatesUserDirBtn_pressed );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/ComposerManager/geometry" ) ).toByteArray() );

  mModel = new QgsLayoutManagerModel( QgsProject::instance()->layoutManager(),
                                      this );
  mComposerListView->setModel( mModel );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QWidget::close );
  connect( mComposerListView->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &QgsComposerManager::toggleButtons );

  mShowButton = mButtonBox->addButton( tr( "&Show" ), QDialogButtonBox::ActionRole );
  connect( mShowButton, &QAbstractButton::clicked, this, &QgsComposerManager::showClicked );

  mDuplicateButton = mButtonBox->addButton( tr( "&Duplicate" ), QDialogButtonBox::ActionRole );
  connect( mDuplicateButton, &QAbstractButton::clicked, this, &QgsComposerManager::duplicateClicked );

  mRemoveButton = mButtonBox->addButton( tr( "&Remove" ), QDialogButtonBox::ActionRole );
  connect( mRemoveButton, &QAbstractButton::clicked, this, &QgsComposerManager::removeClicked );

  mRenameButton = mButtonBox->addButton( tr( "Re&name" ), QDialogButtonBox::ActionRole );
  connect( mRenameButton, &QAbstractButton::clicked, this, &QgsComposerManager::renameClicked );

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

  mTemplatePathLineEdit->setText( settings.value( QStringLiteral( "UI/ComposerManager/templatePath" ), QString() ).toString() );
}

QgsComposerManager::~QgsComposerManager()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/ComposerManager/geometry" ), saveGeometry() );
}

void QgsComposerManager::toggleButtons()
{
  // Nothing selected: no button.
  if ( mComposerListView->selectionModel()->selectedRows().isEmpty() )
  {
    mShowButton->setEnabled( false );
    mRemoveButton->setEnabled( false );
    mRenameButton->setEnabled( false );
    mDuplicateButton->setEnabled( false );
  }
  // toggle everything if one composer is selected
  else if ( mComposerListView->selectionModel()->selectedRows().count() == 1 )
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

void QgsComposerManager::addTemplates( const QMap<QString, QString> &templates )
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
  Q_FOREACH ( const QString &path, paths )
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

QMap<QString, QString> QgsComposerManager::templatesFromPath( const QString &path ) const
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
    if ( infoIt->suffix().toLower() == QLatin1String( "qpt" ) )
    {
      templateMap.insert( infoIt->baseName(), infoIt->absoluteFilePath() );
    }
  }
  return templateMap;
}

void QgsComposerManager::mAddButton_clicked()
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
      templateFile.setFileName( mTemplate->currentData().toString() );
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

  QgsComposer *newComposer = nullptr;
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
      loadedOK = newComposer->loadFromTemplate( templateDoc, true );
      newComposer->activate();
    }
  }

  if ( !loadedOK )
  {
    QgisApp::instance()->deleteComposer( newComposer );
    newComposer = nullptr;
    QMessageBox::warning( this, tr( "Template error" ), tr( "Error, could not load template file" ) );
  }
}

void QgsComposerManager::mTemplate_currentIndexChanged( int indx )
{
  bool specific = ( indx == 1 ); // comes just after empty template
  mTemplatePathLineEdit->setEnabled( specific );
  mTemplatePathBtn->setEnabled( specific );
}

void QgsComposerManager::mTemplatePathBtn_pressed()
{
  QgsSettings settings;
  QString lastTmplDir = settings.value( QStringLiteral( "UI/lastComposerTemplateDir" ), QDir::homePath() ).toString();
  QString tmplPath = QFileDialog::getOpenFileName( this,
                     tr( "Choose template" ),
                     lastTmplDir,
                     tr( "Composer templates" ) + " (*.qpt)" );
  if ( !tmplPath.isEmpty() )
  {
    mTemplatePathLineEdit->setText( tmplPath );
    settings.setValue( QStringLiteral( "UI/ComposerManager/templatePath" ), tmplPath );
    QFileInfo tmplFileInfo( tmplPath );
    settings.setValue( QStringLiteral( "UI/lastComposerTemplateDir" ), tmplFileInfo.absolutePath() );
  }
}

void QgsComposerManager::mTemplatesDefaultDirBtn_pressed()
{
  openLocalDirectory( mDefaultTemplatesDir );
}

void QgsComposerManager::mTemplatesUserDirBtn_pressed()
{
  openLocalDirectory( mUserTemplatesDir );
}

void QgsComposerManager::openLocalDirectory( const QString &localDirPath )
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
void QgsComposerManager::showEvent( QShowEvent *event )
{
  if ( !event->spontaneous() )
  {
    QgisApp::instance()->addWindow( mWindowAction );
  }
}

void QgsComposerManager::changeEvent( QEvent *event )
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

void QgsComposerManager::removeClicked()
{
  QModelIndexList composerItems = mComposerListView->selectionModel()->selectedRows();
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
    message = tr( "Do you really want to remove the map composer '%1'?" ).arg(
                mComposerListView->model()->data( composerItems.at( 0 ), Qt::DisplayRole ).toString() );
  }

  if ( QMessageBox::warning( this, title, message, QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
  {
    return;
  }

  QList<QgsComposition *> composerList;
  // Find the QgsComposers that need to be deleted
  Q_FOREACH ( const QModelIndex &index, composerItems )
  {
    QgsComposition *c = mModel->compositionFromIndex( index );
    if ( c )
    {
      composerList << c;
    }
  }

  // Once we have the composer list, we can delete all of them !
  Q_FOREACH ( QgsComposition *c, composerList )
  {
    QgsProject::instance()->layoutManager()->removeComposition( c );
  }
}

void QgsComposerManager::showClicked()
{
  Q_FOREACH ( const QModelIndex &index, mComposerListView->selectionModel()->selectedRows() )
  {
    QgsComposition *c = mModel->compositionFromIndex( index );
    if ( c )
    {
      QgisApp::instance()->openComposer( c );
    }
  }
}

void QgsComposerManager::duplicateClicked()
{
  if ( mComposerListView->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }

  QgsComposition *currentComposer = mModel->compositionFromIndex( mComposerListView->selectionModel()->selectedRows().at( 0 ) );
  if ( !currentComposer )
    return;
  QString currentTitle = currentComposer->name();

  QString newTitle;
  if ( !QgisApp::instance()->uniqueComposerTitle( this, newTitle, false, currentTitle + tr( " copy" ) ) )
  {
    return;
  }

  // provide feedback, since loading of template into duplicate composer will be hidden
  QDialog *dlg = new QgsBusyIndicatorDialog( tr( "Duplicating composer..." ) );
  dlg->setStyleSheet( QgisApp::instance()->styleSheet() );
  dlg->show();

  QgsComposition *newComposition = QgsProject::instance()->layoutManager()->duplicateComposition( currentComposer->name(),
                                   newTitle );
  QgsComposer *newComposer = QgisApp::instance()->openComposer( newComposition );
  dlg->close();
  delete dlg;
  dlg = nullptr;
  if ( !newComposer )
  {
    QMessageBox::warning( this, tr( "Duplicate Composer" ),
                          tr( "Composer duplication failed." ) );
  }
  else
  {
    newComposer->activate();
  }
}

void QgsComposerManager::renameClicked()
{
  if ( mComposerListView->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }

  QgsComposition *currentComposer = mModel->compositionFromIndex( mComposerListView->selectionModel()->selectedRows().at( 0 ) );
  if ( !currentComposer )
    return;

  QString currentTitle = currentComposer->name();
  QString newTitle;
  if ( !QgisApp::instance()->uniqueComposerTitle( this, newTitle, false, currentTitle ) )
  {
    return;
  }
  currentComposer->setName( newTitle );
}

//
// QgsLayoutManagerModel
//

QgsLayoutManagerModel::QgsLayoutManagerModel( QgsLayoutManager *manager, QObject *parent )
  : QAbstractListModel( parent )
  , mLayoutManager( manager )
{
  connect( mLayoutManager, &QgsLayoutManager::compositionAboutToBeAdded, this, &QgsLayoutManagerModel::compositionAboutToBeAdded );
  connect( mLayoutManager, &QgsLayoutManager::compositionAdded, this, &QgsLayoutManagerModel::compositionAdded );
  connect( mLayoutManager, &QgsLayoutManager::compositionAboutToBeRemoved, this, &QgsLayoutManagerModel::compositionAboutToBeRemoved );
  connect( mLayoutManager, &QgsLayoutManager::compositionRemoved, this, &QgsLayoutManagerModel::compositionRemoved );
  connect( mLayoutManager, &QgsLayoutManager::compositionRenamed, this, &QgsLayoutManagerModel::compositionRenamed );
}

int QgsLayoutManagerModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mLayoutManager->compositions().count();
}

QVariant QgsLayoutManagerModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
      return mLayoutManager->compositions().at( index.row() )->name();

    case CompositionRole:
      return QVariant::fromValue( mLayoutManager->compositions().at( index.row() ) );

    default:
      return QVariant();
  }
}

bool QgsLayoutManagerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
  {
    return false;
  }
  if ( index.row() >= mLayoutManager->compositions().count() )
  {
    return false;
  }

  if ( value.toString().isEmpty() )
    return false;

  QgsComposition *c = compositionFromIndex( index );
  if ( !c )
    return false;

  //has name changed?
  bool changed = c->name() != value.toString();
  if ( !changed )
    return true;

  //check if name already exists
  QStringList cNames;
  Q_FOREACH ( QgsComposition *comp, QgsProject::instance()->layoutManager()->compositions() )
  {
    cNames << comp->name();
  }
  if ( cNames.contains( value.toString() ) )
  {
    //name exists!
    QMessageBox::warning( nullptr, tr( "Rename composer" ), tr( "There is already a composer named \"%1\"" ).arg( value.toString() ) );
    return false;
  }

  c->setName( value.toString() );
  return true;
}

Qt::ItemFlags QgsLayoutManagerModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractListModel::flags( index );

  if ( index.isValid() )
  {
    return flags | Qt::ItemIsEditable;
  }
  else
  {
    return flags;
  }
}

QgsComposition *QgsLayoutManagerModel::compositionFromIndex( const QModelIndex &index ) const
{
  return qobject_cast< QgsComposition * >( qvariant_cast<QObject *>( data( index, CompositionRole ) ) );
}

void QgsLayoutManagerModel::compositionAboutToBeAdded( const QString & )
{
  int row = mLayoutManager->compositions().count();
  beginInsertRows( QModelIndex(), row, row );
}

void QgsLayoutManagerModel::compositionAboutToBeRemoved( const QString &name )
{
  QgsComposition *c = mLayoutManager->compositionByName( name );
  int row = mLayoutManager->compositions().indexOf( c );
  if ( row >= 0 )
    beginRemoveRows( QModelIndex(), row, row );
}

void QgsLayoutManagerModel::compositionAdded( const QString & )
{
  endInsertRows();
}

void QgsLayoutManagerModel::compositionRemoved( const QString & )
{
  endRemoveRows();
}

void QgsLayoutManagerModel::compositionRenamed( QgsComposition *composition, const QString & )
{
  int row = mLayoutManager->compositions().indexOf( composition );
  QModelIndex index = createIndex( row, 0 );
  emit dataChanged( index, index, QVector<int>() << Qt::DisplayRole );
}
