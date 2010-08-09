/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton                                      *
 *   aps02ts@macbuntu                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgslegend.h"
#include "qgslegendlayer.h"
#include "qgslegendsymbologyitem.h"
#include "qgslogger.h"

#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include "qgsrendererv2.h"
#include "qgssymbolv2.h"

#include "qgssinglesymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"

#include <iostream>
#include <QAction>
#include <QCoreApplication>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QFileDialog>


QgsLegendLayer::QgsLegendLayer( QgsMapLayer* layer )
    : QgsLegendItem( ),
    mLyr( layer )
{
  mType = LEGEND_LAYER;
  setFlags( Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  setCheckState( 0, Qt::Checked );
  setText( 0, layer->name() );
  setupFont();

  // Set the initial visibility flag for layers
  // This user option allows the user to turn off initial drawing of
  // layers when they are added to the map. This is useful when adding
  // many layers and the user wants to adjusty symbology, etc prior to
  // actually viewing the layer.
  QSettings settings;
  bool visible = settings.value( "/qgis/new_layers_visible", true ).toBool();
  mLyr.setVisible( visible );

  // not in overview by default
  mLyr.setInOverview( false );

  // setup connections that will update the layer icons
  if ( qobject_cast<QgsVectorLayer *>( layer ) )
  {
    QgsDebugMsg( "Connecting signals for updating icons, layer " + layer->name() );
    connect( layer, SIGNAL( editingStarted() ), this, SLOT( updateIcon() ) );
    connect( layer, SIGNAL( editingStopped() ), this, SLOT( updateIcon() ) );
  }
  connect( layer, SIGNAL( layerNameChanged() ), this, SLOT( layerNameChanged() ) );

  updateIcon();
  setToolTip( 0, layer->publicSource() );
}

QgsLegendLayer::~QgsLegendLayer()
{
  mType = LEGEND_LAYER;
}

void QgsLegendLayer::setupFont() //private method
{
  QFont myFont = font( 0 );
  myFont.setBold( true ); //visually differentiate layer labels from the rest
  setFont( 0, myFont );
}

bool QgsLegendLayer::isLeafNode()
{
  return false;
}

QgsLegendItem::DRAG_ACTION QgsLegendLayer::accept( LEGEND_ITEM_TYPE type )
{
  if ( type == LEGEND_LAYER || type == LEGEND_GROUP )
  {
    return REORDER;
  }
  else
  {
    return NO_ACTION;
  }
}

QgsLegendItem::DRAG_ACTION QgsLegendLayer::accept( const QgsLegendItem* li ) const
{
  if ( li && li != this )
  {
    LEGEND_ITEM_TYPE type = li->type();
    if ( type == LEGEND_LAYER )
    {
      //if(parent() == li->parent())
      //{
      return REORDER;
      //}
    }
    else if ( type == LEGEND_GROUP )
    {
      //only parent legend layers can change positions with groups
      if ( parent() == 0 )
      {
        return REORDER;
      }
    }
  }
  return NO_ACTION;
}


QgsMapLayer* QgsLegendLayer::layer()
{
  return mLyr.layer();
}

void QgsLegendLayer::refreshSymbology( const QString& key, double widthScale )
{
  QgsMapLayer* theMapLayer = QgsMapLayerRegistry::instance()->mapLayer( key );
  if ( !theMapLayer )
  {
    return;
  }

  if ( theMapLayer->type() == QgsMapLayer::VectorLayer ) // VECTOR
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( theMapLayer );
    if ( vlayer->isUsingRendererV2() )
      vectorLayerSymbologyV2( vlayer );
    else
      vectorLayerSymbology( vlayer, widthScale ); // get and change symbology
  }
  else if ( theMapLayer->type() == QgsMapLayer::RasterLayer ) // RASTER
  {
    QgsRasterLayer* rlayer = qobject_cast<QgsRasterLayer *>( theMapLayer );
    rasterLayerSymbology( rlayer ); // get and change symbology
  }

  updateIcon();
}

void QgsLegendLayer::changeSymbologySettings( const QgsMapLayer* theMapLayer,
    const SymbologyList& newSymbologyItems )
{
  if ( !theMapLayer )
  {
    return;
  }

  QgsLegendSymbologyItem* theSymbologyItem = 0;

  //remove the symbology items under the legend layer
  for ( int i = childCount(); i >= 0; --i )
  {
    theSymbologyItem = dynamic_cast<QgsLegendSymbologyItem *>( child( i ) );
    if ( theSymbologyItem )
    {
      delete takeChild( i );
    }
  }

  //add the new symbology items
  int childposition = 0; //position to insert the items
  for ( SymbologyList::const_iterator it = newSymbologyItems.begin(); it != newSymbologyItems.end(); ++it )
  {
    QgsLegendSymbologyItem* theItem = new QgsLegendSymbologyItem( it->second.width(), it->second.height() );
    theItem->setText( 0, it->first );
    theItem->setIcon( 0, QIcon( it->second ) );
    insertChild( childposition, theItem );

    ++childposition;
  }

}



