/***************************************************************************
    qgsmaptoolidentify.cpp  -  map tool for identifying features
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgscursors.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsidentifyresults.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmaptoolidentify.h"
#include "qgsrasterlayer.h"
#include "qgsrubberband.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsattributedialog.h"

#include <QSettings>
#include <QMessageBox>
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>

QgsMapToolIdentify::QgsMapToolIdentify( QgsMapCanvas* canvas )
    : QgsMapTool( canvas ),
    mResults( 0 ),
    mRubberBand( 0 )
{
  // set cursor
  QPixmap myIdentifyQPixmap = QPixmap(( const char ** ) identify_cursor );
  mCursor = QCursor( myIdentifyQPixmap, 1, 1 );

  mLayer = 0; // Initialize mLayer, useful in removeLayer SLOT
}

QgsMapToolIdentify::~QgsMapToolIdentify()
{
  if ( mResults )
  {
    mResults->done( 0 );
  }

  delete mRubberBand;
}

void QgsMapToolIdentify::canvasMoveEvent( QMouseEvent * e )
{
}

void QgsMapToolIdentify::canvasPressEvent( QMouseEvent * e )
{
}

void QgsMapToolIdentify::canvasReleaseEvent( QMouseEvent * e )
{
  if ( !mCanvas || mCanvas->isDrawing() )
  {
    return;
  }

  mLayer = mCanvas->currentLayer();

  // delete rubber band if there was any
  delete mRubberBand;
  mRubberBand = 0;

  // call identify method for selected layer

  if ( mLayer )
  {
    // In the special case of the WMS provider,
    // coordinates are sent back to the server as pixel coordinates
    // not the layer's native CRS.  So identify on screen coordinates!
    if (
      ( mLayer->type() == QgsMapLayer::RasterLayer )
      &&
      ( dynamic_cast<QgsRasterLayer*>( mLayer )->providerKey() == "wms" )
    )
    {
      identifyRasterWmsLayer( QgsPoint( e->x(), e->y() ) );
    }
    else
    {
      // convert screen coordinates to map coordinates
      QgsPoint idPoint = mCanvas->getCoordinateTransform()->toMapCoordinates( e->x(), e->y() );

      if ( mLayer->type() == QgsMapLayer::VectorLayer )
      {
        identifyVectorLayer( idPoint );
      }
      else if ( mLayer->type() == QgsMapLayer::RasterLayer )
      {
        identifyRasterLayer( idPoint );
      }
      else
      {
        QgsDebugMsg( "unknown layer type!" );
      }
    }

  }
  else
  {
    QMessageBox::warning( mCanvas,
                          tr( "No active layer" ),
                          tr( "To identify features, you must choose an active layer by clicking on its name in the legend" ) );
  }


}


void QgsMapToolIdentify::identifyRasterLayer( const QgsPoint& point )
{
  QgsRasterLayer *layer = dynamic_cast<QgsRasterLayer*>( mLayer );
  if ( !layer )
    return;

  QMap<QString, QString> attributes;
  layer->identify( point, attributes );

  if ( !mResults )
  {
    QgsAttributeAction aa;
    mResults = new QgsIdentifyResults( aa, mCanvas->window() );
    mResults->setAttribute( Qt::WA_DeleteOnClose );
    // Be informed when the dialog box is closed so that we can stop using it.
    connect( mResults, SIGNAL( accepted() ), this, SLOT( resultsDialogGone() ) );
    connect( mResults, SIGNAL( rejected() ), this, SLOT( resultsDialogGone() ) );
    mResults->restorePosition();
  }
  else
  {
    mResults->raise();
    mResults->clear();
  }

  mResults->setTitle( layer->name() );
  mResults->setColumnText( 0, tr( "Band" ) );

  QMap<QString, QString>::iterator it;
  for ( it = attributes.begin(); it != attributes.end(); it++ )
  {
    mResults->addAttribute( it.key(), it.value() );
  }

  mResults->addAttribute( tr( "(clicked coordinate)" ), point.toString() );

  mResults->showAllAttributes();
  mResults->show();
}


void QgsMapToolIdentify::identifyRasterWmsLayer( const QgsPoint& point )
{
  QgsRasterLayer *layer = dynamic_cast<QgsRasterLayer*>( mLayer );
  if ( !layer )
  {
    return;
  }

  //if WMS layer does not cover the view origin,
  //we need to map the view pixel coordinates
  //to WMS layer pixel coordinates
  QgsRectangle viewExtent = mCanvas->extent();
  double mapUnitsPerPixel = mCanvas->mapUnitsPerPixel();
  if ( mapUnitsPerPixel == 0 )
  {
    return;
  }
  double xMinView = viewExtent.xMinimum();
  double yMaxView = viewExtent.yMaximum();

  QgsRectangle layerExtent = layer->extent();
  double xMinLayer = layerExtent.xMinimum();
  double yMaxLayer = layerExtent.yMaximum();

  double i, j;

  if ( xMinView < xMinLayer )
  {
    i = ( int )( point.x() - ( xMinLayer - xMinView ) / mapUnitsPerPixel );
  }
  else
  {
    i = point.x();
  }

  if ( yMaxView > yMaxLayer )
  {
    j = ( int )( point.y() - ( yMaxView - yMaxLayer ) / mapUnitsPerPixel );
  }
  else
  {
    j = point.y();
  }


  QString text = layer->identifyAsText( QgsPoint( i, j ) );

  if ( text.isEmpty() )
  {
    showError();
    return;
  }

  QgsMessageViewer* viewer = new QgsMessageViewer();
  viewer->setWindowTitle( layer->name() );
  viewer->setMessageAsPlainText( tr( "WMS identify result for %1:\n%2" ).arg( point.toString() ).arg( text ) );

  viewer->showMessage(); // deletes itself on close
}

void QgsMapToolIdentify::identifyVectorLayer( const QgsPoint& point )
{
  QgsVectorLayer *layer = dynamic_cast<QgsVectorLayer*>( mLayer );
  if ( !layer )
    return;

  // load identify radius from settings
  QSettings settings;
  double identifyValue = settings.value( "/Map/identifyRadius", QGis::DEFAULT_IDENTIFY_RADIUS ).toDouble();
  QString ellipsoid = settings.value( "/qgis/measure/ellipsoid", "WGS84" ).toString();

  int featureCount = 0;
  QgsAttributeAction& actions = *layer->actions();
  QString fieldIndex = layer->displayField();
  const QgsFieldMap& fields = layer->pendingFields();

  // init distance/area calculator
  QgsDistanceArea calc;
  calc.setProjectionsEnabled( mCanvas->hasCrsTransformEnabled() ); // project?
  calc.setEllipsoid( ellipsoid );
  calc.setSourceCrs( layer->srs().srsid() );

  mFeatureList.clear();
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // toLayerCoordinates will throw an exception for an 'invalid' point.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  try
  {
    // create the search rectangle
    double searchRadius = mCanvas->extent().width() * ( identifyValue / 100.0 );

    QgsRectangle r;
    r.setXMinimum( point.x() - searchRadius );
    r.setXMaximum( point.x() + searchRadius );
    r.setYMinimum( point.y() - searchRadius );
    r.setYMaximum( point.y() + searchRadius );

    r = toLayerCoordinates( layer, r );

    layer->select( layer->pendingAllAttributesList(), r, true, true );
    QgsFeature f;
    while ( layer->nextFeature( f ) )
      mFeatureList << QgsFeature( f );
  }
  catch ( QgsCsException & cse )
  {
    Q_UNUSED( cse );
    // catch exception for 'invalid' point and proceed with no features found
    QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
  }

  QApplication::restoreOverrideCursor();

  if ( layer->isEditable() && mFeatureList.size() == 1 )
  {
    editFeature( mFeatureList[0] );
    return;
  }

  // display features falling within the search radius
  if ( !mResults )
  {
    mResults = new QgsIdentifyResults( actions, mCanvas->window() );
    mResults->setAttribute( Qt::WA_DeleteOnClose );
    // Be informed when the dialog box is closed so that we can stop using it.
    connect( mResults, SIGNAL( accepted() ), this, SLOT( resultsDialogGone() ) );
    connect( mResults, SIGNAL( rejected() ), this, SLOT( resultsDialogGone() ) );
    connect( mResults, SIGNAL( selectedFeatureChanged( int ) ), this, SLOT( highlightFeature( int ) ) );
    connect( mResults, SIGNAL( editFeature( int ) ), this, SLOT( editFeature( int ) ) );

    // restore the identify window position and show it
    mResults->restorePosition();
  }
  else
  {
    mResults->raise();
    mResults->clear();
    mResults->setActions( actions );
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  int lastFeatureId = 0;
  QgsFeatureList::iterator f_it = mFeatureList.begin();

  for ( ; f_it != mFeatureList.end(); ++f_it )
  {
    featureCount++;

    QTreeWidgetItem* featureNode = mResults->addNode( "foo" );
    featureNode->setData( 0, Qt::UserRole, QVariant( f_it->id() ) ); // save feature id
    lastFeatureId = f_it->id();
    featureNode->setText( 0, fieldIndex );

    if ( layer->isEditable() )
      mResults->addEdit( featureNode, f_it->id() );

    const QgsAttributeMap& attr = f_it->attributeMap();

    for ( QgsAttributeMap::const_iterator it = attr.begin(); it != attr.end(); ++it )
    {
      //QgsDebugMsg(it->fieldName() + " == " + fieldIndex);

      if ( fields[it.key()].name() == fieldIndex )
      {
        featureNode->setText( 1, it->toString() );
      }
      QString attributeName = layer->attributeDisplayName( it.key() );
      mResults->addAttribute( featureNode, attributeName, it->isNull() ? "NULL" : it->toString() );
    }

    // Calculate derived attributes and insert:
    // measure distance or area depending on geometry type
    if ( layer->geometryType() == QGis::Line )
    {
      double dist = calc.measure( f_it->geometry() );
      QString str = calc.textUnit( dist, 3, mCanvas->mapUnits(), false );
      mResults->addDerivedAttribute( featureNode, tr( "Length" ), str );
      if ( f_it->geometry()->wkbType() == QGis::WKBLineString )
      {
        // Add the start and end points in as derived attributes
        str = QLocale::system().toString( f_it->geometry()->asPolyline().first().x(), 'g', 10 );
        mResults->addDerivedAttribute( featureNode, tr( "firstX", "attributes get sorted; translation for lastX should be lexically larger than this one" ), str );
        str = QLocale::system().toString( f_it->geometry()->asPolyline().first().y(), 'g', 10 );
        mResults->addDerivedAttribute( featureNode, tr( "firstY" ), str );
        str = QLocale::system().toString( f_it->geometry()->asPolyline().last().x(), 'g', 10 );
        mResults->addDerivedAttribute( featureNode, tr( "lastX", "attributes get sorted; translation for firstX should be lexically smaller than this one" ), str );
        str = QLocale::system().toString( f_it->geometry()->asPolyline().last().y(), 'g', 10 );
        mResults->addDerivedAttribute( featureNode, tr( "lastY" ), str );
      }
    }
    else if ( layer->geometryType() == QGis::Polygon )
    {
      double area = calc.measure( f_it->geometry() );
      QString str = calc.textUnit( area, 3, mCanvas->mapUnits(), true );
      mResults->addDerivedAttribute( featureNode, tr( "Area" ), str );
    }
    else if ( layer->geometryType() == QGis::Point )
    {
      // Include the x and y coordinates of the point as a derived attribute
      QString str;
      str = QLocale::system().toString( f_it->geometry()->asPoint().x(), 'g', 10 );
      mResults->addDerivedAttribute( featureNode, "X", str );
      str = QLocale::system().toString( f_it->geometry()->asPoint().y(), 'g', 10 );
      mResults->addDerivedAttribute( featureNode, "Y", str );
    }

    // Add actions
    QgsAttributeAction::aIter iter = actions.begin();
    for ( register int i = 0; iter != actions.end(); ++iter, ++i )
    {
      mResults->addAction( featureNode, i, tr( "action" ), iter->name() );
    }
  }

  QgsDebugMsg( "Feature count on identify: " + QString::number( featureCount ) );

  //also test the not commited features //todo: eliminate copy past code

  if ( featureCount == 1 )
  {
    mResults->showAllAttributes();
    highlightFeature( lastFeatureId );
  }
  else if ( featureCount == 0 )
  {
    mResults->setMessage( tr( "No features found" ), tr( "No features were found in the active layer at the point you clicked" ) );
  }

  mResults->setTitle( tr( "%1 - %n feature(s) found", "Identify results window title", featureCount ).arg( layer->name() ) );

  QApplication::restoreOverrideCursor();

  mResults->show();
}

void QgsMapToolIdentify::showError()
{
#if 0
  QMessageBox::warning(
    this,
    mapLayer->lastErrorTitle(),
    tr( "Could not draw %1 because:\n%2", "COMMENTED OUT" ).arg( mapLayer->name() ).arg( mapLayer->lastError() )
  );
#endif

  QgsMessageViewer * mv = new QgsMessageViewer();
  mv->setWindowTitle( mLayer->lastErrorTitle() );
  mv->setMessageAsPlainText( tr( "Could not identify objects on %1 because:\n%2" )
                             .arg( mLayer->name() ).arg( mLayer->lastError() ) );
  mv->exec(); // deletes itself on close
}

void QgsMapToolIdentify::resultsDialogGone()
{
  mResults = 0;

  delete mRubberBand;
  mRubberBand = 0;
}

void QgsMapToolIdentify::deactivate()
{
  if ( mResults )
    mResults->done( 0 ); // close the window
  QgsMapTool::deactivate();
}

void QgsMapToolIdentify::highlightFeature( int featureId )
{
  QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( mLayer );
  if ( !layer )
    return;

  delete mRubberBand;
  mRubberBand = 0;

  QgsFeature feat;
  if ( ! layer->featureAtId( featureId, feat, true, false ) )
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
    mRubberBand->setToGeometry( feat.geometry(), layer );
    mRubberBand->setWidth( 2 );
    mRubberBand->setColor( Qt::red );
    mRubberBand->show();
  }
}

void QgsMapToolIdentify::editFeature( int featureId )
{
  for ( QgsFeatureList::iterator it = mFeatureList.begin(); it != mFeatureList.end(); it++ )
  {
    if ( it->id() == featureId )
    {
      editFeature( *it );
      break;
    }
  }
}

void QgsMapToolIdentify::editFeature( QgsFeature &f )
{
  QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( mLayer );
  if ( !layer )
    return;

  if ( !layer->isEditable() )
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

void QgsMapToolIdentify::removeLayer( QString layerID )
{
  if ( mLayer )
  {
    if ( mLayer->type() == QgsMapLayer::VectorLayer )
    {
      if ( mLayer->getLayerID() == layerID )
      {
        if ( mResults )
        {
          mResults->clear();
          delete mRubberBand;
          mRubberBand = 0;
        }
        mLayer = 0;
      }
    }
  }
}
