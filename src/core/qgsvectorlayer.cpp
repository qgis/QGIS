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
#include <QString>
#include <QDomNode>
#include <QVector>
#include <QStringBuilder>

#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsactionmanager.h"
#include "qgis.h" //for globals
#include "qgsapplication.h"
#include "qgsclipper.h"
#include "qgsconditionalstyle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgscsexception.h"
#include "qgscurve.h"
#include "qgsdatasourceuri.h"
#include "qgsexpressionfieldbuffer.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfields.h"
#include "qgsgeometrycache.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaptopixel.h"
#include "qgsmessagelog.h"
#include "qgsogcutils.h"
#include "qgspainting.h"
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
#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerrenderer.h"
#include "qgsvectorlayerundocommand.h"
#include "qgspointv2.h"
#include "qgsrenderer.h"
#include "qgssymbollayer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsdiagramrenderer.h"
#include "qgsstyle.h"
#include "qgspallabeling.h"
#include "qgssimplifymethod.h"
#include "qgsexpressioncontext.h"
#include "qgsfeedback.h"
#include "qgsxmlutils.h"
#include "qgsunittypes.h"

#include "diagram/qgsdiagram.h"

#ifdef TESTPROVIDERLIB
#include <dlfcn.h>
#endif

typedef bool saveStyle_t(
  const QString &uri,
  const QString &qmlStyle,
  const QString &sldStyle,
  const QString &styleName,
  const QString &styleDescription,
  const QString &uiFileContent,
  bool useAsDefault,
  QString &errCause
);

typedef QString loadStyle_t(
  const QString &uri,
  QString &errCause
);

typedef int listStyles_t(
  const QString &uri,
  QStringList &ids,
  QStringList &names,
  QStringList &descriptions,
  QString &errCause
);

typedef QString getStyleById_t(
  const QString &uri,
  QString styleID,
  QString &errCause
);

typedef bool deleteStyleById_t(
  const QString &uri,
  QString styleID,
  QString &errCause
);

QgsVectorLayer::QgsVectorLayer( const QString &vectorLayerPath,
                                const QString &baseName,
                                const QString &providerKey,
                                bool loadDefaultStyleFlag )
  : QgsMapLayer( VectorLayer, baseName, vectorLayerPath )
  , mDataProvider( nullptr )
  , mProviderKey( providerKey )
  , mReadOnly( false )
  , mWkbType( QgsWkbTypes::Unknown )
  , mRenderer( nullptr )
  , mLabeling( new QgsVectorLayerSimpleLabeling )
  , mLabelFontNotFoundNotified( false )
  , mFeatureBlendMode( QPainter::CompositionMode_SourceOver ) // Default to normal feature blending
  , mLayerTransparency( 0 )
  , mVertexMarkerOnlyForSelection( false )
  , mCache( new QgsGeometryCache() )
  , mEditBuffer( nullptr )
  , mJoinBuffer( nullptr )
  , mExpressionFieldBuffer( nullptr )
  , mDiagramRenderer( nullptr )
  , mDiagramLayerSettings( nullptr )
  , mValidExtent( false )
  , mLazyExtent( true )
  , mSymbolFeatureCounted( false )
  , mEditCommandActive( false )

{
  mActions = new QgsActionManager( this );
  mConditionalStyles = new QgsConditionalLayerStyles();

  mJoinBuffer = new QgsVectorLayerJoinBuffer( this );
  connect( mJoinBuffer, &QgsVectorLayerJoinBuffer::joinedFieldsChanged, this, &QgsVectorLayer::onJoinedFieldsChanged );

  // if we're given a provider type, try to create and bind one to this layer
  if ( !vectorLayerPath.isEmpty() && !mProviderKey.isEmpty() )
  {
    setDataSource( vectorLayerPath, baseName, providerKey, loadDefaultStyleFlag );
  }

  connect( this, &QgsVectorLayer::selectionChanged, this, [ = ] { emit repaintRequested(); } );
  connect( QgsProject::instance()->relationManager(), &QgsRelationManager::relationsLoaded, this, &QgsVectorLayer::onRelationsLoaded );

  // Default simplify drawing settings
  QgsSettings settings;
  mSimplifyMethod.setSimplifyHints( static_cast< QgsVectorSimplifyMethod::SimplifyHints >( settings.value( QStringLiteral( "qgis/simplifyDrawingHints" ), static_cast< int>( mSimplifyMethod.simplifyHints() ) ).toInt() ) );
  mSimplifyMethod.setSimplifyAlgorithm( static_cast< QgsVectorSimplifyMethod::SimplifyAlgorithm >( settings.value( QStringLiteral( "qgis/simplifyAlgorithm" ), static_cast< int>( mSimplifyMethod.simplifyAlgorithm() ) ).toInt() ) );
  mSimplifyMethod.setThreshold( settings.value( QStringLiteral( "qgis/simplifyDrawingTol" ), mSimplifyMethod.threshold() ).toFloat() );
  mSimplifyMethod.setForceLocalOptimization( settings.value( QStringLiteral( "qgis/simplifyLocal" ), mSimplifyMethod.forceLocalOptimization() ).toBool() );
  mSimplifyMethod.setMaximumScale( settings.value( QStringLiteral( "qgis/simplifyMaxScale" ), mSimplifyMethod.maximumScale() ).toFloat() );
} // QgsVectorLayer ctor



QgsVectorLayer::~QgsVectorLayer()
{
  emit willBeDeleted();

  mValid = false;

  delete mDataProvider;
  delete mEditBuffer;
  delete mJoinBuffer;
  delete mExpressionFieldBuffer;
  delete mCache;
  delete mLabeling;
  delete mDiagramLayerSettings;
  delete mDiagramRenderer;

  delete mActions;

  delete mRenderer;
  delete mConditionalStyles;
}

QString QgsVectorLayer::storageType() const
{
  if ( mDataProvider )
  {
    return mDataProvider->storageType();
  }
  return QString();
}


QString QgsVectorLayer::capabilitiesString() const
{
  if ( mDataProvider )
  {
    return mDataProvider->capabilitiesString();
  }
  return QString();
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

void QgsVectorLayer::reload()
{
  if ( mDataProvider )
  {
    mDataProvider->reloadData();
    updateFields();
  }
}

QgsMapLayerRenderer *QgsVectorLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  return new QgsVectorLayerRenderer( this, rendererContext );
}


void QgsVectorLayer::drawVertexMarker( double x, double y, QPainter &p, QgsVectorLayer::VertexMarkerType type, int m )
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

void QgsVectorLayer::select( QgsFeatureId fid )
{
  mSelectedFeatureIds.insert( fid );

  emit selectionChanged( QgsFeatureIds() << fid, QgsFeatureIds(), false );
}

void QgsVectorLayer::select( const QgsFeatureIds &featureIds )
{
  mSelectedFeatureIds.unite( featureIds );

  emit selectionChanged( featureIds, QgsFeatureIds(), false );
}

void QgsVectorLayer::deselect( const QgsFeatureId fid )
{
  mSelectedFeatureIds.remove( fid );

  emit selectionChanged( QgsFeatureIds(), QgsFeatureIds() << fid, false );
}

void QgsVectorLayer::deselect( const QgsFeatureIds &featureIds )
{
  mSelectedFeatureIds.subtract( featureIds );

  emit selectionChanged( QgsFeatureIds(), featureIds, false );
}

void QgsVectorLayer::selectByRect( QgsRectangle &rect, QgsVectorLayer::SelectBehavior behavior )
{
  // normalize the rectangle
  rect.normalize();

  QgsFeatureIds newSelection;

  QgsFeatureIterator features = getFeatures( QgsFeatureRequest()
                                .setFilterRect( rect )
                                .setFlags( QgsFeatureRequest::ExactIntersect | QgsFeatureRequest::NoGeometry )
                                .setSubsetOfAttributes( QgsAttributeList() ) );

  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    newSelection << feat.id();
  }
  features.close();

  selectByIds( newSelection, behavior );
}

void QgsVectorLayer::selectByExpression( const QString &expression, QgsVectorLayer::SelectBehavior behavior )
{
  QgsFeatureIds newSelection;

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( this ) );

  if ( behavior == SetSelection || behavior == AddToSelection )
  {
    QgsFeatureRequest request = QgsFeatureRequest().setFilterExpression( expression )
                                .setExpressionContext( context )
                                .setFlags( QgsFeatureRequest::NoGeometry )
                                .setSubsetOfAttributes( QgsAttributeList() );

    QgsFeatureIterator features = getFeatures( request );

    if ( behavior == AddToSelection )
    {
      newSelection = selectedFeatureIds();
    }
    QgsFeature feat;
    while ( features.nextFeature( feat ) )
    {
      newSelection << feat.id();
    }
    features.close();
  }
  else if ( behavior == IntersectSelection || behavior == RemoveFromSelection )
  {
    QgsExpression exp( expression );
    exp.prepare( &context );

    QgsFeatureIds oldSelection = selectedFeatureIds();
    QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( oldSelection );

    //refine request
    if ( !exp.needsGeometry() )
      request.setFlags( QgsFeatureRequest::NoGeometry );
    request.setSubsetOfAttributes( exp.referencedColumns(), fields() );

    QgsFeatureIterator features = getFeatures( request );
    QgsFeature feat;
    while ( features.nextFeature( feat ) )
    {
      context.setFeature( feat );
      bool matches = exp.evaluate( &context ).toBool();

      if ( matches && behavior == IntersectSelection )
      {
        newSelection << feat.id();
      }
      else if ( !matches && behavior == RemoveFromSelection )
      {
        newSelection << feat.id();
      }
    }
  }

  selectByIds( newSelection );
}

void QgsVectorLayer::selectByIds( const QgsFeatureIds &ids, QgsVectorLayer::SelectBehavior behavior )
{
  QgsFeatureIds newSelection;

  switch ( behavior )
  {
    case SetSelection:
      newSelection = ids;
      break;

    case AddToSelection:
      newSelection = mSelectedFeatureIds + ids;
      break;

    case RemoveFromSelection:
      newSelection = mSelectedFeatureIds - ids;
      break;

    case IntersectSelection:
      newSelection = mSelectedFeatureIds.intersect( ids );
      break;
  }

  QgsFeatureIds deselectedFeatures = mSelectedFeatureIds - newSelection;
  mSelectedFeatureIds = newSelection;

  emit selectionChanged( newSelection, deselectedFeatures, true );
}

void QgsVectorLayer::modifySelection( const QgsFeatureIds &selectIds, const QgsFeatureIds &deselectIds )
{
  QgsFeatureIds intersectingIds = selectIds & deselectIds;
  if ( !intersectingIds.isEmpty() )
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
  selectByIds( ids );
}

void QgsVectorLayer::selectAll()
{
  selectByIds( allFeatureIds() );
}

QgsFeatureIds QgsVectorLayer::allFeatureIds() const
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

void QgsVectorLayer::invertSelectionInRectangle( QgsRectangle &rect )
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
  if ( mSelectedFeatureIds.isEmpty() )
    return;

  selectByIds( QgsFeatureIds() );
}

QgsVectorDataProvider *QgsVectorLayer::dataProvider()
{
  return mDataProvider;
}

const QgsVectorDataProvider *QgsVectorLayer::dataProvider() const
{
  return mDataProvider;
}

void QgsVectorLayer::setProviderEncoding( const QString &encoding )
{
  if ( mValid && mDataProvider && mDataProvider->encoding() != encoding )
  {
    mDataProvider->setEncoding( encoding );
    updateFields();
  }
}

void QgsVectorLayer::setDiagramRenderer( QgsDiagramRenderer *r )
{
  delete mDiagramRenderer;
  mDiagramRenderer = r;
  emit rendererChanged();
  emit styleChanged();
}

QgsWkbTypes::GeometryType QgsVectorLayer::geometryType() const
{
  if ( mValid && mDataProvider )
  {
    return QgsWkbTypes::geometryType( mDataProvider->wkbType() );
  }
  else
  {
    QgsDebugMsg( "invalid layer or pointer to mDataProvider is null" );
  }

  // We shouldn't get here, and if we have, other things are likely to
  // go wrong. Code that uses the type() return value should be
  // rewritten to cope with a value of Qgis::Unknown. To make this
  // need known, the following message is printed every time we get
  // here.
  QgsDebugMsg( "WARNING: This code should never be reached. Problems may occur..." );

  return QgsWkbTypes::UnknownGeometry;
}

bool QgsVectorLayer::hasGeometryType() const
{
  QgsWkbTypes::GeometryType t = geometryType();
  return t != QgsWkbTypes::NullGeometry && t != QgsWkbTypes::UnknownGeometry;
}

QgsWkbTypes::Type QgsVectorLayer::wkbType() const
{
  return mWkbType;
}

