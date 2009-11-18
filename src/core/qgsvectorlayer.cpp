/***************************************************************************
                               qgsvectorlayer.cpp
  This class implements a generic means to display vector layers. The features
  and attributes are read from the data store using a "data provider" plugin.
  QgsVectorLayer can be used with any data store for which an appropriate
  plugin is available.
                              -------------------
          begin                : Oct 29, 2003
          copyright            : (C) 2003 by Gary E.Sherman
          email                : sherman at mrcc.com

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

#include <cassert>
#include <cfloat>
#include <cstring>
#include <climits>
#include <cmath>
#include <iosfwd>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QSettings>
#include <QString>

#include "qgsvectorlayer.h"

// renderers
#include "qgscontinuouscolorrenderer.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgsrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsuniquevaluerenderer.h"

#include "qgsattributeaction.h"

#include "qgis.h" //for globals
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslabel.h"
#include "qgslogger.h"
#include "qgsmaptopixel.h"
#include "qgspoint.h"
#include "qgsproviderregistry.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgssinglesymbolrenderer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayerundocommand.h"
#include "qgsvectoroverlay.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsclipper.h"
#include "qgsproject.h"

#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2.h"
#include "qgssinglesymbolrendererv2.h"

#ifdef TESTPROVIDERLIB
#include <dlfcn.h>
#endif


static const char * const ident_ = "$Id$";

// typedef for the QgsDataProvider class factory
typedef QgsDataProvider * create_it( const QString* uri );



QgsVectorLayer::QgsVectorLayer( QString vectorLayerPath,
                                QString baseName,
                                QString providerKey,
                                bool loadDefaultStyleFlag )
    : QgsMapLayer( VectorLayer, baseName, vectorLayerPath ),
    mUpdateThreshold( 0 ),     // XXX better default value?
    mDataProvider( NULL ),
    mProviderKey( providerKey ),
    mEditable( false ),
    mModified( false ),
    mMaxUpdatedIndex( -1 ),
    mRenderer( 0 ),
    mLabel( 0 ),
    mLabelOn( false ),
    mActiveCommand( NULL ),
    mVertexMarkerOnlyForSelection( false ),
    mFetching( false ),
    mRendererV2( NULL ),
    mUsingRendererV2( false )
{
  mActions = new QgsAttributeAction;

  // if we're given a provider type, try to create and bind one to this layer
  if ( ! mProviderKey.isEmpty() )
  {
    setDataProvider( mProviderKey );
  }
  if ( mValid )
  {
    // Always set crs
    setCoordinateSystem();

    QSettings settings;
    if ( settings.value( "/qgis/use_symbology_ng", false ).toBool() )
    {
      // using symbology-ng!
      setUsingRendererV2( true );
    }

    // check if there is a default style / propertysheet defined
    // for this layer and if so apply it
    bool defaultLoadedFlag = false;
    if ( loadDefaultStyleFlag )
    {
      loadDefaultStyle( defaultLoadedFlag );
    }

    // if the default style failed to load or was disabled use some very basic defaults
    if ( !defaultLoadedFlag )
    {
      // add single symbol renderer
      if ( mUsingRendererV2 )
      {
        setRendererV2( QgsFeatureRendererV2::defaultRenderer( geometryType() ) );
      }
      else
      {
        QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( geometryType() );
        setRenderer( renderer );
      }
    }

    // Get the update threshold from user settings. We
    // do this only on construction to avoid the penality of
    // fetching this each time the layer is drawn. If the user
    // changes the threshold from the preferences dialog, it will
    // have no effect on existing layers
    // TODO: load this setting somewhere else [MD]
    //QSettings settings;
    //mUpdateThreshold = settings.readNumEntry("Map/updateThreshold", 1000);
  }
} // QgsVectorLayer ctor



QgsVectorLayer::~QgsVectorLayer()
{
  QgsDebugMsg( "In QgsVectorLayer destructor" );

  emit layerDeleted();

  mValid = false;

  if ( mRenderer )
  {
    delete mRenderer;
  }
  // delete the provider object
  delete mDataProvider;

  delete mLabel;

  // Destroy any cached geometries and clear the references to them
  deleteCachedGeometries();

  delete mActions;

  //delete remaining overlays

  QList<QgsVectorOverlay*>::iterator overlayIt = mOverlays.begin();
  for ( ; overlayIt != mOverlays.end(); ++overlayIt )
  {
    delete *overlayIt;
  }
}

QString QgsVectorLayer::storageType() const
{
  if ( mDataProvider )
  {
    return mDataProvider->storageType();
  }
  return 0;
}


QString QgsVectorLayer::capabilitiesString() const
{
  if ( mDataProvider )
  {
    return mDataProvider->capabilitiesString();
  }
  return 0;
}

QString QgsVectorLayer::dataComment() const
{
  if ( mDataProvider )
  {
    return mDataProvider->dataComment();
  }
  return QString();
}


QString QgsVectorLayer::providerType() const
{
  return mProviderKey;
}

/**
 * sets the preferred display field based on some fuzzy logic
 */
void QgsVectorLayer::setDisplayField( QString fldName )
{
  // If fldName is provided, use it as the display field, otherwise
  // determine the field index for the feature column of the identify
  // dialog. We look for fields containing "name" first and second for
  // fields containing "id". If neither are found, the first field
  // is used as the node.
  QString idxName = "";
  QString idxId = "";

  if ( !fldName.isEmpty() )
  {
    mDisplayField = fldName;
  }
  else
  {
    const QgsFieldMap &fields = pendingFields();
    int fieldsSize = fields.size();

    for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
    {
      QString fldName = it.value().name();
      QgsDebugMsg( "Checking field " + fldName + " of " + QString::number( fieldsSize ) + " total" );

      // Check the fields and keep the first one that matches.
      // We assume that the user has organized the data with the
      // more "interesting" field names first. As such, name should
      // be selected before oldname, othername, etc.
      if ( fldName.indexOf( "name", false ) > -1 )
      {
        if ( idxName.isEmpty() )
        {
          idxName = fldName;
        }
      }
      if ( fldName.indexOf( "descrip", false ) > -1 )
      {
        if ( idxName.isEmpty() )
        {
          idxName = fldName;
        }
      }
      if ( fldName.indexOf( "id", false ) > -1 )
      {
        if ( idxId.isEmpty() )
        {
          idxId = fldName;
        }
      }
    }

    //if there were no fields in the dbf just return - otherwise qgis segfaults!
    if ( fieldsSize == 0 )
      return;

    if ( idxName.length() > 0 )
    {
      mDisplayField = idxName;
    }
    else
    {
      if ( idxId.length() > 0 )
      {
        mDisplayField = idxId;
      }
      else
      {
        mDisplayField = fields[0].name();
      }
    }

  }
}

// NOTE this is a temporary method added by Tim to prevent label clipping
// which was occurring when labeller was called in the main draw loop
// This method will probably be removed again in the near future!
void QgsVectorLayer::drawLabels( QgsRenderContext& rendererContext )
{
  QgsDebugMsg( "Starting draw of labels" );

  if ( mRenderer && mLabelOn &&
       ( !label()->scaleBasedVisibility() ||
         ( label()->minScale() <= rendererContext.rendererScale() &&
           rendererContext.rendererScale() <= label()->maxScale() ) ) )
  {
    QgsAttributeList attributes = mRenderer->classificationAttributes();

    // Add fields required for labels
    mLabel->addRequiredFields( attributes );

    QgsDebugMsg( "Selecting features based on view extent" );

    int featureCount = 0;

    try
    {
      // select the records in the extent. The provider sets a spatial filter
      // and sets up the selection set for retrieval
      select( attributes, rendererContext.extent() );

      QgsFeature fet;
      while ( nextFeature( fet ) )
      {
        if ( mRenderer->willRenderFeature( &fet ) )
        {
          bool sel = mSelectedFeatureIds.contains( fet.id() );
          mLabel->renderLabel( rendererContext, fet, sel, 0 );
        }
        featureCount++;
      }
    }
    catch ( QgsCsException &e )
    {
      Q_UNUSED( e );
      QgsLogger::critical( "Error projecting label locations, caught in " + QString( __FILE__ ) + ", line " + QString( __LINE__ ) );
    }

#ifdef QGISDEBUG
    QgsLogger::debug( "Total features processed", featureCount, 1, __FILE__, __FUNCTION__, __LINE__ );
#endif

    // XXX Something in our draw event is triggering an additional draw event when resizing [TE 01/26/06]
    // XXX Calling this will begin processing the next draw event causing image havoc and recursion crashes.
    //qApp->processEvents();

  }
}


unsigned char *QgsVectorLayer::drawLineString( unsigned char *feature, QgsRenderContext &renderContext )
{
  QPainter *p = renderContext.painter();
  unsigned char *ptr = feature + 5;
  unsigned int wkbType = *(( int* )( feature + 1 ) );
  unsigned int nPoints = *(( int* )ptr );
  ptr = feature + 9;

  bool hasZValue = ( wkbType == QGis::WKBLineString25D );

  std::vector<double> x( nPoints );
  std::vector<double> y( nPoints );
  std::vector<double> z( nPoints, 0.0 );

  // Extract the points from the WKB format into the x and y vectors.
  for ( register unsigned int i = 0; i < nPoints; ++i )
  {
    x[i] = *(( double * ) ptr );
    ptr += sizeof( double );
    y[i] = *(( double * ) ptr );
    ptr += sizeof( double );

    if ( hasZValue ) // ignore Z value
      ptr += sizeof( double );
  }

  // Transform the points into map coordinates (and reproject if
  // necessary)

  transformPoints( x, y, z, renderContext );

  // Work around a +/- 32768 limitation on coordinates
  // Look through the x and y coordinates and see if there are any
  // that need trimming. If one is found, there's no need to look at
  // the rest of them so end the loop at that point.
  for ( register unsigned int i = 0; i < nPoints; ++i )
  {
    if ( std::abs( x[i] ) > QgsClipper::MAX_X ||
         std::abs( y[i] ) > QgsClipper::MAX_Y )
    {
      QgsClipper::trimFeature( x, y, true ); // true = polyline
      nPoints = x.size(); // trimming may change nPoints.
      break;
    }
  }

  // set up QPolygonF class with transformed points
  QPolygonF pa( nPoints );
  for ( register unsigned int i = 0; i < nPoints; ++i )
  {
    pa[i].setX( x[i] );
    pa[i].setY( y[i] );
  }

#ifdef QGISDEBUGVERBOSE
  // this is only used for verbose debug output
  for ( int i = 0; i < pa.size(); ++i )
  {
    QgsDebugMsgLevel( "pa" + QString::number( pa.point( i ).x() ), 2 );
    QgsDebugMsgLevel( "pa" + QString::number( pa.point( i ).y() ), 2 );
  }
#endif

  // The default pen gives bevelled joins between segements of the
  // polyline, which is good enough for the moment.
  //preserve a copy of the pen before we start fiddling with it
  QPen pen = p->pen(); // to be kept original

  //
  // experimental alpha transparency
  // 255 = opaque
  //
  QPen myTransparentPen = p->pen(); // store current pen
  QColor myColor = myTransparentPen.color();
  //only set transparency from layer level if renderer does not provide
  //transparency on class level
  if ( !mRenderer->usesTransparency() )
  {
    myColor.setAlpha( mTransparencyLevel );
  }
  myTransparentPen.setColor( myColor );
  p->setPen( myTransparentPen );
  p->drawPolyline( pa );

  // draw vertex markers if in editing mode, but only to the main canvas
  if ( mEditable && renderContext.drawEditingInformation() )
  {

    std::vector<double>::const_iterator xIt;
    std::vector<double>::const_iterator yIt;
    for ( xIt = x.begin(), yIt = y.begin(); xIt != x.end(); ++xIt, ++yIt )
    {
      drawVertexMarker(( int )( *xIt ), ( int )( *yIt ), *p, mCurrentVertexMarkerType, mCurrentVertexMarkerSize );
    }
  }

  //restore the pen
  p->setPen( pen );

  return ptr;
}

unsigned char *QgsVectorLayer::drawPolygon( unsigned char *feature, QgsRenderContext &renderContext )
{
  QPainter *p = renderContext.painter();
  typedef std::pair<std::vector<double>, std::vector<double> > ringType;
  typedef ringType* ringTypePtr;
  typedef std::vector<ringTypePtr> ringsType;

  // get number of rings in the polygon
  unsigned int numRings = *(( int* )( feature + 1 + sizeof( int ) ) );

  if ( numRings == 0 )  // sanity check for zero rings in polygon
    return feature + 9;

  unsigned int wkbType = *(( int* )( feature + 1 ) );

  bool hasZValue = ( wkbType == QGis::WKBPolygon25D );

  int total_points = 0;

  // A vector containing a pointer to a pair of double vectors.The
  // first vector in the pair contains the x coordinates, and the
  // second the y coordinates.
  ringsType rings;

  // Set pointer to the first ring
  unsigned char* ptr = feature + 1 + 2 * sizeof( int );

  for ( register unsigned int idx = 0; idx < numRings; idx++ )
  {
    unsigned int nPoints = *(( int* )ptr );

    ringTypePtr ring = new ringType( std::vector<double>( nPoints ), std::vector<double>( nPoints ) );
    ptr += 4;

    // create a dummy vector for the z coordinate
    std::vector<double> zVector( nPoints, 0.0 );
    // Extract the points from the WKB and store in a pair of
    // vectors.
    for ( register unsigned int jdx = 0; jdx < nPoints; jdx++ )
    {
      ring->first[jdx] = *(( double * ) ptr );
      ptr += sizeof( double );
      ring->second[jdx] = *(( double * ) ptr );
      ptr += sizeof( double );

      if ( hasZValue )
        ptr += sizeof( double );
    }
    // If ring has fewer than two points, what is it then?
    // Anyway, this check prevents a crash
    if ( nPoints < 1 )
    {
      QgsDebugMsg( "Ring has only " + QString::number( nPoints ) + " points! Skipping this ring." );
      continue;
    }

    transformPoints( ring->first, ring->second, zVector, renderContext );

    // Work around a +/- 32768 limitation on coordinates
    // Look through the x and y coordinates and see if there are any
    // that need trimming. If one is found, there's no need to look at
    // the rest of them so end the loop at that point.
    for ( register unsigned int i = 0; i < nPoints; ++i )
    {
      if ( std::abs( ring->first[i] ) > QgsClipper::MAX_X ||
           std::abs( ring->second[i] ) > QgsClipper::MAX_Y )
      {
        QgsClipper::trimFeature( ring->first, ring->second, false );
        break;
      }
    }

    // Don't bother keeping the ring if it has been trimmed out of
    // existence.
    if ( ring->first.size() == 0 )
      delete ring;
    else
    {
      rings.push_back( ring );
      total_points += ring->first.size();
    }
  }

  // Now we draw the polygons

  // use painter paths for drawing polygons with holes
  // when adding polygon to the path they invert the area
  // this means that adding inner rings to the path creates
  // holes in outer ring
  QPainterPath path; // OddEven fill rule by default

  // Only try to draw polygons if there is something to draw
  if ( total_points > 0 )
  {
    // Store size here and use it in the loop to avoid penalty of
    // multiple calls to size()
    int numRings = rings.size();
    for ( register int i = 0; i < numRings; ++i )
    {
      // Store the pointer in a variable with a short name so as to make
      // the following code easier to type and read.
      ringTypePtr r = rings[i];
      // only do this once to avoid penalty of additional calls
      unsigned ringSize = r->first.size();

      // Transfer points to the array of QPointF
      QPolygonF pa( ringSize );
      for ( register unsigned int j = 0; j != ringSize; ++j )
      {
        pa[j].setX( r->first[j] );
        pa[j].setY( r->second[j] );
      }

      path.addPolygon( pa );

      // Tidy up the pointed to pairs of vectors as we finish with them
      delete rings[i];
    }

#if 0
    // A bit of code to aid in working out what values of
    // QgsClipper::minX, etc cause the X11 zoom bug.
    int largestX  = -std::numeric_limits<int>::max();
    int smallestX = std::numeric_limits<int>::max();
    int largestY  = -std::numeric_limits<int>::max();
    int smallestY = std::numeric_limits<int>::max();

    for ( int i = 0; i < pa.size(); ++i )
    {
      largestX  = std::max( largestX,  pa.point( i ).x() );
      smallestX = std::min( smallestX, pa.point( i ).x() );
      largestY  = std::max( largestY,  pa.point( i ).y() );
      smallestY = std::min( smallestY, pa.point( i ).y() );
    }
    QgsDebugMsg( QString( "Largest  X coordinate was %1" ).arg( largestX ) );
    QgsDebugMsg( QString( "Smallest X coordinate was %1" ).arg( smallestX ) );
    QgsDebugMsg( QString( "Largest  Y coordinate was %1" ).arg( largestY ) );
    QgsDebugMsg( QString( "Smallest Y coordinate was %1" ).arg( smallestY ) );
#endif

    //preserve a copy of the brush and pen before we start fiddling with it
    QBrush brush = p->brush(); //to be kept as original
    QPen pen = p->pen(); // to be kept original
    //
    // experimental alpha transparency
    // 255 = opaque
    //
    QBrush myTransparentBrush = p->brush();
    QColor myColor = brush.color();

    //only set transparency from layer level if renderer does not provide
    //transparency on class level
    if ( !mRenderer->usesTransparency() )
    {
      myColor.setAlpha( mTransparencyLevel );
    }
    myTransparentBrush.setColor( myColor );
    QPen myTransparentPen = p->pen(); // store current pen
    myColor = myTransparentPen.color();

    //only set transparency from layer level if renderer does not provide
    //transparency on class level
    if ( !mRenderer->usesTransparency() )
    {
      myColor.setAlpha( mTransparencyLevel );
    }
    myTransparentPen.setColor( myColor );

    p->setBrush( myTransparentBrush );
    p->setPen( myTransparentPen );

    //
    // draw the polygon
    //
    p->drawPath( path );


    // draw vertex markers if in editing mode, but only to the main canvas
    if ( mEditable && renderContext.drawEditingInformation() )
    {
      for ( int i = 0; i < path.elementCount(); ++i )
      {
        const QPainterPath::Element & e = path.elementAt( i );
        drawVertexMarker(( int )e.x, ( int )e.y, *p, mCurrentVertexMarkerType, mCurrentVertexMarkerSize );
      }
    }

    //
    //restore brush and pen to original
    //
    p->setBrush( brush );
    p->setPen( pen );

  } // totalPoints > 0

  return ptr;
}

