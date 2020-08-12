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
#include "qgsprintlayout.h"
#include "qgsreport.h"
#include "qgsreadwritecontext.h"
#include "qgshelp.h"

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

  QgsGui::enableAutoGeometryRestore( this );
  mTemplateFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mTemplateFileWidget->setFilter( tr( "Layout templates" ) + QStringLiteral( " (*.qpt *.QPT)" ) );
  mTemplateFileWidget->setDialogTitle( tr( "Select a Template" ) );
  mTemplateFileWidget->lineEdit()->setShowClearButton( false );
  QgsSettings settings;
  mTemplateFileWidget->setDefaultRoot( settings.value( QStringLiteral( "lastComposerTemplateDir" ), QString(), QgsSettings::App ).toString() );
  mTemplateFileWidget->setFilePath( settings.value( QStringLiteral( "ComposerManager/templatePath" ), QString(), QgsSettings::App ).toString() );

  connect( mTemplateFileWidget, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "ComposerManager/templatePath" ), mTemplateFileWidget->filePath(), QgsSettings::App );
    QFileInfo tmplFileInfo( mTemplateFileWidget->filePath() );
    settings.setValue( QStringLiteral( "lastComposerTemplateDir" ), tmplFileInfo.absolutePath(), QgsSettings::App );
  } );

  mModel = new QgsLayoutManagerModel( QgsProject::instance()->layoutManager(),
                                      this );
  mProxyModel = new QgsLayoutManagerProxyModel( mLayoutListView );
  mProxyModel->setSourceModel( mModel );
  mLayoutListView->setModel( mProxyModel );

  mSearchLineEdit->setShowSearchIcon( true );
  mSearchLineEdit->setShowClearButton( true );
  mSearchLineEdit->setFocus();
  connect( mSearchLineEdit, &QgsFilterLineEdit::textChanged, mProxyModel, &QgsLayoutManagerProxyModel::setFilterString );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QWidget::close );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsLayoutManagerDialog::showHelp );
  connect( mLayoutListView->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &QgsLayoutManagerDialog::toggleButtons );
  connect( mLayoutListView, &QListView::doubleClicked, this, &QgsLayoutManagerDialog::itemDoubleClicked );

  connect( mShowButton, &QAbstractButton::clicked, this, &QgsLayoutManagerDialog::showClicked );
  connect( mDuplicateButton, &QAbstractButton::clicked, this, &QgsLayoutManagerDialog::duplicateClicked );
  connect( mRemoveButton, &QAbstractButton::clicked, this, &QgsLayoutManagerDialog::removeClicked );
  connect( mRenameButton, &QAbstractButton::clicked, this, &QgsLayoutManagerDialog::renameClicked );

#ifdef Q_OS_MAC
  // Create action to select this window
  mWindowAction = new QAction( windowTitle(), this );
  connect( mWindowAction, &QAction::triggered, this, &QgsLayoutManagerDialog::activate );
#endif

  mTemplate->addItem( tr( "Empty Layout" ) );
  mTemplate->addItem( tr( "Empty Report" ) );
  mTemplate->addItem( tr( "Specific" ) );

  mUserTemplatesDir = QgsApplication::qgisSettingsDirPath() + "/composer_templates";
  QMap<QString, QString> userTemplateMap = defaultTemplates( true );
  addTemplates( userTemplateMap );

  // TODO QGIS 4: Remove this, default templates should not be shipped in the application folder
  mDefaultTemplatesDir = QgsApplication::pkgDataPath() + "/composer_templates";
  QMap<QString, QString> defaultTemplateMap = defaultTemplates( false );
  addTemplates( defaultTemplateMap );
  addTemplates( otherTemplates() );

  mTemplatesDefaultDirBtn->setToolTip( tr( "Use <i>Settings --> Options --> Layouts --> Layout Paths</i> to configure the folders in which QGIS will search for print layout templates." ) );

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
  updateTemplateButtonEnabledState();
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
  const QStringList paths = QgsApplication::layoutTemplatePaths();
  for ( const QString &path : paths )
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

  const QFileInfoList fileInfoList = templateDir.entryInfoList( QDir::Files );
  for ( const QFileInfo &info : fileInfoList )
  {
    if ( info.suffix().compare( QLatin1String( "qpt" ), Qt::CaseInsensitive ) == 0 )
    {
      templateMap.insert( info.baseName(), info.absoluteFilePath() );
    }
  }
  return templateMap;
}