QgsRectangle QgsVectorLayer::boundingBoxOfSelected() const
{
  if ( !mValid || mSelectedFeatureIds.isEmpty() ) //no selected features
  {
    return QgsRectangle( 0, 0, 0, 0 );
  }

  QgsRectangle r, retval;
  retval.setMinimal();

  QgsFeature fet;
  if ( mDataProvider->capabilities() & QgsVectorDataProvider::SelectAtId )
  {
    QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                          .setFilterFids( mSelectedFeatureIds )
                                          .setSubsetOfAttributes( QgsAttributeList() ) );

    while ( fit.nextFeature( fet ) )
    {
      if ( !fet.hasGeometry() )
        continue;
      r = fet.geometry().boundingBox();
      retval.combineExtentWith( r );
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
        if ( fet.hasGeometry() )
        {
          r = fet.geometry().boundingBox();
          retval.combineExtentWith( r );
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
  if ( !mLabeling )
    return false;

  // for simple labeling the mode can be "no labels" - so we need to check
  // in properties whether we are really enabled or not
  if ( mLabeling->type() == QLatin1String( "simple" ) )
    return customProperty( QStringLiteral( "labeling/enabled" ), QVariant( false ) ).toBool();

  // for other labeling implementations we always assume that labeling is enabled
  return true;
}

bool QgsVectorLayer::diagramsEnabled() const
{
  if ( !mDiagramRenderer || !mDiagramLayerSettings )
    return false;

  QList<QgsDiagramSettings> settingList = mDiagramRenderer->diagramSettings();
  if ( !settingList.isEmpty() )
  {
    return settingList.at( 0 ).enabled;
  }
  return false;
}

long QgsVectorLayer::featureCount( const QString &legendKey ) const
{
  if ( !mSymbolFeatureCounted )
    return -1;

  return mSymbolFeatureCountMap.value( legendKey );
}

/** \ingroup core
 * Used by QgsVectorLayer::countSymbolFeatures() to provide an interruption checker
 *  @note not available in Python bindings
 */
class QgsVectorLayerInterruptionCheckerDuringCountSymbolFeatures: public QgsInterruptionChecker
{
  public:

    //! Constructor
    explicit QgsVectorLayerInterruptionCheckerDuringCountSymbolFeatures( QProgressDialog *dialog )
      : mDialog( dialog )
    {
    }

    bool mustStop() const override
    {
      if ( mDialog->isVisible() )
      {
        // So that we get a chance of hitting the Abort button
#ifdef Q_OS_LINUX
        // For some reason on Windows hasPendingEvents() always return true,
        // but one iteration is actually enough on Windows to get good interactivity
        // whereas on Linux we must allow for far more iterations.
        // For safety limit the number of iterations
        int nIters = 0;
        while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
        {
          QCoreApplication::processEvents();
        }
        return mDialog->wasCanceled();
      }
      return false;
    }

  private:
    QProgressDialog *mDialog = nullptr;
};

bool QgsVectorLayer::countSymbolFeatures( bool showProgress )
{
  if ( mSymbolFeatureCounted )
    return true;

  mSymbolFeatureCountMap.clear();

  if ( !mValid )
  {
    QgsDebugMsg( "invoked with invalid layer" );
    return false;
  }
  if ( !mDataProvider )
  {
    QgsDebugMsg( "invoked with null mDataProvider" );
    return false;
  }
  if ( !mRenderer )
  {
    QgsDebugMsg( "invoked with null mRenderer" );
    return false;
  }

  QgsLegendSymbolList symbolList = mRenderer->legendSymbolItems();
  QgsLegendSymbolList::const_iterator symbolIt = symbolList.constBegin();

  for ( ; symbolIt != symbolList.constEnd(); ++symbolIt )
  {
    mSymbolFeatureCountMap.insert( symbolIt->first, 0 );
  }

  long nFeatures = featureCount();

  QWidget *mainWindow = nullptr;
  Q_FOREACH ( QWidget *widget, qApp->topLevelWidgets() )
  {
    if ( widget->objectName() == QLatin1String( "QgisApp" ) )
    {
      mainWindow = widget;
      break;
    }
  }

  QProgressDialog progressDialog( tr( "Updating feature count for layer %1" ).arg( name() ), tr( "Abort" ), 0, nFeatures, mainWindow );
  progressDialog.setWindowTitle( tr( "QGIS" ) );
  progressDialog.setWindowModality( Qt::WindowModal );
  if ( showProgress )
  {
    // Properly initialize to 0 as recommended in doc so that the evaluation
    // of the total time properly works
    progressDialog.setValue( 0 );
  }
  int featuresCounted = 0;

  // Renderer (rule based) may depend on context scale, with scale is ignored if 0
  QgsRenderContext renderContext;
  renderContext.setRendererScale( 0 );
  renderContext.expressionContext().appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( this ) );

  QgsFeatureRequest request;
  if ( !mRenderer->filterNeedsGeometry() )
    request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setSubsetOfAttributes( mRenderer->usedAttributes( renderContext ), mFields );
  QgsFeatureIterator fit = getFeatures( request );
  QgsVectorLayerInterruptionCheckerDuringCountSymbolFeatures interruptionCheck( &progressDialog );
  if ( showProgress )
  {
    fit.setInterruptionChecker( &interruptionCheck );
  }

  mRenderer->startRender( renderContext, fields() );

  QgsFeature f;
  QTime time;
  time.start();
  while ( fit.nextFeature( f ) )
  {
    renderContext.expressionContext().setFeature( f );
    QSet<QString> featureKeyList = mRenderer->legendKeysForFeature( f, renderContext );
    Q_FOREACH ( const QString &key, featureKeyList )
    {
      mSymbolFeatureCountMap[key] += 1;
    }
    ++featuresCounted;

    if ( showProgress )
    {
      // Refresh progress every 50 features or second
      if ( ( featuresCounted % 50 == 0 ) || time.elapsed() > 1000 )
      {
        time.restart();
        if ( featuresCounted > nFeatures ) //sometimes the feature count is not correct
        {
          progressDialog.setMaximum( 0 );
        }
        progressDialog.setValue( featuresCounted );
      }
      // So that we get a chance of hitting the Abort button
#ifdef Q_OS_LINUX
      // For some reason on Windows hasPendingEvents() always return true,
      // but one iteration is actually enough on Windows to get good interactivity
      // whereas on Linux we must allow for far more iterations.
      // For safety limit the number of iterations
      int nIters = 0;
      while ( QCoreApplication::hasPendingEvents() && ++nIters < 100 )
#endif
      {
        QCoreApplication::processEvents();
      }
      if ( progressDialog.wasCanceled() )
      {
        mSymbolFeatureCountMap.clear();
        mRenderer->stopRender( renderContext );
        return false;
      }
    }
  }
  mRenderer->stopRender( renderContext );
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

QgsRectangle QgsVectorLayer::extent() const
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
    QgsDebugMsg( "Extent of layer: " + mbr.toString() );
    // store the extent
    mValidExtent = true;
    mExtent = mbr;

    mLazyExtent = false;
  }

  if ( mValidExtent )
    return QgsMapLayer::extent();

  if ( !mValid || !mDataProvider )
  {
    QgsDebugMsg( "invoked with invalid layer or null mDataProvider" );
    return rect;
  }

  if ( !mEditBuffer ||
       ( mEditBuffer->mDeletedFeatureIds.isEmpty() && mEditBuffer->mChangedGeometries.isEmpty() ) ||
       QgsDataSourceUri( mDataProvider->dataSourceUri() ).useEstimatedMetadata() )
  {
    mDataProvider->updateExtents();

    // get the extent of the layer from the provider
    // but only when there are some features already
    if ( mDataProvider->featureCount() != 0 )
    {
      QgsRectangle r = mDataProvider->extent();
      rect.combineExtentWith( r );
    }

    if ( mEditBuffer )
    {
      for ( QgsFeatureMap::const_iterator it = mEditBuffer->mAddedFeatures.constBegin(); it != mEditBuffer->mAddedFeatures.constEnd(); ++it )
      {
        if ( it->hasGeometry() )
        {
          QgsRectangle r = it->geometry().boundingBox();
          rect.combineExtentWith( r );
        }
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
      if ( fet.hasGeometry() && fet.geometry().type() != QgsWkbTypes::UnknownGeometry )
      {
        QgsRectangle bb = fet.geometry().boundingBox();
        rect.combineExtentWith( bb );
      }
    }
  }

  if ( rect.xMinimum() > rect.xMaximum() && rect.yMinimum() > rect.yMaximum() )
  {
    // special case when there are no features in provider nor any added
    rect = QgsRectangle(); // use rectangle with zero coordinates
  }

  mValidExtent = true;
  mExtent = rect;

  // Send this (hopefully) up the chain to the map canvas
  emit recalculateExtents();

  return rect;
}

QString QgsVectorLayer::subsetString() const
{
  if ( !mValid || !mDataProvider )
  {
    QgsDebugMsg( "invoked with invalid layer or null mDataProvider" );
    return QString();
  }
  return mDataProvider->subsetString();
}

bool QgsVectorLayer::setSubsetString( const QString &subset )
{
  if ( !mValid || !mDataProvider )
  {
    QgsDebugMsg( "invoked with invalid layer or null mDataProvider" );
    return false;
  }

  bool res = mDataProvider->setSubsetString( subset );

  // get the updated data source string from the provider
  mDataSource = mDataProvider->dataSourceUri();
  updateExtents();
  updateFields();

  if ( res )
    emit repaintRequested();

  return res;
}

bool QgsVectorLayer::simplifyDrawingCanbeApplied( const QgsRenderContext &renderContext, QgsVectorSimplifyMethod::SimplifyHint simplifyHint ) const
{
  if ( mValid && mDataProvider && !mEditBuffer && ( hasGeometryType() && geometryType() != QgsWkbTypes::PointGeometry ) && ( mSimplifyMethod.simplifyHints() & simplifyHint ) && renderContext.useRenderingOptimization() )
  {
    double maximumSimplificationScale = mSimplifyMethod.maximumScale();

    // check maximum scale at which generalisation should be carried out
    return !( maximumSimplificationScale > 1 && renderContext.rendererScale() <= maximumSimplificationScale );
  }
  return false;
}

QgsConditionalLayerStyles *QgsVectorLayer::conditionalStyles() const
{
  return mConditionalStyles;
}

QgsFeatureIterator QgsVectorLayer::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid || !mDataProvider )
    return QgsFeatureIterator();

  return QgsFeatureIterator( new QgsVectorLayerFeatureIterator( new QgsVectorLayerFeatureSource( this ), true, request ) );
}

bool QgsVectorLayer::addFeature( QgsFeature &feature, bool alsoUpdateExtent )
{
  Q_UNUSED( alsoUpdateExtent ); // TODO[MD]
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return false;

  bool success = mEditBuffer->addFeature( feature );

  if ( success )
    updateExtents();

  return success;
}

bool QgsVectorLayer::updateFeature( QgsFeature &f )
{
  QgsFeatureRequest req;
  req.setFilterFid( f.id() );
  if ( !f.hasGeometry() )
    req.setFlags( QgsFeatureRequest::NoGeometry );
  if ( f.attributes().isEmpty() )
    req.setSubsetOfAttributes( QgsAttributeList() );

  QgsFeature current;
  if ( !getFeatures( req ).nextFeature( current ) )
  {
    QgsDebugMsg( QString( "feature %1 could not be retrieved" ).arg( f.id() ) );
    return false;
  }

  if ( f.hasGeometry() && current.hasGeometry() && !f.geometry().isGeosEqual( current.geometry() ) )
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
    if ( fa.at( attr ) != ca.at( attr ) )
    {
      if ( !changeAttributeValue( f.id(), attr, fa.at( attr ), ca.at( attr ) ) )
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
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return false;

  QgsVectorLayerEditUtils utils( this );
  bool result = utils.insertVertex( x, y, atFeatureId, beforeVertex );
  if ( result )
    updateExtents();
  return result;
}


bool QgsVectorLayer::insertVertex( const QgsPointV2 &point, QgsFeatureId atFeatureId, int beforeVertex )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return false;

  QgsVectorLayerEditUtils utils( this );
  bool result = utils.insertVertex( point, atFeatureId, beforeVertex );
  if ( result )
    updateExtents();
  return result;
}


bool QgsVectorLayer::moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return false;

  QgsVectorLayerEditUtils utils( this );
  bool result = utils.moveVertex( x, y, atFeatureId, atVertex );

  if ( result )
    updateExtents();
  return result;
}

bool QgsVectorLayer::moveVertex( const QgsPointV2 &p, QgsFeatureId atFeatureId, int atVertex )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return false;

  QgsVectorLayerEditUtils utils( this );
  bool result = utils.moveVertex( p, atFeatureId, atVertex );

  if ( result )
    updateExtents();
  return result;
}

QgsVectorLayer::EditResult QgsVectorLayer::deleteVertex( QgsFeatureId featureId, int vertex )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return QgsVectorLayer::InvalidLayer;

  QgsVectorLayerEditUtils utils( this );
  EditResult result = utils.deleteVertex( featureId, vertex );

  if ( result == Success )
    updateExtents();
  return result;
}


bool QgsVectorLayer::deleteSelectedFeatures( int *deletedCount )
{
  if ( !mValid || !mDataProvider || !( mDataProvider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
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
  Q_FOREACH ( QgsFeatureId fid, selectedFeatures )
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

int QgsVectorLayer::addRing( const QList<QgsPoint> &ring, QgsFeatureId *featureId )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return 6;

  QgsVectorLayerEditUtils utils( this );
  int result = 5;

  //first try with selected features
  if ( !mSelectedFeatureIds.isEmpty() )
  {
    result = utils.addRing( ring, mSelectedFeatureIds, featureId );
  }

  if ( result != 0 )
  {
    //try with all intersecting features
    result = utils.addRing( ring, QgsFeatureIds(), featureId );
  }

  return result;
}

int QgsVectorLayer::addRing( QgsCurve *ring, QgsFeatureId *featureId )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
  {
    delete ring;
    return 6;
  }

  if ( !ring )
  {
    return 1;
  }

  if ( !ring->isClosed() )
  {
    delete ring;
    return 2;
  }

  QgsVectorLayerEditUtils utils( this );
  int result = 5;

  //first try with selected features
  if ( !mSelectedFeatureIds.isEmpty() )
  {
    result = utils.addRing( static_cast< QgsCurve * >( ring->clone() ), mSelectedFeatureIds, featureId );
  }

  if ( result != 0 )
  {
    //try with all intersecting features
    result = utils.addRing( static_cast< QgsCurve * >( ring->clone() ), QgsFeatureIds(), featureId );
  }

  delete ring;
  return result;
}

int QgsVectorLayer::addPart( const QList<QgsPoint> &points )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
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
  int result = utils.addPart( points, *mSelectedFeatureIds.constBegin() );

  if ( result == 0 )
    updateExtents();
  return result;
}

int QgsVectorLayer::addPart( const QgsPointSequence &points )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
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
  int result = utils.addPart( points, *mSelectedFeatureIds.constBegin() );

  if ( result == 0 )
    updateExtents();
  return result;
}

int QgsVectorLayer::addPart( QgsCurve *ring )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
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
  int result = utils.addPart( ring, *mSelectedFeatureIds.constBegin() );

  if ( result == 0 )
    updateExtents();
  return result;
}

int QgsVectorLayer::translateFeature( QgsFeatureId featureId, double dx, double dy )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  int result = utils.translateFeature( featureId, dx, dy );

  if ( result == 0 )
    updateExtents();
  return result;
}

int QgsVectorLayer::splitParts( const QList<QgsPoint> &splitLine, bool topologicalEditing )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  return utils.splitParts( splitLine, topologicalEditing );
}

int QgsVectorLayer::splitFeatures( const QList<QgsPoint> &splitLine, bool topologicalEditing )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  return utils.splitFeatures( splitLine, topologicalEditing );
}

int QgsVectorLayer::addTopologicalPoints( const QgsGeometry &geom )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  return utils.addTopologicalPoints( geom );
}

int QgsVectorLayer::addTopologicalPoints( const QgsPoint &p )
{
  if ( !mValid || !mEditBuffer || !mDataProvider )
    return -1;

  QgsVectorLayerEditUtils utils( this );
  return utils.addTopologicalPoints( p );
}

void QgsVectorLayer::setLabeling( QgsAbstractVectorLayerLabeling *labeling )
{
  if ( mLabeling == labeling )
    return;

  delete mLabeling;
  mLabeling = labeling;
}

bool QgsVectorLayer::startEditing()
{
  if ( !mValid || !mDataProvider )
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

  emit beforeEditingStarted();

  mDataProvider->enterUpdateMode();

  if ( mDataProvider->transaction() )
  {
    mEditBuffer = new QgsVectorLayerEditPassthrough( this );
  }
  else
  {
    mEditBuffer = new QgsVectorLayerEditBuffer( this );
  }
  // forward signals
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::layerModified, this, &QgsVectorLayer::invalidateSymbolCountedFlag );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::layerModified, this, &QgsVectorLayer::layerModified ); // TODO[MD]: necessary?
  //connect( mEditBuffer, SIGNAL( layerModified() ), this, SLOT( triggerRepaint() ) ); // TODO[MD]: works well?
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::featureAdded, this, &QgsVectorLayer::featureAdded );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::featureDeleted, this, &QgsVectorLayer::onFeatureDeleted );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::geometryChanged, this, &QgsVectorLayer::geometryChanged );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::attributeValueChanged, this, &QgsVectorLayer::attributeValueChanged );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::attributeAdded, this, &QgsVectorLayer::attributeAdded );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::attributeDeleted, this, &QgsVectorLayer::attributeDeleted );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::committedAttributesDeleted, this, &QgsVectorLayer::committedAttributesDeleted );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::committedAttributesAdded, this, &QgsVectorLayer::committedAttributesAdded );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::committedFeaturesAdded, this, &QgsVectorLayer::committedFeaturesAdded );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::committedFeaturesRemoved, this, &QgsVectorLayer::committedFeaturesRemoved );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::committedAttributeValuesChanges, this, &QgsVectorLayer::committedAttributeValuesChanges );
  connect( mEditBuffer, &QgsVectorLayerEditBuffer::committedGeometriesChanges, this, &QgsVectorLayer::committedGeometriesChanges );

  updateFields();

  emit editingStarted();

  return true;
}

