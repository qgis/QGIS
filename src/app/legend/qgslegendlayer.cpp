/***************************************************************************
    qgslegendlayer.cpp
    ---------------------
    begin                : January 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgslegend.h"
#include "qgslegendlayer.h"
#include "qgslegendsymbologyitem.h"
#include "qgslogger.h"

#include "qgsfield.h"
#include "qgsmapcanvasmap.h"
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

#include <QAction>
#include <QCoreApplication>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QFileDialog>
#include <QProgressDialog>


QgsLegendLayer::QgsLegendLayer( QgsMapLayer* layer )
    : QgsLegendItem( )
    , mLyr( layer )
    , mDrawingOrder( -1 )
    , mShowFeatureCount( false )
{
  mType = LEGEND_LAYER;

  Qt::ItemFlags flags = Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer || vlayer->hasGeometryType() )
  {
    flags |= Qt::ItemIsUserCheckable;
  }

  setFlags( flags );

  setCheckState( 0, Qt::Checked );

  layerNameChanged();
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
    connect( layer, SIGNAL( layerModified() ), this, SLOT( updateAfterLayerModification() ) ); // TODO[MD]: should have symbologyChanged signal
  }
  if ( qobject_cast<QgsRasterLayer *>( layer ) )
  {
    connect( layer, SIGNAL( dataChanged() ), this, SLOT( updateAfterLayerModification() ) );
  }
  connect( layer, SIGNAL( layerNameChanged() ), this, SLOT( layerNameChanged() ) );

  updateIcon();
  setToolTip( 0, layer->publicSource() );
}

QgsLegendLayer::~QgsLegendLayer()
{
  mType = LEGEND_LAYER;
}

void QgsLegendLayer::setCheckState( int column, Qt::CheckState state )
{
  if ( flags() & Qt::ItemIsUserCheckable )
  {
    QTreeWidgetItem::setCheckState( column, state );
  }
}

void QgsLegendLayer::setupFont()
{
  QSettings settings;
  QFont myFont = font( 0 );
  //visually differentiate layer labels from the rest
  myFont.setBold( settings.value( "/qgis/legendLayersBold", true ).toBool() );
  setFont( 0, myFont );
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



void QgsLegendLayer::vectorLayerSymbology( QgsVectorLayer* layer, double widthScale )
{
  if ( !layer )
  {
    return;
  }

  SymbologyList itemList;
  if ( layer->hasGeometryType() )
  {
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
        const QgsFields& fields = layer->pendingFields();
        for ( QgsAttributeList::iterator it = classfieldlist.begin(); it != classfieldlist.end(); ++it )
        {
          int idx = *it;
          if ( idx < 0 || idx >= fields.count() )
            continue;
          QString classfieldname = layer->attributeAlias( idx );
          if ( classfieldname.isEmpty() )
          {
            classfieldname = fields[idx].name();
          }
          itemList.append( qMakePair( classfieldname, QPixmap() ) );
        }
      }
    }

    QMap< QgsSymbol*, int > featureCountMap;
    if ( mShowFeatureCount )
    {
      updateItemListCount( layer, sym, featureCountMap );
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
      else  if (( *it )->type() == QGis::Polygon )
      {
        img = ( *it )->getPolygonSymbolAsImage();
      }
      else
      {
        // must be a layer without geometry then
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

      if ( mShowFeatureCount )
      {
        int fCount = featureCountMap[*it];
        if ( fCount >= 0 )
        {
          values += ( " [" + QString::number( fCount ) + "]" );
        }
      }

      QPixmap pix = QPixmap::fromImage( img ); // convert to pixmap
      itemList.append( qMakePair( values, pix ) );
    }
  }
  changeSymbologySettings( layer, itemList );
}


void QgsLegendLayer::vectorLayerSymbologyV2( QgsVectorLayer* layer )
{
  QSize iconSize( 16, 16 );

  QgsFeatureRendererV2* renderer = layer->rendererV2();
  if ( renderer )
  {
    SymbologyList itemList = renderer->legendSymbologyItems( iconSize );
    if ( mShowFeatureCount )
    {
      updateItemListCountV2( itemList, layer );
    }

    changeSymbologySettings( layer, itemList );
    layerNameChanged(); // update total count
  }
}

void QgsLegendLayer::rasterLayerSymbology( QgsRasterLayer* layer )
{
  SymbologyList itemList;
  QList< QPair< QString, QColor > > rasterItemList = layer->legendSymbologyItems();
  QList< QPair< QString, QColor > >::const_iterator itemIt = rasterItemList.constBegin();
  for ( ; itemIt != rasterItemList.constEnd(); ++itemIt )
  {
    QPixmap itemPixmap( treeWidget()->iconSize() );
    itemPixmap.fill( itemIt->second );
    itemList.append( qMakePair( itemIt->first, itemPixmap ) );
  }
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
    QPixmap myPixmap = QgsApplication::getThemePixmap(  "/mIconOverview.png" );
    QPainter p( &newIcon );
    p.drawPixmap( 0, 0, myPixmap );
    p.end();
  }*/

  //editable
  if ( theLayer->isEditable() )
  {
    QPixmap myPixmap;
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( theLayer );
    if ( vlayer->isModified() )
    {
      myPixmap = QgsApplication::getThemePixmap( "/mIconEditableEdits.png" );
    }
    else
    {
      myPixmap = QgsApplication::getThemePixmap( "/mIconEditable.png" );
    }
    // use editable icon instead of the layer's type icon
    newIcon = myPixmap;

    // Overlay the editable icon on the default icon
    /*QPainter p( &newIcon );
    p.drawPixmap( 0, 0, myPixmap );
    p.end();*/
  }

  // TODO: projection error icon?

  QIcon theIcon( newIcon );
  QgsLegend* l = legend();
  if ( l )
  {
    l->blockSignals( true ); //prevent unnecessary canvas redraw
  }
  setIcon( 0, theIcon );
  if ( l )
  {
    l->blockSignals( false );
  }
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
          return QgsApplication::getThemePixmap( "/mIconPointLayer.png" );
        case QGis::Line:
          return QgsApplication::getThemePixmap( "/mIconLineLayer.png" );
        case QGis::Polygon:
          return QgsApplication::getThemePixmap( "/mIconPolygonLayer.png" );
        case QGis::NoGeometry:
          return QgsApplication::getThemePixmap( "/mIconTableLayer.png" );
        default:
          return QgsApplication::getThemePixmap( "/mIconLayer.png" );
      }
    }
    else if ( theLayer->type() == QgsMapLayer::RasterLayer )
    {
      QSettings s;
      if ( s.value( "/qgis/createRasterLegendIcons", true ).toBool() )
      {
        QgsRasterLayer* rlayer = qobject_cast<QgsRasterLayer *>( theLayer );
        return rlayer->previewAsPixmap( QSize( 32, 32 ) );
      }
      else
      {
        return QPixmap();
      }
    }
  }

  // undefined - should never reach this
  return QgsApplication::getThemePixmap( "/mIconLayer.png" );
}

