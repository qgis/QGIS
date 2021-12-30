#include "qgs3dviewsmanager.h"

#include "qgs3dmapcanvasdockwidget.h"
#include "qgsnewnamedialog.h"
#include "qgisapp.h"

Qgs3DViewsManager::Qgs3DViewsManager( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  setupUi( this );
  m3DViewsListView->setModel( &mListModel );

  m3DViewsListView->setEditTriggers( QAbstractItemView::NoEditTriggers );
  m3DViewsListView->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( mOpenButton, &QToolButton::clicked, this, &Qgs3DViewsManager::openClicked );
  connect( mDuplicateButton, &QToolButton::clicked, this, &Qgs3DViewsManager::duplicateClicked );
  connect( mRemoveButton, &QToolButton::clicked, this, &Qgs3DViewsManager::removeClicked );
  connect( mRenameButton, &QToolButton::clicked, this, &Qgs3DViewsManager::renameClicked );
}

void Qgs3DViewsManager::openClicked()
{
  if ( m3DViewsListView->selectionModel()->selectedRows().isEmpty() )
    return;

  QString viewName = mListModel.stringList()[ m3DViewsListView->selectionModel()->selectedRows().at( 0 ).row() ];
  Qgs3DMapCanvasDockWidget *widget = m3DMapViewsWidgets->value( viewName, nullptr );
  if ( !widget )
  {
    widget = QgisApp::instance()->open3DMapView( viewName );
  }
  if ( widget )
  {
    widget->show();
    widget->activateWindow();
    widget->raise();
  }
}

void Qgs3DViewsManager::duplicateClicked()
{
  if ( m3DViewsListView->selectionModel()->selectedRows().isEmpty() )
    return;

  QString existingViewName = mListModel.stringList()[ m3DViewsListView->selectionModel()->selectedRows().at( 0 ).row() ];
  QString newViewName = askUserForATitle( existingViewName, "Duplicate", false );

  QgisApp::instance()->duplicate3DMapView( existingViewName, newViewName );
  reload();
}

void Qgs3DViewsManager::removeClicked()
{
  if ( m3DViewsListView->selectionModel()->selectedRows().isEmpty() )
    return;

  QString viewName = mListModel.stringList()[ m3DViewsListView->selectionModel()->selectedRows().at( 0 ).row() ];
  m3DMapViewsDom->remove( viewName );
  Qgs3DMapCanvasDockWidget *w = ( *m3DMapViewsWidgets )[ viewName ];
  m3DMapViewsWidgets->remove( viewName );
  w->close();
  reload();
}

void Qgs3DViewsManager::renameClicked()
{
  if ( m3DViewsListView->selectionModel()->selectedRows().isEmpty() )
    return;

  QString oldTitle = mListModel.stringList()[ m3DViewsListView->selectionModel()->selectedRows().at( 0 ).row() ];
  QString newTitle = askUserForATitle( oldTitle, "Rename", true );

  if ( newTitle.isEmpty() )
    return;

  QDomElement dom = m3DMapViewsDom->value( oldTitle );
  Qgs3DMapCanvasDockWidget *widget = m3DMapViewsWidgets->value( oldTitle, nullptr );

  m3DMapViewsDom->remove( oldTitle );
  m3DMapViewsDom->insert( newTitle, dom );
  if ( widget )
  {
    m3DMapViewsWidgets->remove( oldTitle );
    m3DMapViewsWidgets->insert( newTitle, widget );
    widget->setName( newTitle );
  }
  reload();
}

void Qgs3DViewsManager::reload()
{
  if ( !m3DMapViewsDom || !m3DMapViewsWidgets )
    return;

  mListModel.setStringList( m3DMapViewsDom->keys() );
}

void Qgs3DViewsManager::set3DMapViewsDom( QMap<QString, QDomElement> &mapViews3DDom )
{
  m3DMapViewsDom = &mapViews3DDom;
  reload();
}

void Qgs3DViewsManager::set3DMapViewsWidgets( QMap<QString, Qgs3DMapCanvasDockWidget *> &mapViews3DWidgets )
{
  m3DMapViewsWidgets = &mapViews3DWidgets;
  reload();
}

QString Qgs3DViewsManager::askUserForATitle( QString oldTitle, QString action, bool allowExistingTitle )
{
  QString newTitle = oldTitle;
  QStringList notAllowedTitles = m3DMapViewsDom->keys();
  if ( allowExistingTitle )
    notAllowedTitles.removeOne( oldTitle );
  QgsNewNameDialog dlg( QStringLiteral( "3D view" ), newTitle, QStringList(), notAllowedTitles, Qt::CaseSensitive, this );
  dlg.setWindowTitle( QStringLiteral( "%1 3D Map View" ).arg( action ) );
  dlg.setHintString( QStringLiteral( "Enter a unique 3D map view title" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setAllowEmptyName( false );
  dlg.setConflictingNameWarning( tr( "Title already exists!" ) );

  if ( dlg.exec() != QDialog::Accepted )
    return QString();
  newTitle = dlg.name();
  return newTitle;
}