bool QgsVectorLayer::readXml( const QDomNode &layer_node )
{
  QgsDebugMsg( QString( "Datasource in QgsVectorLayer::readXml: " ) + mDataSource.toLocal8Bit().data() );

  //process provider key
  QDomNode pkeyNode = layer_node.namedItem( QStringLiteral( "provider" ) );

  if ( pkeyNode.isNull() )
  {
    mProviderKey = QLatin1String( "" );
  }
  else
  {
    QDomElement pkeyElt = pkeyNode.toElement();
    mProviderKey = pkeyElt.text();
  }

  // determine type of vector layer
  if ( !mProviderKey.isNull() )
  {
    // if the provider string isn't empty, then we successfully
    // got the stored provider
  }
  else if ( mDataSource.contains( QLatin1String( "dbname=" ) ) )
  {
    mProviderKey = QStringLiteral( "postgres" );
  }
  else
  {
    mProviderKey = QStringLiteral( "ogr" );
  }

  if ( !setDataProvider( mProviderKey ) )
  {
    return false;
  }

  QDomElement pkeyElem = pkeyNode.toElement();
  if ( !pkeyElem.isNull() )
  {
    QString encodingString = pkeyElem.attribute( QStringLiteral( "encoding" ) );
    if ( !encodingString.isEmpty() )
    {
      mDataProvider->setEncoding( encodingString );
    }
  }

  // load vector joins - does not resolve references to layers yet
  mJoinBuffer->readXml( layer_node );

  updateFields();

  QString errorMsg;
  if ( !readSymbology( layer_node, errorMsg ) )
  {
    return false;
  }

  readStyleManager( layer_node );

  QDomNode depsNode = layer_node.namedItem( QStringLiteral( "dataDependencies" ) );
  QDomNodeList depsNodes = depsNode.childNodes();
  QSet<QgsMapLayerDependency> sources;
  for ( int i = 0; i < depsNodes.count(); i++ )
  {
    QString source = depsNodes.at( i ).toElement().attribute( QStringLiteral( "id" ) );
    sources << QgsMapLayerDependency( source );
  }
  setDependencies( sources );

  setLegend( QgsMapLayerLegend::defaultVectorLegend( this ) );

  return mValid;               // should be true if read successfully

} // void QgsVectorLayer::readXml


void QgsVectorLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, bool loadDefaultStyleFlag )
{
  QgsWkbTypes::GeometryType geomType = mValid && mDataProvider ? geometryType() : QgsWkbTypes::UnknownGeometry;

  mDataSource = dataSource;
  mLayerName = capitalizeLayerName( baseName );
  setName( mLayerName );
  setDataProvider( provider );

  if ( !mValid )
    return;

  // Always set crs
  setCoordinateSystem();

  // reset style if loading default style, style is missing, or geometry type has changed
  if ( !renderer() || !legend() || geomType != geometryType() || loadDefaultStyleFlag )
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
      setRenderer( QgsFeatureRenderer::defaultRenderer( geometryType() ) );
    }

    setLegend( QgsMapLayerLegend::defaultVectorLegend( this ) );
  }

  emit repaintRequested();
}


bool QgsVectorLayer::setDataProvider( QString const &provider )
{
  mProviderKey = provider;     // XXX is this necessary?  Usually already set
  // XXX when execution gets here.

  //XXX - This was a dynamic cast but that kills the Windows
  //      version big-time with an abnormal termination error
  delete mDataProvider;
  mDataProvider = ( QgsVectorDataProvider * )( QgsProviderRegistry::instance()->provider( provider, mDataSource ) );
  if ( !mDataProvider )
  {
    QgsDebugMsg( " unable to get data provider" );
    return false;
  }

  connect( mDataProvider, &QgsVectorDataProvider::raiseError, this, &QgsVectorLayer::raiseError );

  QgsDebugMsg( "Instantiated the data provider plugin" );

  mValid = mDataProvider->isValid();
  if ( !mValid )
  {
    QgsDebugMsg( "Invalid provider plugin " + QString( mDataSource.toUtf8() ) );
    return false;
  }

  // TODO: Check if the provider has the capability to send fullExtentCalculated
  connect( mDataProvider, &QgsVectorDataProvider::fullExtentCalculated, this, &QgsVectorLayer::updateExtents );

  // get and store the feature type
  mWkbType = mDataProvider->wkbType();

  mExpressionFieldBuffer = new QgsExpressionFieldBuffer();
  updateFields();

  if ( mProviderKey == QLatin1String( "postgres" ) )
  {
    QgsDebugMsg( "Beautifying layer name " + name() );

    // adjust the display name for postgres layers
    QRegExp reg( R"lit("[^"]+"\."([^"] + )"( \([^)]+\))?)lit" );
    if ( reg.indexIn( name() ) >= 0 )
    {
      QStringList stuff = reg.capturedTexts();
      QString lName = stuff[1];

      const QMap<QString, QgsMapLayer *> &layers = QgsProject::instance()->mapLayers();

      QMap<QString, QgsMapLayer *>::const_iterator it;
      for ( it = layers.constBegin(); it != layers.constEnd() && ( *it )->name() != lName; ++it )
        ;

      if ( it != layers.constEnd() && stuff.size() > 2 )
      {
        lName += '.' + stuff[2].mid( 2, stuff[2].length() - 3 );
      }

      if ( !lName.isEmpty() )
        setName( lName );
    }

    QgsDebugMsg( "Beautified layer name " + name() );

    // deal with unnecessary schema qualification to make v.in.ogr happy
    mDataSource = mDataProvider->dataSourceUri();
  }
  else if ( mProviderKey == QLatin1String( "osm" ) )
  {
    // make sure that the "observer" has been removed from URI to avoid crashes
    mDataSource = mDataProvider->dataSourceUri();
  }
  else if ( provider == QLatin1String( "ogr" ) )
  {
    // make sure that the /vsigzip or /vsizip is added to uri, if applicable
    mDataSource = mDataProvider->dataSourceUri();
    if ( mDataSource.right( 10 ) == QLatin1String( "|layerid=0" ) )
      mDataSource.chop( 10 );
  }

  connect( mDataProvider, &QgsVectorDataProvider::dataChanged, this, &QgsVectorLayer::dataChanged );
  connect( mDataProvider, &QgsVectorDataProvider::dataChanged, this, &QgsVectorLayer::removeSelection );

  return true;
} // QgsVectorLayer:: setDataProvider




/* virtual */
bool QgsVectorLayer::writeXml( QDomNode &layer_node,
                               QDomDocument &document ) const
{
  // first get the layer element so that we can append the type attribute

  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || ( "maplayer" != mapLayerNode.nodeName() ) )
  {
    QgsDebugMsg( "can't find <maplayer>" );
    return false;
  }

  mapLayerNode.setAttribute( QStringLiteral( "type" ), QStringLiteral( "vector" ) );

  // set the geometry type
  mapLayerNode.setAttribute( QStringLiteral( "geometry" ), QgsWkbTypes::geometryDisplayString( geometryType() ) );

  // add provider node
  if ( mDataProvider )
  {
    QDomElement provider  = document.createElement( QStringLiteral( "provider" ) );
    provider.setAttribute( QStringLiteral( "encoding" ), mDataProvider->encoding() );
    QDomText providerText = document.createTextNode( providerType() );
    provider.appendChild( providerText );
    layer_node.appendChild( provider );
  }

  //save joins
  mJoinBuffer->writeXml( layer_node, document );

  // dependencies
  QDomElement dependenciesElement = document.createElement( QStringLiteral( "layerDependencies" ) );
  Q_FOREACH ( const QgsMapLayerDependency &dep, dependencies() )
  {
    if ( dep.type() != QgsMapLayerDependency::PresenceDependency )
      continue;
    QDomElement depElem = document.createElement( QStringLiteral( "layer" ) );
    depElem.setAttribute( QStringLiteral( "id" ), dep.layerId() );
    dependenciesElement.appendChild( depElem );
  }
  layer_node.appendChild( dependenciesElement );

  // change dependencies
  QDomElement dataDependenciesElement = document.createElement( QStringLiteral( "dataDependencies" ) );
  Q_FOREACH ( const QgsMapLayerDependency &dep, dependencies() )
  {
    if ( dep.type() != QgsMapLayerDependency::DataDependency )
      continue;
    QDomElement depElem = document.createElement( QStringLiteral( "layer" ) );
    depElem.setAttribute( QStringLiteral( "id" ), dep.layerId() );
    dataDependenciesElement.appendChild( depElem );
  }
  layer_node.appendChild( dataDependenciesElement );

  // save expression fields
  mExpressionFieldBuffer->writeXml( layer_node, document );

  writeStyleManager( layer_node, document );

  // renderer specific settings
  QString errorMsg;
  return writeSymbology( layer_node, document, errorMsg );
} // bool QgsVectorLayer::writeXml


void QgsVectorLayer::resolveReferences( QgsProject *project )
{
  mJoinBuffer->resolveReferences( project );
}


bool QgsVectorLayer::readSymbology( const QDomNode &layerNode, QString &errorMessage )
{
  if ( !mExpressionFieldBuffer )
    mExpressionFieldBuffer = new QgsExpressionFieldBuffer();
  mExpressionFieldBuffer->readXml( layerNode );

  updateFields();

  readStyle( layerNode, errorMessage );

  mDisplayExpression = layerNode.namedItem( QStringLiteral( "previewExpression" ) ).toElement().text();
  mMapTipTemplate = layerNode.namedItem( QStringLiteral( "mapTip" ) ).toElement().text();

  QString displayField = layerNode.namedItem( QStringLiteral( "displayfield" ) ).toElement().text();

  // Try to migrate pre QGIS 3.0 display field property
  if ( mFields.lookupField( displayField ) < 0 )
  {
    // if it's not a field, it's a maptip
    if ( mMapTipTemplate.isEmpty() )
      mMapTipTemplate = displayField;
  }
  else
  {
    if ( mDisplayExpression.isEmpty() )
      mDisplayExpression = QgsExpression::quotedColumnRef( displayField );
  }

  // process the attribute actions
  mActions->readXml( layerNode );

  QDomNode annotationFormNode = layerNode.namedItem( QStringLiteral( "annotationform" ) );
  if ( !annotationFormNode.isNull() )
  {
    QDomElement e = annotationFormNode.toElement();
    mAnnotationForm = QgsProject::instance()->readPath( e.text() );
  }

  mAttributeAliasMap.clear();
  QDomNode aliasesNode = layerNode.namedItem( QStringLiteral( "aliases" ) );
  if ( !aliasesNode.isNull() )
  {
    QDomElement aliasElem;

    QDomNodeList aliasNodeList = aliasesNode.toElement().elementsByTagName( QStringLiteral( "alias" ) );
    for ( int i = 0; i < aliasNodeList.size(); ++i )
    {
      aliasElem = aliasNodeList.at( i ).toElement();

      QString field;
      if ( aliasElem.hasAttribute( QStringLiteral( "field" ) ) )
      {
        field = aliasElem.attribute( QStringLiteral( "field" ) );
      }
      else
      {
        int index = aliasElem.attribute( QStringLiteral( "index" ) ).toInt();

        if ( index >= 0 && index < fields().count() )
          field = fields().at( index ).name();
      }

      mAttributeAliasMap.insert( field, aliasElem.attribute( QStringLiteral( "name" ) ) );
    }
  }

  // default expressions
  mDefaultExpressionMap.clear();
  QDomNode defaultsNode = layerNode.namedItem( QStringLiteral( "defaults" ) );
  if ( !defaultsNode.isNull() )
  {
    QDomNodeList defaultNodeList = defaultsNode.toElement().elementsByTagName( QStringLiteral( "default" ) );
    for ( int i = 0; i < defaultNodeList.size(); ++i )
    {
      QDomElement defaultElem = defaultNodeList.at( i ).toElement();

      QString field = defaultElem.attribute( QStringLiteral( "field" ), QString() );
      QString expression = defaultElem.attribute( QStringLiteral( "expression" ), QString() );
      if ( field.isEmpty() || expression.isEmpty() )
        continue;

      mDefaultExpressionMap.insert( field, expression );
    }
  }

  // constraints
  mFieldConstraints.clear();
  mFieldConstraintStrength.clear();
  QDomNode constraintsNode = layerNode.namedItem( "constraints" );
  if ( !constraintsNode.isNull() )
  {
    QDomNodeList constraintNodeList = constraintsNode.toElement().elementsByTagName( "constraint" );
    for ( int i = 0; i < constraintNodeList.size(); ++i )
    {
      QDomElement constraintElem = constraintNodeList.at( i ).toElement();

      QString field = constraintElem.attribute( "field", QString() );
      int constraints = constraintElem.attribute( "constraints", QString( "0" ) ).toInt();
      if ( field.isEmpty() || constraints == 0 )
        continue;

      mFieldConstraints.insert( field, static_cast< QgsFieldConstraints::Constraints >( constraints ) );

      int uniqueStrength = constraintElem.attribute( "unique_strength", QString( "1" ) ).toInt();
      int notNullStrength = constraintElem.attribute( "notnull_strength", QString( "1" ) ).toInt();
      int expStrength = constraintElem.attribute( "exp_strength", QString( "1" ) ).toInt();

      mFieldConstraintStrength.insert( qMakePair( field, QgsFieldConstraints::ConstraintUnique ), static_cast< QgsFieldConstraints::ConstraintStrength >( uniqueStrength ) );
      mFieldConstraintStrength.insert( qMakePair( field, QgsFieldConstraints::ConstraintNotNull ), static_cast< QgsFieldConstraints::ConstraintStrength >( notNullStrength ) );
      mFieldConstraintStrength.insert( qMakePair( field, QgsFieldConstraints::ConstraintExpression ), static_cast< QgsFieldConstraints::ConstraintStrength >( expStrength ) );
    }
  }
  mFieldConstraintExpressions.clear();
  QDomNode constraintExpressionsNode = layerNode.namedItem( "constraintExpressions" );
  if ( !constraintExpressionsNode.isNull() )
  {
    QDomNodeList constraintNodeList = constraintExpressionsNode.toElement().elementsByTagName( "constraint" );
    for ( int i = 0; i < constraintNodeList.size(); ++i )
    {
      QDomElement constraintElem = constraintNodeList.at( i ).toElement();

      QString field = constraintElem.attribute( "field", QString() );
      QString exp = constraintElem.attribute( "exp", QString() );
      QString desc = constraintElem.attribute( "desc", QString() );
      if ( field.isEmpty() || exp.isEmpty() )
        continue;

      mFieldConstraintExpressions.insert( field, qMakePair( exp, desc ) );
    }
  }

  updateFields();

  //Attributes excluded from WMS and WFS
  mExcludeAttributesWMS.clear();
  QDomNode excludeWMSNode = layerNode.namedItem( QStringLiteral( "excludeAttributesWMS" ) );
  if ( !excludeWMSNode.isNull() )
  {
    QDomNodeList attributeNodeList = excludeWMSNode.toElement().elementsByTagName( QStringLiteral( "attribute" ) );
    for ( int i = 0; i < attributeNodeList.size(); ++i )
    {
      mExcludeAttributesWMS.insert( attributeNodeList.at( i ).toElement().text() );
    }
  }

  mExcludeAttributesWFS.clear();
  QDomNode excludeWFSNode = layerNode.namedItem( QStringLiteral( "excludeAttributesWFS" ) );
  if ( !excludeWFSNode.isNull() )
  {
    QDomNodeList attributeNodeList = excludeWFSNode.toElement().elementsByTagName( QStringLiteral( "attribute" ) );
    for ( int i = 0; i < attributeNodeList.size(); ++i )
    {
      mExcludeAttributesWFS.insert( attributeNodeList.at( i ).toElement().text() );
    }
  }

  // Load editor widget configuration
  QDomElement widgetsElem = layerNode.namedItem( QStringLiteral( "fieldConfiguration" ) ).toElement();

  QDomNodeList fieldConfigurationElementList = widgetsElem.elementsByTagName( QStringLiteral( "field" ) );

  for ( int i = 0; i < fieldConfigurationElementList.size(); ++i )
  {
    const QDomElement fieldConfigElement = fieldConfigurationElementList.at( i ).toElement();
    const QDomElement fieldWidgetElement = fieldConfigElement.elementsByTagName( QStringLiteral( "editWidget" ) ).at( 0 ).toElement();

    QString fieldName = fieldConfigElement.attribute( QStringLiteral( "name" ) );

    const QString widgetType = fieldWidgetElement.attribute( QStringLiteral( "type" ) );
    const QDomElement cfgElem = fieldConfigElement.elementsByTagName( QStringLiteral( "config" ) ).at( 0 ).toElement();
    const QDomElement optionsElem = cfgElem.childNodes().at( 0 ).toElement();
    QVariantMap optionsMap = QgsXmlUtils::readVariant( optionsElem ).toMap();
    QgsEditorWidgetSetup setup = QgsEditorWidgetSetup( widgetType, optionsMap );
    mFieldWidgetSetups[fieldName] = setup;
  }

  mEditFormConfig.readXml( layerNode );

  mAttributeTableConfig.readXml( layerNode );

  mConditionalStyles->readXml( layerNode );

  readCustomProperties( layerNode, QStringLiteral( "variable" ) );

  QDomElement mapLayerNode = layerNode.toElement();
  if ( mapLayerNode.attribute( QStringLiteral( "readOnly" ), QStringLiteral( "0" ) ).toInt() == 1 )
    mReadOnly = true;

  updateFields();

  return true;
}