void QgsLegendLayer::addToPopupMenu( QMenu& theMenu )
{
  QgsMapLayer *lyr = layer();
  QAction *toggleEditingAction = QgisApp::instance()->actionToggleEditing();
  QAction *saveLayerEditsAction = QgisApp::instance()->actionSaveActiveLayerEdits();
  QAction *allEditsAction = QgisApp::instance()->actionAllEdits();

  // zoom to layer extent
  theMenu.addAction( QgsApplication::getThemeIcon( "/mActionZoomToLayer.png" ),
                     tr( "&Zoom to Layer Extent" ), legend(), SLOT( legendLayerZoom() ) );
  if ( lyr->type() == QgsMapLayer::RasterLayer )
  {
    theMenu.addAction( tr( "&Zoom to Best Scale (100%)" ), legend(), SLOT( legendLayerZoomNative() ) );

    QgsRasterLayer *rasterLayer =  qobject_cast<QgsRasterLayer *>( lyr );
    if ( rasterLayer && rasterLayer->rasterType() != QgsRasterLayer::Palette )
    {
      theMenu.addAction( tr( "&Stretch Using Current Extent" ), legend(), SLOT( legendLayerStretchUsingCurrentExtent() ) );
    }
  }

  // show in overview
  QAction* showInOverviewAction = theMenu.addAction( tr( "&Show in Overview" ), this, SLOT( showInOverview() ) );
  showInOverviewAction->setCheckable( true );
  showInOverviewAction->blockSignals( true );
  showInOverviewAction->setChecked( mLyr.isInOverview() );
  showInOverviewAction->blockSignals( false );

  // remove from canvas
  theMenu.addAction( QgsApplication::getThemeIcon( "/mActionRemoveLayer.png" ), tr( "&Remove" ), QgisApp::instance(), SLOT( removeLayer() ) );

  // duplicate layer
  QAction* duplicateLayersAction = theMenu.addAction( QgsApplication::getThemeIcon( "/mActionAddMap.png" ), tr( "&Duplicate" ), QgisApp::instance(), SLOT( duplicateLayers() ) );

  // set layer crs
  theMenu.addAction( QgsApplication::getThemeIcon( "/mActionSetCRS.png" ), tr( "&Set Layer CRS" ), QgisApp::instance(), SLOT( setLayerCRS() ) );

  // assign layer crs to project
  theMenu.addAction( QgsApplication::getThemeIcon( "/mActionSetProjectCRS.png" ), tr( "Set &Project CRS from Layer" ), QgisApp::instance(), SLOT( setProjectCRSFromLayer() ) );

  theMenu.addSeparator();

  if ( lyr->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( lyr );

    // attribute table
    theMenu.addAction( QgsApplication::getThemeIcon( "/mActionOpenTable.png" ), tr( "&Open Attribute Table" ),
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
      if ( saveLayerEditsAction && vlayer->isModified() )
      {
        theMenu.addAction( saveLayerEditsAction );
      }
    }

    if ( allEditsAction->isEnabled() )
    {
      theMenu.addAction( allEditsAction );
    }

    // disable duplication of memory layers
    if ( vlayer->storageType() == "Memory storage" && legend()->selectedLayers().count() == 1 )
    {
      duplicateLayersAction->setEnabled( false );
    }

    // save as vector file
    theMenu.addAction( tr( "Save As..." ), QgisApp::instance(), SLOT( saveAsFile() ) );

    // save selection as vector file
    QAction* saveSelectionAsAction = theMenu.addAction( tr( "Save Selection As..." ), QgisApp::instance(), SLOT( saveSelectionAsVectorFile() ) );
    if ( vlayer->selectedFeatureCount() == 0 )
    {
      saveSelectionAsAction->setEnabled( false );
    }

    if ( !vlayer->isEditable() && vlayer->dataProvider()->supportsSubsetString() && vlayer->vectorJoins().isEmpty() )
      theMenu.addAction( tr( "&Filter..." ), QgisApp::instance(), SLOT( layerSubsetString() ) );

    //show number of features in legend if requested
    QAction* showNFeaturesAction = new QAction( tr( "Show Feature Count" ), &theMenu );
    showNFeaturesAction->setCheckable( true );
    showNFeaturesAction->setChecked( mShowFeatureCount );
    QObject::connect( showNFeaturesAction, SIGNAL( toggled( bool ) ), this, SLOT( setShowFeatureCount( bool ) ) );
    theMenu.addAction( showNFeaturesAction );
    theMenu.addSeparator();
  }
  else if ( lyr->type() == QgsMapLayer::RasterLayer )
  {
    theMenu.addAction( tr( "Save As..." ), QgisApp::instance(), SLOT( saveAsRasterFile() ) );
  }
  else if ( lyr->type() == QgsMapLayer::PluginLayer && legend()->selectedLayers().count() == 1 )
  {
    // disable duplication of plugin layers
    duplicateLayersAction->setEnabled( false );
  }
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

QString QgsLegendLayer::label()
{
  QString name = mLyr.layer()->name();
  QgsVectorLayer *vlayer = dynamic_cast<QgsVectorLayer *>( mLyr.layer() );
  if ( mShowFeatureCount && vlayer && vlayer->featureCount() >= 0 )
  {
    name += QString( " [%1]" ).arg( vlayer->featureCount() );
  }
  return name;
}

void QgsLegendLayer::layerNameChanged()
{
  setText( 0, label() );
}

void QgsLegendLayer::beforeEdit()
{
  // Reset to layer name without possible feature count
  setText( 0, mLyr.layer()->name() );
}

void QgsLegendLayer::afterEdit()
{
  // Reset label with possible feature count, important if text was not changed
  layerNameChanged();
}

QString QgsLegendLayer::layerName()
{
  // The text could be edited (Rename), in that case we have to return the new name
  if ( text( 0 ) != label() && text( 0 ) != mLyr.layer()->name() )
  {
    return text( 0 );
  }
  return mLyr.layer()->name();
}

void QgsLegendLayer::updateAfterLayerModification()
{
  double widthScale = 1.0;
  QgsMapCanvas* canvas = QgisApp::instance()->mapCanvas();
  if ( canvas && canvas->map() )
  {
    widthScale = canvas->map()->paintDevice().logicalDpiX() / 25.4;
  }
  refreshSymbology( mLyr.layer()->id(), widthScale );
  layerNameChanged();
}

void QgsLegendLayer::updateItemListCountV2( SymbologyList& itemList, QgsVectorLayer* layer )
{
  if ( !layer )
  {
    return;
  }

  QgsFeatureRendererV2* renderer = layer->rendererV2();
  if ( !renderer )
  {
    return;
  }

  // Count features
  if ( !layer->countSymbolFeatures() )
  {
    QgsDebugMsg( "Cannot get feature counts" );
    return;
  }

  QMap<QString, QPixmap> itemMap;
  SymbologyList::const_iterator symbologyIt = itemList.constBegin();
  for ( ; symbologyIt != itemList.constEnd(); ++ symbologyIt )
  {
    itemMap.insert( symbologyIt->first, symbologyIt->second );
  }

  itemList.clear();

  QgsLegendSymbolList symbolList = renderer->legendSymbolItems();
  QgsLegendSymbolList::const_iterator symbolIt = symbolList.constBegin();
  symbolIt = symbolList.constBegin();
  for ( ; symbolIt != symbolList.constEnd(); ++symbolIt )
  {
    itemList.push_back( qMakePair( symbolIt->first + " [" + QString::number( layer->featureCount( symbolIt->second ) ) + "]", itemMap[symbolIt->first] ) );
  }
}

void QgsLegendLayer::updateItemListCount( QgsVectorLayer* layer, const QList<QgsSymbol*>& sym, QMap< QgsSymbol*, int >& featureCountMap )
{
  featureCountMap.clear();
  QList<QgsSymbol*>::const_iterator symbolIt = sym.constBegin();
  for ( ; symbolIt != sym.constEnd(); ++symbolIt )
  {
    featureCountMap.insert( *symbolIt, 0 );
  }

  QgsRenderer* renderer = const_cast<QgsRenderer*>( layer->renderer() );
  if ( !renderer )
  {
    return;
  }

  //go through all features and count the number of occurrences
  int nFeatures = layer->pendingFeatureCount();
  QProgressDialog p( tr( "Updating feature count for layer %1" ).arg( layer->name() ), tr( "Abort" ), 0, nFeatures );
  p.setWindowModality( Qt::WindowModal );
  int featuresCounted = 0;

  QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ) );
  QgsFeature f;
  QgsSymbol* currentSymbol = 0;

  while ( fit.nextFeature( f ) )
  {
    currentSymbol = renderer->symbolForFeature( &f );
    if ( currentSymbol )
    {
      featureCountMap[currentSymbol] += 1;
    }
    ++featuresCounted;

    if ( featuresCounted % 50 == 0 )
    {
      if ( featuresCounted > nFeatures ) //sometimes the feature count is not correct
      {
        p.setMaximum( 0 );
      }
      p.setValue( featuresCounted );
      if ( p.wasCanceled() ) //set all entries to -1 (= invalid)
      {
        QMap< QgsSymbol*, int >::iterator cIt = featureCountMap.begin();
        for ( ; cIt != featureCountMap.end(); ++cIt )
        {
          cIt.value() = -1;
        }
        return;
      }
    }
  }
  p.setValue( nFeatures );
}

void QgsLegendLayer::setShowFeatureCount( bool show, bool update )
{
  if ( show != mShowFeatureCount )
  {
    mShowFeatureCount = show;
    if ( update )
    {
      updateAfterLayerModification();
    }
  }
}

void QgsLegendLayer::setDrawingOrder( int order )
{
  QgsDebugMsg( QString( "order %1: %2=>%3" ).arg( mLyr.layer()->name() ).arg( mDrawingOrder ).arg( order ) );
  mDrawingOrder = order;
}
