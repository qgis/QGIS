/***************************************************************************
                         qgsprocessingparameters.cpp
                         ---------------------------
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

#include "qgsprocessingparameters.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingalgorithm.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsprocessingoutputs.h"
#include "qgssettings.h"
#include "qgsvectorfilewriter.h"
#include "qgsreferencedgeometry.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingparametertype.h"
#include "qgsrasterfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsmeshlayer.h"
#include "qgsapplication.h"
#include "qgslayoutmanager.h"
#include "qgsprintlayout.h"
#include <functional>


QVariant QgsProcessingOutputLayerDefinition::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "sink" ), sink.toVariant() );
  map.insert( QStringLiteral( "create_options" ), createOptions );
  return map;
}

bool QgsProcessingOutputLayerDefinition::loadVariant( const QVariantMap &map )
{
  sink.loadVariant( map.value( QStringLiteral( "sink" ) ) );
  createOptions = map.value( QStringLiteral( "create_options" ) ).toMap();
  return true;
}

bool QgsProcessingParameters::isDynamic( const QVariantMap &parameters, const QString &name )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().propertyType() != QgsProperty::StaticProperty;
  else
    return false;
}

QString QgsProcessingParameters::parameterAsString( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  return parameterAsString( definition, parameters.value( definition->name() ), context );
}

QString QgsProcessingParameters::parameterAsString( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );

  if ( !val.isValid() )
  {
    // fall back to default
    val = definition->defaultValue();
  }

  if ( val == QgsProcessing::TEMPORARY_OUTPUT )
  {
    if ( const QgsProcessingDestinationParameter *destParam = dynamic_cast< const QgsProcessingDestinationParameter * >( definition ) )
      return destParam->generateTemporaryDestination();
  }

  return val.toString();
}

QString QgsProcessingParameters::parameterAsExpression( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  return parameterAsExpression( definition, parameters.value( definition->name() ), context );
}

QString QgsProcessingParameters::parameterAsExpression( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );

  if ( val.isValid() && !val.toString().isEmpty() )
  {
    QgsExpression e( val.toString() );
    if ( e.isValid() )
      return val.toString();
  }

  // fall back to default
  return definition->defaultValue().toString();
}

double QgsProcessingParameters::parameterAsDouble( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return 0;

  return parameterAsDouble( definition, parameters.value( definition->name() ), context );
}

double QgsProcessingParameters::parameterAsDouble( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return 0;

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsDouble( context.expressionContext(), definition->defaultValue().toDouble() );

  bool ok = false;
  double res = val.toDouble( &ok );
  if ( ok )
    return res;

  // fall back to default
  val = definition->defaultValue();
  return val.toDouble();
}

int QgsProcessingParameters::parameterAsInt( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return 0;

  return parameterAsInt( definition, parameters.value( definition->name() ), context );
}

int QgsProcessingParameters::parameterAsInt( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return 0;

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsInt( context.expressionContext(), definition->defaultValue().toInt() );

  bool ok = false;
  double dbl = val.toDouble( &ok );
  if ( !ok )
  {
    // fall back to default
    val = definition->defaultValue();
    dbl = val.toDouble( &ok );
  }

  //String representations of doubles in QVariant will not convert to int
  //work around this by first converting to double, and then checking whether the double is convertible to int
  if ( ok )
  {
    double round = std::round( dbl );
    if ( round  > std::numeric_limits<int>::max() || round < -std::numeric_limits<int>::max() )
    {
      //double too large to fit in int
      return 0;
    }
    return static_cast< int >( std::round( dbl ) );
  }

  return val.toInt();
}

QList< int > QgsProcessingParameters::parameterAsInts( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QList< int >();

  return parameterAsInts( definition, parameters.value( definition->name() ), context );
}

QList< int > QgsProcessingParameters::parameterAsInts( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return QList< int >();

  QList< int > resultList;
  QVariant val = value;
  if ( val.isValid() )
  {
    if ( val.canConvert<QgsProperty>() )
      resultList << val.value< QgsProperty >().valueAsInt( context.expressionContext(), definition->defaultValue().toInt() );
    else if ( val.type() == QVariant::List )
    {
      QVariantList list = val.toList();
      for ( auto it = list.constBegin(); it != list.constEnd(); ++it )
        resultList << it->toInt();
    }
    else
    {
      QStringList parts = val.toString().split( ';' );
      for ( auto it = parts.constBegin(); it != parts.constEnd(); ++it )
        resultList << it->toInt();
    }
  }

  if ( ( resultList.isEmpty() || resultList.at( 0 ) == 0 ) )
  {
    resultList.clear();
    // check default
    if ( definition->defaultValue().isValid() )
    {
      if ( definition->defaultValue().type() == QVariant::List )
      {
        QVariantList list = definition->defaultValue().toList();
        for ( auto it = list.constBegin(); it != list.constEnd(); ++it )
          resultList << it->toInt();
      }
      else
      {
        QStringList parts = definition->defaultValue().toString().split( ';' );
        for ( auto it = parts.constBegin(); it != parts.constEnd(); ++it )
          resultList << it->toInt();
      }
    }
  }

  return resultList;
}

int QgsProcessingParameters::parameterAsEnum( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return 0;

  return parameterAsEnum( definition, parameters.value( definition->name() ), context );
}

int QgsProcessingParameters::parameterAsEnum( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return 0;

  int val = parameterAsInt( definition, value, context );
  const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( definition );
  if ( enumDef && val >= enumDef->options().size() )
  {
    return enumDef->defaultValue().toInt();
  }
  return val;
}

QList<int> QgsProcessingParameters::parameterAsEnums( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QList<int>();

  return parameterAsEnums( definition, parameters.value( definition->name() ), context );
}

QList<int> QgsProcessingParameters::parameterAsEnums( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return QList<int>();

  QVariantList resultList;
  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    resultList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else if ( val.type() == QVariant::List )
  {
    Q_FOREACH ( const QVariant &var, val.toList() )
      resultList << var;
  }
  else if ( val.type() == QVariant::String )
  {
    Q_FOREACH ( const QString &var, val.toString().split( ',' ) )
      resultList << var;
  }
  else
    resultList << val;

  if ( resultList.isEmpty() )
    return QList< int >();

  if ( ( !val.isValid() || !resultList.at( 0 ).isValid() ) && definition )
  {
    resultList.clear();
    // check default
    if ( definition->defaultValue().type() == QVariant::List )
    {
      Q_FOREACH ( const QVariant &var, definition->defaultValue().toList() )
        resultList << var;
    }
    else if ( definition->defaultValue().type() == QVariant::String )
    {
      Q_FOREACH ( const QString &var, definition->defaultValue().toString().split( ',' ) )
        resultList << var;
    }
    else
      resultList << definition->defaultValue();
  }

  QList< int > result;
  const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( definition );
  Q_FOREACH ( const QVariant &var, resultList )
  {
    int resInt = var.toInt();
    if ( !enumDef || resInt < enumDef->options().size() )
    {
      result << resInt;
    }
  }
  return result;
}

bool QgsProcessingParameters::parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return false;

  return parameterAsBool( definition, parameters.value( definition->name() ), context );
}

bool QgsProcessingParameters::parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return false;

  QVariant def = definition->defaultValue();

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsBool( context.expressionContext(), def.toBool() );
  else if ( val.isValid() )
    return val.toBool();
  else
    return def.toBool();
}

QgsFeatureSink *QgsProcessingParameters::parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsFields &fields,
    QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
    QgsProcessingContext &context, QString &destinationIdentifier, QgsFeatureSink::SinkFlags sinkFlags )
{
  QVariant val;
  if ( definition )
  {
    val = parameters.value( definition->name() );
  }

  return parameterAsSink( definition, val, fields, geometryType, crs, context, destinationIdentifier, sinkFlags );
}

QgsFeatureSink *QgsProcessingParameters::parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs, QgsProcessingContext &context, QString &destinationIdentifier, QgsFeatureSink::SinkFlags sinkFlags )
{
  QVariant val = value;

  QgsProject *destinationProject = nullptr;
  QString destName;
  QVariantMap createOptions;
  if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    destinationProject = fromVar.destinationProject;
    createOptions = fromVar.createOptions;

    val = fromVar.sink;
    destName = fromVar.destinationName;
  }

  QString dest;
  if ( val.canConvert<QgsProperty>() )
  {
    dest = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }
  else if ( !val.isValid() || val.toString().isEmpty() )
  {
    if ( definition && definition->flags() & QgsProcessingParameterDefinition::FlagOptional && !definition->defaultValue().isValid() )
    {
      // unset, optional sink, no default => no sink
      return nullptr;
    }
    // fall back to default
    dest = definition->defaultValue().toString();
  }
  else
  {
    dest = val.toString();
  }
  if ( dest == QgsProcessing::TEMPORARY_OUTPUT )
  {
    if ( const QgsProcessingDestinationParameter *destParam = dynamic_cast< const QgsProcessingDestinationParameter * >( definition ) )
      dest = destParam->generateTemporaryDestination();
  }

  if ( dest.isEmpty() )
    return nullptr;

  std::unique_ptr< QgsFeatureSink > sink( QgsProcessingUtils::createFeatureSink( dest, context, fields, geometryType, crs, createOptions, sinkFlags ) );
  destinationIdentifier = dest;

  if ( destinationProject )
  {
    if ( destName.isEmpty() && definition )
    {
      destName = definition->description();
    }
    QString outputName;
    if ( definition )
      outputName = definition->name();
    context.addLayerToLoadOnCompletion( destinationIdentifier, QgsProcessingContext::LayerDetails( destName, destinationProject, outputName, QgsProcessingUtils::Vector ) );
  }

  return sink.release();
}

QgsProcessingFeatureSource *QgsProcessingParameters::parameterAsSource( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return nullptr;

  return parameterAsSource( definition, parameters.value( definition->name() ), context );
}

QgsProcessingFeatureSource *QgsProcessingParameters::parameterAsSource( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return nullptr;

  return QgsProcessingUtils::variantToSource( value, context, definition->defaultValue() );
}

QString QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingFeedback *feedback )
{
  if ( !definition )
    return QString();

  QVariant val = parameters.value( definition->name() );

  bool selectedFeaturesOnly = false;
  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    selectedFeaturesOnly = fromVar.selectedFeaturesOnly;
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.canConvert<QgsProperty>() )
  {
    val = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }

  QgsVectorLayer *vl = nullptr;
  vl = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( val ) );

  if ( !vl )
  {
    QString layerRef;
    if ( val.canConvert<QgsProperty>() )
    {
      layerRef = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
    }
    else if ( !val.isValid() || val.toString().isEmpty() )
    {
      // fall back to default
      val = definition->defaultValue();

      // default value may be a vector layer
      vl = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( val ) );
      if ( !vl )
        layerRef = definition->defaultValue().toString();
    }
    else
    {
      layerRef = val.toString();
    }

    if ( !vl )
    {
      if ( layerRef.isEmpty() )
        return QString();

      vl = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( layerRef, context, true, QgsProcessingUtils::Vector ) );
    }
  }

  if ( !vl )
    return QString();

  return QgsProcessingUtils::convertToCompatibleFormat( vl, selectedFeaturesOnly, definition->name(),
         compatibleFormats, preferredFormat, context, feedback );
}


QgsMapLayer *QgsProcessingParameters::parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return nullptr;

  return parameterAsLayer( definition, parameters.value( definition->name() ), context );
}

QgsMapLayer *QgsProcessingParameters::parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return nullptr;

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
  {
    val = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }

  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) ) )
  {
    return layer;
  }

  if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.canConvert<QgsProperty>() && val.value< QgsProperty >().propertyType() == QgsProperty::StaticProperty )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  if ( !val.isValid() || val.toString().isEmpty() )
  {
    // fall back to default
    val = definition->defaultValue();
  }

  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) ) )
  {
    return layer;
  }

  QString layerRef = val.toString();
  if ( layerRef.isEmpty() )
    layerRef = definition->defaultValue().toString();

  if ( layerRef.isEmpty() )
    return nullptr;

  return QgsProcessingUtils::mapLayerFromString( layerRef, context );
}

QgsRasterLayer *QgsProcessingParameters::parameterAsRasterLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  return qobject_cast< QgsRasterLayer *>( parameterAsLayer( definition, parameters, context ) );
}

QgsRasterLayer *QgsProcessingParameters::parameterAsRasterLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  return qobject_cast< QgsRasterLayer *>( parameterAsLayer( definition, value, context ) );
}

QgsMeshLayer *QgsProcessingParameters::parameterAsMeshLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  return qobject_cast< QgsMeshLayer *>( parameterAsLayer( definition, parameters, context ) );
}

QgsMeshLayer *QgsProcessingParameters::parameterAsMeshLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  return qobject_cast< QgsMeshLayer *>( parameterAsLayer( definition, value, context ) );
}

QString QgsProcessingParameters::parameterAsOutputLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  QVariant val;
  if ( definition )
  {
    val = parameters.value( definition->name() );
  }
  return parameterAsOutputLayer( definition, val, context );
}

QString QgsProcessingParameters::parameterAsOutputLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  QVariant val = value;

  QgsProject *destinationProject = nullptr;
  QVariantMap createOptions;
  QString destName;
  if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    destinationProject = fromVar.destinationProject;
    createOptions = fromVar.createOptions;
    val = fromVar.sink;
    destName = fromVar.destinationName;
  }

  QString dest;
  if ( val.canConvert<QgsProperty>() )
  {
    dest = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }
  else if ( definition && ( !val.isValid() || val.toString().isEmpty() ) )
  {
    // fall back to default
    dest = definition->defaultValue().toString();
  }
  else
  {
    dest = val.toString();
  }
  if ( dest == QgsProcessing::TEMPORARY_OUTPUT )
  {
    if ( const QgsProcessingDestinationParameter *destParam = dynamic_cast< const QgsProcessingDestinationParameter * >( definition ) )
      dest = destParam->generateTemporaryDestination();
  }

  if ( destinationProject )
  {
    QString outputName;
    if ( destName.isEmpty() && definition )
    {
      destName = definition->description();
    }
    if ( definition )
      outputName = definition->name();

    QgsProcessingUtils::LayerHint layerTypeHint = QgsProcessingUtils::UnknownType;
    if ( definition->type() == QgsProcessingParameterVectorDestination::typeName() )
      layerTypeHint = QgsProcessingUtils::Vector;
    else if ( definition->type() == QgsProcessingParameterRasterDestination::typeName() )
      layerTypeHint = QgsProcessingUtils::Raster;

    context.addLayerToLoadOnCompletion( dest, QgsProcessingContext::LayerDetails( destName, destinationProject, outputName, layerTypeHint ) );
  }

  return dest;
}

QString QgsProcessingParameters::parameterAsFileOutput( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  QVariant val;
  if ( definition )
  {
    val = parameters.value( definition->name() );
  }
  return parameterAsFileOutput( definition, val, context );
}

QString QgsProcessingParameters::parameterAsFileOutput( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  QVariant val = value;

  if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  QString dest;
  if ( val.canConvert<QgsProperty>() )
  {
    dest = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }
  else if ( !val.isValid() || val.toString().isEmpty() )
  {
    // fall back to default
    dest = definition->defaultValue().toString();
  }
  else
  {
    dest = val.toString();
  }
  if ( dest == QgsProcessing::TEMPORARY_OUTPUT )
  {
    if ( const QgsProcessingDestinationParameter *destParam = dynamic_cast< const QgsProcessingDestinationParameter * >( definition ) )
      dest = destParam->generateTemporaryDestination();
  }
  return dest;
}

QgsVectorLayer *QgsProcessingParameters::parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  return qobject_cast< QgsVectorLayer *>( parameterAsLayer( definition, parameters, context ) );
}

QgsVectorLayer *QgsProcessingParameters::parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  return qobject_cast< QgsVectorLayer *>( parameterAsLayer( definition, value, context ) );
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QgsCoordinateReferenceSystem();

  return parameterAsCrs( definition, parameters.value( definition->name() ), context );
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsCrs( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QgsCoordinateReferenceSystem();

  QVariant val = value;

  if ( val.canConvert<QgsCoordinateReferenceSystem>() )
  {
    // input is a QgsCoordinateReferenceSystem - done!
    return val.value< QgsCoordinateReferenceSystem >();
  }
  else if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.canConvert<QgsProperty>() && val.value< QgsProperty >().propertyType() == QgsProperty::StaticProperty )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  // maybe a map layer
  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) ) )
    return layer->crs();

  if ( val.canConvert<QgsProperty>() )
    val = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );

  if ( !val.isValid() )
  {
    // fall back to default
    val = definition->defaultValue();
  }

  QString crsText = val.toString();
  if ( crsText.isEmpty() )
    crsText = definition->defaultValue().toString();

  if ( crsText.isEmpty() )
    return QgsCoordinateReferenceSystem();

  // maybe special string
  if ( context.project() && crsText.compare( QLatin1String( "ProjectCrs" ), Qt::CaseInsensitive ) == 0 )
    return context.project()->crs();

  // maybe a map layer reference
  if ( QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( crsText, context ) )
    return layer->crs();

  // else CRS from string
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( crsText );
  return crs;
}

QgsRectangle QgsProcessingParameters::parameterAsExtent( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context,
    const QgsCoordinateReferenceSystem &crs )
{
  if ( !definition )
    return QgsRectangle();

  return parameterAsExtent( definition, parameters.value( definition->name() ), context, crs );
}

QgsRectangle QgsProcessingParameters::parameterAsExtent( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs )
{
  if ( !definition )
    return QgsRectangle();

  QVariant val = value;

  if ( val.canConvert< QgsRectangle >() )
  {
    return val.value<QgsRectangle>();
  }
  if ( val.canConvert< QgsReferencedRectangle >() )
  {
    QgsReferencedRectangle rr = val.value<QgsReferencedRectangle>();
    if ( crs.isValid() && rr.crs().isValid() && crs != rr.crs() )
    {
      QgsCoordinateTransform ct( rr.crs(), crs, context.project() );
      try
      {
        return ct.transformBoundingBox( rr );
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error transforming extent geometry" ) );
      }
    }
    return rr;
  }

  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.canConvert<QgsProperty>() && val.value< QgsProperty >().propertyType() == QgsProperty::StaticProperty )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  // maybe parameter is a direct layer value?
  QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) );

  QString rectText;
  if ( val.canConvert<QgsProperty>() )
    rectText = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else
    rectText = val.toString();

  if ( rectText.isEmpty() && !layer )
    return QgsRectangle();

  QRegularExpression rx( QStringLiteral( "^(.*?)\\s*,\\s*(.*?),\\s*(.*?),\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$" ) );
  QRegularExpressionMatch match = rx.match( rectText );
  if ( match.hasMatch() )
  {
    bool xMinOk = false;
    double xMin = match.captured( 1 ).toDouble( &xMinOk );
    bool xMaxOk = false;
    double xMax = match.captured( 2 ).toDouble( &xMaxOk );
    bool yMinOk = false;
    double yMin = match.captured( 3 ).toDouble( &yMinOk );
    bool yMaxOk = false;
    double yMax = match.captured( 4 ).toDouble( &yMaxOk );
    if ( xMinOk && xMaxOk && yMinOk && yMaxOk )
    {
      QgsRectangle rect( xMin, yMin, xMax, yMax );
      QgsCoordinateReferenceSystem rectCrs( match.captured( 5 ) );
      if ( crs.isValid() && rectCrs.isValid() && crs != rectCrs )
      {
        QgsCoordinateTransform ct( rectCrs, crs, context.project() );
        try
        {
          return ct.transformBoundingBox( rect );
        }
        catch ( QgsCsException & )
        {
          QgsMessageLog::logMessage( QObject::tr( "Error transforming extent geometry" ) );
        }
      }
      return rect;
    }
  }

  // try as layer extent
  if ( !layer )
    layer = QgsProcessingUtils::mapLayerFromString( rectText, context );

  if ( layer )
  {
    QgsRectangle rect = layer->extent();
    if ( crs.isValid() && layer->crs().isValid() && crs != layer->crs() )
    {
      QgsCoordinateTransform ct( layer->crs(), crs, context.project() );
      try
      {
        return ct.transformBoundingBox( rect );
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error transforming extent geometry" ) );
      }
    }
    return rect;
  }
  return QgsRectangle();
}

QgsGeometry QgsProcessingParameters::parameterAsExtentGeometry( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs )
{
  if ( !definition )
    return QgsGeometry();

  QVariant val = parameters.value( definition->name() );

  if ( val.canConvert< QgsReferencedRectangle >() )
  {
    QgsReferencedRectangle rr = val.value<QgsReferencedRectangle>();
    QgsGeometry g = QgsGeometry::fromRect( rr );
    if ( crs.isValid() && rr.crs().isValid() && crs != rr.crs() )
    {
      g = g.densifyByCount( 20 );
      QgsCoordinateTransform ct( rr.crs(), crs, context.project() );
      try
      {
        g.transform( ct );
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error transforming extent geometry" ) );
      }
      return g;
    }
  }

  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.canConvert<QgsProperty>() && val.value< QgsProperty >().propertyType() == QgsProperty::StaticProperty )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  QString rectText;
  if ( val.canConvert<QgsProperty>() )
    rectText = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else
    rectText = val.toString();

  if ( !rectText.isEmpty() )
  {
    QRegularExpression rx( QStringLiteral( "^(.*?)\\s*,\\s*(.*?),\\s*(.*?),\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$" ) );
    QRegularExpressionMatch match = rx.match( rectText );
    if ( match.hasMatch() )
    {
      bool xMinOk = false;
      double xMin = match.captured( 1 ).toDouble( &xMinOk );
      bool xMaxOk = false;
      double xMax = match.captured( 2 ).toDouble( &xMaxOk );
      bool yMinOk = false;
      double yMin = match.captured( 3 ).toDouble( &yMinOk );
      bool yMaxOk = false;
      double yMax = match.captured( 4 ).toDouble( &yMaxOk );
      if ( xMinOk && xMaxOk && yMinOk && yMaxOk )
      {
        QgsRectangle rect( xMin, yMin, xMax, yMax );
        QgsCoordinateReferenceSystem rectCrs( match.captured( 5 ) );
        QgsGeometry g = QgsGeometry::fromRect( rect );
        if ( crs.isValid() && rectCrs.isValid() && crs != rectCrs )
        {
          g = g.densifyByCount( 20 );
          QgsCoordinateTransform ct( rectCrs, crs, context.project() );
          try
          {
            g.transform( ct );
          }
          catch ( QgsCsException & )
          {
            QgsMessageLog::logMessage( QObject::tr( "Error transforming extent geometry" ) );
          }
          return g;
        }
      }
    }
  }

  // try as layer extent

  // maybe parameter is a direct layer value?
  QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) );
  if ( !layer )
    layer = QgsProcessingUtils::mapLayerFromString( rectText, context );

  if ( layer )
  {
    QgsRectangle rect = layer->extent();
    QgsGeometry g = QgsGeometry::fromRect( rect );
    if ( crs.isValid() && layer->crs().isValid() && crs != layer->crs() )
    {
      g = g.densifyByCount( 20 );
      QgsCoordinateTransform ct( layer->crs(), crs, context.project() );
      try
      {
        g.transform( ct );
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error transforming extent geometry" ) );
      }
    }
    return g;
  }

  return QgsGeometry::fromRect( parameterAsExtent( definition, parameters, context, crs ) );
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsExtentCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  QVariant val = parameters.value( definition->name() );

  if ( val.canConvert< QgsReferencedRectangle >() )
  {
    QgsReferencedRectangle rr = val.value<QgsReferencedRectangle>();
    if ( rr.crs().isValid() )
    {
      return rr.crs();
    }
  }

  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.canConvert<QgsProperty>() && val.value< QgsProperty >().propertyType() == QgsProperty::StaticProperty )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  QString valueAsString;
  if ( val.canConvert<QgsProperty>() )
    valueAsString = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else
    valueAsString = val.toString();

  QRegularExpression rx( QStringLiteral( "^(.*?)\\s*,\\s*(.*?),\\s*(.*?),\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$" ) );

  QRegularExpressionMatch match = rx.match( valueAsString );
  if ( match.hasMatch() )
  {
    QgsCoordinateReferenceSystem crs( match.captured( 5 ) );
    if ( crs.isValid() )
      return crs;
  }

  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.canConvert<QgsProperty>() && val.value< QgsProperty >().propertyType() == QgsProperty::StaticProperty )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  // try as layer crs
  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) ) )
    return layer->crs();
  else if ( QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( valueAsString, context ) )
    return layer->crs();

  if ( context.project() )
    return context.project()->crs();
  else
    return QgsCoordinateReferenceSystem();
}

QgsPointXY QgsProcessingParameters::parameterAsPoint( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs )
{
  if ( !definition )
    return QgsPointXY();

  return parameterAsPoint( definition, parameters.value( definition->name() ), context, crs );
}

QgsPointXY QgsProcessingParameters::parameterAsPoint( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs )
{
  if ( !definition )
    return QgsPointXY();

  QVariant val = value;
  if ( val.canConvert< QgsPointXY >() )
  {
    return val.value<QgsPointXY>();
  }
  if ( val.canConvert< QgsGeometry >() )
  {
    const QgsGeometry geom = val.value<QgsGeometry>();
    if ( !geom.isNull() )
      return geom.centroid().asPoint();
  }
  if ( val.canConvert< QgsReferencedPointXY >() )
  {
    QgsReferencedPointXY rp = val.value<QgsReferencedPointXY>();
    if ( crs.isValid() && rp.crs().isValid() && crs != rp.crs() )
    {
      QgsCoordinateTransform ct( rp.crs(), crs, context.project() );
      try
      {
        return ct.transform( rp );
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error transforming point geometry" ) );
      }
    }
    return rp;
  }

  QString pointText = parameterAsString( definition, value, context );
  if ( pointText.isEmpty() )
    pointText = definition->defaultValue().toString();

  if ( pointText.isEmpty() )
    return QgsPointXY();

  QRegularExpression rx( QStringLiteral( "^\\s*\\(?\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*\\)?\\s*$" ) );

  QString valueAsString = parameterAsString( definition, value, context );
  QRegularExpressionMatch match = rx.match( valueAsString );
  if ( match.hasMatch() )
  {
    bool xOk = false;
    double x = match.captured( 1 ).toDouble( &xOk );
    bool yOk = false;
    double y = match.captured( 2 ).toDouble( &yOk );

    if ( xOk && yOk )
    {
      QgsPointXY pt( x, y );

      QgsCoordinateReferenceSystem pointCrs( match.captured( 3 ) );
      if ( crs.isValid() && pointCrs.isValid() && crs != pointCrs )
      {
        QgsCoordinateTransform ct( pointCrs, crs, context.project() );
        try
        {
          return ct.transform( pt );
        }
        catch ( QgsCsException & )
        {
          QgsMessageLog::logMessage( QObject::tr( "Error transforming point geometry" ) );
        }
      }
      return pt;
    }
  }

  return QgsPointXY();
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsPointCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  QVariant val = parameters.value( definition->name() );
  return parameterAsPointCrs( definition, val, context );
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsPointCrs( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( value.canConvert< QgsReferencedPointXY >() )
  {
    QgsReferencedPointXY rr = value.value<QgsReferencedPointXY>();
    if ( rr.crs().isValid() )
    {
      return rr.crs();
    }
  }

  QRegularExpression rx( QStringLiteral( "^\\s*\\(?\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*\\)?\\s*$" ) );

  QString valueAsString = parameterAsString( definition, value, context );
  QRegularExpressionMatch match = rx.match( valueAsString );
  if ( match.hasMatch() )
  {
    QgsCoordinateReferenceSystem crs( match.captured( 3 ) );
    if ( crs.isValid() )
      return crs;
  }

  if ( context.project() )
    return context.project()->crs();
  else
    return QgsCoordinateReferenceSystem();
}

QString QgsProcessingParameters::parameterAsFile( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  QString fileText = parameterAsString( definition, parameters, context );
  if ( fileText.isEmpty() )
    fileText = definition->defaultValue().toString();
  return fileText;
}

QString QgsProcessingParameters::parameterAsFile( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  QString fileText = parameterAsString( definition, value, context );
  if ( fileText.isEmpty() )
    fileText = definition->defaultValue().toString();
  return fileText;
}

QVariantList QgsProcessingParameters::parameterAsMatrix( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QVariantList();

  return parameterAsMatrix( definition, parameters.value( definition->name() ), context );
}

QVariantList QgsProcessingParameters::parameterAsMatrix( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QVariantList();

  QString resultString;
  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    resultString = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else if ( val.type() == QVariant::List )
    return val.toList();
  else
    resultString = val.toString();

  if ( resultString.isEmpty() )
  {
    // check default
    if ( definition->defaultValue().type() == QVariant::List )
      return definition->defaultValue().toList();
    else
      resultString = definition->defaultValue().toString();
  }

  QVariantList result;
  Q_FOREACH ( const QString &s, resultString.split( ',' ) )
    result << s;

  return result;
}

QList<QgsMapLayer *> QgsProcessingParameters::parameterAsLayerList( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QList<QgsMapLayer *>();

  return parameterAsLayerList( definition, parameters.value( definition->name() ), context );
}

QList<QgsMapLayer *> QgsProcessingParameters::parameterAsLayerList( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QList<QgsMapLayer *>();

  QVariant val = value;
  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) ) )
  {
    return QList<QgsMapLayer *>() << layer;
  }

  QList<QgsMapLayer *> layers;

  std::function< void( const QVariant &var ) > processVariant;
  processVariant = [ &layers, &context, &definition, &processVariant ]( const QVariant & var )
  {
    if ( var.type() == QVariant::List )
    {
      Q_FOREACH ( const QVariant &listVar, var.toList() )
      {
        processVariant( listVar );
      }
    }
    else if ( var.type() == QVariant::StringList )
    {
      Q_FOREACH ( const QString &s, var.toStringList() )
      {
        processVariant( s );
      }
    }
    else if ( var.canConvert<QgsProperty>() )
      processVariant( var.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() ) );
    else if ( var.canConvert<QgsProcessingOutputLayerDefinition>() )
    {
      // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
      QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
      QVariant sink = fromVar.sink;
      if ( sink.canConvert<QgsProperty>() )
      {
        processVariant( sink.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() ) );
      }
    }
    else if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( var ) ) )
    {
      layers << layer;
    }
    else
    {
      QgsMapLayer *alayer = QgsProcessingUtils::mapLayerFromString( var.toString(), context );
      if ( alayer )
        layers << alayer;
    }
  };

  processVariant( val );

  if ( layers.isEmpty() )
  {
    // check default
    if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( definition->defaultValue() ) ) )
    {
      layers << layer;
    }
    else if ( definition->defaultValue().type() == QVariant::List )
    {
      Q_FOREACH ( const QVariant &var, definition->defaultValue().toList() )
      {
        if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( var ) ) )
        {
          layers << layer;
        }
        else
        {
          processVariant( var );
        }
      }
    }
    else
      processVariant( definition->defaultValue() );
  }

  return layers;
}

QList<double> QgsProcessingParameters::parameterAsRange( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QList<double>();

  return parameterAsRange( definition, parameters.value( definition->name() ), context );
}

QList<double> QgsProcessingParameters::parameterAsRange( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QList<double>();

  QStringList resultStringList;
  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else if ( val.type() == QVariant::List )
  {
    Q_FOREACH ( const QVariant &var, val.toList() )
      resultStringList << var.toString();
  }
  else
    resultStringList << val.toString();

  if ( ( resultStringList.isEmpty() || ( resultStringList.size() == 1 && resultStringList.at( 0 ).isEmpty() ) ) )
  {
    resultStringList.clear();
    // check default
    if ( definition->defaultValue().type() == QVariant::List )
    {
      Q_FOREACH ( const QVariant &var, definition->defaultValue().toList() )
        resultStringList << var.toString();
    }
    else
      resultStringList << definition->defaultValue().toString();
  }

  if ( resultStringList.size() == 1 )
  {
    resultStringList = resultStringList.at( 0 ).split( ',' );
  }

  if ( resultStringList.size() < 2 )
    return QList< double >() << 0.0 << 0.0;

  return QList< double >() << resultStringList.at( 0 ).toDouble() << resultStringList.at( 1 ).toDouble();
}

QStringList QgsProcessingParameters::parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  QStringList resultStringList;
  return parameterAsFields( definition, parameters.value( definition->name() ), context );
}

QStringList QgsProcessingParameters::parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  QStringList resultStringList;
  QVariant val = value;
  if ( val.isValid() )
  {
    if ( val.canConvert<QgsProperty>() )
      resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
    else if ( val.type() == QVariant::List )
    {
      Q_FOREACH ( const QVariant &var, val.toList() )
        resultStringList << var.toString();
    }
    else
      resultStringList.append( val.toString().split( ';' ) );
  }

  if ( ( resultStringList.isEmpty() || resultStringList.at( 0 ).isEmpty() ) )
  {
    resultStringList.clear();
    // check default
    if ( definition->defaultValue().isValid() )
    {
      if ( definition->defaultValue().type() == QVariant::List )
      {
        Q_FOREACH ( const QVariant &var, definition->defaultValue().toList() )
          resultStringList << var.toString();
      }
      else
        resultStringList.append( definition->defaultValue().toString().split( ';' ) );
    }
  }

  return resultStringList;
}

QgsPrintLayout *QgsProcessingParameters::parameterAsLayout( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return nullptr;

  return parameterAsLayout( definition, parameters.value( definition->name() ), context );
}

QgsPrintLayout *QgsProcessingParameters::parameterAsLayout( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  const QString layoutName = parameterAsString( definition, value, context );
  if ( layoutName.isEmpty() )
    return nullptr;

  if ( !context.project() )
    return nullptr;

  QgsMasterLayoutInterface *l = context.project()->layoutManager()->layoutByName( layoutName );
  if ( l && l->layoutType() == QgsMasterLayoutInterface::PrintLayout )
    return static_cast< QgsPrintLayout * >( l );
  else
    return nullptr;
}

QgsLayoutItem *QgsProcessingParameters::parameterAsLayoutItem( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, QgsPrintLayout *layout )
{
  if ( !definition )
    return nullptr;

  return parameterAsLayoutItem( definition, parameters.value( definition->name() ), context, layout );
}

QgsLayoutItem *QgsProcessingParameters::parameterAsLayoutItem( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, QgsPrintLayout *layout )
{
  if ( !layout )
    return nullptr;

  const QString id = parameterAsString( definition, value, context );
  if ( id.isEmpty() )
    return nullptr;

  // prefer matching by uuid, since it's guaranteed to be unique.
  if ( QgsLayoutItem *item = layout->itemByUuid( id ) )
    return item;
  else if ( QgsLayoutItem *item = layout->itemById( id ) )
    return item;
  else
    return nullptr;
}

QgsProcessingParameterDefinition *QgsProcessingParameters::parameterFromVariantMap( const QVariantMap &map )
{
  QString type = map.value( QStringLiteral( "parameter_type" ) ).toString();
  QString name = map.value( QStringLiteral( "name" ) ).toString();
  std::unique_ptr< QgsProcessingParameterDefinition > def;
  if ( type == QgsProcessingParameterBoolean::typeName() )
    def.reset( new QgsProcessingParameterBoolean( name ) );
  else if ( type == QgsProcessingParameterCrs::typeName() )
    def.reset( new QgsProcessingParameterCrs( name ) );
  else if ( type == QgsProcessingParameterMapLayer::typeName() )
    def.reset( new QgsProcessingParameterMapLayer( name ) );
  else if ( type == QgsProcessingParameterExtent::typeName() )
    def.reset( new QgsProcessingParameterExtent( name ) );
  else if ( type == QgsProcessingParameterPoint::typeName() )
    def.reset( new QgsProcessingParameterPoint( name ) );
  else if ( type == QgsProcessingParameterFile::typeName() )
    def.reset( new QgsProcessingParameterFile( name ) );
  else if ( type == QgsProcessingParameterMatrix::typeName() )
    def.reset( new QgsProcessingParameterMatrix( name ) );
  else if ( type == QgsProcessingParameterMultipleLayers::typeName() )
    def.reset( new QgsProcessingParameterMultipleLayers( name ) );
  else if ( type == QgsProcessingParameterNumber::typeName() )
    def.reset( new QgsProcessingParameterNumber( name ) );
  else if ( type == QgsProcessingParameterRange::typeName() )
    def.reset( new QgsProcessingParameterRange( name ) );
  else if ( type == QgsProcessingParameterRasterLayer::typeName() )
    def.reset( new QgsProcessingParameterRasterLayer( name ) );
  else if ( type == QgsProcessingParameterEnum::typeName() )
    def.reset( new QgsProcessingParameterEnum( name ) );
  else if ( type == QgsProcessingParameterString::typeName() )
    def.reset( new QgsProcessingParameterString( name ) );
  else if ( type == QgsProcessingParameterAuthConfig::typeName() )
    def.reset( new QgsProcessingParameterAuthConfig( name ) );
  else if ( type == QgsProcessingParameterExpression::typeName() )
    def.reset( new QgsProcessingParameterExpression( name ) );
  else if ( type == QgsProcessingParameterVectorLayer::typeName() )
    def.reset( new QgsProcessingParameterVectorLayer( name ) );
  else if ( type == QgsProcessingParameterField::typeName() )
    def.reset( new QgsProcessingParameterField( name ) );
  else if ( type == QgsProcessingParameterFeatureSource::typeName() )
    def.reset( new QgsProcessingParameterFeatureSource( name ) );
  else if ( type == QgsProcessingParameterFeatureSink::typeName() )
    def.reset( new QgsProcessingParameterFeatureSink( name ) );
  else if ( type == QgsProcessingParameterVectorDestination::typeName() )
    def.reset( new QgsProcessingParameterVectorDestination( name ) );
  else if ( type == QgsProcessingParameterRasterDestination::typeName() )
    def.reset( new QgsProcessingParameterRasterDestination( name ) );
  else if ( type == QgsProcessingParameterFileDestination::typeName() )
    def.reset( new QgsProcessingParameterFileDestination( name ) );
  else if ( type == QgsProcessingParameterFolderDestination::typeName() )
    def.reset( new QgsProcessingParameterFolderDestination( name ) );
  else if ( type == QgsProcessingParameterBand::typeName() )
    def.reset( new QgsProcessingParameterBand( name ) );
  else if ( type == QgsProcessingParameterMeshLayer::typeName() )
    def.reset( new QgsProcessingParameterMeshLayer( name ) );
  else if ( type == QgsProcessingParameterLayout::typeName() )
    def.reset( new QgsProcessingParameterLayout( name ) );
  else if ( type == QgsProcessingParameterLayoutItem::typeName() )
    def.reset( new QgsProcessingParameterLayoutItem( name ) );
  else
  {
    QgsProcessingParameterType *paramType = QgsApplication::instance()->processingRegistry()->parameterType( type );
    if ( paramType )
      def.reset( paramType->create( name ) );
  }

  if ( !def )
    return nullptr;

  def->fromVariantMap( map );
  return def.release();
}

QString QgsProcessingParameters::descriptionFromName( const QString &name )
{
  QString desc = name;
  desc.replace( '_', ' ' );
  return desc;
}

QgsProcessingParameterDefinition *QgsProcessingParameters::parameterFromScriptCode( const QString &code )
{
  bool isOptional = false;
  QString name;
  QString definition;
  QString type;
  if ( !parseScriptCodeParameterOptions( code, isOptional, name, type, definition ) )
    return nullptr;

  QString description = descriptionFromName( name );

  if ( type == QStringLiteral( "boolean" ) )
    return QgsProcessingParameterBoolean::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "crs" ) )
    return QgsProcessingParameterCrs::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "layer" ) )
    return QgsProcessingParameterMapLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "extent" ) )
    return QgsProcessingParameterExtent::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "point" ) )
    return QgsProcessingParameterPoint::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "file" ) )
    return QgsProcessingParameterFile::fromScriptCode( name, description, isOptional, definition, QgsProcessingParameterFile::File );
  else if ( type == QStringLiteral( "folder" ) )
    return QgsProcessingParameterFile::fromScriptCode( name, description, isOptional, definition, QgsProcessingParameterFile::Folder );
  else if ( type == QStringLiteral( "matrix" ) )
    return QgsProcessingParameterMatrix::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "multiple" ) )
    return QgsProcessingParameterMultipleLayers::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "number" ) )
    return QgsProcessingParameterNumber::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "distance" ) )
    return QgsProcessingParameterDistance::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "scale" ) )
    return QgsProcessingParameterScale::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "range" ) )
    return QgsProcessingParameterRange::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "raster" ) )
    return QgsProcessingParameterRasterLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "enum" ) )
    return QgsProcessingParameterEnum::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "string" ) )
    return QgsProcessingParameterString::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "authcfg" ) )
    return QgsProcessingParameterAuthConfig::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "expression" ) )
    return QgsProcessingParameterExpression::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "field" ) )
    return QgsProcessingParameterField::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "vector" ) )
    return QgsProcessingParameterVectorLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "source" ) )
    return QgsProcessingParameterFeatureSource::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "sink" ) )
    return QgsProcessingParameterFeatureSink::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "vectordestination" ) )
    return QgsProcessingParameterVectorDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "rasterdestination" ) )
    return QgsProcessingParameterRasterDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "filedestination" ) )
    return QgsProcessingParameterFileDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "folderdestination" ) )
    return QgsProcessingParameterFolderDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "band" ) )
    return QgsProcessingParameterBand::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "mesh" ) )
    return QgsProcessingParameterMeshLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "layout" ) )
    return QgsProcessingParameterLayout::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QStringLiteral( "layoutitem" ) )
    return QgsProcessingParameterLayoutItem::fromScriptCode( name, description, isOptional, definition );

  return nullptr;
}

bool QgsProcessingParameters::parseScriptCodeParameterOptions( const QString &code, bool &isOptional, QString &name, QString &type, QString &definition )
{
  QRegularExpression re( QStringLiteral( "(?:#*)(.*?)=\\s*(.*)" ) );
  QRegularExpressionMatch m = re.match( code );
  if ( !m.hasMatch() )
    return false;

  name = m.captured( 1 );
  QString tokens = m.captured( 2 );
  if ( tokens.startsWith( QLatin1String( "optional" ), Qt::CaseInsensitive ) )
  {
    isOptional = true;
    tokens.remove( 0, 8 ); // length "optional" = 8
  }
  else
  {
    isOptional = false;
  }

  tokens = tokens.trimmed();

  QRegularExpression re2( QStringLiteral( "(.*?)\\s+(.*)" ) );
  m = re2.match( tokens );
  if ( !m.hasMatch() )
  {
    type = tokens.toLower().trimmed();
    definition.clear();
  }
  else
  {
    type = m.captured( 1 ).toLower().trimmed();
    definition = m.captured( 2 );
  }
  return true;
}

//
// QgsProcessingParameterDefinition
//

QgsProcessingParameterDefinition::QgsProcessingParameterDefinition( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : mName( name )
  , mDescription( description )
  , mDefault( defaultValue )
  , mFlags( optional ? FlagOptional : 0 )
{}

bool QgsProcessingParameterDefinition::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() && !mDefault.isValid() )
    return mFlags & FlagOptional;

  if ( ( input.type() == QVariant::String && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.type() == QVariant::String && mDefault.toString().isEmpty() ) )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterDefinition::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterDefinition::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += type() + ' ';
  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterDefinition::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  // base class method is probably not much use
  if ( QgsProcessingParameterType *t = QgsApplication::processingRegistry()->parameterType( type() ) )
  {
    switch ( outputType )
    {
      case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
      {
        QString code = t->className() + QStringLiteral( "('%1', '%2'" ).arg( name(), description() );
        if ( mFlags & FlagOptional )
          code += QStringLiteral( ", optional=True" );

        QgsProcessingContext c;
        code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
        return code;
      }
    }
  }

  // oh well, we tried
  return QString();
}

QVariantMap QgsProcessingParameterDefinition::toVariantMap() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "parameter_type" ), type() );
  map.insert( QStringLiteral( "name" ), mName );
  map.insert( QStringLiteral( "description" ), mDescription );
  map.insert( QStringLiteral( "default" ), mDefault );
  map.insert( QStringLiteral( "flags" ), static_cast< int >( mFlags ) );
  map.insert( QStringLiteral( "metadata" ), mMetadata );
  return map;
}

bool QgsProcessingParameterDefinition::fromVariantMap( const QVariantMap &map )
{
  mName = map.value( QStringLiteral( "name" ) ).toString();
  mDescription = map.value( QStringLiteral( "description" ) ).toString();
  mDefault = map.value( QStringLiteral( "default" ) );
  mFlags = static_cast< Flags >( map.value( QStringLiteral( "flags" ) ).toInt() );
  mMetadata = map.value( QStringLiteral( "metadata" ) ).toMap();
  return true;
}

QgsProcessingAlgorithm *QgsProcessingParameterDefinition::algorithm() const
{
  return mAlgorithm;
}

QgsProcessingProvider *QgsProcessingParameterDefinition::provider() const
{
  return mAlgorithm ? mAlgorithm->provider() : nullptr;
}

QString QgsProcessingParameterDefinition::toolTip() const
{
  return QStringLiteral( "<p><b>%1</b></p><p>%2</p>" ).arg(
           description(),
           QObject::tr( "Python identifier: ‘%1’" ).arg( QStringLiteral( "<i>%1</i>" ).arg( name() ) ) );
}

QgsProcessingParameterBoolean::QgsProcessingParameterBoolean( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{}

QgsProcessingParameterDefinition *QgsProcessingParameterBoolean::clone() const
{
  return new QgsProcessingParameterBoolean( *this );
}

QString QgsProcessingParameterBoolean::valueAsPythonString( const QVariant &val, QgsProcessingContext & ) const
{
  if ( !val.isValid() )
    return QStringLiteral( "None" );

  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );
  return val.toBool() ? QStringLiteral( "True" ) : QStringLiteral( "False" );
}

QString QgsProcessingParameterBoolean::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += type() + ' ';
  code += mDefault.toBool() ? QStringLiteral( "true" ) : QStringLiteral( "false" );
  return code.trimmed();
}

QgsProcessingParameterBoolean *QgsProcessingParameterBoolean::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterBoolean( name, description, definition.toLower().trimmed() != QStringLiteral( "false" ), isOptional );
}

QgsProcessingParameterCrs::QgsProcessingParameterCrs( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterCrs::clone() const
{
  return new QgsProcessingParameterCrs( *this );
}

bool QgsProcessingParameterCrs::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsCoordinateReferenceSystem>() )
  {
    return true;
  }
  else if ( input.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    return true;
  }
  else if ( input.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    return true;
  }

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  // direct map layer value
  if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( input ) ) )
    return true;

  if ( input.type() != QVariant::String || input.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterCrs::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsCoordinateReferenceSystem>() )
  {
    if ( !value.value< QgsCoordinateReferenceSystem >().isValid() )
      return QStringLiteral( "QgsCoordinateReferenceSystem()" );
    else
      return QStringLiteral( "QgsCoordinateReferenceSystem('%1')" ).arg( value.value< QgsCoordinateReferenceSystem >().authid() );
  }

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), value );
  QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
  if ( layer )
    return QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer->source() ) );

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QgsProcessingParameterCrs *QgsProcessingParameterCrs::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterCrs( name, description, definition.compare( QLatin1String( "none" ), Qt::CaseInsensitive ) == 0 ? QVariant() : definition, isOptional );
}

QgsProcessingParameterMapLayer::QgsProcessingParameterMapLayer( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterMapLayer::clone() const
{
  return new QgsProcessingParameterMapLayer( *this );
}

bool QgsProcessingParameterMapLayer::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( input ) ) )
  {
    return true;
  }

  if ( input.type() != QVariant::String || input.toString().isEmpty() )
    return mFlags & FlagOptional;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  // try to load as layer
  if ( QgsProcessingUtils::mapLayerFromString( input.toString(), *context ) )
    return true;

  return false;
}

QString QgsProcessingParameterMapLayer::valueAsPythonString( const QVariant &val, QgsProcessingContext &context ) const
{
  if ( !val.isValid() )
    return QStringLiteral( "None" );

  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer->source() ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QgsProcessingParameterMapLayer *QgsProcessingParameterMapLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterMapLayer( name, description, definition, isOptional );
}

QgsProcessingParameterExtent::QgsProcessingParameterExtent( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterExtent::clone() const
{
  return new QgsProcessingParameterExtent( *this );
}

bool QgsProcessingParameterExtent::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    return true;
  }
  else if ( input.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    return true;
  }

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.canConvert< QgsRectangle >() )
  {
    QgsRectangle r = input.value<QgsRectangle>();
    return !r.isNull();
  }
  if ( input.canConvert< QgsReferencedRectangle >() )
  {
    QgsReferencedRectangle r = input.value<QgsReferencedRectangle>();
    return !r.isNull();
  }

  // direct map layer value
  if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( input ) ) )
    return true;

  if ( input.type() != QVariant::String || input.toString().isEmpty() )
    return mFlags & FlagOptional;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  QRegularExpression rx( QStringLiteral( "^(.*?)\\s*,\\s*(.*?)\\s*,\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$" ) );
  QRegularExpressionMatch match = rx.match( input.toString() );
  if ( match.hasMatch() )
  {
    bool xMinOk = false;
    ( void )match.captured( 1 ).toDouble( &xMinOk );
    bool xMaxOk = false;
    ( void )match.captured( 2 ).toDouble( &xMaxOk );
    bool yMinOk = false;
    ( void )match.captured( 3 ).toDouble( &yMinOk );
    bool yMaxOk = false;
    ( void )match.captured( 4 ).toDouble( &yMaxOk );
    if ( xMinOk && xMaxOk && yMinOk && yMaxOk )
      return true;
  }

  // try as layer extent
  return QgsProcessingUtils::mapLayerFromString( input.toString(), *context );
}

QString QgsProcessingParameterExtent::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert< QgsRectangle >() )
  {
    QgsRectangle r = value.value<QgsRectangle>();
    return QStringLiteral( "'%1, %3, %2, %4'" ).arg( qgsDoubleToString( r.xMinimum() ),
           qgsDoubleToString( r.yMinimum() ),
           qgsDoubleToString( r.xMaximum() ),
           qgsDoubleToString( r.yMaximum() ) );
  }
  if ( value.canConvert< QgsReferencedRectangle >() )
  {
    QgsReferencedRectangle r = value.value<QgsReferencedRectangle>();
    return QStringLiteral( "'%1, %3, %2, %4 [%5]'" ).arg( qgsDoubleToString( r.xMinimum() ),
           qgsDoubleToString( r.yMinimum() ),
           qgsDoubleToString( r.xMaximum() ),
           qgsDoubleToString( r.yMaximum() ),                                                                                                                             r.crs().authid() );
  }

  QVariantMap p;
  p.insert( name(), value );
  QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
  if ( layer )
    return QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer->source() ) );

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QgsProcessingParameterExtent *QgsProcessingParameterExtent::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterExtent( name, description, definition, isOptional );
}

QgsProcessingParameterPoint::QgsProcessingParameterPoint( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterPoint::clone() const
{
  return new QgsProcessingParameterPoint( *this );
}

bool QgsProcessingParameterPoint::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.canConvert< QgsPointXY >() )
  {
    return true;
  }
  if ( input.canConvert< QgsReferencedPointXY >() )
  {
    return true;
  }
  if ( input.canConvert< QgsGeometry >() )
  {
    return true;
  }

  if ( input.type() == QVariant::String )
  {
    if ( input.toString().isEmpty() )
      return mFlags & FlagOptional;
  }

  QRegularExpression rx( QStringLiteral( "^\\s*\\(?\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*\\)?\\s*$" ) );

  QRegularExpressionMatch match = rx.match( input.toString() );
  if ( match.hasMatch() )
  {
    bool xOk = false;
    ( void )match.captured( 1 ).toDouble( &xOk );
    bool yOk = false;
    ( void )match.captured( 2 ).toDouble( &yOk );
    return xOk && yOk;
  }
  else
    return false;
}

QString QgsProcessingParameterPoint::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert< QgsPointXY >() )
  {
    QgsPointXY r = value.value<QgsPointXY>();
    return QStringLiteral( "'%1,%2'" ).arg( qgsDoubleToString( r.x() ),
                                            qgsDoubleToString( r.y() ) );
  }
  else if ( value.canConvert< QgsReferencedPointXY >() )
  {
    QgsReferencedPointXY r = value.value<QgsReferencedPointXY>();
    return QStringLiteral( "'%1,%2 [%3]'" ).arg( qgsDoubleToString( r.x() ),
           qgsDoubleToString( r.y() ),
           r.crs().authid() );
  }
  else if ( value.canConvert< QgsGeometry >() )
  {
    const QgsGeometry g = value.value<QgsGeometry>();
    if ( !g.isNull() )
    {
      const QString wkt = g.asWkt();
      return QStringLiteral( "QgsGeometry.fromWkt('%1')" ).arg( wkt );
    }
  }

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QgsProcessingParameterPoint *QgsProcessingParameterPoint::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterPoint( name, description, definition, isOptional );
}

QgsProcessingParameterFile::QgsProcessingParameterFile( const QString &name, const QString &description, Behavior behavior, const QString &extension, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mBehavior( behavior )
  , mExtension( extension )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterFile::clone() const
{
  return new QgsProcessingParameterFile( *this );
}

bool QgsProcessingParameterFile::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  QString string = input.toString().trimmed();

  if ( input.type() != QVariant::String || string.isEmpty() )
    return mFlags & FlagOptional;

  switch ( mBehavior )
  {
    case File:
    {
      if ( !mExtension.isEmpty() )
        return string.endsWith( mExtension, Qt::CaseInsensitive );
      return true;
    }

    case Folder:
      return true;
  }
  return true;
}

QString QgsProcessingParameterFile::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += ( mBehavior == File ? QStringLiteral( "file" ) : QStringLiteral( "folder" ) ) + ' ';
  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterFile::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {

      QString code = QStringLiteral( "QgsProcessingParameterFile('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );
      code += QStringLiteral( ", behavior=%1" ).arg( mBehavior == File ? QStringLiteral( "QgsProcessingParameterFile.File" ) : QStringLiteral( "QgsProcessingParameterFile.Folder" ) );
      code += QStringLiteral( ", extension='%1'" ).arg( mExtension );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterFile::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "behavior" ), mBehavior );
  map.insert( QStringLiteral( "extension" ), mExtension );
  return map;
}

bool QgsProcessingParameterFile::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mBehavior = static_cast< Behavior >( map.value( QStringLiteral( "behavior" ) ).toInt() );
  mExtension = map.value( QStringLiteral( "extension" ) ).toString();
  return true;
}

QgsProcessingParameterFile *QgsProcessingParameterFile::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition, QgsProcessingParameterFile::Behavior behavior )
{
  return new QgsProcessingParameterFile( name, description, behavior, QString(), definition, isOptional );
}

QgsProcessingParameterMatrix::QgsProcessingParameterMatrix( const QString &name, const QString &description, int numberRows, bool fixedNumberRows, const QStringList &headers, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mHeaders( headers )
  , mNumberRows( numberRows )
  , mFixedNumberRows( fixedNumberRows )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterMatrix::clone() const
{
  return new QgsProcessingParameterMatrix( *this );
}

bool QgsProcessingParameterMatrix::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.type() == QVariant::String )
  {
    if ( input.toString().isEmpty() )
      return mFlags & FlagOptional;
    return true;
  }
  else if ( input.type() == QVariant::List )
  {
    if ( input.toList().isEmpty() )
      return mFlags & FlagOptional;
    return true;
  }
  else if ( input.type() == QVariant::Double || input.type() == QVariant::Int )
  {
    return true;
  }

  return false;
}

QString QgsProcessingParameterMatrix::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), value );
  QVariantList list = QgsProcessingParameters::parameterAsMatrix( this, p, context );

  QStringList parts;
  Q_FOREACH ( const QVariant &v, list )
  {
    if ( v.type() == QVariant::List )
    {
      QStringList parts2;
      Q_FOREACH ( const QVariant &v2, v.toList() )
      {
        if ( v2.isNull() || !v2.isValid() )
          parts2 << QStringLiteral( "None" );
        else if ( v2.toString().isEmpty() )
          parts2 << QStringLiteral( "''" );
        else
          parts2 << v2.toString();
      }
      parts << parts2.join( ',' ).prepend( '[' ).append( ']' );
    }
    else
    {
      if ( v.isNull() || !v.isValid() )
        parts << QStringLiteral( "None" );
      else if ( v.toString().isEmpty() )
        parts << QStringLiteral( "''" );
      else
        parts << v.toString();
    }
  }

  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterMatrix::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterMatrix('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );
      code += QStringLiteral( ", numberRows=" ).arg( mNumberRows );
      code += QStringLiteral( ", hasFixedNumberRows=" ).arg( mFixedNumberRows ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QStringList headers;
      headers.reserve( mHeaders.size() );
      for ( const QString &h : mHeaders )
        headers << QgsProcessingUtils::stringToPythonLiteral( h );
      code += QStringLiteral( ", headers=[%1]" ).arg( headers.join( ',' ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterMatrix::headers() const
{
  return mHeaders;
}

void QgsProcessingParameterMatrix::setHeaders( const QStringList &headers )
{
  mHeaders = headers;
}

int QgsProcessingParameterMatrix::numberRows() const
{
  return mNumberRows;
}

void QgsProcessingParameterMatrix::setNumberRows( int numberRows )
{
  mNumberRows = numberRows;
}

bool QgsProcessingParameterMatrix::hasFixedNumberRows() const
{
  return mFixedNumberRows;
}

void QgsProcessingParameterMatrix::setHasFixedNumberRows( bool fixedNumberRows )
{
  mFixedNumberRows = fixedNumberRows;
}

QVariantMap QgsProcessingParameterMatrix::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "headers" ), mHeaders );
  map.insert( QStringLiteral( "rows" ), mNumberRows );
  map.insert( QStringLiteral( "fixed_number_rows" ), mFixedNumberRows );
  return map;
}

bool QgsProcessingParameterMatrix::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mHeaders = map.value( QStringLiteral( "headers" ) ).toStringList();
  mNumberRows = map.value( QStringLiteral( "rows" ) ).toInt();
  mFixedNumberRows = map.value( QStringLiteral( "fixed_number_rows" ) ).toBool();
  return true;
}

QgsProcessingParameterMatrix *QgsProcessingParameterMatrix::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterMatrix( name, description, 0, false, QStringList(), definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterMultipleLayers::QgsProcessingParameterMultipleLayers( const QString &name, const QString &description, QgsProcessing::SourceType layerType, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mLayerType( layerType )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterMultipleLayers::clone() const
{
  return new QgsProcessingParameterMultipleLayers( *this );
}

bool QgsProcessingParameterMultipleLayers::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( input ) ) )
  {
    return true;
  }

  if ( input.type() == QVariant::String )
  {
    if ( input.toString().isEmpty() )
      return mFlags & FlagOptional;

    if ( mMinimumNumberInputs > 1 )
      return false;

    if ( !context )
      return true;

    return QgsProcessingUtils::mapLayerFromString( input.toString(), *context );
  }
  else if ( input.type() == QVariant::List )
  {
    if ( input.toList().count() < mMinimumNumberInputs )
      return mFlags & FlagOptional;

    if ( mMinimumNumberInputs > input.toList().count() )
      return false;

    if ( !context )
      return true;

    Q_FOREACH ( const QVariant &v, input.toList() )
    {
      if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( v ) ) )
        continue;

      if ( !QgsProcessingUtils::mapLayerFromString( v.toString(), *context ) )
        return false;
    }
    return true;
  }
  else if ( input.type() == QVariant::StringList )
  {
    if ( input.toStringList().count() < mMinimumNumberInputs )
      return mFlags & FlagOptional;

    if ( mMinimumNumberInputs > input.toStringList().count() )
      return false;

    if ( !context )
      return true;

    Q_FOREACH ( const QString &v, input.toStringList() )
    {
      if ( !QgsProcessingUtils::mapLayerFromString( v, *context ) )
        return false;
    }
    return true;
  }
  return false;
}

QString QgsProcessingParameterMultipleLayers::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), value );
  QList<QgsMapLayer *> list = QgsProcessingParameters::parameterAsLayerList( this, p, context );
  if ( !list.isEmpty() )
  {
    QStringList parts;
    Q_FOREACH ( const QgsMapLayer *layer, list )
    {
      parts << QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer->source() ) );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QString QgsProcessingParameterMultipleLayers::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  switch ( mLayerType )
  {
    case QgsProcessing::TypeRaster:
      code += QStringLiteral( "multiple raster" );
      break;

    case QgsProcessing::TypeFile:
      code += QStringLiteral( "multiple file" );
      break;

    default:
      code += QStringLiteral( "multiple vector" );
      break;
  }
  code += ' ';
  if ( mDefault.type() == QVariant::List )
  {
    QStringList parts;
    Q_FOREACH ( const QVariant &var, mDefault.toList() )
    {
      parts << var.toString();
    }
    code += parts.join( ',' );
  }
  else if ( mDefault.type() == QVariant::StringList )
  {
    code += mDefault.toStringList().join( ',' );
  }
  else
  {
    code += mDefault.toString();
  }
  return code.trimmed();
}

QString QgsProcessingParameterMultipleLayers::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterMultipleLayers('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      QString layerType = QStringLiteral( "QgsProcessing.%1" ).arg( QgsProcessing::sourceTypeToString( mLayerType ) );

      code += QStringLiteral( ", layerType=%1" ).arg( layerType );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QgsProcessing::SourceType QgsProcessingParameterMultipleLayers::layerType() const
{
  return mLayerType;
}

void QgsProcessingParameterMultipleLayers::setLayerType( QgsProcessing::SourceType type )
{
  mLayerType = type;
}

int QgsProcessingParameterMultipleLayers::minimumNumberInputs() const
{
  return mMinimumNumberInputs;
}

void QgsProcessingParameterMultipleLayers::setMinimumNumberInputs( int minimumNumberInputs )
{
  if ( mMinimumNumberInputs >= 1 || !( flags() & QgsProcessingParameterDefinition::FlagOptional ) )
    mMinimumNumberInputs = minimumNumberInputs;
}

QVariantMap QgsProcessingParameterMultipleLayers::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "layer_type" ), mLayerType );
  map.insert( QStringLiteral( "min_inputs" ), mMinimumNumberInputs );
  return map;
}

bool QgsProcessingParameterMultipleLayers::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mLayerType = static_cast< QgsProcessing::SourceType >( map.value( QStringLiteral( "layer_type" ) ).toInt() );
  mMinimumNumberInputs = map.value( QStringLiteral( "min_inputs" ) ).toInt();
  return true;
}

QgsProcessingParameterMultipleLayers *QgsProcessingParameterMultipleLayers::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString type = definition;
  QString defaultVal;
  QRegularExpression re( QStringLiteral( "(.*?)\\s+(.*)" ) );
  QRegularExpressionMatch m = re.match( definition );
  if ( m.hasMatch() )
  {
    type = m.captured( 1 ).toLower().trimmed();
    defaultVal = m.captured( 2 );
  }
  QgsProcessing::SourceType layerType = QgsProcessing::TypeVectorAnyGeometry;
  if ( type == QStringLiteral( "vector" ) )
    layerType = QgsProcessing::TypeVectorAnyGeometry;
  else if ( type == QStringLiteral( "raster" ) )
    layerType = QgsProcessing::TypeRaster;
  else if ( type == QStringLiteral( "file" ) )
    layerType = QgsProcessing::TypeFile;
  return new QgsProcessingParameterMultipleLayers( name, description, layerType, defaultVal.isEmpty() ? QVariant() : defaultVal, isOptional );
}

QgsProcessingParameterNumber::QgsProcessingParameterNumber( const QString &name, const QString &description, Type type, const QVariant &defaultValue, bool optional, double minValue, double maxValue )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mMin( minValue )
  , mMax( maxValue )
  , mDataType( type )
{
  if ( mMin >= mMax )
  {
    QgsMessageLog::logMessage( QObject::tr( "Invalid number parameter \"%1\": min value %2 is >= max value %3!" ).arg( name ).arg( mMin ).arg( mMax ), QObject::tr( "Processing" ) );
  }
}

QgsProcessingParameterDefinition *QgsProcessingParameterNumber::clone() const
{
  return new QgsProcessingParameterNumber( *this );
}

bool QgsProcessingParameterNumber::checkValueIsAcceptable( const QVariant &value, QgsProcessingContext * ) const
{
  QVariant input = value;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & FlagOptional;

    input = defaultValue();
  }

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  bool ok = false;
  double res = input.toDouble( &ok );
  if ( !ok )
    return mFlags & FlagOptional;

  return !( res < mMin || res > mMax );
}

QString QgsProcessingParameterNumber::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  return value.toString();
}

QString QgsProcessingParameterNumber::toolTip() const
{
  QString text = QgsProcessingParameterDefinition::toolTip();
  QStringList parts;
  if ( mMin > std::numeric_limits<double>::lowest() + 1 )
    parts << QObject::tr( "Minimum value: %1" ).arg( mMin );
  if ( mMax < std::numeric_limits<double>::max() )
    parts << QObject::tr( "Maximum value: %1" ).arg( mMax );
  if ( mDefault.isValid() )
    parts << QObject::tr( "Default value: %1" ).arg( mDataType == Integer ? mDefault.toInt() : mDefault.toDouble() );
  QString extra = parts.join( QStringLiteral( "<br />" ) );
  if ( !extra.isEmpty() )
    text += QStringLiteral( "<p>%1</p>" ).arg( extra );
  return text;
}

QString QgsProcessingParameterNumber::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterNumber('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      code += QStringLiteral( ", type=%1" ).arg( mDataType == Integer ? QStringLiteral( "QgsProcessingParameterNumber.Integer" ) : QStringLiteral( "QgsProcessingParameterNumber.Double" ) );

      if ( mMin != std::numeric_limits<double>::lowest() + 1 )
        code += QStringLiteral( ", minValue=%1" ).arg( mMin );
      if ( mMax != std::numeric_limits<double>::max() )
        code += QStringLiteral( ", maxValue=%1" ).arg( mMax );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

double QgsProcessingParameterNumber::minimum() const
{
  return mMin;
}

void QgsProcessingParameterNumber::setMinimum( double min )
{
  mMin = min;
}

double QgsProcessingParameterNumber::maximum() const
{
  return mMax;
}

void QgsProcessingParameterNumber::setMaximum( double max )
{
  mMax = max;
}

QgsProcessingParameterNumber::Type QgsProcessingParameterNumber::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterNumber::setDataType( Type dataType )
{
  mDataType = dataType;
}

QVariantMap QgsProcessingParameterNumber::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "min" ), mMin );
  map.insert( QStringLiteral( "max" ), mMax );
  map.insert( QStringLiteral( "data_type" ), mDataType );
  return map;
}

bool QgsProcessingParameterNumber::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mMin = map.value( QStringLiteral( "min" ) ).toDouble();
  mMax = map.value( QStringLiteral( "max" ) ).toDouble();
  mDataType = static_cast< Type >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  return true;
}

QgsProcessingParameterNumber *QgsProcessingParameterNumber::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterNumber( name, description, Double, definition.isEmpty() ? QVariant()
         : ( definition.toLower().trimmed() == QStringLiteral( "none" ) ? QVariant() : definition ), isOptional );
}

QgsProcessingParameterRange::QgsProcessingParameterRange( const QString &name, const QString &description, QgsProcessingParameterNumber::Type type, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mDataType( type )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterRange::clone() const
{
  return new QgsProcessingParameterRange( *this );
}

bool QgsProcessingParameterRange::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() == QVariant::String )
  {
    QStringList list = input.toString().split( ',' );
    if ( list.count() != 2 )
      return mFlags & FlagOptional;
    bool ok = false;
    list.at( 0 ).toDouble( &ok );
    bool ok2 = false;
    list.at( 1 ).toDouble( &ok2 );
    if ( !ok || !ok2 )
      return mFlags & FlagOptional;
    return true;
  }
  else if ( input.type() == QVariant::List )
  {
    if ( input.toList().count() != 2 )
      return mFlags & FlagOptional;

    bool ok = false;
    input.toList().at( 0 ).toDouble( &ok );
    bool ok2 = false;
    input.toList().at( 1 ).toDouble( &ok2 );
    if ( !ok || !ok2 )
      return mFlags & FlagOptional;
    return true;
  }

  return false;
}

QString QgsProcessingParameterRange::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), value );
  QList< double > parts = QgsProcessingParameters::parameterAsRange( this, p, context );

  QStringList stringParts;
  Q_FOREACH ( double v, parts )
  {
    stringParts << QString::number( v );
  }
  return stringParts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterRange::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterRange('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      code += QStringLiteral( ", type=%1" ).arg( mDataType == QgsProcessingParameterNumber::Integer ? QStringLiteral( "QgsProcessingParameterNumber.Integer" ) : QStringLiteral( "QgsProcessingParameterNumber.Double" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QgsProcessingParameterNumber::Type QgsProcessingParameterRange::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterRange::setDataType( QgsProcessingParameterNumber::Type dataType )
{
  mDataType = dataType;
}

QVariantMap QgsProcessingParameterRange::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "data_type" ), mDataType );
  return map;
}

bool QgsProcessingParameterRange::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mDataType = static_cast< QgsProcessingParameterNumber::Type >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  return true;
}

QgsProcessingParameterRange *QgsProcessingParameterRange::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterRange( name, description, QgsProcessingParameterNumber::Double, definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterRasterLayer::QgsProcessingParameterRasterLayer( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterRasterLayer::clone() const
{
  return new QgsProcessingParameterRasterLayer( *this );
}

bool QgsProcessingParameterRasterLayer::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( qobject_cast< QgsRasterLayer * >( qvariant_cast<QObject *>( input ) ) )
    return true;

  if ( input.type() != QVariant::String || input.toString().isEmpty() )
    return mFlags & FlagOptional;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  // try to load as layer
  if ( QgsProcessingUtils::mapLayerFromString( input.toString(), *context, true, QgsProcessingUtils::Raster ) )
    return true;

  return false;
}

QString QgsProcessingParameterRasterLayer::valueAsPythonString( const QVariant &val, QgsProcessingContext &context ) const
{
  if ( !val.isValid() )
    return QStringLiteral( "None" );

  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsRasterLayer *layer = QgsProcessingParameters::parameterAsRasterLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer->source() ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QgsProcessingParameterRasterLayer *QgsProcessingParameterRasterLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterRasterLayer( name, description, definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterEnum::QgsProcessingParameterEnum( const QString &name, const QString &description, const QStringList &options, bool allowMultiple, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mOptions( options )
  , mAllowMultiple( allowMultiple )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterEnum::clone() const
{
  return new QgsProcessingParameterEnum( *this );
}

bool QgsProcessingParameterEnum::checkValueIsAcceptable( const QVariant &value, QgsProcessingContext * ) const
{
  QVariant input = value;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & FlagOptional;

    input = defaultValue();
  }

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() == QVariant::List )
  {
    if ( !mAllowMultiple )
      return false;

    const QVariantList values = input.toList();
    if ( values.empty() && !( mFlags & FlagOptional ) )
      return false;

    for ( const QVariant &val : values )
    {
      bool ok = false;
      int res = val.toInt( &ok );
      if ( !ok )
        return false;
      else if ( res < 0 || res >= mOptions.count() )
        return false;
    }

    return true;
  }
  else if ( input.type() == QVariant::String )
  {
    QStringList parts = input.toString().split( ',' );
    if ( parts.count() > 1 && !mAllowMultiple )
      return false;

    Q_FOREACH ( const QString &part, parts )
    {
      bool ok = false;
      int res = part.toInt( &ok );
      if ( !ok )
        return false;
      else if ( res < 0 || res >= mOptions.count() )
        return false;
    }
    return true;
  }
  else if ( input.type() == QVariant::Int || input.type() == QVariant::Double )
  {
    bool ok = false;
    int res = input.toInt( &ok );
    if ( !ok )
      return false;
    else if ( res >= 0 && res < mOptions.count() )
      return true;
  }
  return false;
}

QString QgsProcessingParameterEnum::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.type() == QVariant::List )
  {
    QStringList parts;
    Q_FOREACH ( const QVariant &val, value.toList() )
    {
      parts << QString::number( static_cast< int >( val.toDouble() ) );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else if ( value.type() == QVariant::String )
  {
    QStringList parts = value.toString().split( ',' );
    if ( parts.count() > 1 )
    {
      return parts.join( ',' ).prepend( '[' ).append( ']' );
    }
  }

  return QString::number( static_cast< int >( value.toDouble() ) );
}

QString QgsProcessingParameterEnum::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "enum " );

  if ( mAllowMultiple )
    code += QStringLiteral( "multiple " );

  code += mOptions.join( ';' ) + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterEnum::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterEnum('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      QStringList options;
      options.reserve( mOptions.size() );
      for ( const QString &o : mOptions )
        options << QgsProcessingUtils::stringToPythonLiteral( o );
      code += QStringLiteral( ", options=[%1]" ).arg( options.join( ',' ) );

      code += QStringLiteral( ", allowMultiple=%1" ).arg( mAllowMultiple ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterEnum::options() const
{
  return mOptions;
}

void QgsProcessingParameterEnum::setOptions( const QStringList &options )
{
  mOptions = options;
}

bool QgsProcessingParameterEnum::allowMultiple() const
{
  return mAllowMultiple;
}

void QgsProcessingParameterEnum::setAllowMultiple( bool allowMultiple )
{
  mAllowMultiple = allowMultiple;
}

QVariantMap QgsProcessingParameterEnum::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "options" ), mOptions );
  map.insert( QStringLiteral( "allow_multiple" ), mAllowMultiple );
  return map;
}

bool QgsProcessingParameterEnum::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mOptions = map.value( QStringLiteral( "options" ) ).toStringList();
  mAllowMultiple = map.value( QStringLiteral( "allow_multiple" ) ).toBool();
  return true;
}

QgsProcessingParameterEnum *QgsProcessingParameterEnum::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString defaultVal;
  bool multiple = false;
  QString def = definition;
  if ( def.startsWith( QLatin1String( "multiple" ), Qt::CaseInsensitive ) )
  {
    multiple = true;
    def = def.mid( 9 );
  }

  QRegularExpression re( QStringLiteral( "(.*)\\s+(.*?)$" ) );
  QRegularExpressionMatch m = re.match( def );
  QString values = def;
  if ( m.hasMatch() )
  {
    values = m.captured( 1 ).trimmed();
    defaultVal = m.captured( 2 );
  }

  return new QgsProcessingParameterEnum( name, description, values.split( ';' ), multiple, defaultVal.isEmpty() ? QVariant() : defaultVal, isOptional );
}

QgsProcessingParameterString::QgsProcessingParameterString( const QString &name, const QString &description, const QVariant &defaultValue, bool multiLine, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mMultiLine( multiLine )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterString::clone() const
{
  return new QgsProcessingParameterString( *this );
}

QString QgsProcessingParameterString::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() || value.isNull() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterString::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "string " );

  if ( mMultiLine )
    code += QStringLiteral( "long " );

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterString::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterString('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );
      code += QStringLiteral( ", multiLine=%1" ).arg( mMultiLine ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

bool QgsProcessingParameterString::multiLine() const
{
  return mMultiLine;
}

void QgsProcessingParameterString::setMultiLine( bool multiLine )
{
  mMultiLine = multiLine;
}

QVariantMap QgsProcessingParameterString::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "multiline" ), mMultiLine );
  return map;
}

bool QgsProcessingParameterString::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mMultiLine = map.value( QStringLiteral( "multiline" ) ).toBool();
  return true;
}

QgsProcessingParameterString *QgsProcessingParameterString::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString def = definition;
  bool multiLine = false;
  if ( def.startsWith( QLatin1String( "long" ), Qt::CaseInsensitive ) )
  {
    multiLine = true;
    def = def.mid( 5 );
  }

  if ( def.startsWith( '"' ) || def.startsWith( '\'' ) )
    def = def.mid( 1 );
  if ( def.endsWith( '"' ) || def.endsWith( '\'' ) )
    def.chop( 1 );

  QVariant defaultValue = def;
  if ( def == QStringLiteral( "None" ) )
    defaultValue = QVariant();

  return new QgsProcessingParameterString( name, description, defaultValue, multiLine, isOptional );
}

//
// QgsProcessingParameterAuthConfig
//

QgsProcessingParameterAuthConfig::QgsProcessingParameterAuthConfig( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterAuthConfig::clone() const
{
  return new QgsProcessingParameterAuthConfig( *this );
}

QString QgsProcessingParameterAuthConfig::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterAuthConfig::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "authcfg " );

  code += mDefault.toString();
  return code.trimmed();
}

QgsProcessingParameterAuthConfig *QgsProcessingParameterAuthConfig::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString def = definition;

  if ( def.startsWith( '"' ) || def.startsWith( '\'' ) )
    def = def.mid( 1 );
  if ( def.endsWith( '"' ) || def.endsWith( '\'' ) )
    def.chop( 1 );

  QVariant defaultValue = def;
  if ( def == QStringLiteral( "None" ) )
    defaultValue = QVariant();

  return new QgsProcessingParameterAuthConfig( name, description, defaultValue, isOptional );
}


//
// QgsProcessingParameterExpression
//

QgsProcessingParameterExpression::QgsProcessingParameterExpression( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameterName( parentLayerParameterName )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterExpression::clone() const
{
  return new QgsProcessingParameterExpression( *this );
}

QString QgsProcessingParameterExpression::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QStringList QgsProcessingParameterExpression::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentLayerParameterName.isEmpty() )
    depends << mParentLayerParameterName;
  return depends;
}

QString QgsProcessingParameterExpression::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterExpression('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      code += QStringLiteral( ", parentLayerParameterName='%1'" ).arg( mParentLayerParameterName );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterExpression::parentLayerParameterName() const
{
  return mParentLayerParameterName;
}

void QgsProcessingParameterExpression::setParentLayerParameterName( const QString &parentLayerParameterName )
{
  mParentLayerParameterName = parentLayerParameterName;
}

QVariantMap QgsProcessingParameterExpression::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "parent_layer" ), mParentLayerParameterName );
  return map;
}

bool QgsProcessingParameterExpression::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( QStringLiteral( "parent_layer" ) ).toString();
  return true;
}

QgsProcessingParameterExpression *QgsProcessingParameterExpression::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterExpression( name, description, definition, QString(), isOptional );
}

QgsProcessingParameterVectorLayer::QgsProcessingParameterVectorLayer( const QString &name, const QString &description, const QList<int> &types, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , QgsProcessingParameterLimitedDataTypes( types )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterVectorLayer::clone() const
{
  return new QgsProcessingParameterVectorLayer( *this );
}

bool QgsProcessingParameterVectorLayer::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext *context ) const
{
  if ( !v.isValid() )
    return mFlags & FlagOptional;

  QVariant var = v;

  if ( var.canConvert<QgsProperty>() )
  {
    QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( var ) ) )
    return true;

  if ( var.type() != QVariant::String || var.toString().isEmpty() )
    return mFlags & FlagOptional;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  // try to load as layer
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context, true, QgsProcessingUtils::Vector ) )
    return true;

  return false;
}

QString QgsProcessingParameterVectorLayer::valueAsPythonString( const QVariant &val, QgsProcessingContext &context ) const
{
  if ( !val.isValid() )
    return QStringLiteral( "None" );

  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsVectorLayer *layer = QgsProcessingParameters::parameterAsVectorLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer->source() ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QString QgsProcessingParameterVectorLayer::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterVectorLayer('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      if ( !mDataTypes.empty() )
      {
        QStringList options;
        for ( int t : mDataTypes )
          options << QStringLiteral( "QgsProcessing.%1" ).arg( QgsProcessing::sourceTypeToString( static_cast< QgsProcessing::SourceType >( t ) ) );
        code += QStringLiteral( ", types=[%1]" ).arg( options.join( ',' ) );
      }

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QList<int> QgsProcessingParameterLimitedDataTypes::dataTypes() const
{
  return mDataTypes;
}

void QgsProcessingParameterLimitedDataTypes::setDataTypes( const QList<int> &types )
{
  mDataTypes = types;
}

QVariantMap QgsProcessingParameterVectorLayer::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  QVariantList types;
  Q_FOREACH ( int type, mDataTypes )
  {
    types << type;
  }
  map.insert( QStringLiteral( "data_types" ), types );
  return map;
}

bool QgsProcessingParameterVectorLayer::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mDataTypes.clear();
  QVariantList values = map.value( QStringLiteral( "data_types" ) ).toList();
  Q_FOREACH ( const QVariant &val, values )
  {
    mDataTypes << val.toInt();
  }
  return true;
}

QgsProcessingParameterVectorLayer *QgsProcessingParameterVectorLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterVectorLayer( name, description, QList< int>(),  definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterMeshLayer::QgsProcessingParameterMeshLayer( const QString &name,
    const QString &description,
    const QVariant &defaultValue,
    bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterMeshLayer::clone() const
{
  return new QgsProcessingParameterMeshLayer( *this );
}

bool QgsProcessingParameterMeshLayer::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext *context ) const
{
  if ( !v.isValid() )
    return mFlags & FlagOptional;

  QVariant var = v;

  if ( var.canConvert<QgsProperty>() )
  {
    QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( qobject_cast< QgsMeshLayer * >( qvariant_cast<QObject *>( var ) ) )
    return true;

  if ( var.type() != QVariant::String || var.toString().isEmpty() )
    return mFlags & FlagOptional;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  // try to load as layer
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context, true, QgsProcessingUtils::Mesh ) )
    return true;

  return false;
}

QString QgsProcessingParameterMeshLayer::valueAsPythonString( const QVariant &val, QgsProcessingContext &context ) const
{
  if ( !val.isValid() )
    return QStringLiteral( "None" );

  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsMeshLayer *layer = QgsProcessingParameters::parameterAsMeshLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer->source() ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QgsProcessingParameterMeshLayer *QgsProcessingParameterMeshLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterMeshLayer( name, description,  definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterField::QgsProcessingParameterField( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, DataType type, bool allowMultiple, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameterName( parentLayerParameterName )
  , mDataType( type )
  , mAllowMultiple( allowMultiple )
{

}


QgsProcessingParameterDefinition *QgsProcessingParameterField::clone() const
{
  return new QgsProcessingParameterField( *this );
}

bool QgsProcessingParameterField::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() == QVariant::List || input.type() == QVariant::StringList )
  {
    if ( !mAllowMultiple )
      return false;

    if ( input.toList().isEmpty() && !( mFlags & FlagOptional ) )
      return false;
  }
  else if ( input.type() == QVariant::String )
  {
    if ( input.toString().isEmpty() )
      return mFlags & FlagOptional;

    QStringList parts = input.toString().split( ';' );
    if ( parts.count() > 1 && !mAllowMultiple )
      return false;
  }
  else
  {
    if ( input.toString().isEmpty() )
      return mFlags & FlagOptional;
  }
  return true;
}

QString QgsProcessingParameterField::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.type() == QVariant::List )
  {
    QStringList parts;
    Q_FOREACH ( const QVariant &val, value.toList() )
    {
      parts << QgsProcessingUtils::stringToPythonLiteral( val.toString() );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else if ( value.type() == QVariant::StringList )
  {
    QStringList parts;
    Q_FOREACH ( QString s, value.toStringList() )
    {
      parts << QgsProcessingUtils::stringToPythonLiteral( s );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterField::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "field " );

  switch ( mDataType )
  {
    case Numeric:
      code += QStringLiteral( "numeric " );
      break;

    case String:
      code += QStringLiteral( "string " );
      break;

    case DateTime:
      code += QStringLiteral( "datetime " );
      break;

    case Any:
      break;
  }

  if ( mAllowMultiple )
    code += QStringLiteral( "multiple " );

  code += mParentLayerParameterName + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterField::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterField('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      QString dataType;
      switch ( mDataType )
      {
        case Any:
          dataType = QStringLiteral( "QgsProcessingParameterField.Any" );
          break;

        case Numeric:
          dataType = QStringLiteral( "QgsProcessingParameterField.Numeric" );
          break;

        case String:
          dataType = QStringLiteral( "QgsProcessingParameterField.String" );
          break;

        case DateTime:
          dataType = QStringLiteral( "QgsProcessingParameterField.DateTime" );
          break;
      }
      code += QStringLiteral( ", type=%1" ).arg( dataType );

      code += QStringLiteral( ", parentLayerParameterName='%1'" ).arg( mParentLayerParameterName );
      code += QStringLiteral( ", allowMultiple=%1" ).arg( mAllowMultiple ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterField::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentLayerParameterName.isEmpty() )
    depends << mParentLayerParameterName;
  return depends;
}

QString QgsProcessingParameterField::parentLayerParameterName() const
{
  return mParentLayerParameterName;
}

void QgsProcessingParameterField::setParentLayerParameterName( const QString &parentLayerParameterName )
{
  mParentLayerParameterName = parentLayerParameterName;
}

QgsProcessingParameterField::DataType QgsProcessingParameterField::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterField::setDataType( DataType dataType )
{
  mDataType = dataType;
}

bool QgsProcessingParameterField::allowMultiple() const
{
  return mAllowMultiple;
}

void QgsProcessingParameterField::setAllowMultiple( bool allowMultiple )
{
  mAllowMultiple = allowMultiple;
}

QVariantMap QgsProcessingParameterField::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "parent_layer" ), mParentLayerParameterName );
  map.insert( QStringLiteral( "data_type" ), mDataType );
  map.insert( QStringLiteral( "allow_multiple" ), mAllowMultiple );
  return map;
}

bool QgsProcessingParameterField::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( QStringLiteral( "parent_layer" ) ).toString();
  mDataType = static_cast< DataType >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  mAllowMultiple = map.value( QStringLiteral( "allow_multiple" ) ).toBool();
  return true;
}

QgsProcessingParameterField *QgsProcessingParameterField::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString parent;
  DataType type = Any;
  bool allowMultiple = false;
  QString def = definition;

  if ( def.startsWith( QLatin1String( "numeric " ), Qt::CaseInsensitive ) )
  {
    type = Numeric;
    def = def.mid( 8 );
  }
  else if ( def.startsWith( QLatin1String( "string " ), Qt::CaseInsensitive ) )
  {
    type = String;
    def = def.mid( 7 );
  }
  else if ( def.startsWith( QLatin1String( "datetime " ), Qt::CaseInsensitive ) )
  {
    type = DateTime;
    def = def.mid( 9 );
  }

  if ( def.startsWith( QLatin1String( "multiple" ), Qt::CaseInsensitive ) )
  {
    allowMultiple = true;
    def = def.mid( 8 ).trimmed();
  }

  QRegularExpression re( QStringLiteral( "(.*?)\\s+(.*)$" ) );
  QRegularExpressionMatch m = re.match( def );
  if ( m.hasMatch() )
  {
    parent = m.captured( 1 ).trimmed();
    def = m.captured( 2 );
  }
  else
  {
    parent = def;
    def.clear();
  }

  return new QgsProcessingParameterField( name, description, def.isEmpty() ? QVariant() : def, parent, type, allowMultiple, isOptional );
}

QgsProcessingParameterFeatureSource::QgsProcessingParameterFeatureSource( const QString &name, const QString &description, const QList<int> &types, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , QgsProcessingParameterLimitedDataTypes( types )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterFeatureSource::clone() const
{
  return new QgsProcessingParameterFeatureSource( *this );
}

bool QgsProcessingParameterFeatureSource::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  QVariant var = input;
  if ( !var.isValid() )
    return mFlags & FlagOptional;

  if ( var.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( var );
    var = fromVar.source;
  }
  else if ( var.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }
  if ( qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( input ) ) )
  {
    return true;
  }

  if ( var.type() != QVariant::String || var.toString().isEmpty() )
    return mFlags & FlagOptional;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  // try to load as layer
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context, true, QgsProcessingUtils::Vector ) )
    return true;

  return false;
}

QString QgsProcessingParameterFeatureSource::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );
    if ( fromVar.source.propertyType() == QgsProperty::StaticProperty )
    {
      if ( fromVar.selectedFeaturesOnly )
      {
        return QStringLiteral( "QgsProcessingFeatureSourceDefinition('%1', True)" ).arg( fromVar.source.staticValue().toString() );
      }
      else
      {
        QString layerString = fromVar.source.staticValue().toString();
        // prefer to use layer source instead of id if possible (since it's persistent)
        if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( layerString, context, true, QgsProcessingUtils::Vector ) ) )
          layerString = layer->source();
        return QgsProcessingUtils::stringToPythonLiteral( layerString );
      }
    }
    else
    {
      if ( fromVar.selectedFeaturesOnly )
      {
        return QStringLiteral( "QgsProcessingFeatureSourceDefinition(QgsProperty.fromExpression('%1'), True)" ).arg( fromVar.source.asExpression() );
      }
      else
      {
        return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( fromVar.source.asExpression() );
      }
    }
  }
  else if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( value ) ) )
  {
    return QgsProcessingUtils::stringToPythonLiteral( layer->source() );
  }

  QString layerString = value.toString();

  // prefer to use layer source if possible (since it's persistent)
  if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( layerString, context, true, QgsProcessingUtils::Vector ) ) )
    layerString = layer->source();

  return QgsProcessingUtils::stringToPythonLiteral( layerString );
}

QString QgsProcessingParameterFeatureSource::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "source " );

  Q_FOREACH ( int type, mDataTypes )
  {
    switch ( type )
    {
      case QgsProcessing::TypeVectorPoint:
        code += QStringLiteral( "point " );
        break;

      case QgsProcessing::TypeVectorLine:
        code += QStringLiteral( "line " );
        break;

      case QgsProcessing::TypeVectorPolygon:
        code += QStringLiteral( "polygon " );
        break;

    }
  }

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterFeatureSource::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterFeatureSource('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      if ( !mDataTypes.empty() )
      {
        QStringList options;
        options.reserve( mDataTypes.size() );
        for ( int t : mDataTypes )
          options << QStringLiteral( "QgsProcessing.%1" ).arg( QgsProcessing::sourceTypeToString( static_cast< QgsProcessing::SourceType >( t ) ) );
        code += QStringLiteral( ", types=[%1]" ).arg( options.join( ',' ) );
      }

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QgsProcessingParameterLimitedDataTypes::QgsProcessingParameterLimitedDataTypes( const QList<int> &types )
  : mDataTypes( types )
{

}

QVariantMap QgsProcessingParameterFeatureSource::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  QVariantList types;
  Q_FOREACH ( int type, mDataTypes )
  {
    types << type;
  }
  map.insert( QStringLiteral( "data_types" ), types );
  return map;
}

bool QgsProcessingParameterFeatureSource::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mDataTypes.clear();
  QVariantList values = map.value( QStringLiteral( "data_types" ) ).toList();
  Q_FOREACH ( const QVariant &val, values )
  {
    mDataTypes << val.toInt();
  }
  return true;
}

QgsProcessingParameterFeatureSource *QgsProcessingParameterFeatureSource::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QList< int > types;
  QString def = definition;
  while ( true )
  {
    if ( def.startsWith( QLatin1String( "point" ), Qt::CaseInsensitive ) )
    {
      types << QgsProcessing::TypeVectorPoint;
      def = def.mid( 6 );
      continue;
    }
    else if ( def.startsWith( QLatin1String( "line" ), Qt::CaseInsensitive ) )
    {
      types << QgsProcessing::TypeVectorLine;
      def = def.mid( 5 );
      continue;
    }
    else if ( def.startsWith( QLatin1String( "polygon" ), Qt::CaseInsensitive ) )
    {
      types << QgsProcessing::TypeVectorPolygon;
      def = def.mid( 8 );
      continue;
    }
    break;
  }

  return new QgsProcessingParameterFeatureSource( name, description, types, def, isOptional );
}

QgsProcessingParameterFeatureSink::QgsProcessingParameterFeatureSink( const QString &name, const QString &description, QgsProcessing::SourceType type, const QVariant &defaultValue, bool optional, bool createByDefault )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional, createByDefault )
  , mDataType( type )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterFeatureSink::clone() const
{
  return new QgsProcessingParameterFeatureSink( *this );
}

bool QgsProcessingParameterFeatureSink::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  QVariant var = input;
  if ( !var.isValid() )
    return mFlags & FlagOptional;

  if ( var.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterFeatureSink::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == QgsProperty::StaticProperty )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( fromVar.sink.asExpression() );
    }
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterFeatureSink::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "sink " );

  switch ( mDataType )
  {
    case QgsProcessing::TypeVectorPoint:
      code += QStringLiteral( "point " );
      break;

    case QgsProcessing::TypeVectorLine:
      code += QStringLiteral( "line " );
      break;

    case QgsProcessing::TypeVectorPolygon:
      code += QStringLiteral( "polygon " );
      break;

    case QgsProcessing::TypeVector:
      code += QStringLiteral( "table " );
      break;

    default:
      break;
  }

  code += mDefault.toString();
  return code.trimmed();
}

QgsProcessingOutputDefinition *QgsProcessingParameterFeatureSink::toOutputDefinition() const
{
  return new QgsProcessingOutputVectorLayer( name(), description(), mDataType );
}

QString QgsProcessingParameterFeatureSink::defaultFileExtension() const
{
  if ( originalProvider() )
  {
    return originalProvider()->defaultVectorFileExtension( hasGeometry() );
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->defaultVectorFileExtension( hasGeometry() );
  }
  else
  {
    QgsSettings settings;
    if ( hasGeometry() )
    {
      return settings.value( QStringLiteral( "Processing/DefaultOutputVectorLayerExt" ), QStringLiteral( "shp" ), QgsSettings::Core ).toString();
    }
    else
    {
      return QStringLiteral( "dbf" );
    }
  }
}

QString QgsProcessingParameterFeatureSink::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterFeatureSink('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      code += QStringLiteral( ", type=QgsProcessing.%1" ).arg( QgsProcessing::sourceTypeToString( mDataType ) );

      code += QStringLiteral( ", createByDefault=%1" ).arg( createByDefault() ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterFeatureSink::supportedOutputVectorLayerExtensions() const
{
  if ( originalProvider() )
  {
    if ( hasGeometry() )
      return originalProvider()->supportedOutputVectorLayerExtensions();
    else
      return originalProvider()->supportedOutputTableExtensions();
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    if ( hasGeometry() )
      return p->supportedOutputVectorLayerExtensions();
    else
      return p->supportedOutputTableExtensions();
  }
  else
  {
    return QgsVectorFileWriter::supportedFormatExtensions();
  }
}

QgsProcessing::SourceType QgsProcessingParameterFeatureSink::dataType() const
{
  return mDataType;
}

bool QgsProcessingParameterFeatureSink::hasGeometry() const
{
  switch ( mDataType )
  {
    case QgsProcessing::TypeMapLayer:
    case QgsProcessing::TypeVectorAnyGeometry:
    case QgsProcessing::TypeVectorPoint:
    case QgsProcessing::TypeVectorLine:
    case QgsProcessing::TypeVectorPolygon:
      return true;

    case QgsProcessing::TypeRaster:
    case QgsProcessing::TypeFile:
    case QgsProcessing::TypeVector:
    case QgsProcessing::TypeMesh:
      return false;
  }
  return true;
}

void QgsProcessingParameterFeatureSink::setDataType( QgsProcessing::SourceType type )
{
  mDataType = type;
}

QVariantMap QgsProcessingParameterFeatureSink::toVariantMap() const
{
  QVariantMap map = QgsProcessingDestinationParameter::toVariantMap();
  map.insert( QStringLiteral( "data_type" ), mDataType );
  return map;
}

bool QgsProcessingParameterFeatureSink::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingDestinationParameter::fromVariantMap( map );
  mDataType = static_cast< QgsProcessing::SourceType >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  return true;
}

QString QgsProcessingParameterFeatureSink::generateTemporaryDestination() const
{
  if ( supportsNonFileBasedOutput() )
    return QStringLiteral( "memory:%1" ).arg( description() );
  else
    return QgsProcessingDestinationParameter::generateTemporaryDestination();
}

QgsProcessingParameterFeatureSink *QgsProcessingParameterFeatureSink::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QgsProcessing::SourceType type = QgsProcessing::TypeVectorAnyGeometry;
  QString def = definition;
  if ( def.startsWith( QLatin1String( "point" ), Qt::CaseInsensitive ) )
  {
    type = QgsProcessing::TypeVectorPoint;
    def = def.mid( 6 );
  }
  else if ( def.startsWith( QLatin1String( "line" ), Qt::CaseInsensitive ) )
  {
    type = QgsProcessing::TypeVectorLine;
    def = def.mid( 5 );
  }
  else if ( def.startsWith( QLatin1String( "polygon" ), Qt::CaseInsensitive ) )
  {
    type = QgsProcessing::TypeVectorPolygon;
    def = def.mid( 8 );
  }
  else if ( def.startsWith( QLatin1String( "table" ), Qt::CaseInsensitive ) )
  {
    type = QgsProcessing::TypeVector;
    def = def.mid( 6 );
  }

  return new QgsProcessingParameterFeatureSink( name, description, type, definition, isOptional );
}

QgsProcessingParameterRasterDestination::QgsProcessingParameterRasterDestination( const QString &name, const QString &description, const QVariant &defaultValue, bool optional, bool createByDefault )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional, createByDefault )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterRasterDestination::clone() const
{
  return new QgsProcessingParameterRasterDestination( *this );
}

bool QgsProcessingParameterRasterDestination::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  QVariant var = input;
  if ( !var.isValid() )
    return mFlags & FlagOptional;

  if ( var.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterRasterDestination::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == QgsProperty::StaticProperty )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( fromVar.sink.asExpression() );
    }
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QgsProcessingOutputDefinition *QgsProcessingParameterRasterDestination::toOutputDefinition() const
{
  return new QgsProcessingOutputRasterLayer( name(), description() );
}

QString QgsProcessingParameterRasterDestination::defaultFileExtension() const
{
  if ( originalProvider() )
  {
    return originalProvider()->defaultRasterFileExtension();
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->defaultRasterFileExtension();
  }
  else
  {
    QgsSettings settings;
    return settings.value( QStringLiteral( "Processing/DefaultOutputRasterLayerExt" ), QStringLiteral( "tif" ), QgsSettings::Core ).toString();
  }
}

QStringList QgsProcessingParameterRasterDestination::supportedOutputRasterLayerExtensions() const
{
  if ( originalProvider() )
  {
    return originalProvider()->supportedOutputRasterLayerExtensions();
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->supportedOutputRasterLayerExtensions();
  }
  else
  {
    return QgsRasterFileWriter::supportedFormatExtensions();
  }
}

QgsProcessingParameterRasterDestination *QgsProcessingParameterRasterDestination::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterRasterDestination( name, description, definition.isEmpty() ? QVariant() : definition, isOptional );
}


QgsProcessingParameterFileDestination::QgsProcessingParameterFileDestination( const QString &name, const QString &description, const QString &fileFilter, const QVariant &defaultValue, bool optional, bool createByDefault )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional, createByDefault )
  , mFileFilter( fileFilter.isEmpty() ? QObject::tr( "All files (*.*)" ) : fileFilter )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterFileDestination::clone() const
{
  return new QgsProcessingParameterFileDestination( *this );
}

bool QgsProcessingParameterFileDestination::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  QVariant var = input;
  if ( !var.isValid() )
    return mFlags & FlagOptional;

  if ( var.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  // possible enhancement - check that value is compatible with file filter?

  return true;
}

QString QgsProcessingParameterFileDestination::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == QgsProperty::StaticProperty )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( fromVar.sink.asExpression() );
    }
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QgsProcessingOutputDefinition *QgsProcessingParameterFileDestination::toOutputDefinition() const
{
  if ( !mFileFilter.isEmpty() && mFileFilter.contains( QStringLiteral( "htm" ), Qt::CaseInsensitive ) )
  {
    return new QgsProcessingOutputHtml( name(), description() );
  }
  else
  {
    return new QgsProcessingOutputFile( name(), description() );
  }
}

QString QgsProcessingParameterFileDestination::defaultFileExtension() const
{
  if ( mFileFilter.isEmpty() || mFileFilter == QObject::tr( "All files (*.*)" ) )
    return QStringLiteral( "file" );

  // get first extension from filter
  QRegularExpression rx( QStringLiteral( ".*?\\(\\*\\.([a-zA-Z0-9._]+).*" ) );
  QRegularExpressionMatch match = rx.match( mFileFilter );
  if ( !match.hasMatch() )
    return QStringLiteral( "file" );

  return match.captured( 1 );
}

QString QgsProcessingParameterFileDestination::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterFileDestination('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      code += QStringLiteral( ", fileFilter=%1" ).arg( QgsProcessingUtils::stringToPythonLiteral( mFileFilter ) );

      code += QStringLiteral( ", createByDefault=%1" ).arg( createByDefault() ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterFileDestination::fileFilter() const
{
  return mFileFilter;
}

void QgsProcessingParameterFileDestination::setFileFilter( const QString &fileFilter )
{
  mFileFilter = fileFilter;
}

QVariantMap QgsProcessingParameterFileDestination::toVariantMap() const
{
  QVariantMap map = QgsProcessingDestinationParameter::toVariantMap();
  map.insert( QStringLiteral( "file_filter" ), mFileFilter );
  return map;
}

bool QgsProcessingParameterFileDestination::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingDestinationParameter::fromVariantMap( map );
  mFileFilter = map.value( QStringLiteral( "file_filter" ) ).toString();
  return true;

}

QgsProcessingParameterFileDestination *QgsProcessingParameterFileDestination::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterFileDestination( name, description, QString(), definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterFolderDestination::QgsProcessingParameterFolderDestination( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional )
{}

QgsProcessingParameterDefinition *QgsProcessingParameterFolderDestination::clone() const
{
  return new QgsProcessingParameterFolderDestination( *this );
}

bool QgsProcessingParameterFolderDestination::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  QVariant var = input;
  if ( !var.isValid() )
    return mFlags & FlagOptional;

  if ( var.canConvert<QgsProperty>() )
  {
    QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QgsProcessingOutputDefinition *QgsProcessingParameterFolderDestination::toOutputDefinition() const
{
  return new QgsProcessingOutputFolder( name(), description() );
}

QString QgsProcessingParameterFolderDestination::defaultFileExtension() const
{
  return QString();
}

QgsProcessingParameterFolderDestination *QgsProcessingParameterFolderDestination::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterFolderDestination( name, description, definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingDestinationParameter::QgsProcessingDestinationParameter( const QString &name, const QString &description, const QVariant &defaultValue, bool optional, bool createByDefault )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mCreateByDefault( createByDefault )
{

}

QVariantMap QgsProcessingDestinationParameter::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "supports_non_file_outputs" ), mSupportsNonFileBasedOutputs );
  map.insert( QStringLiteral( "create_by_default" ), mCreateByDefault );
  return map;
}

bool QgsProcessingDestinationParameter::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mSupportsNonFileBasedOutputs = map.value( QStringLiteral( "supports_non_file_outputs" ) ).toBool();
  mCreateByDefault = map.value( QStringLiteral( "create_by_default" ), QStringLiteral( "1" ) ).toBool();
  return true;
}

QString QgsProcessingDestinationParameter::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      // base class method is probably not much use
      if ( QgsProcessingParameterType *t = QgsApplication::processingRegistry()->parameterType( type() ) )
      {
        QString code = t->className() + QStringLiteral( "('%1', '%2'" ).arg( name(), description() );
        if ( mFlags & FlagOptional )
          code += QStringLiteral( ", optional=True" );

        code += QStringLiteral( ", createByDefault=%1" ).arg( mCreateByDefault ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

        QgsProcessingContext c;
        code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
        return code;
      }
      break;
    }
  }
  // oh well, we tried
  return QString();
}

QString QgsProcessingDestinationParameter::generateTemporaryDestination() const
{
  if ( defaultFileExtension().isEmpty() )
  {
    return QgsProcessingUtils::generateTempFilename( name() );
  }
  else
  {
    return QgsProcessingUtils::generateTempFilename( name() + '.' + defaultFileExtension() );
  }
}

bool QgsProcessingDestinationParameter::createByDefault() const
{
  return mCreateByDefault;
}

void QgsProcessingDestinationParameter::setCreateByDefault( bool createByDefault )
{
  mCreateByDefault = createByDefault;
}

QgsProcessingParameterVectorDestination::QgsProcessingParameterVectorDestination( const QString &name, const QString &description, QgsProcessing::SourceType type, const QVariant &defaultValue, bool optional, bool createByDefault )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional, createByDefault )
  , mDataType( type )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterVectorDestination::clone() const
{
  return new QgsProcessingParameterVectorDestination( *this );
}

bool QgsProcessingParameterVectorDestination::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  QVariant var = input;
  if ( !var.isValid() )
    return mFlags & FlagOptional;

  if ( var.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterVectorDestination::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == QgsProperty::StaticProperty )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( fromVar.sink.asExpression() );
    }
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterVectorDestination::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "vectorDestination " );

  switch ( mDataType )
  {
    case QgsProcessing::TypeVectorPoint:
      code += QStringLiteral( "point " );
      break;

    case QgsProcessing::TypeVectorLine:
      code += QStringLiteral( "line " );
      break;

    case QgsProcessing::TypeVectorPolygon:
      code += QStringLiteral( "polygon " );
      break;

    default:
      break;
  }

  code += mDefault.toString();
  return code.trimmed();
}

QgsProcessingOutputDefinition *QgsProcessingParameterVectorDestination::toOutputDefinition() const
{
  return new QgsProcessingOutputVectorLayer( name(), description(), mDataType );
}

QString QgsProcessingParameterVectorDestination::defaultFileExtension() const
{
  if ( originalProvider() )
  {
    return originalProvider()->defaultVectorFileExtension( hasGeometry() );
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->defaultVectorFileExtension( hasGeometry() );
  }
  else
  {
    QgsSettings settings;
    if ( hasGeometry() )
    {
      return settings.value( QStringLiteral( "Processing/DefaultOutputVectorLayerExt" ), QStringLiteral( "shp" ), QgsSettings::Core ).toString();
    }
    else
    {
      return QStringLiteral( "dbf" );
    }
  }
}

QString QgsProcessingParameterVectorDestination::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterVectorDestination('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      code += QStringLiteral( ", type=QgsProcessing.%1" ).arg( QgsProcessing::sourceTypeToString( mDataType ) );

      code += QStringLiteral( ", createByDefault=%1" ).arg( createByDefault() ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterVectorDestination::supportedOutputVectorLayerExtensions() const
{
  if ( originalProvider() )
  {
    if ( hasGeometry() )
      return originalProvider()->supportedOutputVectorLayerExtensions();
    else
      return originalProvider()->supportedOutputTableExtensions();
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    if ( hasGeometry() )
      return p->supportedOutputVectorLayerExtensions();
    else
      return p->supportedOutputTableExtensions();
  }
  else
  {
    return QgsVectorFileWriter::supportedFormatExtensions();
  }
}

QgsProcessing::SourceType QgsProcessingParameterVectorDestination::dataType() const
{
  return mDataType;
}

bool QgsProcessingParameterVectorDestination::hasGeometry() const
{
  switch ( mDataType )
  {
    case QgsProcessing::TypeMapLayer:
    case QgsProcessing::TypeVectorAnyGeometry:
    case QgsProcessing::TypeVectorPoint:
    case QgsProcessing::TypeVectorLine:
    case QgsProcessing::TypeVectorPolygon:
      return true;

    case QgsProcessing::TypeRaster:
    case QgsProcessing::TypeFile:
    case QgsProcessing::TypeVector:
    case QgsProcessing::TypeMesh:
      return false;
  }
  return true;
}

void QgsProcessingParameterVectorDestination::setDataType( QgsProcessing::SourceType type )
{
  mDataType = type;
}

QVariantMap QgsProcessingParameterVectorDestination::toVariantMap() const
{
  QVariantMap map = QgsProcessingDestinationParameter::toVariantMap();
  map.insert( QStringLiteral( "data_type" ), mDataType );
  return map;
}

bool QgsProcessingParameterVectorDestination::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingDestinationParameter::fromVariantMap( map );
  mDataType = static_cast< QgsProcessing::SourceType >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  return true;
}

QgsProcessingParameterVectorDestination *QgsProcessingParameterVectorDestination::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QgsProcessing::SourceType type = QgsProcessing::TypeVectorAnyGeometry;
  QString def = definition;
  if ( def.startsWith( QLatin1String( "point" ), Qt::CaseInsensitive ) )
  {
    type = QgsProcessing::TypeVectorPoint;
    def = def.mid( 6 );
  }
  else if ( def.startsWith( QLatin1String( "line" ), Qt::CaseInsensitive ) )
  {
    type = QgsProcessing::TypeVectorLine;
    def = def.mid( 5 );
  }
  else if ( def.startsWith( QLatin1String( "polygon" ), Qt::CaseInsensitive ) )
  {
    type = QgsProcessing::TypeVectorPolygon;
    def = def.mid( 8 );
  }

  return new QgsProcessingParameterVectorDestination( name, description, type, definition, isOptional );
}

QgsProcessingParameterBand::QgsProcessingParameterBand( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, bool optional, bool allowMultiple )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameterName( parentLayerParameterName )
  , mAllowMultiple( allowMultiple )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterBand::clone() const
{
  return new QgsProcessingParameterBand( *this );
}

bool QgsProcessingParameterBand::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() == QVariant::List || input.type() == QVariant::StringList )
  {
    if ( !mAllowMultiple )
      return false;

    if ( input.toList().isEmpty() && !( mFlags & FlagOptional ) )
      return false;
  }
  else
  {
    bool ok = false;
    double res = input.toInt( &ok );
    Q_UNUSED( res );
    if ( !ok )
      return mFlags & FlagOptional;
  }
  return true;
}

bool QgsProcessingParameterBand::allowMultiple() const
{
  return mAllowMultiple;
}

void QgsProcessingParameterBand::setAllowMultiple( bool allowMultiple )
{
  mAllowMultiple = allowMultiple;
}

QString QgsProcessingParameterBand::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.type() == QVariant::List )
  {
    QStringList parts;
    QVariantList values = value.toList();
    for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
    {
      parts << QString::number( static_cast< int >( it->toDouble() ) );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else if ( value.type() == QVariant::StringList )
  {
    QStringList parts;
    QStringList values = value.toStringList();
    for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
    {
      parts << QString::number( static_cast< int >( it->toDouble() ) );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }

  return value.toString();
}

QString QgsProcessingParameterBand::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "band " );

  if ( mAllowMultiple )
    code += QStringLiteral( "multiple " );

  code += mParentLayerParameterName + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QStringList QgsProcessingParameterBand::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentLayerParameterName.isEmpty() )
    depends << mParentLayerParameterName;
  return depends;
}

QString QgsProcessingParameterBand::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterBand('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      code += QStringLiteral( ", parentLayerParameterName='%1'" ).arg( mParentLayerParameterName );
      code += QStringLiteral( ", allowMultiple=%1" ).arg( mAllowMultiple ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterBand::parentLayerParameterName() const
{
  return mParentLayerParameterName;
}

void QgsProcessingParameterBand::setParentLayerParameterName( const QString &parentLayerParameterName )
{
  mParentLayerParameterName = parentLayerParameterName;
}

QVariantMap QgsProcessingParameterBand::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "parent_layer" ), mParentLayerParameterName );
  map.insert( QStringLiteral( "allow_multiple" ), mAllowMultiple );
  return map;
}

bool QgsProcessingParameterBand::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( QStringLiteral( "parent_layer" ) ).toString();
  mAllowMultiple = map.value( QStringLiteral( "allow_multiple" ) ).toBool();
  return true;
}

QgsProcessingParameterBand *QgsProcessingParameterBand::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString parent;
  QString def = definition;
  bool allowMultiple = false;

  if ( def.startsWith( QLatin1String( "multiple" ), Qt::CaseInsensitive ) )
  {
    allowMultiple = true;
    def = def.mid( 8 ).trimmed();
  }

  QRegularExpression re( QStringLiteral( "(.*?)\\s+(.*)$" ) );
  QRegularExpressionMatch m = re.match( def );
  if ( m.hasMatch() )
  {
    parent = m.captured( 1 ).trimmed();
    def = m.captured( 2 );
  }
  else
  {
    parent = def;
    def.clear();
  }

  return new QgsProcessingParameterBand( name, description, def.isEmpty() ? QVariant() : def, parent, isOptional, allowMultiple );
}

//
// QgsProcessingParameterDistance
//

QgsProcessingParameterDistance::QgsProcessingParameterDistance( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentParameterName, bool optional, double minValue, double maxValue )
  : QgsProcessingParameterNumber( name, description, Double, defaultValue, optional, minValue, maxValue )
  , mParentParameterName( parentParameterName )
{

}

QgsProcessingParameterDistance *QgsProcessingParameterDistance::clone() const
{
  return new QgsProcessingParameterDistance( *this );
}

QString QgsProcessingParameterDistance::type() const
{
  return typeName();
}

QStringList QgsProcessingParameterDistance::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentParameterName.isEmpty() )
    depends << mParentParameterName;
  return depends;
}

QString QgsProcessingParameterDistance::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterDistance('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      code += QStringLiteral( ", parentParameterName='%1'" ).arg( mParentParameterName );

      if ( minimum() != std::numeric_limits<double>::lowest() + 1 )
        code += QStringLiteral( ", minValue=%1" ).arg( minimum() );
      if ( maximum() != std::numeric_limits<double>::max() )
        code += QStringLiteral( ", maxValue=%1" ).arg( maximum() );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterDistance::parentParameterName() const
{
  return mParentParameterName;
}

void QgsProcessingParameterDistance::setParentParameterName( const QString &parentParameterName )
{
  mParentParameterName = parentParameterName;
}

QVariantMap QgsProcessingParameterDistance::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterNumber::toVariantMap();
  map.insert( QStringLiteral( "parent" ), mParentParameterName );
  map.insert( QStringLiteral( "default_unit" ), static_cast< int >( mDefaultUnit ) );
  return map;
}

bool QgsProcessingParameterDistance::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterNumber::fromVariantMap( map );
  mParentParameterName = map.value( QStringLiteral( "parent" ) ).toString();
  mDefaultUnit = static_cast< QgsUnitTypes::DistanceUnit>( map.value( QStringLiteral( "default_unit" ), QgsUnitTypes::DistanceUnknownUnit ).toInt() );
  return true;
}


//
// QgsProcessingParameterScale
//

QgsProcessingParameterScale::QgsProcessingParameterScale( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterNumber( name, description, Double, defaultValue, optional )
{

}

QgsProcessingParameterScale *QgsProcessingParameterScale::clone() const
{
  return new QgsProcessingParameterScale( *this );
}

QString QgsProcessingParameterScale::type() const
{
  return typeName();
}

QString QgsProcessingParameterScale::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterScale('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QgsProcessingParameterScale *QgsProcessingParameterScale::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterScale( name, description, definition.isEmpty() ? QVariant()
                                          : ( definition.toLower().trimmed() == QStringLiteral( "none" ) ? QVariant() : definition ), isOptional );
}


//
// QgsProcessingParameterLayout
//

QgsProcessingParameterLayout::QgsProcessingParameterLayout( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{}

QgsProcessingParameterDefinition *QgsProcessingParameterLayout::clone() const
{
  return new QgsProcessingParameterLayout( *this );
}

QString QgsProcessingParameterLayout::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() || value.isNull() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterLayout::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "layout " );

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterLayout::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterLayout('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QgsProcessingParameterLayout *QgsProcessingParameterLayout::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString def = definition;

  if ( def.startsWith( '"' ) || def.startsWith( '\'' ) )
    def = def.mid( 1 );
  if ( def.endsWith( '"' ) || def.endsWith( '\'' ) )
    def.chop( 1 );

  QVariant defaultValue = def;
  if ( def == QStringLiteral( "None" ) )
    defaultValue = QVariant();

  return new QgsProcessingParameterLayout( name, description, defaultValue, isOptional );
}


//
// QString mParentLayerParameterName;
//

QgsProcessingParameterLayoutItem::QgsProcessingParameterLayoutItem( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayoutParameterName, int itemType, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayoutParameterName( parentLayoutParameterName )
  , mItemType( itemType )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterLayoutItem::clone() const
{
  return new QgsProcessingParameterLayoutItem( *this );
}

QString QgsProcessingParameterLayoutItem::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() || value.isNull() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterLayoutItem::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QStringLiteral( "optional " );
  code += QStringLiteral( "layoutitem " );
  if ( mItemType >= 0 )
    code += QString::number( mItemType ) + ' ';

  code += mParentLayoutParameterName + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterLayoutItem::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterLayoutItem('%1', '%2'" ).arg( name(), description() );
      if ( mFlags & FlagOptional )
        code += QStringLiteral( ", optional=True" );

      if ( mItemType >= 0 )
        code += QStringLiteral( ", itemType=%1" ).arg( mItemType );

      code += QStringLiteral( ", parentLayoutParameterName='%1'" ).arg( mParentLayoutParameterName );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterLayoutItem::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "parent_layout" ), mParentLayoutParameterName );
  map.insert( QStringLiteral( "item_type" ), mItemType );
  return map;
}

bool QgsProcessingParameterLayoutItem::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayoutParameterName = map.value( QStringLiteral( "parent_layout" ) ).toString();
  mItemType = map.value( QStringLiteral( "item_type" ) ).toInt();
  return true;
}

QStringList QgsProcessingParameterLayoutItem::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentLayoutParameterName.isEmpty() )
    depends << mParentLayoutParameterName;
  return depends;
}

QgsProcessingParameterLayoutItem *QgsProcessingParameterLayoutItem::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString parent;
  QString def = definition;
  int itemType = -1;
  QRegularExpression re( QStringLiteral( "(\\d+)?\\s*(.*?)\\s+(.*)$" ) );
  QRegularExpressionMatch m = re.match( def );
  if ( m.hasMatch() )
  {
    itemType = m.captured( 1 ).trimmed().isEmpty() ? -1 : m.captured( 1 ).trimmed().toInt();
    parent = m.captured( 2 ).trimmed().isEmpty() ? m.captured( 3 ).trimmed() : m.captured( 2 ).trimmed();
    def = !m.captured( 2 ).trimmed().isEmpty() ? m.captured( 3 ) : QString();
  }
  else
  {
    parent = def;
    def.clear();
  }

  return new QgsProcessingParameterLayoutItem( name, description, def.isEmpty() ? QVariant() : def, parent, itemType, isOptional );
}

QString QgsProcessingParameterLayoutItem::parentLayoutParameterName() const
{
  return mParentLayoutParameterName;
}

void QgsProcessingParameterLayoutItem::setParentLayoutParameterName( const QString &name )
{
  mParentLayoutParameterName = name;
}

int QgsProcessingParameterLayoutItem::itemType() const
{
  return mItemType;
}

void QgsProcessingParameterLayoutItem::setItemType( int type )
{
  mItemType = type;
}