void QgsLegendLayer::vectorLayerSymbology( const QgsVectorLayer* layer, double widthScale )
{
  SymbologyList itemList;

  //add the new items
  QString lw, uv, label;
  const QgsRenderer* renderer = layer->renderer();
  const QList<QgsSymbol*> sym = renderer->symbols();

  //create an item for each classification field (only one for most renderers)
  QSettings settings;
  if ( settings.value( "/qgis/showLegendClassifiers", false ).toBool() )
  {
    if ( renderer->needsAttributes() )
    {
      QgsAttributeList classfieldlist = renderer->classificationAttributes();
      const QgsFieldMap& fields = layer->pendingFields();
      for ( QgsAttributeList::iterator it = classfieldlist.begin(); it != classfieldlist.end(); ++it )
      {
        QString classfieldname = layer->attributeAlias( *it );
        if ( classfieldname.isEmpty() )
        {
          classfieldname = fields[*it].name();
        }
        itemList.append( qMakePair( classfieldname, QPixmap() ) );
      }
    }
  }

  for ( QList<QgsSymbol*>::const_iterator it = sym.begin(); it != sym.end(); ++it )
  {
    QImage img;
    if (( *it )->type() == QGis::Point )
    {
      img = ( *it )->getPointSymbolAsImage( widthScale );
    }
    else if (( *it )->type() == QGis::Line )
    {
      img = ( *it )->getLineSymbolAsImage();
    }
    else //polygon
    {
      img = ( *it )->getPolygonSymbolAsImage();
    }

    QString values;
    lw = ( *it )->lowerValue();
    if ( !lw.isEmpty() )
    {
      values += lw;
    }
    uv = ( *it )->upperValue();
    if ( !uv.isEmpty() && lw != uv )
    {
      values += " - ";
      values += uv;
    }
    label = ( *it )->label();
    if ( !label.isEmpty() )
    {
      values += " ";
      values += label;
    }

    QPixmap pix = QPixmap::fromImage( img ); // convert to pixmap
    itemList.append( qMakePair( values, pix ) );
  }

  changeSymbologySettings( layer, itemList );
}


void QgsLegendLayer::vectorLayerSymbologyV2( QgsVectorLayer* layer )
{
  QSize iconSize( 16, 16 );

#if 0 // unused
  QSettings settings;
  bool showClassifiers = settings.value( "/qgis/showLegendClassifiers", false ).toBool();
#endif

  SymbologyList itemList = layer->rendererV2()->legendSymbologyItems( iconSize );

  changeSymbologySettings( layer, itemList );
}

void QgsLegendLayer::rasterLayerSymbology( QgsRasterLayer* layer )
{
  SymbologyList itemList;
  QPixmap legendpixmap = layer->legendAsPixmap( true ).scaled( 20, 20, Qt::KeepAspectRatio );
  itemList.append( qMakePair( QString(), legendpixmap ) );

  changeSymbologySettings( layer, itemList );
}

void QgsLegendLayer::updateIcon()
{
  QPixmap newIcon( getOriginalPixmap() );

  QgsMapLayer* theLayer = layer();

  //overview
  // FIXME: overview icon is missing
  /*
  if ( theFile->isInOverview() )
  {
    // Overlay the overview icon on the default icon
    QPixmap myPixmap = QgisApp::getThemePixmap(  "/mIconOverview.png" );
    QPainter p( &newIcon );
    p.drawPixmap( 0, 0, myPixmap );
    p.end();
  }*/

  //editable
  if ( theLayer->isEditable() )
  {
    QPixmap myPixmap = QgisApp::getThemePixmap( "/mIconEditable.png" );
    // use editable icon instead of the layer's type icon
    newIcon = myPixmap;

    // Overlay the editable icon on the default icon
    /*QPainter p( &newIcon );
    p.drawPixmap( 0, 0, myPixmap );
    p.end();*/
  }

  // TODO: projection error icon?

  QIcon theIcon( newIcon );
  setIcon( 0, theIcon );
}

QPixmap QgsLegendLayer::getOriginalPixmap()
{
  QgsMapLayer* theLayer = layer();
  if ( theLayer )
  {

    if ( theLayer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( theLayer );
      switch ( vlayer->geometryType() )
      {
        case QGis::Point:
          return QgisApp::getThemePixmap( "/mIconPointLayer.png" );
          break;
        case QGis::Line:
          return QgisApp::getThemePixmap( "/mIconLineLayer.png" );
          break;
        case QGis::Polygon:
          return QgisApp::getThemePixmap( "/mIconPolygonLayer.png" );
          break;
        default:
          return QgisApp::getThemePixmap( "/mIconLayer.png" );
      }
    }
    else if ( theLayer->type() == QgsMapLayer::RasterLayer )
    {
      QSettings s;
      if( s.value( "/qgis/createRasterLegendIcons", true ).toBool() )
      {
        QgsRasterLayer* rlayer = qobject_cast<QgsRasterLayer *>( theLayer );
        QPixmap myPixmap( 32, 32 );
        rlayer->thumbnailAsPixmap( &myPixmap );
        return myPixmap;
      }
      else
      {
        return QPixmap();
      }
    }
  }

  // undefined - should never reach this
  return QgisApp::getThemePixmap( "/mIconLayer.png" );
}