bool QgsVectorLayer::readStyle( const QDomNode &node, QString &errorMessage )
{
  bool result = true;
  emit readCustomSymbology( node.toElement(), errorMessage );

  if ( hasGeometryType() )
  {
    // try renderer v2 first
    QDomElement rendererElement = node.firstChildElement( RENDERER_TAG_NAME );
    if ( !rendererElement.isNull() )
    {
      QgsFeatureRenderer *r = QgsFeatureRenderer::load( rendererElement );
      if ( r )
      {
        setRenderer( r );
      }
      else
      {
        result = false;
      }
    }

    // make sure layer has a renderer - if none exists, fallback to a default renderer
    if ( !renderer() )
    {
      setRenderer( QgsFeatureRenderer::defaultRenderer( geometryType() ) );
    }

    QDomElement labelingElement = node.firstChildElement( QStringLiteral( "labeling" ) );
    if ( !labelingElement.isNull() )
    {
      QgsAbstractVectorLayerLabeling *l = QgsAbstractVectorLayerLabeling::create( labelingElement );
      setLabeling( l ? l : new QgsVectorLayerSimpleLabeling );
    }

    // get and set the blend mode if it exists
    QDomNode blendModeNode = node.namedItem( QStringLiteral( "blendMode" ) );
    if ( !blendModeNode.isNull() )
    {
      QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( e.text().toInt() ) ) );
    }

    // get and set the feature blend mode if it exists
    QDomNode featureBlendModeNode = node.namedItem( QStringLiteral( "featureBlendMode" ) );
    if ( !featureBlendModeNode.isNull() )
    {
      QDomElement e = featureBlendModeNode.toElement();
      setFeatureBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( e.text().toInt() ) ) );
    }

    // get and set the layer transparency if it exists
    QDomNode layerTransparencyNode = node.namedItem( QStringLiteral( "layerTransparency" ) );
    if ( !layerTransparencyNode.isNull() )
    {
      QDomElement e = layerTransparencyNode.toElement();
      setLayerTransparency( e.text().toInt() );
    }

    QDomElement e = node.toElement();

    // get the simplification drawing settings
    mSimplifyMethod.setSimplifyHints( static_cast< QgsVectorSimplifyMethod::SimplifyHints >( e.attribute( QStringLiteral( "simplifyDrawingHints" ), QStringLiteral( "1" ) ).toInt() ) );
    mSimplifyMethod.setSimplifyAlgorithm( static_cast< QgsVectorSimplifyMethod::SimplifyAlgorithm >( e.attribute( QStringLiteral( "simplifyAlgorithm" ), QStringLiteral( "0" ) ).toInt() ) );
    mSimplifyMethod.setThreshold( e.attribute( QStringLiteral( "simplifyDrawingTol" ), QStringLiteral( "1" ) ).toFloat() );
    mSimplifyMethod.setForceLocalOptimization( e.attribute( QStringLiteral( "simplifyLocal" ), QStringLiteral( "1" ) ).toInt() );
    mSimplifyMethod.setMaximumScale( e.attribute( QStringLiteral( "simplifyMaxScale" ), QStringLiteral( "1" ) ).toFloat() );

    //also restore custom properties (for labeling-ng)
    readCustomProperties( node, QStringLiteral( "labeling" ) );

    //diagram renderer and diagram layer settings
    delete mDiagramRenderer;
    mDiagramRenderer = nullptr;
    QDomElement singleCatDiagramElem = node.firstChildElement( QStringLiteral( "SingleCategoryDiagramRenderer" ) );
    if ( !singleCatDiagramElem.isNull() )
    {
      mDiagramRenderer = new QgsSingleCategoryDiagramRenderer();
      mDiagramRenderer->readXml( singleCatDiagramElem, this );
    }
    QDomElement linearDiagramElem = node.firstChildElement( QStringLiteral( "LinearlyInterpolatedDiagramRenderer" ) );
    if ( !linearDiagramElem.isNull() )
    {
      mDiagramRenderer = new QgsLinearlyInterpolatedDiagramRenderer();
      mDiagramRenderer->readXml( linearDiagramElem, this );
    }

    if ( mDiagramRenderer )
    {
      QDomElement diagramSettingsElem = node.firstChildElement( QStringLiteral( "DiagramLayerSettings" ) );
      if ( !diagramSettingsElem.isNull() )
      {
        delete mDiagramLayerSettings;
        mDiagramLayerSettings = new QgsDiagramLayerSettings();
        mDiagramLayerSettings->readXml( diagramSettingsElem, this );
      }
    }
  }
  return result;
}

bool QgsVectorLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage ) const
{
  ( void )writeStyle( node, doc, errorMessage );

  QDomElement fieldConfigurationElement = doc.createElement( QStringLiteral( "fieldConfiguration" ) );
  node.appendChild( fieldConfigurationElement );

  int index = 0;
  Q_FOREACH ( const QgsField &field, mFields )
  {

    QDomElement fieldElement = doc.createElement( QStringLiteral( "field" ) );
    fieldElement.setAttribute( QStringLiteral( "name" ), field.name() );

    fieldConfigurationElement.appendChild( fieldElement );

    QgsEditorWidgetSetup widgetSetup = field.editorWidgetSetup();

    // TODO : wrap this part in an if to only save if it was user-modified
    QDomElement editWidgetElement = doc.createElement( QStringLiteral( "editWidget" ) );
    fieldElement.appendChild( editWidgetElement );
    editWidgetElement.setAttribute( "type", field.editorWidgetSetup().type() );
    QDomElement editWidgetConfigElement = doc.createElement( QStringLiteral( "config" ) );

    editWidgetConfigElement.appendChild( QgsXmlUtils::writeVariant( widgetSetup.config(), doc ) );
    editWidgetElement.appendChild( editWidgetConfigElement );
    // END TODO : wrap this part in an if to only save if it was user-modified

    ++index;
  }

  QDomElement afField = doc.createElement( QStringLiteral( "annotationform" ) );
  QDomText afText = doc.createTextNode( QgsProject::instance()->writePath( mAnnotationForm ) );
  afField.appendChild( afText );
  node.appendChild( afField );

  //attribute aliases
  QDomElement aliasElem = doc.createElement( QStringLiteral( "aliases" ) );
  Q_FOREACH ( const QgsField &field, mFields )
  {
    QDomElement aliasEntryElem = doc.createElement( QStringLiteral( "alias" ) );
    aliasEntryElem.setAttribute( QStringLiteral( "field" ), field.name() );
    aliasEntryElem.setAttribute( QStringLiteral( "index" ), mFields.indexFromName( field.name() ) );
    aliasEntryElem.setAttribute( QStringLiteral( "name" ), field.alias() );
    aliasElem.appendChild( aliasEntryElem );
  }
  node.appendChild( aliasElem );

  //exclude attributes WMS
  QDomElement excludeWMSElem = doc.createElement( QStringLiteral( "excludeAttributesWMS" ) );
  QSet<QString>::const_iterator attWMSIt = mExcludeAttributesWMS.constBegin();
  for ( ; attWMSIt != mExcludeAttributesWMS.constEnd(); ++attWMSIt )
  {
    QDomElement attrElem = doc.createElement( QStringLiteral( "attribute" ) );
    QDomText attrText = doc.createTextNode( *attWMSIt );
    attrElem.appendChild( attrText );
    excludeWMSElem.appendChild( attrElem );
  }
  node.appendChild( excludeWMSElem );

  //exclude attributes WFS
  QDomElement excludeWFSElem = doc.createElement( QStringLiteral( "excludeAttributesWFS" ) );
  QSet<QString>::const_iterator attWFSIt = mExcludeAttributesWFS.constBegin();
  for ( ; attWFSIt != mExcludeAttributesWFS.constEnd(); ++attWFSIt )
  {
    QDomElement attrElem = doc.createElement( QStringLiteral( "attribute" ) );
    QDomText attrText = doc.createTextNode( *attWFSIt );
    attrElem.appendChild( attrText );
    excludeWFSElem.appendChild( attrElem );
  }
  node.appendChild( excludeWFSElem );

  //default expressions
  QDomElement defaultsElem = doc.createElement( QStringLiteral( "defaults" ) );
  Q_FOREACH ( const QgsField &field, mFields )
  {
    QDomElement defaultElem = doc.createElement( QStringLiteral( "default" ) );
    defaultElem.setAttribute( QStringLiteral( "field" ), field.name() );
    defaultElem.setAttribute( QStringLiteral( "expression" ), field.defaultValueExpression() );
    defaultsElem.appendChild( defaultElem );
  }
  node.appendChild( defaultsElem );

  // constraints
  QDomElement constraintsElem = doc.createElement( "constraints" );
  Q_FOREACH ( const QgsField &field, mFields )
  {
    QDomElement constraintElem = doc.createElement( "constraint" );
    constraintElem.setAttribute( "field", field.name() );
    constraintElem.setAttribute( "constraints", field.constraints().constraints() );
    constraintElem.setAttribute( "unique_strength", field.constraints().constraintStrength( QgsFieldConstraints::ConstraintUnique ) );
    constraintElem.setAttribute( "notnull_strength", field.constraints().constraintStrength( QgsFieldConstraints::ConstraintNotNull ) );
    constraintElem.setAttribute( "exp_strength", field.constraints().constraintStrength( QgsFieldConstraints::ConstraintExpression ) );
    constraintsElem.appendChild( constraintElem );
  }
  node.appendChild( constraintsElem );

  // constraint expressions
  QDomElement constraintExpressionsElem = doc.createElement( "constraintExpressions" );
  Q_FOREACH ( const QgsField &field, mFields )
  {
    QDomElement constraintExpressionElem = doc.createElement( "constraint" );
    constraintExpressionElem.setAttribute( "field", field.name() );
    constraintExpressionElem.setAttribute( "exp", field.constraints().constraintExpression() );
    constraintExpressionElem.setAttribute( "desc", field.constraints().constraintDescription() );
    constraintExpressionsElem.appendChild( constraintExpressionElem );
  }
  node.appendChild( constraintExpressionsElem );

  // add attribute actions
  mActions->writeXml( node );
  mAttributeTableConfig.writeXml( node );
  mEditFormConfig.writeXml( node );
  mConditionalStyles->writeXml( node, doc );

  // save expression fields
  if ( !mExpressionFieldBuffer )
  {
    // can happen when saving style on a invalid layer
    QgsExpressionFieldBuffer dummy;
    dummy.writeXml( node, doc );
  }
  else
    mExpressionFieldBuffer->writeXml( node, doc );

  // save readonly state
  node.toElement().setAttribute( QStringLiteral( "readOnly" ), mReadOnly );

  // save preview expression
  QDomElement prevExpElem = doc.createElement( QStringLiteral( "previewExpression" ) );
  QDomText prevExpText = doc.createTextNode( mDisplayExpression );
  prevExpElem.appendChild( prevExpText );
  node.appendChild( prevExpElem );

  // save map tip
  QDomElement mapTipElem = doc.createElement( QStringLiteral( "mapTip" ) );
  QDomText mapTipText = doc.createTextNode( mMapTipTemplate );
  mapTipElem.appendChild( mapTipText );
  node.toElement().appendChild( mapTipElem );

  return true;
}

bool QgsVectorLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage ) const
{
  QDomElement mapLayerNode = node.toElement();

  emit writeCustomSymbology( mapLayerNode, doc, errorMessage );

  if ( hasGeometryType() )
  {
    if ( mRenderer )
    {
      QDomElement rendererElement = mRenderer->save( doc );
      node.appendChild( rendererElement );
    }

    if ( mLabeling )
    {
      QDomElement labelingElement = mLabeling->save( doc );
      node.appendChild( labelingElement );
    }

    // save the simplification drawing settings
    mapLayerNode.setAttribute( QStringLiteral( "simplifyDrawingHints" ), QString::number( mSimplifyMethod.simplifyHints() ) );
    mapLayerNode.setAttribute( QStringLiteral( "simplifyAlgorithm" ), QString::number( mSimplifyMethod.simplifyAlgorithm() ) );
    mapLayerNode.setAttribute( QStringLiteral( "simplifyDrawingTol" ), QString::number( mSimplifyMethod.threshold() ) );
    mapLayerNode.setAttribute( QStringLiteral( "simplifyLocal" ), mSimplifyMethod.forceLocalOptimization() ? 1 : 0 );
    mapLayerNode.setAttribute( QStringLiteral( "simplifyMaxScale" ), QString::number( mSimplifyMethod.maximumScale() ) );

    //save customproperties (for labeling ng)
    writeCustomProperties( node, doc );

    // add the blend mode field
    QDomElement blendModeElem  = doc.createElement( QStringLiteral( "blendMode" ) );
    QDomText blendModeText = doc.createTextNode( QString::number( QgsPainting::getBlendModeEnum( blendMode() ) ) );
    blendModeElem.appendChild( blendModeText );
    node.appendChild( blendModeElem );

    // add the feature blend mode field
    QDomElement featureBlendModeElem  = doc.createElement( QStringLiteral( "featureBlendMode" ) );
    QDomText featureBlendModeText = doc.createTextNode( QString::number( QgsPainting::getBlendModeEnum( featureBlendMode() ) ) );
    featureBlendModeElem.appendChild( featureBlendModeText );
    node.appendChild( featureBlendModeElem );

    // add the layer transparency
    QDomElement layerTransparencyElem  = doc.createElement( QStringLiteral( "layerTransparency" ) );
    QDomText layerTransparencyText = doc.createTextNode( QString::number( layerTransparency() ) );
    layerTransparencyElem.appendChild( layerTransparencyText );
    node.appendChild( layerTransparencyElem );

    if ( mDiagramRenderer )
    {
      mDiagramRenderer->writeXml( mapLayerNode, doc, this );
      if ( mDiagramLayerSettings )
        mDiagramLayerSettings->writeXml( mapLayerNode, doc, this );
    }
  }
  return true;
}

bool QgsVectorLayer::readSld( const QDomNode &node, QString &errorMessage )
{
  // get the Name element
  QDomElement nameElem = node.firstChildElement( QStringLiteral( "Name" ) );
  if ( nameElem.isNull() )
  {
    errorMessage = QStringLiteral( "Warning: Name element not found within NamedLayer while it's required." );
  }

  if ( hasGeometryType() )
  {
    QgsFeatureRenderer *r = QgsFeatureRenderer::loadSld( node, geometryType(), errorMessage );
    if ( !r )
      return false;

    setRenderer( r );

    // labeling
    readSldLabeling( node );
  }
  return true;
}

bool QgsVectorLayer::writeSld( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsStringMap &props ) const
{
  Q_UNUSED( errorMessage );

  // store the Name element
  QDomElement nameNode = doc.createElement( QStringLiteral( "se:Name" ) );
  nameNode.appendChild( doc.createTextNode( name() ) );
  node.appendChild( nameNode );

  QgsStringMap localProps = QgsStringMap( props );
  if ( hasScaleBasedVisibility() )
  {
    QgsSymbolLayerUtils::mergeScaleDependencies( minimumScale(), maximumScale(), localProps );
  }

  if ( hasGeometryType() )
  {
    node.appendChild( mRenderer->writeSld( doc, name(), localProps ) );
  }
  return true;
}


