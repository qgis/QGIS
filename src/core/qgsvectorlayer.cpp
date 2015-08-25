/***************************************************************************
                               qgsvectorlayer.cpp
                              --------------------
          begin                : Oct 29, 2003
          copyright            : (C) 2003 by Gary E.Sherman
          email                : sherman at mrcc.com

  This class implements a generic means to display vector layers. The features
  and attributes are read from the data store using a "data provider" plugin.
  QgsVectorLayer can be used with any data store for which an appropriate
  plugin is available.

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QProgressDialog>
#include <QSettings>
#include <QString>
#include <QDomNode>
#include <QVector>

#include "qgsvectorlayer.h"
#include "qgsattributeaction.h"
#include "qgis.h" //for globals
#include "qgsapplication.h"
#include "qgsclipper.h"
#include "qgsconditionalstyle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgscurvev2.h"
#include "qgsdatasourceuri.h"
#include "qgsexpressionfieldbuffer.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfield.h"
#include "qgsgeometrycache.h"
#include "qgsgeometry.h"
#include "qgslabel.h"
#include "qgslegacyhelpers.h"
#include "qgslogger.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsmessagelog.h"
#include "qgsogcutils.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsrectangle.h"
#include "qgsrelationmanager.h"
#include "qgsrendercontext.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsvectorlayereditpassthrough.h"
#include "qgsvectorlayereditutils.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvectorlayerrenderer.h"
#include "qgsvectorlayerundocommand.h"
#include "qgspointv2.h"
#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsdiagramrendererv2.h"
#include "qgsstylev2.h"
#include "qgssymbologyv2conversion.h"
#include "qgspallabeling.h"
#include "qgssimplifymethod.h"
#include "qgsexpressioncontext.h"

#include "diagram/qgsdiagram.h"

#ifdef TESTPROVIDERLIB
#include <dlfcn.h>
#endif

typedef bool saveStyle_t(
  const QString& uri,
  const QString& qmlStyle,
  const QString& sldStyle,
  const QString& styleName,
  const QString& styleDescription,
  const QString& uiFileContent,
  bool useAsDefault,
  QString& errCause
);

typedef QString loadStyle_t(
  const QString& uri,
  QString& errCause
);

typedef int listStyles_t(
  const QString& uri,
  QStringList &ids,
  QStringList &names,
  QStringList &descriptions,
  QString& errCause
);

typedef QString getStyleById_t(
  const QString& uri,
  QString styleID,
  QString& errCause
);

QgsVectorLayer::QgsVectorLayer( QString vectorLayerPath,
                                QString baseName,
                                QString providerKey,
                                bool loadDefaultStyleFlag )
    : QgsMapLayer( VectorLayer, baseName, vectorLayerPath )
    , mDataProvider( NULL )
    , mProviderKey( providerKey )
    , mReadOnly( false )
    , mWkbType( QGis::WKBUnknown )
    , mRendererV2( NULL )
    , mLabel( 0 )
    , mLabelOn( false )
    , mLabelFontNotFoundNotified( false )
    , mFeatureBlendMode( QPainter::CompositionMode_SourceOver ) // Default to normal feature blending
    , mLayerTransparency( 0 )
    , mVertexMarkerOnlyForSelection( false )
    , mEditorLayout( GeneratedLayout )
    , mFeatureFormSuppress( SuppressDefault )
    , mCache( new QgsGeometryCache() )
    , mEditBuffer( 0 )
    , mJoinBuffer( 0 )
    , mExpressionFieldBuffer( 0 )
    , mDiagramRenderer( 0 )
    , mDiagramLayerSettings( 0 )
    , mValidExtent( false )
    , mLazyExtent( true )
    , mSymbolFeatureCounted( false )
    , mEditCommandActive( false )
{
  mActions = new QgsAttributeAction( this );
  mConditionalStyles = new QgsConditionalLayerStyles();

  // if we're given a provider type, try to create and bind one to this layer
  if ( ! mProviderKey.isEmpty() )
  {
    setDataSource( vectorLayerPath, baseName, providerKey, loadDefaultStyleFlag );
  }

  connect( this, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SIGNAL( selectionChanged() ) );
  connect( this, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SIGNAL( repaintRequested() ) );
  connect( QgsProject::instance()->relationManager(), SIGNAL( relationsLoaded() ), this, SLOT( onRelationsLoaded() ) );

  // Default simplify drawing settings
  QSettings settings;
  mSimplifyMethod.setSimplifyHints(( QgsVectorSimplifyMethod::SimplifyHints ) settings.value( "/qgis/simplifyDrawingHints", ( int ) mSimplifyMethod.simplifyHints() ).toInt() );
  mSimplifyMethod.setThreshold( settings.value( "/qgis/simplifyDrawingTol", mSimplifyMethod.threshold() ).toFloat() );
  mSimplifyMethod.setForceLocalOptimization( settings.value( "/qgis/simplifyLocal", mSimplifyMethod.forceLocalOptimization() ).toBool() );
  mSimplifyMethod.setMaximumScale( settings.value( "/qgis/simplifyMaxScale", mSimplifyMethod.maximumScale() ).toFloat() );
} // QgsVectorLayer ctor



QgsVectorLayer::~QgsVectorLayer()
{
  QgsDebugMsg( "entered." );

  emit layerDeleted();

  mValid = false;

  delete mDataProvider;
  delete mEditBuffer;
  delete mJoinBuffer;
  delete mExpressionFieldBuffer;
  delete mCache;
  delete mLabel;
  delete mDiagramLayerSettings;
  delete mDiagramRenderer;

  delete mActions;

  delete mRendererV2;
  delete mConditionalStyles;
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
  if ( !hasGeometryType() )
    return;

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
    int fieldsSize = mUpdatedFields.size();

    for ( int idx = 0; idx < mUpdatedFields.count(); ++idx )
    {
      QString fldName = mUpdatedFields[idx].name();
      QgsDebugMsg( "Checking field " + fldName + " of " + QString::number( fieldsSize ) + " total" );

      // Check the fields and keep the first one that matches.
      // We assume that the user has organized the data with the
      // more "interesting" field names first. As such, name should
      // be selected before oldname, othername, etc.
      if ( fldName.indexOf( "name", 0, Qt::CaseInsensitive ) > -1 )
      {
        if ( idxName.isEmpty() )
        {
          idxName = fldName;
        }
      }
      if ( fldName.indexOf( "descrip", 0, Qt::CaseInsensitive ) > -1 )
      {
        if ( idxName.isEmpty() )
        {
          idxName = fldName;
        }
      }
      if ( fldName.indexOf( "id", 0, Qt::CaseInsensitive ) > -1 )
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
        mDisplayField = mUpdatedFields[0].name();
      }
    }

  }
}

// NOTE this is a temporary method added by Tim to prevent label clipping
// which was occurring when labeller was called in the main draw loop
// This method will probably be removed again in the near future!
void QgsVectorLayer::drawLabels( QgsRenderContext& rendererContext )
{
  if ( !hasGeometryType() )
    return;

  QgsDebugMsg( "Starting draw of labels: " + id() );

  if ( mRendererV2 && mLabelOn && mLabel &&
       ( !mLabel->scaleBasedVisibility() ||
         ( mLabel->minScale() <= rendererContext.rendererScale() &&
           rendererContext.rendererScale() <= mLabel->maxScale() ) ) )
  {
    QgsAttributeList attributes;
    foreach ( QString attrName, mRendererV2->usedAttributes() )
    {
      int attrNum = fieldNameIndex( attrName );
      attributes.append( attrNum );
    }
    // make sure the renderer is ready for classification ("symbolForFeature")
    mRendererV2->startRender( rendererContext, fields() );

    // Add fields required for labels
    mLabel->addRequiredFields( attributes );

    QgsDebugMsg( "Selecting features based on view extent" );

    int featureCount = 0;

    try
    {
      // select the records in the extent. The provider sets a spatial filter
      // and sets up the selection set for retrieval
      QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                            .setFilterRect( rendererContext.extent() )
                                            .setSubsetOfAttributes( attributes ) );

      QgsFeature fet;
      while ( fit.nextFeature( fet ) )
      {
        if ( mRendererV2->willRenderFeature( fet, rendererContext ) )
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
      QgsDebugMsg( "Error projecting label locations" );
    }

    if ( mRendererV2 )
    {
      mRendererV2->stopRender( rendererContext );
    }

    QgsDebugMsg( QString( "Total features processed %1" ).arg( featureCount ) );
  }
}

void QgsVectorLayer::reload()
{
  if ( mDataProvider )
  {
    mDataProvider->reloadData();
  }
}

QgsMapLayerRenderer* QgsVectorLayer::createMapRenderer( QgsRenderContext& rendererContext )
{
  return new QgsVectorLayerRenderer( this, rendererContext );
}

bool QgsVectorLayer::draw( QgsRenderContext& rendererContext )
{
  QgsVectorLayerRenderer renderer( this, rendererContext );
  return renderer.render();
}

void QgsVectorLayer::drawVertexMarker( double x, double y, QPainter& p, QgsVectorLayer::VertexMarkerType type, int m )
{
  if ( type == QgsVectorLayer::SemiTransparentCircle )
  {
    p.setPen( QColor( 50, 100, 120, 200 ) );
    p.setBrush( QColor( 200, 200, 210, 120 ) );
    p.drawEllipse( x - m, y - m, m * 2 + 1, m * 2 + 1 );
  }
  else if ( type == QgsVectorLayer::Cross )
  {
    p.setPen( QColor( 255, 0, 0 ) );
    p.drawLine( x - m, y + m, x + m, y - m );
    p.drawLine( x - m, y - m, x + m, y + m );
  }
}

void QgsVectorLayer::select( const QgsFeatureId& fid )
{
  mSelectedFeatureIds.insert( fid );

  emit selectionChanged( QgsFeatureIds() << fid, QgsFeatureIds(), false );
}

void QgsVectorLayer::select( const QgsFeatureIds& featureIds )
{
  mSelectedFeatureIds.unite( featureIds );

  emit selectionChanged( featureIds, QgsFeatureIds(), false );
}

void QgsVectorLayer::deselect( const QgsFeatureId fid )
{
  mSelectedFeatureIds.remove( fid );

  emit selectionChanged( QgsFeatureIds(), QgsFeatureIds() << fid, false );
}

void QgsVectorLayer::deselect( const QgsFeatureIds& featureIds )
{
  mSelectedFeatureIds.subtract( featureIds );

  emit selectionChanged( QgsFeatureIds(), featureIds, false );
}

void QgsVectorLayer::select( QgsRectangle & rect, bool addToSelection )
{
  // normalize the rectangle
  rect.normalize();

  //select all the elements
  QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                        .setFilterRect( rect )
                                        .setFlags( QgsFeatureRequest::ExactIntersect | QgsFeatureRequest::NoGeometry )
                                        .setSubsetOfAttributes( QgsAttributeList() ) );

  QgsFeatureIds ids;

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    ids << f.id();
  }

  if ( !addToSelection )
  {
    setSelectedFeatures( mSelectedFeatureIds + ids );
  }
  else
  {
    select( ids );
  }
}

void QgsVectorLayer::modifySelection( QgsFeatureIds selectIds, QgsFeatureIds deselectIds )
{
  QgsFeatureIds intersectingIds = selectIds & deselectIds;
  if ( intersectingIds.count() > 0 )
  {
    QgsDebugMsg( "Trying to select and deselect the same item at the same time. Unsure what to do. Selecting dubious items." );
  }

  mSelectedFeatureIds -= deselectIds;
  mSelectedFeatureIds += selectIds;

  emit selectionChanged( selectIds, deselectIds - intersectingIds, false );
}

void QgsVectorLayer::invertSelection()
{
  QgsFeatureIds ids = allFeatureIds();
  ids.subtract( mSelectedFeatureIds );
  setSelectedFeatures( ids );
}

void QgsVectorLayer::selectAll()
{
  setSelectedFeatures( allFeatureIds() );
}

QgsFeatureIds QgsVectorLayer::allFeatureIds()
{
  QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                        .setFlags( QgsFeatureRequest::NoGeometry )
                                        .setSubsetOfAttributes( QgsAttributeList() ) );

  QgsFeatureIds ids;

  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    ids << fet.id();
  }

  return ids;
}

void QgsVectorLayer::invertSelectionInRectangle( QgsRectangle & rect )
{
  // normalize the rectangle
  rect.normalize();

  QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                        .setFilterRect( rect )
                                        .setFlags( QgsFeatureRequest::NoGeometry | QgsFeatureRequest::ExactIntersect )
                                        .setSubsetOfAttributes( QgsAttributeList() ) );

  QgsFeatureIds selectIds;
  QgsFeatureIds deselectIds;

  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    if ( mSelectedFeatureIds.contains( fet.id() ) )
    {
      deselectIds << fet.id();
    }
    else
    {
      selectIds << fet.id();
    }
  }

  modifySelection( selectIds, deselectIds );
}

void QgsVectorLayer::removeSelection()
{
  if ( mSelectedFeatureIds.size() == 0 )
    return;

  setSelectedFeatures( QgsFeatureIds() );
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
  if ( mDataProvider && mDataProvider->encoding() != encoding )
  {
    mDataProvider->setEncoding( encoding );
    updateFields();
  }
}

void QgsVectorLayer::setDiagramRenderer( QgsDiagramRendererV2* r )
{
  delete mDiagramRenderer;
  mDiagramRenderer = r;
  emit rendererChanged();
}

QGis::GeometryType QgsVectorLayer::geometryType() const
{
  if ( mDataProvider )
  {
    QGis::WkbType type = mDataProvider->geometryType();
    return ( QGis::GeometryType )( QgsWKBTypes::geometryType(( QgsWKBTypes::Type )type ) );
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

bool QgsVectorLayer::hasGeometryType() const
{
  QGis::GeometryType t = geometryType();
  return ( t != QGis::NoGeometry && t != QGis::UnknownGeometry );
}

QGis::WkbType QgsVectorLayer::wkbType() const
{
  return mWkbType;
}

QgsRectangle QgsVectorLayer::boundingBoxOfSelected()
{
  if ( mSelectedFeatureIds.size() == 0 ) //no selected features
  {
    return QgsRectangle( 0, 0, 0, 0 );
  }

  QgsRectangle r, retval;
  retval.setMinimal();

  QgsFeature fet;
  if ( mDataProvider->capabilities() & QgsVectorDataProvider::SelectAtId )
  {
    foreach ( QgsFeatureId fid, mSelectedFeatureIds )
    {
      if ( getFeatures( QgsFeatureRequest()
                        .setFilterFid( fid )
                        .setSubsetOfAttributes( QgsAttributeList() ) )
           .nextFeature( fet ) &&
           fet.constGeometry() )
      {
        r = fet.constGeometry()->boundingBox();
        retval.combineExtentWith( &r );
      }
    }
  }
  else
  {
    QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                          .setSubsetOfAttributes( QgsAttributeList() ) );

    while ( fit.nextFeature( fet ) )
    {
      if ( mSelectedFeatureIds.contains( fet.id() ) )
      {
        if ( fet.constGeometry() )
        {
          r = fet.constGeometry()->boundingBox();
          retval.combineExtentWith( &r );
        }
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

bool QgsVectorLayer::labelsEnabled() const
{
  return customProperty( "labeling/enabled", QVariant( false ) ).toBool();
}

bool QgsVectorLayer::diagramsEnabled() const
{
  if ( !mDiagramRenderer || !mDiagramLayerSettings )
    return false;

  QList<QgsDiagramSettings> settingList = mDiagramRenderer->diagramSettings();
  if ( settingList.size() > 0 )
  {
    return settingList.at( 0 ).enabled;
  }
  return false;
}

long QgsVectorLayer::featureCount( QgsSymbolV2* symbol )
{
  if ( !mSymbolFeatureCounted ) return -1;
  return mSymbolFeatureCountMap.value( symbol );
}

bool QgsVectorLayer::countSymbolFeatures( bool showProgress )
{
  if ( mSymbolFeatureCounted ) return true;
  mSymbolFeatureCountMap.clear();

  if ( !mDataProvider )
  {
    QgsDebugMsg( "invoked with null mDataProvider" );
    return false;
  }
  if ( !mRendererV2 )
  {
    QgsDebugMsg( "invoked with null mRendererV2" );
    return false;
  }

  QgsLegendSymbolList symbolList = mRendererV2->legendSymbolItems();
  QgsLegendSymbolList::const_iterator symbolIt = symbolList.constBegin();

  for ( ; symbolIt != symbolList.constEnd(); ++symbolIt )
  {
    mSymbolFeatureCountMap.insert( symbolIt->second, 0 );
  }

  long nFeatures = featureCount();
  QProgressDialog progressDialog( tr( "Updating feature count for layer %1" ).arg( name() ), tr( "Abort" ), 0, nFeatures );
  progressDialog.setWindowTitle( tr( "QGIS" ) );
  progressDialog.setWindowModality( Qt::WindowModal );
  int featuresCounted = 0;

  QgsFeatureIterator fit = getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ) );

  // Renderer (rule based) may depend on context scale, with scale is ignored if 0
  QgsRenderContext renderContext;
  renderContext.setRendererScale( 0 );
  renderContext.expressionContext() << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( this );

  mRendererV2->startRender( renderContext, fields() );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    renderContext.expressionContext().setFeature( f );
    QgsSymbolV2List featureSymbolList = mRendererV2->originalSymbolsForFeature( f, renderContext );
    for ( QgsSymbolV2List::iterator symbolIt = featureSymbolList.begin(); symbolIt != featureSymbolList.end(); ++symbolIt )
    {
      mSymbolFeatureCountMap[*symbolIt] += 1;
    }
    ++featuresCounted;

    if ( showProgress )
    {
      if ( featuresCounted % 50 == 0 )
      {
        if ( featuresCounted > nFeatures ) //sometimes the feature count is not correct
        {
          progressDialog.setMaximum( 0 );
        }
        progressDialog.setValue( featuresCounted );
        if ( progressDialog.wasCanceled() )
        {
          mSymbolFeatureCountMap.clear();
          mRendererV2->stopRender( renderContext );
          return false;
        }
      }
    }
  }
  mRendererV2->stopRender( renderContext );
  progressDialog.setValue( nFeatures );
  mSymbolFeatureCounted = true;
  return true;
}

void QgsVectorLayer::updateExtents()
{
  mValidExtent = false;
}

void QgsVectorLayer::setExtent( const QgsRectangle &r )
{
  QgsMapLayer::setExtent( r );
  mValidExtent = true;
}

QgsRectangle QgsVectorLayer::extent()
{
  QgsRectangle rect;
  rect.setMinimal();

  if ( !hasGeometryType() )
    return rect;

  if ( !mValidExtent && mLazyExtent && mDataProvider )
  {
    // get the extent
    QgsRectangle mbr = mDataProvider->extent();

    // show the extent
    QString s = mbr.toString();

    QgsDebugMsg( "Extent of layer: " +  s );
    // store the extent
    setExtent( mbr );

    mLazyExtent = false;
  }

  if ( mValidExtent )
    return QgsMapLayer::extent();

  if ( !mDataProvider )
  {
    QgsDebugMsg( "invoked with null mDataProvider" );
  }

  if ( mDataProvider && mEditBuffer && mEditBuffer->mDeletedFeatureIds.isEmpty() && mEditBuffer->mChangedGeometries.isEmpty() )
  {
    mDataProvider->updateExtents();

    // get the extent of the layer from the provider
    // but only when there are some features already
    if ( mDataProvider->featureCount() != 0 )
    {
      QgsRectangle r = mDataProvider->extent();
      rect.combineExtentWith( &r );
    }

    for ( QgsFeatureMap::iterator it = mEditBuffer->mAddedFeatures.begin(); it != mEditBuffer->mAddedFeatures.end(); ++it )
    {
      if ( it->constGeometry() )
      {
        QgsRectangle r = it->constGeometry()->boundingBox();
        rect.combineExtentWith( &r );
      }
    }
  }
  else
  {
    QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                          .setSubsetOfAttributes( QgsAttributeList() ) );

    QgsFeature fet;
    while ( fit.nextFeature( fet ) )
    {
      if ( fet.constGeometry() && fet.constGeometry()->type() != QGis::UnknownGeometry )
      {
        QgsRectangle bb = fet.constGeometry()->boundingBox();
        rect.combineExtentWith( &bb );
      }
    }
  }

  if ( rect.xMinimum() > rect.xMaximum() && rect.yMinimum() > rect.yMaximum() )
  {
    // special case when there are no features in provider nor any added
    rect = QgsRectangle(); // use rectangle with zero coordinates
  }

  setExtent( rect );

  // Send this (hopefully) up the chain to the map canvas
  emit recalculateExtents();

  return rect;
}

QString QgsVectorLayer::subsetString()
{
  if ( ! mDataProvider )
  {
    QgsDebugMsg( "invoked with null mDataProvider" );
    return 0;
  }
  return mDataProvider->subsetString();
}

bool QgsVectorLayer::setSubsetString( QString subset )
{
  if ( ! mDataProvider )
  {
    QgsDebugMsg( "invoked with null mDataProvider" );
    return false;
  }

  bool res = mDataProvider->setSubsetString( subset );

  // get the updated data source string from the provider
  mDataSource = mDataProvider->dataSourceUri();
  updateExtents();

  if ( res )
    emit repaintRequested();

  return res;
}

bool QgsVectorLayer::simplifyDrawingCanbeApplied( const QgsRenderContext& renderContext, QgsVectorSimplifyMethod::SimplifyHint simplifyHint ) const
{
  if ( mDataProvider && !mEditBuffer && ( hasGeometryType() && geometryType() != QGis::Point ) && ( mSimplifyMethod.simplifyHints() & simplifyHint ) && renderContext.useRenderingOptimization() )
  {
    double maximumSimplificationScale = mSimplifyMethod.maximumScale();

    // check maximum scale at which generalisation should be carried out
    if ( maximumSimplificationScale > 1 && renderContext.rendererScale() <= maximumSimplificationScale )
      return false;

    return true;
  }
  return false;
}

QgsConditionalLayerStyles* QgsVectorLayer::conditionalStyles() const
{
  return mConditionalStyles;
}

QgsFeatureIterator QgsVectorLayer::getFeatures( const QgsFeatureRequest& request )
{
  if ( !mDataProvider )
    return QgsFeatureIterator();

  return QgsFeatureIterator( new QgsVectorLayerFeatureIterator( new QgsVectorLayerFeatureSource( this ), true, request ) );
}


bool QgsVectorLayer::addFeature( QgsFeature& f, bool alsoUpdateExtent )
{
  Q_UNUSED( alsoUpdateExtent ); // TODO[MD]
  if ( !mEditBuffer || !mDataProvider )
    return false;

  bool success = mEditBuffer->addFeature( f );

  if ( success )
    updateExtents();

  return success;
}

bool QgsVectorLayer::updateFeature( QgsFeature &f )
{
  QgsFeatureRequest req;
  req.setFilterFid( f.id() );
  if ( !f.constGeometry() )
    req.setFlags( QgsFeatureRequest::NoGeometry );
  if ( f.attributes().isEmpty() )
    req.setSubsetOfAttributes( QgsAttributeList() );

  QgsFeature current;
  if ( !getFeatures( req ).nextFeature( current ) )
  {
    QgsDebugMsg( QString( "feature %1 could not be retrieved" ).arg( f.id() ) );
    return false;
  }

  if ( f.constGeometry() && current.constGeometry() && f.constGeometry() != current.constGeometry() && !f.constGeometry()->isGeosEqual( *current.constGeometry() ) )
  {
    if ( !changeGeometry( f.id(), f.geometry() ) )
    {
      QgsDebugMsg( QString( "geometry of feature %1 could not be changed." ).arg( f.id() ) );
      return false;
    }
  }

  QgsAttributes fa = f.attributes();
  QgsAttributes ca = current.attributes();

  for ( int attr = 0; attr < fa.count(); ++attr )
  {
    if ( fa[attr] != ca[attr] )
    {
      if ( !changeAttributeValue( f.id(), attr, fa[attr], ca[attr] ) )
      {
        QgsDebugMsg( QString( "attribute %1 of feature %2 could not be changed." ).arg( attr ).arg( f.id() ) );
        return false;
      }
    }
  }

  return true;
}


bool QgsVectorLayer::insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex )
{
  if ( !mEditBuffer || !mDataProvider )
    return false;

  QgsVectorLayerEditUtils utils( this );
  return utils.insertVertex( x, y, atFeatureId, beforeVertex );
}


bool QgsVectorLayer::moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex )
{
  if ( !mEditBuffer || !mDataProvider )
    return false;

  QgsVectorLayerEditUtils utils( this );
  return utils.moveVertex( x, y, atFeatureId, atVertex );
}

bool QgsVectorLayer::moveVertex( const QgsPointV2& p, QgsFeatureId atFeatureId, int atVertex )
{
  if ( !mEditBuffer || !mDataProvider )
    return false;

  QgsVectorLayerEditUtils utils( this );
  return utils.moveVertex( p, atFeatureId, atVertex );
}

bool QgsVectorLayer::deleteVertex( QgsFeatureId atFeatureId, int atVertex )
{
  if ( !mEditBuffer || !mDataProvider )
    return false;

  QgsVectorLayerEditUtils utils( this );
  return utils.deleteVertex( atFeatureId, atVertex );
}


bool QgsVectorLayer::deleteSelectedFeatures( int* deletedCount )
{
  if ( !( mDataProvider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
  {
    return false;
  }

  if ( !isEditable() )
  {
    return false;
  }

  int deleted = 0;
  int count = mSelectedFeatureIds.size();
  // Make a copy since deleteFeature modifies mSelectedFeatureIds
  QgsFeatureIds selectedFeatures( mSelectedFeatureIds );
  foreach ( QgsFeatureId fid, selectedFeatures )
  {
    deleted += deleteFeature( fid );  // removes from selection
  }

  triggerRepaint();
  updateExtents();

  if ( deletedCount )
  {
    *deletedCount = deleted;
  }

  return deleted == count;
}

int QgsVectorLayer::addRing( const QList<QgsPoint>& ring )
{
  if ( !mEditBuffer || !mDataProvider )
    return 6;

  QgsVectorLayerEditUtils utils( this );
  return utils.addRing( ring );
}

int QgsVectorLayer::addRing( QgsCurveV2* ring )
{
  if ( !mEditBuffer || !mDataProvider )
  {
    return 6;
  }

  if ( !ring )
  {
    return 1;
  }

  if ( !ring->isClosed() )
  {
    delete ring; return 2;
  }

  QgsVectorLayerEditUtils utils( this );
  return utils.addRing( ring );
}

int QgsVectorLayer::addPart( const QList<QgsPoint> &points )
{
  if ( !mEditBuffer || !mDataProvider )
    return 7;

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

  QgsVectorLayerEditUtils utils( this );
  return utils.addPart( points, *mSelectedFeatureIds.constBegin() );
}

int QgsVectorLayer::addPart( QgsCurveV2* ring )
{
  if ( !mEditBuffer || !mDataProvider )
    return 7;

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

  QgsVectorLayerEditUtils utils( this );
  return utils.addPart( ring, *mSelectedFeatureIds.constBegin() );
}

int QgsVectorLayer::translateFeature( QgsFeatureId featureId, double dx, double dy )
{
  if ( !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  return utils.translateFeature( featureId, dx, dy );
}

int QgsVectorLayer::splitParts( const QList<QgsPoint>& splitLine, bool topologicalEditing )
{
  if ( !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  return utils.splitParts( splitLine, topologicalEditing );
}

int QgsVectorLayer::splitFeatures( const QList<QgsPoint>& splitLine, bool topologicalEditing )
{
  if ( !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  return utils.splitFeatures( splitLine, topologicalEditing );
}

int QgsVectorLayer::removePolygonIntersections( QgsGeometry* geom, QgsFeatureIds ignoreFeatures )
{
  if ( !hasGeometryType() )
    return 1;

  int returnValue = 0;

  //first test if geom really has type polygon or multipolygon
  if ( geom->type() != QGis::Polygon )
  {
    return 1;
  }

  //get bounding box of geom
  QgsRectangle geomBBox = geom->boundingBox();

  //get list of features that intersect this bounding box
  QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                        .setFilterRect( geomBBox )
                                        .setFlags( QgsFeatureRequest::ExactIntersect )
                                        .setSubsetOfAttributes( QgsAttributeList() ) );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    if ( ignoreFeatures.contains( f.id() ) )
    {
      continue;
    }

    //call geometry->makeDifference for each feature
    const QgsGeometry *currentGeom = f.constGeometry();
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

int QgsVectorLayer::addTopologicalPoints( const QgsGeometry *geom )
{
  if ( !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  return utils.addTopologicalPoints( geom );
}

int QgsVectorLayer::addTopologicalPoints( const QgsPoint& p )
{
  if ( !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  return utils.addTopologicalPoints( p );
}

QgsLabel *QgsVectorLayer::label()
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

  if ( mReadOnly )
  {
    return false;
  }

  if ( mEditBuffer )
  {
    // editing already underway
    return false;
  }

  if ( mDataProvider->transaction() )
  {
    mEditBuffer = new QgsVectorLayerEditPassthrough( this );
  }
  else
  {
    mEditBuffer = new QgsVectorLayerEditBuffer( this );
  }
  // forward signals
  connect( mEditBuffer, SIGNAL( layerModified() ), this, SLOT( invalidateSymbolCountedFlag() ) );
  connect( mEditBuffer, SIGNAL( layerModified() ), this, SIGNAL( layerModified() ) ); // TODO[MD]: necessary?
  //connect( mEditBuffer, SIGNAL( layerModified() ), this, SLOT( triggerRepaint() ) ); // TODO[MD]: works well?
  connect( mEditBuffer, SIGNAL( featureAdded( QgsFeatureId ) ), this, SIGNAL( featureAdded( QgsFeatureId ) ) );
  connect( mEditBuffer, SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( onFeatureDeleted( QgsFeatureId ) ) );
  connect( mEditBuffer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry& ) ), this, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry& ) ) );
  connect( mEditBuffer, SIGNAL( attributeValueChanged( QgsFeatureId, int, QVariant ) ), this, SIGNAL( attributeValueChanged( QgsFeatureId, int, QVariant ) ) );
  connect( mEditBuffer, SIGNAL( attributeAdded( int ) ), this, SIGNAL( attributeAdded( int ) ) );
  connect( mEditBuffer, SIGNAL( attributeDeleted( int ) ), this, SIGNAL( attributeDeleted( int ) ) );

  connect( mEditBuffer, SIGNAL( committedAttributesDeleted( const QString &, const QgsAttributeList & ) ),
           this, SIGNAL( committedAttributesDeleted( const QString &, const QgsAttributeList & ) ) );

  connect( mEditBuffer, SIGNAL( committedAttributesAdded( const QString &, const QList<QgsField> & ) ),
           this, SIGNAL( committedAttributesAdded( const QString &, const QList<QgsField> & ) ) );

  connect( mEditBuffer, SIGNAL( committedFeaturesAdded( QString, QgsFeatureList ) ), this, SIGNAL( committedFeaturesAdded( QString, QgsFeatureList ) ) );
  connect( mEditBuffer, SIGNAL( committedFeaturesRemoved( QString, QgsFeatureIds ) ), this, SIGNAL( committedFeaturesRemoved( QString, QgsFeatureIds ) ) );

  connect( mEditBuffer, SIGNAL( committedAttributeValuesChanges( const QString &, const QgsChangedAttributesMap & ) ),
           this, SIGNAL( committedAttributeValuesChanges( const QString &, const QgsChangedAttributesMap & ) ) );

  connect( mEditBuffer, SIGNAL( committedGeometriesChanges( const QString &, const QgsGeometryMap & ) ),
           this, SIGNAL( committedGeometriesChanges( const QString &, const QgsGeometryMap & ) ) );

  updateFields();

  emit editingStarted();

  return true;
}

bool QgsVectorLayer::readXml( const QDomNode& layer_node )
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

  //load vector joins
  if ( !mJoinBuffer )
  {
    mJoinBuffer = new QgsVectorLayerJoinBuffer( this );
    connect( mJoinBuffer, SIGNAL( joinedFieldsChanged() ), this, SLOT( onJoinedFieldsChanged() ) );
  }
  mJoinBuffer->readXml( layer_node );

  if ( !mExpressionFieldBuffer )
    mExpressionFieldBuffer = new QgsExpressionFieldBuffer();
  mExpressionFieldBuffer->readXml( layer_node );

  updateFields();
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( checkJoinLayerRemove( QString ) ) );

  QDomNode prevExpNode = layer_node.namedItem( "previewExpression" );

  if ( prevExpNode.isNull() )
  {
    mDisplayExpression = "";
  }
  else
  {
    QDomElement prevExpElem = prevExpNode.toElement();
    mDisplayExpression = prevExpElem.text();
  }

  QString errorMsg;
  if ( !readSymbology( layer_node, errorMsg ) )
  {
    return false;
  }

  readStyleManager( layer_node );


  setLegend( QgsMapLayerLegend::defaultVectorLegend( this ) );

  return mValid;               // should be true if read successfully

} // void QgsVectorLayer::readXml


void QgsVectorLayer::setDataSource( QString dataSource, QString baseName, QString provider, bool loadDefaultStyleFlag )
{
  QGis::GeometryType oldGeomType = geometryType();

  mDataSource = dataSource;
  mLayerName = capitaliseLayerName( baseName );
  setLayerName( mLayerName );
  setDataProvider( provider );

  if ( mValid )
  {
    // Always set crs
    setCoordinateSystem();

    // reset style if loading default style, style is missing, or geometry type has changed
    if ( !rendererV2() || !legend() || oldGeomType != geometryType() || loadDefaultStyleFlag )
    {
      // check if there is a default style / propertysheet defined
      // for this layer and if so apply it
      bool defaultLoadedFlag = false;
      if ( loadDefaultStyleFlag )
      {
        loadDefaultStyle( defaultLoadedFlag );
      }

      // if the default style failed to load or was disabled use some very basic defaults
      if ( !defaultLoadedFlag && hasGeometryType() )
      {
        // add single symbol renderer
        setRendererV2( QgsFeatureRendererV2::defaultRenderer( geometryType() ) );
      }

      setLegend( QgsMapLayerLegend::defaultVectorLegend( this ) );
    }

    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( checkJoinLayerRemove( QString ) ) );
    emit repaintRequested();
  }
}


bool QgsVectorLayer::setDataProvider( QString const & provider )
{
  mProviderKey = provider;     // XXX is this necessary?  Usually already set
  // XXX when execution gets here.

  //XXX - This was a dynamic cast but that kills the Windows
  //      version big-time with an abnormal termination error
  delete mDataProvider;
  mDataProvider =
    ( QgsVectorDataProvider* )( QgsProviderRegistry::instance()->provider( provider, mDataSource ) );

  if ( mDataProvider )
  {
    QgsDebugMsg( "Instantiated the data provider plugin" );

    mValid = mDataProvider->isValid();
    if ( mValid )
    {
      // TODO: Check if the provider has the capability to send fullExtentCalculated
      connect( mDataProvider, SIGNAL( fullExtentCalculated() ), this, SLOT( updateExtents() ) );

      // get and store the feature type
      mWkbType = mDataProvider->geometryType();

      mJoinBuffer = new QgsVectorLayerJoinBuffer( this );
      connect( mJoinBuffer, SIGNAL( joinedFieldsChanged() ), this, SLOT( onJoinedFieldsChanged() ) );
      mExpressionFieldBuffer = new QgsExpressionFieldBuffer();
      updateFields();

      // look at the fields in the layer and set the primary
      // display field using some real fuzzy logic
      setDisplayField();

      if ( mProviderKey == "postgres" )
      {
        QgsDebugMsg( "Beautifying layer name " + name() );

        // adjust the display name for postgres layers
        QRegExp reg( "\"[^\"]+\"\\.\"([^\"]+)\"( \\([^)]+\\))?" );
        if ( reg.indexIn( name() ) >= 0 )
        {
          QStringList stuff = reg.capturedTexts();
          QString lName = stuff[1];

          const QMap<QString, QgsMapLayer*> &layers = QgsMapLayerRegistry::instance()->mapLayers();

          QMap<QString, QgsMapLayer*>::const_iterator it;
          for ( it = layers.constBegin(); it != layers.constEnd() && ( *it )->name() != lName; ++it )
            ;

          if ( it != layers.constEnd() && stuff.size() > 2 )
          {
            lName += "." + stuff[2].mid( 2, stuff[2].length() - 3 );
          }

          if ( !lName.isEmpty() )
            setLayerName( lName );
        }

        QgsDebugMsg( "Beautified layer name " + name() );

        // deal with unnecessary schema qualification to make v.in.ogr happy
        mDataSource = mDataProvider->dataSourceUri();
      }
      else if ( mProviderKey == "osm" )
      {
        // make sure that the "observer" has been removed from URI to avoid crashes
        mDataSource = mDataProvider->dataSourceUri();
      }
      else if ( provider == "ogr" )
      {
        // make sure that the /vsigzip or /vsizip is added to uri, if applicable
        mDataSource = mDataProvider->dataSourceUri();
        if ( mDataSource.right( 10 ) == "|layerid=0" )
          mDataSource.chop( 10 );
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

  connect( mDataProvider, SIGNAL( dataChanged() ), this, SIGNAL( dataChanged() ) );
  connect( mDataProvider, SIGNAL( dataChanged() ), this, SLOT( removeSelection() ) );
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
  mapLayerNode.setAttribute( "geometry", QGis::vectorGeometryType( geometryType() ) );

  // add provider node
  if ( mDataProvider )
  {
    QDomElement provider  = document.createElement( "provider" );
    provider.setAttribute( "encoding", mDataProvider->encoding() );
    QDomText providerText = document.createTextNode( providerType() );
    provider.appendChild( providerText );
    layer_node.appendChild( provider );
  }

  // save preview expression
  QDomElement prevExpElem = document.createElement( "previewExpression" );
  QDomText prevExpText = document.createTextNode( mDisplayExpression );
  prevExpElem.appendChild( prevExpText );
  layer_node.appendChild( prevExpElem );

  //save joins
  mJoinBuffer->writeXml( layer_node, document );

  // save expression fields
  mExpressionFieldBuffer->writeXml( layer_node, document );

  writeStyleManager( layer_node, document );

  // renderer specific settings
  QString errorMsg;
  return writeSymbology( layer_node, document, errorMsg );
} // bool QgsVectorLayer::writeXml

bool QgsVectorLayer::readSymbology( const QDomNode& node, QString& errorMessage )
{
  emit readCustomSymbology( node.toElement(), errorMessage );

  if ( hasGeometryType() )
  {
    // try renderer v2 first
    QDomElement rendererElement = node.firstChildElement( RENDERER_TAG_NAME );
    if ( !rendererElement.isNull() )
    {
      QgsFeatureRendererV2* r = QgsFeatureRendererV2::load( rendererElement );
      if ( !r )
        return false;

      setRendererV2( r );
    }
    else
    {
      QgsFeatureRendererV2* r = QgsSymbologyV2Conversion::readOldRenderer( node, geometryType() );
      if ( !r )
        r = QgsFeatureRendererV2::defaultRenderer( geometryType() );

      setRendererV2( r );
    }

    // get and set the display field if it exists.
    QDomNode displayFieldNode = node.namedItem( "displayfield" );
    if ( !displayFieldNode.isNull() )
    {
      QDomElement e = displayFieldNode.toElement();
      setDisplayField( e.text() );
    }

    // get and set the blend mode if it exists
    QDomNode blendModeNode = node.namedItem( "blendMode" );
    if ( !blendModeNode.isNull() )
    {
      QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) e.text().toInt() ) );
    }

    // get and set the feature blend mode if it exists
    QDomNode featureBlendModeNode = node.namedItem( "featureBlendMode" );
    if ( !featureBlendModeNode.isNull() )
    {
      QDomElement e = featureBlendModeNode.toElement();
      setFeatureBlendMode( QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) e.text().toInt() ) );
    }

    // get and set the layer transparency if it exists
    QDomNode layerTransparencyNode = node.namedItem( "layerTransparency" );
    if ( !layerTransparencyNode.isNull() )
    {
      QDomElement e = layerTransparencyNode.toElement();
      setLayerTransparency( e.text().toInt() );
    }

    // use scale dependent visibility flag
    QDomElement e = node.toElement();
    if ( mLabel )
    {
      mLabel->setScaleBasedVisibility( e.attribute( "scaleBasedLabelVisibilityFlag", "0" ) == "1" );
      mLabel->setMinScale( e.attribute( "minLabelScale", "1" ).toFloat() );
      mLabel->setMaxScale( e.attribute( "maxLabelScale", "100000000" ).toFloat() );
    }

    // get the simplification drawing settings
    mSimplifyMethod.setSimplifyHints(( QgsVectorSimplifyMethod::SimplifyHints ) e.attribute( "simplifyDrawingHints", "1" ).toInt() );
    mSimplifyMethod.setThreshold( e.attribute( "simplifyDrawingTol", "1" ).toFloat() );
    mSimplifyMethod.setForceLocalOptimization( e.attribute( "simplifyLocal", "1" ).toInt() );
    mSimplifyMethod.setMaximumScale( e.attribute( "simplifyMaxScale", "1" ).toFloat() );

    //also restore custom properties (for labeling-ng)
    readCustomProperties( node, "labeling" );

    // Test if labeling is on or off
    QDomNode labelnode = node.namedItem( "label" );
    QDomElement element = labelnode.toElement();
    int hasLabelsEnabled = element.text().toInt();
    Q_NOWARN_DEPRECATED_PUSH
    if ( hasLabelsEnabled < 1 )
    {
      enableLabels( false );
    }
    else
    {
      enableLabels( true );
    }
    Q_NOWARN_DEPRECATED_POP

    QDomNode labelattributesnode = node.namedItem( "labelattributes" );

    if ( !labelattributesnode.isNull() && mLabel )
    {
      QgsDebugMsg( "calling readXML" );
      mLabel->readXML( labelattributesnode );
    }

    //diagram renderer and diagram layer settings
    delete mDiagramRenderer; mDiagramRenderer = 0;
    QDomElement singleCatDiagramElem = node.firstChildElement( "SingleCategoryDiagramRenderer" );
    if ( !singleCatDiagramElem.isNull() )
    {
      mDiagramRenderer = new QgsSingleCategoryDiagramRenderer();
      mDiagramRenderer->readXML( singleCatDiagramElem, this );
    }
    QDomElement linearDiagramElem = node.firstChildElement( "LinearlyInterpolatedDiagramRenderer" );
    if ( !linearDiagramElem.isNull() )
    {
      mDiagramRenderer = new QgsLinearlyInterpolatedDiagramRenderer();
      mDiagramRenderer->readXML( linearDiagramElem, this );
    }

    if ( mDiagramRenderer )
    {
      QDomElement diagramSettingsElem = node.firstChildElement( "DiagramLayerSettings" );
      if ( !diagramSettingsElem.isNull() )
      {
        mDiagramLayerSettings = new QgsDiagramLayerSettings();
        mDiagramLayerSettings->readXML( diagramSettingsElem, this );
      }
    }
  }

  // process the attribute actions
  mActions->readXML( node );


  QDomNode editFormNode = node.namedItem( "editform" );
  if ( !editFormNode.isNull() )
  {
    QDomElement e = editFormNode.toElement();
    mEditForm = QgsProject::instance()->readPath( e.text() );
  }

  QDomNode editFormInitNode = node.namedItem( "editforminit" );
  if ( !editFormInitNode.isNull() )
  {
    mEditFormInit = editFormInitNode.toElement().text();
  }

  QDomNode fFSuppNode = node.namedItem( "featformsuppress" );
  if ( fFSuppNode.isNull() )
  {
    mFeatureFormSuppress = SuppressDefault;
  }
  else
  {
    QDomElement e = fFSuppNode.toElement();
    mFeatureFormSuppress = ( QgsVectorLayer::FeatureFormSuppress )e.text().toInt();
  }

  QDomNode annotationFormNode = node.namedItem( "annotationform" );
  if ( !annotationFormNode.isNull() )
  {
    QDomElement e = annotationFormNode.toElement();
    mAnnotationForm = QgsProject::instance()->readPath( e.text() );
  }

  mAttributeAliasMap.clear();
  QDomNode aliasesNode = node.namedItem( "aliases" );
  if ( !aliasesNode.isNull() )
  {
    QDomElement aliasElem;

    QDomNodeList aliasNodeList = aliasesNode.toElement().elementsByTagName( "alias" );
    for ( int i = 0; i < aliasNodeList.size(); ++i )
    {
      aliasElem = aliasNodeList.at( i ).toElement();

      QString field;
      if ( aliasElem.hasAttribute( "field" ) )
      {
        field = aliasElem.attribute( "field" );
      }
      else
      {
        int index = aliasElem.attribute( "index" ).toInt();

        if ( index >= 0 && index < fields().count() )
          field = fields()[ index ].name();
      }

      mAttributeAliasMap.insert( field, aliasElem.attribute( "name" ) );
    }
  }

  // tab display
  QDomNode editorLayoutNode = node.namedItem( "editorlayout" );
  if ( editorLayoutNode.isNull() )
  {
    mEditorLayout = GeneratedLayout;
  }
  else
  {
    if ( editorLayoutNode.toElement().text() == "uifilelayout" )
    {
      mEditorLayout = UiFileLayout;
    }
    else if ( editorLayoutNode.toElement().text() == "tablayout" )
    {
      mEditorLayout = TabLayout;
    }
    else
    {
      mEditorLayout = GeneratedLayout;
    }
  }

  //Attributes excluded from WMS and WFS
  mExcludeAttributesWMS.clear();
  QDomNode excludeWMSNode = node.namedItem( "excludeAttributesWMS" );
  if ( !excludeWMSNode.isNull() )
  {
    QDomNodeList attributeNodeList = excludeWMSNode.toElement().elementsByTagName( "attribute" );
    for ( int i = 0; i < attributeNodeList.size(); ++i )
    {
      mExcludeAttributesWMS.insert( attributeNodeList.at( i ).toElement().text() );
    }
  }

  mExcludeAttributesWFS.clear();
  QDomNode excludeWFSNode = node.namedItem( "excludeAttributesWFS" );
  if ( !excludeWFSNode.isNull() )
  {
    QDomNodeList attributeNodeList = excludeWFSNode.toElement().elementsByTagName( "attribute" );
    for ( int i = 0; i < attributeNodeList.size(); ++i )
    {
      mExcludeAttributesWFS.insert( attributeNodeList.at( i ).toElement().text() );
    }
  }

  // tabs and groups display info
  mAttributeEditorElements.clear();
  QDomNode attributeEditorFormNode = node.namedItem( "attributeEditorForm" );
  QDomNodeList attributeEditorFormNodeList = attributeEditorFormNode.toElement().childNodes();

  for ( int i = 0; i < attributeEditorFormNodeList.size(); i++ )
  {
    QDomElement elem = attributeEditorFormNodeList.at( i ).toElement();

    QgsAttributeEditorElement *attributeEditorWidget = attributeEditorElementFromDomElement( elem, this );
    mAttributeEditorElements.append( attributeEditorWidget );
  }

  conditionalStyles()->readXml( node );

  return true;
}

QgsAttributeEditorElement* QgsVectorLayer::attributeEditorElementFromDomElement( QDomElement &elem, QObject* parent )
{
  QgsAttributeEditorElement* newElement = NULL;

  if ( elem.tagName() == "attributeEditorContainer" )
  {
    QgsAttributeEditorContainer* container = new QgsAttributeEditorContainer( elem.attribute( "name" ), parent );

    QDomNodeList childNodeList = elem.childNodes();

    for ( int i = 0; i < childNodeList.size(); i++ )
    {
      QDomElement childElem = childNodeList.at( i ).toElement();
      QgsAttributeEditorElement *myElem = attributeEditorElementFromDomElement( childElem, container );
      if ( myElem )
        container->addChildElement( myElem );
    }

    newElement = container;
  }
  else if ( elem.tagName() == "attributeEditorField" )
  {
    QString name = elem.attribute( "name" );
    int idx = fieldNameIndex( name );
    newElement = new QgsAttributeEditorField( name, idx, parent );
  }
  else if ( elem.tagName() == "attributeEditorRelation" )
  {
    // At this time, the relations are not loaded
    // So we only grab the id and delegate the rest to onRelationsLoaded()
    QString name = elem.attribute( "name" );
    newElement = new QgsAttributeEditorRelation( name, elem.attribute( "relation", "[None]" ), parent );
  }
  return newElement;
}

bool QgsVectorLayer::writeSymbology( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const
{
  QDomElement mapLayerNode = node.toElement();

  emit writeCustomSymbology( mapLayerNode, doc, errorMessage );

  if ( hasGeometryType() )
  {
    QDomElement rendererElement = mRendererV2->save( doc );
    node.appendChild( rendererElement );

    // use scale dependent visibility flag
    if ( mLabel )
    {
      mapLayerNode.setAttribute( "scaleBasedLabelVisibilityFlag", mLabel->scaleBasedVisibility() ? 1 : 0 );
      mapLayerNode.setAttribute( "minLabelScale", QString::number( mLabel->minScale() ) );
      mapLayerNode.setAttribute( "maxLabelScale", QString::number( mLabel->maxScale() ) );
    }

    // save the simplification drawing settings
    mapLayerNode.setAttribute( "simplifyDrawingHints", QString::number( mSimplifyMethod.simplifyHints() ) );
    mapLayerNode.setAttribute( "simplifyDrawingTol", QString::number( mSimplifyMethod.threshold() ) );
    mapLayerNode.setAttribute( "simplifyLocal", mSimplifyMethod.forceLocalOptimization() ? 1 : 0 );
    mapLayerNode.setAttribute( "simplifyMaxScale", QString::number( mSimplifyMethod.maximumScale() ) );

    //save customproperties (for labeling ng)
    writeCustomProperties( node, doc );

    // add the blend mode field
    QDomElement blendModeElem  = doc.createElement( "blendMode" );
    QDomText blendModeText = doc.createTextNode( QString::number( QgsMapRenderer::getBlendModeEnum( blendMode() ) ) );
    blendModeElem.appendChild( blendModeText );
    node.appendChild( blendModeElem );

    // add the feature blend mode field
    QDomElement featureBlendModeElem  = doc.createElement( "featureBlendMode" );
    QDomText featureBlendModeText = doc.createTextNode( QString::number( QgsMapRenderer::getBlendModeEnum( featureBlendMode() ) ) );
    featureBlendModeElem.appendChild( featureBlendModeText );
    node.appendChild( featureBlendModeElem );

    // add the layer transparency
    QDomElement layerTransparencyElem  = doc.createElement( "layerTransparency" );
    QDomText layerTransparencyText = doc.createTextNode( QString::number( layerTransparency() ) );
    layerTransparencyElem.appendChild( layerTransparencyText );
    node.appendChild( layerTransparencyElem );

    // add the display field
    QDomElement dField  = doc.createElement( "displayfield" );
    QDomText dFieldText = doc.createTextNode( displayField() );
    dField.appendChild( dFieldText );
    node.appendChild( dField );

    // add label node
    QDomElement labelElem = doc.createElement( "label" );
    QDomText labelText = doc.createTextNode( "" );

    Q_NOWARN_DEPRECATED_PUSH
    if ( hasLabelsEnabled() )
    {
      labelText.setData( "1" );
    }
    else
    {
      labelText.setData( "0" );
    }
    Q_NOWARN_DEPRECATED_POP
    labelElem.appendChild( labelText );

    node.appendChild( labelElem );

    // Now we get to do all that all over again for QgsLabel

    if ( mLabel )
    {
      QString fieldname = mLabel->labelField( QgsLabel::Text );
      if ( fieldname != "" )
      {
        dField  = doc.createElement( "labelfield" );
        dFieldText = doc.createTextNode( fieldname );
        dField.appendChild( dFieldText );
        node.appendChild( dField );
      }

      mLabel->writeXML( node, doc );
    }

    if ( mDiagramRenderer )
    {
      mDiagramRenderer->writeXML( mapLayerNode, doc, this );
      if ( mDiagramLayerSettings )
        mDiagramLayerSettings->writeXML( mapLayerNode, doc, this );
    }
  }

  // FIXME
  // edittypes are written to the layerNode
  // by slot QgsEditorWidgetRegistry::writeMapLayer()
  // triggered by signal QgsProject::writeMapLayer()
  // still other editing settings are written here,
  // although they are not part of symbology either

  QDomElement efField  = doc.createElement( "editform" );
  QDomText efText = doc.createTextNode( QgsProject::instance()->writePath( mEditForm ) );
  efField.appendChild( efText );
  node.appendChild( efField );

  QDomElement efiField  = doc.createElement( "editforminit" );
  if ( !mEditFormInit.isEmpty() )
    efiField.appendChild( doc.createTextNode( mEditFormInit ) );
  node.appendChild( efiField );

  QDomElement fFSuppElem  = doc.createElement( "featformsuppress" );
  QDomText fFSuppText = doc.createTextNode( QString::number( featureFormSuppress() ) );
  fFSuppElem.appendChild( fFSuppText );
  node.appendChild( fFSuppElem );

  QDomElement afField = doc.createElement( "annotationform" );
  QDomText afText = doc.createTextNode( QgsProject::instance()->writePath( mAnnotationForm ) );
  afField.appendChild( afText );
  node.appendChild( afField );

  // tab display
  QDomElement editorLayoutElem  = doc.createElement( "editorlayout" );
  switch ( mEditorLayout )
  {
    case UiFileLayout:
      editorLayoutElem.appendChild( doc.createTextNode( "uifilelayout" ) );
      break;

    case TabLayout:
      editorLayoutElem.appendChild( doc.createTextNode( "tablayout" ) );
      break;

    case GeneratedLayout:
    default:
      editorLayoutElem.appendChild( doc.createTextNode( "generatedlayout" ) );
      break;
  }

  node.appendChild( editorLayoutElem );

  //attribute aliases
  if ( mAttributeAliasMap.size() > 0 )
  {
    QDomElement aliasElem = doc.createElement( "aliases" );
    QMap<QString, QString>::const_iterator a_it = mAttributeAliasMap.constBegin();
    for ( ; a_it != mAttributeAliasMap.constEnd(); ++a_it )
    {
      int idx = fieldNameIndex( a_it.key() );
      if ( idx < 0 )
        continue;

      QDomElement aliasEntryElem = doc.createElement( "alias" );
      aliasEntryElem.setAttribute( "field", a_it.key() );
      aliasEntryElem.setAttribute( "index", idx );
      aliasEntryElem.setAttribute( "name", a_it.value() );
      aliasElem.appendChild( aliasEntryElem );
    }
    node.appendChild( aliasElem );
  }

  //exclude attributes WMS
  QDomElement excludeWMSElem = doc.createElement( "excludeAttributesWMS" );
  QSet<QString>::const_iterator attWMSIt = mExcludeAttributesWMS.constBegin();
  for ( ; attWMSIt != mExcludeAttributesWMS.constEnd(); ++attWMSIt )
  {
    QDomElement attrElem = doc.createElement( "attribute" );
    QDomText attrText = doc.createTextNode( *attWMSIt );
    attrElem.appendChild( attrText );
    excludeWMSElem.appendChild( attrElem );
  }
  node.appendChild( excludeWMSElem );

  //exclude attributes WFS
  QDomElement excludeWFSElem = doc.createElement( "excludeAttributesWFS" );
  QSet<QString>::const_iterator attWFSIt = mExcludeAttributesWFS.constBegin();
  for ( ; attWFSIt != mExcludeAttributesWFS.constEnd(); ++attWFSIt )
  {
    QDomElement attrElem = doc.createElement( "attribute" );
    QDomText attrText = doc.createTextNode( *attWFSIt );
    attrElem.appendChild( attrText );
    excludeWFSElem.appendChild( attrElem );
  }
  node.appendChild( excludeWFSElem );

  // tabs and groups of edit form
  if ( mAttributeEditorElements.size() > 0 )
  {
    QDomElement tabsElem = doc.createElement( "attributeEditorForm" );

    for ( QList< QgsAttributeEditorElement* >::const_iterator it = mAttributeEditorElements.begin(); it != mAttributeEditorElements.end(); ++it )
    {
      QDomElement attributeEditorWidgetElem = ( *it )->toDomElement( doc );
      tabsElem.appendChild( attributeEditorWidgetElem );
    }

    node.appendChild( tabsElem );
  }

  // add attribute actions
  mActions->writeXML( node, doc );

  mConditionalStyles->writeXml( node, doc );

  return true;
}

bool QgsVectorLayer::readSld( const QDomNode& node, QString& errorMessage )
{
  // get the Name element
  QDomElement nameElem = node.firstChildElement( "Name" );
  if ( nameElem.isNull() )
  {
    errorMessage = "Warning: Name element not found within NamedLayer while it's required.";
  }

  if ( hasGeometryType() )
  {
    QgsFeatureRendererV2* r = QgsFeatureRendererV2::loadSld( node, geometryType(), errorMessage );
    if ( !r )
      return false;

    setRendererV2( r );

    // labeling
    readSldLabeling( node );
  }
  return true;
}


bool QgsVectorLayer::writeSld( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const
{
  Q_UNUSED( errorMessage );

  // store the Name element
  QDomElement nameNode = doc.createElement( "se:Name" );
  nameNode.appendChild( doc.createTextNode( name() ) );
  node.appendChild( nameNode );

  if ( hasGeometryType() )
  {
    node.appendChild( mRendererV2->writeSld( doc, name() ) );
  }
  return true;
}


bool QgsVectorLayer::changeGeometry( QgsFeatureId fid, QgsGeometry* geom )
{
  if ( !mEditBuffer || !mDataProvider )
  {
    return false;
  }

  updateExtents();

  return mEditBuffer->changeGeometry( fid, geom );
}


bool QgsVectorLayer::changeAttributeValue( QgsFeatureId fid, int field, QVariant value, bool emitSignal )
{
  Q_UNUSED( emitSignal );
  return changeAttributeValue( fid, field, value );
}

bool QgsVectorLayer::changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue )
{
  if ( !mEditBuffer || !mDataProvider )
    return false;

  return mEditBuffer->changeAttributeValue( fid, field, newValue, oldValue );
}

bool QgsVectorLayer::addAttribute( const QgsField &field )
{
  if ( !mEditBuffer || !mDataProvider )
    return false;

  return mEditBuffer->addAttribute( field );
}

void QgsVectorLayer::remAttributeAlias( int attIndex )
{
  if ( attIndex < 0 || attIndex >= fields().count() )
    return;

  QString name = fields()[ attIndex ].name();
  if ( mAttributeAliasMap.contains( name ) )
  {
    mAttributeAliasMap.remove( name );
    emit layerModified();
  }
}

void QgsVectorLayer::addAttributeAlias( int attIndex, QString aliasString )
{
  if ( attIndex < 0 || attIndex >= fields().count() )
    return;

  QString name = fields()[ attIndex ].name();

  mAttributeAliasMap.insert( name, aliasString );
  emit layerModified(); // TODO[MD]: should have a different signal?
}

void QgsVectorLayer::addAttributeEditorWidget( QgsAttributeEditorElement* data )
{
  mAttributeEditorElements.append( data );
}

const QString QgsVectorLayer::editorWidgetV2( int fieldIdx ) const
{
  if ( fieldIdx < 0 || fieldIdx >= mUpdatedFields.count() )
    return "TextEdit";

  return mEditorWidgetV2Types.value( mUpdatedFields[fieldIdx].name(), "TextEdit" );
}

const QString QgsVectorLayer::editorWidgetV2( const QString& fieldName ) const
{
  return mEditorWidgetV2Types.value( fieldName, "TextEdit" );
}

const QgsEditorWidgetConfig QgsVectorLayer::editorWidgetV2Config( int fieldIdx ) const
{
  if ( fieldIdx < 0 || fieldIdx >= mUpdatedFields.count() )
    return QgsEditorWidgetConfig();

  return mEditorWidgetV2Configs.value( mUpdatedFields[fieldIdx].name() );
}

const QgsEditorWidgetConfig QgsVectorLayer::editorWidgetV2Config( const QString& fieldName ) const
{
  return mEditorWidgetV2Configs.value( fieldName );
}

QString QgsVectorLayer::attributeAlias( int attributeIndex ) const
{
  if ( attributeIndex < 0 || attributeIndex >= fields().count() )
    return "";

  QString name = fields()[ attributeIndex ].name();

  return mAttributeAliasMap.value( name, "" );
}

QString QgsVectorLayer::attributeDisplayName( int attributeIndex ) const
{
  QString displayName = attributeAlias( attributeIndex );
  if ( displayName.isEmpty() )
  {
    if ( attributeIndex >= 0 && attributeIndex < mUpdatedFields.count() )
    {
      displayName = mUpdatedFields[attributeIndex].name();
    }
  }
  return displayName;
}

bool QgsVectorLayer::deleteAttribute( int index )
{
  if ( index < 0 || index >= fields().count() )
    return false;

  if ( mUpdatedFields.fieldOrigin( index ) == QgsFields::OriginExpression )
  {
    removeExpressionField( index );
    return true;
  }

  if ( !mEditBuffer || !mDataProvider )
    return false;

  return mEditBuffer->deleteAttribute( index );
}

bool QgsVectorLayer::deleteAttributes( QList<int> attrs )
{
  bool deleted = false;

  // Remove multiple occurences of same attribute
  attrs = attrs.toSet().toList();

  qSort( attrs.begin(), attrs.end(), qGreater<int>() );

  Q_FOREACH ( int attr, attrs )
  {
    if ( deleteAttribute( attr ) )
    {
      deleted = true;
    }
  }

  return deleted;
}

bool QgsVectorLayer::deleteFeature( QgsFeatureId fid )
{
  if ( !mEditBuffer )
    return false;

  bool res = mEditBuffer->deleteFeature( fid );
  if ( res )
    mSelectedFeatureIds.remove( fid ); // remove it from selection

  updateExtents();

  return res;
}

QgsAttributeList QgsVectorLayer::pkAttributeList() const
{
  QgsAttributeList pkAttributesList;

  QgsAttributeList providerIndexes = mDataProvider->pkAttributeIndexes();
  for ( int i = 0; i < mUpdatedFields.count(); ++i )
  {
    if ( mUpdatedFields.fieldOrigin( i ) == QgsFields::OriginProvider &&
         providerIndexes.contains( mUpdatedFields.fieldOriginIndex( i ) ) )
      pkAttributesList << i;
  }

  return pkAttributesList;
}

long QgsVectorLayer::featureCount() const
{
  return mDataProvider->featureCount() +
         ( mEditBuffer ? mEditBuffer->mAddedFeatures.size() - mEditBuffer->mDeletedFeatureIds.size() : 0 );
}

bool QgsVectorLayer::commitChanges()
{
  mCommitErrors.clear();

  if ( !mDataProvider )
  {
    mCommitErrors << tr( "ERROR: no provider" );
    return false;
  }

  if ( !mEditBuffer )
  {
    mCommitErrors << tr( "ERROR: layer not editable" );
    return false;
  }

  emit beforeCommitChanges();

  bool success = mEditBuffer->commitChanges( mCommitErrors );

  if ( success )
  {
    delete mEditBuffer;
    mEditBuffer = 0;
    undoStack()->clear();
    emit editingStopped();
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Commit errors:\n  %1" ).arg( mCommitErrors.join( "\n  " ) ) );
  }

  if ( mCache )
  {
    mCache->deleteCachedGeometries();
  }

  updateFields();
  mDataProvider->updateExtents();

  emit repaintRequested();

  return success;
}

const QStringList &QgsVectorLayer::commitErrors()
{
  return mCommitErrors;
}

bool QgsVectorLayer::rollBack( bool deleteBuffer )
{
  if ( !mEditBuffer )
  {
    return false;
  }

  emit beforeRollBack();

  mEditBuffer->rollBack();

  if ( isModified() )
  {
    // new undo stack roll back method
    // old method of calling every undo could cause many canvas refreshes
    undoStack()->setIndex( 0 );
  }

  updateFields();

  if ( deleteBuffer )
  {
    delete mEditBuffer;
    mEditBuffer = 0;
    undoStack()->clear();
  }
  emit editingStopped();

  if ( mCache )
  {
    mCache->deleteCachedGeometries();
  }

  emit repaintRequested();
  return true;
}

void QgsVectorLayer::setSelectedFeatures( const QgsFeatureIds& ids )
{
  QgsFeatureIds deselectedFeatures = mSelectedFeatureIds - ids;

  mSelectedFeatureIds = ids;

  emit selectionChanged( ids, deselectedFeatures, true );
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
  QgsFeatureList features;
  QgsFeature f;

  if ( mSelectedFeatureIds.count() <= 8 )
  {
    // for small amount of selected features, fetch them directly
    // because request with FilterFids would go iterate over the whole layer
    foreach ( QgsFeatureId fid, mSelectedFeatureIds )
    {
      getFeatures( QgsFeatureRequest( fid ) ).nextFeature( f );
      features << f;
    }
  }
  else
  {
    QgsFeatureIterator it = selectedFeaturesIterator();

    while ( it.nextFeature( f ) )
    {
      features.push_back( f );
    }
  }

  return features;
}

QgsFeatureIterator QgsVectorLayer::selectedFeaturesIterator( QgsFeatureRequest request )
{
  if ( mSelectedFeatureIds.count() == 0 )
    return QgsFeatureIterator();

  if ( geometryType() == QGis::NoGeometry )
    request.setFlags( QgsFeatureRequest::NoGeometry );

  if ( mSelectedFeatureIds.count() == 1 )
    request.setFilterFid( *mSelectedFeatureIds.constBegin() );
  else
    request.setFilterFids( mSelectedFeatureIds );

  return getFeatures( request );
}

bool QgsVectorLayer::addFeatures( QgsFeatureList features, bool makeSelected )
{
  if ( !mEditBuffer || !mDataProvider )
    return false;

  bool res = mEditBuffer->addFeatures( features );

  if ( makeSelected )
  {
    QgsFeatureIds ids;

    for ( QgsFeatureList::iterator iter = features.begin(); iter != features.end(); ++iter )
      ids << iter->id();

    setSelectedFeatures( ids );
  }

  updateExtents();

  return res;
}


bool QgsVectorLayer::snapPoint( QgsPoint& point, double tolerance )
{
  if ( !hasGeometryType() )
    return false;

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
  if ( !hasGeometryType() )
    return 1;

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

  if ( mCache->cachedGeometriesRect().contains( searchRect ) )
  {
    QgsGeometryMap& cachedGeometries = mCache->cachedGeometries();
    for ( QgsGeometryMap::iterator it = cachedGeometries.begin(); it != cachedGeometries.end() ; ++it )
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

    QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                          .setFilterRect( searchRect )
                                          .setFlags( QgsFeatureRequest::ExactIntersect )
                                          .setSubsetOfAttributes( QgsAttributeList() ) );

    while ( fit.nextFeature( f ) )
    {
      snapToGeometry( startPoint, f.id(), f.constGeometry(), sqrSnappingTolerance, snappingResults, snap_to );
      ++n;
    }
  }

  return n == 0 ? 2 : 0;
}

void QgsVectorLayer::snapToGeometry( const QgsPoint& startPoint,
                                     QgsFeatureId featureId,
                                     const QgsGeometry* geom,
                                     double sqrSnappingTolerance,
                                     QMultiMap<double, QgsSnappingResult>& snappingResults,
                                     QgsSnapper::SnappingType snap_to ) const
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
      sqrDistSegmentSnap = geom->closestSegmentWithContext( startPoint, snappedPoint, afterVertex, NULL, crs().geographicFlag() ? 1e-12 : 1e-8 );

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
  QgsVectorLayerEditUtils utils( this );
  return utils.insertSegmentVerticesForSnap( snapResults );
}


void QgsVectorLayer::setCoordinateSystem()
{
  QgsDebugMsg( "----- Computing Coordinate System" );

  //
  // Get the layers project info and set up the QgsCoordinateTransform
  // for this layer
  //

  if ( hasGeometryType() )
  {
    // get CRS directly from provider
    setCrs( mDataProvider->crs() );
  }
  else
  {
    setCrs( QgsCoordinateReferenceSystem( GEO_EPSG_CRS_AUTHID ) );
  }
}


const QString QgsVectorLayer::displayField() const
{
  return mDisplayField;
}

void QgsVectorLayer::setDisplayExpression( const QString &displayExpression )
{
  mDisplayExpression = displayExpression;
}

const QString QgsVectorLayer::displayExpression()
{
  return mDisplayExpression;
}

bool QgsVectorLayer::isEditable() const
{
  return ( mEditBuffer && mDataProvider );
}

bool QgsVectorLayer::isReadOnly() const
{
  return mReadOnly;
}

bool QgsVectorLayer::setReadOnly( bool readonly )
{
  // exit if the layer is in editing mode
  if ( readonly && mEditBuffer )
    return false;

  mReadOnly = readonly;
  return true;
}

bool QgsVectorLayer::isModified() const
{
  emit beforeModifiedCheck();
  return mEditBuffer && mEditBuffer->isModified();
}

QgsVectorLayer::EditType QgsVectorLayer::editType( int idx )
{
  if ( idx < 0 || idx >= mUpdatedFields.count() )
    return Hidden;

  Q_NOWARN_DEPRECATED_PUSH
  return QgsLegacyHelpers::convertEditType( editorWidgetV2( idx ), editorWidgetV2Config( idx ), this, mUpdatedFields[ idx ].name() );
  Q_NOWARN_DEPRECATED_POP
}

void QgsVectorLayer::setEditType( int idx, EditType type )
{
  if ( idx < 0 || idx >= mUpdatedFields.count() )
    return;

  QgsEditorWidgetConfig cfg;

  Q_NOWARN_DEPRECATED_PUSH
  const QString widgetType = QgsLegacyHelpers::convertEditType( type, cfg, this, mUpdatedFields[idx].name() );
  Q_NOWARN_DEPRECATED_POP

  setEditorWidgetV2( idx, widgetType );
  setEditorWidgetV2Config( idx, cfg );
}

QgsVectorLayer::EditorLayout QgsVectorLayer::editorLayout()
{
  return mEditorLayout;
}

void QgsVectorLayer::setEditorLayout( EditorLayout editorLayout )
{
  mEditorLayout = editorLayout;
}

void QgsVectorLayer::setEditorWidgetV2( int attrIdx, const QString& widgetType )
{
  if ( attrIdx < 0 || attrIdx >= mUpdatedFields.count() )
    return;

  mEditorWidgetV2Types[ mUpdatedFields[ attrIdx ].name()] = widgetType;
}

void QgsVectorLayer::setEditorWidgetV2Config( int attrIdx, const QgsEditorWidgetConfig& config )
{
  if ( attrIdx < 0 || attrIdx >= mUpdatedFields.count() )
    return;

  mEditorWidgetV2Configs[ mUpdatedFields[ attrIdx ].name()] = config;
}

QString QgsVectorLayer::editForm()
{
  return mEditForm;
}

void QgsVectorLayer::setEditForm( QString ui )
{
  if ( ui.isEmpty() || ui.isNull() )
  {
    setEditorLayout( GeneratedLayout );
  }
  else
  {
    setEditorLayout( UiFileLayout );
  }
  mEditForm = ui;
}

void QgsVectorLayer::setAnnotationForm( const QString& ui )
{
  mAnnotationForm = ui;
}

QString QgsVectorLayer::editFormInit()
{
  return mEditFormInit;
}

void QgsVectorLayer::setEditFormInit( QString function )
{
  mEditFormInit = function;
}

QMap< QString, QVariant > QgsVectorLayer::valueMap( int idx )
{
  return editorWidgetV2Config( idx );
}

QgsVectorLayer::RangeData QgsVectorLayer::range( int idx )
{
  const QgsEditorWidgetConfig cfg = editorWidgetV2Config( idx );
  return RangeData(
           cfg.value( "Min" ),
           cfg.value( "Max" ),
           cfg.value( "Step" )
         );
}

QString QgsVectorLayer::dateFormat( int idx )
{
  return editorWidgetV2Config( idx ).value( "DateFormat" ).toString();
}

QSize QgsVectorLayer::widgetSize( int idx )
{
  const QgsEditorWidgetConfig cfg = editorWidgetV2Config( idx );
  return QSize( cfg.value( "Width" ).toInt(), cfg.value( "Height" ).toInt() );
}

bool QgsVectorLayer::fieldEditable( int idx )
{
  if ( idx >= 0 && idx < mUpdatedFields.count() )
  {
    if ( mUpdatedFields.fieldOrigin( idx ) == QgsFields::OriginJoin
         || mUpdatedFields.fieldOrigin( idx ) == QgsFields::OriginExpression )
      return false;
    return mFieldEditables.value( mUpdatedFields[idx].name(), true );
  }
  else
    return true;
}

bool QgsVectorLayer::labelOnTop( int idx )
{
  if ( idx >= 0 && idx < mUpdatedFields.count() )
    return mLabelOnTop.value( mUpdatedFields[idx].name(), false );
  else
    return false;
}

void QgsVectorLayer::setFieldEditable( int idx, bool editable )
{
  if ( idx >= 0 && idx < mUpdatedFields.count() )
    mFieldEditables[ mUpdatedFields[idx].name()] = editable;
}

void QgsVectorLayer::setLabelOnTop( int idx, bool onTop )
{
  if ( idx >= 0 && idx < mUpdatedFields.count() )
    mLabelOnTop[ mUpdatedFields[idx].name()] = onTop;
}

void QgsVectorLayer::setRendererV2( QgsFeatureRendererV2 *r )
{
  if ( !hasGeometryType() )
    return;

  if ( r != mRendererV2 )
  {
    delete mRendererV2;
    mRendererV2 = r;
    mSymbolFeatureCounted = false;
    mSymbolFeatureCountMap.clear();

    emit rendererChanged();
  }
}



void QgsVectorLayer::beginEditCommand( QString text )
{
  if ( !mDataProvider )
  {
    return;
  }
  if ( !mDataProvider->transaction() )
  {
    undoStack()->beginMacro( text );
    mEditCommandActive = true;
    emit editCommandStarted( text );
  }
}

void QgsVectorLayer::endEditCommand()
{
  if ( !mDataProvider )
  {
    return;
  }
  if ( !mDataProvider->transaction() )
  {
    undoStack()->endMacro();
    mEditCommandActive = false;
    if ( mDeletedFids.count() )
    {
      emit featuresDeleted( mDeletedFids );
      mDeletedFids.clear();
    }
    emit editCommandEnded();
  }
}

void QgsVectorLayer::destroyEditCommand()
{
  if ( !mDataProvider )
  {
    return;
  }
  if ( !mDataProvider->transaction() )
  {
    undoStack()->endMacro();
    undoStack()->undo();
    mEditCommandActive = false;
    mDeletedFids.clear();
    emit editCommandDestroyed();
  }
}


void QgsVectorLayer::setCheckedState( int idx, QString checked, QString unchecked )
{
  QgsEditorWidgetConfig cfg = editorWidgetV2Config( idx );
  cfg["CheckedState"] = checked;
  cfg["UncheckedState"] = unchecked;
  setEditorWidgetV2Config( idx, cfg );
}

int QgsVectorLayer::fieldNameIndex( const QString& fieldName ) const
{
  return fields().fieldNameIndex( fieldName );
}

bool QgsVectorLayer::addJoin( const QgsVectorJoinInfo& joinInfo )
{
  return mJoinBuffer && mJoinBuffer->addJoin( joinInfo );
}

void QgsVectorLayer::checkJoinLayerRemove( QString theLayerId )
{
  removeJoin( theLayerId );
}

void QgsVectorLayer::removeJoin( const QString& joinLayerId )
{
  if ( mJoinBuffer )
    mJoinBuffer->removeJoin( joinLayerId );
}

const QList< QgsVectorJoinInfo > QgsVectorLayer::vectorJoins() const
{
  if ( mJoinBuffer )
    return mJoinBuffer->vectorJoins();
  else
    return QList< QgsVectorJoinInfo >();
}

int QgsVectorLayer::addExpressionField( const QString& exp, const QgsField& fld )
{
  mExpressionFieldBuffer->addExpression( exp, fld );
  updateFields();
  int idx = mUpdatedFields.indexFromName( fld.name() );
  emit attributeAdded( idx );
  return idx;
}

void QgsVectorLayer::removeExpressionField( int index )
{
  int oi = mUpdatedFields.fieldOriginIndex( index );
  mExpressionFieldBuffer->removeExpression( oi );
  updateFields();
  emit attributeDeleted( index );
}

const QString QgsVectorLayer::expressionField( int index )
{
  int oi = mUpdatedFields.fieldOriginIndex( index );
  return mExpressionFieldBuffer->expressions().value( oi ).expression;
}

void QgsVectorLayer::updateExpressionField( int index, const QString& exp )
{
  int oi = mUpdatedFields.fieldOriginIndex( index );
  mExpressionFieldBuffer->updateExpression( oi, exp );
}

void QgsVectorLayer::updateFields()
{
  if ( !mDataProvider )
    return;

  QgsFields oldFields = mUpdatedFields;

  mUpdatedFields = mDataProvider->fields();

  // added / removed fields
  if ( mEditBuffer )
    mEditBuffer->updateFields( mUpdatedFields );

  // joined fields
  if ( mJoinBuffer && mJoinBuffer->containsJoins() )
    mJoinBuffer->updateFields( mUpdatedFields );

  if ( mExpressionFieldBuffer )
    mExpressionFieldBuffer->updateFields( mUpdatedFields );

  if ( oldFields != mUpdatedFields )
    emit updatedFields();
}


void QgsVectorLayer::createJoinCaches()
{
  if ( mJoinBuffer->containsJoins() )
  {
    mJoinBuffer->createJoinCaches();
  }
}

void QgsVectorLayer::uniqueValues( int index, QList<QVariant> &uniqueValues, int limit )
{
  uniqueValues.clear();
  if ( !mDataProvider )
  {
    return;
  }

  QgsFields::FieldOrigin origin = mUpdatedFields.fieldOrigin( index );
  if ( origin == QgsFields::OriginUnknown )
  {
    return;
  }

  if ( origin == QgsFields::OriginProvider ) //a provider field
  {
    mDataProvider->uniqueValues( index, uniqueValues, limit );

    if ( mEditBuffer )
    {
      QSet<QString> vals;
      Q_FOREACH ( const QVariant& v, uniqueValues )
      {
        vals << v.toString();
      }

      QMapIterator< QgsFeatureId, QgsAttributeMap > it( mEditBuffer->changedAttributeValues() );
      while ( it.hasNext() && ( limit < 0 || uniqueValues.count() < limit ) )
      {
        it.next();
        QVariant v = it.value().value( index );
        if ( v.isValid() )
        {
          QString vs = v.toString();
          if ( !vals.contains( vs ) )
          {
            vals << vs;
            uniqueValues << v;
          }
        }
      }
    }

    return;
  }
  else if ( origin == QgsFields::OriginJoin )
  {
    int sourceLayerIndex;
    const QgsVectorJoinInfo* join = mJoinBuffer->joinForFieldIndex( index, mUpdatedFields, sourceLayerIndex );
    Q_ASSERT( join );

    QgsVectorLayer *vl = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( join->joinLayerId ) );

    if ( vl )
      vl->dataProvider()->uniqueValues( sourceLayerIndex, uniqueValues, limit );

    return;
  }
  else if ( origin == QgsFields::OriginEdit || origin == QgsFields::OriginExpression )
  {
    // the layer is editable, but in certain cases it can still be avoided going through all features
    if ( origin == QgsFields::OriginEdit && mEditBuffer->mDeletedFeatureIds.isEmpty() && mEditBuffer->mAddedFeatures.isEmpty() && !mEditBuffer->mDeletedAttributeIds.contains( index ) && mEditBuffer->mChangedAttributeValues.isEmpty() )
    {
      mDataProvider->uniqueValues( index, uniqueValues, limit );
      return;
    }

    // we need to go through each feature
    QgsAttributeList attList;
    attList << index;

    QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                          .setFlags( QgsFeatureRequest::NoGeometry )
                                          .setSubsetOfAttributes( attList ) );

    QgsFeature f;
    QVariant currentValue;
    QHash<QString, QVariant> val;
    while ( fit.nextFeature( f ) )
    {
      currentValue = f.attribute( index );
      val.insert( currentValue.toString(), currentValue );
      if ( limit >= 0 && val.size() >= limit )
      {
        break;
      }
    }

    uniqueValues = val.values();
    return;
  }

  Q_ASSERT_X( false, "QgsVectorLayer::uniqueValues()", "Unknown source of the field!" );
}

QVariant QgsVectorLayer::minimumValue( int index )
{
  if ( !mDataProvider )
  {
    return QVariant();
  }

  QgsFields::FieldOrigin origin = mUpdatedFields.fieldOrigin( index );
  if ( origin == QgsFields::OriginUnknown )
  {
    return QVariant();
  }

  if ( origin == QgsFields::OriginProvider ) //a provider field
  {
    return mDataProvider->minimumValue( index );
  }
  else if ( origin == QgsFields::OriginJoin )
  {
    int sourceLayerIndex;
    const QgsVectorJoinInfo* join = mJoinBuffer->joinForFieldIndex( index, mUpdatedFields, sourceLayerIndex );
    Q_ASSERT( join );

    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( join->joinLayerId ) );
    Q_ASSERT( vl );

    return vl->minimumValue( sourceLayerIndex );
  }
  else if ( origin == QgsFields::OriginEdit || origin == QgsFields::OriginExpression )
  {
    // the layer is editable, but in certain cases it can still be avoided going through all features
    if ( origin == QgsFields::OriginEdit &&
         mEditBuffer->mDeletedFeatureIds.isEmpty() &&
         mEditBuffer->mAddedFeatures.isEmpty() && !
         mEditBuffer->mDeletedAttributeIds.contains( index ) &&
         mEditBuffer->mChangedAttributeValues.isEmpty() )
    {
      return mDataProvider->minimumValue( index );
    }

    // we need to go through each feature
    QgsAttributeList attList;
    attList << index;

    QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                          .setFlags( QgsFeatureRequest::NoGeometry )
                                          .setSubsetOfAttributes( attList ) );

    QgsFeature f;
    double minimumValue = std::numeric_limits<double>::max();
    double currentValue = 0;
    while ( fit.nextFeature( f ) )
    {
      currentValue = f.attribute( index ).toDouble();
      if ( currentValue < minimumValue )
      {
        minimumValue = currentValue;
      }
    }
    return QVariant( minimumValue );
  }

  Q_ASSERT_X( false, "QgsVectorLayer::minimumValue()", "Unknown source of the field!" );
  return QVariant();
}

QVariant QgsVectorLayer::maximumValue( int index )
{
  if ( !mDataProvider )
  {
    return QVariant();
  }

  QgsFields::FieldOrigin origin = mUpdatedFields.fieldOrigin( index );
  if ( origin == QgsFields::OriginUnknown )
  {
    return QVariant();
  }

  if ( origin == QgsFields::OriginProvider ) //a provider field
  {
    return mDataProvider->maximumValue( index );
  }
  else if ( origin == QgsFields::OriginJoin )
  {
    int sourceLayerIndex;
    const QgsVectorJoinInfo* join = mJoinBuffer->joinForFieldIndex( index, mUpdatedFields, sourceLayerIndex );
    Q_ASSERT( join );

    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( join->joinLayerId ) );
    Q_ASSERT( vl );

    return vl->maximumValue( sourceLayerIndex );
  }
  else if ( origin == QgsFields::OriginEdit || origin == QgsFields::OriginExpression )
  {
    // the layer is editable, but in certain cases it can still be avoided going through all features
    if ( origin == QgsFields::OriginEdit &&
         mEditBuffer->mDeletedFeatureIds.isEmpty() &&
         mEditBuffer->mAddedFeatures.isEmpty() &&
         !mEditBuffer->mDeletedAttributeIds.contains( index ) &&
         mEditBuffer->mChangedAttributeValues.isEmpty() )
    {
      return mDataProvider->maximumValue( index );
    }

    // we need to go through each feature
    QgsAttributeList attList;
    attList << index;

    QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                          .setFlags( QgsFeatureRequest::NoGeometry )
                                          .setSubsetOfAttributes( attList ) );

    QgsFeature f;
    double maximumValue = -std::numeric_limits<double>::max();
    double currentValue = 0;
    while ( fit.nextFeature( f ) )
    {
      currentValue = f.attribute( index ).toDouble();
      if ( currentValue > maximumValue )
      {
        maximumValue = currentValue;
      }
    }
    return QVariant( maximumValue );
  }

  Q_ASSERT_X( false, "QgsVectorLayer::maximumValue()", "Unknown source of the field!" );
  return QVariant();
}

QList<QVariant> QgsVectorLayer::getValues( const QString &fieldOrExpression, bool& ok, bool selectedOnly )
{
  QList<QVariant> values;

  QScopedPointer<QgsExpression> expression;
  QgsExpressionContext context;

  int attrNum = fieldNameIndex( fieldOrExpression );

  if ( attrNum == -1 )
  {
    // try to use expression
    expression.reset( new QgsExpression( fieldOrExpression ) );
    context << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope()
    << QgsExpressionContextUtils::layerScope( this );

    if ( expression->hasParserError() || !expression->prepare( &context ) )
    {
      ok = false;
      return values;
    }
  }

  QgsFeature f;
  QStringList lst;
  if ( expression.isNull() )
    lst.append( fieldOrExpression );
  else
    lst = expression->referencedColumns();

  QgsFeatureRequest request = QgsFeatureRequest()
                              .setFlags(( expression && expression->needsGeometry() ) ?
                                        QgsFeatureRequest::NoFlags :
                                        QgsFeatureRequest::NoGeometry )
                              .setSubsetOfAttributes( lst, fields() );

  QgsFeatureIterator fit;
  if ( !selectedOnly )
  {
    fit = getFeatures( request );
  }
  else
  {
    fit = selectedFeaturesIterator( request );
  }

  // create list of non-null attribute values
  while ( fit.nextFeature( f ) )
  {
    if ( expression )
    {
      context.setFeature( f );
      QVariant v = expression->evaluate( &context );
      values << v;
    }
    else
    {
      values << f.attribute( attrNum );
    }
  }
  ok = true;
  return values;
}

QList<double> QgsVectorLayer::getDoubleValues( const QString &fieldOrExpression, bool& ok, bool selectedOnly, int* nullCount )
{
  QList<double> values;

  if ( nullCount )
    *nullCount = 0;

  QList<QVariant> variantValues = getValues( fieldOrExpression, ok, selectedOnly );
  if ( !ok )
    return values;

  bool convertOk;
  foreach ( QVariant value, variantValues )
  {
    double val = value.toDouble( &convertOk );
    if ( convertOk )
      values << val;
    else if ( value.isNull() )
    {
      if ( nullCount )
        *nullCount += 1;
    }
  }
  ok = true;
  return values;
}


/** Write blend mode for features */
void QgsVectorLayer::setFeatureBlendMode( const QPainter::CompositionMode &featureBlendMode )
{
  mFeatureBlendMode = featureBlendMode;
  emit featureBlendModeChanged( featureBlendMode );
}

