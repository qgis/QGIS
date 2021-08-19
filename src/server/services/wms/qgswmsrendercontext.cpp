/***************************************************************************
                              qgswmsrendercontext.cpp
                              ---------------------
  begin                : March 22, 2019
  copyright            : (C) 2019 by Paul Blottiere
  email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertree.h"

#include "qgsrasterlayer.h"
#include "qgswmsrendercontext.h"
#include "qgswmsserviceexception.h"
#include "qgsserverprojectutils.h"

using namespace QgsWms;

const double OGC_PX_M = 0.00028; // OGC reference pixel size in meter
QgsWmsRenderContext::QgsWmsRenderContext( const QgsProject *project, QgsServerInterface *interface )
  : mProject( project )
  , mInterface( interface )
  , mFlags()
{
}

QgsWmsRenderContext::~QgsWmsRenderContext()
{
  qDeleteAll( mExternalLayers );
  mExternalLayers.clear();
}

void QgsWmsRenderContext::setParameters( const QgsWmsParameters &parameters )
{
  mParameters = parameters;

  initRestrictedLayers();
  initNicknameLayers();

  searchLayersToRender();
  removeUnwantedLayers();
  checkLayerReadPermissions();

  std::reverse( mLayersToRender.begin(), mLayersToRender.end() );
}

void QgsWmsRenderContext::setFlag( const Flag flag, const bool on )
{
  if ( on )
  {
    mFlags |= flag;
  }
  else
  {
    mFlags &= ~flag;
  }
}

bool QgsWmsRenderContext::testFlag( Flag flag ) const
{
  return mFlags.testFlag( flag );
}

QgsWmsParameters QgsWmsRenderContext::parameters() const
{
  return mParameters;
}

const QgsServerSettings &QgsWmsRenderContext::settings() const
{
  return *mInterface->serverSettings();
}

const QgsProject *QgsWmsRenderContext::project() const
{
  return mProject;
}

QDomElement QgsWmsRenderContext::sld( const QgsMapLayer &layer ) const
{
  QDomElement sld;

  const QString nickname = layerNickname( layer );
  if ( mSlds.contains( nickname ) )
  {
    sld = mSlds[ nickname ];
  }

  return sld;
}

QString QgsWmsRenderContext::style( const QgsMapLayer &layer ) const
{
  QString style;

  const QString nickname = layerNickname( layer );
  if ( mStyles.contains( nickname ) )
  {
    style = mStyles[ nickname ];
  }

  return style;
}

QgsWmsParametersLayer QgsWmsRenderContext::parameters( const QgsMapLayer &layer ) const
{
  QgsWmsParametersLayer parameters;

  for ( const auto &params : mParameters.layersParameters() )
  {
    if ( params.mNickname == layerNickname( layer ) )
    {
      parameters = params;
      break;
    }
  }

  return parameters;
}

int QgsWmsRenderContext::imageQuality() const
{
  int imageQuality = QgsServerProjectUtils::wmsImageQuality( *mProject );

  if ( !mParameters.imageQuality().isEmpty() )
  {
    imageQuality = mParameters.imageQualityAsInt();
  }

  return imageQuality;
}

int QgsWmsRenderContext::tileBuffer() const
{
  int tileBuffer = 0;

  if ( mParameters.tiledAsBool() )
  {
    tileBuffer = QgsServerProjectUtils::wmsTileBuffer( *mProject );
  }

  return tileBuffer;
}

bool QgsWmsRenderContext::renderMapTiles() const
{
  return QgsServerProjectUtils::wmsRenderMapTiles( *mProject );
}

int QgsWmsRenderContext::precision() const
{
  int precision = QgsServerProjectUtils::wmsFeatureInfoPrecision( *mProject );

  if ( mParameters.wmsPrecisionAsInt() > -1 )
  {
    precision = mParameters.wmsPrecisionAsInt();
  }

  return precision;
}

qreal QgsWmsRenderContext::dotsPerMm() const
{
  // Apply DPI parameter if present. This is an extension of QGIS Server
  // compared to WMS 1.3.
  // Because of backwards compatibility, this parameter is optional
  qreal dpm = 1 / OGC_PX_M;

  if ( !mParameters.dpi().isEmpty() )
  {
    dpm = mParameters.dpiAsDouble() / 0.0254;
  }

  return dpm / 1000.0;
}

QStringList QgsWmsRenderContext::flattenedQueryLayers( const QStringList &layerNames ) const
{
  QStringList result;
  std::function <QStringList( const QString &name )> findLeaves = [ & ]( const QString & name ) -> QStringList
  {
    QStringList _result;
    if ( mLayerGroups.contains( name ) )
    {
      const auto &layers  { mLayerGroups[ name ] };
      for ( const auto &l : layers )
      {
        const auto nick { layerNickname( *l ) };
        // This handles the case for root (fake) group
        if ( mLayerGroups.contains( nick ) )
        {
          _result.append( name );
        }
        else
        {
          _result.append( findLeaves( nick ) );
        }
      }
    }
    else
    {
      _result.append( name );
    }
    return _result;
  };

  for ( const auto &name : std::as_const( layerNames ) )
  {
    result.append( findLeaves( name ) );
  }
  return result;
}

QList<QgsMapLayer *> QgsWmsRenderContext::layersToRender() const
{
  return mLayersToRender;
}

QList<QgsMapLayer *> QgsWmsRenderContext::layers() const
{
  return mNicknameLayers.values();
}

double QgsWmsRenderContext::scaleDenominator() const
{
  double denominator = -1;

  if ( mScaleDenominator >= 0 )
  {
    denominator = mScaleDenominator;
  }
  else if ( mFlags & UseScaleDenominator && ! mParameters.scale().isEmpty() )
  {
    denominator = mParameters.scaleAsDouble();
  }

  return denominator;
}

void QgsWmsRenderContext::setScaleDenominator( double scaleDenominator )
{
  mScaleDenominator = scaleDenominator;
  removeUnwantedLayers();
}

bool QgsWmsRenderContext::updateExtent() const
{
  bool update = false;

  if ( mFlags & UpdateExtent && ! mParameters.bbox().isEmpty() )
  {
    update = true;
  }

  return update;
}

QString QgsWmsRenderContext::layerNickname( const QgsMapLayer &layer ) const
{
  QString name = layer.shortName();
  if ( QgsServerProjectUtils::wmsUseLayerIds( *mProject ) )
  {
    name = layer.id();
  }
  else if ( name.isEmpty() )
  {
    name = layer.name();
  }

  return name;
}

QgsMapLayer *QgsWmsRenderContext::layer( const QString &nickname ) const
{
  QgsMapLayer *mlayer = nullptr;

  for ( auto layer : mLayersToRender )
  {
    if ( layerNickname( *layer ).compare( nickname ) == 0 )
    {
      mlayer = layer;
      break;
    }
  }

  return mlayer;
}

bool QgsWmsRenderContext::isValidLayer( const QString &nickname ) const
{
  return layer( nickname ) != nullptr;
}

QList<QgsMapLayer *> QgsWmsRenderContext::layersFromGroup( const QString &nickname ) const
{
  return mLayerGroups.value( nickname );
}

bool QgsWmsRenderContext::isValidGroup( const QString &name ) const
{
  return mLayerGroups.contains( name );
}

void QgsWmsRenderContext::initNicknameLayers()
{
  for ( QgsMapLayer *ml : mProject->mapLayers() )
  {
    mNicknameLayers.insert( layerNickname( *ml ), ml );
  }

  // init groups
  const QString rootName { QgsServerProjectUtils::wmsRootName( *mProject ) };
  const QgsLayerTreeGroup *root = mProject->layerTreeRoot();

  initLayerGroupsRecursive( root, rootName.isEmpty() ? mProject->title() : rootName );
}

void QgsWmsRenderContext::initLayerGroupsRecursive( const QgsLayerTreeGroup *group, const QString &groupName )
{
  if ( !groupName.isEmpty() )
  {
    mLayerGroups[groupName] = QList<QgsMapLayer *>();
    const auto projectLayerTreeRoot { mProject->layerTreeRoot() };
    const auto treeGroupLayers { group->findLayers() };
    // Fast track if there is no custom layer order,
    // otherwise reorder layers.
    if ( ! projectLayerTreeRoot->hasCustomLayerOrder() )
    {
      for ( const auto &tl : treeGroupLayers )
      {
        mLayerGroups[groupName].push_back( tl->layer() );
      }
    }
    else
    {
      const auto projectLayerOrder { projectLayerTreeRoot->layerOrder() };
      // Flat list containing the layers from the tree nodes
      QList<QgsMapLayer *> groupLayersList;
      for ( const auto &tl : treeGroupLayers )
      {
        groupLayersList << tl->layer();
      }
      for ( const auto &l : projectLayerOrder )
      {
        if ( groupLayersList.contains( l ) )
        {
          mLayerGroups[groupName].push_back( l );
        }
      }
    }
  }

  for ( const QgsLayerTreeNode *child : group->children() )
  {
    if ( child->nodeType() == QgsLayerTreeNode::NodeGroup )
    {
      QString name = child->customProperty( QStringLiteral( "wmsShortName" ) ).toString();

      if ( name.isEmpty() )
        name = child->name();

      initLayerGroupsRecursive( static_cast<const QgsLayerTreeGroup *>( child ), name );

    }
  }
}

void QgsWmsRenderContext::initRestrictedLayers()
{
  mRestrictedLayers.clear();

  // get name of restricted layers/groups in project
  const QStringList restricted = QgsServerProjectUtils::wmsRestrictedLayers( *mProject );

  // extract restricted layers from excluded groups
  QStringList restrictedLayersNames;
  QgsLayerTreeGroup *root = mProject->layerTreeRoot();

  for ( const QString &l : std::as_const( restricted ) )
  {
    const QgsLayerTreeGroup *group = root->findGroup( l );
    if ( group )
    {
      const QList<QgsLayerTreeLayer *> groupLayers = group->findLayers();
      for ( QgsLayerTreeLayer *treeLayer : groupLayers )
      {
        restrictedLayersNames.append( treeLayer->name() );
      }
    }
    else
    {
      restrictedLayersNames.append( l );
    }
  }

  // build output with names, ids or short name according to the configuration
  const QList<QgsLayerTreeLayer *> layers = root->findLayers();
  for ( QgsLayerTreeLayer *layer : layers )
  {
    if ( restrictedLayersNames.contains( layer->name() ) )
    {
      mRestrictedLayers.append( layerNickname( *layer->layer() ) );
    }
  }
}

void QgsWmsRenderContext::searchLayersToRender()
{
  mLayersToRender.clear();
  mStyles.clear();
  mSlds.clear();

  if ( ! mParameters.sldBody().isEmpty() )
  {
    searchLayersToRenderSld();
  }
  else
  {
    searchLayersToRenderStyle();
  }

  if ( mFlags & AddQueryLayers )
  {
    const QStringList queryLayerNames = flattenedQueryLayers( mParameters.queryLayersNickname() );
    for ( const QString &layerName : queryLayerNames )
    {
      const QList<QgsMapLayer *> layers = mNicknameLayers.values( layerName );
      for ( QgsMapLayer *lyr : layers )
        if ( !mLayersToRender.contains( lyr ) )
        {
          mLayersToRender.append( lyr );
        }
    }
  }

  if ( mFlags & AddAllLayers )
  {
    const QStringList queryLayerNames = flattenedQueryLayers( mParameters.allLayersNickname() );
    for ( const QString &layerName : queryLayerNames )
    {
      const QList<QgsMapLayer *> layers = mNicknameLayers.values( layerName );
      for ( QgsMapLayer *lyr : layers )
        if ( !mLayersToRender.contains( lyr ) )
        {
          mLayersToRender.append( lyr );
        }
    }
  }
}

void QgsWmsRenderContext::searchLayersToRenderSld()
{
  const QString sld = mParameters.sldBody();

  if ( sld.isEmpty() )
  {
    return;
  }

  QDomDocument doc;
  ( void )doc.setContent( sld, true );
  QDomElement docEl = doc.documentElement();

  QDomElement root = doc.firstChildElement( "StyledLayerDescriptor" );
  QDomElement namedElem = root.firstChildElement( "NamedLayer" );

  if ( docEl.isNull() )
  {
    return;
  }

  QDomNodeList named = docEl.elementsByTagName( "NamedLayer" );
  for ( int i = 0; i < named.size(); ++i )
  {
    QDomNodeList names = named.item( i ).toElement().elementsByTagName( "Name" );
    if ( !names.isEmpty() )
    {
      QString lname = names.item( 0 ).toElement().text();
      if ( mNicknameLayers.contains( lname ) )
      {
        mSlds[lname] = namedElem;
        mLayersToRender.append( mNicknameLayers.values( lname ) );
      }
      else if ( mLayerGroups.contains( lname ) )
      {
        for ( QgsMapLayer *layer : mLayerGroups[lname] )
        {
          const QString name = layerNickname( *layer );
          mSlds[name] = namedElem;
          mLayersToRender.insert( 0, layer );
        }
      }
      else
      {
        QgsWmsParameter param( QgsWmsParameter::LAYER );
        param.mValue = lname;
        throw QgsBadRequestException( QgsServiceException::OGC_LayerNotDefined,
                                      param );
      }
    }
  }
}

void QgsWmsRenderContext::searchLayersToRenderStyle()
{
  for ( const QgsWmsParametersLayer &param : mParameters.layersParameters() )
  {
    const QString nickname = param.mNickname;
    const QString style = param.mStyle;

    if ( ! param.mExternalUri.isEmpty() && ( mFlags & AddExternalLayers ) )
    {
      std::unique_ptr<QgsMapLayer> layer = std::make_unique< QgsRasterLayer >( param.mExternalUri, param.mNickname, QStringLiteral( "wms" ) );

      if ( layer->isValid() )
      {
        // to delete later
        mExternalLayers.append( layer.release() );
        mLayersToRender.append( mExternalLayers.last() );
      }
    }
    else if ( mNicknameLayers.contains( nickname ) )
    {
      if ( !style.isEmpty() )
      {
        mStyles[nickname] = style;
      }

      mLayersToRender.append( mNicknameLayers.values( nickname ) );
    }
    else if ( mLayerGroups.contains( nickname ) )
    {
      // Reverse order of layers from a group
      QList<QString> layersFromGroup;
      for ( QgsMapLayer *layer : mLayerGroups[nickname] )
      {
        const QString nickname = layerNickname( *layer );
        if ( !style.isEmpty() )
        {
          mStyles[ nickname ] = style;
        }
        layersFromGroup.push_front( nickname );
      }

      for ( const auto &name : layersFromGroup )
      {
        mLayersToRender.append( mNicknameLayers.values( name ) );
      }
    }
    else
    {
      QgsWmsParameter param( QgsWmsParameter::LAYER );
      param.mValue = nickname;
      throw QgsBadRequestException( QgsServiceException::OGC_LayerNotDefined,
                                    param );
    }
  }
}

bool QgsWmsRenderContext::layerScaleVisibility( const QString &name ) const
{
  bool visible = false;

  if ( ! mNicknameLayers.contains( name ) )
  {
    return visible;
  }

  const QList<QgsMapLayer *>layers = mNicknameLayers.values( name );
  for ( QgsMapLayer *layer : layers )
  {
    bool scaleBasedVisibility = layer->hasScaleBasedVisibility();
    bool useScaleConstraint = ( scaleDenominator() > 0 && scaleBasedVisibility );

    if ( !useScaleConstraint || layer->isInScaleRange( scaleDenominator() ) )
    {
      visible = true;
    }
  }

  return visible;
}

QMap<QString, QList<QgsMapLayer *> > QgsWmsRenderContext::layerGroups() const
{
  return mLayerGroups;
}

int QgsWmsRenderContext::mapWidth() const
{
  int width = mParameters.widthAsInt();

  // May use SRCWIDTH to define image map size
  if ( ( mFlags & UseSrcWidthHeight ) && mParameters.srcWidthAsInt() > 0 )
  {
    width = mParameters.srcWidthAsInt();
  }

  return width;
}

int QgsWmsRenderContext::mapHeight() const
{
  int height = mParameters.heightAsInt();

  // May use SRCHEIGHT to define image map size
  if ( ( mFlags & UseSrcWidthHeight ) && mParameters.srcHeightAsInt() > 0 )
  {
    height = mParameters.srcHeightAsInt();
  }

  return height;
}

bool QgsWmsRenderContext::isValidWidthHeight() const
{
  return isValidWidthHeight( mapWidth(), mapHeight() );
}

bool QgsWmsRenderContext::isValidWidthHeight( int width, int height ) const
{
  //test if maxWidth / maxHeight are set in the project or as an env variable
  //and WIDTH / HEIGHT parameter is in the range allowed range
  //WIDTH
  const int wmsMaxWidthProj = QgsServerProjectUtils::wmsMaxWidth( *mProject );
  const int wmsMaxWidthEnv = settings().wmsMaxWidth();
  int wmsMaxWidth;
  if ( wmsMaxWidthEnv != -1 && wmsMaxWidthProj != -1 )
  {
    // both are set, so we take the more conservative one
    wmsMaxWidth = std::min( wmsMaxWidthProj, wmsMaxWidthEnv );
  }
  else
  {
    // none or one are set, so we take the bigger one which is the one set or -1
    wmsMaxWidth = std::max( wmsMaxWidthProj, wmsMaxWidthEnv );
  }

  if ( wmsMaxWidth != -1 && width > wmsMaxWidth )
  {
    return false;
  }

  //HEIGHT
  const int wmsMaxHeightProj = QgsServerProjectUtils::wmsMaxHeight( *mProject );
  const int wmsMaxHeightEnv = settings().wmsMaxHeight();
  int wmsMaxHeight;
  if ( wmsMaxHeightEnv != -1 && wmsMaxHeightProj != -1 )
  {
    // both are set, so we take the more conservative one
    wmsMaxHeight = std::min( wmsMaxHeightProj, wmsMaxHeightEnv );
  }
  else
  {
    // none or one are set, so we take the bigger one which is the one set or -1
    wmsMaxHeight = std::max( wmsMaxHeightProj, wmsMaxHeightEnv );
  }

  if ( wmsMaxHeight != -1 && height > wmsMaxHeight )
  {
    return false;
  }

  // Sanity check from internal QImage checks (see qimage.cpp)
  // this is to report a meaningful error message in case of
  // image creation failure and to differentiate it from out
  // of memory conditions.

  // depth for now it cannot be anything other than 32, but I don't like
  // to hardcode it: I hope we will support other depths in the future.
  uint depth = 32;
  switch ( mParameters.format() )
  {
    case QgsWmsParameters::Format::JPG:
    case QgsWmsParameters::Format::PNG:
    default:
      depth = 32;
  }

  const int bytes_per_line = ( ( width * depth + 31 ) >> 5 ) << 2; // bytes per scanline (must be multiple of 4)

  if ( std::numeric_limits<int>::max() / depth < static_cast<uint>( width )
       || bytes_per_line <= 0
       || height <= 0
       || std::numeric_limits<int>::max() / static_cast<uint>( bytes_per_line ) < static_cast<uint>( height )
       || std::numeric_limits<int>::max() / sizeof( uchar * ) < static_cast<uint>( height ) )
  {
    return false;
  }

  return true;
}

double QgsWmsRenderContext::mapTileBuffer( const int mapWidth ) const
{
  double buffer;
  if ( mFlags & UseTileBuffer )
  {
    const QgsRectangle extent = mParameters.bboxAsRectangle();
    if ( !mParameters.bbox().isEmpty() && extent.isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    mParameters[QgsWmsParameter::BBOX] );
    }
    buffer = tileBuffer() * ( extent.width() / mapWidth );
  }
  else
  {
    buffer = 0;
  }
  return buffer;
}

QSize QgsWmsRenderContext::mapSize( const bool aspectRatio ) const
{
  int width = mapWidth();
  int height = mapHeight();

  // Adapt width / height if the aspect ratio does not correspond with the BBOX.
  // Required by WMS spec. 1.3.
  if ( aspectRatio
       && mParameters.versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) )
  {
    QgsRectangle extent = mParameters.bboxAsRectangle();
    if ( !mParameters.bbox().isEmpty() && extent.isEmpty() )
    {
      throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                    mParameters[QgsWmsParameter::BBOX] );
    }

    QString crs = mParameters.crs();
    if ( crs.compare( "CRS:84", Qt::CaseInsensitive ) == 0 )
    {
      crs = QString( "EPSG:4326" );
      extent.invert();
    }

    QgsCoordinateReferenceSystem outputCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
    if ( outputCrs.hasAxisInverted() )
    {
      extent.invert();
    }

    if ( !extent.isEmpty() && height > 0 && width > 0 )
    {
      const double mapRatio = extent.width() / extent.height();
      const double imageRatio = static_cast<double>( width ) / static_cast<double>( height );
      if ( !qgsDoubleNear( mapRatio, imageRatio, 0.0001 ) )
      {
        // inspired by MapServer, mapdraw.c L115
        const double cellsize = ( extent.width() / static_cast<double>( width ) ) * 0.5 + ( extent.height() / static_cast<double>( height ) ) * 0.5;
        width = extent.width() / cellsize;
        height = extent.height() / cellsize;
      }
    }
  }

  if ( width <= 0 )
  {
    throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                  mParameters[QgsWmsParameter::WIDTH] );
  }
  else if ( height <= 0 )
  {
    throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue,
                                  mParameters[QgsWmsParameter::HEIGHT] );
  }

  return QSize( width, height );
}

void QgsWmsRenderContext::removeUnwantedLayers()
{
  QList<QgsMapLayer *> layers;

  for ( QgsMapLayer *layer : mLayersToRender )
  {
    const QString nickname = layerNickname( *layer );

    if ( ! isExternalLayer( nickname ) )
    {
      if ( !layerScaleVisibility( nickname ) )
        continue;

      if ( mRestrictedLayers.contains( nickname ) )
        continue;

      if ( mFlags & UseWfsLayersOnly )
      {
        if ( layer->type() != QgsMapLayerType::VectorLayer )
        {
          continue;
        }

        const QStringList wfsLayers = QgsServerProjectUtils::wfsLayerIds( *mProject );
        if ( ! wfsLayers.contains( layer->id() ) )
        {
          continue;
        }
      }
    }

    layers.append( layer );
  }

  mLayersToRender = layers;
}

bool QgsWmsRenderContext::isExternalLayer( const QString &name ) const
{
  for ( const auto &layer : mExternalLayers )
  {
    if ( layer->name().compare( name ) == 0 )
      return true;
  }

  return false;
}

void QgsWmsRenderContext::checkLayerReadPermissions()
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  for ( const auto layer : mLayersToRender )
  {
    if ( !accessControl()->layerReadPermission( layer ) )
    {
      throw QgsSecurityException( QStringLiteral( "You are not allowed to access to the layer: %1" ).arg( layer->name() ) );
    }
  }
#endif
}

#ifdef HAVE_SERVER_PYTHON_PLUGINS
QgsAccessControl *QgsWmsRenderContext::accessControl() const
{
  return mInterface->accessControls();
}
#endif
