#include "qgsmaplayerstyleguiutils.h"

#include <QAction>
#include <QInputDialog>
#include <QMenu>

#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerstylemanager.h"


void QgsMapLayerStyleGuiUtils::addStyleManagerMenu( QMenu* menu, QgsMapLayer* layer )
{
  layer->enableStyleManager();
  QMenu* m = new QMenu( tr( "Styles" ) );
  QAction* actionAdd = m->addAction( tr( "Add" ), this, SLOT( addStyle() ) );
  actionAdd->setData( QVariant::fromValue<QObject*>( layer ) );
  QMenu* mRemove = m->addMenu( tr( "Remove" ) );
  m->addSeparator();

  QgsMapLayerStyleManager* mgr = layer->styleManager();
  foreach ( QString name, mgr->styles() )
  {
    bool active = name == mgr->currentStyle();
    if ( name.isEmpty() )
      name = defaultStyleName();
    QAction* actionUse = m->addAction( name, this, SLOT( useStyle() ) );
    actionUse->setCheckable( true );
    actionUse->setChecked( active );
    actionUse->setData( QVariant::fromValue<QObject*>( layer ) );

    QAction* actionRemove = mRemove->addAction( name, this, SLOT( removeStyle() ) );
    actionRemove->setData( QVariant::fromValue<QObject*>( layer ) );
  }

  menu->addMenu( m );
}

QString QgsMapLayerStyleGuiUtils::defaultStyleName()
{
  return tr( "(default)" );
}


void QgsMapLayerStyleGuiUtils::addStyle()
{
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( !a )
    return;
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( a->data().value<QObject*>() );
  if ( !layer )
    return;

  bool ok;
  QString text = QInputDialog::getText( 0, tr( "New style" ),
                                        tr( "Style name:" ), QLineEdit::Normal,
                                        "newstyle", &ok );
  if ( !ok || text.isEmpty() )
    return;

  bool res = layer->styleManager()->addStyleFromLayer( text );
  qDebug( "ADD: %d", res );

  if ( res ) // make it active!
    layer->styleManager()->setCurrentStyle( text );
}

void QgsMapLayerStyleGuiUtils::useStyle()
{
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( !a )
    return;
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( a->data().value<QObject*>() );
  if ( !layer )
    return;
  QString name = a->text();
  if ( name == defaultStyleName() )
    name.clear();

  bool res = layer->styleManager()->setCurrentStyle( name );
  qDebug( "USE: %d", res );

  layer->triggerRepaint();
}


void QgsMapLayerStyleGuiUtils::removeStyle()
{
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( !a )
    return;
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( a->data().value<QObject*>() );
  if ( !layer )
    return;
  QString name = a->text();
  if ( name == defaultStyleName() )
    name.clear();

  bool needsRefresh = ( layer->styleManager()->currentStyle() == name );

  bool res = layer->styleManager()->removeStyle( name );
  qDebug( "DEL: %d", res );

  if ( needsRefresh )
    layer->triggerRepaint();
}