/** Read blend mode for layer */
QPainter::CompositionMode QgsVectorLayer::featureBlendMode() const
{
  return mFeatureBlendMode;
}

/** Write transparency for layer */
void QgsVectorLayer::setLayerTransparency( int layerTransparency )
{
  mLayerTransparency = layerTransparency;
  emit layerTransparencyChanged( layerTransparency );
}

/** Read transparency for layer */
int QgsVectorLayer::layerTransparency() const
{
  return mLayerTransparency;
}



void QgsVectorLayer::readSldLabeling( const QDomNode& node )
{
  QDomElement element = node.toElement();
  if ( element.isNull() )
    return;

  QDomElement userStyleElem = element.firstChildElement( "UserStyle" );
  if ( userStyleElem.isNull() )
  {
    QgsDebugMsg( "Info: UserStyle element not found." );
    return;
  }

  QDomElement featureTypeStyleElem = userStyleElem.firstChildElement( "FeatureTypeStyle" );
  if ( featureTypeStyleElem.isNull() )
  {
    QgsDebugMsg( "Info: FeatureTypeStyle element not found." );
    return;
  }

  // use last rule
  QDomElement ruleElem = featureTypeStyleElem.lastChildElement( "Rule" );
  if ( ruleElem.isNull() )
  {
    QgsDebugMsg( "Info: Rule element not found." );
    return;
  }

  // use last text symbolizer
  QDomElement textSymbolizerElem = ruleElem.lastChildElement( "TextSymbolizer" );
  if ( textSymbolizerElem.isNull() )
  {
    QgsDebugMsg( "Info: TextSymbolizer element not found." );
    return;
  }

  // Label
  setCustomProperty( "labeling/enabled", false );
  QDomElement labelElem = textSymbolizerElem.firstChildElement( "Label" );
  if ( !labelElem.isNull() )
  {
    QDomElement propertyNameElem = labelElem.firstChildElement( "PropertyName" );
    if ( !propertyNameElem.isNull() )
    {
      // enable labeling
      setCustomProperty( "labeling", "pal" );
      setCustomProperty( "labeling/enabled", true );

      // set labeling defaults
      setCustomProperty( "labeling/fontFamily", "Sans-Serif" );
      setCustomProperty( "labeling/fontItalic", false );
      setCustomProperty( "labeling/fontSize", 10 );
      setCustomProperty( "labeling/fontSizeInMapUnits", false );
      setCustomProperty( "labeling/fontBold", false );
      setCustomProperty( "labeling/fontUnderline", false );
      setCustomProperty( "labeling/textColorR", 0 );
      setCustomProperty( "labeling/textColorG", 0 );
      setCustomProperty( "labeling/textColorB", 0 );
      setCustomProperty( "labeling/textTransp", 0 );
      setCustomProperty( "labeling/bufferDraw", false );
      setCustomProperty( "labeling/bufferSize", 1 );
      setCustomProperty( "labeling/bufferSizeInMapUnits", false );
      setCustomProperty( "labeling/bufferColorR", 255 );
      setCustomProperty( "labeling/bufferColorG", 255 );
      setCustomProperty( "labeling/bufferColorB", 255 );
      setCustomProperty( "labeling/bufferTransp", 0 );
      setCustomProperty( "labeling/placement", QgsPalLayerSettings::AroundPoint );
      setCustomProperty( "labeling/xOffset", 0 );
      setCustomProperty( "labeling/yOffset", 0 );
      setCustomProperty( "labeling/labelOffsetInMapUnits", false );
      setCustomProperty( "labeling/angleOffset", 0 );

      // label attribute
      QString labelAttribute = propertyNameElem.text();
      setCustomProperty( "labeling/fieldName", labelAttribute );
      setCustomProperty( "labeling/isExpression", false );

      int fieldIndex = fieldNameIndex( labelAttribute );
      if ( fieldIndex == -1 )
      {
        // label attribute is not in columns, check if it is an expression
        QgsExpression exp( labelAttribute );
        if ( !exp.hasEvalError() )
        {
          setCustomProperty( "labeling/isExpression", true );
        }
        else
        {
          QgsDebugMsg( "SLD label attribute error: " + exp.evalErrorString() );
        }
      }
    }
    else
    {
      QgsDebugMsg( "Info: PropertyName element not found." );
      return;
    }
  }
  else
  {
    QgsDebugMsg( "Info: Label element not found." );
    return;
  }

  // Font
  QDomElement fontElem = textSymbolizerElem.firstChildElement( "Font" );
  if ( !fontElem.isNull() )
  {
    QString cssName;
    QString elemText;
    QDomElement cssElem = fontElem.firstChildElement( "CssParameter" );
    while ( !cssElem.isNull() )
    {
      cssName = cssElem.attribute( "name", "not_found" );
      if ( cssName != "not_found" )
      {
        elemText = cssElem.text();
        if ( cssName == "font-family" )
        {
          setCustomProperty( "labeling/fontFamily", elemText );
        }
        else if ( cssName == "font-style" )
        {
          setCustomProperty( "labeling/fontItalic", ( elemText == "italic" ) || ( elemText == "Italic" ) );
        }
        else if ( cssName == "font-size" )
        {
          bool ok;
          int fontSize = elemText.toInt( &ok );
          if ( ok )
          {
            setCustomProperty( "labeling/fontSize", fontSize );
          }
        }
        else if ( cssName == "font-weight" )
        {
          setCustomProperty( "labeling/fontBold", ( elemText == "bold" ) || ( elemText == "Bold" ) );
        }
        else if ( cssName == "font-underline" )
        {
          setCustomProperty( "labeling/fontUnderline", ( elemText == "underline" ) || ( elemText == "Underline" ) );
        }
      }

      cssElem = cssElem.nextSiblingElement( "CssParameter" );
    }
  }

  // Fill
  QColor textColor = QgsOgcUtils::colorFromOgcFill( textSymbolizerElem.firstChildElement( "Fill" ) );
  if ( textColor.isValid() )
  {
    setCustomProperty( "labeling/textColorR", textColor.red() );
    setCustomProperty( "labeling/textColorG", textColor.green() );
    setCustomProperty( "labeling/textColorB", textColor.blue() );
    setCustomProperty( "labeling/textTransp", 100 - ( int )( 100 * textColor.alphaF() ) );
  }

  // Halo
  QDomElement haloElem = textSymbolizerElem.firstChildElement( "Halo" );
  if ( !haloElem.isNull() )
  {
    setCustomProperty( "labeling/bufferDraw", true );
    setCustomProperty( "labeling/bufferSize", 1 );

    QDomElement radiusElem = haloElem.firstChildElement( "Radius" );
    if ( !radiusElem.isNull() )
    {
      bool ok;
      double bufferSize = radiusElem.text().toDouble( &ok );
      if ( ok )
      {
        setCustomProperty( "labeling/bufferSize", bufferSize );
      }
    }

    QColor bufferColor = QgsOgcUtils::colorFromOgcFill( haloElem.firstChildElement( "Fill" ) );
    if ( bufferColor.isValid() )
    {
      setCustomProperty( "labeling/bufferColorR", bufferColor.red() );
      setCustomProperty( "labeling/bufferColorG", bufferColor.green() );
      setCustomProperty( "labeling/bufferColorB", bufferColor.blue() );
      setCustomProperty( "labeling/bufferTransp", 100 - ( int )( 100 * bufferColor.alphaF() ) );
    }
  }

  // LabelPlacement
  QDomElement labelPlacementElem = textSymbolizerElem.firstChildElement( "LabelPlacement" );
  if ( !labelPlacementElem.isNull() )
  {
    // PointPlacement
    QDomElement pointPlacementElem = labelPlacementElem.firstChildElement( "PointPlacement" );
    if ( !pointPlacementElem.isNull() )
    {
      setCustomProperty( "labeling/placement", QgsPalLayerSettings::OverPoint );

      QDomElement displacementElem = pointPlacementElem.firstChildElement( "Displacement" );
      if ( !displacementElem.isNull() )
      {
        QDomElement displacementXElem = displacementElem.firstChildElement( "DisplacementX" );
        if ( !displacementXElem.isNull() )
        {
          bool ok;
          double xOffset = displacementXElem.text().toDouble( &ok );
          if ( ok )
          {
            setCustomProperty( "labeling/xOffset", xOffset );
          }
        }
        QDomElement displacementYElem = displacementElem.firstChildElement( "DisplacementY" );
        if ( !displacementYElem.isNull() )
        {
          bool ok;
          double yOffset = displacementYElem.text().toDouble( &ok );
          if ( ok )
          {
            setCustomProperty( "labeling/yOffset", yOffset );
          }
        }
      }

      QDomElement rotationElem = pointPlacementElem.firstChildElement( "Rotation" );
      if ( !rotationElem.isNull() )
      {
        bool ok;
        double rotation = rotationElem.text().toDouble( &ok );
        if ( ok )
        {
          setCustomProperty( "labeling/angleOffset", rotation );
        }
      }
    }
  }
}

