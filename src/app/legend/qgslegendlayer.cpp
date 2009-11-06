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
#include "qgslegendlayerfile.h"
#include "qgslegendlayerfilegroup.h"
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

QgsLegendLayer::QgsLegendLayer( QTreeWidgetItem* parent, QString name )
    : QgsLegendItem( parent, name )
{
  mType = LEGEND_LAYER;
  setFlags( Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  setCheckState( 0, Qt::Checked );
  setText( 0, name );
  setupFont();
}

QgsLegendLayer::QgsLegendLayer( QTreeWidget* parent, QString name ): QgsLegendItem( parent, name )
{
  mType = LEGEND_LAYER;
  setFlags( Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  setCheckState( 0, Qt::Checked );
  setText( 0, name );
  setupFont();
}

QgsLegendLayer::QgsLegendLayer( QString name ): QgsLegendItem()
{
  mType = LEGEND_LAYER;
  setFlags( Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  setCheckState( 0, Qt::Checked );
  setText( 0, name );
  setupFont();
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

QgsLegendLayerFile* QgsLegendLayer::firstLayerFile() const
{
  //first find the legend layer file group
  QgsLegendLayerFileGroup* llfg = 0;
  for ( int i = 0; i < childCount(); ++i )
  {
    llfg = dynamic_cast<QgsLegendLayerFileGroup *>( child( i ) );
    if ( llfg )
    {
      break;
    }
  }

  if ( !llfg )
  {
    return 0;
  }

  QTreeWidgetItem* llf = llfg->child( 0 );
  if ( !llf )
  {
    return 0;
  }
  QgsLegendLayerFile* legendlayerfile = dynamic_cast<QgsLegendLayerFile *>( llf );
  return legendlayerfile;
}

QgsMapLayer* QgsLegendLayer::firstMapLayer() const
{
  QgsLegendLayerFile* llf = firstLayerFile();
  if ( llf )
  {
    return llf->layer();
  }
  else
  {
    return 0;
  }
}

std::list<QgsMapLayer*> QgsLegendLayer::mapLayers()
{
  std::list<QgsMapLayer*> list;
  std::list<QgsLegendLayerFile*> llist = legendLayerFiles();
  for ( std::list<QgsLegendLayerFile*>::iterator it = llist.begin(); it != llist.end(); ++it )
  {
    list.push_back(( *it )->layer() );
  }
  return list;
}

std::list<QgsLegendLayerFile*> QgsLegendLayer::legendLayerFiles()
{
  std::list<QgsLegendLayerFile*> list;

  //find the layer file group
  QgsLegendLayerFileGroup* theLayerGroup = 0;
  for ( int i = 0; i < childCount(); ++i )
  {
    theLayerGroup = dynamic_cast<QgsLegendLayerFileGroup *>( child( i ) );
    if ( theLayerGroup )
    {
      break;
    }
  }

  //add all the legend layer files in the group
  if ( theLayerGroup )
  {
    QgsLegendLayerFile* theFile = 0;
    for ( int i = 0; i < theLayerGroup->childCount(); ++i )
    {
      theFile = dynamic_cast<QgsLegendLayerFile *>( theLayerGroup->child( i ) );
      if ( theFile )
      {
        list.push_back( theFile );
      }
    }
  }
  return list;
}

void QgsLegendLayer::updateLayerSymbologySettings( const QgsMapLayer* mapLayer )
{
  if ( mapLayer )
  {
    //find all layers
    std::list<QgsMapLayer*> theMapLayers = mapLayers();
    for ( std::list<QgsMapLayer*>::iterator it = theMapLayers.begin(); it != theMapLayers.end(); ++it )
    {
      if (( *it ) != mapLayer )
      {
        ( *it )->copySymbologySettings( *mapLayer );
      }
    }
    // source might have changed - e.g. other subset
    setToolTip( 0, mapLayer->publicSource() );
  }
}

void QgsLegendLayer::updateCheckState()
{
  std::list<QgsLegendLayerFile*> llfiles = legendLayerFiles();
  if ( llfiles.size() < 1 )
  {
    return;
  }

  std::list<QgsLegendLayerFile*>::iterator iter = llfiles.begin();
  Qt::CheckState theState = ( *iter )->checkState( 0 );
  for ( ; iter != llfiles.end(); ++iter )
  {
    if ( theState != ( *iter )->checkState( 0 ) )
    {
      theState = Qt::PartiallyChecked;
      break;
    }
  }

  if ( theState != checkState( 0 ) )
  {
    treeWidget()->blockSignals( true );
    setCheckState( 0, theState );
    //notify the legend that the check state has changed
    legend()->updateCheckStates( this, theState );
    treeWidget()->blockSignals( false );
  }
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
  else // RASTER
  {
    QgsRasterLayer* rlayer = qobject_cast<QgsRasterLayer *>( theMapLayer );
    rasterLayerSymbology( rlayer ); // get and change symbology
  }
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

  //copy the legend settings for the other layer files in the same legend layer
  updateLayerSymbologySettings( theMapLayer );

}



void QgsLegendLayer::vectorLayerSymbology( const QgsVectorLayer* layer, double widthScale )
{
  SymbologyList itemList;

  //add the new items
  QString lw, uv, label;
  const QgsRenderer* renderer = layer->renderer();
  const QList<QgsSymbol*> sym = renderer->symbols();

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
    itemList.push_back( std::make_pair( values, pix ) );
  }


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
        itemList.push_front( std::make_pair( classfieldname, QPixmap() ) );
      }
    }
  }

  changeSymbologySettings( layer, itemList );
}

