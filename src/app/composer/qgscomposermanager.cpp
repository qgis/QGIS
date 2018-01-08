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
#include <QPushButton>

QgsComposerManager::QgsComposerManager( QWidget *parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/ComposerManager/geometry" ) ).toByteArray() );

  mModel = new QgsComposerManagerModel( QgsProject::instance()->layoutManager(),
                                        this );
  mComposerListView->setModel( mModel );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QWidget::close );
  connect( mComposerListView->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &QgsComposerManager::toggleButtons );

  mShowButton = mButtonBox->addButton( tr( "&Show" ), QDialogButtonBox::ActionRole );
  connect( mShowButton, &QAbstractButton::clicked, this, &QgsComposerManager::showClicked );

#ifdef Q_OS_MAC
  // Create action to select this window
  mWindowAction = new QAction( windowTitle(), this );
  connect( mWindowAction, SIGNAL( triggered() ), this, SLOT( activate() ) );
#endif
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
  }
  // toggle everything if one composer is selected
  else if ( mComposerListView->selectionModel()->selectedRows().count() == 1 )
  {
    mShowButton->setEnabled( true );
  }
  // toggle only show and remove buttons in other cases
  else
  {
    mShowButton->setEnabled( true );
  }
}

void QgsComposerManager::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
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

//
// QgsComposerManagerModel
//

QgsComposerManagerModel::QgsComposerManagerModel( QgsLayoutManager *manager, QObject *parent )
  : QAbstractListModel( parent )
  , mLayoutManager( manager )
{
  connect( mLayoutManager, &QgsLayoutManager::compositionAboutToBeAdded, this, &QgsComposerManagerModel::compositionAboutToBeAdded );
  connect( mLayoutManager, &QgsLayoutManager::compositionAdded, this, &QgsComposerManagerModel::compositionAdded );
  connect( mLayoutManager, &QgsLayoutManager::compositionAboutToBeRemoved, this, &QgsComposerManagerModel::compositionAboutToBeRemoved );
  connect( mLayoutManager, &QgsLayoutManager::compositionRemoved, this, &QgsComposerManagerModel::compositionRemoved );
  connect( mLayoutManager, &QgsLayoutManager::compositionRenamed, this, &QgsComposerManagerModel::compositionRenamed );
}

int QgsComposerManagerModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mLayoutManager->compositions().count();
}

QVariant QgsComposerManagerModel::data( const QModelIndex &index, int role ) const
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

bool QgsComposerManagerModel::setData( const QModelIndex &index, const QVariant &value, int role )
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

Qt::ItemFlags QgsComposerManagerModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractListModel::flags( index );

  return flags;
}

QgsComposition *QgsComposerManagerModel::compositionFromIndex( const QModelIndex &index ) const
{
  return qobject_cast< QgsComposition * >( qvariant_cast<QObject *>( data( index, CompositionRole ) ) );
}

void QgsComposerManagerModel::compositionAboutToBeAdded( const QString & )
{
  int row = mLayoutManager->compositions().count();
  beginInsertRows( QModelIndex(), row, row );
}

void QgsComposerManagerModel::compositionAboutToBeRemoved( const QString &name )
{
  QgsComposition *c = mLayoutManager->compositionByName( name );
  int row = mLayoutManager->compositions().indexOf( c );
  if ( row >= 0 )
    beginRemoveRows( QModelIndex(), row, row );
}

void QgsComposerManagerModel::compositionAdded( const QString & )
{
  endInsertRows();
}

void QgsComposerManagerModel::compositionRemoved( const QString & )
{
  endRemoveRows();
}

void QgsComposerManagerModel::compositionRenamed( QgsComposition *composition, const QString & )
{
  int row = mLayoutManager->compositions().indexOf( composition );
  QModelIndex index = createIndex( row, 0 );
  emit dataChanged( index, index, QVector<int>() << Qt::DisplayRole );
}