void QgsVectorLayer::setDiagramLayerSettings( const QgsDiagramLayerSettings& s )
{
  if ( !mDiagramLayerSettings )
    mDiagramLayerSettings = new QgsDiagramLayerSettings();
  *mDiagramLayerSettings = s;
}

QString QgsVectorLayer::metadata()
{
  QString myMetadata = "<html><body>";

  //-------------

  myMetadata += "<p class=\"subheaderglossy\">";
  myMetadata += tr( "General" );
  myMetadata += "</p>\n";

  // data comment
  if ( !( dataComment().isEmpty() ) )
  {
    myMetadata += "<p class=\"glossy\">" + tr( "Layer comment" ) + "</p>\n";
    myMetadata += "<p>";
    myMetadata += dataComment();
    myMetadata += "</p>\n";
  }

  //storage type
  myMetadata += "<p class=\"glossy\">" + tr( "Storage type of this layer" ) + "</p>\n";
  myMetadata += "<p>";
  myMetadata += storageType();
  myMetadata += "</p>\n";

  if ( dataProvider() )
  {
    //provider description
    myMetadata += "<p class=\"glossy\">" + tr( "Description of this provider" ) + "</p>\n";
    myMetadata += "<p>";
    myMetadata += dataProvider()->description().replace( "\n", "<br>" );
    myMetadata += "</p>\n";
  }

  // data source
  myMetadata += "<p class=\"glossy\">" + tr( "Source for this layer" ) + "</p>\n";
  myMetadata += "<p>";
  myMetadata += publicSource();
  myMetadata += "</p>\n";

  //geom type

  QGis::GeometryType type = geometryType();

  if ( type < 0 || type > QGis::NoGeometry )
  {
    QgsDebugMsg( "Invalid vector type" );
  }
  else
  {
    QString typeString( QGis::vectorGeometryType( geometryType() ) );

    myMetadata += "<p class=\"glossy\">" + tr( "Geometry type of the features in this layer" ) + "</p>\n";
    myMetadata += "<p>";
    myMetadata += typeString;
    myMetadata += "</p>\n";
  }

  QgsAttributeList pkAttrList = pkAttributeList();
  if ( !pkAttrList.isEmpty() )
  {
    myMetadata += "<p class=\"glossy\">" + tr( "Primary key attributes" ) + "</p>\n";
    myMetadata += "<p>";
    foreach ( int idx, pkAttrList )
    {
      myMetadata += fields()[ idx ].name() + " ";
    }
    myMetadata += "</p>\n";
  }


  //feature count
  myMetadata += "<p class=\"glossy\">" + tr( "The number of features in this layer" ) + "</p>\n";
  myMetadata += "<p>";
  myMetadata += QString::number( featureCount() );
  myMetadata += "</p>\n";
  //capabilities
  myMetadata += "<p class=\"glossy\">" + tr( "Editing capabilities of this layer" ) + "</p>\n";
  myMetadata += "<p>";
  myMetadata += capabilitiesString();
  myMetadata += "</p>\n";

  //-------------

  QgsRectangle myExtent = extent();
  myMetadata += "<p class=\"subheaderglossy\">";
  myMetadata += tr( "Extents" );
  myMetadata += "</p>\n";

  //extents in layer cs  TODO...maybe make a little nested table to improve layout...
  myMetadata += "<p class=\"glossy\">" + tr( "In layer spatial reference system units" ) + "</p>\n";
  myMetadata += "<p>";
  // Try to be a bit clever over what number format we use for the
  // extents. Some people don't like it using scientific notation when the
  // numbers get large, but for small numbers this is the more practical
  // option (so we can't force the format to 'f' for all values).
  // The scheme:
  // - for all numbers with more than 5 digits, force non-scientific notation
  // and 2 digits after the decimal point.
  // - for all smaller numbers let the OS decide which format to use (it will
  // generally use non-scientific unless the number gets much less than 1).

  if ( !myExtent.isEmpty() )
  {
    QString xMin, yMin, xMax, yMax;
    double changeoverValue = 99999; // The 'largest' 5 digit number
    if ( qAbs( myExtent.xMinimum() ) > changeoverValue )
    {
      xMin = QString( "%1" ).arg( myExtent.xMinimum(), 0, 'f', 2 );
    }
    else
    {
      xMin = QString( "%1" ).arg( myExtent.xMinimum() );
    }
    if ( qAbs( myExtent.yMinimum() ) > changeoverValue )
    {
      yMin = QString( "%1" ).arg( myExtent.yMinimum(), 0, 'f', 2 );
    }
    else
    {
      yMin = QString( "%1" ).arg( myExtent.yMinimum() );
    }
    if ( qAbs( myExtent.xMaximum() ) > changeoverValue )
    {
      xMax = QString( "%1" ).arg( myExtent.xMaximum(), 0, 'f', 2 );
    }
    else
    {
      xMax = QString( "%1" ).arg( myExtent.xMaximum() );
    }
    if ( qAbs( myExtent.yMaximum() ) > changeoverValue )
    {
      yMax = QString( "%1" ).arg( myExtent.yMaximum(), 0, 'f', 2 );
    }
    else
    {
      yMax = QString( "%1" ).arg( myExtent.yMaximum() );
    }

    myMetadata += tr( "xMin,yMin %1,%2 : xMax,yMax %3,%4" )
                  .arg( xMin ).arg( yMin ).arg( xMax ).arg( yMax );
  }
  else
  {
    myMetadata += tr( "unknown extent" );
  }

  myMetadata += "</p>\n";

  //extents in project cs

  try
  {
#if 0
    // TODO: currently disabled, will revisit later [MD]
    QgsRectangle myProjectedExtent = coordinateTransform->transformBoundingBox( extent() );
    myMetadata += "<p class=\"glossy\">" + tr( "In project spatial reference system units" ) + "</p>\n";
    myMetadata += "<p>";
    myMetadata += tr( "xMin,yMin %1,%2 : xMax,yMax %3,%4" )
                  .arg( myProjectedExtent.xMinimum() )
                  .arg( myProjectedExtent.yMinimum() )
                  .arg( myProjectedExtent.xMaximum() )
                  .arg( myProjectedExtent.yMaximum() );
    myMetadata += "</p>\n";
#endif

    //
    // Display layer spatial ref system
    //
    myMetadata += "<p class=\"glossy\">" + tr( "Layer Spatial Reference System" ) + "</p>\n";
    myMetadata += "<p>";
    myMetadata += crs().toProj4().replace( QRegExp( "\"" ), " \"" );
    myMetadata += "</p>\n";

    //
    // Display project (output) spatial ref system
    //
#if 0
    // TODO: disabled for now, will revisit later [MD]
    //myMetadata += "<tr><td bgcolor=\"gray\">";
    myMetadata += "<p class=\"glossy\">" + tr( "Project (Output) Spatial Reference System" ) + "</p>\n";
    myMetadata += "<p>";
    myMetadata += coordinateTransform->destCRS().toProj4().replace( QRegExp( "\"" ), " \"" );
    myMetadata += "</p>\n";
#endif
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( cse.what() );

    myMetadata += "<p class=\"glossy\">" + tr( "In project spatial reference system units" ) + "</p>\n";
    myMetadata += "<p>";
    myMetadata += tr( "(Invalid transformation of layer extents)" );
    myMetadata += "</p>\n";

  }

#if 0
  //
  // Add the info about each field in the attribute table
  //
  myMetadata += "<p class=\"glossy\">" + tr( "Attribute field info" ) + "</p>\n";
  myMetadata += "<p>";

  // Start a nested table in this trow
  myMetadata += "<table width=\"100%\">";
  myMetadata += "<tr><th>";
  myMetadata += tr( "Field" );
  myMetadata += "</th>";
  myMetadata += "<th>";
  myMetadata += tr( "Type" );
  myMetadata += "</th>";
  myMetadata += "<th>";
  myMetadata += tr( "Length" );
  myMetadata += "</th>";
  myMetadata += "<th>";
  myMetadata += tr( "Precision" );
  myMetadata += "</th>";
  myMetadata += "<th>";
  myMetadata += tr( "Comment" );
  myMetadata += "</th>";

  //get info for each field by looping through them
  const QgsFields& myFields = pendingFields();
  for ( int i = 0, n = myFields.size(); i < n; ++i )
  {
    const QgsField& myField = fields[i];

    myMetadata += "<tr><td>";
    myMetadata += myField.name();
    myMetadata += "</td>";
    myMetadata += "<td>";
    myMetadata += myField.typeName();
    myMetadata += "</td>";
    myMetadata += "<td>";
    myMetadata += QString( "%1" ).arg( myField.length() );
    myMetadata += "</td>";
    myMetadata += "<td>";
    myMetadata += QString( "%1" ).arg( myField.precision() );
    myMetadata += "</td>";
    myMetadata += "<td>";
    myMetadata += QString( "%1" ).arg( myField.comment() );
    myMetadata += "</td></tr>";
  }

  //close field list
  myMetadata += "</table>"; //end of nested table
#endif

  myMetadata += "</body></html>";
  return myMetadata;
}

