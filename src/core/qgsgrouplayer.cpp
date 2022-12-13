/***************************************************************************
    qgsgrouplayer.cpp
    ----------------
  Date                 : September 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrouplayer.h"
#include "qgsmaplayerfactory.h"
#include "qgspainting.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsgrouplayerrenderer.h"
#include "qgsmaplayerref.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatetransform.h"
#include "qgspainteffect.h"
#include "qgsmessagelog.h"
#include "qgspainteffectregistry.h"
#include "qgsapplication.h"
#include "qgsmaplayerutils.h"
#include "qgsthreadingutils.h"

QgsGroupLayer::QgsGroupLayer( const QString &name, const LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::GroupLayer, name )
  , mTransformContext( options.transformContext )
{
  mShouldValidateCrs = false;
  mValid = true;

  mPaintEffect.reset( QgsPaintEffectRegistry::defaultStack() );
  mPaintEffect->setEnabled( false );

  QgsDataProvider::ProviderOptions providerOptions;
  providerOptions.transformContext = options.transformContext;
  mDataProvider = new QgsGroupLayerDataProvider( providerOptions, QgsDataProvider::ReadFlags() );
}

QgsGroupLayer::~QgsGroupLayer()
{
  emit willBeDeleted();
  delete mDataProvider;
}

QgsGroupLayer *QgsGroupLayer::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsGroupLayer::LayerOptions options( mTransformContext );
  std::unique_ptr< QgsGroupLayer > layer = std::make_unique< QgsGroupLayer >( name(), options );
  QgsMapLayer::clone( layer.get() );
  layer->setChildLayers( _qgis_listRefToRaw( mChildren ) );
  layer->setPaintEffect( mPaintEffect ? mPaintEffect->clone() : nullptr );
  return layer.release();
}

QgsMapLayerRenderer *QgsGroupLayer::createMapRenderer( QgsRenderContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return new QgsGroupLayerRenderer( this, context );
}

QgsRectangle QgsGroupLayer::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsMapLayerUtils::combinedExtent( childLayers(), crs(), mTransformContext );
}

void QgsGroupLayer::setTransformContext( const QgsCoordinateTransformContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
    mDataProvider->setTransformContext( context );

  mTransformContext = context;
  invalidateWgs84Extent();
}

bool QgsGroupLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mReadFlags & QgsMapLayer::FlagDontResolveLayers )
  {
    return false;
  }

  const QList< QgsMapLayer * > currentLayers = _qgis_listRefToRaw( mChildren );
  for ( QgsMapLayer *layer : currentLayers )
  {
    disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapLayer::triggerRepaint );
  }

  mChildren.clear();
  const QDomNodeList childLayersElements = layerNode.toElement().elementsByTagName( QStringLiteral( "childLayers" ) );
  const QDomNodeList children = childLayersElements.at( 0 ).childNodes();
  for ( int i = 0; i < children.size(); ++i )
  {
    const QDomElement childElement = children.at( i ).toElement();
    const QString id = childElement.attribute( QStringLiteral( "layerid" ) );
    mChildren.append( QgsMapLayerRef( id ) );
  }
  invalidateWgs84Extent();

  QString errorMsg;
  readSymbology( layerNode, errorMsg, context );

  triggerRepaint();

  return mValid;
}

bool QgsGroupLayer::writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // first get the layer element so that we can append the type attribute
  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "can't find maplayer node" ), 2 );
    return false;
  }

  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( QgsMapLayerType::GroupLayer ) );

  QDomElement childLayersElement = doc.createElement( QStringLiteral( "childLayers" ) );
  for ( auto it = mChildren.constBegin(); it != mChildren.constEnd(); ++it )
  {
    QDomElement childElement = doc.createElement( QStringLiteral( "child" ) );
    childElement.setAttribute( QStringLiteral( "layerid" ), it->layerId );
    childLayersElement.appendChild( childElement );
  }
  mapLayerNode.appendChild( childLayersElement );

  // renderer specific settings
  QString errorMsg;
  return writeSymbology( layer_node, doc, errorMsg, context );
}

bool QgsGroupLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &, const QgsReadWriteContext &, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // add the layer opacity
  if ( categories.testFlag( Rendering ) )
  {
    QDomElement layerOpacityElem  = doc.createElement( QStringLiteral( "layerOpacity" ) );
    const QDomText layerOpacityText = doc.createTextNode( QString::number( opacity() ) );
    layerOpacityElem.appendChild( layerOpacityText );
    node.appendChild( layerOpacityElem );

    if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect.get() ) )
    {
      QDomElement paintEffectElement = doc.createElement( QStringLiteral( "paintEffect" ) );
      mPaintEffect->saveProperties( doc, paintEffectElement );
      node.appendChild( paintEffectElement );
    }
  }

  if ( categories.testFlag( Symbology ) )
  {
    // add the blend mode field
    QDomElement blendModeElem  = doc.createElement( QStringLiteral( "blendMode" ) );
    const QDomText blendModeText = doc.createTextNode( QString::number( QgsPainting::getBlendModeEnum( blendMode() ) ) );
    blendModeElem.appendChild( blendModeText );
    node.appendChild( blendModeElem );
  }

  return true;
}

bool QgsGroupLayer::readSymbology( const QDomNode &node, QString &, QgsReadWriteContext &, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( categories.testFlag( Rendering ) )
  {
    const QDomNode layerOpacityNode = node.namedItem( QStringLiteral( "layerOpacity" ) );
    if ( !layerOpacityNode.isNull() )
    {
      const QDomElement e = layerOpacityNode.toElement();
      setOpacity( e.text().toDouble() );
    }

    //restore layer effect
    const QDomElement effectElem = node.namedItem( QStringLiteral( "paintEffect" ) ).toElement();
    if ( !effectElem.isNull() )
    {
      const QDomElement effectPropertiesElem = effectElem.firstChildElement( QStringLiteral( "effect" ) ).toElement();
      mPaintEffect.reset( QgsApplication::paintEffectRegistry()->createEffect( effectPropertiesElem ) );
    }
    else
    {
      mPaintEffect.reset( QgsPaintEffectRegistry::defaultStack() );
      mPaintEffect->setEnabled( false );
    }
  }

  if ( categories.testFlag( Symbology ) )
  {
    // get and set the blend mode if it exists
    const QDomNode blendModeNode = node.namedItem( QStringLiteral( "blendMode" ) );
    if ( !blendModeNode.isNull() )
    {
      const QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( e.text().toInt() ) ) );
    }
  }

  return true;
}

QgsDataProvider *QgsGroupLayer::dataProvider()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider;
}

const QgsDataProvider *QgsGroupLayer::dataProvider() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider;
}

QString QgsGroupLayer::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString metadata = QStringLiteral( "<html>\n<body>\n<h1>" ) + tr( "General" ) + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + name() + QStringLiteral( "</td></tr>\n" );

  // Extent
  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Extent" ) + QStringLiteral( "</td><td>" ) + extent().toString() + QStringLiteral( "</td></tr>\n" );


  metadata += QLatin1String( "\n</body>\n</html>\n" );
  return metadata;
}

void QgsGroupLayer::resolveReferences( QgsProject *project )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsMapLayer::resolveReferences( project );
  for ( int i = 0; i < mChildren.size(); ++i )
  {
    mChildren[i].resolve( project );

    if ( mChildren[i].layer )
    {
      connect( mChildren[i].layer, &QgsMapLayer::repaintRequested, this, &QgsMapLayer::triggerRepaint, Qt::UniqueConnection );

      // group layer inherits first valid child layer's crs
      if ( !crs().isValid() )
      {
        setCrs( mChildren[i].layer->crs() );
        mDataProvider->setCrs( crs() );
      }
    }
  }
  invalidateWgs84Extent();
}

void QgsGroupLayer::setChildLayers( const QList< QgsMapLayer * > &layers )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QList< QgsMapLayer * > currentLayers = _qgis_listRefToRaw( mChildren );
  for ( QgsMapLayer *layer : layers )
  {
    if ( !currentLayers.contains( layer ) )
    {
      connect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapLayer::triggerRepaint, Qt::UniqueConnection );
    }
  }
  for ( QgsMapLayer *layer : currentLayers )
  {
    if ( !layers.contains( layer ) )
    {
      disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapLayer::triggerRepaint );
    }
  }
  mChildren = _qgis_listRawToRef( layers );

  // group layer inherits first valid child layer's crs
  for ( const QgsMapLayer *layer : layers )
  {
    if ( layer->isValid() && layer->crs().isValid( ) )
    {
      setCrs( layer->crs() );
      mDataProvider->setCrs( crs() );
      break;
    }
  }

  triggerRepaint();
}

QList< QgsMapLayer * > QgsGroupLayer::childLayers() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return _qgis_listRefToRaw( mChildren );
}

QgsPaintEffect *QgsGroupLayer::paintEffect() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mPaintEffect.get();
}

void QgsGroupLayer::setPaintEffect( QgsPaintEffect *effect )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mPaintEffect.reset( effect );
}

//
// QgsGroupLayerDataProvider
//
///@cond PRIVATE
QgsGroupLayerDataProvider::QgsGroupLayerDataProvider(
  const ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( QString(), options, flags )
{}

void QgsGroupLayerDataProvider::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mCrs = crs;
}

QgsCoordinateReferenceSystem QgsGroupLayerDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs;
}

QString QgsGroupLayerDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringLiteral( "annotation" );
}

QString QgsGroupLayerDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QString();
}

QgsRectangle QgsGroupLayerDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsRectangle();
}

bool QgsGroupLayerDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}
///@endcond