void QgsVectorLayer::drawRendererV2( QgsRenderContext& rendererContext, bool labeling )
{
  mRendererV2->startRender( rendererContext, pendingFields() );

  QgsSingleSymbolRendererV2* selRenderer = NULL;
  if ( !mSelectedFeatureIds.isEmpty() )
  {
    selRenderer = new QgsSingleSymbolRendererV2( QgsSymbolV2::defaultSymbol( geometryType() ) );
    selRenderer->symbol()->setColor( QgsRenderer::selectionColor() );
    selRenderer->startRender( rendererContext, pendingFields() );
  }

  QgsFeature fet;
  while ( nextFeature( fet ) )
  {
    if ( mSelectedFeatureIds.contains( fet.id() ) )
      selRenderer->renderFeature( fet, rendererContext );
    else
      mRendererV2->renderFeature( fet, rendererContext );

    if ( labeling )
      rendererContext.labelingEngine()->registerFeature( this, fet );
  }

  mRendererV2->stopRender( rendererContext );

  if ( selRenderer )
  {
    selRenderer->stopRender( rendererContext );
    delete selRenderer;
  }
}

void QgsVectorLayer::drawRendererV2Levels( QgsRenderContext& rendererContext, bool labeling )
{
  QHash< QgsSymbolV2*, QList<QgsFeature> > features; // key = symbol, value = array of features

  // startRender must be called before symbolForFeature() calls to make sure renderer is ready
  mRendererV2->startRender( rendererContext, pendingFields() );

  QgsSingleSymbolRendererV2* selRenderer = NULL;
  if ( !mSelectedFeatureIds.isEmpty() )
  {
    selRenderer = new QgsSingleSymbolRendererV2( QgsSymbolV2::defaultSymbol( geometryType() ) );
    selRenderer->symbol()->setColor( QgsRenderer::selectionColor() );
    selRenderer->startRender( rendererContext, pendingFields() );
  }

  // 1. fetch features
  QgsFeature fet;
  while ( nextFeature( fet ) )
  {
    QgsSymbolV2* sym = mRendererV2->symbolForFeature( fet );
    if ( !features.contains( sym ) )
    {
      features.insert( sym, QList<QgsFeature>() );
    }
    features[sym].append( fet );

    if ( labeling )
      rendererContext.labelingEngine()->registerFeature( this, fet );
  }

  // find out the order
  QgsSymbolV2LevelOrder levels;
  QgsSymbolV2List symbols = mRendererV2->symbols();
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbolV2* sym = symbols[i];
    for ( int j = 0; j < sym->symbolLayerCount(); j++ )
    {
      int level = sym->symbolLayer( j )->renderingPass();
      QgsSymbolV2LevelItem item( sym, j );
      while ( level >= levels.count() ) // append new empty levels
        levels.append( QgsSymbolV2Level() );
      levels[level].append( item );
    }
  }

  // 2. draw features in correct order
  for ( int l = 0; l < levels.count(); l++ )
  {
    QgsSymbolV2Level& level = levels[l];
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolV2LevelItem& item = level[i];
      if ( !features.contains( item.symbol() ) )
      {
        QgsDebugMsg( "level item's symbol not found!" );
        continue;
      }
      int layer = item.layer();
      QList<QgsFeature>& lst = features[item.symbol()];
      QList<QgsFeature>::iterator fit;
      for ( fit = lst.begin(); fit != lst.end(); ++fit )
      {
        if ( mSelectedFeatureIds.contains( fit->id() ) )
          selRenderer->renderFeature( *fit, rendererContext );
        else
          mRendererV2->renderFeature( *fit, rendererContext, layer );
      }
    }
  }

  mRendererV2->stopRender( rendererContext );

  if ( selRenderer )
  {
    selRenderer->stopRender( rendererContext );
    delete selRenderer;
  }
}

bool QgsVectorLayer::draw( QgsRenderContext& rendererContext )
{
  if ( mUsingRendererV2 )
  {
    if ( mRendererV2 == NULL )
      return FALSE;

    QgsDebugMsg( "rendering v2:\n" + mRendererV2->dump() );

    // TODO: really needed?
    updateFeatureCount();

    QgsAttributeList attributes;
    foreach( QString attrName, mRendererV2->usedAttributes() )
    {
      int attrNum = QgsFeatureRendererV2::fieldNameIndex( pendingFields(), attrName );
      attributes.append( attrNum );
      QgsDebugMsg( "attrs: " + attrName + " - " + QString::number( attrNum ) );
    }

    bool labeling = FALSE;
    if ( rendererContext.labelingEngine() )
    {
      int attrIndex;
      if ( rendererContext.labelingEngine()->prepareLayer( this, attrIndex ) )
      {
        if ( !attributes.contains( attrIndex ) )
          attributes << attrIndex;
        labeling = TRUE;
      }
    }

    select( attributes, rendererContext.extent() );

    if ( mRendererV2->usingSymbolLevels() )
      drawRendererV2Levels( rendererContext, labeling );
    else
      drawRendererV2( rendererContext, labeling );

    return TRUE;
  }

  //set update threshold before each draw to make sure the current setting is picked up
  QSettings settings;
  mUpdateThreshold = settings.value( "Map/updateThreshold", 0 ).toInt();
  //draw ( p, viewExtent, theMapToPixelTransform, ct, drawingToEditingCanvas, 1., 1.);

  if ( mRenderer )
  {
    // painter is active (begin has been called
    /* Steps to draw the layer
       1. get the features in the view extent by SQL query
       2. read WKB for a feature
       3. transform
       4. draw
    */

    QPen pen;
    /*Pointer to a marker image*/
    QImage marker;
    //vertex marker type for selection
    QgsVectorLayer::VertexMarkerType vertexMarker = QgsVectorLayer::NoMarker;
    int vertexMarkerSize = 7;

    if ( mEditable )
    {
      // Destroy all cached geometries and clear the references to them
      deleteCachedGeometries();
      mCachedGeometriesRect = rendererContext.extent();
      vertexMarker = currentVertexMarkerType();
      vertexMarkerSize = currentVertexMarkerSize();
      mVertexMarkerOnlyForSelection = settings.value( "/qgis/digitizing/marker_only_for_selected", false ).toBool();
    }

    updateFeatureCount();
    int totalFeatures = pendingFeatureCount();
    int featureCount = 0;
    QgsFeature fet;
    QgsAttributeList attributes = mRenderer->classificationAttributes();

    bool labeling = FALSE;
    if ( rendererContext.labelingEngine() )
    {
      int attrIndex;
      if ( rendererContext.labelingEngine()->prepareLayer( this, attrIndex ) )
      {
        if ( !attributes.contains( attrIndex ) )
          attributes << attrIndex;
        labeling = TRUE;
      }
    }

    select( attributes, rendererContext.extent() );

    try
    {
      while ( nextFeature( fet ) )
      {

        if ( rendererContext.renderingStopped() )
        {
          break;
        }

#ifndef Q_WS_MAC //MH: disable this on Mac for now to avoid problems with resizing
        if ( mUpdateThreshold > 0 && 0 == featureCount % mUpdateThreshold )
        {
          emit screenUpdateRequested();
          emit drawingProgress( featureCount, totalFeatures );
          qApp->processEvents();
        }
        else if ( featureCount % 1000 == 0 )
        {
          emit drawingProgress( featureCount, totalFeatures );
          qApp->processEvents();
        }
#else
        Q_UNUSED( totalFeatures );
#endif //Q_WS_MAC

        // check if feature is selected
        // only show selections of the current layer
        // TODO: create a mechanism to let layer know whether it's current layer or not [MD]
        bool sel = mSelectedFeatureIds.contains( fet.id() );

        mCurrentVertexMarkerType = QgsVectorLayer::NoMarker;
        mCurrentVertexMarkerSize = 7;

        if ( mEditable )
        {
          // Cache this for the use of (e.g.) modifying the feature's uncommitted geometry.
          mCachedGeometries[fet.id()] = *fet.geometry();

          if ( !mVertexMarkerOnlyForSelection || sel )
          {
            mCurrentVertexMarkerType = vertexMarker;
            mCurrentVertexMarkerSize = vertexMarkerSize;
          }
        }

        //QgsDebugMsg(QString("markerScale before renderFeature(): %1").arg(markerScaleFactor));
        // markerScalerFactore reflects the wanted scaling of the marker

        double opacity = 1.0;
        if ( !mRenderer->usesTransparency() )
        {
          opacity = ( mTransparencyLevel * 1.0 ) / 255.0;
        }
        mRenderer->renderFeature( rendererContext, fet, &marker, sel, opacity );

        // markerScalerFactore now reflects the actual scaling of the marker that the render performed.
        //QgsDebugMsg(QString("markerScale after renderFeature(): %1").arg(markerScaleFactor));

        //double scale = rendererContext.scaleFactor() /  markerScaleFactor;
        drawFeature( rendererContext, fet, &marker );

        if ( labeling )
        {
          rendererContext.labelingEngine()->registerFeature( this, fet );
        }

        ++featureCount;
      }
    }
    catch ( QgsCsException &cse )
    {
      QString msg( "Failed to transform a point while drawing a feature of type '"
                   + fet.typeName() + "'. Ignoring this feature." );
      msg += cse.what();
      QgsLogger::warning( msg );
    }

  }
  else
  {
    QgsLogger::warning( "QgsRenderer is null in QgsVectorLayer::draw()" );
  }

  if ( mEditable )
  {
    QgsDebugMsg( QString( "Cached %1 geometries." ).arg( mCachedGeometries.count() ) );
  }

  return TRUE; // Assume success always
}

void QgsVectorLayer::deleteCachedGeometries()
{
  // Destroy any cached geometries
  mCachedGeometries.clear();
  mCachedGeometriesRect = QgsRectangle();
}

void QgsVectorLayer::drawVertexMarker( int x, int y, QPainter& p, QgsVectorLayer::VertexMarkerType type, int m )
{
  if ( type == QgsVectorLayer::SemiTransparentCircle )
  {
    p.setPen( QColor( 50, 100, 120, 200 ) );
    p.setBrush( QColor( 200, 200, 210, 120 ) );
    p.drawEllipse( x - m, y - m, m*2 + 1, m*2 + 1 );
  }
  else if ( type == QgsVectorLayer::Cross )
  {
    p.setPen( QColor( 255, 0, 0 ) );
    p.drawLine( x - m, y + m, x + m, y - m );
    p.drawLine( x - m, y - m, x + m, y + m );
  }
}

void QgsVectorLayer::select( int number, bool emitSignal )
{
  mSelectedFeatureIds.insert( number );

  if ( emitSignal )
  {
    emit selectionChanged();
  }
}

void QgsVectorLayer::deselect( int number, bool emitSignal )
{
  mSelectedFeatureIds.remove( number );

  if ( emitSignal )
  {
    emit selectionChanged();
  }
}

void QgsVectorLayer::select( QgsRectangle & rect, bool lock )
{
  // normalize the rectangle
  rect.normalize();

  if ( lock == false )
  {
    removeSelection( FALSE ); // don't emit signal
  }

  //select all the elements
  select( QgsAttributeList(), rect, false, true );

  QgsFeature f;
  while ( nextFeature( f ) )
  {
    select( f.id(), false ); // don't emit signal (not to redraw it everytime)
  }

  emit selectionChanged(); // now emit signal to redraw layer
}

void QgsVectorLayer::invertSelection()
{
  // copy the ids of selected features to tmp
  QgsFeatureIds tmp = mSelectedFeatureIds;

  removeSelection( FALSE ); // don't emit signal

  select( QgsAttributeList(), QgsRectangle(), false );

  QgsFeature fet;
  while ( nextFeature( fet ) )
  {
    select( fet.id(), false ); // don't emit signal
  }

  for ( QgsFeatureIds::iterator iter = tmp.begin(); iter != tmp.end(); ++iter )
  {
    mSelectedFeatureIds.remove( *iter );
  }

  emit selectionChanged();
}

void QgsVectorLayer::invertSelectionInRectangle( QgsRectangle & rect )
{
  // normalize the rectangle
  rect.normalize();

  select( QgsAttributeList(), rect, false, true );

  QgsFeature fet;
  while ( nextFeature( fet ) )
  {
    if ( mSelectedFeatureIds.contains( fet.id() ) )
    {
      deselect( fet.id(), false ); // don't emit signal
    }
    else
    {
      select( fet.id(), false ); // don't emit signal
    }
  }

  emit selectionChanged();
}

void QgsVectorLayer::removeSelection( bool emitSignal )
{
  mSelectedFeatureIds.clear();

  if ( emitSignal )
    emit selectionChanged();
}

void QgsVectorLayer::triggerRepaint()
{
  emit repaintRequested();
}

QgsVectorDataProvider* QgsVectorLayer::dataProvider()
{
  return mDataProvider;
}

const QgsVectorDataProvider* QgsVectorLayer::dataProvider() const
{
  return mDataProvider;
}

void QgsVectorLayer::setProviderEncoding( const QString& encoding )
{
  if ( mDataProvider )
  {
    mDataProvider->setEncoding( encoding );
  }
}