void QgsLayoutManagerDialog::mAddButton_clicked()
{
  QFile templateFile;
  bool loadingTemplate = ( mTemplate->currentIndex() > 1 );
  QDomDocument templateDoc;
  QString storedTitle;
  if ( loadingTemplate )
  {
    if ( mTemplate->currentIndex() == 2 )
    {
      templateFile.setFileName( mTemplateFileWidget->filePath() );
    }
    else
    {
      templateFile.setFileName( mTemplate->currentData().toString() );
    }

    if ( !templateFile.exists() )
    {
      QMessageBox::warning( this, tr( "Create Layout" ), tr( "Template file “%1” not found." ).arg( templateFile.fileName() ) );
      return;
    }
    if ( !templateFile.open( QIODevice::ReadOnly ) )
    {
      QMessageBox::warning( this, tr( "Create Layout" ), tr( "Could not read template file “%1”." ).arg( templateFile.fileName() ) );
      return;
    }


    if ( templateDoc.setContent( &templateFile, false ) )
    {
      QDomElement layoutElem = templateDoc.documentElement();
      if ( !layoutElem.isNull() )
        storedTitle = layoutElem.attribute( QStringLiteral( "name" ) );
    }
  }

  if ( mTemplate->currentIndex() == 1 ) // if it's an empty report
  {
    createReport();
  }
  else
  {
    QString title;
    if ( !QgisApp::instance()->uniqueLayoutTitle( this, title, true, QgsMasterLayoutInterface::PrintLayout, storedTitle ) )
    {
      return;
    }

    if ( title.isEmpty() )
    {
      title = QgsProject::instance()->layoutManager()->generateUniqueTitle( QgsMasterLayoutInterface::PrintLayout );
    }

    std::unique_ptr< QgsPrintLayout > layout = qgis::make_unique< QgsPrintLayout >( QgsProject::instance() );
    if ( loadingTemplate )
    {
      bool loadedOK = false;
      ( void )layout->loadFromTemplate( templateDoc, QgsReadWriteContext(), true, &loadedOK );
      if ( !loadedOK )
      {
        QMessageBox::warning( this, tr( "Create Layout" ), tr( "Invalid template file “%1”." ).arg( templateFile.fileName() ) );
        layout.reset();
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
}

void QgsLayoutManagerDialog::mTemplate_currentIndexChanged( int indx )
{
  bool specific = ( indx == 2 ); // comes just after empty templates
  mTemplateFileWidget->setEnabled( specific );
}

void QgsLayoutManagerDialog::mTemplatesDefaultDirBtn_pressed()
{
  if ( QDir( mDefaultTemplatesDir ).exists() )
    openLocalDirectory( mDefaultTemplatesDir );
  else
  {
    const QStringList paths = QgsApplication::layoutTemplatePaths();
    if ( !paths.empty() )
      openLocalDirectory( paths.at( 0 ) );
  }
}

void QgsLayoutManagerDialog::mTemplatesUserDirBtn_pressed()
{
  openLocalDirectory( mUserTemplatesDir );
}

void QgsLayoutManagerDialog::createReport()
{
  QString title;
  if ( !QgisApp::instance()->uniqueLayoutTitle( this, title, true, QgsMasterLayoutInterface::Report ) )
  {
    return;
  }

  if ( title.isEmpty() )
  {
    title = QgsProject::instance()->layoutManager()->generateUniqueTitle( QgsMasterLayoutInterface::Report );
  }

  std::unique_ptr< QgsReport > report = qgis::make_unique< QgsReport >( QgsProject::instance() );
  report->setName( title );

  std::unique_ptr< QgsLayout > header = qgis::make_unique< QgsLayout >( QgsProject::instance() );
  header->initializeDefaults();
  report->setHeader( header.release() );
  report->setHeaderEnabled( true );

  QgisApp::instance()->openLayoutDesignerDialog( report.get() );
  QgsProject::instance()->layoutManager()->addLayout( report.release() );
}

void QgsLayoutManagerDialog::openLocalDirectory( const QString &localDirPath )
{
  QDir localDir;
  if ( !localDir.mkpath( localDirPath ) )
  {
    QMessageBox::warning( this, tr( "Open Directory" ), tr( "Could not open or create local directory “%1”." ).arg( localDirPath ) );
  }
  else
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( localDirPath ) );
  }
}

void QgsLayoutManagerDialog::updateTemplateButtonEnabledState()
{
  mTemplatesDefaultDirBtn->setEnabled( QDir( mDefaultTemplatesDir ).exists() || !QgsApplication::layoutTemplatePaths().empty() );
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
    title = tr( "Remove Layout" );
    message = tr( "Do you really want to remove the print layout “%1”?" ).arg(
                mLayoutListView->model()->data( layoutItems.at( 0 ), Qt::DisplayRole ).toString() );
  }
  else
  {
    title = tr( "Remove Layouts" );
    message = tr( "Do you really want to remove all selected print layouts?" );
  }

  if ( QMessageBox::warning( this, title, message, QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
  {
    return;
  }

  QList<QgsMasterLayoutInterface *> layoutList;
  // Find the layouts that need to be deleted
  for ( const QModelIndex &index : layoutItems )
  {
    QgsMasterLayoutInterface *l = mModel->layoutFromIndex( mProxyModel->mapToSource( index ) );
    if ( l )
    {
      layoutList << l;
    }
  }

  // Once we have the layout list, we can delete all of them !
  for ( QgsMasterLayoutInterface *l : qgis::as_const( layoutList ) )
  {
    QgsProject::instance()->layoutManager()->removeLayout( l );
  }
}

void QgsLayoutManagerDialog::showClicked()
{
  const QModelIndexList layoutItems = mLayoutListView->selectionModel()->selectedRows();
  for ( const QModelIndex &index : layoutItems )
  {
    if ( QgsMasterLayoutInterface *l = mModel->layoutFromIndex( mProxyModel->mapToSource( index ) ) )
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

  QgsMasterLayoutInterface *currentLayout = mModel->layoutFromIndex( mProxyModel->mapToSource( mLayoutListView->selectionModel()->selectedRows().at( 0 ) ) );
  if ( !currentLayout )
    return;
  QString currentTitle = currentLayout->name();

  QString newTitle;
  if ( !QgisApp::instance()->uniqueLayoutTitle( this, newTitle, true, currentLayout->layoutType(), tr( "%1 copy" ).arg( currentTitle ) ) )
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
    QMessageBox::warning( this, tr( "Duplicate Layout" ),
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

  QgsMasterLayoutInterface *currentLayout = mModel->layoutFromIndex( mProxyModel->mapToSource( mLayoutListView->selectionModel()->selectedRows().at( 0 ) ) );
  if ( !currentLayout )
    return;

  QString currentTitle = currentLayout->name();
  QString newTitle;
  if ( !QgisApp::instance()->uniqueLayoutTitle( this, newTitle, true, currentLayout->layoutType(), currentTitle ) )
  {
    return;
  }
  currentLayout->setName( newTitle );
}

void QgsLayoutManagerDialog::itemDoubleClicked( const QModelIndex &index )
{
  if ( QgsMasterLayoutInterface *l = mModel->layoutFromIndex( mProxyModel->mapToSource( index ) ) )
  {
    QgisApp::instance()->openLayoutDesignerDialog( l );
  }
}

void QgsLayoutManagerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "print_composer/overview_composer.html#the-layout-manager" ) );
}


