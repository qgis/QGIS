/***************************************************************************
                         qgslayoutmanagerdialog.cpp
                         -----------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutmanagerdialog.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsbusyindicatordialog.h"
#include "qgslayoutdesignerdialog.h"
#include "qgslayout.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgslayoutview.h"
#include "qgslayoutmanager.h"
#include "qgsproject.h"
#include "qgsgui.h"

#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QUrl>

QgsLayoutManagerDialog::QgsLayoutManagerDialog( QWidget *parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  connect( mAddButton, &QPushButton::clicked, this, &QgsLayoutManagerDialog::mAddButton_clicked );
  connect( mTemplate, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutManagerDialog::mTemplate_currentIndexChanged );
  connect( mTemplatesDefaultDirBtn, &QPushButton::pressed, this, &QgsLayoutManagerDialog::mTemplatesDefaultDirBtn_pressed );
  connect( mTemplatesUserDirBtn, &QPushButton::pressed, this, &QgsLayoutManagerDialog::mTemplatesUserDirBtn_pressed );

  QgsGui::instance()->enableAutoGeometryRestore( this );
  mTemplateFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mTemplateFileWidget->setFilter( tr( "Layout templates" ) + QStringLiteral( " (*.qpt *.QPT)" ) );
  mTemplateFileWidget->setDialogTitle( tr( "Select a Template" ) );
  mTemplateFileWidget->lineEdit()->setShowClearButton( false );
  QgsSettings settings;
  mTemplateFileWidget->setDefaultRoot( settings.value( QStringLiteral( "UI/lastComposerTemplateDir" ), QString() ).toString() );
  mTemplateFileWidget->setFilePath( settings.value( QStringLiteral( "UI/ComposerManager/templatePath" ), QString() ).toString() );

  connect( mTemplateFileWidget, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "UI/ComposerManager/templatePath" ), mTemplateFileWidget->filePath() );
    QFileInfo tmplFileInfo( mTemplateFileWidget->filePath() );
    settings.setValue( QStringLiteral( "UI/lastComposerTemplateDir" ), tmplFileInfo.absolutePath() );
  } );

  mModel = new QgsLayoutManagerModel( QgsProject::instance()->layoutManager(),
                                      this );
  mLayoutListView->setModel( mModel );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QWidget::close );
  connect( mLayoutListView->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &QgsLayoutManagerDialog::toggleButtons );

  mShowButton = mButtonBox->addButton( tr( "&Show" ), QDialogButtonBox::ActionRole );
  connect( mShowButton, &QAbstractButton::clicked, this, &QgsLayoutManagerDialog::showClicked );

  mDuplicateButton = mButtonBox->addButton( tr( "&Duplicate" ), QDialogButtonBox::ActionRole );
  connect( mDuplicateButton, &QAbstractButton::clicked, this, &QgsLayoutManagerDialog::duplicateClicked );

  mRemoveButton = mButtonBox->addButton( tr( "&Remove" ), QDialogButtonBox::ActionRole );
  connect( mRemoveButton, &QAbstractButton::clicked, this, &QgsLayoutManagerDialog::removeClicked );

  mRenameButton = mButtonBox->addButton( tr( "Re&name" ), QDialogButtonBox::ActionRole );
  connect( mRenameButton, &QAbstractButton::clicked, this, &QgsLayoutManagerDialog::renameClicked );

#ifdef Q_OS_MAC
  // Create action to select this window
  mWindowAction = new QAction( windowTitle(), this );
  connect( mWindowAction, SIGNAL( triggered() ), this, SLOT( activate() ) );
#endif

  mTemplate->addItem( tr( "Empty layout" ) );
  mTemplate->addItem( tr( "Specific" ) );

  mUserTemplatesDir = QgsApplication::qgisSettingsDirPath() + "/composer_templates";
  QMap<QString, QString> userTemplateMap = defaultTemplates( true );
  this->addTemplates( userTemplateMap );

  mDefaultTemplatesDir = QgsApplication::pkgDataPath() + "/composer_templates";
  QMap<QString, QString> defaultTemplateMap = defaultTemplates( false );
  this->addTemplates( defaultTemplateMap );
  this->addTemplates( this->otherTemplates() );

  toggleButtons();
}

void QgsLayoutManagerDialog::toggleButtons()
{
  // Nothing selected: no button.
  if ( mLayoutListView->selectionModel()->selectedRows().isEmpty() )
  {
    mShowButton->setEnabled( false );
    mRemoveButton->setEnabled( false );
    mRenameButton->setEnabled( false );
    mDuplicateButton->setEnabled( false );
  }
  // toggle everything if one layout is selected
  else if ( mLayoutListView->selectionModel()->selectedRows().count() == 1 )
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

void QgsLayoutManagerDialog::addTemplates( const QMap<QString, QString> &templates )
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

void QgsLayoutManagerDialog::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
}

QMap<QString, QString> QgsLayoutManagerDialog::defaultTemplates( bool fromUser ) const
{
  //search for default templates in $pkgDataPath/composer_templates
  // user templates in $qgisSettingsDirPath/composer_templates
  return templatesFromPath( fromUser ? mUserTemplatesDir : mDefaultTemplatesDir );
}

QMap<QString, QString> QgsLayoutManagerDialog::otherTemplates() const
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

QMap<QString, QString> QgsLayoutManagerDialog::templatesFromPath( const QString &path ) const
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

void QgsLayoutManagerDialog::mAddButton_clicked()
{
  QFile templateFile;
  bool loadingTemplate = ( mTemplate->currentIndex() > 0 );
  if ( loadingTemplate )
  {
    if ( mTemplate->currentIndex() == 1 )
    {
      templateFile.setFileName( mTemplateFileWidget->filePath() );
    }
    else
    {
      templateFile.setFileName( mTemplate->currentData().toString() );
    }

    if ( !templateFile.exists() )
    {
      QMessageBox::warning( this, tr( "Create layout" ), tr( "Template file “%1” not found." ).arg( templateFile.fileName() ) );
      return;
    }
    if ( !templateFile.open( QIODevice::ReadOnly ) )
    {
      QMessageBox::warning( this, tr( "Create layout" ), tr( "Could not read template file “%1”." ).arg( templateFile.fileName() ) );
      return;
    }
  }

  QString title;
  if ( !QgisApp::instance()->uniqueLayoutTitle( this, title, true ) )
  {
    return;
  }

  if ( title.isEmpty() )
  {
    title = QgsProject::instance()->layoutManager()->generateUniqueTitle();
  }

  std::unique_ptr< QgsLayout > layout = qgis::make_unique< QgsLayout >( QgsProject::instance() );
  if ( loadingTemplate )
  {
    QDomDocument templateDoc;
    if ( templateDoc.setContent( &templateFile, false ) )
    {
      bool loadedOK = false;
      ( void )layout->loadFromTemplate( templateDoc, QgsReadWriteContext(), true, &loadedOK );
      if ( !loadedOK )
      {
        QMessageBox::warning( this, tr( "Create layout" ), tr( "Invalid template file “%1”." ).arg( templateFile.fileName() ) );
        layout.reset();
      }
    }
  }
  else
  {
    layout->initializeDefaults();
  }

  if ( layout )
  {
    layout->setName( title );
    QgisApp::instance()->openLayoutDesignerDialog( layout.get() );
    QgsProject::instance()->layoutManager()->addLayout( layout.release() );
  }
}

void QgsLayoutManagerDialog::mTemplate_currentIndexChanged( int indx )
{
  bool specific = ( indx == 1 ); // comes just after empty template
  mTemplateFileWidget->setEnabled( specific );
}

void QgsLayoutManagerDialog::mTemplatesDefaultDirBtn_pressed()
{
  openLocalDirectory( mDefaultTemplatesDir );
}

void QgsLayoutManagerDialog::mTemplatesUserDirBtn_pressed()
{
  openLocalDirectory( mUserTemplatesDir );
}

void QgsLayoutManagerDialog::openLocalDirectory( const QString &localDirPath )
{
  QDir localDir;
  if ( !localDir.mkpath( localDirPath ) )
  {
    QMessageBox::warning( this, tr( "Open directory" ), tr( "Could not open or create local directory “%1”." ).arg( localDirPath ) );
  }
  else
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( localDirPath ) );
  }
}

#ifdef Q_OS_MAC
void QgsLayoutManagerDialog::showEvent( QShowEvent *event )
{
  if ( !event->spontaneous() )
  {
    QgisApp::instance()->addWindow( mWindowAction );
  }
}

void QgsLayoutManagerDialog::changeEvent( QEvent *event )
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

void QgsLayoutManagerDialog::removeClicked()
{
  const QModelIndexList layoutItems = mLayoutListView->selectionModel()->selectedRows();
  if ( layoutItems.isEmpty() )
  {
    return;
  }

  QString title;
  QString message;
  if ( layoutItems.count() == 1 )
  {
    title = tr( "Remove layout" );
    message = tr( "Do you really want to remove the print layout “%1”?" ).arg(
                mLayoutListView->model()->data( layoutItems.at( 0 ), Qt::DisplayRole ).toString() );
  }
  else
  {
    title = tr( "Remove layouts" );
    message = tr( "Do you really want to remove all selected print layouts?" );
  }

  if ( QMessageBox::warning( this, title, message, QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
  {
    return;
  }

  QList<QgsLayout *> layoutList;
  // Find the layouts that need to be deleted
  for ( const QModelIndex &index : layoutItems )
  {
    QgsLayout *l = mModel->layoutFromIndex( index );
    if ( l )
    {
      layoutList << l;
    }
  }

  // Once we have the layout list, we can delete all of them !
  for ( QgsLayout *l : qgis::as_const( layoutList ) )
  {
    QgsProject::instance()->layoutManager()->removeLayout( l );
  }
}

void QgsLayoutManagerDialog::showClicked()
{
  const QModelIndexList layoutItems = mLayoutListView->selectionModel()->selectedRows();
  for ( const QModelIndex &index : layoutItems )
  {
    if ( QgsLayout *l = mModel->layoutFromIndex( index ) )
    {
      QgisApp::instance()->openLayoutDesignerDialog( l );
    }
  }
}

void QgsLayoutManagerDialog::duplicateClicked()
{
  if ( mLayoutListView->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }

  QgsLayout *currentLayout = mModel->layoutFromIndex( mLayoutListView->selectionModel()->selectedRows().at( 0 ) );
  if ( !currentLayout )
    return;
  QString currentTitle = currentLayout->name();

  QString newTitle;
  if ( !QgisApp::instance()->uniqueLayoutTitle( this, newTitle, false, tr( "%1 copy" ).arg( currentTitle ) ) )
  {
    return;
  }

  // provide feedback, since loading of template into duplicate layout will be hidden
  QDialog *dlg = new QgsBusyIndicatorDialog( tr( "Duplicating layout…" ) );
  dlg->setStyleSheet( QgisApp::instance()->styleSheet() );
  dlg->show();

  QgsLayoutDesignerDialog *newDialog = QgisApp::instance()->duplicateLayout( currentLayout, newTitle );

  dlg->close();
  delete dlg;
  dlg = nullptr;

  if ( !newDialog )
  {
    QMessageBox::warning( this, tr( "Duplicate layout" ),
                          tr( "Layout duplication failed." ) );
  }
  else
  {
    newDialog->activate();
  }
}

void QgsLayoutManagerDialog::renameClicked()
{
  if ( mLayoutListView->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }

  QgsLayout *currentLayout = mModel->layoutFromIndex( mLayoutListView->selectionModel()->selectedRows().at( 0 ) );
  if ( !currentLayout )
    return;

  QString currentTitle = currentLayout->name();
  QString newTitle;
  if ( !QgisApp::instance()->uniqueLayoutTitle( this, newTitle, false, currentTitle ) )
  {
    return;
  }
  currentLayout->setName( newTitle );
}

//
// QgsLayoutManagerModel
//

QgsLayoutManagerModel::QgsLayoutManagerModel( QgsLayoutManager *manager, QObject *parent )
  : QAbstractListModel( parent )
  , mLayoutManager( manager )
{
  connect( mLayoutManager, &QgsLayoutManager::layoutAboutToBeAdded, this, &QgsLayoutManagerModel::layoutAboutToBeAdded );
  connect( mLayoutManager, &QgsLayoutManager::layoutAdded, this, &QgsLayoutManagerModel::layoutAdded );
  connect( mLayoutManager, &QgsLayoutManager::layoutAboutToBeRemoved, this, &QgsLayoutManagerModel::layoutAboutToBeRemoved );
  connect( mLayoutManager, &QgsLayoutManager::layoutRemoved, this, &QgsLayoutManagerModel::layoutRemoved );
  connect( mLayoutManager, &QgsLayoutManager::layoutRenamed, this, &QgsLayoutManagerModel::layoutRenamed );
}

int QgsLayoutManagerModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mLayoutManager->layouts().count();
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
      return mLayoutManager->layouts().at( index.row() )->name();

    case LayoutRole:
      return QVariant::fromValue( mLayoutManager->layouts().at( index.row() ) );

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
  if ( index.row() >= mLayoutManager->layouts().count() )
  {
    return false;
  }

  if ( value.toString().isEmpty() )
    return false;

  QgsLayout *layout = layoutFromIndex( index );
  if ( !layout )
    return false;

  //has name changed?
  bool changed = layout->name() != value.toString();
  if ( !changed )
    return true;

  //check if name already exists
  QStringList layoutNames;
  const QList< QgsLayout * > layouts = QgsProject::instance()->layoutManager()->layouts();
  for ( QgsLayout *l : layouts )
  {
    layoutNames << l->name();
  }
  if ( layoutNames.contains( value.toString() ) )
  {
    //name exists!
    QMessageBox::warning( nullptr, tr( "Rename layout" ), tr( "There is already a layout named “%1”." ).arg( value.toString() ) );
    return false;
  }

  layout->setName( value.toString() );
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

QgsLayout *QgsLayoutManagerModel::layoutFromIndex( const QModelIndex &index ) const
{
  return qobject_cast< QgsLayout * >( qvariant_cast<QObject *>( data( index, LayoutRole ) ) );
}

void QgsLayoutManagerModel::layoutAboutToBeAdded( const QString & )
{
  int row = mLayoutManager->layouts().count();
  beginInsertRows( QModelIndex(), row, row );
}

void QgsLayoutManagerModel::layoutAboutToBeRemoved( const QString &name )
{
  QgsLayout *l = mLayoutManager->layoutByName( name );
  int row = mLayoutManager->layouts().indexOf( l );
  if ( row >= 0 )
    beginRemoveRows( QModelIndex(), row, row );
}

void QgsLayoutManagerModel::layoutAdded( const QString & )
{
  endInsertRows();
}

void QgsLayoutManagerModel::layoutRemoved( const QString & )
{
  endRemoveRows();
}

void QgsLayoutManagerModel::layoutRenamed( QgsLayout *layout, const QString & )
{
  int row = mLayoutManager->layouts().indexOf( layout );
  QModelIndex index = createIndex( row, 0 );
  emit dataChanged( index, index, QVector<int>() << Qt::DisplayRole );
}