static QPixmap _symbolPreviewPixmap(QgsSymbolV2* sym, QSize iconSize)
{
  QPainter p;
  QPixmap pix(iconSize);
  pix.fill(Qt::white);
  p.begin(&pix);
  p.setRenderHint(QPainter::Antialiasing);
  sym->drawPreviewIcon(&p, iconSize);
  p.end();
  return pix;
}

void QgsLegendLayer::vectorLayerSymbologyV2( QgsVectorLayer* layer )
{
  SymbologyList itemList;

  QSize iconSize(16,16);

  QSettings settings;
  bool showClassifiers = settings.value( "/qgis/showLegendClassifiers", false ).toBool();

  QgsFeatureRendererV2* renderer = layer->rendererV2();
  QString rendererType = renderer->type();
  if (rendererType == "singleSymbol")
  {
   QgsSingleSymbolRendererV2* r = static_cast<QgsSingleSymbolRendererV2*>(renderer);
    QPixmap pix = _symbolPreviewPixmap(r->symbol(), iconSize);

    itemList.push_back( std::make_pair( "", pix ) );
  }
  else if (rendererType == "categorizedSymbol")
  {
    QgsCategorizedSymbolRendererV2* r = static_cast<QgsCategorizedSymbolRendererV2*>(renderer);
    if (showClassifiers)
    {
      itemList.push_back( std::make_pair( r->classAttribute(), QPixmap() ) );
    }

    int count = r->categories().count();
    for (int i = 0; i < count; i++)
    {
      const QgsRendererCategoryV2& cat = r->categories()[i];
      QPixmap pix = _symbolPreviewPixmap( cat.symbol(), iconSize );
      itemList.push_back( std::make_pair( cat.label(), pix ) );
    }
  }
  else if (rendererType == "graduatedSymbol")
  {
    QgsGraduatedSymbolRendererV2* r = static_cast<QgsGraduatedSymbolRendererV2*>(renderer);
    if (showClassifiers)
    {
      itemList.push_back( std::make_pair( r->classAttribute(), QPixmap() ) );
    }

    int count = r->ranges().count();
    for (int i = 0; i < count; i++)
    {
      const QgsRendererRangeV2& range = r->ranges()[i];
      QPixmap pix = _symbolPreviewPixmap( range.symbol(), iconSize );
      itemList.push_back( std::make_pair( range.label(), pix ) );
    }
  }
  else
  {
    // nothing for unknown renderers
  }

  changeSymbologySettings( layer, itemList );
}

void QgsLegendLayer::rasterLayerSymbology( QgsRasterLayer* layer )
{
  SymbologyList itemList;
  QPixmap legendpixmap = layer->legendAsPixmap( true ).scaled( 20, 20, Qt::KeepAspectRatio );
  itemList.push_back( std::make_pair( "", legendpixmap ) );

  changeSymbologySettings( layer, itemList );

  updateIcon();
}

void QgsLegendLayer::updateIcon()
{
  QPixmap newIcon( getOriginalPixmap() );

  QgsMapLayer* theLayer = firstMapLayer();

  if ( mapLayers().size() == 1 )
  {
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
  }

  QIcon theIcon( newIcon );
  setIcon( 0, theIcon );
}