void QgsVectorLayer::invalidateSymbolCountedFlag()
{
  mSymbolFeatureCounted = false;
}

void QgsVectorLayer::onRelationsLoaded()
{
  Q_FOREACH ( QgsAttributeEditorElement* elem, mAttributeEditorElements )
  {
    if ( elem->type() == QgsAttributeEditorElement::AeTypeContainer )
    {
      QgsAttributeEditorContainer* cont = dynamic_cast< QgsAttributeEditorContainer* >( elem );
      if ( !cont )
        continue;

      QList<QgsAttributeEditorElement*> relations = cont->findElements( QgsAttributeEditorElement::AeTypeRelation );
      Q_FOREACH ( QgsAttributeEditorElement* relElem, relations )
      {
        QgsAttributeEditorRelation* rel = dynamic_cast< QgsAttributeEditorRelation* >( relElem );
        if ( !rel )
          continue;

        rel->init( QgsProject::instance()->relationManager() );
      }
    }
  }
}

void QgsVectorLayer::onJoinedFieldsChanged()
{
  // some of the fields of joined layers have changed -> we need to update this layer's fields too
  updateFields();
}

void QgsVectorLayer::onFeatureDeleted( const QgsFeatureId& fid )
{
  if ( mEditCommandActive )
    mDeletedFids << fid;
  else
    emit featuresDeleted( QgsFeatureIds() << fid );

  emit featureDeleted( fid );
}