bool QgsVectorLayer::changeGeometry( QgsFeatureId fid, const QgsGeometry &geom )
{
  if ( !mEditBuffer || !mDataProvider )
  {
    return false;
  }

  updateExtents();

  bool result = mEditBuffer->changeGeometry( fid, geom );

  if ( result )
    updateExtents();
  return result;
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

void QgsVectorLayer::removeFieldAlias( int attIndex )
{
  if ( attIndex < 0 || attIndex >= fields().count() )
    return;

  QString name = fields().at( attIndex ).name();
  mFields[ attIndex ].setAlias( QString() );
  if ( mAttributeAliasMap.contains( name ) )
  {
    mAttributeAliasMap.remove( name );
    updateFields();
    mEditFormConfig.setFields( mFields );
    emit layerModified();
  }
}

bool QgsVectorLayer::renameAttribute( int index, const QString &newName )
{
  if ( index < 0 || index >= fields().count() )
    return false;

  switch ( mFields.fieldOrigin( index ) )
  {
    case QgsFields::OriginExpression:
    {
      if ( mExpressionFieldBuffer )
      {
        int oi = mFields.fieldOriginIndex( index );
        mExpressionFieldBuffer->renameExpression( oi, newName );
        updateFields();
        return true;
      }
      else
      {
        return false;
      }
    }

    case QgsFields::OriginProvider:
    case QgsFields::OriginEdit:

      if ( !mEditBuffer || !mDataProvider )
        return false;

      return mEditBuffer->renameAttribute( index, newName );

    case QgsFields::OriginJoin:
    case QgsFields::OriginUnknown:
      return false;

  }

  return false; // avoid warning
}

void QgsVectorLayer::setFieldAlias( int attIndex, const QString &aliasString )
{
  if ( attIndex < 0 || attIndex >= fields().count() )
    return;

  QString name = fields().at( attIndex ).name();

  mAttributeAliasMap.insert( name, aliasString );
  mFields[ attIndex ].setAlias( aliasString );
  mEditFormConfig.setFields( mFields );
  emit layerModified(); // TODO[MD]: should have a different signal?
}

QString QgsVectorLayer::attributeAlias( int index ) const
{
  if ( index < 0 || index >= fields().count() )
    return QString();

  return fields().at( index ).alias();
}

QString QgsVectorLayer::attributeDisplayName( int index ) const
{
  if ( index >= 0 && index < mFields.count() )
    return mFields.at( index ).displayName();
  else
    return QString();
}

QgsStringMap QgsVectorLayer::attributeAliases() const
{
  QgsStringMap map;
  Q_FOREACH ( const QgsField &field, fields() )
  {
    if ( !field.alias().isEmpty() )
      map.insert( field.name(), field.alias() );
  }
  return map;
}

bool QgsVectorLayer::deleteAttribute( int index )
{
  if ( index < 0 || index >= fields().count() )
    return false;

  if ( mFields.fieldOrigin( index ) == QgsFields::OriginExpression )
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

  // Remove multiple occurrences of same attribute
  attrs = attrs.toSet().toList();

  std::sort( attrs.begin(), attrs.end(), std::greater<int>() );

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
  {
    mSelectedFeatureIds.remove( fid ); // remove it from selection
    updateExtents();
  }

  return res;
}

bool QgsVectorLayer::deleteFeatures( const QgsFeatureIds &fids )
{
  if ( !mEditBuffer )
  {
    QgsDebugMsg( "Cannot delete features (mEditBuffer==NULL)" );
    return false;
  }

  bool res = mEditBuffer->deleteFeatures( fids );

  if ( res )
  {
    mSelectedFeatureIds.subtract( fids ); // remove it from selection
    updateExtents();
  }

  return res;
}

QgsAttributeList QgsVectorLayer::pkAttributeList() const
{
  QgsAttributeList pkAttributesList;

  QgsAttributeList providerIndexes = mDataProvider->pkAttributeIndexes();
  for ( int i = 0; i < mFields.count(); ++i )
  {
    if ( mFields.fieldOrigin( i ) == QgsFields::OriginProvider &&
         providerIndexes.contains( mFields.fieldOriginIndex( i ) ) )
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
    mEditBuffer = nullptr;
    undoStack()->clear();
    emit editingStopped();
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Commit errors:\n  %1" ).arg( mCommitErrors.join( QStringLiteral( "\n  " ) ) ) );
  }

  if ( mCache )
  {
    mCache->deleteCachedGeometries();
  }

  updateFields();
  mDataProvider->updateExtents();

  mDataProvider->leaveUpdateMode();

  emit repaintRequested();

  return success;
}

QStringList QgsVectorLayer::commitErrors() const
{
  return mCommitErrors;
}

bool QgsVectorLayer::rollBack( bool deleteBuffer )
{
  if ( !mEditBuffer )
  {
    return false;
  }

  bool rollbackExtent = !mEditBuffer->mDeletedFeatureIds.isEmpty() ||
                        !mEditBuffer->mAddedFeatures.isEmpty() ||
                        !mEditBuffer->mChangedGeometries.isEmpty();

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
    mEditBuffer = nullptr;
    undoStack()->clear();
  }
  emit editingStopped();

  if ( mCache )
  {
    mCache->deleteCachedGeometries();
  }

  if ( rollbackExtent )
    updateExtents();

  mDataProvider->leaveUpdateMode();

  emit repaintRequested();
  return true;
}

int QgsVectorLayer::selectedFeatureCount() const
{
  return mSelectedFeatureIds.size();
}

const QgsFeatureIds &QgsVectorLayer::selectedFeatureIds() const
{
  return mSelectedFeatureIds;
}

QgsFeatureList QgsVectorLayer::selectedFeatures() const
{
  QgsFeatureList features;
  QgsFeature f;

  if ( mSelectedFeatureIds.count() <= 8 )
  {
    // for small amount of selected features, fetch them directly
    // because request with FilterFids would go iterate over the whole layer
    Q_FOREACH ( QgsFeatureId fid, mSelectedFeatureIds )
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

QgsFeatureIterator QgsVectorLayer::selectedFeaturesIterator( QgsFeatureRequest request ) const
{
  if ( mSelectedFeatureIds.isEmpty() )
    return QgsFeatureIterator();

  if ( geometryType() == QgsWkbTypes::NullGeometry )
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

    selectByIds( ids );
  }

  updateExtents();

  return res;
}


bool QgsVectorLayer::snapPoint( QgsPoint &point, double tolerance )
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


int QgsVectorLayer::snapWithContext( const QgsPoint &startPoint, double snappingTolerance,
                                     QMultiMap<double, QgsSnappingResult> &snappingResults,
                                     QgsSnapper::SnappingType snap_to )
{
  if ( !hasGeometryType() )
    return 1;

  if ( snappingTolerance <= 0 || !mDataProvider )
  {
    return 1;
  }

  QgsRectangle searchRect( startPoint.x() - snappingTolerance, startPoint.y() - snappingTolerance,
                           startPoint.x() + snappingTolerance, startPoint.y() + snappingTolerance );
  double sqrSnappingTolerance = snappingTolerance * snappingTolerance;

  int n = 0;
  QgsFeature f;

  if ( mCache->cachedGeometriesRect().contains( searchRect ) )
  {
    QgsGeometryMap &cachedGeometries = mCache->cachedGeometries();
    for ( QgsGeometryMap::iterator it = cachedGeometries.begin(); it != cachedGeometries.end() ; ++it )
    {
      QgsGeometry g = it.value();
      if ( g.boundingBox().intersects( searchRect ) )
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
      snapToGeometry( startPoint, f.id(), f.geometry(), sqrSnappingTolerance, snappingResults, snap_to );
      ++n;
    }
  }

  return n == 0 ? 2 : 0;
}

void QgsVectorLayer::snapToGeometry( const QgsPoint &startPoint,
                                     QgsFeatureId featureId,
                                     const QgsGeometry &geom,
                                     double sqrSnappingTolerance,
                                     QMultiMap<double, QgsSnappingResult> &snappingResults,
                                     QgsSnapper::SnappingType snap_to ) const
{
  if ( geom.isNull() )
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
    snappedPoint = geom.closestVertex( startPoint, atVertex, beforeVertex, afterVertex, sqrDistVertexSnap );
    if ( sqrDistVertexSnap < sqrSnappingTolerance )
    {
      snappingResultVertex.snappedVertex = snappedPoint;
      snappingResultVertex.snappedVertexNr = atVertex;
      snappingResultVertex.beforeVertexNr = beforeVertex;
      if ( beforeVertex != -1 ) // make sure the vertex is valid
      {
        snappingResultVertex.beforeVertex = geom.vertexAt( beforeVertex );
      }
      snappingResultVertex.afterVertexNr = afterVertex;
      if ( afterVertex != -1 ) // make sure the vertex is valid
      {
        snappingResultVertex.afterVertex = geom.vertexAt( afterVertex );
      }
      snappingResultVertex.snappedAtGeometry = featureId;
      snappingResultVertex.layer = this;
      snappingResults.insert( sqrt( sqrDistVertexSnap ), snappingResultVertex );
      return;
    }
  }
  if ( snap_to == QgsSnapper::SnapToSegment || snap_to == QgsSnapper::SnapToVertexAndSegment ) // snap to segment
  {
    if ( geometryType() != QgsWkbTypes::PointGeometry ) // cannot snap to segment for points/multipoints
    {
      sqrDistSegmentSnap = geom.closestSegmentWithContext( startPoint, snappedPoint, afterVertex, nullptr, crs().isGeographic() ? 1e-12 : 1e-8 );

      if ( sqrDistSegmentSnap < sqrSnappingTolerance )
      {
        snappingResultSegment.snappedVertex = snappedPoint;
        snappingResultSegment.snappedVertexNr = -1;
        snappingResultSegment.beforeVertexNr = afterVertex - 1;
        snappingResultSegment.afterVertexNr = afterVertex;
        snappingResultSegment.snappedAtGeometry = featureId;
        snappingResultSegment.beforeVertex = geom.vertexAt( afterVertex - 1 );
        snappingResultSegment.afterVertex = geom.vertexAt( afterVertex );
        snappingResultSegment.layer = this;
        snappingResults.insert( sqrt( sqrDistSegmentSnap ), snappingResultSegment );
      }
    }
  }
}

int QgsVectorLayer::insertSegmentVerticesForSnap( const QList<QgsSnappingResult> &snapResults )
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


QString QgsVectorLayer::displayField() const
{
  QgsExpression exp( mDisplayExpression );
  if ( exp.isField() )
  {
    return static_cast<const QgsExpression::NodeColumnRef *>( exp.rootNode() )->name();
  }

  return QString();
}

void QgsVectorLayer::setDisplayExpression( const QString &displayExpression )
{
  if ( mDisplayExpression == displayExpression )
    return;

  mDisplayExpression = displayExpression;
  emit displayExpressionChanged();
}

QString QgsVectorLayer::displayExpression() const
{
  if ( !mDisplayExpression.isEmpty() || mFields.isEmpty() )
  {
    return mDisplayExpression;
  }
  else
  {
    QString idxName;

    Q_FOREACH ( const QgsField &field, mFields )
    {
      QString fldName = field.name();

      // Check the fields and keep the first one that matches.
      // We assume that the user has organized the data with the
      // more "interesting" field names first. As such, name should
      // be selected before oldname, othername, etc.
      if ( fldName.indexOf( QLatin1String( "name" ), 0, Qt::CaseInsensitive ) > -1 )
      {
        idxName = fldName;
        break;
      }
      if ( fldName.indexOf( QLatin1String( "descrip" ), 0, Qt::CaseInsensitive ) > -1 )
      {
        idxName = fldName;
        break;
      }
      if ( fldName.indexOf( QLatin1String( "id" ), 0, Qt::CaseInsensitive ) > -1 )
      {
        idxName = fldName;
        break;
      }
    }

    if ( !idxName.isNull() )
    {
      return QgsExpression::quotedColumnRef( idxName );
    }
    else
    {
      return QgsExpression::quotedColumnRef( mFields.at( 0 ).name() );
    }
  }
}

bool QgsVectorLayer::isEditable() const
{
  return ( mEditBuffer && mDataProvider );
}

bool QgsVectorLayer::isSpatial() const
{
  return geometryType() != QgsWkbTypes::NullGeometry;
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
  emit readOnlyChanged();
  return true;
}

bool QgsVectorLayer::isModified() const
{
  emit beforeModifiedCheck();
  return mEditBuffer && mEditBuffer->isModified();
}

void QgsVectorLayer::setAnnotationForm( const QString &ui )
{
  mAnnotationForm = ui;
}

void QgsVectorLayer::setRenderer( QgsFeatureRenderer *r )
{
  if ( !hasGeometryType() )
    return;

  if ( r != mRenderer )
  {
    delete mRenderer;
    mRenderer = r;
    mSymbolFeatureCounted = false;
    mSymbolFeatureCountMap.clear();

    emit rendererChanged();
    emit styleChanged();
  }
}

void QgsVectorLayer::beginEditCommand( const QString &text )
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
    if ( !mDeletedFids.isEmpty() )
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

bool QgsVectorLayer::addJoin( const QgsVectorLayerJoinInfo &joinInfo )
{
  return mJoinBuffer->addJoin( joinInfo );
}


bool QgsVectorLayer::removeJoin( const QString &joinLayerId )
{
  return mJoinBuffer->removeJoin( joinLayerId );
}

const QList< QgsVectorLayerJoinInfo > QgsVectorLayer::vectorJoins() const
{
  return mJoinBuffer->vectorJoins();
}

int QgsVectorLayer::addExpressionField( const QString &exp, const QgsField &fld )
{
  emit beforeAddingExpressionField( fld.name() );
  mExpressionFieldBuffer->addExpression( exp, fld );
  updateFields();
  int idx = mFields.indexFromName( fld.name() );
  emit attributeAdded( idx );
  return idx;
}

void QgsVectorLayer::removeExpressionField( int index )
{
  emit beforeRemovingExpressionField( index );
  int oi = mFields.fieldOriginIndex( index );
  mExpressionFieldBuffer->removeExpression( oi );
  updateFields();
  emit attributeDeleted( index );
}

QString QgsVectorLayer::expressionField( int index ) const
{
  int oi = mFields.fieldOriginIndex( index );
  if ( oi < 0 || oi >= mExpressionFieldBuffer->expressions().size() )
    return QString();

  return mExpressionFieldBuffer->expressions().at( oi ).cachedExpression.expression();
}

void QgsVectorLayer::updateExpressionField( int index, const QString &exp )
{
  int oi = mFields.fieldOriginIndex( index );
  mExpressionFieldBuffer->updateExpression( oi, exp );
}

