/***************************************************************************
                         qgsprocessingutils.cpp
                         ------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingutils.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsprocessingcontext.h"

QList<QgsRasterLayer *> QgsProcessingUtils::compatibleRasterLayers( QgsProject *project, bool sort )
{
  if ( !project )
    return QList<QgsRasterLayer *>();

  QList<QgsRasterLayer *> layers;
  Q_FOREACH ( QgsRasterLayer *l, project->layers<QgsRasterLayer *>() )
  {
    if ( canUseLayer( l ) )
      layers << l;
  }

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsRasterLayer * a, const QgsRasterLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
}

QList<QgsVectorLayer *> QgsProcessingUtils::compatibleVectorLayers( QgsProject *project, const QList<QgsWkbTypes::GeometryType> &geometryTypes, bool sort )
{
  if ( !project )
    return QList<QgsVectorLayer *>();

  QList<QgsVectorLayer *> layers;
  Q_FOREACH ( QgsVectorLayer *l, project->layers<QgsVectorLayer *>() )
  {
    if ( canUseLayer( l, geometryTypes ) )
      layers << l;
  }

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsVectorLayer * a, const QgsVectorLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
}

QList<QgsMapLayer *> QgsProcessingUtils::compatibleLayers( QgsProject *project, bool sort )
{
  if ( !project )
    return QList<QgsMapLayer *>();

  QList<QgsMapLayer *> layers;
  Q_FOREACH ( QgsRasterLayer *rl, compatibleRasterLayers( project, false ) )
    layers << rl;
  Q_FOREACH ( QgsVectorLayer *vl, compatibleVectorLayers( project, QList< QgsWkbTypes::GeometryType >(), false ) )
    layers << vl;

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsMapLayer * a, const QgsMapLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
}

QgsMapLayer *QgsProcessingUtils::mapLayerFromProject( const QString &string, QgsProject *project )
{
  if ( string.isEmpty() )
    return nullptr;

  QList< QgsMapLayer * > layers = compatibleLayers( project, false );
  Q_FOREACH ( QgsMapLayer *l, layers )
  {
    if ( l->id() == string )
      return l;
  }
  Q_FOREACH ( QgsMapLayer *l, layers )
  {
    if ( l->name() == string )
      return l;
  }
  Q_FOREACH ( QgsMapLayer *l, layers )
  {
    if ( normalizeLayerSource( l->source() ) == normalizeLayerSource( string ) )
      return l;
  }
  return nullptr;
}
///@cond PRIVATE
class ProjectionSettingRestorer
{
  public:

    ProjectionSettingRestorer()
    {
      QgsSettings settings;
      previousSetting = settings.value( QStringLiteral( "/Projections/defaultBehavior" ) ).toString();
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), QString() );
    }

    ~ProjectionSettingRestorer()
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), previousSetting );
    }

    QString previousSetting;
};
///@endcond PRIVATE

QgsMapLayer *QgsProcessingUtils::mapLayerFromString( const QString &string )
{
  if ( QFileInfo::exists( string ) )
  {
    // TODO - remove when there is a cleaner way to block the unknown projection dialog!
    ProjectionSettingRestorer restorer;
    ( void )restorer; // no warnings

    // brute force attempt to load a matching layer
    std::unique_ptr< QgsVectorLayer > layer( new QgsVectorLayer( string, QStringLiteral( "temp" ), QStringLiteral( "ogr" ), false ) );
    if ( layer->isValid() )
    {
      return layer.release();
    }
    std::unique_ptr< QgsRasterLayer > rasterLayer( new QgsRasterLayer( string, QStringLiteral( "temp" ), QStringLiteral( "gdal" ), false ) );
    if ( rasterLayer->isValid() )
    {
      return rasterLayer.release();
    }
  }
  return nullptr;
}

bool QgsProcessingUtils::canUseLayer( const QgsRasterLayer *layer )
{
  // only gdal file-based layers
  return layer && layer->providerType() == QStringLiteral( "gdal" );
}

bool QgsProcessingUtils::canUseLayer( const QgsVectorLayer *layer, const QList<QgsWkbTypes::GeometryType> &geometryTypes )
{
  return layer &&
         ( geometryTypes.isEmpty() || geometryTypes.contains( layer->geometryType() ) );
}

QString QgsProcessingUtils::normalizeLayerSource( const QString &source )
{
  QString normalized = source;
  normalized.replace( '\\', '/' );
  normalized.replace( '"', "'" );
  return normalized.trimmed();
}

QgsFeatureIterator QgsProcessingUtils::getFeatures( QgsVectorLayer *layer, const QgsProcessingContext &context, const QgsFeatureRequest &request )
{
  bool useSelection = context.flags() & QgsProcessingContext::UseSelectionIfPresent && layer->selectedFeatureCount() > 0;

  QgsFeatureRequest req( request );
  req.setInvalidGeometryCheck( context.invalidGeometryCheck() );
  req.setInvalidGeometryCallback( context.invalidGeometryCallback() );
  if ( useSelection )
  {
    return layer->selectedFeaturesIterator( req );
  }
  else
  {
    return layer->getFeatures( req );
  }
}

long QgsProcessingUtils::featureCount( QgsVectorLayer *layer, const QgsProcessingContext &context )
{
  bool useSelection = context.flags() & QgsProcessingContext::UseSelectionIfPresent && layer->selectedFeatureCount() > 0;
  if ( useSelection )
    return layer->selectedFeatureCount();
  else
    return layer->featureCount();
}

QList<QVariant> QgsProcessingUtils::uniqueValues( QgsVectorLayer *layer, int fieldIndex, const QgsProcessingContext &context )
{
  if ( !layer )
    return QList<QVariant>();

  if ( fieldIndex < 0 || fieldIndex >= layer->fields().count() )
    return QList<QVariant>();

  bool useSelection = context.flags() & QgsProcessingContext::UseSelectionIfPresent && layer->selectedFeatureCount() > 0;
  if ( !useSelection )
  {
    // not using selection, so use provider optimised version
    QList<QVariant> values;
    layer->uniqueValues( fieldIndex, values );
    return values;
  }
  else
  {
    // using selection, so we have to iterate through selected features
    QSet<QVariant> values;
    QgsFeature f;
    QgsFeatureIterator it = layer->selectedFeaturesIterator( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() << fieldIndex ).setFlags( QgsFeatureRequest::NoGeometry ) );
    while ( it.nextFeature( f ) )
    {
      values.insert( f.attribute( fieldIndex ) );
    }
    return values.toList();
  }
}