QgsVectorLayer::ValueRelationData QgsVectorLayer::valueRelation( int idx )
{
  if ( editorWidgetV2( idx ) == "ValueRelation" )
  {
    QgsEditorWidgetConfig cfg = editorWidgetV2Config( idx );

    return ValueRelationData( cfg.value( "Layer" ).toString(),
                              cfg.value( "Key" ).toString(),
                              cfg.value( "Value" ).toString(),
                              cfg.value( "AllowNull" ).toBool(),
                              cfg.value( "OrderByValue" ).toBool(),
                              cfg.value( "AllowMulti" ).toBool(),
                              cfg.value( "FilterExpression" ).toString()
                            );
  }
  else
  {
    return ValueRelationData();
  }
}

QList<QgsRelation> QgsVectorLayer::referencingRelations( int idx )
{
  return QgsProject::instance()->relationManager()->referencingRelations( this, idx );
}

QList<QgsAttributeEditorElement*> &QgsVectorLayer::attributeEditorElements()
{
  return mAttributeEditorElements;
}

void QgsVectorLayer::clearAttributeEditorWidgets()
{
  mAttributeEditorElements.clear();
}

QDomElement QgsAttributeEditorContainer::toDomElement( QDomDocument& doc ) const
{
  QDomElement elem = doc.createElement( "attributeEditorContainer" );
  elem.setAttribute( "name", mName );

  Q_FOREACH ( QgsAttributeEditorElement* child, mChildren )
  {
    elem.appendChild( child->toDomElement( doc ) );
  }
  return elem;
}

