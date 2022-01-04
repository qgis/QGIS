/***************************************************************************
  qgs3dviewsmanagerdialog.cpp
  --------------------------------------
  Date                 : December 2021
  Copyright            : (C) 2021 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dviewsmanagerdialog.h"

#include "qgisapp.h"
#include "qgs3dmapcanvasdockwidget.h"
#include "qgsnewnamedialog.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dviewsmanager.h"

#include <QMessageBox>

Qgs3DViewsManagerDialog::Qgs3DViewsManagerDialog( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  setupUi( this );

  mListModel = new QStringListModel( this );
  m3DViewsListView->setModel( mListModel );

  m3DViewsListView->setEditTriggers( QAbstractItemView::NoEditTriggers );
  m3DViewsListView->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QWidget::close );

  connect( mShowButton, &QToolButton::clicked, this, &Qgs3DViewsManagerDialog::showClicked );
  connect( mHideButton, &QToolButton::clicked, this, &Qgs3DViewsManagerDialog::hideClicked );
  connect( mDuplicateButton, &QToolButton::clicked, this, &Qgs3DViewsManagerDialog::duplicateClicked );
  connect( mRemoveButton, &QToolButton::clicked, this, &Qgs3DViewsManagerDialog::removeClicked );
  connect( mRenameButton, &QToolButton::clicked, this, &Qgs3DViewsManagerDialog::renameClicked );
  mShowButton->setEnabled( false );
  mHideButton->setEnabled( false );

  connect( m3DViewsListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &Qgs3DViewsManagerDialog::showHideButtonStateChanged );

  connect( QgsProject::instance()->views3DManager(), &Qgs3DViewsManager::viewsListChanged, this, &Qgs3DViewsManagerDialog::onViewsListChanged );
}

void Qgs3DViewsManagerDialog::onViewsListChanged()
{
  reload();
}

void Qgs3DViewsManagerDialog::showClicked()
{
  if ( m3DViewsListView->selectionModel()->selectedRows().isEmpty() )
    return;

  QString viewName = m3DViewsListView->selectionModel()->selectedRows().at( 0 ).data( Qt::DisplayRole ).toString();

  Qgs3DMapCanvasDockWidget *widget = QgisApp::instance()->findChild<Qgs3DMapCanvasDockWidget *>( viewName + QStringLiteral( "DockObject" ) );

  if ( !widget )
  {
    widget = QgisApp::instance()->open3DMapView( viewName );
  }

  if ( widget )
  {
    widget->show();
    QgsProject::instance()->setDirty();
  }

}

void Qgs3DViewsManagerDialog::hideClicked()
{
  if ( m3DViewsListView->selectionModel()->selectedRows().isEmpty() )
    return;

  QString viewName = m3DViewsListView->selectionModel()->selectedRows().at( 0 ).data( Qt::DisplayRole ).toString();

  Qgs3DMapCanvasDockWidget *widget = QgisApp::instance()->findChild<Qgs3DMapCanvasDockWidget *>( viewName + QStringLiteral( "DockObject" ) );
  if ( widget )
  {
    widget->close();
  }

  QgsProject::instance()->setDirty();
}

void Qgs3DViewsManagerDialog::duplicateClicked()
{
  if ( m3DViewsListView->selectionModel()->selectedRows().isEmpty() )
    return;

  QString existingViewName = m3DViewsListView->selectionModel()->selectedRows().at( 0 ).data( Qt::DisplayRole ).toString();
  QString newViewName = askUserForATitle( existingViewName, tr( "Duplicate" ), false );

  QgisApp::instance()->duplicate3DMapView( existingViewName, newViewName );

  QgsProject::instance()->setDirty();
}

void Qgs3DViewsManagerDialog::removeClicked()
{
  if ( m3DViewsListView->selectionModel()->selectedRows().isEmpty() )
    return;

  QString warningTitle = tr( "Remove 3D View" );
  QString warningMessage = tr( "Do you really want to remove selected 3D view?" );

  if ( QMessageBox::warning( this, warningTitle, warningMessage, QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
    return;

  QString viewName = m3DViewsListView->selectionModel()->selectedRows().at( 0 ).data( Qt::DisplayRole ).toString();

  QgsProject::instance()->views3DManager()->remove3DView( viewName );
  if ( Qgs3DMapCanvasDockWidget *w = QgisApp::instance()->findChild<Qgs3DMapCanvasDockWidget *>( viewName + QStringLiteral( "DockObject" ) ) )
    w->close();

  QgsProject::instance()->setDirty();
}

void Qgs3DViewsManagerDialog::renameClicked()
{
  if ( m3DViewsListView->selectionModel()->selectedRows().isEmpty() )
    return;

  QString oldTitle = m3DViewsListView->selectionModel()->selectedRows().at( 0 ).data( Qt::DisplayRole ).toString();
  QString newTitle = askUserForATitle( oldTitle, tr( "Rename" ), true );

  if ( newTitle.isEmpty() )
    return;

  QgsProject::instance()->views3DManager()->rename3DView( oldTitle, newTitle );

  if ( Qgs3DMapCanvasDockWidget *widget = QgisApp::instance()->findChild<Qgs3DMapCanvasDockWidget *>( oldTitle + QStringLiteral( "DockObject" ) ) )
  {
    widget->setWindowTitle( newTitle );
    widget->setObjectName( newTitle + QStringLiteral( "DockObject" ) );
    widget->mapCanvas3D()->setObjectName( newTitle );
  }

  QgsProject::instance()->setDirty();
}

void Qgs3DViewsManagerDialog::showHideButtonStateChanged( const QModelIndex &current, const QModelIndex &previous )
{
  if ( !current.isValid() )
  {
    mShowButton->setEnabled( false );
    mHideButton->setEnabled( false );
    return;
  }

  QString viewName = current.data( Qt::DisplayRole ).toString();
  bool isOpen = QgsProject::instance()->views3DManager()->get3DViewSettings( viewName ).attribute( QStringLiteral( "isOpen" ), QStringLiteral( "0" ) ).toInt() == 1;
  mShowButton->setEnabled( !isOpen );
  mHideButton->setEnabled( isOpen );
}

void Qgs3DViewsManagerDialog::reload()
{
  QStringList names = QgsProject::instance()->views3DManager()->get3DViewsNames();
  mListModel->setStringList( names );
}

QString Qgs3DViewsManagerDialog::askUserForATitle( QString oldTitle, QString action, bool allowExistingTitle )
{
  QString newTitle = oldTitle;
  QStringList notAllowedTitles = mListModel->stringList();
  if ( allowExistingTitle )
    notAllowedTitles.removeOne( oldTitle );
  QgsNewNameDialog dlg( tr( "3D view" ), newTitle, QStringList(), notAllowedTitles, Qt::CaseSensitive, this );
  dlg.setWindowTitle( tr( "%1 3D Map View" ).arg( action ) );
  dlg.setHintString( tr( "Enter a unique 3D map view title" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setAllowEmptyName( false );
  dlg.setConflictingNameWarning( tr( "Title already exists!" ) );

  if ( dlg.exec() != QDialog::Accepted )
    return QString();
  newTitle = dlg.name();
  return newTitle;
}