const QgsRenderer* QgsVectorLayer::renderer() const
{
  return mRenderer;
}

void QgsVectorLayer::setRenderer( QgsRenderer * r )
{
  if ( r != mRenderer )
  {
    delete mRenderer;
    mRenderer = r;
  }
}

QGis::GeometryType QgsVectorLayer::geometryType() const
{
  if ( mDataProvider )
  {
    int type = mDataProvider->geometryType();
    switch ( type )
    {
      case QGis::WKBPoint:
      case QGis::WKBPoint25D:
        return QGis::Point;

      case QGis::WKBLineString:
      case QGis::WKBLineString25D:
        return QGis::Line;

      case QGis::WKBPolygon:
      case QGis::WKBPolygon25D:
        return QGis::Polygon;

      case QGis::WKBMultiPoint:
      case QGis::WKBMultiPoint25D:
        return QGis::Point;

      case QGis::WKBMultiLineString:
      case QGis::WKBMultiLineString25D:
        return QGis::Line;

      case QGis::WKBMultiPolygon:
      case QGis::WKBMultiPolygon25D:
        return QGis::Polygon;
    }
    QgsDebugMsg( QString( "Data Provider Geometry type is not recognised, is %1" ).arg( type ) );
  }
  else
  {
    QgsDebugMsg( "pointer to mDataProvider is null" );
  }

  // We shouldn't get here, and if we have, other things are likely to
  // go wrong. Code that uses the type() return value should be
  // rewritten to cope with a value of QGis::Unknown. To make this
  // need known, the following message is printed every time we get
  // here.
  QgsDebugMsg( "WARNING: This code should never be reached. Problems may occur..." );

  return QGis::UnknownGeometry;
}

QGis::WkbType QgsVectorLayer::wkbType() const
{
  return ( QGis::WkbType )( mWkbType );
}

QgsRectangle QgsVectorLayer::boundingBoxOfSelected()
{
  if ( mSelectedFeatureIds.size() == 0 )//no selected features
  {
    return QgsRectangle( 0, 0, 0, 0 );
  }

  QgsRectangle r, retval;


  select( QgsAttributeList(), QgsRectangle(), true );

  retval.setMinimal();

  QgsFeature fet;
  while ( nextFeature( fet ) )
  {
    if ( mSelectedFeatureIds.contains( fet.id() ) )
    {
      if ( fet.geometry() )
      {
        r = fet.geometry()->boundingBox();
        retval.combineExtentWith( &r );
      }
    }
  }

  if ( retval.width() == 0.0 || retval.height() == 0.0 )
  {
    // If all of the features are at the one point, buffer the
    // rectangle a bit. If they are all at zero, do something a bit
    // more crude.

    if ( retval.xMinimum() == 0.0 && retval.xMaximum() == 0.0 &&
         retval.yMinimum() == 0.0 && retval.yMaximum() == 0.0 )
    {
      retval.set( -1.0, -1.0, 1.0, 1.0 );
    }
  }

  return retval;
}



long QgsVectorLayer::featureCount() const
{
  if ( !mDataProvider )
  {
    QgsLogger::warning( " QgsVectorLayer::featureCount() invoked with null mDataProvider" );
    return 0;
  }

  return mDataProvider->featureCount();
} // QgsVectorLayer::featureCount

long QgsVectorLayer::updateFeatureCount() const
{
  if ( !mDataProvider )
  {
    QgsLogger::warning( " QgsVectorLayer::updateFeatureCount() invoked with null mDataProvider" );
    return 0;
  }
  return mDataProvider->updateFeatureCount();
}

void QgsVectorLayer::updateExtents()
{
  mLayerExtent.setMinimal();

  if ( !mDataProvider )
    QgsLogger::warning( " QgsVectorLayer::updateExtents() invoked with null mDataProvider" );

  if ( mDeletedFeatureIds.isEmpty() && mChangedGeometries.isEmpty() )
  {
    // get the extent of the layer from the provider
    // but only when there are some features already
    if ( mDataProvider->featureCount() != 0 )
    {
      QgsRectangle r = mDataProvider->extent();
      mLayerExtent.combineExtentWith( &r );
    }

    for ( QgsFeatureList::iterator it = mAddedFeatures.begin(); it != mAddedFeatures.end(); it++ )
    {
      QgsRectangle r = it->geometry()->boundingBox();
      mLayerExtent.combineExtentWith( &r );
    }
  }
  else
  {
    select( QgsAttributeList(), QgsRectangle(), true );

    QgsFeature fet;
    while ( nextFeature( fet ) )
    {
      if ( fet.geometry() )
      {
        QgsRectangle bb = fet.geometry()->boundingBox();
        mLayerExtent.combineExtentWith( &bb );
      }
    }
  }

  if ( mLayerExtent.xMinimum() > mLayerExtent.xMaximum() && mLayerExtent.yMinimum() > mLayerExtent.yMaximum() )
  {
    // special case when there are no features in provider nor any added
    mLayerExtent = QgsRectangle(); // use rectangle with zero coordinates
  }

  // Send this (hopefully) up the chain to the map canvas
  emit recalculateExtents();
}

QString QgsVectorLayer::subsetString()
{
  if ( ! mDataProvider )
  {
    QgsLogger::warning( " QgsVectorLayer::subsetString() invoked with null mDataProvider" );
    return 0;
  }
  return mDataProvider->subsetString();
}

bool QgsVectorLayer::setSubsetString( QString subset )
{
  if ( ! mDataProvider )
  {
    QgsLogger::warning( " QgsVectorLayer::setSubsetString() invoked with null mDataProvider" );
    return false;
  }

  bool res = mDataProvider->setSubsetString( subset );

  // get the updated data source string from the provider
  mDataSource = mDataProvider->dataSourceUri();
  updateExtents();

  return res;
}

void QgsVectorLayer::updateFeatureAttributes( QgsFeature &f )
{
  // do not update when we aren't in editing mode
  if ( !mEditable )
    return;

  if ( mChangedAttributeValues.contains( f.id() ) )
  {
    const QgsAttributeMap &map = mChangedAttributeValues[f.id()];
    for ( QgsAttributeMap::const_iterator it = map.begin(); it != map.end(); it++ )
      f.changeAttribute( it.key(), it.value() );
  }

  // remove all attributes that will disappear
  QgsAttributeMap map = f.attributeMap();
  for ( QgsAttributeMap::const_iterator it = map.begin(); it != map.end(); it++ )
    if ( !mUpdatedFields.contains( it.key() ) )
      f.deleteAttribute( it.key() );

  // null/add all attributes that were added, but don't exist in the feature yet
  for ( QgsFieldMap::const_iterator it = mUpdatedFields.begin(); it != mUpdatedFields.end(); it++ )
    if ( !map.contains( it.key() ) )
      f.changeAttribute( it.key(), QVariant( QString::null ) );
}

void QgsVectorLayer::updateFeatureGeometry( QgsFeature &f )
{
  if ( mChangedGeometries.contains( f.id() ) )
    f.setGeometry( mChangedGeometries[f.id()] );
}


void QgsVectorLayer::select( QgsAttributeList attributes, QgsRectangle rect, bool fetchGeometries, bool useIntersect )
{
  if ( !mDataProvider )
    return;

  mFetching        = true;
  mFetchRect       = rect;
  mFetchAttributes = attributes;
  mFetchGeometry   = fetchGeometries;

  mFetchConsidered = mDeletedFeatureIds;

  if ( mEditable )
  {
    mFetchAddedFeaturesIt = mAddedFeatures.begin();
    mFetchChangedGeomIt = mChangedGeometries.begin();
  }

  //look in the normal features of the provider
  if ( mFetchAttributes.size() > 0 )
  {
    if ( mEditable )
    {
      // fetch only available field from provider
      QgsAttributeList provAttributes;
      for ( QgsAttributeList::iterator it = mFetchAttributes.begin(); it != mFetchAttributes.end(); it++ )
      {
        if ( !mUpdatedFields.contains( *it ) || mAddedAttributeIds.contains( *it ) )
          continue;

        provAttributes << *it;
      }

      mDataProvider->select( provAttributes, rect, fetchGeometries, useIntersect );
    }
    else
      mDataProvider->select( mFetchAttributes, rect, fetchGeometries, useIntersect );
  }
  else
  {
    mDataProvider->select( QgsAttributeList(), rect, fetchGeometries, useIntersect );
  }
}

bool QgsVectorLayer::nextFeature( QgsFeature &f )
{
  if ( !mFetching )
    return false;

  if ( mEditable )
  {
    if ( !mFetchRect.isEmpty() )
    {
      // check if changed geometries are in rectangle
      for ( ; mFetchChangedGeomIt != mChangedGeometries.end(); mFetchChangedGeomIt++ )
      {
        int fid = mFetchChangedGeomIt.key();

        if ( mFetchConsidered.contains( fid ) )
          // skip deleted features
          continue;

        mFetchConsidered << fid;

        if ( !mFetchChangedGeomIt->intersects( mFetchRect ) )
          // skip changed geometries not in rectangle and don't check again
          continue;

        f.setFeatureId( fid );
        f.setValid( true );

        if ( mFetchGeometry )
          f.setGeometry( mFetchChangedGeomIt.value() );

        if ( mFetchAttributes.size() > 0 )
        {
          if ( fid < 0 )
          {
            // fid<0 => in mAddedFeatures
            bool found = false;

            for ( QgsFeatureList::iterator it = mAddedFeatures.begin(); it != mAddedFeatures.end(); it++ )
            {
              if ( fid != it->id() )
              {
                found = true;
                f.setAttributeMap( it->attributeMap() );
                break;
              }
            }

            if ( !found )
              QgsLogger::warning( QString( "No attributes for the added feature %1 found" ).arg( f.id() ) );
          }
          else
          {
            // retrieve attributes from provider
            QgsFeature tmp;
            mDataProvider->featureAtId( fid, tmp, false, mDataProvider->attributeIndexes() );
            updateFeatureAttributes( tmp );
            f.setAttributeMap( tmp.attributeMap() );
          }
        }

        // return complete feature
        mFetchChangedGeomIt++;
        return true;
      }

      // no more changed geometries
    }

    for ( ; mFetchAddedFeaturesIt != mAddedFeatures.end(); mFetchAddedFeaturesIt++ )
    {
      int fid = mFetchAddedFeaturesIt->id();

      if ( mFetchConsidered.contains( fid ) )
        // must have changed geometry outside rectangle
        continue;

      if ( !mFetchRect.isEmpty() &&
           mFetchAddedFeaturesIt->geometry() &&
           !mFetchAddedFeaturesIt->geometry()->intersects( mFetchRect ) )
        // skip added features not in rectangle
        continue;

      f.setFeatureId( fid );
      f.setValid( true );

      if ( mFetchGeometry )
        f.setGeometry( *mFetchAddedFeaturesIt->geometry() );

      if ( mFetchAttributes.size() > 0 )
      {
        f.setAttributeMap( mFetchAddedFeaturesIt->attributeMap() );
        updateFeatureAttributes( f );
      }

      mFetchAddedFeaturesIt++;
      return true;
    }

    // no more added features
  }

  while ( dataProvider()->nextFeature( f ) )
  {
    if ( mFetchConsidered.contains( f.id() ) )
      continue;

    if ( mEditable )
      updateFeatureAttributes( f );

    // found it
    return true;
  }

  mFetching = false;
  return false;
}

bool QgsVectorLayer::featureAtId( int featureId, QgsFeature& f, bool fetchGeometries, bool fetchAttributes )
{
  if ( !mDataProvider )
    return false;

  if ( mDeletedFeatureIds.contains( featureId ) )
    return false;

  if ( fetchGeometries && mChangedGeometries.contains( featureId ) )
  {
    f.setFeatureId( featureId );
    f.setValid( true );
    f.setGeometry( mChangedGeometries[featureId] );

    if ( fetchAttributes )
    {
      if ( featureId < 0 )
      {
        // featureId<0 => in mAddedFeatures
        bool found = false;

        for ( QgsFeatureList::iterator it = mAddedFeatures.begin(); it != mAddedFeatures.end(); it++ )
        {
          if ( featureId != it->id() )
          {
            found = true;
            f.setAttributeMap( it->attributeMap() );
            break;
          }
        }

        if ( !found )
          QgsLogger::warning( QString( "No attributes for the added feature %1 found" ).arg( f.id() ) );
      }
      else
      {
        // retrieve attributes from provider
        QgsFeature tmp;
        mDataProvider->featureAtId( featureId, tmp, false, mDataProvider->attributeIndexes() );
        updateFeatureAttributes( tmp );
        f.setAttributeMap( tmp.attributeMap() );
      }
      updateFeatureAttributes( f );
    }
    return true;
  }

  //added features
  for ( QgsFeatureList::iterator iter = mAddedFeatures.begin(); iter != mAddedFeatures.end(); ++iter )
  {
    if ( iter->id() == featureId )
    {
      f.setFeatureId( iter->id() );
      f.setValid( true );
      if ( fetchGeometries )
        f.setGeometry( *iter->geometry() );

      if ( fetchAttributes )
        f.setAttributeMap( iter->attributeMap() );

      return true;
    }
  }

  // regular features
  if ( fetchAttributes )
  {
    if ( mDataProvider->featureAtId( featureId, f, fetchGeometries, mDataProvider->attributeIndexes() ) )
    {
      updateFeatureAttributes( f );
      return true;
    }
  }
  else
  {
    if ( mDataProvider->featureAtId( featureId, f, fetchGeometries, QgsAttributeList() ) )
    {
      return true;
    }
  }
  return false;
}