void QgsAttributeEditorContainer::addChildElement( QgsAttributeEditorElement *widget )
{
  mChildren.append( widget );
}

void QgsAttributeEditorContainer::setName( const QString& name )
{
  mName = name;
}

QList<QgsAttributeEditorElement*> QgsAttributeEditorContainer::findElements( QgsAttributeEditorElement::AttributeEditorType type ) const
{
  QList<QgsAttributeEditorElement*> results;

  Q_FOREACH ( QgsAttributeEditorElement* elem, mChildren )
  {
    if ( elem->type() == type )
    {
      results.append( elem );
    }

    if ( elem->type() == AeTypeContainer )
    {
      QgsAttributeEditorContainer* cont = dynamic_cast<QgsAttributeEditorContainer*>( elem );
      if ( cont )
        results += cont->findElements( type );
    }
  }

  return results;
}

QDomElement QgsAttributeEditorField::toDomElement( QDomDocument& doc ) const
{
  QDomElement elem = doc.createElement( "attributeEditorField" );
  elem.setAttribute( "name", mName );
  elem.setAttribute( "index", mIdx );
  return elem;
}

int QgsVectorLayer::listStylesInDatabase( QStringList &ids, QStringList &names, QStringList &descriptions, QString &msgError )
{
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QLibrary *myLib = pReg->providerLibrary( mProviderKey );
  if ( !myLib )
  {
    msgError = QObject::tr( "Unable to load %1 provider" ).arg( mProviderKey );
    return -1;
  }
  listStyles_t* listStylesExternalMethod = ( listStyles_t * ) cast_to_fptr( myLib->resolve( "listStyles" ) );

  if ( !listStylesExternalMethod )
  {
    delete myLib;
    msgError = QObject::tr( "Provider %1 has no %2 method" ).arg( mProviderKey ).arg( "listStyles" );
    return -1;
  }

  return listStylesExternalMethod( mDataSource, ids, names, descriptions, msgError );
}