void QgsVectorLayer::updateFields()
{
  if ( !mDataProvider )
    return;

  QgsFields oldFields = mFields;

  mFields = mDataProvider->fields();

  // added / removed fields
  if ( mEditBuffer )
    mEditBuffer->updateFields( mFields );

  // joined fields
  if ( mJoinBuffer->containsJoins() )
    mJoinBuffer->updateFields( mFields );

  if ( mExpressionFieldBuffer )
    mExpressionFieldBuffer->updateFields( mFields );

  // set aliases and default values
  QMap< QString, QString >::const_iterator aliasIt = mAttributeAliasMap.constBegin();
  for ( ; aliasIt != mAttributeAliasMap.constEnd(); ++aliasIt )
  {
    int index = mFields.lookupField( aliasIt.key() );
    if ( index < 0 )
      continue;

    mFields[ index ].setAlias( aliasIt.value() );
  }
  QMap< QString, QString >::const_iterator defaultIt = mDefaultExpressionMap.constBegin();
  for ( ; defaultIt != mDefaultExpressionMap.constEnd(); ++defaultIt )
  {
    int index = mFields.lookupField( defaultIt.key() );
    if ( index < 0 )
      continue;

    mFields[ index ].setDefaultValueExpression( defaultIt.value() );
  }

  QMap< QString, QgsFieldConstraints::Constraints >::const_iterator constraintIt = mFieldConstraints.constBegin();
  for ( ; constraintIt != mFieldConstraints.constEnd(); ++constraintIt )
  {
    int index = mFields.lookupField( constraintIt.key() );
    if ( index < 0 )
      continue;

    QgsFieldConstraints constraints = mFields.at( index ).constraints();

    // always keep provider constraints intact
    if ( !( constraints.constraints() & QgsFieldConstraints::ConstraintNotNull ) && ( constraintIt.value() & QgsFieldConstraints::ConstraintNotNull ) )
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginLayer );
    if ( !( constraints.constraints() & QgsFieldConstraints::ConstraintUnique ) && ( constraintIt.value() & QgsFieldConstraints::ConstraintUnique ) )
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginLayer );
    if ( !( constraints.constraints() & QgsFieldConstraints::ConstraintExpression ) && ( constraintIt.value() & QgsFieldConstraints::ConstraintExpression ) )
      constraints.setConstraint( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintOriginLayer );
    mFields[ index ].setConstraints( constraints );
  }

  QMap< QString, QPair< QString, QString > >::const_iterator constraintExpIt = mFieldConstraintExpressions.constBegin();
  for ( ; constraintExpIt != mFieldConstraintExpressions.constEnd(); ++constraintExpIt )
  {
    int index = mFields.lookupField( constraintExpIt.key() );
    if ( index < 0 )
      continue;

    QgsFieldConstraints constraints = mFields.at( index ).constraints();

    // always keep provider constraints intact
    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintOriginProvider )
      continue;

    constraints.setConstraintExpression( constraintExpIt.value().first, constraintExpIt.value().second );
    mFields[ index ].setConstraints( constraints );
  }

  QMap< QPair< QString, QgsFieldConstraints::Constraint >, QgsFieldConstraints::ConstraintStrength >::const_iterator constraintStrengthIt = mFieldConstraintStrength.constBegin();
  for ( ; constraintStrengthIt != mFieldConstraintStrength.constEnd(); ++constraintStrengthIt )
  {
    int index = mFields.lookupField( constraintStrengthIt.key().first );
    if ( index < 0 )
      continue;

    QgsFieldConstraints constraints = mFields.at( index ).constraints();

    // always keep provider constraints intact
    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintOriginProvider )
      continue;

    constraints.setConstraintStrength( constraintStrengthIt.key().second, constraintStrengthIt.value() );
    mFields[ index ].setConstraints( constraints );
  }

  auto fieldWidgetIterator = mFieldWidgetSetups.constBegin();
  for ( ; fieldWidgetIterator != mFieldWidgetSetups.constEnd(); ++ fieldWidgetIterator )
  {
    int index = mFields.indexOf( fieldWidgetIterator.key() );
    if ( index < 0 )
      continue;

    mFields[index].setEditorWidgetSetup( fieldWidgetIterator.value() );
  }

  if ( oldFields != mFields )
  {
    emit updatedFields();
    mEditFormConfig.setFields( mFields );
  }
}


QVariant QgsVectorLayer::defaultValue( int index, const QgsFeature &feature, QgsExpressionContext *context ) const
{
  if ( index < 0 || index >= mFields.count() )
    return QVariant();

  QString expression = mFields.at( index ).defaultValueExpression();
  if ( expression.isEmpty() )
    return mDataProvider->defaultValue( index );

  QgsExpressionContext *evalContext = context;
  std::unique_ptr< QgsExpressionContext > tempContext;
  if ( !evalContext )
  {
    // no context passed, so we create a default one
    tempContext.reset( new QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( this ) ) );
    evalContext = tempContext.get();
  }

  if ( feature.isValid() )
  {
    QgsExpressionContextScope *featScope = new QgsExpressionContextScope();
    featScope->setFeature( feature );
    featScope->setFields( feature.fields() );
    evalContext->appendScope( featScope );
  }

  QVariant val;
  QgsExpression exp( expression );
  exp.prepare( evalContext );
  if ( exp.hasEvalError() )
  {
    QgsLogger::warning( "Error evaluating default value: " + exp.evalErrorString() );
  }
  else
  {
    val = exp.evaluate( evalContext );
  }

  if ( feature.isValid() )
  {
    delete evalContext->popScope();
  }

  return val;
}

void QgsVectorLayer::setDefaultValueExpression( int index, const QString &expression )
{
  if ( index < 0 || index >= mFields.count() )
    return;

  if ( expression.isEmpty() )
  {
    mDefaultExpressionMap.remove( mFields.at( index ).name() );
  }
  else
  {
    mDefaultExpressionMap.insert( mFields.at( index ).name(), expression );
  }
  updateFields();
}

QString QgsVectorLayer::defaultValueExpression( int index ) const
{
  if ( index < 0 || index >= mFields.count() )
    return QString();
  else
    return mFields.at( index ).defaultValueExpression();
}

