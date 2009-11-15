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
#include "qgsvectorfilewriter.h"
#include "qgsgenericprojectionselector.h"
#include "qgsattributetabledialog.h"
#include "qgsencodingfiledialog.h"

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
  mLyr.setInOverview( FALSE );

  // Add check if vector layer when connecting to selectionChanged slot
  // Ticket #811 - racicot
  QgsMapLayer *currentLayer = mLyr.layer();
  QgsVectorLayer *isVectLyr = qobject_cast<QgsVectorLayer *>( currentLayer );
  if ( isVectLyr )
  {
    connect( mLyr.layer(), SIGNAL( editingStarted() ), this, SLOT( updateLegendItem() ) );
    connect( mLyr.layer(), SIGNAL( editingStopped() ), this, SLOT( updateLegendItem() ) );
  }
  connect( mLyr.layer(), SIGNAL( layerNameChanged() ), this, SLOT( layerNameChanged() ) );
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
    itemList.append( qMakePair( values, pix ) );
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
        itemList.append( qMakePair( classfieldname, QPixmap() ) );
      }
    }
  }

  changeSymbologySettings( layer, itemList );
}


void QgsLegendLayer::vectorLayerSymbologyV2( QgsVectorLayer* layer )
{
  QSize iconSize(16,16);

  QSettings settings;
  bool showClassifiers = settings.value( "/qgis/showLegendClassifiers", false ).toBool();

  SymbologyList itemList = layer->rendererV2()->legendSymbologyItems(iconSize);

  changeSymbologySettings( layer, itemList );
}

void QgsLegendLayer::rasterLayerSymbology( QgsRasterLayer* layer )
{
  SymbologyList itemList;
  QPixmap legendpixmap = layer->legendAsPixmap( true ).scaled( 20, 20, Qt::KeepAspectRatio );
  itemList.append( qMakePair( QString(), legendpixmap ) );

  changeSymbologySettings( layer, itemList );

  updateIcon();
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
      QgsRasterLayer* rlayer = qobject_cast<QgsRasterLayer *>( theLayer );
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

  QgsMapLayer* lyr = layer();

  // zoom to layer extent
  theMenu.addAction( QgisApp::getThemeIcon( "/mActionZoomToLayer.png" ),
                     tr( "&Zoom to layer extent" ), legend(), SLOT( legendLayerZoom() ) );
  if ( lyr->type() == QgsMapLayer::RasterLayer )
  {
    theMenu.addAction( tr( "&Zoom to best scale (100%)" ), legend(), SLOT( legendLayerZoomNative() ) );
  }

  // show in overview
  QAction* showInOverviewAction = theMenu.addAction( tr( "&Show in overview" ), this, SLOT( showInOverview() ) );
  showInOverviewAction->setCheckable( true );
  showInOverviewAction->blockSignals( true );
  showInOverviewAction->setChecked( mLyr.isInOverview() );
  showInOverviewAction->blockSignals( false );

  // remove from canvas
  theMenu.addAction( QgisApp::getThemeIcon( "/mActionRemove.png" ),
                     tr( "&Remove" ), legend(), SLOT( legendLayerRemove() ) );

  theMenu.addSeparator();

  if ( lyr->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( lyr );

    // attribute table
    QAction* tableAction = theMenu.addAction( tr( "&Open attribute table" ), this, SLOT( table() ) );

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

    // save as shapefile
    QAction* saveShpAction = theMenu.addAction( tr( "Save as shapefile..." ), this, SLOT( saveAsShapefile() ) );

    // save selection as shapefile
    QAction* saveSelectionAction = theMenu.addAction( tr( "Save selection as shapefile..." ), this, SLOT( saveSelectionAsShapefile() ) );
    if ( vlayer->selectedFeatureCount() == 0 )
    {
      saveSelectionAction->setEnabled( false );
    }

    theMenu.addSeparator();
  }

  // properties goes on bottom of menu for consistency with normal ui standards
  // e.g. kde stuff
  theMenu.addAction( tr( "&Properties" ), legend(), SLOT( legendLayerShowProperties() ) );

}

void QgsLegendLayer::table()
{
  QgsVectorLayer * myLayer = qobject_cast<QgsVectorLayer *>( mLyr.layer() );
  QgsAttributeTableDialog *mDialog = new QgsAttributeTableDialog( myLayer );
  mDialog->show();
  // the dialog will be deleted by itself on close
}