bool QgsVectorLayer::addFeature( QgsFeature& f, bool alsoUpdateExtent )
{
  static int addedIdLowWaterMark = -1;

  if ( !mDataProvider )
  {
    return false;
  }

  if ( !( mDataProvider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
  {
    return false;
  }

  if ( !isEditable() )
  {
    return false;
  }

  //assign a temporary id to the feature (use negative numbers)
  addedIdLowWaterMark--;

  QgsDebugMsg( "Assigned feature id " + QString::number( addedIdLowWaterMark ) );

  // Force a feature ID (to keep other functions in QGIS happy,
  // providers will use their own new feature ID when we commit the new feature)
  // and add to the known added features.
  f.setFeatureId( addedIdLowWaterMark );
  editFeatureAdd( f );
  mCachedGeometries[f.id()] = *f.geometry();

  setModified( true );

  if ( alsoUpdateExtent )
  {
    updateExtents();
  }

  return true;
}


bool QgsVectorLayer::insertVertex( double x, double y, int atFeatureId, int beforeVertex )
{
  if ( !mEditable )
  {
    return false;
  }

  if ( mDataProvider )
  {
    QgsGeometry geometry;
    if ( !mChangedGeometries.contains( atFeatureId ) )
    {
      // first time this geometry has changed since last commit
      if ( !mCachedGeometries.contains( atFeatureId ) )
      {
        return false;
      }
      geometry = mCachedGeometries[atFeatureId];
      //mChangedGeometries[atFeatureId] = mCachedGeometries[atFeatureId];
    }
    else
    {
      geometry = mChangedGeometries[atFeatureId];
    }
    geometry.insertVertex( x, y, beforeVertex );
    mCachedGeometries[atFeatureId] = geometry;
    editGeometryChange( atFeatureId, geometry );

    setModified( true, true ); // only geometry was changed

    return true;
  }
  return false;
}


bool QgsVectorLayer::moveVertex( double x, double y, int atFeatureId, int atVertex )
{
  if ( !mEditable )
  {
    return false;
  }

  if ( mDataProvider )
  {
    QgsGeometry geometry;
    if ( !mChangedGeometries.contains( atFeatureId ) )
    {
      // first time this geometry has changed since last commit
      if ( !mCachedGeometries.contains( atFeatureId ) )
      {
        return false;
      }
      geometry = mCachedGeometries[atFeatureId];
      //mChangedGeometries[atFeatureId] = mCachedGeometries[atFeatureId];
    }
    else
    {
      geometry = mChangedGeometries[atFeatureId];
    }

    geometry.moveVertex( x, y, atVertex );
    mCachedGeometries[atFeatureId] = geometry;
    editGeometryChange( atFeatureId, geometry );

    setModified( true, true ); // only geometry was changed

    return true;
  }
  return false;
}


bool QgsVectorLayer::deleteVertex( int atFeatureId, int atVertex )
{
  if ( !mEditable )
  {
    return false;
  }

  if ( mDataProvider )
  {
    QgsGeometry geometry;
    if ( !mChangedGeometries.contains( atFeatureId ) )
    {
      // first time this geometry has changed since last commit
      if ( !mCachedGeometries.contains( atFeatureId ) )
      {
        return false;
      }
      geometry = mCachedGeometries[atFeatureId];
    }
    else
    {
      geometry = mChangedGeometries[atFeatureId];
    }

    if ( !geometry.deleteVertex( atVertex ) )
    {
      return false;
    }
    mCachedGeometries[atFeatureId] = geometry;
    editGeometryChange( atFeatureId, geometry );

    setModified( true, true ); // only geometry was changed

    return true;
  }
  return false;
}


bool QgsVectorLayer::deleteSelectedFeatures()
{
  if ( !( mDataProvider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
  {
    return false;
  }

  if ( !isEditable() )
  {
    return false;
  }

  if ( mSelectedFeatureIds.size() == 0 )
    return true;

  while ( mSelectedFeatureIds.size() > 0 )
  {
    int fid = *mSelectedFeatureIds.begin();
    deleteFeature( fid );  // removes from selection
  }

  emit selectionChanged();

  triggerRepaint();
  updateExtents();

  return true;
}

int QgsVectorLayer::addRing( const QList<QgsPoint>& ring )
{
  int addRingReturnCode = 5; //default: return code for 'ring not inserted'
  double xMin, yMin, xMax, yMax;
  QgsRectangle bBox;

  if ( boundingBoxFromPointList( ring, xMin, yMin, xMax, yMax ) == 0 )
  {
    bBox.setXMinimum( xMin ); bBox.setYMinimum( yMin );
    bBox.setXMaximum( xMax ); bBox.setYMaximum( yMax );
  }
  else
  {
    return 3; //ring not valid
  }

  select( QgsAttributeList(), bBox, true, true );

  QgsFeature f;
  while ( nextFeature( f ) )
  {
    addRingReturnCode = f.geometry()->addRing( ring );
    if ( addRingReturnCode == 0 )
    {
      editGeometryChange( f.id(), *f.geometry() );

      setModified( true, true );
      break;
    }
  }

  return addRingReturnCode;
}

int QgsVectorLayer::addIsland( const QList<QgsPoint>& ring )
{
  //number of selected features must be 1

  if ( mSelectedFeatureIds.size() < 1 )
  {
    QgsDebugMsg( "Number of selected features <1" );
    return 4;
  }
  else if ( mSelectedFeatureIds.size() > 1 )
  {
    QgsDebugMsg( "Number of selected features >1" );
    return 5;
  }

  int selectedFeatureId = *mSelectedFeatureIds.constBegin();

  //look if geometry of selected feature already contains geometry changes
  QgsGeometryMap::iterator changedIt = mChangedGeometries.find( selectedFeatureId );
  if ( changedIt != mChangedGeometries.end() )
  {
    QgsGeometry geom = *changedIt;
    int returnValue = geom.addIsland( ring );
    editGeometryChange( selectedFeatureId, geom );
    mCachedGeometries[selectedFeatureId] = geom;
    return returnValue;
  }

  //look if id of selected feature belongs to an added feature
  /*
  for ( QgsFeatureList::iterator addedIt = mAddedFeatures.begin(); addedIt != mAddedFeatures.end(); ++addedIt )
  {
    if ( addedIt->id() == selectedFeatureId )
    {
      return addedIt->geometry()->addIsland( ring );
      mCachedGeometries[selectedFeatureId] = *addedIt->geometry();
    }
  }
  */

  //is the feature contained in the view extent (mCachedGeometries) ?
  QgsGeometryMap::iterator cachedIt = mCachedGeometries.find( selectedFeatureId );
  if ( cachedIt != mCachedGeometries.end() )
  {
    int errorCode = cachedIt->addIsland( ring );
    if ( errorCode == 0 )
    {
      editGeometryChange( selectedFeatureId, *cachedIt );
      mCachedGeometries[selectedFeatureId] = *cachedIt;
      setModified( true, true );
    }
    return errorCode;
  }
  else //maybe the selected feature has been moved outside the visible area and therefore is not contained in mCachedGeometries
  {
    QgsFeature f;
    QgsGeometry* fGeom = 0;
    if ( featureAtId( selectedFeatureId, f, true, false ) )
    {
      fGeom = f.geometryAndOwnership();
      if ( fGeom )
      {
        int errorCode = fGeom->addIsland( ring );
        editGeometryChange( selectedFeatureId, *fGeom );
        setModified( true, true );
        delete fGeom;
        return errorCode;
      }
    }
  }

  return 6; //geometry not found
}

int QgsVectorLayer::translateFeature( int featureId, double dx, double dy )
{
  //look if geometry of selected feature already contains geometry changes
  QgsGeometryMap::iterator changedIt = mChangedGeometries.find( featureId );
  if ( changedIt != mChangedGeometries.end() )
  {
    QgsGeometry geom = *changedIt;
    int errorCode = geom.translate( dx, dy );
    editGeometryChange( featureId, geom );
    return errorCode;
  }

  //look if id of selected feature belongs to an added feature
  /*
  for ( QgsFeatureList::iterator addedIt = mAddedFeatures.begin(); addedIt != mAddedFeatures.end(); ++addedIt )
  {
    if ( addedIt->id() == featureId )
    {
      return addedIt->geometry()->translate( dx, dy );
    }
  }
  */

  //else look in mCachedGeometries to make access faster
  QgsGeometryMap::iterator cachedIt = mCachedGeometries.find( featureId );
  if ( cachedIt != mCachedGeometries.end() )
  {
    int errorCode = cachedIt->translate( dx, dy );
    if ( errorCode == 0 )
    {
      editGeometryChange( featureId, *cachedIt );
      setModified( true, true );
    }
    return errorCode;
  }

  //else get the geometry from provider (may be slow)
  QgsFeature f;
  if ( mDataProvider && mDataProvider->featureAtId( featureId, f, true ) )
  {
    if ( f.geometry() )
    {
      QgsGeometry translateGeom( *( f.geometry() ) );
      int errorCode = translateGeom.translate( dx, dy );
      if ( errorCode == 0 )
      {
        editGeometryChange( featureId, translateGeom );
        setModified( true, true );
      }
      return errorCode;
    }
  }
  return 1; //geometry not found
}

int QgsVectorLayer::splitFeatures( const QList<QgsPoint>& splitLine, bool topologicalEditing )
{
  QgsFeatureList newFeatures; //store all the newly created features
  double xMin, yMin, xMax, yMax;
  QgsRectangle bBox; //bounding box of the split line
  int returnCode = 0;
  int splitFunctionReturn; //return code of QgsGeometry::splitGeometry
  int numberOfSplitedFeatures = 0;

  QgsFeatureList featureList;
  const QgsFeatureIds selectedIds = selectedFeaturesIds();

  if ( selectedIds.size() > 0 )//consider only the selected features if there is a selection
  {
    featureList = selectedFeatures();
  }
  else //else consider all the feature that intersect the bounding box of the split line
  {
    if ( boundingBoxFromPointList( splitLine, xMin, yMin, xMax, yMax ) == 0 )
    {
      bBox.setXMinimum( xMin ); bBox.setYMinimum( yMin );
      bBox.setXMaximum( xMax ); bBox.setYMaximum( yMax );
    }
    else
    {
      return 1;
    }

    if ( bBox.isEmpty() )
    {
      //if the bbox is a line, try to make a square out of it
      if ( bBox.width() == 0.0 && bBox.height() > 0 )
      {
        bBox.setXMinimum( bBox.xMinimum() - bBox.height() / 2 );
        bBox.setXMaximum( bBox.xMaximum() + bBox.height() / 2 );
      }
      else if ( bBox.height() == 0.0 && bBox.width() > 0 )
      {
        bBox.setYMinimum( bBox.yMinimum() - bBox.width() / 2 );
        bBox.setYMaximum( bBox.yMaximum() + bBox.width() / 2 );
      }
      else
      {
        return 2;
      }
    }

    select( pendingAllAttributesList(), bBox, true, true );

    QgsFeature f;
    while ( nextFeature( f ) )
      featureList << QgsFeature( f );
  }

  QgsFeatureList::iterator select_it = featureList.begin();
  for ( ; select_it != featureList.end(); ++select_it )
  {
    QList<QgsGeometry*> newGeometries;
    QList<QgsPoint> topologyTestPoints;
    QgsGeometry* newGeometry = 0;
    splitFunctionReturn = select_it->geometry()->splitGeometry( splitLine, newGeometries, topologicalEditing, topologyTestPoints );
    if ( splitFunctionReturn == 0 )
    {
      //change this geometry
      editGeometryChange( select_it->id(), *( select_it->geometry() ) );
      //update of cached geometries is necessary because we use addTopologicalPoints() later
      mCachedGeometries[select_it->id()] = *( select_it->geometry() );

      //insert new features
      for ( int i = 0; i < newGeometries.size(); ++i )
      {
        newGeometry = newGeometries.at( i );
        QgsFeature newFeature;
        newFeature.setGeometry( newGeometry );
        newFeature.setAttributeMap( select_it->attributeMap() );
        newFeatures.append( newFeature );
      }

      setModified( true, true );
      if ( topologicalEditing )
      {
        QList<QgsPoint>::const_iterator topol_it = topologyTestPoints.constBegin();
        for ( ; topol_it != topologyTestPoints.constEnd(); ++topol_it )
        {
          addTopologicalPoints( *topol_it );
        }
      }
      ++numberOfSplitedFeatures;
    }
    else if ( splitFunctionReturn > 1 ) //1 means no split but also no error
    {
      returnCode = 3;
    }
  }

  if ( numberOfSplitedFeatures == 0 && selectedIds.size() > 0 )
  {
    //There is a selection but no feature has been split.
    //Maybe user forgot that only the selected features are split
    returnCode = 4;
  }


  //now add the new features to this vectorlayer
  addFeatures( newFeatures, false );

  return returnCode;
}

int QgsVectorLayer::removePolygonIntersections( QgsGeometry* geom )
{
  int returnValue = 0;

  //first test if geom really has type polygon or multipolygon
  if ( geom->type() != QGis::Polygon )
  {
    return 1;
  }

  //get bounding box of geom
  QgsRectangle geomBBox = geom->boundingBox();

  //get list of features that intersect this bounding box
  select( QgsAttributeList(), geomBBox, true, true );

  QgsFeature f;
  while ( nextFeature( f ) )
  {
    //call geometry->makeDifference for each feature
    QgsGeometry *currentGeom = f.geometry();
    if ( currentGeom )
    {
      if ( geom->makeDifference( currentGeom ) != 0 )
      {
        returnValue = 2;
      }
    }
  }
  return returnValue;
}

int QgsVectorLayer::addTopologicalPoints( QgsGeometry* geom )
{
  if ( !geom )
  {
    return 1;
  }

  int returnVal = 0;

  QGis::WkbType wkbType = geom->wkbType();

  switch ( wkbType )
  {
      //line
    case QGis::WKBLineString25D:
    case QGis::WKBLineString:
    {
      QgsPolyline theLine = geom->asPolyline();
      QgsPolyline::const_iterator line_it = theLine.constBegin();
      for ( ; line_it != theLine.constEnd(); ++line_it )
      {
        if ( addTopologicalPoints( *line_it ) != 0 )
        {
          returnVal = 2;
        }
      }
      break;
    }

    //multiline
    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      QgsMultiPolyline theMultiLine = geom->asMultiPolyline();
      QgsPolyline currentPolyline;

      for ( int i = 0; i < theMultiLine.size(); ++i )
      {
        QgsPolyline::const_iterator line_it = currentPolyline.constBegin();
        for ( ; line_it != currentPolyline.constEnd(); ++line_it )
        {
          if ( addTopologicalPoints( *line_it ) != 0 )
          {
            returnVal = 2;
          }
        }
      }
      break;
    }

    //polygon
    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon:
    {
      QgsPolygon thePolygon = geom->asPolygon();
      QgsPolyline currentRing;

      for ( int i = 0; i < thePolygon.size(); ++i )
      {
        currentRing = thePolygon.at( i );
        QgsPolyline::const_iterator line_it = currentRing.constBegin();
        for ( ; line_it != currentRing.constEnd(); ++line_it )
        {
          if ( addTopologicalPoints( *line_it ) != 0 )
          {
            returnVal = 2;
          }
        }
      }
      break;
    }

    //multipolygon
    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      QgsMultiPolygon theMultiPolygon = geom->asMultiPolygon();
      QgsPolygon currentPolygon;
      QgsPolyline currentRing;

      for ( int i = 0; i < theMultiPolygon.size(); ++i )
      {
        currentPolygon = theMultiPolygon.at( i );
        for ( int j = 0; j < currentPolygon.size(); ++j )
        {
          currentRing = currentPolygon.at( j );
          QgsPolyline::const_iterator line_it = currentRing.constBegin();
          for ( ; line_it != currentRing.constEnd(); ++line_it )
          {
            if ( addTopologicalPoints( *line_it ) != 0 )
            {
              returnVal = 2;
            }
          }
        }
      }
      break;
    }
    default:
      break;
  }
  return returnVal;
}

int QgsVectorLayer::addTopologicalPoints( const QgsPoint& p )
{
  QMultiMap<double, QgsSnappingResult> snapResults; //results from the snapper object
  //we also need to snap to vertex to make sure the vertex does not already exist in this geometry
  QMultiMap<double, QgsSnappingResult> vertexSnapResults;

  QList<QgsSnappingResult> filteredSnapResults; //we filter out the results that are on existing vertices

  //work with a tolerance because coordinate projection may introduce some rounding
  double threshold =  0.0000001;
  if ( mCRS && mCRS->mapUnits() == QGis::Meters )
  {
    threshold = 0.001;
  }
  else if ( mCRS && mCRS->mapUnits() == QGis::Feet )
  {
    threshold = 0.0001;
  }


  if ( snapWithContext( p, threshold, snapResults, QgsSnapper::SnapToSegment ) != 0 )
  {
    return 2;
  }

  QMultiMap<double, QgsSnappingResult>::const_iterator snap_it = snapResults.constBegin();
  QMultiMap<double, QgsSnappingResult>::const_iterator vertex_snap_it;
  for ( ; snap_it != snapResults.constEnd(); ++snap_it )
  {
    //test if p is already a vertex of this geometry. If yes, don't insert it
    bool vertexAlreadyExists = false;
    if ( snapWithContext( p, threshold, vertexSnapResults, QgsSnapper::SnapToVertex ) != 0 )
    {
      continue;
    }

    vertex_snap_it = vertexSnapResults.constBegin();
    for ( ; vertex_snap_it != vertexSnapResults.constEnd(); ++vertex_snap_it )
    {
      if ( snap_it.value().snappedAtGeometry == vertex_snap_it.value().snappedAtGeometry )
      {
        vertexAlreadyExists = true;
      }
    }

    if ( !vertexAlreadyExists )
    {
      filteredSnapResults.push_back( *snap_it );
    }
  }
  insertSegmentVerticesForSnap( filteredSnapResults );
  return 0;
}

QgsLabel * QgsVectorLayer::label()
{
  return mLabel;
}

const QgsLabel *QgsVectorLayer::label() const
{
  return mLabel;
}

void QgsVectorLayer::enableLabels( bool on )
{
  mLabelOn = on;
}

bool QgsVectorLayer::hasLabelsEnabled( void ) const
{
  return mLabelOn;
}

bool QgsVectorLayer::startEditing()
{
  if ( !mDataProvider )
  {
    return false;
  }

  // allow editing if provider supports any of the capabilities
  if ( !( mDataProvider->capabilities() & QgsVectorDataProvider::EditingCapabilities ) )
  {
    return false;
  }

  if ( mEditable )
  {
    // editing already underway
    return false;
  }

  mEditable = true;

  mUpdatedFields = mDataProvider->fields();

  mMaxUpdatedIndex = -1;

  for ( QgsFieldMap::const_iterator it = mUpdatedFields.begin(); it != mUpdatedFields.end(); it++ )
    if ( it.key() > mMaxUpdatedIndex )
      mMaxUpdatedIndex = it.key();

  emit editingStarted();

  return true;
}

bool QgsVectorLayer::readXml( QDomNode & layer_node )
{
  QgsDebugMsg( QString( "Datasource in QgsVectorLayer::readXml: " ) + mDataSource.toLocal8Bit().data() );

  //process provider key
  QDomNode pkeyNode = layer_node.namedItem( "provider" );

  if ( pkeyNode.isNull() )
  {
    mProviderKey = "";
  }
  else
  {
    QDomElement pkeyElt = pkeyNode.toElement();
    mProviderKey = pkeyElt.text();
  }

  // determine type of vector layer
  if ( ! mProviderKey.isNull() )
  {
    // if the provider string isn't empty, then we successfully
    // got the stored provider
  }
  else if ( mDataSource.contains( "dbname=" ) )
  {
    mProviderKey = "postgres";
  }
  else
  {
    mProviderKey = "ogr";
  }

  if ( ! setDataProvider( mProviderKey ) )
  {
    return false;
  }

  QDomElement pkeyElem = pkeyNode.toElement();
  if ( !pkeyElem.isNull() )
  {
    QString encodingString = pkeyElem.attribute( "encoding" );
    if ( !encodingString.isEmpty() )
    {
      mDataProvider->setEncoding( encodingString );
    }
  }

  QString errorMsg;
  if ( !readSymbology( layer_node, errorMsg ) )
  {
    return false;
  }

  return mValid;               // should be true if read successfully

} // void QgsVectorLayer::readXml



bool QgsVectorLayer::setDataProvider( QString const & provider )
{
  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  mProviderKey = provider;     // XXX is this necessary?  Usually already set
  // XXX when execution gets here.

  //XXX - This was a dynamic cast but that kills the Windows
  //      version big-time with an abnormal termination error
  mDataProvider =
    ( QgsVectorDataProvider* )( QgsProviderRegistry::instance()->getProvider( provider, mDataSource ) );

  if ( mDataProvider )
  {
    QgsDebugMsg( "Instantiated the data provider plugin" );

    mValid = mDataProvider->isValid();
    if ( mValid )
    {

      // TODO: Check if the provider has the capability to send fullExtentCalculated
      connect( mDataProvider, SIGNAL( fullExtentCalculated() ), this, SLOT( updateExtents() ) );

      // get the extent
      QgsRectangle mbr = mDataProvider->extent();

      // show the extent
      QString s = mbr.toString();
      QgsDebugMsg( "Extent of layer: " +  s );
      // store the extent
      mLayerExtent.setXMaximum( mbr.xMaximum() );
      mLayerExtent.setXMinimum( mbr.xMinimum() );
      mLayerExtent.setYMaximum( mbr.yMaximum() );
      mLayerExtent.setYMinimum( mbr.yMinimum() );

      // get and store the feature type
      mWkbType = mDataProvider->geometryType();

      // look at the fields in the layer and set the primary
      // display field using some real fuzzy logic
      setDisplayField();

      if ( mProviderKey == "postgres" )
      {
        QgsDebugMsg( "Beautifying layer name " + name() );

        // adjust the display name for postgres layers
        QRegExp reg( "\"[^\"]+\"\\.\"([^\"]+)\" \\(([^)]+)\\)" );
        if ( reg.indexIn( name() ) >= 0 )
        {
          QStringList stuff = reg.capturedTexts();
          QString lName = stuff[1];

          const QMap<QString, QgsMapLayer*> &layers = QgsMapLayerRegistry::instance()->mapLayers();

          QMap<QString, QgsMapLayer*>::const_iterator it;
          for ( it = layers.constBegin(); it != layers.constEnd() && ( *it )->name() != lName; it++ )
            ;

          if ( it != layers.constEnd() )
            lName += "." + stuff[2];

          if ( !lName.isEmpty() )
            setLayerName( lName );
        }

        QgsDebugMsg( "Beautifying layer name " + name() );

        // deal with unnecessary schema qualification to make v.in.ogr happy
        mDataSource = mDataProvider->dataSourceUri();
      }
      else if ( mProviderKey == "osm" )
      {
        // make sure that the "observer" has been removed from URI to avoid crashes
        mDataSource = mDataProvider->dataSourceUri();
      }

      // label
      mLabel = new QgsLabel( mDataProvider->fields() );
      mLabelOn = false;
    }
    else
    {
      QgsDebugMsg( "Invalid provider plugin " + QString( mDataSource.toUtf8() ) );
      return false;
    }
  }
  else
  {
    QgsDebugMsg( " unable to get data provider" );

    return false;
  }

  return true;

} // QgsVectorLayer:: setDataProvider




/* virtual */
bool QgsVectorLayer::writeXml( QDomNode & layer_node,
                               QDomDocument & document )
{
  // first get the layer element so that we can append the type attribute

  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || ( "maplayer" != mapLayerNode.nodeName() ) )
  {
    QgsDebugMsg( "can't find <maplayer>" );
    return false;
  }

  mapLayerNode.setAttribute( "type", "vector" );

  // set the geometry type
  mapLayerNode.setAttribute( "geometry", QGis::qgisVectorGeometryType[geometryType()] );

  // add provider node
  if ( mDataProvider )
  {
    QDomElement provider  = document.createElement( "provider" );
    provider.setAttribute( "encoding", mDataProvider->encoding() );
    QDomText providerText = document.createTextNode( providerType() );
    provider.appendChild( providerText );
    layer_node.appendChild( provider );
  }

  // renderer specific settings
  QString errorMsg;
  if ( !writeSymbology( layer_node, document, errorMsg ) )
  {
    return false;
  }

  return true;
} // bool QgsVectorLayer::writeXml

bool QgsVectorLayer::readSymbology( const QDomNode& node, QString& errorMessage )
{
  // try renderer v2 first
  QDomElement rendererElement = node.firstChildElement( RENDERER_TAG_NAME );
  if ( !rendererElement.isNull() )
  {
    // using renderer v2
    setUsingRendererV2( true );

    QgsFeatureRendererV2* r = QgsFeatureRendererV2::load( rendererElement );
    if ( r == NULL )
      return false;

    setRendererV2( r );
  }
  else
  {
    // using renderer v1
    setUsingRendererV2( false );

    // create and bind a renderer to this layer

    QDomNode singlenode = node.namedItem( "singlesymbol" );
    QDomNode graduatednode = node.namedItem( "graduatedsymbol" );
    QDomNode continuousnode = node.namedItem( "continuoussymbol" );
    QDomNode uniquevaluenode = node.namedItem( "uniquevalue" );

    QgsRenderer * renderer = 0;
    int returnCode = 1;

    if ( !singlenode.isNull() )
    {
      renderer = new QgsSingleSymbolRenderer( geometryType() );
      returnCode = renderer->readXML( singlenode, *this );
    }
    else if ( !graduatednode.isNull() )
    {
      renderer = new QgsGraduatedSymbolRenderer( geometryType() );
      returnCode = renderer->readXML( graduatednode, *this );
    }
    else if ( !continuousnode.isNull() )
    {
      renderer = new QgsContinuousColorRenderer( geometryType() );
      returnCode = renderer->readXML( continuousnode, *this );
    }
    else if ( !uniquevaluenode.isNull() )
    {
      renderer = new QgsUniqueValueRenderer( geometryType() );
      returnCode = renderer->readXML( uniquevaluenode, *this );
    }

    if ( !renderer )
    {
      errorMessage = tr( "Unknown renderer" );
      return false;
    }

    if ( returnCode == 1 )
    {
      errorMessage = tr( "No renderer object" ); delete renderer; return false;
    }
    else if ( returnCode == 2 )
    {
      errorMessage = tr( "Classification field not found" ); delete renderer; return false;
    }

    mRenderer = renderer;

  }

  // process the attribute actions
  mActions->readXML( node );

  // get and set the display field if it exists.
  QDomNode displayFieldNode = node.namedItem( "displayfield" );
  if ( !displayFieldNode.isNull() )
  {
    QDomElement e = displayFieldNode.toElement();
    setDisplayField( e.text() );
  }

  // use scale dependent visibility flag
  QDomElement e = node.toElement();
  label()->setScaleBasedVisibility( e.attribute( "scaleBasedLabelVisibilityFlag", "0" ) == "1" );
  label()->setMinScale( e.attribute( "minLabelScale", "1" ).toFloat() );
  label()->setMaxScale( e.attribute( "maxLabelScale", "100000000" ).toFloat() );

  mEditTypes.clear();
  QDomNode editTypesNode = node.namedItem( "edittypes" );
  if ( !editTypesNode.isNull() )
  {
    QDomNodeList editTypeNodes = editTypesNode.childNodes();

    for ( int i = 0; i < editTypeNodes.size(); i++ )
    {
      QDomNode editTypeNode = editTypeNodes.at( i );
      QDomElement editTypeElement = editTypeNode.toElement();

      QString name = editTypeElement.attribute( "name" );

      EditType editType = ( EditType ) editTypeElement.attribute( "type" ).toInt();
      mEditTypes.insert( name, editType );

      if ( editType == ValueMap && editTypeNode.hasChildNodes() )
      {
        mValueMaps.insert( name, QMap<QString, QVariant>() );

        QDomNodeList valueMapNodes = editTypeNode.childNodes();
        for ( int j = 0; j < valueMapNodes.size(); j++ )
        {
          QDomElement value = valueMapNodes.at( j ).toElement();
          mValueMaps[ name ].insert( value.attribute( "key" ), value.attribute( "value" ) );
        }
      }
      else if ( editType == EditRange || editType == SliderRange )
      {
        QVariant min = editTypeElement.attribute( "min" );
        QVariant max = editTypeElement.attribute( "max" );
        QVariant step = editTypeElement.attribute( "step" );

        mRanges[ name ] = RangeData( min, max, step );
      }
      else if ( editType == CheckBox )
      {
        mCheckedStates[ name ] = QPair<QString, QString>( editTypeElement.attribute( "checked" ), editTypeElement.attribute( "unchecked" ) );
      }
    }
  }

  QDomNode editFormNode = node.namedItem( "editform" );
  if ( !editFormNode.isNull() )
  {
    QDomElement e = editFormNode.toElement();
    mEditForm = QgsProject::instance()->readPath( e.text() );
  }

  mAttributeAliasMap.clear();
  QDomNode aliasesNode = node.namedItem( "aliases" );
  if ( !aliasesNode.isNull() )
  {
    QDomElement aliasElem;
    int index;
    QString name;

    QDomNodeList aliasNodeList = aliasesNode.toElement().elementsByTagName( "alias" );
    for ( int i = 0; i < aliasNodeList.size(); ++i )
    {
      aliasElem = aliasNodeList.at( i ).toElement();
      index = aliasElem.attribute( "index" ).toInt();
      name = aliasElem.attribute( "name" );
      mAttributeAliasMap.insert( index, name );
    }
  }

  // Test if labeling is on or off
  QDomNode labelnode = node.namedItem( "label" );
  QDomElement element = labelnode.toElement();
  int hasLabelsEnabled = element.text().toInt();
  if ( hasLabelsEnabled < 1 )
  {
    enableLabels( false );
  }
  else
  {
    enableLabels( true );
  }

  QDomNode labelattributesnode = node.namedItem( "labelattributes" );

  if ( !labelattributesnode.isNull() )
  {
    QgsDebugMsg( "qgsvectorlayer calling label readXML routine" );
    mLabel->readXML( labelattributesnode );
  }

  return true;
}

bool QgsVectorLayer::writeSymbology( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const
{
  if ( mUsingRendererV2 )
  {
    QDomElement rendererElement = mRendererV2->save( doc );
    node.appendChild( rendererElement );
  }
  else
  {
    //classification field(s)
    QgsAttributeList attributes = mRenderer->classificationAttributes();
    const QgsFieldMap providerFields = mDataProvider->fields();
    for ( QgsAttributeList::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
    {
      QDomElement classificationElement = doc.createElement( "classificationattribute" );
      QDomText classificationText = doc.createTextNode( providerFields[*it].name() );
      classificationElement.appendChild( classificationText );
      node.appendChild( classificationElement );
    }

    // renderer settings
    const QgsRenderer * myRenderer = renderer();
    if ( myRenderer )
    {
      if ( !myRenderer->writeXML( node, doc, *this ) )
      {
        errorMessage = "renderer failed to save";
        return false;
      }
    }
    else
    {
      QgsDebugMsg( "no renderer" );
      errorMessage = "no renderer";
      return false;
    }
  }

  // use scale dependent visibility flag
  QDomElement mapLayerNode = node.toElement();
  mapLayerNode.setAttribute( "scaleBasedLabelVisibilityFlag", label()->scaleBasedVisibility() ? 1 : 0 );
  mapLayerNode.setAttribute( "minLabelScale", label()->minScale() );
  mapLayerNode.setAttribute( "maxLabelScale", label()->maxScale() );

  //edit types
  if ( mEditTypes.size() > 0 )
  {
    QDomElement editTypesElement = doc.createElement( "edittypes" );

    for ( QMap<QString, EditType>::const_iterator it = mEditTypes.begin(); it != mEditTypes.end(); ++it )
    {
      QDomElement editTypeElement = doc.createElement( "edittype" );
      editTypeElement.setAttribute( "name", it.key() );
      editTypeElement.setAttribute( "type", it.value() );

      if ( it.value() == ValueMap )
      {
        if ( mValueMaps.contains( it.key() ) )
        {
          const QMap<QString, QVariant> &map = mValueMaps[ it.key()];

          for ( QMap<QString, QVariant>::const_iterator vmit = map.begin(); vmit != map.end(); vmit++ )
          {
            QDomElement value = doc.createElement( "valuepair" );
            value.setAttribute( "key", vmit.key() );
            value.setAttribute( "value", vmit.value().toString() );
            editTypeElement.appendChild( value );
          }
        }
      }
      else if ( it.value() == EditRange || it.value() == SliderRange )
      {
        if ( mRanges.contains( it.key() ) )
        {
          editTypeElement.setAttribute( "min", mRanges[ it.key()].mMin.toString() );
          editTypeElement.setAttribute( "max", mRanges[ it.key()].mMax.toString() );
          editTypeElement.setAttribute( "step", mRanges[ it.key()].mStep.toString() );
        }
      }
      else if ( it.value() == CheckBox )
      {
        if ( mCheckedStates.contains( it.key() ) )
        {
          editTypeElement.setAttribute( "checked", mCheckedStates[ it.key()].first );
          editTypeElement.setAttribute( "unchecked", mCheckedStates[ it.key()].second );
        }
      }

      editTypesElement.appendChild( editTypeElement );
    }

    node.appendChild( editTypesElement );
  }

  QDomElement efField  = doc.createElement( "editform" );
  QDomText efText = doc.createTextNode( QgsProject::instance()->writePath( mEditForm ) );
  efField.appendChild( efText );
  node.appendChild( efField );

  //attribute aliases
  if ( mAttributeAliasMap.size() > 0 )
  {
    QDomElement aliasElem = doc.createElement( "aliases" );
    QMap<int, QString>::const_iterator a_it = mAttributeAliasMap.constBegin();
    for ( ; a_it != mAttributeAliasMap.constEnd(); ++a_it )
    {
      QDomElement aliasEntryElem = doc.createElement( "alias" );
      aliasEntryElem.setAttribute( "index", QString::number( a_it.key() ) );
      aliasEntryElem.setAttribute( "name", a_it.value() );
      aliasElem.appendChild( aliasEntryElem );
    }
    node.appendChild( aliasElem );
  }

  // add the display field
  QDomElement dField  = doc.createElement( "displayfield" );
  QDomText dFieldText = doc.createTextNode( displayField() );
  dField.appendChild( dFieldText );
  node.appendChild( dField );

  // add label node
  QDomElement label  = doc.createElement( "label" );
  QDomText labelText = doc.createTextNode( "" );

  if ( hasLabelsEnabled() )
  {
    labelText.setData( "1" );
  }
  else
  {
    labelText.setData( "0" );
  }
  label.appendChild( labelText );

  node.appendChild( label );

  // add attribute actions
  mActions->writeXML( node, doc );

  // Now we get to do all that all over again for QgsLabel

  // XXX Since this is largely a cut-n-paste from the previous, this
  // XXX therefore becomes a candidate to be generalized into a separate
  // XXX function.  I think.

  const QgsLabel *myLabel = this->label();

  if ( myLabel )
  {
    QString fieldname = myLabel->labelField( QgsLabel::Text );
    if ( fieldname != "" )
    {
      dField  = doc.createElement( "labelfield" );
      dFieldText = doc.createTextNode( fieldname );
      dField.appendChild( dFieldText );
      node.appendChild( dField );
    }

    myLabel->writeXML( node, doc );
  }

  //save vector overlays (e.g. diagrams)
  QList<QgsVectorOverlay*>::const_iterator overlay_it = mOverlays.constBegin();
  for ( ; overlay_it != mOverlays.constEnd(); ++overlay_it )
  {
    if ( *overlay_it )
    {
      ( *overlay_it )->writeXML( mapLayerNode, doc );
    }
  }

  return true;
}


bool QgsVectorLayer::changeGeometry( int fid, QgsGeometry* geom )
{
  if ( !mEditable || !mDataProvider )
  {
    return false;
  }

  editGeometryChange( fid, *geom );
  mCachedGeometries[fid] = *geom;
  setModified( true, true );
  return true;
}


bool QgsVectorLayer::changeAttributeValue( int fid, int field, QVariant value, bool emitSignal )
{
  if ( !isEditable() )
    return false;

  editAttributeChange( fid, field, value );
  setModified( true, false );

  if ( emitSignal )
    emit attributeValueChanged( fid, field, value );

  return true;
}

bool QgsVectorLayer::addAttribute( const QgsField &field )
{
  if ( !isEditable() )
    return false;

  for ( QgsFieldMap::const_iterator it = mUpdatedFields.begin(); it != mUpdatedFields.end(); it++ )
  {
    if ( it.value().name() == field.name() )
      return false;
  }

  if ( !mDataProvider->supportedType( field ) )
    return false;

  mMaxUpdatedIndex++;

  if ( mActiveCommand != NULL )
  {
    mActiveCommand->storeAttributeAdd( mMaxUpdatedIndex, field );
  }

  mUpdatedFields.insert( mMaxUpdatedIndex, field );
  mAddedAttributeIds.insert( mMaxUpdatedIndex );

  setModified( true, false );

  emit attributeAdded( mMaxUpdatedIndex );

  return true;
}

bool QgsVectorLayer::addAttribute( QString name, QString type )
{
  const QMap<QString, QVariant::Type> &map = mDataProvider->supportedNativeTypes();

  if ( !map.contains( type ) )
    return false;

  return addAttribute( QgsField( name, map[ type ], type ) );
}

void QgsVectorLayer::addAttributeAlias( int attIndex, QString aliasString )
{
  mAttributeAliasMap.insert( attIndex, aliasString );
  emit layerModified( false );
}

QString QgsVectorLayer::attributeAlias( int attributeIndex ) const
{
  QMap<int, QString>::const_iterator alias_it = mAttributeAliasMap.find( attributeIndex );
  if ( alias_it != mAttributeAliasMap.constEnd() )
  {
    return alias_it.value();
  }
  else
  {
    return QString();
  }
}

QString QgsVectorLayer::attributeDisplayName( int attributeIndex ) const
{
  QString displayName = attributeAlias( attributeIndex );
  if ( displayName.isEmpty() )
  {
    const QgsFieldMap& fields = pendingFields();
    QgsFieldMap::const_iterator fieldIt = fields.find( attributeIndex );
    if ( fieldIt != fields.constEnd() )
    {
      displayName = fieldIt->name();
    }
  }
  return displayName;
}

bool QgsVectorLayer::deleteAttribute( int index )
{
  if ( !isEditable() )
    return false;

  if ( mDeletedAttributeIds.contains( index ) )
    return false;

  if ( !mAddedAttributeIds.contains( index ) &&
       !mDataProvider->fields().contains( index ) )
    return false;

  if ( mActiveCommand != NULL )
  {
    mActiveCommand->storeAttributeDelete( index, mUpdatedFields[index] );
  }

  mDeletedAttributeIds.insert( index );
  mAddedAttributeIds.remove( index );
  mUpdatedFields.remove( index );
  mAttributeAliasMap.remove( index );

  setModified( true, false );

  emit attributeDeleted( index );

  return true;
}

bool QgsVectorLayer::deleteFeature( int fid )
{
  if ( !isEditable() )
    return false;

  if ( mDeletedFeatureIds.contains( fid ) )
    return true;

  mSelectedFeatureIds.remove( fid ); // remove it from selection
  editFeatureDelete( fid );

  setModified( true, false );

  emit featureDeleted( fid );

  return true;
}

const QgsFieldMap &QgsVectorLayer::pendingFields() const
{
  return isEditable() ? mUpdatedFields : mDataProvider->fields();
}

QgsAttributeList QgsVectorLayer::pendingAllAttributesList()
{
  return isEditable() ? mUpdatedFields.keys() : mDataProvider->attributeIndexes();
}

int QgsVectorLayer::pendingFeatureCount()
{
  return mDataProvider->featureCount()
         + mAddedFeatures.size()
         - mDeletedFeatureIds.size();
}

bool QgsVectorLayer::commitChanges()
{
  bool success = true;

  //clear the cache image so markers dont appear anymore on next draw
  setCacheImage( 0 );

  mCommitErrors.clear();

  if ( !mDataProvider )
  {
    mCommitErrors << tr( "ERROR: no provider" );
    return false;
  }

  if ( !isEditable() )
  {
    mCommitErrors << tr( "ERROR: layer not editable" );
    return false;
  }

  int cap = mDataProvider->capabilities();

  //
  // delete attributes
  //
  bool attributesChanged = false;
  if ( mDeletedAttributeIds.size() > 0 )
  {
    if (( cap & QgsVectorDataProvider::DeleteAttributes ) && mDataProvider->deleteAttributes( mDeletedAttributeIds ) )
    {
      mCommitErrors << tr( "SUCCESS: %n attribute(s) deleted.", "deleted attributes count", mDeletedAttributeIds.size() );
      mDeletedAttributeIds.clear();
      attributesChanged = true;
    }
    else
    {
      mCommitErrors << tr( "ERROR: %n attribute(s) not deleted.", "not deleted attributes count", mDeletedAttributeIds.size() );
      success = false;
    }
  }

  //
  // add attributes
  //
  if ( mAddedAttributeIds.size() > 0 )
  {
    QList<QgsField> addedAttributes;
    for ( QgsAttributeIds::const_iterator it = mAddedAttributeIds.begin(); it != mAddedAttributeIds.end(); it++ )
      addedAttributes << mUpdatedFields[ *it ];

    if (( cap & QgsVectorDataProvider::AddAttributes ) && mDataProvider->addAttributes( addedAttributes ) )
    {
      mCommitErrors << tr( "SUCCESS: %n attribute(s) added.", "added attributes count", mAddedAttributeIds.size() );
      mAddedAttributeIds.clear();
      attributesChanged = true;
    }
    else
    {
      mCommitErrors << tr( "ERROR: %n new attribute(s) not added", "not added attributes count", mAddedAttributeIds.size() );
      success = false;
    }
  }

  //
  // remap changed and attributes of added features
  //
  bool attributeChangesOk = true;
  if ( attributesChanged )
  {
    // map updates field indexes to names
    QMap<int, QString> src;
    for ( QgsFieldMap::const_iterator it = mUpdatedFields.begin(); it != mUpdatedFields.end(); it++ )
    {
      src[ it.key()] = it.value().name();
    }

    int maxAttrIdx = -1;
    const QgsFieldMap &pFields = mDataProvider->fields();

    // map provider table names to field indexes
    QMap<QString, int> dst;
    for ( QgsFieldMap::const_iterator it = pFields.begin(); it != pFields.end(); it++ )
    {
      dst[ it.value().name()] = it.key();
      if ( it.key() > maxAttrIdx )
        maxAttrIdx = it.key();
    }

    // if adding attributes failed add fields that are now missing
    // (otherwise we'll loose updates when doing the remapping)
    if ( mAddedAttributeIds.size() > 0 )
    {
      for ( QgsAttributeIds::const_iterator it = mAddedAttributeIds.begin(); it != mAddedAttributeIds.end(); it++ )
      {
        QString name =  mUpdatedFields[ *it ].name();
        if ( dst.contains( name ) )
        {
          // it's there => so we don't need to add it anymore
          mAddedAttributeIds.remove( *it );
          mCommitErrors << tr( "SUCCESS: attribute %1 was added." ).arg( name );
        }
        else
        {
          // field not there => put it behind the existing attributes
          dst[ name ] = ++maxAttrIdx;
          attributeChangesOk = false;   // don't try attribute updates - they'll fail.
          mCommitErrors << tr( "ERROR: attribute %1 not added" ).arg( name );
        }
      }
    }

    // map updated fields to provider fields
    QMap<int, int> remap;
    for ( QMap<int, QString>::const_iterator it = src.begin(); it != src.end(); it++ )
    {
      if ( dst.contains( it.value() ) )
      {
        remap[ it.key()] = dst[ it.value()];
      }
    }

    // remap changed attributes
    for ( QgsChangedAttributesMap::iterator fit = mChangedAttributeValues.begin(); fit != mChangedAttributeValues.end(); fit++ )
    {
      QgsAttributeMap &src = fit.value();
      QgsAttributeMap dst;

      for ( QgsAttributeMap::const_iterator it = src.begin(); it != src.end(); it++ )
      {
        if ( remap.contains( it.key() ) )
        {
          dst[ remap[it.key()] ] = it.value();
        }
      }
      src = dst;
    }

    // remap features of added attributes
    for ( QgsFeatureList::iterator fit = mAddedFeatures.begin(); fit != mAddedFeatures.end(); fit++ )
    {
      const QgsAttributeMap &src = fit->attributeMap();
      QgsAttributeMap dst;

      for ( QgsAttributeMap::const_iterator it = src.begin(); it != src.end(); it++ )
        if ( remap.contains( it.key() ) )
          dst[ remap[it.key()] ] = it.value();

      fit->setAttributeMap( dst );
    }

    QgsFieldMap attributes;

    // update private field map
    for ( QMap<int, int>::iterator it = remap.begin(); it != remap.end(); it++ )
      attributes[ it.value()] = mUpdatedFields[ it.key()];

    mUpdatedFields = attributes;
  }

  if ( attributeChangesOk )
  {
    //
    // change attributes
    //
    if ( mChangedAttributeValues.size() > 0 )
    {
      if (( cap & QgsVectorDataProvider::ChangeAttributeValues ) && mDataProvider->changeAttributeValues( mChangedAttributeValues ) )
      {
        mCommitErrors << tr( "SUCCESS: %n attribute value(s) changed.", "changed attribute values count", mChangedAttributeValues.size() );
        mChangedAttributeValues.clear();
      }
      else
      {
        mCommitErrors << tr( "ERROR: %n attribute value change(s) not applied.", "not changed attribute values count", mChangedAttributeValues.size() );
        success = false;
      }
    }

    //
    //  add features
    //
    if ( mAddedFeatures.size() > 0 )
    {
      for ( QgsFeatureList::iterator iter = mAddedFeatures.begin(); iter != mAddedFeatures.end(); ++iter )
      {
        if ( mDeletedFeatureIds.contains( iter->id() ) )
        {
          mDeletedFeatureIds.remove( iter->id() );

          if ( mChangedGeometries.contains( iter->id() ) )
            mChangedGeometries.remove( iter->id() );

          mAddedFeatures.erase( iter );

          continue;
        }

        if ( mChangedGeometries.contains( iter->id() ) )
        {
          iter->setGeometry( mChangedGeometries.take( iter->id() ) );
        }
      }

      if (( cap & QgsVectorDataProvider::AddFeatures ) && mDataProvider->addFeatures( mAddedFeatures ) )
      {
        mCommitErrors << tr( "SUCCESS: %n feature(s) added.", "added features count", mAddedFeatures.size() );
        mAddedFeatures.clear();
      }
      else
      {
        mCommitErrors << tr( "ERROR: %n feature(s) not added.", "not added features count", mAddedFeatures.size() );
        success = false;
      }
    }
  }

  //
  // update geometries
  //
  if ( mChangedGeometries.size() > 0 )
  {
    if (( cap & QgsVectorDataProvider::ChangeGeometries ) && mDataProvider->changeGeometryValues( mChangedGeometries ) )
    {
      mCommitErrors << tr( "SUCCESS: %n geometries were changed.", "changed geometries count", mChangedGeometries.size() );
      mChangedGeometries.clear();
    }
    else
    {
      mCommitErrors << tr( "ERROR: %n geometries not changed.", "not changed geometries count", mChangedGeometries.size() );
      success = false;
    }
  }

  //
  // delete features
  //
  if ( mDeletedFeatureIds.size() > 0 )
  {
    if (( cap & QgsVectorDataProvider::DeleteFeatures ) && mDataProvider->deleteFeatures( mDeletedFeatureIds ) )
    {
      mCommitErrors << tr( "SUCCESS: %n feature(s) deleted.", "deleted features count", mDeletedFeatureIds.size() );
      for ( QgsFeatureIds::const_iterator it = mDeletedFeatureIds.begin(); it != mDeletedFeatureIds.end(); it++ )
      {
        mChangedAttributeValues.remove( *it );
        mChangedGeometries.remove( *it );
      }
      mDeletedFeatureIds.clear();
    }
    else
    {
      mCommitErrors << tr( "ERROR: %n feature(s) not deleted.", "not deleted features count", mDeletedFeatureIds.size() );
      success = false;
    }
  }

  deleteCachedGeometries();

  if ( success )
  {
    mEditable = false;
    setModified( FALSE );

    mUpdatedFields.clear();
    mMaxUpdatedIndex = -1;
    undoStack()->clear();
    emit editingStopped();
  }


  mDataProvider->updateExtents();
  mDataProvider->updateFeatureCount();

  triggerRepaint();

  QgsDebugMsg( "result:\n  " + mCommitErrors.join( "\n  " ) );

  return success;
}

const QStringList &QgsVectorLayer::commitErrors()
{
  return mCommitErrors;
}

bool QgsVectorLayer::rollBack()
{
  if ( !isEditable() )
  {
    return false;
  }

  if ( isModified() )
  {
    while ( mAddedAttributeIds.size() > 0 )
    {
      int idx = *mAddedAttributeIds.begin();
      mAddedAttributeIds.remove( idx );
      mUpdatedFields.remove( idx );
      emit attributeDeleted( idx );
    }

    while ( mDeletedAttributeIds.size() > 0 )
    {
      int idx = *mDeletedAttributeIds.begin();
      mDeletedAttributeIds.remove( idx );
      emit attributeAdded( idx );
    }

    // roll back changed attribute values
    mChangedAttributeValues.clear();

    // roll back changed geometries
    mChangedGeometries.clear();

    // Roll back added features
    // Delete the features themselves before deleting the references to them.
    mAddedFeatures.clear();

    // Roll back deleted features
    mDeletedFeatureIds.clear();

    // clear private field map
    mUpdatedFields.clear();
    mMaxUpdatedIndex = -1;
  }

  deleteCachedGeometries();

  mEditable = false;
  emit editingStopped();

  setModified( FALSE );
  // invalidate the cache so the layer updates properly to show its original
  // after the rollback
  setCacheImage( 0 );
  triggerRepaint();


  return true;
}

void QgsVectorLayer::setSelectedFeatures( const QgsFeatureIds& ids )
{
  // TODO: check whether features with these ID exist
  mSelectedFeatureIds = ids;
  emit selectionChanged();
}

int QgsVectorLayer::selectedFeatureCount()
{
  return mSelectedFeatureIds.size();
}

const QgsFeatureIds& QgsVectorLayer::selectedFeaturesIds() const
{
  return mSelectedFeatureIds;
}


QgsFeatureList QgsVectorLayer::selectedFeatures()
{
  if ( !mDataProvider )
  {
    return QgsFeatureList();
  }

  QgsFeatureList features;

  QgsAttributeList allAttrs = mDataProvider->attributeIndexes();

  for ( QgsFeatureIds::iterator it = mSelectedFeatureIds.begin(); it != mSelectedFeatureIds.end(); ++it )
  {
    QgsFeature feat;

    bool selectionIsAddedFeature = FALSE;

    // Check this selected item against the uncommitted added features
    for ( QgsFeatureList::iterator iter = mAddedFeatures.begin(); iter != mAddedFeatures.end(); ++iter )
    {
      if ( *it == iter->id() )
      {
        feat = QgsFeature( *iter );
        selectionIsAddedFeature = TRUE;
        break;
      }
    }

    // if the geometry is not newly added, get it from provider
    if ( !selectionIsAddedFeature )
    {
      mDataProvider->featureAtId( *it, feat, true, allAttrs );
    }

    updateFeatureAttributes( feat );
    updateFeatureGeometry( feat );

    features << feat;
  } // for each selected

  return features;
}

bool QgsVectorLayer::addFeatures( QgsFeatureList features, bool makeSelected )
{
  if ( !mDataProvider )
  {
    return false;
  }

  if ( !( mDataProvider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
  {
    return false;
  }

  if ( !isEditable() )
  {
    return false;
  }

  if ( makeSelected )
  {
    mSelectedFeatureIds.clear();
  }

  for ( QgsFeatureList::iterator iter = features.begin(); iter != features.end(); ++iter )
  {
    addFeature( *iter );

    if ( makeSelected )
    {
      mSelectedFeatureIds.insert( iter->id() );
    }
  }

  updateExtents();

  if ( makeSelected )
  {
    emit selectionChanged();
  }

  return true;
}


bool QgsVectorLayer::copySymbologySettings( const QgsMapLayer& other )
{
  const QgsVectorLayer* vl = qobject_cast<const QgsVectorLayer *>( &other );

  // exit if both vectorlayer are the same
  if ( this == vl )
  {
    return false;
  }

  if ( !vl )
  {
    return false;
  }
  delete mRenderer;

  QgsRenderer* r = vl->mRenderer;
  if ( r )
  {
    mRenderer = r->clone();
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsVectorLayer::hasCompatibleSymbology( const QgsMapLayer& other ) const
{
  // vector layers are symbology compatible if they have the same type, the same sequence of numerical/ non numerical fields and the same field names

  const QgsVectorLayer* otherVectorLayer = qobject_cast<const QgsVectorLayer *>( &other );
  if ( otherVectorLayer )
  {
    if ( otherVectorLayer->type() != type() )
    {
      return false;
    }

    const QgsFieldMap& fieldsThis = mDataProvider->fields();
    const QgsFieldMap& fieldsOther = otherVectorLayer ->mDataProvider->fields();

    if ( fieldsThis.size() != fieldsOther.size() )
    {
      return false;
    }

    // TODO: fill two sets with the numerical types for both layers

    uint fieldsThisSize = fieldsThis.size();

    for ( uint i = 0; i < fieldsThisSize; ++i )
    {
      if ( fieldsThis[i].name() != fieldsOther[i].name() ) // field names need to be the same
      {
        return false;
      }
      // TODO: compare types of the fields
    }
    return true; // layers are symbology compatible if the code reaches this point
  }
  return false;
}

bool QgsVectorLayer::snapPoint( QgsPoint& point, double tolerance )
{
  QMultiMap<double, QgsSnappingResult> snapResults;
  int result = snapWithContext( point, tolerance, snapResults, QgsSnapper::SnapToVertex );

  if ( result != 0 )
  {
    return false;
  }

  if ( snapResults.size() < 1 )
  {
    return false;
  }

  QMultiMap<double, QgsSnappingResult>::const_iterator snap_it = snapResults.constBegin();
  point.setX( snap_it.value().snappedVertex.x() );
  point.setY( snap_it.value().snappedVertex.y() );
  return true;
}


int QgsVectorLayer::snapWithContext( const QgsPoint& startPoint, double snappingTolerance,
                                     QMultiMap<double, QgsSnappingResult>& snappingResults,
                                     QgsSnapper::SnappingType snap_to )
{
  if ( snappingTolerance <= 0 || !mDataProvider )
  {
    return 1;
  }

  QList<QgsFeature> featureList;
  QgsRectangle searchRect( startPoint.x() - snappingTolerance, startPoint.y() - snappingTolerance,
                           startPoint.x() + snappingTolerance, startPoint.y() + snappingTolerance );
  double sqrSnappingTolerance = snappingTolerance * snappingTolerance;

  int n = 0;
  QgsFeature f;

  if ( mCachedGeometriesRect.contains( searchRect ) )
  {
    QgsDebugMsg( "Using cached geometries for snapping." );

    QgsGeometryMap::iterator it = mCachedGeometries.begin();
    for ( ; it != mCachedGeometries.end() ; ++it )
    {
      QgsGeometry* g = &( it.value() );
      if ( g->boundingBox().intersects( searchRect ) )
      {
        snapToGeometry( startPoint, it.key(), g, sqrSnappingTolerance, snappingResults, snap_to );
        ++n;
      }
    }
  }
  else
  {
    // snapping outside cached area

    select( QgsAttributeList(), searchRect, true, true );

    while ( nextFeature( f ) )
    {
      snapToGeometry( startPoint, f.id(), f.geometry(), sqrSnappingTolerance, snappingResults, snap_to );
      ++n;
    }
  }

  return n == 0 ? 2 : 0;
}

void QgsVectorLayer::snapToGeometry( const QgsPoint& startPoint, int featureId, QgsGeometry* geom, double sqrSnappingTolerance,
                                     QMultiMap<double, QgsSnappingResult>& snappingResults, QgsSnapper::SnappingType snap_to ) const
{
  if ( !geom )
  {
    return;
  }

  int atVertex, beforeVertex, afterVertex;
  double sqrDistVertexSnap, sqrDistSegmentSnap;
  QgsPoint snappedPoint;
  QgsSnappingResult snappingResultVertex;
  QgsSnappingResult snappingResultSegment;

  if ( snap_to == QgsSnapper::SnapToVertex || snap_to == QgsSnapper::SnapToVertexAndSegment )
  {
    snappedPoint = geom->closestVertex( startPoint, atVertex, beforeVertex, afterVertex, sqrDistVertexSnap );
    if ( sqrDistVertexSnap < sqrSnappingTolerance )
    {
      snappingResultVertex.snappedVertex = snappedPoint;
      snappingResultVertex.snappedVertexNr = atVertex;
      snappingResultVertex.beforeVertexNr = beforeVertex;
      if ( beforeVertex != -1 ) // make sure the vertex is valid
      {
        snappingResultVertex.beforeVertex = geom->vertexAt( beforeVertex );
      }
      snappingResultVertex.afterVertexNr = afterVertex;
      if ( afterVertex != -1 ) // make sure the vertex is valid
      {
        snappingResultVertex.afterVertex = geom->vertexAt( afterVertex );
      }
      snappingResultVertex.snappedAtGeometry = featureId;
      snappingResultVertex.layer = this;
      snappingResults.insert( sqrt( sqrDistVertexSnap ), snappingResultVertex );
      return;
    }
  }
  if ( snap_to == QgsSnapper::SnapToSegment || snap_to == QgsSnapper::SnapToVertexAndSegment ) // snap to segment
  {
    if ( geometryType() != QGis::Point ) // cannot snap to segment for points/multipoints
    {
      sqrDistSegmentSnap = geom->closestSegmentWithContext( startPoint, snappedPoint, afterVertex );

      if ( sqrDistSegmentSnap < sqrSnappingTolerance )
      {
        snappingResultSegment.snappedVertex = snappedPoint;
        snappingResultSegment.snappedVertexNr = -1;
        snappingResultSegment.beforeVertexNr = afterVertex - 1;
        snappingResultSegment.afterVertexNr = afterVertex;
        snappingResultSegment.snappedAtGeometry = featureId;
        snappingResultSegment.beforeVertex = geom->vertexAt( afterVertex - 1 );
        snappingResultSegment.afterVertex = geom->vertexAt( afterVertex );
        snappingResultSegment.layer = this;
        snappingResults.insert( sqrt( sqrDistSegmentSnap ), snappingResultSegment );
      }
    }
  }

}

int QgsVectorLayer::insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults )
{
  int returnval = 0;
  QgsPoint layerPoint;

  QList<QgsSnappingResult>::const_iterator it = snapResults.constBegin();
  for ( ; it != snapResults.constEnd(); ++it )
  {
    if ( it->snappedVertexNr == -1 ) // segment snap
    {
      layerPoint = it->snappedVertex;
      if ( !insertVertex( layerPoint.x(), layerPoint.y(), it->snappedAtGeometry, it->afterVertexNr ) )
      {
        returnval = 3;
      }
    }
  }
  return returnval;
}

int QgsVectorLayer::boundingBoxFromPointList( const QList<QgsPoint>& list, double& xmin, double& ymin, double& xmax, double& ymax ) const
{
  if ( list.size() < 1 )
  {
    return 1;
  }

  xmin = std::numeric_limits<double>::max();
  xmax = -std::numeric_limits<double>::max();
  ymin = std::numeric_limits<double>::max();
  ymax = -std::numeric_limits<double>::max();

  for ( QList<QgsPoint>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
  {
    if ( it->x() < xmin )
    {
      xmin = it->x();
    }
    if ( it->x() > xmax )
    {
      xmax = it->x();
    }
    if ( it->y() < ymin )
    {
      ymin = it->y();
    }
    if ( it->y() > ymax )
    {
      ymax = it->y();
    }
  }

  return 0;
}

QgsVectorLayer::VertexMarkerType QgsVectorLayer::currentVertexMarkerType()
{
  QSettings settings;
  QString markerTypeString = settings.value( "/qgis/digitizing/marker_style", "SemiTransparentCircle" ).toString();
  if ( markerTypeString == "Cross" )
  {
    return QgsVectorLayer::Cross;
  }
  else if ( markerTypeString == "SemiTransparentCircle" )
  {
    return QgsVectorLayer::SemiTransparentCircle;
  }
  else
  {
    return QgsVectorLayer::NoMarker;
  }
}

int QgsVectorLayer::currentVertexMarkerSize()
{
  QSettings settings;
  return settings.value( "/qgis/digitizing/marker_size", 7 ).toInt();
}

void QgsVectorLayer::drawFeature( QgsRenderContext &renderContext,
                                  QgsFeature& fet,
                                  QImage * marker )
{
  QPainter *p = renderContext.painter();
  // Only have variables, etc outside the switch() statement that are
  // used in all cases of the statement (otherwise they may get
  // executed, but never used, in a bit of code where performance is
  // critical).
  if ( ! fet.isValid() ) { return; }
  bool needToTrim = false;

  QgsGeometry* geom = fet.geometry();
  unsigned char* feature = geom->asWkb();

  QGis::WkbType wkbType = geom->wkbType();

  switch ( wkbType )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    {
      double x = *(( double * )( feature + 5 ) );
      double y = *(( double * )( feature + 5 + sizeof( double ) ) );

      transformPoint( x, y, &renderContext.mapToPixel(), renderContext.coordinateTransform() );
      if ( std::abs( x ) > QgsClipper::MAX_X ||
           std::abs( y ) > QgsClipper::MAX_Y )
      {
        break;
      }

      //QPointF pt(x - (marker->width()/2),  y - (marker->height()/2));
      QPointF pt( x*renderContext.rasterScaleFactor() - ( marker->width() / 2 ),
                  y*renderContext.rasterScaleFactor() - ( marker->height() / 2 ) );

      p->save();
      //p->scale(markerScaleFactor,markerScaleFactor);
      p->scale( 1.0 / renderContext.rasterScaleFactor(), 1.0 / renderContext.rasterScaleFactor() );
      p->drawImage( pt, *marker );
      p->restore();

      break;
    }
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
    {
      unsigned char *ptr = feature + 5;
      unsigned int nPoints = *(( int* )ptr );
      ptr += 4;

      p->save();
      //p->scale(markerScaleFactor, markerScaleFactor);
      p->scale( 1.0 / renderContext.rasterScaleFactor(), 1.0 / renderContext.rasterScaleFactor() );

      for ( register unsigned int i = 0; i < nPoints; ++i )
      {
        ptr += 5;
        double x = *(( double * ) ptr );
        ptr += sizeof( double );
        double y = *(( double * ) ptr );
        ptr += sizeof( double );

        if ( wkbType == QGis::WKBMultiPoint25D ) // ignore Z value
          ptr += sizeof( double );

        transformPoint( x, y, &renderContext.mapToPixel(), renderContext.coordinateTransform() );
        //QPointF pt(x - (marker->width()/2),  y - (marker->height()/2));
        //QPointF pt(x/markerScaleFactor - (marker->width()/2),  y/markerScaleFactor - (marker->height()/2));
        QPointF pt( x*renderContext.rasterScaleFactor() - ( marker->width() / 2 ),
                    y*renderContext.rasterScaleFactor() - ( marker->height() / 2 ) );
        //QPointF pt( x, y );

        // Work around a +/- 32768 limitation on coordinates
        if ( std::abs( x ) > QgsClipper::MAX_X ||
             std::abs( y ) > QgsClipper::MAX_Y )
          needToTrim = true;
        else
          p->drawImage( pt, *marker );
      }
      p->restore();

      break;
    }
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    {
      drawLineString( feature, renderContext );
      break;
    }
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
    {
      unsigned char* ptr = feature + 5;
      unsigned int numLineStrings = *(( int* )ptr );
      ptr = feature + 9;

      for ( register unsigned int jdx = 0; jdx < numLineStrings; jdx++ )
      {
        ptr = drawLineString( ptr, renderContext );
      }
      break;
    }
    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    {
      drawPolygon( feature, renderContext );
      break;
    }
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
    {
      unsigned char *ptr = feature + 5;
      unsigned int numPolygons = *(( int* )ptr );
      ptr = feature + 9;
      for ( register unsigned int kdx = 0; kdx < numPolygons; kdx++ )
        ptr = drawPolygon( ptr, renderContext );
      break;
    }
    default:
      QgsDebugMsg( "Unknown WkbType ENCOUNTERED" );
      break;
  }
}



void QgsVectorLayer::setCoordinateSystem()
{
  QgsDebugMsg( "----- Computing Coordinate System" );

  //
  // Get the layers project info and set up the QgsCoordinateTransform
  // for this layer
  //

  // get CRS directly from provider
  *mCRS = mDataProvider->crs();

  //QgsCoordinateReferenceSystem provides a mechanism for FORCE a srs to be valid
  //which is inolves falling back to system, project or user selected
  //defaults if the srs is not properly intialised.
  //we only nee to do that if the srs is not alreay valid
  if ( !mCRS->isValid() )
  {
    mCRS->validate();
  }
}

// Convenience function to transform the given point
inline void QgsVectorLayer::transformPoint(
  double& x, double& y,
  const QgsMapToPixel* mtp,
  const QgsCoordinateTransform* ct )
{
  // transform the point
  if ( ct )
  {
    double z = 0;
    ct->transformInPlace( x, y, z );
  }

  // transform from projected coordinate system to pixel
  // position on map canvas
  mtp->transformInPlace( x, y );
}

inline void QgsVectorLayer::transformPoints(
  std::vector<double>& x, std::vector<double>& y, std::vector<double>& z,
  QgsRenderContext &renderContext )
{
  // transform the point
  if ( renderContext.coordinateTransform() )
    renderContext.coordinateTransform()->transformInPlace( x, y, z );

  // transform from projected coordinate system to pixel
  // position on map canvas
  renderContext.mapToPixel().transformInPlace( x, y );
}


const QString QgsVectorLayer::displayField() const
{
  return mDisplayField;
}

bool QgsVectorLayer::isEditable() const
{
  return ( mEditable && mDataProvider );
}

bool QgsVectorLayer::isModified() const
{
  return mModified;
}

void QgsVectorLayer::setModified( bool modified, bool onlyGeometry )
{
  mModified = modified;
  emit layerModified( onlyGeometry );
}

QgsVectorLayer::EditType QgsVectorLayer::editType( int idx )
{
  const QgsFieldMap &fields = pendingFields();
  if ( fields.contains( idx ) && mEditTypes.contains( fields[idx].name() ) )
    return mEditTypes[ fields[idx].name()];
  else
    return LineEdit;
}

void QgsVectorLayer::setEditType( int idx, EditType type )
{
  const QgsFieldMap &fields = pendingFields();
  if ( fields.contains( idx ) )
    mEditTypes[ fields[idx].name()] = type;
}

QString QgsVectorLayer::editForm()
{
  return mEditForm;
}

void QgsVectorLayer::setEditForm( QString ui )
{
  mEditForm = ui;
}

QMap< QString, QVariant > &QgsVectorLayer::valueMap( int idx )
{
  const QgsFieldMap &fields = pendingFields();

  // FIXME: throw an exception!?
  if ( !fields.contains( idx ) )
  {
    QgsDebugMsg( QString( "field %1 not found" ).arg( idx ) );
  }

  if ( !mValueMaps.contains( fields[idx].name() ) )
    mValueMaps[ fields[idx].name()] = QMap<QString, QVariant>();

  return mValueMaps[ fields[idx].name()];
}

QgsVectorLayer::RangeData &QgsVectorLayer::range( int idx )
{
  const QgsFieldMap &fields = pendingFields();

  // FIXME: throw an exception!?
  if ( fields.contains( idx ) )
  {
    QgsDebugMsg( QString( "field %1 not found" ).arg( idx ) );
  }

  if ( !mRanges.contains( fields[idx].name() ) )
    mRanges[ fields[idx].name()] = RangeData();

  return mRanges[ fields[idx].name()];
}

void QgsVectorLayer::addOverlay( QgsVectorOverlay* overlay )
{
  mOverlays.push_back( overlay );
}

void QgsVectorLayer::removeOverlay( const QString& typeName )
{
  for ( int i = mOverlays.size() - 1; i >= 0; --i )
  {
    if ( mOverlays.at( i )->typeName() == typeName )
    {
      mOverlays.removeAt( i );
    }
  }
}

void QgsVectorLayer::vectorOverlays( QList<QgsVectorOverlay*>& overlayList )
{
  overlayList = mOverlays;
}

QgsVectorOverlay* QgsVectorLayer::findOverlayByType( const QString& typeName )
{
  QList<QgsVectorOverlay*>::iterator it = mOverlays.begin();
  for ( ; it != mOverlays.end(); ++it )
  {
    if (( *it )->typeName() == typeName )
    {
      return *it;
    }
  }
  return 0; //not found
}


QgsFeatureRendererV2* QgsVectorLayer::rendererV2()
{
  return mRendererV2;
}
void QgsVectorLayer::setRendererV2( QgsFeatureRendererV2* r )
{
  delete mRendererV2;
  mRendererV2 = r;
}
bool QgsVectorLayer::isUsingRendererV2()
{
  return mUsingRendererV2;
}
void QgsVectorLayer::setUsingRendererV2( bool usingRendererV2 )
{
  mUsingRendererV2 = usingRendererV2;
}


void QgsVectorLayer::editGeometryChange( int featureId, QgsGeometry& geometry )
{
  if ( mActiveCommand != NULL )
  {
    mActiveCommand->storeGeometryChange( featureId, mChangedGeometries[ featureId ], geometry );
  }
  mChangedGeometries[ featureId ] = geometry;
}


void QgsVectorLayer::editFeatureAdd( QgsFeature& feature )
{
  if ( mActiveCommand != NULL )
  {
    mActiveCommand->storeFeatureAdd( feature );
  }
  mAddedFeatures.append( feature );
}

void QgsVectorLayer::editFeatureDelete( int featureId )
{
  if ( mActiveCommand != NULL )
  {
    mActiveCommand->storeFeatureDelete( featureId );
  }
  mDeletedFeatureIds.insert( featureId );
}

void QgsVectorLayer::editAttributeChange( int featureId, int field, QVariant value )
{
  if ( mActiveCommand != NULL )
  {
    QVariant original;
    bool isFirstChange = true;
    if ( featureId < 0 )
    {
      // work with added feature
      for ( int i = 0; i < mAddedFeatures.size(); i++ )
      {
        if ( mAddedFeatures[i].id() == featureId && mAddedFeatures[i].attributeMap().contains( field ) )
        {
          original = mAddedFeatures[i].attributeMap()[field];
          isFirstChange = false;
          break;
        }
      }
    }
    else
    {
      if ( mChangedAttributeValues.contains( featureId ) && mChangedAttributeValues[featureId].contains( field ) )
      {
        original = mChangedAttributeValues[featureId][field];
        isFirstChange = false;
      }
    }
    mActiveCommand->storeAttributeChange( featureId, field, original, value, isFirstChange );
  }

  if ( featureId >= 0 )
  {
    // changed attribute of existing feature
    if ( !mChangedAttributeValues.contains( featureId ) )
    {
      mChangedAttributeValues.insert( featureId, QgsAttributeMap() );
    }

    mChangedAttributeValues[featureId].insert( field, value );
  }
  else
  {
    // updated added feature
    for ( int i = 0; i < mAddedFeatures.size(); i++ )
    {
      if ( mAddedFeatures[i].id() == featureId )
      {
        mAddedFeatures[i].changeAttribute( field, value );
        break;
      }
    }
  }
}

void QgsVectorLayer::beginEditCommand( QString text )
{
  if ( mActiveCommand == NULL )
  {
    mActiveCommand = new QgsUndoCommand( this, text );
  }
}

void QgsVectorLayer::endEditCommand()
{
  if ( mActiveCommand != NULL )
  {
    undoStack()->push( mActiveCommand );
    mActiveCommand = NULL;
  }

}

void QgsVectorLayer::destroyEditCommand()
{
  if ( mActiveCommand != NULL )
  {
    undoEditCommand( mActiveCommand );
    delete mActiveCommand;
    mActiveCommand = NULL;
  }

}

void QgsVectorLayer::redoEditCommand( QgsUndoCommand* cmd )
{
  QMap<int, QgsUndoCommand::GeometryChangeEntry>& geometryChange = cmd->mGeometryChange;
  QgsFeatureIds& deletedFeatureIdChange = cmd->mDeletedFeatureIdChange;
  QgsFeatureList& addedFeatures = cmd->mAddedFeatures;
  QMap<int, QgsUndoCommand::AttributeChanges>& attributeChange = cmd->mAttributeChange;
  QgsFieldMap& addedAttributes = cmd->mAddedAttributes;
  QgsFieldMap& deletedAttributes = cmd->mDeletedAttributes;


  // geometry changes
  QMap<int, QgsUndoCommand::GeometryChangeEntry>::iterator it = geometryChange.begin();
  for ( ; it != geometryChange.end(); ++it )
  {
    if ( it.value().target == NULL )
    {
      mChangedGeometries.remove( it.key() );
    }
    else
    {
      mChangedGeometries[it.key()] = *( it.value().target );
    }
  }

  // deleted features
  QgsFeatureIds::iterator delIt = deletedFeatureIdChange.begin();
  for ( ; delIt != deletedFeatureIdChange.end(); ++delIt )
  {
    mDeletedFeatureIds.insert( *delIt );
  }

  // added features
  QgsFeatureList::iterator addIt = addedFeatures.begin();
  for ( ; addIt != addedFeatures.end(); ++addIt )
  {
    mAddedFeatures.append( *addIt );
  }

  // changed attributes
  QMap<int, QgsUndoCommand::AttributeChanges>::iterator attrFeatIt = attributeChange.begin();
  for ( ; attrFeatIt != attributeChange.end(); ++attrFeatIt )
  {
    int fid = attrFeatIt.key();
    // for every changed attribute in feature
    QMap<int, QgsUndoCommand::AttributeChangeEntry>::iterator  attrChIt = attrFeatIt.value().begin();
    for ( ; attrChIt != attrFeatIt.value().end(); ++attrChIt )
    {
      if ( fid >= 0 )
      {
        // existing feature
        if ( attrChIt.value().target.isNull() )
        {
          mChangedAttributeValues[fid].remove( attrChIt.key() );
        }
        else
        {
          mChangedAttributeValues[fid][attrChIt.key()] = attrChIt.value().target;
          QgsFeature f;
          featureAtId( fid, f, false, true );
          f.changeAttribute( attrChIt.key(), attrChIt.value().target );
        }
      }
      else
      {
        // added feature
        for ( int i = 0; i < mAddedFeatures.size(); i++ )
        {
          if ( mAddedFeatures[i].id() == fid )
          {
            mAddedFeatures[i].changeAttribute( attrChIt.key(), attrChIt.value().target );
            break;
          }
        }

      }

    }
  }

  // added attributes
  QgsFieldMap::iterator attrIt = addedAttributes.begin();
  for ( ; attrIt != addedAttributes.end(); ++attrIt )
  {
    int attrIndex = attrIt.key();
    mAddedAttributeIds.insert( attrIndex );
    mUpdatedFields.insert( attrIndex, attrIt.value() );
  }

  // deleted attributes
  QgsFieldMap::iterator dAttrIt = deletedAttributes.begin();
  for ( ; dAttrIt != deletedAttributes.end(); ++dAttrIt )
  {
    int attrIndex = dAttrIt.key();
    mDeletedAttributeIds.insert( attrIndex );
    mUpdatedFields.remove( attrIndex );
  }
  setModified( true );

  // it's not ideal to trigger refresh from here
  triggerRepaint();
}

void QgsVectorLayer::undoEditCommand( QgsUndoCommand* cmd )
{
  QMap<int, QgsUndoCommand::GeometryChangeEntry>& geometryChange = cmd->mGeometryChange;
  QgsFeatureIds& deletedFeatureIdChange = cmd->mDeletedFeatureIdChange;
  QgsFeatureList& addedFeatures = cmd->mAddedFeatures;
  QMap<int, QgsUndoCommand::AttributeChanges>& attributeChange = cmd->mAttributeChange;
  QgsFieldMap& addedAttributes = cmd->mAddedAttributes;
  QgsFieldMap& deletedAttributes = cmd->mDeletedAttributes;

  // deleted attributes
  QgsFieldMap::iterator dAttrIt = deletedAttributes.begin();
  for ( ; dAttrIt != deletedAttributes.end(); ++dAttrIt )
  {
    int attrIndex = dAttrIt.key();
    mDeletedAttributeIds.remove( attrIndex );
    mUpdatedFields.insert( attrIndex, dAttrIt.value() );
  }

  // added attributes
  QgsFieldMap::iterator attrIt = addedAttributes.begin();
  for ( ; attrIt != addedAttributes.end(); ++attrIt )
  {
    int attrIndex = attrIt.key();
    mAddedAttributeIds.remove( attrIndex );
    mUpdatedFields.remove( attrIndex );
  }

  // geometry changes
  QMap<int, QgsUndoCommand::GeometryChangeEntry>::iterator it = geometryChange.begin();
  for ( ; it != geometryChange.end(); ++it )
  {
    if ( it.value().original == NULL )
    {
      mChangedGeometries.remove( it.key() );
    }
    else
    {
      mChangedGeometries[it.key()] = *( it.value().original );
    }
  }

  // deleted features
  QgsFeatureIds::iterator delIt = deletedFeatureIdChange.begin();
  for ( ; delIt != deletedFeatureIdChange.end(); ++delIt )
  {
    mDeletedFeatureIds.remove( *delIt );
  }

  // added features
  QgsFeatureList::iterator addIt = addedFeatures.begin();
  for ( ; addIt != addedFeatures.end(); ++addIt )
  {
    QgsFeatureList::iterator addedIt = mAddedFeatures.begin();
    for ( ; addedIt != mAddedFeatures.end(); ++addedIt )
    {
      if ( addedIt->id() == addIt->id() )
      {
        mAddedFeatures.erase( addedIt );
        break; // feature was found so move to next one
      }
    }
  }

  // updated attributes
  QMap<int, QgsUndoCommand::AttributeChanges>::iterator attrFeatIt = attributeChange.begin();
  for ( ; attrFeatIt != attributeChange.end(); ++attrFeatIt )
  {
    int fid = attrFeatIt.key();
    QMap<int, QgsUndoCommand::AttributeChangeEntry>::iterator  attrChIt = attrFeatIt.value().begin();
    for ( ; attrChIt != attrFeatIt.value().end(); ++attrChIt )
    {
      if ( fid >= 0 )
      {
        if ( attrChIt.value().isFirstChange )
        {
          mChangedAttributeValues[fid].remove( attrChIt.key() );
        }
        else
        {
          mChangedAttributeValues[fid][attrChIt.key()] = attrChIt.value().original;
        }
      }
      else
      {
        // added feature TODO:
        for ( int i = 0; i < mAddedFeatures.size(); i++ )
        {
          if ( mAddedFeatures[i].id() == fid )
          {
            mAddedFeatures[i].changeAttribute( attrChIt.key(), attrChIt.value().original );
            break;
          }
        }

      }
      emit attributeValueChanged( fid, attrChIt.key(), attrChIt.value().original );
    }
  }
  setModified( true );

  // it's not ideal to trigger refresh from here
  triggerRepaint();
}

void QgsVectorLayer::setCheckedState( int idx, QString checked, QString unchecked )
{
  const QgsFieldMap &fields = pendingFields();
  if ( fields.contains( idx ) )
    mCheckedStates[ fields[idx].name()] = QPair<QString, QString>( checked, unchecked );
}

QPair<QString, QString> QgsVectorLayer::checkedState( int idx )
{
  const QgsFieldMap &fields = pendingFields();
  if ( fields.contains( idx ) && mCheckedStates.contains( fields[idx].name() ) )
    return mCheckedStates[ fields[idx].name()];
  else
    return QPair<QString, QString>( "1", "0" );
}