void QgsVectorLayer::uniqueValues( int index, QList<QVariant> &uniqueValues, int limit ) const
{
  uniqueValues.clear();
  if ( !mDataProvider )
  {
    return;
  }

  QgsFields::FieldOrigin origin = mFields.fieldOrigin( index );
  switch ( origin )
  {
    case QgsFields::OriginUnknown:
      return;

    case QgsFields::OriginProvider: //a provider field
    {
      mDataProvider->uniqueValues( index, uniqueValues, limit );

      if ( mEditBuffer )
      {
        QSet<QString> vals;
        Q_FOREACH ( const QVariant &v, uniqueValues )
        {
          vals << v.toString();
        }

        QgsFeatureMap added = mEditBuffer->addedFeatures();
        QMapIterator< QgsFeatureId, QgsFeature > addedIt( added );
        while ( addedIt.hasNext() && ( limit < 0 || uniqueValues.count() < limit ) )
        {
          addedIt.next();
          QVariant v = addedIt.value().attribute( index );
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

    case QgsFields::OriginEdit:
      // the layer is editable, but in certain cases it can still be avoided going through all features
      if ( mEditBuffer->mDeletedFeatureIds.isEmpty() &&
           mEditBuffer->mAddedFeatures.isEmpty() &&
           !mEditBuffer->mDeletedAttributeIds.contains( index ) &&
           mEditBuffer->mChangedAttributeValues.isEmpty() )
      {
        mDataProvider->uniqueValues( index, uniqueValues, limit );
        return;
      }
      FALLTHROUGH;
    //we need to go through each feature
    case QgsFields::OriginJoin:
    case QgsFields::OriginExpression:
    {
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
  }

  Q_ASSERT_X( false, "QgsVectorLayer::uniqueValues()", "Unknown source of the field!" );
}

QStringList QgsVectorLayer::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QStringList results;
  if ( !mDataProvider )
  {
    return results;
  }

  QgsFields::FieldOrigin origin = mFields.fieldOrigin( index );
  switch ( origin )
  {
    case QgsFields::OriginUnknown:
      return results;

    case QgsFields::OriginProvider: //a provider field
    {
      results = mDataProvider->uniqueStringsMatching( index, substring, limit, feedback );

      if ( mEditBuffer )
      {
        QgsFeatureMap added = mEditBuffer->addedFeatures();
        QMapIterator< QgsFeatureId, QgsFeature > addedIt( added );
        while ( addedIt.hasNext() && ( limit < 0 || results.count() < limit ) && ( !feedback || !feedback->isCanceled() ) )
        {
          addedIt.next();
          QVariant v = addedIt.value().attribute( index );
          if ( v.isValid() )
          {
            QString vs = v.toString();
            if ( vs.contains( substring, Qt::CaseInsensitive ) && !results.contains( vs ) )
            {
              results << vs;
            }
          }
        }

        QMapIterator< QgsFeatureId, QgsAttributeMap > it( mEditBuffer->changedAttributeValues() );
        while ( it.hasNext() && ( limit < 0 || results.count() < limit ) && ( !feedback || !feedback->isCanceled() ) )
        {
          it.next();
          QVariant v = it.value().value( index );
          if ( v.isValid() )
          {
            QString vs = v.toString();
            if ( vs.contains( substring, Qt::CaseInsensitive ) && !results.contains( vs ) )
            {
              results << vs;
            }
          }
        }
      }

      return results;
    }

    case QgsFields::OriginEdit:
      // the layer is editable, but in certain cases it can still be avoided going through all features
      if ( mEditBuffer->mDeletedFeatureIds.isEmpty() &&
           mEditBuffer->mAddedFeatures.isEmpty() &&
           !mEditBuffer->mDeletedAttributeIds.contains( index ) &&
           mEditBuffer->mChangedAttributeValues.isEmpty() )
      {
        return mDataProvider->uniqueStringsMatching( index, substring, limit, feedback );
      }
      FALLTHROUGH;
    //we need to go through each feature
    case QgsFields::OriginJoin:
    case QgsFields::OriginExpression:
    {
      QgsAttributeList attList;
      attList << index;

      QgsFeatureRequest request;
      request.setSubsetOfAttributes( attList );
      request.setFlags( QgsFeatureRequest::NoGeometry );
      QString fieldName = mFields.at( index ).name();
      request.setFilterExpression( QStringLiteral( "\"%1\" ILIKE '%%2%'" ).arg( fieldName, substring ) );
      QgsFeatureIterator fit = getFeatures( request );

      QgsFeature f;
      QString currentValue;
      while ( fit.nextFeature( f ) )
      {
        currentValue = f.attribute( index ).toString();
        if ( !results.contains( currentValue ) )
          results << currentValue;

        if ( ( limit >= 0 && results.size() >= limit ) || ( feedback && feedback->isCanceled() ) )
        {
          break;
        }
      }

      return results;
    }
  }

  Q_ASSERT_X( false, "QgsVectorLayer::uniqueStringsMatching()", "Unknown source of the field!" );
  return results;
}

QVariant QgsVectorLayer::minimumValue( int index ) const
{
  if ( !mDataProvider )
  {
    return QVariant();
  }

  QgsFields::FieldOrigin origin = mFields.fieldOrigin( index );

  switch ( origin )
  {
    case QgsFields::OriginUnknown:
      return QVariant();

    case QgsFields::OriginProvider: //a provider field
    {
      QVariant min = mDataProvider->minimumValue( index );
      if ( mEditBuffer )
      {
        QgsFeatureMap added = mEditBuffer->addedFeatures();
        QMapIterator< QgsFeatureId, QgsFeature > addedIt( added );
        while ( addedIt.hasNext() )
        {
          addedIt.next();
          QVariant v = addedIt.value().attribute( index );
          if ( v.isValid() && qgsVariantLessThan( v, min ) )
          {
            min = v;
          }
        }

        QMapIterator< QgsFeatureId, QgsAttributeMap > it( mEditBuffer->changedAttributeValues() );
        while ( it.hasNext() )
        {
          it.next();
          QVariant v = it.value().value( index );
          if ( v.isValid() && qgsVariantLessThan( v, min ) )
          {
            min = v;
          }
        }
      }
      return min;
    }

    case QgsFields::OriginEdit:
    {
      // the layer is editable, but in certain cases it can still be avoided going through all features
      if ( mEditBuffer->mDeletedFeatureIds.isEmpty() &&
           mEditBuffer->mAddedFeatures.isEmpty() && !
           mEditBuffer->mDeletedAttributeIds.contains( index ) &&
           mEditBuffer->mChangedAttributeValues.isEmpty() )
      {
        return mDataProvider->minimumValue( index );
      }
    }
    FALLTHROUGH;
    // no choice but to go through all features
    case QgsFields::OriginExpression:
    case QgsFields::OriginJoin:
    {
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
  }

  Q_ASSERT_X( false, "QgsVectorLayer::minimumValue()", "Unknown source of the field!" );
  return QVariant();
}

QVariant QgsVectorLayer::maximumValue( int index ) const
{
  if ( !mDataProvider )
  {
    return QVariant();
  }

  QgsFields::FieldOrigin origin = mFields.fieldOrigin( index );
  switch ( origin )
  {
    case QgsFields::OriginUnknown:
      return QVariant();

    case QgsFields::OriginProvider: //a provider field
    {
      QVariant min = mDataProvider->maximumValue( index );
      if ( mEditBuffer )
      {
        QgsFeatureMap added = mEditBuffer->addedFeatures();
        QMapIterator< QgsFeatureId, QgsFeature > addedIt( added );
        while ( addedIt.hasNext() )
        {
          addedIt.next();
          QVariant v = addedIt.value().attribute( index );
          if ( v.isValid() && qgsVariantGreaterThan( v, min ) )
          {
            min = v;
          }
        }

        QMapIterator< QgsFeatureId, QgsAttributeMap > it( mEditBuffer->changedAttributeValues() );
        while ( it.hasNext() )
        {
          it.next();
          QVariant v = it.value().value( index );
          if ( v.isValid() && qgsVariantGreaterThan( v, min ) )
          {
            min = v;
          }
        }
      }
      return min;
    }

    case QgsFields::OriginEdit:
      // the layer is editable, but in certain cases it can still be avoided going through all features
      if ( mEditBuffer->mDeletedFeatureIds.isEmpty() &&
           mEditBuffer->mAddedFeatures.isEmpty() &&
           !mEditBuffer->mDeletedAttributeIds.contains( index ) &&
           mEditBuffer->mChangedAttributeValues.isEmpty() )
      {
        return mDataProvider->maximumValue( index );
      }

      FALLTHROUGH;
    //no choice but to go through each feature
    case QgsFields::OriginJoin:
    case QgsFields::OriginExpression:
    {
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
  }

  Q_ASSERT_X( false, "QgsVectorLayer::maximumValue()", "Unknown source of the field!" );
  return QVariant();
}

QVariant QgsVectorLayer::aggregate( QgsAggregateCalculator::Aggregate aggregate, const QString &fieldOrExpression,
                                    const QgsAggregateCalculator::AggregateParameters &parameters, QgsExpressionContext *context, bool *ok ) const
{
  if ( ok )
    *ok = false;

  if ( !mDataProvider )
  {
    return QVariant();
  }

  // test if we are calculating based on a field
  int attrIndex = mFields.lookupField( fieldOrExpression );
  if ( attrIndex >= 0 )
  {
    // aggregate is based on a field - if it's a provider field, we could possibly hand over the calculation
    // to the provider itself
    QgsFields::FieldOrigin origin = mFields.fieldOrigin( attrIndex );
    if ( origin == QgsFields::OriginProvider )
    {
      bool providerOk = false;
      QVariant val = mDataProvider->aggregate( aggregate, attrIndex, parameters, context, providerOk );
      if ( providerOk )
      {
        // provider handled calculation
        if ( ok )
          *ok = true;
        return val;
      }
    }
  }

  // fallback to using aggregate calculator to determine aggregate
  QgsAggregateCalculator c( this );
  c.setParameters( parameters );
  return c.calculate( aggregate, fieldOrExpression, context, ok );
}

QList<QVariant> QgsVectorLayer::getValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly, QgsFeedback *feedback ) const
{
  QList<QVariant> values;

  std::unique_ptr<QgsExpression> expression;
  QgsExpressionContext context;

  int attrNum = mFields.lookupField( fieldOrExpression );

  if ( attrNum == -1 )
  {
    // try to use expression
    expression.reset( new QgsExpression( fieldOrExpression ) );
    context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( this ) );

    if ( expression->hasParserError() || !expression->prepare( &context ) )
    {
      ok = false;
      return values;
    }
  }

  QgsFeature f;
  QSet<QString> lst;
  if ( !expression )
    lst.insert( fieldOrExpression );
  else
    lst = expression->referencedColumns();

  QgsFeatureRequest request = QgsFeatureRequest()
                              .setFlags( ( expression && expression->needsGeometry() ) ?
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
    if ( feedback && feedback->isCanceled() )
    {
      ok = false;
      return values;
    }
  }
  ok = true;
  return values;
}

QList<double> QgsVectorLayer::getDoubleValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly, int *nullCount, QgsFeedback *feedback ) const
{
  QList<double> values;

  if ( nullCount )
    *nullCount = 0;

  QList<QVariant> variantValues = getValues( fieldOrExpression, ok, selectedOnly );
  if ( !ok )
    return values;

  bool convertOk;
  Q_FOREACH ( const QVariant &value, variantValues )
  {
    double val = value.toDouble( &convertOk );
    if ( convertOk )
      values << val;
    else if ( value.isNull() )
    {
      if ( nullCount )
        *nullCount += 1;
    }
    if ( feedback && feedback->isCanceled() )
    {
      ok = false;
      return values;
    }
  }
  ok = true;
  return values;
}

void QgsVectorLayer::setFeatureBlendMode( QPainter::CompositionMode featureBlendMode )
{
  mFeatureBlendMode = featureBlendMode;
  emit featureBlendModeChanged( featureBlendMode );
  emit styleChanged();
}

QPainter::CompositionMode QgsVectorLayer::featureBlendMode() const
{
  return mFeatureBlendMode;
}

void QgsVectorLayer::setLayerTransparency( int layerTransparency )
{
  mLayerTransparency = layerTransparency;
  emit layerTransparencyChanged( layerTransparency );
  emit styleChanged();
}

int QgsVectorLayer::layerTransparency() const
{
  return mLayerTransparency;
}



void QgsVectorLayer::readSldLabeling( const QDomNode &node )
{
  QDomElement element = node.toElement();
  if ( element.isNull() )
    return;

  QDomElement userStyleElem = element.firstChildElement( QStringLiteral( "UserStyle" ) );
  if ( userStyleElem.isNull() )
  {
    QgsDebugMsg( "Info: UserStyle element not found." );
    return;
  }

  QDomElement featureTypeStyleElem = userStyleElem.firstChildElement( QStringLiteral( "FeatureTypeStyle" ) );
  if ( featureTypeStyleElem.isNull() )
  {
    QgsDebugMsg( "Info: FeatureTypeStyle element not found." );
    return;
  }

  // use last rule
  QDomElement ruleElem = featureTypeStyleElem.lastChildElement( QStringLiteral( "Rule" ) );
  if ( ruleElem.isNull() )
  {
    QgsDebugMsg( "Info: Rule element not found." );
    return;
  }

  // use last text symbolizer
  QDomElement textSymbolizerElem = ruleElem.lastChildElement( QStringLiteral( "TextSymbolizer" ) );
  if ( textSymbolizerElem.isNull() )
  {
    QgsDebugMsg( "Info: TextSymbolizer element not found." );
    return;
  }

  // Label
  setCustomProperty( QStringLiteral( "labeling/enabled" ), false );
  QDomElement labelElem = textSymbolizerElem.firstChildElement( QStringLiteral( "Label" ) );
  if ( !labelElem.isNull() )
  {
    QDomElement propertyNameElem = labelElem.firstChildElement( QStringLiteral( "PropertyName" ) );
    if ( !propertyNameElem.isNull() )
    {
      // enable labeling
      setCustomProperty( QStringLiteral( "labeling" ), "pal" );
      setCustomProperty( QStringLiteral( "labeling/enabled" ), true );

      // set labeling defaults
      setCustomProperty( QStringLiteral( "labeling/fontFamily" ), "Sans-Serif" );
      setCustomProperty( QStringLiteral( "labeling/fontItalic" ), false );
      setCustomProperty( QStringLiteral( "labeling/fontSize" ), 10 );
      setCustomProperty( QStringLiteral( "labeling/fontSizeInMapUnits" ), false );
      setCustomProperty( QStringLiteral( "labeling/fontBold" ), false );
      setCustomProperty( QStringLiteral( "labeling/fontUnderline" ), false );
      setCustomProperty( QStringLiteral( "labeling/textColorR" ), 0 );
      setCustomProperty( QStringLiteral( "labeling/textColorG" ), 0 );
      setCustomProperty( QStringLiteral( "labeling/textColorB" ), 0 );
      setCustomProperty( QStringLiteral( "labeling/textTransp" ), 0 );
      setCustomProperty( QStringLiteral( "labeling/bufferDraw" ), false );
      setCustomProperty( QStringLiteral( "labeling/bufferSize" ), 1 );
      setCustomProperty( QStringLiteral( "labeling/bufferSizeInMapUnits" ), false );
      setCustomProperty( QStringLiteral( "labeling/bufferColorR" ), 255 );
      setCustomProperty( QStringLiteral( "labeling/bufferColorG" ), 255 );
      setCustomProperty( QStringLiteral( "labeling/bufferColorB" ), 255 );
      setCustomProperty( QStringLiteral( "labeling/bufferTransp" ), 0 );
      setCustomProperty( QStringLiteral( "labeling/placement" ), QgsPalLayerSettings::AroundPoint );
      setCustomProperty( QStringLiteral( "labeling/xOffset" ), 0 );
      setCustomProperty( QStringLiteral( "labeling/yOffset" ), 0 );
      setCustomProperty( QStringLiteral( "labeling/labelOffsetInMapUnits" ), false );
      setCustomProperty( QStringLiteral( "labeling/angleOffset" ), 0 );

      // label attribute
      QString labelAttribute = propertyNameElem.text();
      setCustomProperty( QStringLiteral( "labeling/fieldName" ), labelAttribute );
      setCustomProperty( QStringLiteral( "labeling/isExpression" ), false );

      int fieldIndex = mFields.lookupField( labelAttribute );
      if ( fieldIndex == -1 )
      {
        // label attribute is not in columns, check if it is an expression
        QgsExpression exp( labelAttribute );
        if ( !exp.hasEvalError() )
        {
          setCustomProperty( QStringLiteral( "labeling/isExpression" ), true );
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
  QDomElement fontElem = textSymbolizerElem.firstChildElement( QStringLiteral( "Font" ) );
  if ( !fontElem.isNull() )
  {
    QString cssName;
    QString elemText;
    QDomElement cssElem = fontElem.firstChildElement( QStringLiteral( "CssParameter" ) );
    while ( !cssElem.isNull() )
    {
      cssName = cssElem.attribute( QStringLiteral( "name" ), QStringLiteral( "not_found" ) );
      if ( cssName != QLatin1String( "not_found" ) )
      {
        elemText = cssElem.text();
        if ( cssName == QLatin1String( "font-family" ) )
        {
          setCustomProperty( QStringLiteral( "labeling/fontFamily" ), elemText );
        }
        else if ( cssName == QLatin1String( "font-style" ) )
        {
          setCustomProperty( QStringLiteral( "labeling/fontItalic" ), ( elemText == QLatin1String( "italic" ) ) || ( elemText == QLatin1String( "Italic" ) ) );
        }
        else if ( cssName == QLatin1String( "font-size" ) )
        {
          bool ok;
          int fontSize = elemText.toInt( &ok );
          if ( ok )
          {
            setCustomProperty( QStringLiteral( "labeling/fontSize" ), fontSize );
          }
        }
        else if ( cssName == QLatin1String( "font-weight" ) )
        {
          setCustomProperty( QStringLiteral( "labeling/fontBold" ), ( elemText == QLatin1String( "bold" ) ) || ( elemText == QLatin1String( "Bold" ) ) );
        }
        else if ( cssName == QLatin1String( "font-underline" ) )
        {
          setCustomProperty( QStringLiteral( "labeling/fontUnderline" ), ( elemText == QLatin1String( "underline" ) ) || ( elemText == QLatin1String( "Underline" ) ) );
        }
      }

      cssElem = cssElem.nextSiblingElement( QStringLiteral( "CssParameter" ) );
    }
  }

  // Fill
  QColor textColor = QgsOgcUtils::colorFromOgcFill( textSymbolizerElem.firstChildElement( QStringLiteral( "Fill" ) ) );
  if ( textColor.isValid() )
  {
    setCustomProperty( QStringLiteral( "labeling/textColorR" ), textColor.red() );
    setCustomProperty( QStringLiteral( "labeling/textColorG" ), textColor.green() );
    setCustomProperty( QStringLiteral( "labeling/textColorB" ), textColor.blue() );
    setCustomProperty( QStringLiteral( "labeling/textTransp" ), 100 - static_cast< int >( 100 * textColor.alphaF() ) );
  }

  // Halo
  QDomElement haloElem = textSymbolizerElem.firstChildElement( QStringLiteral( "Halo" ) );
  if ( !haloElem.isNull() )
  {
    setCustomProperty( QStringLiteral( "labeling/bufferDraw" ), true );
    setCustomProperty( QStringLiteral( "labeling/bufferSize" ), 1 );

    QDomElement radiusElem = haloElem.firstChildElement( QStringLiteral( "Radius" ) );
    if ( !radiusElem.isNull() )
    {
      bool ok;
      double bufferSize = radiusElem.text().toDouble( &ok );
      if ( ok )
      {
        setCustomProperty( QStringLiteral( "labeling/bufferSize" ), bufferSize );
      }
    }

    QColor bufferColor = QgsOgcUtils::colorFromOgcFill( haloElem.firstChildElement( QStringLiteral( "Fill" ) ) );
    if ( bufferColor.isValid() )
    {
      setCustomProperty( QStringLiteral( "labeling/bufferColorR" ), bufferColor.red() );
      setCustomProperty( QStringLiteral( "labeling/bufferColorG" ), bufferColor.green() );
      setCustomProperty( QStringLiteral( "labeling/bufferColorB" ), bufferColor.blue() );
      setCustomProperty( QStringLiteral( "labeling/bufferTransp" ), 100 - static_cast< int >( 100 * bufferColor.alphaF() ) );
    }
  }

  // LabelPlacement
  QDomElement labelPlacementElem = textSymbolizerElem.firstChildElement( QStringLiteral( "LabelPlacement" ) );
  if ( !labelPlacementElem.isNull() )
  {
    // PointPlacement
    QDomElement pointPlacementElem = labelPlacementElem.firstChildElement( QStringLiteral( "PointPlacement" ) );
    if ( !pointPlacementElem.isNull() )
    {
      setCustomProperty( QStringLiteral( "labeling/placement" ), QgsPalLayerSettings::OverPoint );

      QDomElement displacementElem = pointPlacementElem.firstChildElement( QStringLiteral( "Displacement" ) );
      if ( !displacementElem.isNull() )
      {
        QDomElement displacementXElem = displacementElem.firstChildElement( QStringLiteral( "DisplacementX" ) );
        if ( !displacementXElem.isNull() )
        {
          bool ok;
          double xOffset = displacementXElem.text().toDouble( &ok );
          if ( ok )
          {
            setCustomProperty( QStringLiteral( "labeling/xOffset" ), xOffset );
          }
        }
        QDomElement displacementYElem = displacementElem.firstChildElement( QStringLiteral( "DisplacementY" ) );
        if ( !displacementYElem.isNull() )
        {
          bool ok;
          double yOffset = displacementYElem.text().toDouble( &ok );
          if ( ok )
          {
            setCustomProperty( QStringLiteral( "labeling/yOffset" ), yOffset );
          }
        }
      }

      QDomElement rotationElem = pointPlacementElem.firstChildElement( QStringLiteral( "Rotation" ) );
      if ( !rotationElem.isNull() )
      {
        bool ok;
        double rotation = rotationElem.text().toDouble( &ok );
        if ( ok )
        {
          setCustomProperty( QStringLiteral( "labeling/angleOffset" ), rotation );
        }
      }
    }
  }
}

QgsEditFormConfig QgsVectorLayer::editFormConfig() const
{
  return mEditFormConfig;
}

void QgsVectorLayer::setEditFormConfig( const QgsEditFormConfig &editFormConfig )
{
  if ( mEditFormConfig == editFormConfig )
    return;

  mEditFormConfig = editFormConfig;
  emit editFormConfigChanged();
}

QString QgsVectorLayer::mapTipTemplate() const
{
  return mMapTipTemplate;
}

void QgsVectorLayer::setMapTipTemplate( const QString &mapTip )
{
  if ( mMapTipTemplate == mapTip )
    return;

  mMapTipTemplate = mapTip;
  emit mapTipTemplateChanged();
}

QgsAttributeTableConfig QgsVectorLayer::attributeTableConfig() const
{
  QgsAttributeTableConfig config = mAttributeTableConfig;

  if ( config.isEmpty() )
    config.update( fields() );

  return config;
}

void QgsVectorLayer::setAttributeTableConfig( const QgsAttributeTableConfig &attributeTableConfig )
{
  if ( mAttributeTableConfig != attributeTableConfig )
  {
    mAttributeTableConfig = attributeTableConfig;
    emit configChanged();
  }
}

QgsExpressionContext QgsVectorLayer::createExpressionContext() const
{
  return QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( this ) );
}

void QgsVectorLayer::setDiagramLayerSettings( const QgsDiagramLayerSettings &s )
{
  if ( !mDiagramLayerSettings )
    mDiagramLayerSettings = new QgsDiagramLayerSettings();
  *mDiagramLayerSettings = s;
}

QString QgsVectorLayer::htmlMetadata() const
{
  QString myMetadata = QStringLiteral( "<html>\n<body>\n" );

  // Identification section
  myMetadata += QLatin1String( "<h1>" ) % tr( "Identification" ) % QLatin1String( "</h1>\n<hr>\n<table class=\"list-view\">\n" );

  // ID
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "ID" ) % QLatin1String( "</td><td>" ) % id() % QLatin1String( "</td></tr>\n" );

  // original name
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Original" ) % QLatin1String( "</td><td>" ) % originalName() % QLatin1String( "</td></tr>\n" );

  // name
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Name" ) % QLatin1String( "</td><td>" ) % name() % QLatin1String( "</td></tr>\n" );

  // short
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Short" ) % QLatin1String( "</td><td>" ) % shortName() % QLatin1String( "</td></tr>\n" );

  // title
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Title" ) % QLatin1String( "</td><td>" ) % title() % QLatin1String( "</td></tr>\n" );

  // abstract
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Abstract" ) % QLatin1String( "</td><td>" ) % abstract() % QLatin1String( "</td></tr>\n" );

  // keywords
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Keywords" ) % QLatin1String( "</td><td>" ) % keywordList() % QLatin1String( "</td></tr>\n" );

  // lang, waiting for the proper metadata implementation QEP #91 Work package 2
  // myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Language" ) % QLatin1String( "</td><td>en-CA</td></tr>\n" );

  // comment
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Comment" ) % QLatin1String( "</td><td>" ) % dataComment() % QLatin1String( "</td></tr>\n" );

  // date, waiting for the proper metadata implementation QEP #91 Work package 2
  // myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Date" ) % QLatin1String( "</td><td>28/03/17</td></tr>\n" );

  // storage type
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Storage" ) % QLatin1String( "</td><td>" ) % storageType() % QLatin1String( "</td></tr>\n" );

  // data source
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Source" ) % QLatin1String( "</td><td>" ) % publicSource() % QLatin1String( "</td></tr>\n" );

  // encoding
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Encoding" ) % QLatin1String( "</td><td>" ) % dataProvider()->encoding() % QLatin1String( "</td></tr>\n" );

  // Section spatial
  myMetadata += QLatin1String( "</table>\n<br><br><h1>" ) % tr( "Spatial" ) % QLatin1String( "</h1>\n<hr>\n<table class=\"list-view\">\n" );

  // geom type
  QgsWkbTypes::GeometryType type =  geometryType();
  if ( type < 0 || type > QgsWkbTypes::NullGeometry )
  {
    QgsDebugMsg( "Invalid vector type" );
  }
  else
  {
    QString typeString( QgsWkbTypes::geometryDisplayString( geometryType() ) );
    myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Geometry" ) % QLatin1String( "</td><td>" ) % typeString % QLatin1String( "</td></tr>\n" );
  }

  // EPSG
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "CRS" ) % QLatin1String( "</td><td>" ) % crs().authid() % QLatin1String( " - " );
  myMetadata += crs().description() % QLatin1String( " - " );
  if ( crs().isGeographic() )
    myMetadata += tr( "Geographic" );
  else
    myMetadata += tr( "Projected" );
  myMetadata += QLatin1String( "</td></tr>\n" );

  // Extent
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Extent" ) % QLatin1String( "</td><td>" ) % extent().toString() % QLatin1String( "</td></tr>\n" );

  // unit
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Unit" ) % QLatin1String( "</td><td>" ) % QgsUnitTypes::toString( crs().mapUnits() ) % QLatin1String( "</td></tr>\n" );

  // max scale
  // myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Max scale" ) % QLatin1String( "</td><td>" ) % QString::number( maximumScale() ) % QLatin1String( "</td></tr>\n" );

  // min scale
  // myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Min scale" ) % QLatin1String( "</td><td>" ) % QString::number( minimumScale() ) % QLatin1String( "</td></tr>\n" );

  // feature count
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Feature count" ) % QLatin1String( "</td><td>" ) % QString::number( featureCount() ) % QLatin1String( "</td></tr>\n" );

  // Fields section
  myMetadata += QLatin1String( "</table>\n<br><br><h1>" ) % tr( "Fields" ) % QLatin1String( "</h1>\n<hr>\n<table class=\"list-view\">\n" );

  // primary key
  QgsAttributeList pkAttrList = pkAttributeList();
  if ( !pkAttrList.isEmpty() )
  {
    myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Primary key attributes" ) % QLatin1String( "</td><td>" );
    Q_FOREACH ( int idx, pkAttrList )
    {
      myMetadata += fields().at( idx ).name() % ' ';
    }
    myMetadata += QLatin1String( "</td></tr>\n" );
  }

  const QgsFields &myFields = pendingFields();

  // count fields
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Count" ) % QLatin1String( "</td><td>" ) % QString::number( myFields.size() ) % QLatin1String( "</td></tr>\n" );

  myMetadata += "</table>\n<br><table width=\"100%\" class=\"tabular-view\">\n";
  myMetadata += "<tr><th>" % tr( "Field" ) % "</th><th>" % tr( "Type" ) % "</th><th>" % tr( "Length" ) % "</th><th>" % tr( "Precision" ) % "</th><th>" % tr( "Comment" ) % "</th></tr>\n";

  for ( int i = 0; i < myFields.size(); ++i )
  {
    QgsField myField = myFields.at( i );
    QString rowClass = QString( "" );
    if ( i % 2 )
      rowClass = QString( "class=\"odd-row\"" );
    myMetadata += "<tr " % rowClass % "><td>" % myField.name() % "</td><td>" % myField.typeName() % "</td><td>" % QString::number( myField.length() ) % "</td><td>" % QString::number( myField.precision() ) % "</td><td>" % myField.comment() % "</td></tr>\n";
  }

  //close field list and start references
  myMetadata += QLatin1String( "</table>\n<br><br><h1>" ) % tr( "References" ) % QLatin1String( "</h1>\n<hr>\n<table class=\"list-view\">\n" );

  // data URL
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Data URL" ) % QLatin1String( "</td><td>" ) % dataUrl() % QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Data Format" ) % QLatin1String( "</td><td>" ) % dataUrlFormat() % QLatin1String( "</td></tr>\n" );

  // attribution
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Attribution" ) % QLatin1String( "</td><td>" ) % attribution() % QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Attribution URL" ) % QLatin1String( "</td><td>" ) % attributionUrl() % QLatin1String( "</td></tr>\n" );

  // metadata URL
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Metadata URL" ) % QLatin1String( "</td><td>" ) % metadataUrl() % QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Metadata Type" ) % QLatin1String( "</td><td>" ) % metadataUrlType() % QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Metadata Format" ) % QLatin1String( "</td><td>" ) % metadataUrlFormat() % QLatin1String( "</td></tr>\n" );

  // legend URL
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Legend URL" ) % QLatin1String( "</td><td>" ) % legendUrl() % QLatin1String( "</td></tr>\n" );
  myMetadata += QLatin1String( "<tr><td class=\"highlight\">" ) % tr( "Legend Format" ) % QLatin1String( "</td><td>" ) % legendUrlFormat() % QLatin1String( "</td></tr>\n" );

  myMetadata += QStringLiteral( "</body>\n</html>\n" );
  return myMetadata;
}