void QgsLegendLayer::addToPopupMenu( QMenu& theMenu )
{
  QgsMapLayer *lyr = layer();
  QAction *toggleEditingAction = QgisApp::instance()->actionToggleEditing();

  // zoom to layer extent
  theMenu.addAction( QgisApp::getThemeIcon( "/mActionZoomToLayer.png" ),
                     tr( "&Zoom to layer extent" ), legend(), SLOT( legendLayerZoom() ) );
  if ( lyr->type() == QgsMapLayer::RasterLayer )
  {
    theMenu.addAction( tr( "&Zoom to best scale (100%)" ), legend(), SLOT( legendLayerZoomNative() ) );

    QgsRasterLayer *rasterLayer =  qobject_cast<QgsRasterLayer *>( lyr );
    if( rasterLayer && rasterLayer->rasterType() != QgsRasterLayer::Palette )
    {
      theMenu.addAction( tr( "&Stretch using current extent" ), legend(), SLOT( legendLayerStretchUsingCurrentExtent() ) );
    }
  }

  // show in overview
  QAction* showInOverviewAction = theMenu.addAction( tr( "&Show in overview" ), this, SLOT( showInOverview() ) );
  showInOverviewAction->setCheckable( true );
  showInOverviewAction->blockSignals( true );
  showInOverviewAction->setChecked( mLyr.isInOverview() );
  showInOverviewAction->blockSignals( false );

  // remove from canvas
  theMenu.addAction( QgisApp::getThemeIcon( "/mActionRemove.png" ), tr( "&Remove" ), QgisApp::instance(), SLOT( removeLayer() ) );

  theMenu.addSeparator();

  if ( lyr->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( lyr );

    // attribute table
    theMenu.addAction( tr( "&Open attribute table" ),
                       QgisApp::instance(), SLOT( attributeTable() ) );

    // allow editing
    int cap = vlayer->dataProvider()->capabilities();
    if ( cap & QgsVectorDataProvider::EditingCapabilities )
    {
      if ( toggleEditingAction )
      {
        theMenu.addAction( toggleEditingAction );
        toggleEditingAction->setChecked( vlayer->isEditable() );
      }
    }

    // save as vector file
    theMenu.addAction( tr( "Save as..." ), QgisApp::instance(), SLOT( saveAsVectorFile() ) );

    // save selection as vector file
    QAction* saveSelectionAsAction = theMenu.addAction( tr( "Save selection as..." ), QgisApp::instance(), SLOT( saveSelectionAsVectorFile() ) );
    if ( vlayer->selectedFeatureCount() == 0 )
    {
      saveSelectionAsAction->setEnabled( false );
    }

    theMenu.addAction( tr( "&Query..." ), QgisApp::instance(), SLOT( layerSubsetString() ) );

    theMenu.addSeparator();
  }

  // properties goes on bottom of menu for consistency with normal ui standards
  // e.g. kde stuff
  theMenu.addAction( tr( "&Properties" ), QgisApp::instance(), SLOT( layerProperties() ) );
}

//////////

void QgsLegendLayer::setVisible( bool visible )
{
  mLyr.setVisible( visible );
}

bool QgsLegendLayer::isVisible()
{
  return mLyr.isVisible();
}


void QgsLegendLayer::setInOverview( bool isInOverview )
{
  mLyr.setInOverview( isInOverview );
}

bool QgsLegendLayer::isInOverview()
{
  return mLyr.isInOverview();
}

void QgsLegendLayer::showInOverview()
{
  // toggle current status
  setInOverview( ! isInOverview() );

  legend()->updateMapCanvasLayerSet();
  legend()->updateOverview();
}

QString QgsLegendLayer::nameFromLayer( QgsMapLayer* layer )
{
  QString sourcename = layer->publicSource(); //todo: move this duplicated code into a new function
  if ( sourcename.startsWith( "host", Qt::CaseInsensitive ) )
  {
    //this layer is a database layer
    //modify source string such that password is not visible
    sourcename = layer->name();
  }
  else
  {
    //modify source name such that only the file is visible
    sourcename = layer->publicSource().section( '/', -1, -1 );
  }
  return sourcename;
}

QgsMapCanvasLayer& QgsLegendLayer::canvasLayer()
{
  return mLyr;
}

void QgsLegendLayer::layerNameChanged()
{
  QString name = mLyr.layer()->name();
  setText( 0, name );
}
