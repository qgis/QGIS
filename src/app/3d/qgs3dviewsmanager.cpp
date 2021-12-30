#include "qgs3dviewsmanager.h"

#include "qgs3dmapcanvasdockwidget.h"
#include <QDebug>
#include "qgsnewnamedialog.h"
#include "qgisapp.h"

Qgs3DViewsManager::Qgs3DViewsManager( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  setupUi( this );
  m3DViewsListView->setModel( &mListModel );

  m3DViewsListView->setEditTriggers( QAbstractItemView::NoEditTriggers );
  m3DViewsListView->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( m3DViewsListView, &QListView::clicked, [&]( const QModelIndex & index )
  {
    mSelectedViewIndex = index.row();
  } );

  connect( mShowButton, &QToolButton::clicked, [&]()
  {
    if ( mSelectedViewIndex == -1 )
      return;
    QString viewName = mListModel.stringList()[ mSelectedViewIndex ];
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
  } );

  connect( mRenameButton, &QToolButton::clicked, [&]()
  {
    if ( mSelectedViewIndex == -1 )
      return;
    QString oldTitle = mListModel.stringList()[ mSelectedViewIndex ];

    QString newTitle;
    QgsNewNameDialog dlg( QStringLiteral( "3D view" ), newTitle, QStringList(), m3DMapViewsDom->keys(), Qt::CaseSensitive, this );
    dlg.setWindowTitle( "Title" );
    dlg.setHintString( "" );
    dlg.setOverwriteEnabled( false );
    dlg.setAllowEmptyName( false );
    dlg.setConflictingNameWarning( tr( "Title already exists!" ) );

    if ( dlg.exec() != QDialog::Accepted )
      return;
    newTitle = dlg.name();

    QDomElement dom = m3DMapViewsDom->value( oldTitle );
    m3DMapViewsDom->remove( oldTitle );
    m3DMapViewsDom->insert( newTitle, dom );
    Qgs3DMapCanvasDockWidget *widget = m3DMapViewsWidgets->value( oldTitle, nullptr );
    if ( widget )
    {
      m3DMapViewsWidgets->remove( oldTitle );
      m3DMapViewsWidgets->insert( newTitle, widget );
      widget->setName( newTitle );
    }
    reloadListModel();
  } );

  connect( mDuplicateButton, &QToolButton::clicked, [&]()
  {
    if ( mSelectedViewIndex == -1 )
      return;

    QString viewName = mListModel.stringList()[ mSelectedViewIndex ] + "_";

    Qgs3DMapCanvasDockWidget *widget = QgisApp::instance()->duplicate3DMapView( mListModel.stringList()[ mSelectedViewIndex ] );
    if ( widget )
    {
      widget->show();
      widget->activateWindow();
      widget->raise();
    }
    reloadListModel();
  } );

  connect( mRemoveButton, &QToolButton::clicked, [&]()
  {
    if ( mSelectedViewIndex == -1 )
      return;
    QString viewName = mListModel.stringList()[ mSelectedViewIndex ];
    m3DMapViewsDom->remove( viewName );
    Qgs3DMapCanvasDockWidget *w = ( *m3DMapViewsWidgets )[ viewName ];
    m3DMapViewsWidgets->remove( viewName );
    w->close();
    reloadListModel();
  } );
}

Qgs3DViewsManager::~Qgs3DViewsManager()
{
}

void Qgs3DViewsManager::reload()
{
  if ( !m3DMapViewsDom || !m3DMapViewsWidgets )
    return;
  reloadListModel();
}

void Qgs3DViewsManager::set3DMapViewsDom( QMap<QString, QDomElement> &mapViews3DDom )
{
  m3DMapViewsDom = &mapViews3DDom;
  reloadListModel();
}

void Qgs3DViewsManager::set3DMapViewsWidgets( QMap<QString, Qgs3DMapCanvasDockWidget *> &mapViews3DWidgets )
{
  m3DMapViewsWidgets = &mapViews3DWidgets;
}

void Qgs3DViewsManager::reloadListModel()
{
  mListModel.setStringList( m3DMapViewsDom->keys() );
}
