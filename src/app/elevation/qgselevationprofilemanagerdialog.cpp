/***************************************************************************
                         qgselevationprofilemanagerdialog.cpp
                         -----------------------
    begin                : July 2025
    copyright            : (C) 2025 by Nyall Dawson
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

#include "qgselevationprofilemanagerdialog.h"
#include "moc_qgselevationprofilemanagerdialog.cpp"
#include "qgisapp.h"
#include "qgselevationprofilemanager.h"
#include "qgselevationprofilemanagermodel.h"
#include "qgsproject.h"
#include "qgsgui.h"
#include "qgselevationprofile.h"
#include "qgsnewnamedialog.h"

#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QUrl>

QgsElevationProfileManagerDialog::QgsElevationProfileManagerDialog( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  setupUi( this );

  setObjectName( "QgsElevationProfileManagerDialog" );
  QgsGui::enableAutoGeometryRestore( this );

  mModel = new QgsElevationProfileManagerModel( QgsProject::instance()->elevationProfileManager(), this );
  mProxyModel = new QgsElevationProfileManagerProxyModel( mProfileListView );
  mProxyModel->setSourceModel( mModel );
  mProfileListView->setModel( mProxyModel );

  mSearchLineEdit->setShowSearchIcon( true );
  mSearchLineEdit->setShowClearButton( true );
  mSearchLineEdit->setFocus();
  connect( mSearchLineEdit, &QgsFilterLineEdit::textChanged, mProxyModel, &QgsElevationProfileManagerProxyModel::setFilterString );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QWidget::close );
  connect( mProfileListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsElevationProfileManagerDialog::toggleButtons );
  connect( mProfileListView, &QListView::doubleClicked, this, &QgsElevationProfileManagerDialog::itemDoubleClicked );

  connect( mShowButton, &QAbstractButton::clicked, this, &QgsElevationProfileManagerDialog::showClicked );
  connect( mDuplicateButton, &QAbstractButton::clicked, this, &QgsElevationProfileManagerDialog::duplicateClicked );
  connect( mRemoveButton, &QAbstractButton::clicked, this, &QgsElevationProfileManagerDialog::removeClicked );
  connect( mRenameButton, &QAbstractButton::clicked, this, &QgsElevationProfileManagerDialog::renameClicked );

#ifdef Q_OS_MAC
  // Create action to select this window
  mWindowAction = new QAction( windowTitle(), this );
  connect( mWindowAction, &QAction::triggered, this, &QgsElevationProfileManagerDialog::activate );
#endif

  toggleButtons();
}

void QgsElevationProfileManagerDialog::toggleButtons()
{
  // Nothing selected: no button.
  if ( mProfileListView->selectionModel()->selectedRows().isEmpty() )
  {
    mShowButton->setEnabled( false );
    mRemoveButton->setEnabled( false );
    mRenameButton->setEnabled( false );
    mDuplicateButton->setEnabled( false );
  }
  // toggle everything if one profile is selected
  else if ( mProfileListView->selectionModel()->selectedRows().count() == 1 )
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

void QgsElevationProfileManagerDialog::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
}

bool QgsElevationProfileManagerDialog::uniqueProfileTitle( QWidget *parent, QString &title, const QString &currentTitle )
{
  if ( !parent )
  {
    parent = this;
  }
  bool titleValid = false;
  QString newTitle = QString( currentTitle );

  QString chooseMsg = tr( "Enter a unique elevation profile title" );
  QString titleMsg = chooseMsg;

  QStringList profileNames;
  const QList<QgsElevationProfile *> profiles = QgsProject::instance()->elevationProfileManager()->profiles();
  profileNames.reserve( profiles.size() + 1 );
  for ( QgsElevationProfile *l : profiles )
  {
    profileNames << l->name();
  }

  const QString windowTitle = tr( "Rename Elevation Profile" );

  while ( !titleValid )
  {
    QgsNewNameDialog dlg( tr( "elevation profile" ), newTitle, QStringList(), profileNames, Qt::CaseSensitive, parent );
    dlg.setWindowTitle( windowTitle );
    dlg.setHintString( titleMsg );
    dlg.setOverwriteEnabled( false );
    dlg.setAllowEmptyName( true );
    dlg.setConflictingNameWarning( tr( "Title already exists!" ) );

    if ( dlg.exec() != QDialog::Accepted )
    {
      return false;
    }

    newTitle = dlg.name();
    if ( newTitle.isEmpty() )
    {
      titleMsg = chooseMsg + "\n\n" + tr( "Title can not be empty!" );
    }
    else if ( profileNames.indexOf( newTitle, 1 ) >= 0 )
    {
      profileNames[0] = QString(); // clear non-unique name
      titleMsg = chooseMsg + "\n\n" + tr( "Title already exists!" );
    }
    else
    {
      titleValid = true;
    }
  }

  title = newTitle;

  return true;
}

#ifdef Q_OS_MAC
void QgsElevationProfileManagerDialog::showEvent( QShowEvent *event )
{
  if ( !event->spontaneous() )
  {
    QgisApp::instance()->addWindow( mWindowAction );
  }
}

void QgsElevationProfileManagerDialog::changeEvent( QEvent *event )
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

void QgsElevationProfileManagerDialog::removeClicked()
{
  const QModelIndexList profileItems = mProfileListView->selectionModel()->selectedRows();
  if ( profileItems.isEmpty() )
  {
    return;
  }

  QString title;
  QString message;
  if ( profileItems.count() == 1 )
  {
    title = tr( "Remove Elevation Profile" );
    message = tr( "Do you really want to remove the elevation profile “%1”?" ).arg( mProfileListView->model()->data( profileItems.at( 0 ), Qt::DisplayRole ).toString() );
  }
  else
  {
    title = tr( "Remove Elevation Profiles" );
    message = tr( "Do you really want to remove all selected elevation profiles?" );
  }

  if ( QMessageBox::warning( this, title, message, QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
  {
    return;
  }

  QList<QgsElevationProfile *> profileList;
  // Find the profiles that need to be deleted
  for ( const QModelIndex &index : profileItems )
  {
    if ( QgsElevationProfile *l = mModel->profileFromIndex( mProxyModel->mapToSource( index ) ) )
    {
      profileList << l;
    }
  }

  // Once we have the profile list, we can delete all of them !
  for ( QgsElevationProfile *l : std::as_const( profileList ) )
  {
    QgsProject::instance()->elevationProfileManager()->removeProfile( l );
  }
}

void QgsElevationProfileManagerDialog::showClicked()
{
  const QModelIndexList profileItems = mProfileListView->selectionModel()->selectedRows();
  for ( const QModelIndex &index : profileItems )
  {
    if ( QgsElevationProfile *profile = mModel->profileFromIndex( mProxyModel->mapToSource( index ) ) )
    {
      QgisApp::instance()->openElevationProfile( profile );
    }
  }
}

void QgsElevationProfileManagerDialog::duplicateClicked()
{
  if ( mProfileListView->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }

  QgsElevationProfile *currentProfile = mModel->profileFromIndex( mProxyModel->mapToSource( mProfileListView->selectionModel()->selectedRows().at( 0 ) ) );
  if ( !currentProfile )
    return;
  QString currentTitle = currentProfile->name();

  QString newTitle;
  if ( !uniqueProfileTitle( this, newTitle, tr( "%1 copy" ).arg( currentTitle ) ) )
  {
    return;
  }

  auto newProfile = std::make_unique< QgsElevationProfile >( QgsProject::instance() );
  QDomDocument doc;
  const QDomElement profileElem = currentProfile->writeXml( doc, QgsReadWriteContext() );
  newProfile->readXml( profileElem, doc, QgsReadWriteContext() );
  newProfile->resolveReferences( QgsProject::instance() );
  newProfile->setName( newTitle );
  QgsElevationProfile *newProfileRef = newProfile.get();
  if ( QgsProject::instance()->elevationProfileManager()->addProfile( newProfile.release() ) )
  {
    QgisApp::instance()->openElevationProfile( newProfileRef );
  }
}

void QgsElevationProfileManagerDialog::renameClicked()
{
  if ( mProfileListView->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }

  QgsElevationProfile *currentProfile = mModel->profileFromIndex( mProxyModel->mapToSource( mProfileListView->selectionModel()->selectedRows().at( 0 ) ) );
  if ( !currentProfile )
    return;

  QString currentTitle = currentProfile->name();
  QString newTitle;
  if ( !uniqueProfileTitle( this, newTitle, currentTitle ) )
  {
    return;
  }
  currentProfile->setName( newTitle );
}

void QgsElevationProfileManagerDialog::itemDoubleClicked( const QModelIndex &index )
{
  if ( QgsElevationProfile *profile = mModel->profileFromIndex( mProxyModel->mapToSource( index ) ) )
  {
    QgisApp::instance()->openElevationProfile( profile );
  }
}