QString QgsVectorLayer::getStyleFromDatabase( QString styleId, QString &msgError )
{
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QLibrary *myLib = pReg->providerLibrary( mProviderKey );
  if ( !myLib )
  {
    msgError = QObject::tr( "Unable to load %1 provider" ).arg( mProviderKey );
    return QObject::tr( "" );
  }
  getStyleById_t* getStyleByIdMethod = ( getStyleById_t * ) cast_to_fptr( myLib->resolve( "getStyleById" ) );

  if ( !getStyleByIdMethod )
  {
    delete myLib;
    msgError = QObject::tr( "Provider %1 has no %2 method" ).arg( mProviderKey ).arg( "getStyleById" );
    return QObject::tr( "" );
  }

  return getStyleByIdMethod( mDataSource, styleId, msgError );
}


void QgsVectorLayer::saveStyleToDatabase( QString name, QString description,
    bool useAsDefault, QString uiFileContent,  QString &msgError )
{

  QString sldStyle, qmlStyle;
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QLibrary *myLib = pReg->providerLibrary( mProviderKey );
  if ( !myLib )
  {
    msgError = QObject::tr( "Unable to load %1 provider" ).arg( mProviderKey );
    return;
  }
  saveStyle_t* saveStyleExternalMethod = ( saveStyle_t * ) cast_to_fptr( myLib->resolve( "saveStyle" ) );

  if ( !saveStyleExternalMethod )
  {
    delete myLib;
    msgError = QObject::tr( "Provider %1 has no %2 method" ).arg( mProviderKey ).arg( "saveStyle" );
    return;
  }

  QDomDocument qmlDocument, sldDocument;
  this->exportNamedStyle( qmlDocument, msgError );
  if ( !msgError.isNull() )
  {
    return;
  }
  qmlStyle = qmlDocument.toString();

  this->exportSldStyle( sldDocument, msgError );
  if ( !msgError.isNull() )
  {
    return;
  }
  sldStyle = sldDocument.toString();

  saveStyleExternalMethod( mDataSource, qmlStyle, sldStyle, name,
                           description, uiFileContent, useAsDefault, msgError );
}