void QgsLegendLayer::saveAsShapefile()
{
  saveAsShapefileGeneral( FALSE );
}

void QgsLegendLayer::saveSelectionAsShapefile()
{
  saveAsShapefileGeneral( TRUE );
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




void QgsLegendLayer::saveAsShapefileGeneral( bool saveOnlySelection )
{
  QgsCoordinateReferenceSystem destCRS;

  if ( mLyr.layer()->type() != QgsMapLayer::VectorLayer )
    return;

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mLyr.layer() );

  // get a name for the shapefile
  // Get a file to process, starting at the current directory
  QSettings settings;
  QString filter =  QString( "Shapefiles (*.shp)" );
  QString dirName = settings.value( "/UI/lastShapefileDir", "." ).toString();

  QgsEncodingFileDialog* openFileDialog = new QgsEncodingFileDialog( 0,
      tr( "Save layer as..." ),
      dirName,
      filter,
      QString( "" ) );
  openFileDialog->setAcceptMode( QFileDialog::AcceptSave );

  // allow for selection of more than one file
  //openFileDialog->setMode(QFileDialog::AnyFile);

  if ( openFileDialog->exec() != QDialog::Accepted )
    return;


  QString encoding = openFileDialog->encoding();
  QString shapefileName = openFileDialog->selectedFiles().first();
  settings.setValue( "/UI/lastShapefileDir", QFileInfo( shapefileName ).absolutePath() );


  if ( shapefileName.isNull() )
    return;

  // add the extension if not present
  if ( shapefileName.indexOf( ".shp" ) == -1 )
  {
    shapefileName += ".shp";
  }

  destCRS = vlayer->srs();
  // Find out if we have projections enabled or not
  if ( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() )
  {
    destCRS = QgisApp::instance()->mapCanvas()->mapRenderer()->destinationSrs();
  }

  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector();
  mySelector->setSelectedCrsId( destCRS.srsid() );
  mySelector->setMessage( tr( "Select the coordinate reference system for the saved shapefile. "
                              "The data points will be transformed from the layer coordinate reference system." ) );

  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem srs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    destCRS = srs;
    //   destCRS->createFromId(mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId)
  }
  else
  {
    // Aborted CS selection, don't save.
    delete mySelector;
    return;
  }

  delete mySelector;

  // overwrite the file - user will already have been prompted
  // to verify they want to overwrite by the file dialog above
  if ( QFile::exists( shapefileName ) )
  {
    if ( !QgsVectorFileWriter::deleteShapeFile( shapefileName ) )
    {
      return;
    }
  }

  // ok if the file existed it should be deleted now so we can continue...
  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsVectorFileWriter::WriterError error;
  error = QgsVectorFileWriter::writeAsShapefile( vlayer, shapefileName, encoding, &destCRS, saveOnlySelection );

  QApplication::restoreOverrideCursor();

  switch ( error )
  {
    case QgsVectorFileWriter::NoError:
      QMessageBox::information( 0, tr( "Saving done" ), tr( "Export to Shapefile has been completed" ) );
      break;

    case QgsVectorFileWriter::ErrDriverNotFound:
      QMessageBox::warning( 0, tr( "Driver not found" ), tr( "ESRI Shapefile driver is not available" ) );
      break;

    case QgsVectorFileWriter::ErrCreateDataSource:
      QMessageBox::warning( 0, tr( "Error creating shapefile" ),
                            tr( "The shapefile could not be created (%1)" ).arg( shapefileName ) );
      break;

    case QgsVectorFileWriter::ErrCreateLayer:
      QMessageBox::warning( 0, tr( "Error" ), tr( "Layer creation failed" ) );
      break;

    case QgsVectorFileWriter::ErrAttributeTypeUnsupported:
      QMessageBox::warning( 0, tr( "Error" ),
                            tr( "Layer attribute table contains unsupported datatype(s)" ) );
      break;

    case QgsVectorFileWriter::ErrAttributeCreationFailed:
      QMessageBox::warning( 0, tr( "Error" ),
                            tr( "Creation of an attribute failed" ) );
      break;
  }
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

