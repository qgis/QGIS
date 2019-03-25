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

#include "qgswmsrendercontext.h"
#include "qgsserverprojectutils.h"

using namespace QgsWms;

QgsWmsRenderContext::QgsWmsRenderContext( const QgsProject *project, QgsServerInterface *interface )
  : mProject( project )
  , mInterface( interface )
  , mFlags()
{
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

void QgsWmsRenderContext::initNicknameLayers()
{
  for ( QgsMapLayer *ml : mProject->mapLayers() )
  {
    mNicknameLayers[ layerNickname( *ml ) ] = ml;
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
    for ( QgsLayerTreeLayer *layer : group->findLayers() )
    {
      mLayerGroups[groupName].append( layer->layer() );
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
  QStringList restricted = QgsServerProjectUtils::wmsRestrictedLayers( *mProject );

  // extract restricted layers from excluded groups
  QStringList restrictedLayersNames;
  QgsLayerTreeGroup *root = mProject->layerTreeRoot();

  for ( const QString &l : restricted )
  {
    QgsLayerTreeGroup *group = root->findGroup( l );
    if ( group )
    {
      QList<QgsLayerTreeLayer *> groupLayers = group->findLayers();
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
  QList<QgsLayerTreeLayer *> layers = root->findLayers();
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
    for ( const QString &layer : mParameters.queryLayersNickname() )
    {
      if ( mNicknameLayers.contains( layer )
           && !mLayersToRender.contains( mNicknameLayers[layer] ) )
      {
        mLayersToRender.append( mNicknameLayers[layer] );
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
      QString err;
      if ( mNicknameLayers.contains( lname ) )
      {
        mSlds[lname] = namedElem;
        mLayersToRender.append( mNicknameLayers[ lname ] );
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
        throw QgsBadRequestException( QStringLiteral( "LayerNotDefined" ),
                                      QStringLiteral( "Layer \"%1\" does not exist" ).arg( lname ) );
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

    if ( mNicknameLayers.contains( nickname ) )
    {
      if ( !style.isEmpty() )
      {
        mStyles[nickname] = style;
      }

      mLayersToRender.append( mNicknameLayers[ nickname ] );
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

      for ( const auto name : layersFromGroup )
      {
        mLayersToRender.append( mNicknameLayers[ name ] );
      }
    }
    else
    {
      throw QgsBadRequestException( QStringLiteral( "LayerNotDefined" ),
                                    QStringLiteral( "Layer \"%1\" does not exist" ).arg( nickname ) );
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

  const QgsMapLayer *layer = mNicknameLayers[ name ];
  bool scaleBasedVisibility = layer->hasScaleBasedVisibility();
  bool useScaleConstraint = ( scaleDenominator() > 0 && scaleBasedVisibility );

  if ( !useScaleConstraint || layer->isInScaleRange( scaleDenominator() ) )
  {
    visible = true;
  }

  return visible;
}

void QgsWmsRenderContext::removeUnwantedLayers()
{
  QList<QgsMapLayer *> layers;

  for ( QgsMapLayer *layer : mLayersToRender )
  {
    const QString nickname = layerNickname( *layer );

    if ( !layerScaleVisibility( nickname ) )
      continue;

    if ( mRestrictedLayers.contains( nickname ) )
      continue;

    if ( mFlags & UseWfsLayersOnly )
    {
      if ( layer->type() != QgsMapLayer::VectorLayer )
      {
        continue;
      }

      const QStringList wfsLayers = QgsServerProjectUtils::wfsLayerIds( *mProject );
      if ( ! wfsLayers.contains( layer->id() ) )
      {
        continue;
      }
    }

    layers.append( layer );
  }

  mLayersToRender = layers;
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
QgsAccessControl *QgsWmsRenderContext::accessControl()
{
  return mInterface->accessControls();
}
#endif
