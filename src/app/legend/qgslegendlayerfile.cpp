/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton   *
 *   aps02ts@macbuntu   *
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
#include "qgsmaplayer.h"
#include "qgsmaprenderer.h"
#include "qgsrasterlayer.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgenericprojectionselector.h"

// attribute table
#include "qgsattributetabledialog.h"


#include "qgsencodingfiledialog.h"

#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>


QgsLegendLayerFile::QgsLegendLayerFile( QTreeWidgetItem * theLegendItem, QString theString, QgsMapLayer* theLayer )
    : QgsLegendItem( theLegendItem, theString ), mLyr( theLayer )
{
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

  mType = LEGEND_LAYER_FILE;

  setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  setCheckState( 0, Qt::Checked );
  setText( 0, theString );

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

QgsLegendItem::DRAG_ACTION QgsLegendLayerFile::accept( LEGEND_ITEM_TYPE type )
{
  return NO_ACTION;
}

QgsLegendItem::DRAG_ACTION QgsLegendLayerFile::accept( const QgsLegendItem* li ) const
{
  if ( li->type() == QgsLegendItem::LEGEND_LAYER_FILE )
  {
    if ( li->parent() == this->parent() )
    {
      return REORDER;
    }
  }
  return NO_ACTION;
}

QPixmap QgsLegendLayerFile::getOriginalPixmap() const
{
  QPixmap myPixmap = QgisApp::getThemePixmap( "mActionFileSmall.png" );
  return myPixmap;
}

void QgsLegendLayerFile::updateLegendItem()
{
  QPixmap pix = legend()->pixmaps().mOriginalPixmap;

  if ( mLyr.isInOverview() )
  {
    //add overview glasses to the pixmap
    QPainter p( &pix );
    p.drawPixmap( 0, 0, legend()->pixmaps().mInOverviewPixmap );
  }
  if ( mLyr.layer()->isEditable() )
  {
    //add editing icon to the pixmap
    QPainter p( &pix );
    p.drawPixmap( 30, 0, legend()->pixmaps().mEditablePixmap );
  }

  /*
  // TODO:
  if(mLyr.layer()->hasProjectionError())
  {
    //add overview glasses to the pixmap
    QPainter p(&pix);
    p.drawPixmap(60,0, legend()->pixmaps().mProjectionErrorPixmap);
  }
  */

  QIcon theIcon( pix );
  setIcon( 0, theIcon );

}

void QgsLegendLayerFile::setIconAppearance( bool isInOverview,
    bool editable )
{
  QPixmap newIcon( getOriginalPixmap() );

  if ( isInOverview )
  {
    // Overlay the overview icon on the default icon
    QPixmap myPixmap = QgisApp::getThemePixmap( "mIconOverview.png" );
    QPainter p( &newIcon );
    p.drawPixmap( 0, 0, myPixmap );
    p.end();
  }

  if ( editable )
  {
    // Overlay the editable icon on the default icon
    QPixmap myPixmap = QgisApp::getThemePixmap( "mIconEditable.png" );
    QPainter p( &newIcon );
    p.drawPixmap( 0, 0, myPixmap );
    p.end();
  }

  QIcon theIcon( newIcon );
  setIcon( 0, theIcon );

  //also update the icon of the legend layer
  (( QgsLegendLayer* )( parent()->parent() ) )->updateIcon();
}


QString QgsLegendLayerFile::nameFromLayer( QgsMapLayer* layer )
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


void QgsLegendLayerFile::setVisible( bool visible )
{
  mLyr.setVisible( visible );
}

bool QgsLegendLayerFile::isVisible()
{
  return mLyr.isVisible();
}

void QgsLegendLayerFile::setInOverview( bool isInOverview )
{
  mLyr.setInOverview( isInOverview );
}

bool QgsLegendLayerFile::isInOverview()
{
  return mLyr.isInOverview();
}

void QgsLegendLayerFile::showInOverview()
{
  // toggle current status
  setInOverview( ! isInOverview() );

  legend()->updateMapCanvasLayerSet();
  legend()->updateOverview();
}

void QgsLegendLayerFile::saveAsShapefile()
{
  saveAsShapefileGeneral( FALSE );
}

void QgsLegendLayerFile::table()
{
  QgsVectorLayer * myLayer = qobject_cast<QgsVectorLayer *>( mLyr.layer() );
  QgsAttributeTableDialog *mDialog = new QgsAttributeTableDialog( myLayer );
  mDialog->show();
  // the dialog will be deleted by itself on close
}

void QgsLegendLayerFile::saveSelectionAsShapefile()
{
  saveAsShapefileGeneral( TRUE );
}

void QgsLegendLayerFile::saveAsShapefileGeneral( bool saveOnlySelection )
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

bool QgsLegendLayerFile::isEditing()
{
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mLyr.layer() );
  return vlayer && vlayer->isEditable();
}

void QgsLegendLayerFile::layerNameChanged()
{
  QString name = mLyr.layer()->name();
  setText( 0, name );

  // set also parent's name
  legend()->setName( this, name );
}

void QgsLegendLayerFile::addToPopupMenu( QMenu& theMenu, QAction* toggleEditingAction )
{
  QgsMapLayer* lyr = layer();

  // zoom to layer extent
  theMenu.addAction( QgisApp::getThemeIcon( "/mActionZoomToLayer.png" ),
                     tr( "&Zoom to layer extent" ), legend(), SLOT( legendLayerZoom() ) );

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
    theMenu.addAction( tr( "&Open attribute table" ), this, SLOT( table() ) );

    // editing
    int cap = vlayer->dataProvider()->capabilities();
    if ( cap & QgsVectorDataProvider::EditingCapabilities )
    {
      if ( toggleEditingAction )
      {
        theMenu.addAction( toggleEditingAction );
      }
    }

    // save as shapefile
    theMenu.addAction( tr( "Save as shapefile..." ), this, SLOT( saveAsShapefile() ) );

    QAction* saveSelectionAction = theMenu.addAction( tr( "Save selection as shapefile..." ), this, SLOT( saveSelectionAsShapefile() ) );
    if ( vlayer->selectedFeatureCount() == 0 )
    {
      saveSelectionAction->setEnabled( false );
    }

    theMenu.addSeparator();
  }
  else if ( lyr->type() == QgsMapLayer::RasterLayer )
  {
    // TODO: what was this for?
    //QgsRasterLayer* rlayer = qobject_cast<QgsRasterLayer *>(lyr);
    //theMenu.addAction(tr("&Convert to..."), rlayer, SLOT(convertTo()));
  }

  // properties goes on bottom of menu for consistency with normal ui standards
  // e.g. kde stuff
  theMenu.addAction( tr( "&Properties" ), legend(), SLOT( legendLayerShowProperties() ) );
}
