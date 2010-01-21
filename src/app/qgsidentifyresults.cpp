/***************************************************************************
                     qgsidentifyresults.cpp  -  description
                              -------------------
      begin                : Fri Oct 25 2002
      copyright            : (C) 2002 by Gary E.Sherman
      email                : sherman at mrcc dot com
      Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsidentifyresults.h"
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsrubberband.h"
#include "qgsgeometry.h"
#include "qgsattributedialog.h"
#include "qgsmapcanvas.h"
#include "qgsattributeaction.h"

#include <QCloseEvent>
#include <QLabel>
#include <QAction>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QSettings>
#include <QMenu>
#include <QClipboard>
#include <QDockWidget>
#include <QMenuBar>
#include <QPushButton>

#include "qgslogger.h"

static void _runPythonString( const QString &expr )
{
  QgisApp::instance()->runPythonString( expr );
}

void QgsFeatureAction::execute()
{
  int idx;
  QList< QPair<QString, QString> > attributes;
  mResults->retrieveAttributes( mFeatItem, attributes, idx );
  mLayer->actions()->doAction( mAction, attributes, idx, _runPythonString );
}

class QgsIdentifyResultsDock : public QDockWidget
{
  public:
    QgsIdentifyResultsDock( const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0 )
        : QDockWidget( title, parent, flags )
    {
      setObjectName( "IdentifyResultsTableDock" ); // set object name so the position can be saved
    }

    virtual void closeEvent( QCloseEvent * ev )
    {
      deleteLater();
    }
};

// Tree hierarchy
//
// layer [userrole: QgsMapLayer]
//   feature: displayfield|displayvalue [userrole: fid]
//     derived attributes (if any) [userrole: "derived"]
//       name value
//     actions (if any) [userrole: "actions"]
//       edit [userrole: "edit"]
//       action [userrole: "action", idx]
//     name value
//     name value
//     name value
//   feature
//     derived attributes (if any)
//       name value
//     actions (if any)
//       action
//     name value

QgsIdentifyResults::QgsIdentifyResults( QgsMapCanvas *canvas, QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ),
    mActionPopup( 0 ),
    mCanvas( canvas ),
    mDock( NULL )
{
  setupUi( this );
  QSettings mySettings;
  bool myDockFlag = mySettings.value( "/qgis/dockIdentifyResults", false ).toBool();
  if ( myDockFlag )
  {
    mDock = new QgsIdentifyResultsDock( tr( "Identify Results" ) , QgisApp::instance() );
    mDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    mDock->setWidget( this );
    QgisApp::instance()->addDockWidget( Qt::LeftDockWidgetArea, mDock );
  }
  lstResults->setColumnCount( 2 );
  setColumnText( 0, tr( "Feature" ) );
  setColumnText( 1, tr( "Value" ) );

  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );

  connect( lstResults, SIGNAL( itemExpanded( QTreeWidgetItem* ) ),
           this, SLOT( itemExpanded( QTreeWidgetItem* ) ) );

  connect( lstResults, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ),
           this, SLOT( handleCurrentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

  connect( lstResults, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ),
           this, SLOT( itemClicked( QTreeWidgetItem*, int ) ) );
}

QgsIdentifyResults::~QgsIdentifyResults()
{
  clearRubberbands();
  if ( mActionPopup )
    delete mActionPopup;
}

QTreeWidgetItem *QgsIdentifyResults::layerItem( QObject *layer )
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *item = lstResults->topLevelItem( i );

    if ( item->data( 0, Qt::UserRole ).value<QObject*>() == layer )
      return item;
  }

  return 0;
}

void QgsIdentifyResults::addFeature( QgsMapLayer *layer, int fid,
                                     QString displayField, QString displayValue,
                                     const QMap<QString, QString> &attributes,
                                     const QMap<QString, QString> &derivedAttributes )
{
  QTreeWidgetItem *layItem = layerItem( layer );
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );

  if ( layItem == 0 )
  {
    layItem = new QTreeWidgetItem( QStringList() << layer->name() << tr( "Layer" ) );
    layItem->setData( 0, Qt::UserRole, QVariant::fromValue( qobject_cast<QObject *>( layer ) ) );
    lstResults->addTopLevelItem( layItem );

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vlayer )
    {
      connect( vlayer, SIGNAL( layerDeleted() ), this, SLOT( layerDestroyed() ) );
      connect( vlayer, SIGNAL( layerCrsChanged() ), this, SLOT( layerDestroyed() ) );
      connect( vlayer, SIGNAL( featureDeleted( int ) ), this, SLOT( featureDeleted( int ) ) );
      connect( vlayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
      connect( vlayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
    }
    else
    {
      connect( layer, SIGNAL( destroyed() ), this, SLOT( layerDestroyed() ) );
      connect( layer, SIGNAL( layerCrsChanged() ), this, SLOT( layerDestroyed() ) );
    }
  }

  QTreeWidgetItem *featItem = new QTreeWidgetItem( QStringList() << displayField << displayValue );
  featItem->setData( 0, Qt::UserRole, fid );

  for ( QMap<QString, QString>::const_iterator it = attributes.begin(); it != attributes.end(); it++ )
  {
    featItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
  }

  if ( derivedAttributes.size() >= 0 )
  {
    QTreeWidgetItem *derivedItem = new QTreeWidgetItem( QStringList() << tr( "(Derived)" ) );
    derivedItem->setData( 0, Qt::UserRole, "derived" );
    featItem->addChild( derivedItem );

    for ( QMap< QString, QString>::const_iterator it = derivedAttributes.begin(); it != derivedAttributes.end(); it++ )
    {
      derivedItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
    }
  }

  if ( vlayer )
  {
    QTreeWidgetItem *actionItem = new QTreeWidgetItem( QStringList() << tr( "(Actions)" ) );
    actionItem->setData( 0, Qt::UserRole, "actions" );
    featItem->addChild( actionItem );

    QTreeWidgetItem *editItem = new QTreeWidgetItem( QStringList() << "" << ( vlayer->isEditable() ? tr( "Edit feature form" ) : tr( "View feature form" ) ) );
    editItem->setIcon( 0, QgisApp::getThemeIcon( vlayer->isEditable() ? "/mIconEditable.png" : "/mIconEditable.png" ) );
    editItem->setData( 0, Qt::UserRole, "edit" );
    actionItem->addChild( editItem );

    for ( int i = 0; i < vlayer->actions()->size(); i++ )
    {
      const QgsAction &action = vlayer->actions()->at( i );

      if ( !action.runable() )
        continue;

      QTreeWidgetItem *twi = new QTreeWidgetItem( QStringList() << "" << action.name() );
      twi->setIcon( 0, QgisApp::getThemeIcon( "/mAction.png" ) );
      twi->setData( 0, Qt::UserRole, "action" );
      twi->setData( 0, Qt::UserRole + 1, QVariant::fromValue( i ) );
      actionItem->addChild( twi );
    }
  }

  layItem->addChild( featItem );

  highlightFeature( featItem );
}

void QgsIdentifyResults::editingToggled()
{
  QTreeWidgetItem *layItem = layerItem( sender() );
  QgsVectorLayer *vlayer = vectorLayer( layItem );
  if ( !layItem || !vlayer )
    return;

  // iterate features
  int i;
  for ( i = 0; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *featItem = layItem->child( i );

    int j;
    for ( j = 0; j < featItem->childCount() && featItem->child( j )->data( 0, Qt::UserRole ).toString() != "actions"; j++ )
      QgsDebugMsg( QString( "%1: skipped %2" ).arg( featItem->child( j )->data( 0, Qt::UserRole ).toString() ) );

    if ( j == featItem->childCount() || featItem->child( j )->childCount() < 1 )
      continue;

    QTreeWidgetItem *actions = featItem->child( j );

    for ( j = 0; i < actions->childCount() && actions->child( j )->data( 0, Qt::UserRole ).toString() != "edit"; j++ )
      ;

    if ( j == actions->childCount() )
      continue;

    QTreeWidgetItem *editItem = actions->child( j );
    editItem->setIcon( 0, QgisApp::getThemeIcon( vlayer->isEditable() ? "/mIconEditable.png" : "/mIconEditable.png" ) );
    editItem->setText( 1, vlayer->isEditable() ? tr( "Edit feature form" ) : tr( "View feature form" ) );
  }
}

// Call to show the dialog box.
void QgsIdentifyResults::show()
{
  // Enfore a few things before showing the dialog box
  lstResults->sortItems( 0, Qt::AscendingOrder );
  expandColumnsToFit();

  if ( lstResults->topLevelItemCount() > 0 )
  {
    QTreeWidgetItem *layItem = lstResults->topLevelItem( 0 );
    QTreeWidgetItem *featItem = layItem->child( 0 );

    if ( lstResults->topLevelItemCount() == 1 &&
         layItem->childCount() == 1 &&
         QSettings().value( "/Map/identifyAutoFeatureForm", false ).toBool() )
    {
      QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( layItem->data( 0, Qt::UserRole ).value<QObject *>() );
      if ( layer )
      {
        // if this is the only feature and it's on a vector layer
        // don't show the form dialog instead of the results window
        lstResults->setCurrentItem( featItem );
        featureForm();
        return;
      }
    }

    // expand first layer and feature
    featItem->setExpanded( true );
    layItem->setExpanded( true );
  }

  QDialog::show();
  raise();
}

// Slot called when user clicks the Close button
// (saves the current window size/position)
void QgsIdentifyResults::close()
{
  clear();

  delete mActionPopup;
  mActionPopup = 0;

  saveWindowLocation();
  done( 0 );
  if ( mDock )
    mDock->close();
}

// Save the current window size/position before closing
// from window menu or X in titlebar
void QgsIdentifyResults::closeEvent( QCloseEvent *e )
{
  // We'll close in our own good time thanks...
  e->ignore();
  close();
}

void QgsIdentifyResults::itemClicked( QTreeWidgetItem *item, int column )
{
  if ( item->data( 0, Qt::UserRole ).toString() == "edit" )
  {
    lstResults->setCurrentItem( item );
    featureForm();
  }
  else if ( item->data( 0, Qt::UserRole ).toString() == "action" )
  {
    doAction( item, item->data( 0, Qt::UserRole + 1 ).toInt() );
  }
}

// Popup (create if necessary) a context menu that contains a list of
// actions that can be applied to the data in the identify results
// dialog box.

void QgsIdentifyResults::contextMenuEvent( QContextMenuEvent* event )
{
  QTreeWidgetItem *item = lstResults->itemAt( lstResults->viewport()->mapFrom( this, event->pos() ) );
  // if the user clicked below the end of the attribute list, just return
  if ( !item )
    return;

  QgsVectorLayer *vlayer = vectorLayer( item );
  if ( vlayer == 0 )
    return;

  if ( mActionPopup )
    delete mActionPopup;

  mActionPopup = new QMenu();

  mActionPopup->addAction( vlayer->isEditable() ? tr( "Edit feature form" ) : tr( "View feature form" ), this, SLOT( featureForm() ) );
  mActionPopup->addAction( tr( "Zoom to feature" ), this, SLOT( zoomToFeature() ) );
  mActionPopup->addAction( tr( "Copy attribute value" ), this, SLOT( copyAttributeValue() ) );
  mActionPopup->addAction( tr( "Copy feature attributes" ), this, SLOT( copyFeatureAttributes() ) );
  mActionPopup->addSeparator();
  mActionPopup->addAction( tr( "Clear results" ), this, SLOT( clear() ) );
  mActionPopup->addAction( tr( "Clear highlights" ), this, SLOT( clearRubberbands() ) );
  mActionPopup->addAction( tr( "Highlight all" ), this, SLOT( highlightAll() ) );
  mActionPopup->addAction( tr( "Highlight layer" ), this, SLOT( highlightLayer() ) );
  mActionPopup->addSeparator();
  mActionPopup->addAction( tr( "Expand all" ), this, SLOT( expandAll() ) );
  mActionPopup->addAction( tr( "Collapse all" ), this, SLOT( collapseAll() ) );

  if ( vlayer->actions()->size() > 0 )
  {
    mActionPopup->addSeparator();

    // The assumption is made that an instance of QgsIdentifyResults is
    // created for each new Identify Results dialog box, and that the
    // contents of the popup menu doesn't change during the time that
    // such a dialog box is around.
    QAction *a = mActionPopup->addAction( tr( "Run action" ) );
    a->setEnabled( false );

    for ( int i = 0; i < vlayer->actions()->size(); i++ )
    {
      const QgsAction &action = vlayer->actions()->at( i );

      if ( !action.runable() )
        continue;

      QgsFeatureAction *a = new QgsFeatureAction( action.name(), this, vlayer, i, featureItem( item ) );
      mActionPopup->addAction( action.name(), a, SLOT( execute() ) );
    }
  }

  mActionPopup->popup( event->globalPos() );
}

// Restore last window position/size and show the window
void QgsIdentifyResults::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "/Windows/Identify/geometry" ).toByteArray() );
  show();
}

// Save the current window location (store in ~/.qt/qgisrc)
void QgsIdentifyResults::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/Windows/Identify/geometry", saveGeometry() );
}

void QgsIdentifyResults::setColumnText( int column, const QString & label )
{
  QTreeWidgetItem* header = lstResults->headerItem();
  header->setText( column, label );
}

void QgsIdentifyResults::expandColumnsToFit()
{
  lstResults->resizeColumnToContents( 0 );
  lstResults->resizeColumnToContents( 1 );
}

void QgsIdentifyResults::clearRubberbands()
{
  foreach( QgsRubberBand *rb, mRubberBands )
  {
    delete rb;
  }

  mRubberBands.clear();
}

void QgsIdentifyResults::clear()
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    disconnectLayer( lstResults->topLevelItem( i )->data( 0, Qt::UserRole ).value<QObject *>() );
  }

  lstResults->clear();
  clearRubberbands();
}

void QgsIdentifyResults::activate()
{
#if 0
  foreach( QgsRubberBand *rb, mRubberBands )
  {
    rb->show();
  }
#endif

  if ( lstResults->topLevelItemCount() > 0 )
  {
    show();
    raise();
  }
}

void QgsIdentifyResults::deactivate()
{
#if 0
  foreach( QgsRubberBand *rb, mRubberBands )
  {
    rb->hide();
  }
#endif
}

void QgsIdentifyResults::doAction( QTreeWidgetItem *item, int action )
{
  int idx;
  QList< QPair<QString, QString> > attributes;
  QTreeWidgetItem *featItem = retrieveAttributes( item, attributes, idx );
  if ( !featItem )
    return;

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( featItem->parent()->data( 0, Qt::UserRole ).value<QObject *>() );
  if ( !layer )
    return;

  idx = -1;
  if ( item->parent() == featItem )
  {
    QString fieldName = item->data( 0, Qt::DisplayRole ).toString();

    for ( QgsFieldMap::const_iterator it = layer->pendingFields().begin(); it != layer->pendingFields().end(); it++ )
    {
      if ( it->name() == fieldName )
      {
        idx = it.key();
        break;
      }
    }
  }

  layer->actions()->doAction( action, attributes, idx );
}

QTreeWidgetItem *QgsIdentifyResults::featureItem( QTreeWidgetItem *item )
{
  QTreeWidgetItem *featItem;
  if ( item->parent() )
  {
    if ( item->parent()->parent() )
    {
      if ( item->parent()->parent()->parent() )
      {
        // derived or action attribute item
        featItem = item->parent()->parent();
      }
      else
      {
        // attribute item
        featItem = item->parent();
      }
    }
    else
    {
      // feature item
      featItem = item;
    }
  }
  else
  {
    // layer item
    if ( item->childCount() > 1 )
      return 0;

    featItem = item->child( 0 );
  }

  return featItem;
}

QTreeWidgetItem *QgsIdentifyResults::layerItem( QTreeWidgetItem *item )
{
  if ( item && item->parent() )
  {
    item = featureItem( item )->parent();
  }

  return item;
}


QgsVectorLayer *QgsIdentifyResults::vectorLayer( QTreeWidgetItem *item )
{
  item = layerItem( item );
  if ( !item )
    return NULL;
  return qobject_cast<QgsVectorLayer *>( item->data( 0, Qt::UserRole ).value<QObject *>() );
}


QTreeWidgetItem *QgsIdentifyResults::retrieveAttributes( QTreeWidgetItem *item, QList< QPair<QString, QString> > &attributes, int &idx )
{
  QTreeWidgetItem *featItem = featureItem( item );

  idx = -1;

  attributes.clear();
  for ( int i = 0; i < featItem->childCount(); i++ )
  {
    QTreeWidgetItem *item = featItem->child( i );
    if ( item->childCount() > 0 )
      continue;
    if ( item == lstResults->currentItem() )
      idx = attributes.size();
    attributes << QPair<QString, QString>( item->data( 0, Qt::DisplayRole ).toString(), item->data( 1, Qt::DisplayRole ).toString() );
  }

  return featItem;
}

void QgsIdentifyResults::itemExpanded( QTreeWidgetItem* item )
{
  expandColumnsToFit();
}

void QgsIdentifyResults::handleCurrentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous )
{
  if ( current == NULL )
  {
    emit selectedFeatureChanged( 0, 0 );
    return;
  }

  QTreeWidgetItem *layItem = layerItem( current );

  if ( current == layItem )
  {
    highlightLayer( layItem );
  }
  else
  {
    clearRubberbands();
    highlightFeature( current );
  }
}

void QgsIdentifyResults::layerDestroyed()
{
  QObject *theSender = sender();

  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *layItem = lstResults->topLevelItem( i );

    if ( layItem->data( 0, Qt::UserRole ).value<QObject *>() == sender() )
    {
      for ( int j = 0; j < layItem->childCount(); j++ )
      {
        delete mRubberBands.take( layItem->child( i ) );
      }
    }
  }

  disconnectLayer( theSender );
  delete layerItem( theSender );

  if ( lstResults->topLevelItemCount() == 0 )
  {
    close();
  }
}

void QgsIdentifyResults::disconnectLayer( QObject *layer )
{
  if ( !layer )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( vlayer )
  {
    disconnect( vlayer, SIGNAL( layerDeleted() ), this, SLOT( layerDestroyed() ) );
    disconnect( vlayer, SIGNAL( featureDeleted( int ) ), this, SLOT( featureDeleted( int ) ) );
    disconnect( vlayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
    disconnect( vlayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
  }
  else
  {
    disconnect( layer, SIGNAL( destroyed() ), this, SLOT( layerDestroyed() ) );
  }
}

void QgsIdentifyResults::featureDeleted( int fid )
{
  QTreeWidgetItem *layItem = layerItem( sender() );

  if ( !layItem )
    return;

  for ( int i = 0; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *featItem = layItem->child( i );

    if ( featItem && featItem->data( 0, Qt::UserRole ).toInt() == fid )
    {
      delete mRubberBands.take( featItem );
      delete featItem;
      break;
    }
  }

  if ( layItem->childCount() == 0 )
  {
    delete layItem;
  }

  if ( lstResults->topLevelItemCount() == 0 )
  {
    close();
  }
}

void QgsIdentifyResults::highlightFeature( QTreeWidgetItem *item )
{
  QgsVectorLayer *layer = vectorLayer( item );
  if ( !layer )
    return;

  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return;

  if ( mRubberBands.contains( featItem ) )
    return;

  int fid = featItem->data( 0, Qt::UserRole ).toInt();

  QgsFeature feat;
  if ( !layer->featureAtId( fid, feat, true, false ) )
  {
    return;
  }

  if ( !feat.geometry() )
  {
    return;
  }

  QgsRubberBand *rb = new QgsRubberBand( mCanvas, feat.geometry()->type() == QGis::Polygon );
  if ( rb )
  {
    rb->setToGeometry( feat.geometry(), layer );
    rb->setWidth( 2 );
    rb->setColor( Qt::red );
    rb->show();
    mRubberBands.insert( featItem, rb );
  }
}

void QgsIdentifyResults::zoomToFeature()
{
  QTreeWidgetItem *item = lstResults->currentItem();

  QgsVectorLayer *layer = vectorLayer( item );
  if ( !layer )
    return;

  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return;

  int fid = featItem->data( 0, Qt::UserRole ).toInt();

  QgsFeature feat;
  if ( ! layer->featureAtId( fid, feat, true, false ) )
  {
    return;
  }

  if ( !feat.geometry() )
  {
    return;
  }

  QgsRectangle rect = mCanvas->mapRenderer()->layerExtentToOutputExtent( layer, feat.geometry()->boundingBox() );

  if ( rect.isEmpty() )
  {
    QgsPoint c = rect.center();
    rect = mCanvas->extent();
    rect.expand( 0.25, &c );
  }

  mCanvas->setExtent( rect );
  mCanvas->refresh();
}

void QgsIdentifyResults::featureForm()
{
  QTreeWidgetItem *item = lstResults->currentItem();

  QgsVectorLayer *vlayer = vectorLayer( item );
  if ( !vlayer )
    return;

  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return;

  int fid = featItem->data( 0, Qt::UserRole ).toInt();

  QgsFeature f;
  if ( !vlayer->featureAtId( fid, f ) )
    return;

  QgsAttributeMap src = f.attributeMap();

  if ( vlayer->isEditable() )
    vlayer->beginEditCommand( tr( "Attribute changed" ) );

  QgsAttributeDialog *ad = new QgsAttributeDialog( vlayer, &f );

  if ( !vlayer->isEditable() && vlayer->actions()->size() > 0 )
  {
    ad->dialog()->setContextMenuPolicy( Qt::ActionsContextMenu );

    QAction *a = new QAction( tr( "Run actions" ), ad->dialog() );
    a->setEnabled( false );
    ad->dialog()->addAction( a );

    for ( int i = 0; i < vlayer->actions()->size(); i++ )
    {
      const QgsAction &action = vlayer->actions()->at( i );

      if ( !action.runable() )
        continue;

      QgsFeatureAction *a = new QgsFeatureAction( action.name(), this, vlayer, i, featItem );
      ad->dialog()->addAction( a );
      connect( a, SIGNAL( triggered() ), a, SLOT( execute() ) );

      QPushButton *pb = ad->dialog()->findChild<QPushButton *>( action.name() );
      if ( pb )
        connect( pb, SIGNAL( clicked() ), a, SLOT( execute() ) );
    }
  }

  if ( vlayer->isEditable() )
  {
    if ( ad->exec() )
    {
      const QgsAttributeMap &dst = f.attributeMap();
      for ( QgsAttributeMap::const_iterator it = dst.begin(); it != dst.end(); it++ )
      {
        if ( !src.contains( it.key() ) || it.value() != src[it.key()] )
        {
          vlayer->changeAttributeValue( f.id(), it.key(), it.value() );
        }
      }
      vlayer->endEditCommand();
    }
    else
    {
      vlayer->destroyEditCommand();
    }

    delete ad;

    mCanvas->refresh();
  }
  else
  {
    QgsRubberBand *rb = mRubberBands.take( featItem );
    ad->setHighlight( rb );
    ad->show();
  }
}

void QgsIdentifyResults::highlightAll()
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *layItem = lstResults->topLevelItem( i );

    for ( int j = 0; j < layItem->childCount(); j++ )
    {
      highlightFeature( layItem->child( j ) );
    }
  }
}

void QgsIdentifyResults::highlightLayer()
{
  highlightLayer( lstResults->currentItem() );
}

void QgsIdentifyResults::highlightLayer( QTreeWidgetItem *item )
{
  QTreeWidgetItem *layItem = layerItem( item );
  if ( !layItem )
    return;

  clearRubberbands();

  for ( int i = 0; i < layItem->childCount(); i++ )
  {
    highlightFeature( layItem->child( i ) );
  }
}


void QgsIdentifyResults::expandAll()
{
  lstResults->expandAll();
}

void QgsIdentifyResults::collapseAll()
{
  lstResults->collapseAll();
}

void QgsIdentifyResults::copyAttributeValue()
{
  QClipboard *clipboard = QApplication::clipboard();
  QString text = lstResults->currentItem()->data( 1, Qt::DisplayRole ).toString();
  QgsDebugMsg( QString( "set clipboard: %1" ).arg( text ) );
  clipboard->setText( text );
}

void QgsIdentifyResults::copyFeatureAttributes()
{
  QClipboard *clipboard = QApplication::clipboard();
  QString text;

  int idx;
  QList< QPair<QString, QString> > attributes;
  retrieveAttributes( lstResults->currentItem(), attributes, idx );

  for ( QList< QPair<QString, QString> >::iterator it = attributes.begin(); it != attributes.end(); it++ )
  {
    text += QString( "%1: %2\n" ).arg( it->first ).arg( it->second );
  }

  QgsDebugMsg( QString( "set clipboard: %1" ).arg( text ) );
  clipboard->setText( text );
}