QPixmap QgsLegendLayer::getOriginalPixmap() const
{
  QgsMapLayer* firstLayer = firstMapLayer();
  if ( firstLayer )
  {
    if ( firstLayer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( firstLayer );
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
    else if ( firstLayer->type() == QgsMapLayer::RasterLayer )
    {
      QgsRasterLayer* rlayer = qobject_cast<QgsRasterLayer *>( firstLayer );
      QPixmap myPixmap( 32, 32 );
      rlayer->thumbnailAsPixmap( &myPixmap );
      return myPixmap;
    }
  }

  // undefined - should never reach this
  return QgisApp::getThemePixmap( "/mIconLayer.png" );
}

void QgsLegendLayer::addToPopupMenu( QMenu& theMenu, QAction* toggleEditingAction )
{
  std::list<QgsLegendLayerFile*> files = legendLayerFiles();
  QgsMapLayer* firstLayer = NULL;
  if ( files.size() > 0 )
  {
    firstLayer = files.front()->layer();
  }

  // zoom to layer extent
  theMenu.addAction( QgisApp::getThemeIcon( "/mActionZoomToLayer.png" ),
                     tr( "&Zoom to layer extent" ), legend(), SLOT( legendLayerZoom() ) );
  if ( firstLayer && firstLayer->type() == QgsMapLayer::RasterLayer )
  {
    theMenu.addAction( tr( "&Zoom to best scale (100%)" ), legend(), SLOT( legendLayerZoomNative() ) );
  }

  // show in overview
  QAction* showInOverviewAction = theMenu.addAction( tr( "&Show in overview" ), this, SLOT( showInOverview() ) );
  showInOverviewAction->setCheckable( true ); // doesn't support tristate
  showInOverviewAction->setChecked( isInOverview() );

  // remove from canvas
  theMenu.addAction( QgisApp::getThemeIcon( "/mActionRemove.png" ),
                     tr( "&Remove" ), legend(), SLOT( legendLayerRemove() ) );

  theMenu.addSeparator();

  if ( firstLayer && firstLayer->type() == QgsMapLayer::VectorLayer )
  {
    // attribute table
    QAction* tableAction = theMenu.addAction( tr( "&Open attribute table" ), this, SLOT( table() ) );
    if ( files.size() != 1 )
    {
      tableAction->setEnabled( false );
    }

    // allow editing
    if ( toggleEditingAction )
    {
      theMenu.addAction( toggleEditingAction );
    }

    QgsVectorLayer* theVectorLayer = qobject_cast<QgsVectorLayer *>( firstLayer );

    if ( files.size() != 1 )
    {
      toggleEditingAction->setEnabled( false );
    }
    if ( theVectorLayer )
    {
      toggleEditingAction->setChecked( theVectorLayer->isEditable() );
    }

    // save as shapefile
    QAction* saveShpAction = theMenu.addAction( tr( "Save as shapefile..." ), this, SLOT( saveAsShapefile() ) );
    if ( files.size() != 1 )
    {
      saveShpAction->setEnabled( false );
    }

    // save selection as shapefile
    QAction* saveSelectionAction = theMenu.addAction( tr( "Save selection as shapefile..." ), this, SLOT( saveSelectionAsShapefile() ) );
    if ( files.size() != 1 || theVectorLayer->selectedFeatureCount() == 0 )
    {
      saveSelectionAction->setEnabled( false );
    }

    theMenu.addSeparator();
  }


  QAction* propertiesAction = theMenu.addAction( tr( "&Properties" ), legend(), SLOT( legendLayerShowProperties() ) );
  if ( files.size() != 1 )
  {
    propertiesAction->setEnabled( false );
  }

}

bool QgsLegendLayer::isInOverview()
{
  // QAction doesn't support tristate checkboxes
  // returns true if just some are in overview
  bool hasVisible = false;

  // find out whether we're showing or hiding them
  std::list<QgsLegendLayerFile*> maplayers = legendLayerFiles();
  for ( std::list<QgsLegendLayerFile*>::iterator it = maplayers.begin(); it != maplayers.end(); ++it )
  {
    if ( *it && ( *it )->isInOverview() )
    {
      hasVisible = true;
      break;
    }
  }

  return hasVisible;
}

void QgsLegendLayer::showInOverview()
{
  std::list<QgsLegendLayerFile*> maplayers = legendLayerFiles();
  bool showLayers = ( ! isInOverview() );

  // set overview visibility
  for ( std::list<QgsLegendLayerFile*>::iterator it = maplayers.begin(); it != maplayers.end(); ++it )
  {
    if ( *it )
      ( *it )->setInOverview( showLayers );
  }

  legend()->updateMapCanvasLayerSet();
  legend()->updateOverview();
}

void QgsLegendLayer::table()
{
  std::list<QgsLegendLayerFile*> maplayers = legendLayerFiles();
  if ( maplayers.size() > 1 )
  {
    QMessageBox::information( 0, tr( "Multiple layers" ),
                              tr( "This item contains multiple layers. Displaying multiple layers in the table is not supported." ) );
  }
  else if ( maplayers.size() == 1 )
  {
    maplayers.front()->table();
  }
}

void QgsLegendLayer::saveAsShapefile()
{
  std::list<QgsLegendLayerFile*> maplayers = legendLayerFiles();
  if ( maplayers.size() == 1 )
  {
    maplayers.front()->saveAsShapefile();
  }
}

void QgsLegendLayer::saveSelectionAsShapefile()
{
  std::list<QgsLegendLayerFile*> maplayers = legendLayerFiles();
  if ( maplayers.size() == 1 )
  {
    maplayers.front()->saveSelectionAsShapefile();
  }
}
