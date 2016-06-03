#include <QAction>
#include <QVBoxLayout>
#include <QToolBar>
#include <QInputDialog>

#include "qgsmaplayerstylemanagerwidget.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"
#include "qgsmapstylepanel.h"
#include "qgsmaplayerstylemanager.h"


QgsMapLayerStyleManagerWidget::QgsMapLayerStyleManagerWidget( QgsMapLayer* layer, QgsMapCanvas *canvas, QWidget *parent )
    : QgsMapStylePanel( layer, canvas, parent )
{
  mModel = new QStandardItemModel( this );
  mStyleList = new QListView( this );
  mStyleList->setModel( mModel );
  mStyleList->setViewMode( QListView::ListMode );
  mStyleList->setResizeMode( QListView::Adjust );

  QToolBar* toolbar = new QToolBar( this );
  QAction* addAction = toolbar->addAction( tr( "Add" ) );
  connect( addAction, SIGNAL( triggered() ), this, SLOT( addStyle() ) );
  QAction* removeAction = toolbar->addAction( tr( "Remove Current" ) );
  connect( removeAction, SIGNAL( triggered() ), this, SLOT( removeStyle() ) );

  connect( canvas, SIGNAL( mapCanvasRefreshed() ), this, SLOT( updateCurrent() ) );

  connect( mStyleList, SIGNAL( clicked( QModelIndex ) ), this, SLOT( styleClicked( QModelIndex ) ) );

  setLayout( new QVBoxLayout() );
  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->addWidget( toolbar );
  layout()->addWidget( mStyleList );

  connect( mLayer->styleManager(), SIGNAL( currentStyleChanged( QString ) ), this, SLOT( currentStyleChanged( QString ) ) );
  connect( mLayer->styleManager(), SIGNAL( styleAdded( QString ) ), this, SLOT( styleAdded( QString ) ) );
  connect( mLayer->styleManager(), SIGNAL( styleremoved( QString ) ), this, SLOT( styleRemoved( QString ) ) );
  connect( mLayer->styleManager(), SIGNAL( styleRenamed( QString, QString ) ), this, SLOT( styleRenamed( QString, QString ) ) );

  mModel->clear();

  Q_FOREACH ( const QString name, mLayer->styleManager()->styles() )
  {
    QString stylename = name;

    if ( stylename.isEmpty() )
      stylename = "(default)";

    QStandardItem* item = new QStandardItem( stylename );
    mModel->appendRow( item );
  }

  QString active = mLayer->styleManager()->currentStyle();
  currentStyleChanged( active );
}

void QgsMapLayerStyleManagerWidget::styleClicked( QModelIndex index )
{
  if ( !mLayer || !index.isValid() )
    return;

  QString name = index.data().toString();
  mLayer->styleManager()->setCurrentStyle( name );
}

void QgsMapLayerStyleManagerWidget::currentStyleChanged( QString name )
{
  QList<QStandardItem*> items = mModel->findItems( name );
  if ( items.isEmpty() )
    return;

  QStandardItem* item = items.at( 0 );

  mStyleList->setCurrentIndex( item->index() );
}

void QgsMapLayerStyleManagerWidget::styleAdded( QString name )
{
  QgsDebugMsg( "Style added" );
  QStandardItem* item = new QStandardItem( name );
  mModel->appendRow( item );
}

void QgsMapLayerStyleManagerWidget::styleRemoved( QString name )
{
  QList<QStandardItem*> items = mModel->findItems( name );
  if ( items.isEmpty() )
    return;

  QStandardItem* item = items.at( 0 );
  mModel->removeRow( item->row() );
}

void QgsMapLayerStyleManagerWidget::styleRenamed( QString oldname, QString newname )
{
  QList<QStandardItem*> items = mModel->findItems( oldname );
  if ( items.isEmpty() )
    return;

  QStandardItem* item = items.at( 0 );
  item->setText( newname );
}

void QgsMapLayerStyleManagerWidget::addStyle()
{
  bool ok;
  QString text = QInputDialog::getText( nullptr, tr( "New style" ),
                                        tr( "Style name:" ), QLineEdit::Normal,
                                        "new style", &ok );
  if ( !ok || text.isEmpty() )
    return;

  bool res = mLayer->styleManager()->addStyleFromLayer( text );
  if ( res ) // make it active!
  {
    mLayer->styleManager()->setCurrentStyle( text );
  }
  else
  {
    QgsDebugMsg( "Failed to add style: " + text );
  }
}

void QgsMapLayerStyleManagerWidget::removeStyle()
{
  QString current =  mLayer->styleManager()->currentStyle();
  QList<QStandardItem*> items = mModel->findItems( current );
  if ( items.isEmpty() )
    return;

  QStandardItem* item = items.at( 0 );
  bool res = mLayer->styleManager()->removeStyle( current );
  if ( res )
  {
      mModel->removeRow( item->row() );
  }
  else
  {
    QgsDebugMsg( "Failed to remove current style" );
  }

}