QString QgsVectorLayer::loadNamedStyle( const QString &theURI, bool &theResultFlag )
{
  return loadNamedStyle( theURI, theResultFlag, false );
}

QString QgsVectorLayer::loadNamedStyle( const QString &theURI, bool &theResultFlag, bool loadFromLocalDB )
{
  QgsDataSourceURI dsUri( theURI );
  if ( !loadFromLocalDB && !dsUri.database().isEmpty() )
  {
    QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
    QLibrary *myLib = pReg->providerLibrary( mProviderKey );
    if ( myLib )
    {
      loadStyle_t* loadStyleExternalMethod = ( loadStyle_t * ) cast_to_fptr( myLib->resolve( "loadStyle" ) );
      if ( loadStyleExternalMethod )
      {
        QString qml, errorMsg;
        qml = loadStyleExternalMethod( mDataSource, errorMsg );
        if ( !qml.isEmpty() )
        {
          theResultFlag = this->applyNamedStyle( qml, errorMsg );
          return QObject::tr( "Loaded from Provider" );
        }
      }
    }
  }

  return QgsMapLayer::loadNamedStyle( theURI, theResultFlag );
}

bool QgsVectorLayer::applyNamedStyle( QString namedStyle, QString& errorMsg )
{
  QDomDocument myDocument( "qgis" );
  myDocument.setContent( namedStyle );

  return importNamedStyle( myDocument, errorMsg );
}


QDomElement QgsAttributeEditorRelation::toDomElement( QDomDocument& doc ) const
{
  QDomElement elem = doc.createElement( "attributeEditorRelation" );
  elem.setAttribute( "name", mName );
  elem.setAttribute( "relation", mRelation.id() );
  return elem;
}

bool QgsAttributeEditorRelation::init( QgsRelationManager* relationManager )
{
  mRelation = relationManager->relation( mRelationId );
  return mRelation.isValid();
}
