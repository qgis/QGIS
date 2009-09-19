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
#include "qgscontexthelp.h"
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

#include "qgslogger.h"

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

// Tree hierachy
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
    mRubberBand( 0 ),
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
    buttonCancel->hide();
  }
  lstResults->setColumnCount( 2 );
  setColumnText( 0, tr( "Feature" ) );
  setColumnText( 1, tr( "Value" ) );

  connect( buttonCancel, SIGNAL( clicked() ),
           this, SLOT( close() ) );

  connect( lstResults, SIGNAL( itemExpanded( QTreeWidgetItem* ) ),
           this, SLOT( itemExpanded( QTreeWidgetItem* ) ) );

  connect( lstResults, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ),
           this, SLOT( handleCurrentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

  connect( lstResults, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ),
           this, SLOT( itemClicked( QTreeWidgetItem*, int ) ) );
}

QgsIdentifyResults::~QgsIdentifyResults()
{
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
  QgsVectorLayer *vlayer = dynamic_cast<QgsVectorLayer*>( layer );

  if ( layItem == 0 )
  {
    layItem = new QTreeWidgetItem( QStringList() << layer->name() << tr( "Layer" ) );
    layItem->setData( 0, Qt::UserRole, QVariant::fromValue( dynamic_cast<QObject*>( layer ) ) );
    lstResults->addTopLevelItem( layItem );

    QgsVectorLayer *vlayer = dynamic_cast<QgsVectorLayer*>( layer );
    if ( vlayer )
    {
      connect( vlayer, SIGNAL( layerDeleted() ), this, SLOT( layerDestroyed() ) );
      connect( vlayer, SIGNAL( featureDeleted( int ) ), this, SLOT( featureDeleted( int ) ) );
      connect( vlayer, SIGNAL( editingStarted() ), this, SLOT( addEditAction() ) );
      connect( vlayer, SIGNAL( editingStopped() ), this, SLOT( removeEditAction() ) );
    }
    else
    {
      connect( layer, SIGNAL( destroyed() ), this, SLOT( layerDestroyed() ) );
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

  if ( vlayer && (vlayer->actions()->size() > 0 || vlayer->isEditable() ) )
  {
    QTreeWidgetItem *actionItem = new QTreeWidgetItem( QStringList() << tr( "(Actions)" ) );
    actionItem->setData( 0, Qt::UserRole, "actions" );
    featItem->addChild( actionItem );

    QTreeWidgetItem *twi;

    if ( vlayer->isEditable() )
    {
      addEditAction( actionItem );
    }

    for ( int i = 0; i < vlayer->actions()->size(); i++ )
    {
      QgsAttributeAction::aIter iter = vlayer->actions()->retrieveAction( i );

      twi = new QTreeWidgetItem( QStringList() << "" << iter->name() );
      twi->setIcon( 0, QgisApp::getThemeIcon( "/mAction.png" ) );
      twi->setData( 0, Qt::UserRole, "action" );
      twi->setData( 0, Qt::UserRole + 1, QVariant::fromValue( i ) );
      actionItem->addChild( twi );
    }
  }

  layItem->addChild( featItem );
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

    if ( layItem->childCount() == 1 )
    {
      QgsVectorLayer *layer = dynamic_cast<QgsVectorLayer *>( layItem->data( 0, Qt::UserRole ).value<QObject *>() );
      if ( layer )
      {
        highlightFeature( featItem );

        if ( layer->isEditable() )
        {
          // if this is the only feature, it's on a vector layer and that layer is editable:
          // don't show the edit dialog instead of the results window
          editFeature( featItem );
          return;
        }
      }
    }

    // expand first layer and feature
    featItem->setExpanded( true );
    layItem->setExpanded( true );
  }

  QDialog::show();
}
// Slot called when user clicks the Close button
// (saves the current window size/position)
void QgsIdentifyResults::close()
{
  saveWindowLocation();
  done( 0 );
}
// Save the current window size/position before closing
// from window menu or X in titlebar
void QgsIdentifyResults::closeEvent( QCloseEvent *e )
{
  // We'll close in our own good time thanks...
  e->ignore();
  close();
}

void QgsIdentifyResults::addEditAction( QTreeWidgetItem *item )
{
  QTreeWidgetItem *editItem = new QTreeWidgetItem( QStringList() << "" << tr( "Edit feature" ) );
  editItem->setIcon( 0, QgisApp::getThemeIcon( "/mIconEditable.png" ) );
  editItem->setData( 0, Qt::UserRole, "edit" );
  item->addChild( editItem );
}

void QgsIdentifyResults::addOrRemoveEditAction( bool addItem )
{
  QTreeWidgetItem *layItem = layerItem( sender() );

  for ( int i = 0; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *featItem = layItem->child( i );
    QTreeWidgetItem *actionsItem = 0;

    for ( int j = 0; j < featItem->childCount(); j++ )
    {
      QTreeWidgetItem *attrItem = featItem->child( j );

      if ( attrItem->childCount() == 0 || attrItem->data( 0, Qt::UserRole ).toString() != "actions" )
        continue;

      actionsItem = attrItem;
      break;
    }

    if ( addItem )
    {
      if( !actionsItem )
      {
        actionsItem = new QTreeWidgetItem( QStringList() << tr( "(Actions)" ) );
        actionsItem->setData( 0, Qt::UserRole, "actions" );
        featItem->addChild( actionsItem );
      }

      addEditAction( actionsItem );
    }
    else if( actionsItem )
    {
      for ( int k = 0; k < actionsItem->childCount(); k++ )
      {
        QTreeWidgetItem *editItem = actionsItem->child( k );
        if ( editItem->data( 0, Qt::UserRole ).toString() != "edit" )
          continue;

        delete editItem;

        if( actionsItem->childCount()==0 )
          delete actionsItem;
         
        break;
      }
    }
    else
    {
      QgsDebugMsg( "actions item not found" );
    }
  }
}

void QgsIdentifyResults::addEditAction()
{
  addOrRemoveEditAction( true );
}

void QgsIdentifyResults::removeEditAction()
{
  addOrRemoveEditAction( false );
}

void QgsIdentifyResults::itemClicked( QTreeWidgetItem *item, int column )
{
  if ( item->data( 0, Qt::UserRole ).toString() == "edit" )
  {
    editFeature( item );
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

  if ( mActionPopup )
    delete mActionPopup;

  QgsVectorLayer *vlayer = vectorLayer( item );
  if ( vlayer == 0 )
    return;

  mActionPopup = new QMenu();

  QAction *a;

  if ( vlayer->isEditable() )
  {
    a = mActionPopup->addAction( tr( "Edit feature" ) );
    a->setEnabled( true );
    a->setData( QVariant::fromValue( -6 ) );
  }

  a = mActionPopup->addAction( tr( "Zoom to feature" ) );
  a->setEnabled( true );
  a->setData( QVariant::fromValue( -5 ) );

  a = mActionPopup->addAction( tr( "Copy attribute value" ) );
  a->setEnabled( true );
  a->setData( QVariant::fromValue( -4 ) );

  a = mActionPopup->addAction( tr( "Copy feature attributes" ) );
  a->setEnabled( true );
  a->setData( QVariant::fromValue( -3 ) );

  mActionPopup->addSeparator();

  a = mActionPopup->addAction( tr( "Expand all" ) );
  a->setEnabled( true );
  a->setData( QVariant::fromValue( -2 ) );

  a = mActionPopup->addAction( tr( "Collapse all" ) );
  a->setEnabled( true );
  a->setData( QVariant::fromValue( -1 ) );

  QgsAttributeAction *actions = vlayer->actions();
  if ( actions && actions->size() > 0 )
  {
    mActionPopup->addSeparator();

    // The assumption is made that an instance of QgsIdentifyResults is
    // created for each new Identify Results dialog box, and that the
    // contents of the popup menu doesn't change during the time that
    // such a dialog box is around.
    a = mActionPopup->addAction( tr( "Run action" ) );
    a->setEnabled( false );

    QgsAttributeAction::aIter iter = actions->begin();
    for ( int i = 0; iter != actions->end(); ++iter, ++i )
    {
      QAction* a = mActionPopup->addAction( iter->name() );
      a->setEnabled( true );
      // The menu action stores an integer that is used later on to
      // associate an menu action with an actual qgis action.
      a->setData( QVariant::fromValue( i ) );
    }
  }

  connect( mActionPopup, SIGNAL( triggered( QAction* ) ), this, SLOT( popupItemSelected( QAction* ) ) );

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

// Run the action that was selected in the popup menu
void QgsIdentifyResults::popupItemSelected( QAction* menuAction )
{
  QTreeWidgetItem *item = lstResults->currentItem();
  if ( item == 0 )
    return;

  int action = menuAction->data().toInt();

  if ( action < 0 )
  {
    switch ( action )
    {
      case -6:
        editFeature( item );
        break;

      case -5:
        zoomToFeature( item );
        break;

      case -4:
      case -3:
      {
        QClipboard *clipboard = QApplication::clipboard();
        QString text;

        if ( action == -4 )
        {
          text = item->data( 1, Qt::DisplayRole ).toString();
        }
        else
        {
          std::vector< std::pair<QString, QString> > attributes;
          retrieveAttributes( item, attributes );

          for ( std::vector< std::pair<QString, QString> >::iterator it = attributes.begin(); it != attributes.end(); it++ )
          {
            text += QString( "%1: %2\n" ).arg( it->first ).arg( it->second );
          }
        }

        QgsDebugMsg( QString( "set clipboard: %1" ).arg( text ) );
        clipboard->setText( text );
      }
      break;

      case -2:
        lstResults->expandAll();
        break;

      case -1:
        lstResults->collapseAll();
        break;

    }
  }
  else
  {
    doAction( item, action );
  }
}

void QgsIdentifyResults::expandColumnsToFit()
{
  lstResults->resizeColumnToContents( 0 );
  lstResults->resizeColumnToContents( 1 );
}

void QgsIdentifyResults::clear()
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    disconnectLayer( lstResults->topLevelItem( i )->data( 0, Qt::UserRole ).value<QObject *>() );
  }

  lstResults->clear();
  clearRubberBand();
}

void QgsIdentifyResults::activate()
{
  if ( mRubberBand )
  {
    mRubberBand->show();
  }

  if ( lstResults->topLevelItemCount() > 0 )
  {
    show();
    raise();
  }
}

void QgsIdentifyResults::deactivate()
{
  if ( mRubberBand )
  {
    mRubberBand->hide();
  }
}

void QgsIdentifyResults::clearRubberBand()
{
  if ( !mRubberBand )
    return;

  delete mRubberBand;
  mRubberBand = 0;
  mRubberBandLayer = 0;
  mRubberBandFid = 0;
}

void QgsIdentifyResults::doAction( QTreeWidgetItem *item, int action )
{
  std::vector< std::pair<QString, QString> > attributes;
  QTreeWidgetItem *featItem = retrieveAttributes( item, attributes );
  if ( !featItem )
    return;

  QgsVectorLayer *layer = dynamic_cast<QgsVectorLayer *>( featItem->parent()->data( 0, Qt::UserRole ).value<QObject *>() );
  if ( !layer )
    return;

  int idx = -1;
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

QgsVectorLayer *QgsIdentifyResults::vectorLayer( QTreeWidgetItem *item )
{
  if ( item->parent() )
  {
    item = featureItem( item )->parent();
  }

  return dynamic_cast<QgsVectorLayer *>( item->data( 0, Qt::UserRole ).value<QObject *>() );
}


QTreeWidgetItem *QgsIdentifyResults::retrieveAttributes( QTreeWidgetItem *item, std::vector< std::pair<QString, QString> > &attributes )
{
  QTreeWidgetItem *featItem = featureItem( item );

  attributes.clear();
  for ( int i = 0; i < featItem->childCount(); i++ )
  {
    QTreeWidgetItem *item = featItem->child( i );
    if ( item->childCount() > 0 )
      continue;
    attributes.push_back( std::make_pair( item->data( 0, Qt::DisplayRole ).toString(), item->data( 1, Qt::DisplayRole ).toString() ) );
  }

  return featItem;
}

void QgsIdentifyResults::on_buttonHelp_clicked()
{
  QgsContextHelp::run( context_id );
}

void QgsIdentifyResults::itemExpanded( QTreeWidgetItem* item )
{
  expandColumnsToFit();
}

void QgsIdentifyResults::handleCurrentItemChanged( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
  if ( current == NULL )
  {
    emit selectedFeatureChanged( 0, 0 );
    return;
  }

  highlightFeature( current );
}

void QgsIdentifyResults::layerDestroyed()
{
  QObject *theSender = sender();

  if ( mRubberBandLayer == theSender )
  {
    clearRubberBand();
  }

  disconnectLayer( theSender );
  delete layerItem( theSender );

  if ( lstResults->topLevelItemCount() == 0 )
  {
    hide();
  }
}

void QgsIdentifyResults::disconnectLayer( QObject *layer )
{
  if ( !layer )
    return;

  QgsVectorLayer *vlayer = dynamic_cast<QgsVectorLayer *>( layer );
  if ( vlayer )
  {
    disconnect( vlayer, SIGNAL( layerDeleted() ), this, SLOT( layerDestroyed() ) );
    disconnect( vlayer, SIGNAL( featureDeleted( int ) ), this, SLOT( featureDeleted( int ) ) );
    disconnect( vlayer, SIGNAL( editingStarted() ), this, SLOT( addEditAction() ) );
    disconnect( vlayer, SIGNAL( editingStopped() ), this, SLOT( removeEditAction() ) );
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
      if ( mRubberBandLayer == sender() && mRubberBandFid == fid )
        clearRubberBand();
      delete featItem;
      break;
    }
  }

  if ( layItem->childCount() == 0 )
  {
    if ( mRubberBandLayer == sender() )
      clearRubberBand();
    delete layItem;
  }

  if ( lstResults->topLevelItemCount() == 0 )
  {
    hide();
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

  int fid = featItem->data( 0, Qt::UserRole ).toInt();

  clearRubberBand();

  QgsFeature feat;
  if ( ! layer->featureAtId( fid, feat, true, false ) )
  {
    return;
  }

  if ( !feat.geometry() )
  {
    return;
  }

  mRubberBand = new QgsRubberBand( mCanvas, feat.geometry()->type() == QGis::Polygon );
  if ( mRubberBand )
  {
    mRubberBandLayer = layer;
    mRubberBandFid = fid;
    mRubberBand->setToGeometry( feat.geometry(), layer );
    mRubberBand->setWidth( 2 );
    mRubberBand->setColor( Qt::red );
    mRubberBand->show();
  }
}

void QgsIdentifyResults::zoomToFeature( QTreeWidgetItem *item )
{
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


void QgsIdentifyResults::editFeature( QTreeWidgetItem *item )
{
  QgsVectorLayer *layer = vectorLayer( item );
  if ( !layer || !layer->isEditable() )
    return;

  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return;

  int fid = featItem->data( 0, Qt::UserRole ).toInt();

  QgsFeature f;
  if ( ! layer->featureAtId( fid, f ) )
    return;

  QgsAttributeMap src = f.attributeMap();

  layer->beginEditCommand( tr( "Attribute changed" ) );
  QgsAttributeDialog *ad = new QgsAttributeDialog( layer, &f );
  if ( ad->exec() )
  {
    const QgsAttributeMap &dst = f.attributeMap();
    for ( QgsAttributeMap::const_iterator it = dst.begin(); it != dst.end(); it++ )
    {
      if ( !src.contains( it.key() ) || it.value() != src[it.key()] )
      {
        layer->changeAttributeValue( f.id(), it.key(), it.value() );
      }
    }
    layer->endEditCommand();
  }
  else
  {
    layer->destroyEditCommand();
  }

  delete ad;
  mCanvas->refresh();
}