void QgsVectorLayer::invalidateSymbolCountedFlag()
{
  mSymbolFeatureCounted = false;
}

void QgsVectorLayer::onJoinedFieldsChanged()
{
  // some of the fields of joined layers have changed -> we need to update this layer's fields too
  updateFields();
}

void QgsVectorLayer::onFeatureDeleted( QgsFeatureId fid )
{
  if ( mEditCommandActive )
    mDeletedFids << fid;
  else
    emit featuresDeleted( QgsFeatureIds() << fid );

  emit featureDeleted( fid );
}

void QgsVectorLayer::onRelationsLoaded()
{
  mEditFormConfig.onRelationsLoaded();
}

QList<QgsRelation> QgsVectorLayer::referencingRelations( int idx ) const
{
  return QgsProject::instance()->relationManager()->referencingRelations( this, idx );
}

int QgsVectorLayer::listStylesInDatabase( QStringList &ids, QStringList &names, QStringList &descriptions, QString &msgError )
{
  std::unique_ptr<QLibrary> myLib( QgsProviderRegistry::instance()->providerLibrary( mProviderKey ) );
  if ( !myLib )
  {
    msgError = QObject::tr( "Unable to load %1 provider" ).arg( mProviderKey );
    return -1;
  }
  listStyles_t *listStylesExternalMethod = reinterpret_cast< listStyles_t * >( cast_to_fptr( myLib->resolve( "listStyles" ) ) );

  if ( !listStylesExternalMethod )
  {
    msgError = QObject::tr( "Provider %1 has no %2 method" ).arg( mProviderKey, QStringLiteral( "listStyles" ) );
    return -1;
  }

  return listStylesExternalMethod( mDataSource, ids, names, descriptions, msgError );
}

QString QgsVectorLayer::getStyleFromDatabase( const QString &styleId, QString &msgError )
{
  std::unique_ptr<QLibrary> myLib( QgsProviderRegistry::instance()->providerLibrary( mProviderKey ) );
  if ( !myLib )
  {
    msgError = QObject::tr( "Unable to load %1 provider" ).arg( mProviderKey );
    return QString();
  }
  getStyleById_t *getStyleByIdMethod = reinterpret_cast< getStyleById_t * >( cast_to_fptr( myLib->resolve( "getStyleById" ) ) );

  if ( !getStyleByIdMethod )
  {
    msgError = QObject::tr( "Provider %1 has no %2 method" ).arg( mProviderKey, QStringLiteral( "getStyleById" ) );
    return QString();
  }

  return getStyleByIdMethod( mDataSource, styleId, msgError );
}

bool QgsVectorLayer::deleteStyleFromDatabase( const QString &styleId, QString &msgError )
{
  std::unique_ptr<QLibrary> myLib( QgsProviderRegistry::instance()->providerLibrary( mProviderKey ) );
  if ( !myLib )
  {
    msgError = QObject::tr( "Unable to load %1 provider" ).arg( mProviderKey );
    return false;
  }
  deleteStyleById_t *deleteStyleByIdMethod = reinterpret_cast< deleteStyleById_t * >( cast_to_fptr( myLib->resolve( "deleteStyleById" ) ) );
  if ( !deleteStyleByIdMethod )
  {
    msgError = QObject::tr( "Provider %1 has no %2 method" ).arg( mProviderKey, QStringLiteral( "deleteStyleById" ) );
    return false;
  }
  return deleteStyleByIdMethod( mDataSource, styleId, msgError );
}


void QgsVectorLayer::saveStyleToDatabase( const QString &name, const QString &description,
    bool useAsDefault, const QString &uiFileContent, QString &msgError )
{

  QString sldStyle, qmlStyle;
  std::unique_ptr<QLibrary> myLib( QgsProviderRegistry::instance()->providerLibrary( mProviderKey ) );
  if ( !myLib )
  {
    msgError = QObject::tr( "Unable to load %1 provider" ).arg( mProviderKey );
    return;
  }
  saveStyle_t *saveStyleExternalMethod = reinterpret_cast< saveStyle_t * >( cast_to_fptr( myLib->resolve( "saveStyle" ) ) );

  if ( !saveStyleExternalMethod )
  {
    msgError = QObject::tr( "Provider %1 has no %2 method" ).arg( mProviderKey, QStringLiteral( "saveStyle" ) );
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



QString QgsVectorLayer::loadNamedStyle( const QString &theURI, bool &resultFlag )
{
  return loadNamedStyle( theURI, resultFlag, false );
}

QString QgsVectorLayer::loadNamedStyle( const QString &theURI, bool &resultFlag, bool loadFromLocalDB )
{
  QgsDataSourceUri dsUri( theURI );
  if ( !loadFromLocalDB && mDataProvider && mDataProvider->isSaveAndLoadStyleToDatabaseSupported() )
  {
    std::unique_ptr<QLibrary> myLib( QgsProviderRegistry::instance()->providerLibrary( mProviderKey ) );
    if ( myLib )
    {
      loadStyle_t *loadStyleExternalMethod = reinterpret_cast< loadStyle_t * >( cast_to_fptr( myLib->resolve( "loadStyle" ) ) );
      if ( loadStyleExternalMethod )
      {
        QString qml, errorMsg;
        qml = loadStyleExternalMethod( mDataSource, errorMsg );
        if ( !qml.isEmpty() )
        {
          QDomDocument myDocument( QStringLiteral( "qgis" ) );
          myDocument.setContent( qml );
          resultFlag = importNamedStyle( myDocument, errorMsg );
          return QObject::tr( "Loaded from Provider" );
        }
      }
    }
  }

  return QgsMapLayer::loadNamedStyle( theURI, resultFlag );
}

QSet<QgsMapLayerDependency> QgsVectorLayer::dependencies() const
{
  if ( mDataProvider )
    return mDataProvider->dependencies() + mDependencies;
  return mDependencies;
}

bool QgsVectorLayer::setDependencies( const QSet<QgsMapLayerDependency> &oDeps )
{
  QSet<QgsMapLayerDependency> deps;
  Q_FOREACH ( const QgsMapLayerDependency &dep, oDeps )
  {
    if ( dep.origin() == QgsMapLayerDependency::FromUser )
      deps << dep;
  }
  if ( hasDependencyCycle( deps ) )
    return false;

  QSet<QgsMapLayerDependency> toAdd = deps - dependencies();

  // disconnect layers that are not present in the list of dependencies anymore
  Q_FOREACH ( const QgsMapLayerDependency &dep, mDependencies )
  {
    QgsVectorLayer *lyr = static_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( dep.layerId() ) );
    if ( lyr == nullptr )
      continue;
    disconnect( lyr, &QgsVectorLayer::featureAdded, this, &QgsVectorLayer::dataChanged );
    disconnect( lyr, &QgsVectorLayer::featureDeleted, this, &QgsVectorLayer::dataChanged );
    disconnect( lyr, &QgsVectorLayer::geometryChanged, this, &QgsVectorLayer::dataChanged );
    disconnect( lyr, &QgsVectorLayer::dataChanged, this, &QgsVectorLayer::dataChanged );
    disconnect( lyr, &QgsVectorLayer::repaintRequested, this, &QgsVectorLayer::triggerRepaint );
  }

  // assign new dependencies
  if ( mDataProvider )
    mDependencies = mDataProvider->dependencies() + deps;
  else
    mDependencies = deps;
  emit dependenciesChanged();

  // connect to new layers
  Q_FOREACH ( const QgsMapLayerDependency &dep, mDependencies )
  {
    QgsVectorLayer *lyr = static_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( dep.layerId() ) );
    if ( lyr == nullptr )
      continue;
    connect( lyr, &QgsVectorLayer::featureAdded, this, &QgsVectorLayer::dataChanged );
    connect( lyr, &QgsVectorLayer::featureDeleted, this, &QgsVectorLayer::dataChanged );
    connect( lyr, &QgsVectorLayer::geometryChanged, this, &QgsVectorLayer::dataChanged );
    connect( lyr, &QgsVectorLayer::dataChanged, this, &QgsVectorLayer::dataChanged );
    connect( lyr, &QgsVectorLayer::repaintRequested, this, &QgsVectorLayer::triggerRepaint );
  }

  // if new layers are present, emit a data change
  if ( ! toAdd.isEmpty() )
    emit dataChanged();

  return true;
}

QgsFieldConstraints::Constraints QgsVectorLayer::fieldConstraints( int fieldIndex ) const
{
  if ( fieldIndex < 0 || fieldIndex >= mFields.count() )
    return nullptr;

  QgsFieldConstraints::Constraints constraints = mFields.at( fieldIndex ).constraints().constraints();

  // make sure provider constraints are always present!
  if ( mFields.fieldOrigin( fieldIndex ) == QgsFields::OriginProvider )
  {
    constraints |= mDataProvider->fieldConstraints( mFields.fieldOriginIndex( fieldIndex ) );
  }

  return constraints;
}

void QgsVectorLayer::setFieldConstraint( int index, QgsFieldConstraints::Constraint constraint, QgsFieldConstraints::ConstraintStrength strength )
{
  if ( index < 0 || index >= mFields.count() )
    return;

  QString name = mFields.at( index ).name();

  // add constraint to existing constraints
  QgsFieldConstraints::Constraints constraints = mFieldConstraints.value( name, nullptr );
  constraints |= constraint;
  mFieldConstraints.insert( name, constraints );

  mFieldConstraintStrength.insert( qMakePair( name, constraint ), strength );

  updateFields();
}

void QgsVectorLayer::removeFieldConstraint( int index, QgsFieldConstraints::Constraint constraint )
{
  if ( index < 0 || index >= mFields.count() )
    return;

  QString name = mFields.at( index ).name();

  // remove constraint from existing constraints
  QgsFieldConstraints::Constraints constraints = mFieldConstraints.value( name, nullptr );
  constraints &= ~constraint;
  mFieldConstraints.insert( name, constraints );

  mFieldConstraintStrength.remove( qMakePair( name, constraint ) );

  updateFields();
}

QString QgsVectorLayer::constraintExpression( int index ) const
{
  if ( index < 0 || index >= mFields.count() )
    return QString();

  return mFields.at( index ).constraints().constraintExpression();
}

QString QgsVectorLayer::constraintDescription( int index ) const
{
  if ( index < 0 || index >= mFields.count() )
    return QString();

  return mFields.at( index ).constraints().constraintDescription();
}

void QgsVectorLayer::setConstraintExpression( int index, const QString &expression, const QString &description )
{
  if ( index < 0 || index >= mFields.count() )
    return;

  if ( expression.isEmpty() )
  {
    mFieldConstraintExpressions.remove( mFields.at( index ).name() );
  }
  else
  {
    mFieldConstraintExpressions.insert( mFields.at( index ).name(), qMakePair( expression, description ) );
  }
  updateFields();
}

void QgsVectorLayer::setEditorWidgetSetup( int index, const QgsEditorWidgetSetup &setup )
{
  if ( index < 0 || index >= mFields.count() )
    return;

  if ( setup.isNull() )
    mFieldWidgetSetups.remove( mFields.at( index ).name() );
  else
    mFieldWidgetSetups.insert( mFields.at( index ).name(), setup );
  updateFields();
}

QgsEditorWidgetSetup QgsVectorLayer::editorWidgetSetup( int index ) const
{

  if ( index < 0 || index >= mFields.count() )
    return QgsEditorWidgetSetup();

  return mFields.at( index ).editorWidgetSetup();
}
