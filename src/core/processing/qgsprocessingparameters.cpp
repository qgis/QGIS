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
#include "qgspointcloudlayer.h"
#include "qgsannotationlayer.h"
#include "qgsapplication.h"
#include "qgslayoutmanager.h"
#include "qgsprintlayout.h"
#include "qgssymbollayerutils.h"
#include "qgsfileutils.h"
#include "qgsproviderregistry.h"
#include <functional>
#include <QRegularExpression>


QVariant QgsProcessingFeatureSourceDefinition::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "source" ), source.toVariant() );
  map.insert( QStringLiteral( "selected_only" ), selectedFeaturesOnly );
  map.insert( QStringLiteral( "feature_limit" ), featureLimit );
  map.insert( QStringLiteral( "flags" ), static_cast< int >( flags ) );
  map.insert( QStringLiteral( "geometry_check" ), static_cast< int >( geometryCheck ) );
  return map;
}

bool QgsProcessingFeatureSourceDefinition::loadVariant( const QVariantMap &map )
{
  source.loadVariant( map.value( QStringLiteral( "source" ) ) );
  selectedFeaturesOnly = map.value( QStringLiteral( "selected_only" ), false ).toBool();
  featureLimit = map.value( QStringLiteral( "feature_limit" ), -1 ).toLongLong();
  flags = static_cast< Flags >( map.value( QStringLiteral( "flags" ), 0 ).toInt() );
  geometryCheck = static_cast< QgsFeatureRequest::InvalidGeometryCheck >( map.value( QStringLiteral( "geometry_check" ), QgsFeatureRequest::GeometryAbortOnInvalid ).toInt() );
  return true;
}


//
// QgsProcessingOutputLayerDefinition
//

void QgsProcessingOutputLayerDefinition::setRemappingDefinition( const QgsRemappingSinkDefinition &definition )
{
  mUseRemapping = true;
  mRemappingDefinition = definition;
}

QVariant QgsProcessingOutputLayerDefinition::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "sink" ), sink.toVariant() );
  map.insert( QStringLiteral( "create_options" ), createOptions );
  if ( mUseRemapping )
    map.insert( QStringLiteral( "remapping" ), QVariant::fromValue( mRemappingDefinition ) );
  return map;
}

bool QgsProcessingOutputLayerDefinition::loadVariant( const QVariantMap &map )
{
  sink.loadVariant( map.value( QStringLiteral( "sink" ) ) );
  createOptions = map.value( QStringLiteral( "create_options" ) ).toMap();
  if ( map.contains( QStringLiteral( "remapping" ) ) )
  {
    mUseRemapping = true;
    mRemappingDefinition = map.value( QStringLiteral( "remapping" ) ).value< QgsRemappingSinkDefinition >();
  }
  else
  {
    mUseRemapping = false;
  }
  return true;
}

bool QgsProcessingOutputLayerDefinition::operator==( const QgsProcessingOutputLayerDefinition &other ) const
{
  return sink == other.sink && destinationProject == other.destinationProject && destinationName == other.destinationName && createOptions == other.createOptions
         && mUseRemapping == other.mUseRemapping && mRemappingDefinition == other.mRemappingDefinition;
}

bool QgsProcessingOutputLayerDefinition::operator!=( const QgsProcessingOutputLayerDefinition &other ) const
{
  return !( *this == other );
}

bool QgsProcessingParameters::isDynamic( const QVariantMap &parameters, const QString &name )
{
  const QVariant val = parameters.value( name );
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

  const QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );

  if ( val.isValid() && !val.toString().isEmpty() )
  {
    const QgsExpression e( val.toString() );
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
  const double res = val.toDouble( &ok );
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
    const double round = std::round( dbl );
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
  const QVariant val = value;
  if ( val.isValid() )
  {
    if ( val.canConvert<QgsProperty>() )
      resultList << val.value< QgsProperty >().valueAsInt( context.expressionContext(), definition->defaultValue().toInt() );
    else if ( val.type() == QVariant::List )
    {
      const QVariantList list = val.toList();
      for ( auto it = list.constBegin(); it != list.constEnd(); ++it )
        resultList << it->toInt();
    }
    else
    {
      const QStringList parts = val.toString().split( ';' );
      for ( auto it = parts.constBegin(); it != parts.constEnd(); ++it )
        resultList << it->toInt();
    }
  }

  if ( resultList.isEmpty() )
  {
    // check default
    if ( definition->defaultValue().isValid() )
    {
      if ( definition->defaultValue().type() == QVariant::List )
      {
        const QVariantList list = definition->defaultValue().toList();
        for ( auto it = list.constBegin(); it != list.constEnd(); ++it )
          resultList << it->toInt();
      }
      else
      {
        const QStringList parts = definition->defaultValue().toString().split( ';' );
        for ( auto it = parts.constBegin(); it != parts.constEnd(); ++it )
          resultList << it->toInt();
      }
    }
  }

  return resultList;
}

QDateTime QgsProcessingParameters::parameterAsDateTime( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QDateTime();

  return parameterAsDateTime( definition, parameters.value( definition->name() ), context );
}

QDateTime QgsProcessingParameters::parameterAsDateTime( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return QDateTime();

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    val = val.value< QgsProperty >().value( context.expressionContext(), definition->defaultValue() );

  QDateTime d = val.toDateTime();
  if ( !d.isValid() && val.type() == QVariant::String )
  {
    d = QDateTime::fromString( val.toString() );
  }

  if ( !d.isValid() )
  {
    // fall back to default
    val = definition->defaultValue();
    d = val.toDateTime();
  }
  if ( !d.isValid() && val.type() == QVariant::String )
  {
    d = QDateTime::fromString( val.toString() );
  }

  return d;
}

QDate QgsProcessingParameters::parameterAsDate( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QDate();

  return parameterAsDate( definition, parameters.value( definition->name() ), context );
}

QDate QgsProcessingParameters::parameterAsDate( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return QDate();

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    val = val.value< QgsProperty >().value( context.expressionContext(), definition->defaultValue() );

  QDate d = val.toDate();
  if ( !d.isValid() && val.type() == QVariant::String )
  {
    d = QDate::fromString( val.toString() );
  }

  if ( !d.isValid() )
  {
    // fall back to default
    val = definition->defaultValue();
    d = val.toDate();
  }
  if ( !d.isValid() && val.type() == QVariant::String )
  {
    d = QDate::fromString( val.toString() );
  }

  return d;
}

QTime QgsProcessingParameters::parameterAsTime( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QTime();

  return parameterAsTime( definition, parameters.value( definition->name() ), context );
}

QTime QgsProcessingParameters::parameterAsTime( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return QTime();

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    val = val.value< QgsProperty >().value( context.expressionContext(), definition->defaultValue() );

  QTime d;

  if ( val.type() == QVariant::DateTime )
    d = val.toDateTime().time();
  else
    d = val.toTime();

  if ( !d.isValid() && val.type() == QVariant::String )
  {
    d = QTime::fromString( val.toString() );
  }

  if ( !d.isValid() )
  {
    // fall back to default
    val = definition->defaultValue();
    d = val.toTime();
  }
  if ( !d.isValid() && val.type() == QVariant::String )
  {
    d = QTime::fromString( val.toString() );
  }

  return d;
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

  const int val = parameterAsInt( definition, value, context );
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
  const QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    resultList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else if ( val.type() == QVariant::List )
  {
    const auto constToList = val.toList();
    for ( const QVariant &var : constToList )
      resultList << var;
  }
  else if ( val.type() == QVariant::String )
  {
    const auto constSplit = val.toString().split( ',' );
    for ( const QString &var : constSplit )
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
      const auto constToList = definition->defaultValue().toList();
      for ( const QVariant &var : constToList )
        resultList << var;
    }
    else if ( definition->defaultValue().type() == QVariant::String )
    {
      const auto constSplit = definition->defaultValue().toString().split( ',' );
      for ( const QString &var : constSplit )
        resultList << var;
    }
    else
      resultList << definition->defaultValue();
  }

  QList< int > result;
  const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( definition );
  const auto constResultList = resultList;
  for ( const QVariant &var : constResultList )
  {
    const int resInt = var.toInt();
    if ( !enumDef || resInt < enumDef->options().size() )
    {
      result << resInt;
    }
  }
  return result;
}

QString QgsProcessingParameters::parameterAsEnumString( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  return parameterAsEnumString( definition, parameters.value( definition->name() ), context );
}

QString QgsProcessingParameters::parameterAsEnumString( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  QString enumText = parameterAsString( definition, value, context );
  const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( definition );
  if ( enumText.isEmpty() || !enumDef->options().contains( enumText ) )
    enumText = definition->defaultValue().toString();

  return enumText;
}

QStringList QgsProcessingParameters::parameterAsEnumStrings( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  return parameterAsEnumStrings( definition, parameters.value( definition->name() ), context );
}

QStringList QgsProcessingParameters::parameterAsEnumStrings( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  const QVariant val = value;

  QStringList enumValues;

  std::function< void( const QVariant &var ) > processVariant;
  processVariant = [ &enumValues, &context, &definition, &processVariant ]( const QVariant & var )
  {
    if ( var.type() == QVariant::List )
    {
      const auto constToList = var.toList();
      for ( const QVariant &listVar : constToList )
      {
        processVariant( listVar );
      }
    }
    else if ( var.type() == QVariant::StringList )
    {
      const auto constToStringList = var.toStringList();
      for ( const QString &s : constToStringList )
      {
        processVariant( s );
      }
    }
    else if ( var.canConvert<QgsProperty>() )
      processVariant( var.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() ) );
    else
    {
      const QStringList parts = var.toString().split( ',' );
      for ( const QString &s : parts )
      {
        enumValues << s;
      }
    }
  };

  processVariant( val );

  const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( definition );
  // check that values are valid enum values. The resulting set will be empty
  // if all values are present in the enumDef->options(), otherwise it will contain
  // values which are invalid
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QSet<QString> subtraction = enumValues.toSet().subtract( enumDef->options().toSet() );
#else
  const QStringList options = enumDef->options();
  const QSet<QString> subtraction = QSet<QString>( enumValues.begin(), enumValues.end() ).subtract( QSet<QString>( options.begin(), options.end() ) );
#endif

  if ( enumValues.isEmpty() || !subtraction.isEmpty() )
  {
    enumValues.clear();
    processVariant( definition->defaultValue() );
  }

  return enumValues;
}

bool QgsProcessingParameters::parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return false;

  return parameterAsBool( definition, parameters.value( definition->name() ), context );
}

bool QgsProcessingParameters::parameterAsBoolean( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return false;

  return parameterAsBoolean( definition, parameters.value( definition->name() ), context );
}

bool QgsProcessingParameters::parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return false;

  const QVariant def = definition->defaultValue();

  const QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsBool( context.expressionContext(), def.toBool() );
  else if ( val.isValid() )
    return val.toBool();
  else
    return def.toBool();
}

bool QgsProcessingParameters::parameterAsBoolean( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  if ( !definition )
    return false;

  const QVariant def = definition->defaultValue();

  const QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsBool( context.expressionContext(), def.toBool() );
  else if ( val.isValid() )
    return val.toBool();
  else
    return def.toBool();
}

QgsFeatureSink *QgsProcessingParameters::parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsFields &fields,
    QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
    QgsProcessingContext &context, QString &destinationIdentifier, QgsFeatureSink::SinkFlags sinkFlags,
    const QVariantMap &createOptions, const QStringList &datasourceOptions, const QStringList &layerOptions )
{
  QVariant val;
  if ( definition )
  {
    val = parameters.value( definition->name() );
  }

  return parameterAsSink( definition, val, fields, geometryType, crs, context, destinationIdentifier, sinkFlags, createOptions, datasourceOptions, layerOptions );
}

QgsFeatureSink *QgsProcessingParameters::parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs, QgsProcessingContext &context, QString &destinationIdentifier, QgsFeatureSink::SinkFlags sinkFlags, const QVariantMap &createOptions, const QStringList &datasourceOptions, const QStringList &layerOptions )
{
  QVariantMap options = createOptions;
  QVariant val = value;

  QgsProject *destinationProject = nullptr;
  QString destName;
  QgsRemappingSinkDefinition remapDefinition;
  bool useRemapDefinition = false;
  if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    destinationProject = fromVar.destinationProject;
    options = fromVar.createOptions;

    val = fromVar.sink;
    destName = fromVar.destinationName;
    if ( fromVar.useRemapping() )
    {
      useRemapDefinition = true;
      remapDefinition = fromVar.remappingDefinition();
    }
  }

  QString dest;
  if ( definition && val.canConvert<QgsProperty>() )
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
    if ( !definition )
    {
      throw QgsProcessingException( QObject::tr( "No parameter definition for the sink" ) );
    }
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

  std::unique_ptr< QgsFeatureSink > sink( QgsProcessingUtils::createFeatureSink( dest, context, fields, geometryType, crs, options, datasourceOptions, layerOptions, sinkFlags, useRemapDefinition ? &remapDefinition : nullptr ) );
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
    context.addLayerToLoadOnCompletion( destinationIdentifier, QgsProcessingContext::LayerDetails( destName, destinationProject, outputName, QgsProcessingUtils::LayerHint::Vector ) );
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

QString parameterAsCompatibleSourceLayerPathInternal( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingFeedback *feedback, QString *layerName )
{
  if ( !definition )
    return QString();

  QVariant val = parameters.value( definition->name() );

  bool selectedFeaturesOnly = false;
  long long featureLimit = -1;
  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    selectedFeaturesOnly = fromVar.selectedFeaturesOnly;
    featureLimit = fromVar.featureLimit;
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
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

      vl = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( layerRef, context, true, QgsProcessingUtils::LayerHint::Vector ) );
    }
  }

  if ( !vl )
    return QString();

  if ( layerName )
    return QgsProcessingUtils::convertToCompatibleFormatAndLayerName( vl, selectedFeaturesOnly, definition->name(),
           compatibleFormats, preferredFormat, context, feedback, *layerName, featureLimit );
  else
    return QgsProcessingUtils::convertToCompatibleFormat( vl, selectedFeaturesOnly, definition->name(),
           compatibleFormats, preferredFormat, context, feedback, featureLimit );
}

QString QgsProcessingParameters::parameterAsCompatibleSourceLayerPath( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingFeedback *feedback )
{
  return parameterAsCompatibleSourceLayerPathInternal( definition, parameters, context, compatibleFormats, preferredFormat, feedback, nullptr );
}

QString QgsProcessingParameters::parameterAsCompatibleSourceLayerPathAndLayerName( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, const QStringList &compatibleFormats, const QString &preferredFormat, QgsProcessingFeedback *feedback, QString *layerName )
{
  QString *destLayer = layerName;
  QString tmp;
  if ( destLayer )
    destLayer->clear();
  else
    destLayer = &tmp;

  return parameterAsCompatibleSourceLayerPathInternal( definition, parameters, context, compatibleFormats, preferredFormat, feedback, destLayer );
}

QgsMapLayer *QgsProcessingParameters::parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingUtils::LayerHint layerHint )
{
  if ( !definition )
    return nullptr;

  return parameterAsLayer( definition, parameters.value( definition->name() ), context, layerHint );
}

QgsMapLayer *QgsProcessingParameters::parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, QgsProcessingUtils::LayerHint layerHint )
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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
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

  return QgsProcessingUtils::mapLayerFromString( layerRef, context, true, layerHint );
}

QgsRasterLayer *QgsProcessingParameters::parameterAsRasterLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  return qobject_cast< QgsRasterLayer *>( parameterAsLayer( definition, parameters, context, QgsProcessingUtils::LayerHint::Raster ) );
}

QgsRasterLayer *QgsProcessingParameters::parameterAsRasterLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  return qobject_cast< QgsRasterLayer *>( parameterAsLayer( definition, value, context, QgsProcessingUtils::LayerHint::Raster ) );
}

QgsMeshLayer *QgsProcessingParameters::parameterAsMeshLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  return qobject_cast< QgsMeshLayer *>( parameterAsLayer( definition, parameters, context, QgsProcessingUtils::LayerHint::Mesh ) );
}

QgsMeshLayer *QgsProcessingParameters::parameterAsMeshLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  return qobject_cast< QgsMeshLayer *>( parameterAsLayer( definition, value, context, QgsProcessingUtils::LayerHint::Mesh ) );
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
  QString destName;
  if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    destinationProject = fromVar.destinationProject;
    val = fromVar.sink;
    destName = fromVar.destinationName;
  }

  QString dest;
  if ( definition && val.canConvert<QgsProperty>() )
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

    QgsProcessingUtils::LayerHint layerTypeHint = QgsProcessingUtils::LayerHint::UnknownType;
    if ( definition && definition->type() == QgsProcessingParameterVectorDestination::typeName() )
      layerTypeHint = QgsProcessingUtils::LayerHint::Vector;
    else if ( definition && definition->type() == QgsProcessingParameterRasterDestination::typeName() )
      layerTypeHint = QgsProcessingUtils::LayerHint::Raster;
    else if ( definition && definition->type() == QgsProcessingParameterPointCloudDestination::typeName() )
      layerTypeHint = QgsProcessingUtils::LayerHint::PointCloud;

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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  QString dest;
  if ( definition && val.canConvert<QgsProperty>() )
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
  return dest;
}

QgsVectorLayer *QgsProcessingParameters::parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  return qobject_cast< QgsVectorLayer *>( parameterAsLayer( definition, parameters, context, QgsProcessingUtils::LayerHint::Vector ) );
}

QgsVectorLayer *QgsProcessingParameters::parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  return qobject_cast< QgsVectorLayer *>( parameterAsLayer( definition, value, context, QgsProcessingUtils::LayerHint::Vector ) );
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

  return QgsProcessingUtils::variantToCrs( value, context, definition->defaultValue() );
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
  if ( val.canConvert< QgsGeometry >() )
  {
    const QgsGeometry geom = val.value<QgsGeometry>();
    if ( !geom.isNull() )
      return geom.boundingBox();
  }
  if ( val.canConvert< QgsReferencedRectangle >() )
  {
    const QgsReferencedRectangle rr = val.value<QgsReferencedRectangle>();
    if ( crs.isValid() && rr.crs().isValid() && crs != rr.crs() )
    {
      QgsCoordinateTransform ct( rr.crs(), crs, context.project() );
      ct.setBallparkTransformsAreAppropriate( true );
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
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
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

  const QRegularExpression rx( QStringLiteral( "^(.*?)\\s*,\\s*(.*?),\\s*(.*?),\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$" ) );
  const QRegularExpressionMatch match = rx.match( rectText );
  if ( match.hasMatch() )
  {
    bool xMinOk = false;
    const double xMin = match.captured( 1 ).toDouble( &xMinOk );
    bool xMaxOk = false;
    const double xMax = match.captured( 2 ).toDouble( &xMaxOk );
    bool yMinOk = false;
    const double yMin = match.captured( 3 ).toDouble( &yMinOk );
    bool yMaxOk = false;
    const double yMax = match.captured( 4 ).toDouble( &yMaxOk );
    if ( xMinOk && xMaxOk && yMinOk && yMaxOk )
    {
      const QgsRectangle rect( xMin, yMin, xMax, yMax );
      const QgsCoordinateReferenceSystem rectCrs( match.captured( 5 ) );
      if ( crs.isValid() && rectCrs.isValid() && crs != rectCrs )
      {
        QgsCoordinateTransform ct( rectCrs, crs, context.project() );
        ct.setBallparkTransformsAreAppropriate( true );
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
    const QgsRectangle rect = layer->extent();
    if ( crs.isValid() && layer->crs().isValid() && crs != layer->crs() )
    {
      QgsCoordinateTransform ct( layer->crs(), crs, context.project() );
      ct.setBallparkTransformsAreAppropriate( true );
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
    const QgsReferencedRectangle rr = val.value<QgsReferencedRectangle>();
    QgsGeometry g = QgsGeometry::fromRect( rr );
    if ( crs.isValid() && rr.crs().isValid() && crs != rr.crs() )
    {
      g = g.densifyByCount( 20 );
      const QgsCoordinateTransform ct( rr.crs(), crs, context.project() );
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
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
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
    const QRegularExpression rx( QStringLiteral( "^(.*?)\\s*,\\s*(.*?),\\s*(.*?),\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$" ) );
    const QRegularExpressionMatch match = rx.match( rectText );
    if ( match.hasMatch() )
    {
      bool xMinOk = false;
      const double xMin = match.captured( 1 ).toDouble( &xMinOk );
      bool xMaxOk = false;
      const double xMax = match.captured( 2 ).toDouble( &xMaxOk );
      bool yMinOk = false;
      const double yMin = match.captured( 3 ).toDouble( &yMinOk );
      bool yMaxOk = false;
      const double yMax = match.captured( 4 ).toDouble( &yMaxOk );
      if ( xMinOk && xMaxOk && yMinOk && yMaxOk )
      {
        const QgsRectangle rect( xMin, yMin, xMax, yMax );
        const QgsCoordinateReferenceSystem rectCrs( match.captured( 5 ) );
        QgsGeometry g = QgsGeometry::fromRect( rect );
        if ( crs.isValid() && rectCrs.isValid() && crs != rectCrs )
        {
          g = g.densifyByCount( 20 );
          const QgsCoordinateTransform ct( rectCrs, crs, context.project() );
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
    const QgsRectangle rect = layer->extent();
    QgsGeometry g = QgsGeometry::fromRect( rect );
    if ( crs.isValid() && layer->crs().isValid() && crs != layer->crs() )
    {
      g = g.densifyByCount( 20 );
      const QgsCoordinateTransform ct( layer->crs(), crs, context.project() );
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
  const QVariant val = parameters.value( definition->name() );
  return parameterAsExtentCrs( definition, val, context );
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsExtentCrs( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  QVariant val = value;
  if ( val.canConvert< QgsReferencedRectangle >() )
  {
    const QgsReferencedRectangle rr = val.value<QgsReferencedRectangle>();
    if ( rr.crs().isValid() )
    {
      return rr.crs();
    }
  }

  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
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

  const QRegularExpression rx( QStringLiteral( "^(.*?)\\s*,\\s*(.*?),\\s*(.*?),\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$" ) );

  const QRegularExpressionMatch match = rx.match( valueAsString );
  if ( match.hasMatch() )
  {
    const QgsCoordinateReferenceSystem crs( match.captured( 5 ) );
    if ( crs.isValid() )
      return crs;
  }

  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
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

  if ( auto *lProject = context.project() )
    return lProject->crs();
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

  const QVariant val = value;
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
    const QgsReferencedPointXY rp = val.value<QgsReferencedPointXY>();
    if ( crs.isValid() && rp.crs().isValid() && crs != rp.crs() )
    {
      const QgsCoordinateTransform ct( rp.crs(), crs, context.project() );
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

  const QRegularExpression rx( QStringLiteral( "^\\s*\\(?\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*\\)?\\s*$" ) );

  const QString valueAsString = parameterAsString( definition, value, context );
  const QRegularExpressionMatch match = rx.match( valueAsString );
  if ( match.hasMatch() )
  {
    bool xOk = false;
    const double x = match.captured( 1 ).toDouble( &xOk );
    bool yOk = false;
    const double y = match.captured( 2 ).toDouble( &yOk );

    if ( xOk && yOk )
    {
      const QgsPointXY pt( x, y );

      const QgsCoordinateReferenceSystem pointCrs( match.captured( 3 ) );
      if ( crs.isValid() && pointCrs.isValid() && crs != pointCrs )
      {
        const QgsCoordinateTransform ct( pointCrs, crs, context.project() );
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
  const QVariant val = parameters.value( definition->name() );
  return parameterAsPointCrs( definition, val, context );
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsPointCrs( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( value.canConvert< QgsReferencedPointXY >() )
  {
    const QgsReferencedPointXY rr = value.value<QgsReferencedPointXY>();
    if ( rr.crs().isValid() )
    {
      return rr.crs();
    }
  }

  const QRegularExpression rx( QStringLiteral( "^\\s*\\(?\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*\\)?\\s*$" ) );

  const QString valueAsString = parameterAsString( definition, value, context );
  const QRegularExpressionMatch match = rx.match( valueAsString );
  if ( match.hasMatch() )
  {
    const QgsCoordinateReferenceSystem crs( match.captured( 3 ) );
    if ( crs.isValid() )
      return crs;
  }

  if ( auto *lProject = context.project() )
    return lProject->crs();
  else
    return QgsCoordinateReferenceSystem();
}

QgsGeometry QgsProcessingParameters::parameterAsGeometry( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs )
{
  if ( !definition )
    return QgsGeometry();

  return parameterAsGeometry( definition, parameters.value( definition->name() ), context, crs );
}

QgsGeometry QgsProcessingParameters::parameterAsGeometry( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, const QgsCoordinateReferenceSystem &crs )
{
  if ( !definition )
    return QgsGeometry();

  const QVariant val = value;
  if ( val.canConvert< QgsGeometry >() )
  {
    return val.value<QgsGeometry>();
  }

  if ( val.canConvert< QgsPointXY >() )
  {
    return QgsGeometry::fromPointXY( val.value<QgsPointXY>() );
  }

  if ( val.canConvert< QgsRectangle >() )
  {
    return QgsGeometry::fromRect( val.value<QgsRectangle>() );
  }

  if ( val.canConvert< QgsReferencedPointXY >() )
  {
    const QgsReferencedPointXY rp = val.value<QgsReferencedPointXY>();
    if ( crs.isValid() && rp.crs().isValid() && crs != rp.crs() )
    {
      const QgsCoordinateTransform ct( rp.crs(), crs, context.project() );
      try
      {
        return QgsGeometry::fromPointXY( ct.transform( rp ) );
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error transforming point geometry" ) );
      }
    }
    return QgsGeometry::fromPointXY( rp );
  }

  if ( val.canConvert< QgsReferencedRectangle >() )
  {
    const QgsReferencedRectangle rr = val.value<QgsReferencedRectangle>();
    QgsGeometry g = QgsGeometry::fromRect( rr );
    if ( crs.isValid() && rr.crs().isValid() && crs != rr.crs() )
    {
      g = g.densifyByCount( 20 );
      const QgsCoordinateTransform ct( rr.crs(), crs, context.project() );
      try
      {
        g.transform( ct );
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error transforming rectangle geometry" ) );
      }
    }
    return g;
  }

  if ( val.canConvert< QgsReferencedGeometry >() )
  {
    QgsReferencedGeometry rg = val.value<QgsReferencedGeometry>();
    if ( crs.isValid() && rg.crs().isValid() && crs != rg.crs() )
    {
      const QgsCoordinateTransform ct( rg.crs(), crs, context.project() );
      try
      {
        rg.transform( ct );
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error transforming geometry" ) );
      }
    }
    return rg;
  }

  QString valueAsString = parameterAsString( definition, value, context );
  if ( valueAsString.isEmpty() )
    valueAsString = definition->defaultValue().toString();

  if ( valueAsString.isEmpty() )
    return QgsGeometry();

  const QRegularExpression rx( QStringLiteral( "^\\s*(?:CRS=(.*);)?(.*?)$" ) );

  const QRegularExpressionMatch match = rx.match( valueAsString );
  if ( match.hasMatch() )
  {
    QgsGeometry g =  QgsGeometry::fromWkt( match.captured( 2 ) );
    if ( !g.isNull() )
    {
      const QgsCoordinateReferenceSystem geomCrs( match.captured( 1 ) );
      if ( crs.isValid() && geomCrs.isValid() && crs != geomCrs )
      {
        const QgsCoordinateTransform ct( geomCrs, crs, context.project() );
        try
        {
          g.transform( ct );
        }
        catch ( QgsCsException & )
        {
          QgsMessageLog::logMessage( QObject::tr( "Error transforming geometry" ) );
        }
      }
      return g;
    }
  }

  return QgsGeometry();
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsGeometryCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  const QVariant val = parameters.value( definition->name() );
  return parameterAsGeometryCrs( definition, val, context );
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsGeometryCrs( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( value.canConvert< QgsReferencedGeometry >() )
  {
    const QgsReferencedGeometry rg = value.value<QgsReferencedGeometry>();
    if ( rg.crs().isValid() )
    {
      return rg.crs();
    }
  }

  if ( value.canConvert< QgsReferencedPointXY >() )
  {
    const QgsReferencedPointXY rp = value.value<QgsReferencedPointXY>();
    if ( rp.crs().isValid() )
    {
      return rp.crs();
    }
  }

  if ( value.canConvert< QgsReferencedRectangle >() )
  {
    const QgsReferencedRectangle rr = value.value<QgsReferencedRectangle>();
    if ( rr.crs().isValid() )
    {
      return rr.crs();
    }
  }

  // Match against EWKT
  const QRegularExpression rx( QStringLiteral( "^\\s*(?:CRS=(.*);)?(.*?)$" ) );

  const QString valueAsString = parameterAsString( definition, value, context );
  const QRegularExpressionMatch match = rx.match( valueAsString );
  if ( match.hasMatch() )
  {
    const QgsCoordinateReferenceSystem crs( match.captured( 1 ) );
    if ( crs.isValid() )
      return crs;
  }

  if ( auto *lProject = context.project() )
    return lProject->crs();
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
  const QVariant val = value;
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
  const auto constSplit = resultString.split( ',' );
  bool ok;
  double number;
  for ( const QString &s : constSplit )
  {
    number = s.toDouble( &ok );
    result << ( ok ? QVariant( number ) : s );
  }

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

  const QVariant val = value;
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
      const auto constToList = var.toList();
      for ( const QVariant &listVar : constToList )
      {
        processVariant( listVar );
      }
    }
    else if ( var.type() == QVariant::StringList )
    {
      const auto constToStringList = var.toStringList();
      for ( const QString &s : constToStringList )
      {
        processVariant( s );
      }
    }
    else if ( var.canConvert<QgsProperty>() )
      processVariant( var.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() ) );
    else if ( var.canConvert<QgsProcessingOutputLayerDefinition>() )
    {
      // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
      const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
      const QVariant sink = fromVar.sink;
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
      const auto constToList = definition->defaultValue().toList();
      for ( const QVariant &var : constToList )
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

QStringList QgsProcessingParameters::parameterAsFileList( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  const QVariant val = value;

  QStringList files;

  std::function< void( const QVariant &var ) > processVariant;
  processVariant = [ &files, &context, &definition, &processVariant ]( const QVariant & var )
  {
    if ( var.type() == QVariant::List )
    {
      const auto constToList = var.toList();
      for ( const QVariant &listVar : constToList )
      {
        processVariant( listVar );
      }
    }
    else if ( var.type() == QVariant::StringList )
    {
      const auto constToStringList = var.toStringList();
      for ( const QString &s : constToStringList )
      {
        processVariant( s );
      }
    }
    else if ( var.canConvert<QgsProperty>() )
      processVariant( var.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() ) );
    else
    {
      files << var.toString();
    }
  };

  processVariant( val );

  if ( files.isEmpty() )
  {
    processVariant( definition->defaultValue() );
  }

  return files;
}

QStringList QgsProcessingParameters::parameterAsFileList( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  return parameterAsFileList( definition, parameters.value( definition->name() ), context );
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
  const QVariant val = value;

  if ( val.canConvert<QgsProperty>() )
    resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else if ( val.type() == QVariant::List )
  {
    const auto constToList = val.toList();
    for ( const QVariant &var : constToList )
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
      const auto constToList = definition->defaultValue().toList();
      for ( const QVariant &var : constToList )
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
    return QList< double >() << std::numeric_limits<double>::quiet_NaN()  << std::numeric_limits<double>::quiet_NaN() ;

  QList< double > result;
  bool ok = false;
  double n = resultStringList.at( 0 ).toDouble( &ok );
  if ( ok )
    result << n;
  else
    result << std::numeric_limits<double>::quiet_NaN() ;
  ok = false;
  n = resultStringList.at( 1 ).toDouble( &ok );
  if ( ok )
    result << n;
  else
    result << std::numeric_limits<double>::quiet_NaN() ;

  return result;
}

QStringList QgsProcessingParameters::parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  const QStringList resultStringList;
  return parameterAsFields( definition, parameters.value( definition->name() ), context );
}

QStringList QgsProcessingParameters::parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  QStringList resultStringList;
  const QVariant val = value;
  if ( val.isValid() )
  {
    if ( val.canConvert<QgsProperty>() )
      resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
    else if ( val.type() == QVariant::List )
    {
      const auto constToList = val.toList();
      for ( const QVariant &var : constToList )
        resultStringList << var.toString();
    }
    else if ( val.type() == QVariant::StringList )
    {
      resultStringList = val.toStringList();
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
        const auto constToList = definition->defaultValue().toList();
        for ( const QVariant &var : constToList )
          resultStringList << var.toString();
      }
      else if ( definition->defaultValue().type() == QVariant::StringList )
      {
        resultStringList = definition->defaultValue().toStringList();
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

QColor QgsProcessingParameters::parameterAsColor( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QColor();

  return parameterAsColor( definition, parameters.value( definition->name() ), context );
}

QColor QgsProcessingParameters::parameterAsColor( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QColor();

  QVariant val = value;
  if ( val.canConvert<QgsProperty>() )
  {
    val = val.value< QgsProperty >().value( context.expressionContext(), definition->defaultValue() );
  }
  if ( val.type() == QVariant::Color )
  {
    QColor c = val.value< QColor >();
    if ( const QgsProcessingParameterColor *colorParam = dynamic_cast< const QgsProcessingParameterColor * >( definition ) )
      if ( !colorParam->opacityEnabled() )
        c.setAlpha( 255 );
    return c;
  }

  QString colorText = parameterAsString( definition, value, context );
  if ( colorText.isEmpty() && !( definition->flags() & QgsProcessingParameterDefinition::FlagOptional ) )
  {
    if ( definition->defaultValue().type() == QVariant::Color )
      return definition->defaultValue().value< QColor >();
    else
      colorText = definition->defaultValue().toString();
  }

  if ( colorText.isEmpty() )
    return QColor();

  bool containsAlpha = false;
  QColor c = QgsSymbolLayerUtils::parseColorWithAlpha( colorText, containsAlpha );
  if ( const QgsProcessingParameterColor *colorParam = dynamic_cast< const QgsProcessingParameterColor * >( definition ) )
    if ( c.isValid() && !colorParam->opacityEnabled() )
      c.setAlpha( 255 );
  return c;
}

QString QgsProcessingParameters::parameterAsConnectionName( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  return parameterAsConnectionName( definition, parameters.value( definition->name() ), context );
}

QString QgsProcessingParameters::parameterAsConnectionName( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  // for now it's just treated identical to strings, but in future we may want flexibility to amend this
  // (hence the new method)
  return parameterAsString( definition, value, context );
}

QString QgsProcessingParameters::parameterAsSchema( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  return parameterAsSchema( definition, parameters.value( definition->name() ), context );
}

QString QgsProcessingParameters::parameterAsSchema( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  // for now it's just treated identical to strings, but in future we may want flexibility to amend this (e.g. if we want to embed connection details into the schema
  // parameter values, such as via a delimiter separated string)
  return parameterAsString( definition, value, context );
}

QString QgsProcessingParameters::parameterAsDatabaseTableName( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  return parameterAsDatabaseTableName( definition, parameters.value( definition->name() ), context );
}

QString QgsProcessingParameters::parameterAsDatabaseTableName( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsProcessingContext &context )
{
  // for now it's just treated identical to strings, but in future we may want flexibility to amend this (e.g. if we want to embed connection details into the table name
  // parameter values, such as via a delimiter separated string)
  return parameterAsString( definition, value, context );
}

QgsPointCloudLayer *QgsProcessingParameters::parameterAsPointCloudLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  return qobject_cast< QgsPointCloudLayer *>( parameterAsLayer( definition, parameters, context, QgsProcessingUtils::LayerHint::PointCloud ) );
}

QgsPointCloudLayer *QgsProcessingParameters::parameterAsPointCloudLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  return qobject_cast< QgsPointCloudLayer *>( parameterAsLayer( definition, value, context, QgsProcessingUtils::LayerHint::PointCloud ) );
}

QgsAnnotationLayer *QgsProcessingParameters::parameterAsAnnotationLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  return qobject_cast< QgsAnnotationLayer *>( parameterAsLayer( definition, parameters, context, QgsProcessingUtils::LayerHint::Annotation ) );
}

QgsAnnotationLayer *QgsProcessingParameters::parameterAsAnnotationLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  return qobject_cast< QgsAnnotationLayer *>( parameterAsLayer( definition, value, context, QgsProcessingUtils::LayerHint::Annotation ) );
}

QgsProcessingParameterDefinition *QgsProcessingParameters::parameterFromVariantMap( const QVariantMap &map )
{
  const QString type = map.value( QStringLiteral( "parameter_type" ) ).toString();
  const QString name = map.value( QStringLiteral( "name" ) ).toString();
  std::unique_ptr< QgsProcessingParameterDefinition > def;

  // probably all these hardcoded values aren't required anymore, and we could
  // always resort to the registry lookup...
  // TODO: confirm
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
  else if ( type == QgsProcessingParameterPointCloudDestination::typeName() )
    def.reset( new QgsProcessingParameterPointCloudDestination( name ) );
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
  else if ( type == QgsProcessingParameterColor::typeName() )
    def.reset( new QgsProcessingParameterColor( name ) );
  else if ( type == QgsProcessingParameterCoordinateOperation::typeName() )
    def.reset( new QgsProcessingParameterCoordinateOperation( name ) );
  else if ( type == QgsProcessingParameterPointCloudLayer::typeName() )
    def.reset( new QgsProcessingParameterPointCloudLayer( name ) );
  else if ( type == QgsProcessingParameterAnnotationLayer::typeName() )
    def.reset( new QgsProcessingParameterAnnotationLayer( name ) );
  else
  {
    QgsProcessingParameterType *paramType = QgsApplication::processingRegistry()->parameterType( type );
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

  const QString description = descriptionFromName( name );

  if ( type == QLatin1String( "boolean" ) )
    return QgsProcessingParameterBoolean::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "crs" ) )
    return QgsProcessingParameterCrs::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "layer" ) )
    return QgsProcessingParameterMapLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "extent" ) )
    return QgsProcessingParameterExtent::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "point" ) )
    return QgsProcessingParameterPoint::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "geometry" ) )
    return QgsProcessingParameterGeometry::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "file" ) )
    return QgsProcessingParameterFile::fromScriptCode( name, description, isOptional, definition, QgsProcessingParameterFile::File );
  else if ( type == QLatin1String( "folder" ) )
    return QgsProcessingParameterFile::fromScriptCode( name, description, isOptional, definition, QgsProcessingParameterFile::Folder );
  else if ( type == QLatin1String( "matrix" ) )
    return QgsProcessingParameterMatrix::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "multiple" ) )
    return QgsProcessingParameterMultipleLayers::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "number" ) )
    return QgsProcessingParameterNumber::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "distance" ) )
    return QgsProcessingParameterDistance::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "duration" ) )
    return QgsProcessingParameterDuration::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "scale" ) )
    return QgsProcessingParameterScale::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "range" ) )
    return QgsProcessingParameterRange::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "raster" ) )
    return QgsProcessingParameterRasterLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "enum" ) )
    return QgsProcessingParameterEnum::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "string" ) )
    return QgsProcessingParameterString::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "authcfg" ) )
    return QgsProcessingParameterAuthConfig::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "expression" ) )
    return QgsProcessingParameterExpression::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "field" ) )
    return QgsProcessingParameterField::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "vector" ) )
    return QgsProcessingParameterVectorLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "source" ) )
    return QgsProcessingParameterFeatureSource::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "sink" ) )
    return QgsProcessingParameterFeatureSink::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "vectordestination" ) )
    return QgsProcessingParameterVectorDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "rasterdestination" ) )
    return QgsProcessingParameterRasterDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "pointclouddestination" ) )
    return QgsProcessingParameterPointCloudDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "filedestination" ) )
    return QgsProcessingParameterFileDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "folderdestination" ) )
    return QgsProcessingParameterFolderDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "band" ) )
    return QgsProcessingParameterBand::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "mesh" ) )
    return QgsProcessingParameterMeshLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "layout" ) )
    return QgsProcessingParameterLayout::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "layoutitem" ) )
    return QgsProcessingParameterLayoutItem::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "color" ) )
    return QgsProcessingParameterColor::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "coordinateoperation" ) )
    return QgsProcessingParameterCoordinateOperation::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "maptheme" ) )
    return QgsProcessingParameterMapTheme::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "datetime" ) )
    return QgsProcessingParameterDateTime::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "providerconnection" ) )
    return QgsProcessingParameterProviderConnection::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "databaseschema" ) )
    return QgsProcessingParameterDatabaseSchema::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "databasetable" ) )
    return QgsProcessingParameterDatabaseTable::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "pointcloud" ) )
    return QgsProcessingParameterPointCloudLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == QLatin1String( "annotation" ) )
    return QgsProcessingParameterAnnotationLayer::fromScriptCode( name, description, isOptional, definition );

  return nullptr;
}

bool QgsProcessingParameters::parseScriptCodeParameterOptions( const QString &code, bool &isOptional, QString &name, QString &type, QString &definition )
{
  const QRegularExpression re( QStringLiteral( "(?:#*)(.*?)=\\s*(.*)" ) );
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

  const QRegularExpression re2( QStringLiteral( "(.*?)\\s+(.*)" ) );
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

QgsProcessingParameterDefinition::QgsProcessingParameterDefinition( const QString &name, const QString &description, const QVariant &defaultValue, bool optional, const QString &help )
  : mName( name )
  , mDescription( description )
  , mHelp( help )
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

QVariant QgsProcessingParameterDefinition::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return value;

  // dive into map and list types and convert each value
  if ( value.type() == QVariant::Type::Map )
  {
    const QVariantMap sourceMap = value.toMap();
    QVariantMap resultMap;
    for ( auto it = sourceMap.constBegin(); it != sourceMap.constEnd(); it++ )
    {
      resultMap[ it.key() ] = valueAsJsonObject( it.value(), context );
    }
    return resultMap;
  }
  else if ( value.type() == QVariant::Type::List || value.type() == QVariant::Type::StringList )
  {
    const QVariantList sourceList = value.toList();
    QVariantList resultList;
    resultList.reserve( sourceList.size() );
    for ( const QVariant &v : sourceList )
    {
      resultList.push_back( valueAsJsonObject( v, context ) );
    }
    return resultList;
  }
  else
  {
    switch ( value.userType() )
    {
      // simple types which can be directly represented in JSON -- note that strings are NOT handled here yet!
      case QMetaType::Bool:
      case QMetaType::Char:
      case QMetaType::Int:
      case QMetaType::Double:
      case QMetaType::Float:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::UInt:
      case QMetaType::ULong:
      case QMetaType::UShort:
        return value;

      default:
        break;
    }


    if ( value.userType() == QMetaType::type( "QgsProperty" ) )
    {
      const QgsProperty prop = value.value< QgsProperty >();
      switch ( prop.propertyType() )
      {
        case QgsProperty::InvalidProperty:
          return QVariant();
        case QgsProperty::StaticProperty:
          return valueAsJsonObject( prop.staticValue(), context );

        // these are not supported for serialization
        case QgsProperty::FieldBasedProperty:
        case QgsProperty::ExpressionBasedProperty:
          QgsDebugMsg( QStringLiteral( "could not convert expression/field based property to JSON object" ) );
          return QVariant();
      }
    }

    // value may be a CRS
    if ( value.userType() == QMetaType::type( "QgsCoordinateReferenceSystem" ) )
    {
      const QgsCoordinateReferenceSystem crs = value.value< QgsCoordinateReferenceSystem >();
      if ( !crs.isValid() )
        return QString();
      else if ( !crs.authid().isEmpty() )
        return crs.authid();
      else
        return crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED );
    }
    else if ( value.userType() == QMetaType::type( "QgsRectangle" ) )
    {
      const QgsRectangle r = value.value<QgsRectangle>();
      return QStringLiteral( "%1, %3, %2, %4" ).arg( qgsDoubleToString( r.xMinimum() ),
             qgsDoubleToString( r.yMinimum() ),
             qgsDoubleToString( r.xMaximum() ),
             qgsDoubleToString( r.yMaximum() ) );
    }
    else if ( value.userType() == QMetaType::type( "QgsReferencedRectangle" ) )
    {
      const QgsReferencedRectangle r = value.value<QgsReferencedRectangle>();
      return QStringLiteral( "%1, %3, %2, %4 [%5]" ).arg( qgsDoubleToString( r.xMinimum() ),
             qgsDoubleToString( r.yMinimum() ),
             qgsDoubleToString( r.xMaximum() ),
             qgsDoubleToString( r.yMaximum() ),                                                                                                                             r.crs().authid() );
    }
    else if ( value.userType() == QMetaType::type( "QgsGeometry" ) )
    {
      const QgsGeometry g = value.value<QgsGeometry>();
      if ( !g.isNull() )
      {
        return g.asWkt();
      }
      else
      {
        return QString();
      }
    }
    else if ( value.userType() == QMetaType::type( "QgsReferencedGeometry" ) )
    {
      const QgsReferencedGeometry g = value.value<QgsReferencedGeometry>();
      if ( !g.isNull() )
      {
        if ( !g.crs().isValid() )
          return g.asWkt();
        else
          return QStringLiteral( "CRS=%1;%2" ).arg( g.crs().authid().isEmpty() ? g.crs().toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) : g.crs().authid(), g.asWkt() );
      }
      else
      {
        return QString();
      }
    }
    else if ( value.userType() == QMetaType::type( "QgsPointXY" ) )
    {
      const QgsPointXY r = value.value<QgsPointXY>();
      return QStringLiteral( "%1,%2" ).arg( qgsDoubleToString( r.x() ),
                                            qgsDoubleToString( r.y() ) );
    }
    else if ( value.userType() == QMetaType::type( "QgsReferencedPointXY" ) )
    {
      const QgsReferencedPointXY r = value.value<QgsReferencedPointXY>();
      return QStringLiteral( "%1,%2 [%3]" ).arg( qgsDoubleToString( r.x() ),
             qgsDoubleToString( r.y() ),
             r.crs().authid() );
    }
    else if ( value.userType() == QMetaType::type( "QgsProcessingFeatureSourceDefinition" ) )
    {
      const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );

      // TODO -- we could consider also serializating the additional properties like invalid feature handling, limits, etc
      return valueAsJsonObject( fromVar.source, context );
    }
    else if ( value.userType() == QMetaType::type( "QgsProcessingOutputLayerDefinition" ) )
    {
      const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
      return valueAsJsonObject( fromVar.sink, context );
    }
    else if ( value.userType() == QMetaType::type( "QColor" ) )
    {
      const QColor fromVar = value.value< QColor >();
      if ( !fromVar.isValid() )
        return QString();

      return QStringLiteral( "rgba( %1, %2, %3, %4 )" ).arg( fromVar.red() ).arg( fromVar.green() ).arg( fromVar.blue() ).arg( QString::number( fromVar.alphaF(), 'f', 2 ) );
    }
    else if ( value.userType() == QMetaType::type( "QDateTime" ) )
    {
      const QDateTime fromVar = value.toDateTime();
      if ( !fromVar.isValid() )
        return QString();

      return fromVar.toString( Qt::ISODate );
    }
    else if ( value.userType() == QMetaType::type( "QDate" ) )
    {
      const QDate fromVar = value.toDate();
      if ( !fromVar.isValid() )
        return QString();

      return fromVar.toString( Qt::ISODate );
    }
    else if ( value.userType() == QMetaType::type( "QTime" ) )
    {
      const QTime fromVar = value.toTime();
      if ( !fromVar.isValid() )
        return QString();

      return fromVar.toString( Qt::ISODate );
    }

    // value may be a map layer
    QVariantMap p;
    p.insert( name(), value );
    if ( QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context ) )
    {
      const QString source = QgsProcessingUtils::normalizeLayerSource( layer->source() );
      if ( !source.isEmpty() )
        return source;
      return layer->id();
    }

    // now we handle strings, after any other specific logic has already been applied
    if ( value.userType() == QMetaType::QString )
      return value;
  }

  // unhandled type
  Q_ASSERT_X( false, "QgsProcessingParameterDefinition::valueAsJsonObject", QStringLiteral( "unsupported variant type %1" ).arg( QMetaType::typeName( value.userType() ) ).toLocal8Bit() );
  return value;
}

QString QgsProcessingParameterDefinition::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  ok = true;

  if ( !value.isValid() )
    return QString();

  switch ( value.userType() )
  {
    // simple types which can be directly represented in JSON -- note that strings are NOT handled here yet!
    case QMetaType::Bool:
    case QMetaType::Char:
    case QMetaType::Int:
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::UShort:
      return value.toString();

    default:
      break;
  }

  if ( value.userType() == QMetaType::type( "QgsProperty" ) )
  {
    const QgsProperty prop = value.value< QgsProperty >();
    switch ( prop.propertyType() )
    {
      case QgsProperty::InvalidProperty:
        return QString();
      case QgsProperty::StaticProperty:
        return valueAsString( prop.staticValue(), context, ok );

      // these are not supported for serialization
      case QgsProperty::FieldBasedProperty:
      case QgsProperty::ExpressionBasedProperty:
        QgsDebugMsg( QStringLiteral( "could not convert expression/field based property to string" ) );
        return QString();
    }
  }

  // value may be a CRS
  if ( value.userType() == QMetaType::type( "QgsCoordinateReferenceSystem" ) )
  {
    const QgsCoordinateReferenceSystem crs = value.value< QgsCoordinateReferenceSystem >();
    if ( !crs.isValid() )
      return QString();
    else if ( !crs.authid().isEmpty() )
      return crs.authid();
    else
      return crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED );
  }
  else if ( value.userType() == QMetaType::type( "QgsRectangle" ) )
  {
    const QgsRectangle r = value.value<QgsRectangle>();
    return QStringLiteral( "%1, %3, %2, %4" ).arg( qgsDoubleToString( r.xMinimum() ),
           qgsDoubleToString( r.yMinimum() ),
           qgsDoubleToString( r.xMaximum() ),
           qgsDoubleToString( r.yMaximum() ) );
  }
  else if ( value.userType() == QMetaType::type( "QgsReferencedRectangle" ) )
  {
    const QgsReferencedRectangle r = value.value<QgsReferencedRectangle>();
    return QStringLiteral( "%1, %3, %2, %4 [%5]" ).arg( qgsDoubleToString( r.xMinimum() ),
           qgsDoubleToString( r.yMinimum() ),
           qgsDoubleToString( r.xMaximum() ),
           qgsDoubleToString( r.yMaximum() ), r.crs().authid() );
  }
  else if ( value.userType() == QMetaType::type( "QgsGeometry" ) )
  {
    const QgsGeometry g = value.value<QgsGeometry>();
    if ( !g.isNull() )
    {
      return g.asWkt();
    }
    else
    {
      return QString();
    }
  }
  else if ( value.userType() == QMetaType::type( "QgsReferencedGeometry" ) )
  {
    const QgsReferencedGeometry g = value.value<QgsReferencedGeometry>();
    if ( !g.isNull() )
    {
      if ( !g.crs().isValid() )
        return g.asWkt();
      else
        return QStringLiteral( "CRS=%1;%2" ).arg( g.crs().authid().isEmpty() ? g.crs().toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) : g.crs().authid(), g.asWkt() );
    }
    else
    {
      return QString();
    }
  }
  else if ( value.userType() == QMetaType::type( "QgsPointXY" ) )
  {
    const QgsPointXY r = value.value<QgsPointXY>();
    return QStringLiteral( "%1,%2" ).arg( qgsDoubleToString( r.x() ),
                                          qgsDoubleToString( r.y() ) );
  }
  else if ( value.userType() == QMetaType::type( "QgsReferencedPointXY" ) )
  {
    const QgsReferencedPointXY r = value.value<QgsReferencedPointXY>();
    return QStringLiteral( "%1,%2 [%3]" ).arg( qgsDoubleToString( r.x() ),
           qgsDoubleToString( r.y() ),
           r.crs().authid() );
  }
  else if ( value.userType() == QMetaType::type( "QgsProcessingFeatureSourceDefinition" ) )
  {
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );
    return valueAsString( fromVar.source, context, ok );
  }
  else if ( value.userType() == QMetaType::type( "QgsProcessingOutputLayerDefinition" ) )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    return valueAsString( fromVar.sink, context, ok );
  }
  else if ( value.userType() == QMetaType::type( "QColor" ) )
  {
    const QColor fromVar = value.value< QColor >();
    if ( !fromVar.isValid() )
      return QString();

    return QStringLiteral( "rgba( %1, %2, %3, %4 )" ).arg( fromVar.red() ).arg( fromVar.green() ).arg( fromVar.blue() ).arg( QString::number( fromVar.alphaF(), 'f', 2 ) );
  }
  else if ( value.userType() == QMetaType::type( "QDateTime" ) )
  {
    const QDateTime fromVar = value.toDateTime();
    if ( !fromVar.isValid() )
      return QString();

    return fromVar.toString( Qt::ISODate );
  }
  else if ( value.userType() == QMetaType::type( "QDate" ) )
  {
    const QDate fromVar = value.toDate();
    if ( !fromVar.isValid() )
      return QString();

    return fromVar.toString( Qt::ISODate );
  }
  else if ( value.userType() == QMetaType::type( "QTime" ) )
  {
    const QTime fromVar = value.toTime();
    if ( !fromVar.isValid() )
      return QString();

    return fromVar.toString( Qt::ISODate );
  }

  // value may be a map layer
  QVariantMap p;
  p.insert( name(), value );
  if ( QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context ) )
  {
    const QString source = QgsProcessingUtils::normalizeLayerSource( layer->source() );
    if ( !source.isEmpty() )
      return source;
    return layer->id();
  }

  // now we handle strings, after any other specific logic has already been applied
  if ( value.userType() == QMetaType::QString )
    return value.toString();

  // unhandled type
  QgsDebugMsg( QStringLiteral( "unsupported variant type %1" ).arg( QMetaType::typeName( value.userType() ) ) );
  ok = false;
  return value.toString();
}

QStringList QgsProcessingParameterDefinition::valueAsStringList( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  ok = true;
  if ( !value.isValid( ) )
    return QStringList();

  if ( value.type() == QVariant::Type::List || value.type() == QVariant::Type::StringList )
  {
    const QVariantList sourceList = value.toList();
    QStringList resultList;
    resultList.reserve( sourceList.size() );
    for ( const QVariant &v : sourceList )
    {
      resultList.append( valueAsStringList( v, context, ok ) );
    }
    return resultList;
  }

  const QString res = valueAsString( value, context, ok );
  if ( !ok )
    return QStringList();

  return {res};
}

QString QgsProcessingParameterDefinition::valueAsPythonComment( const QVariant &, QgsProcessingContext & ) const
{
  return QString();
}

QString QgsProcessingParameterDefinition::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
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
        QString code = t->className() + QStringLiteral( "('%1', %2" )
                       .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
        if ( mFlags & FlagOptional )
          code += QLatin1String( ", optional=True" );

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
  map.insert( QStringLiteral( "help" ), mHelp );
  map.insert( QStringLiteral( "default" ), mDefault );
  map.insert( QStringLiteral( "defaultGui" ), mGuiDefault );
  map.insert( QStringLiteral( "flags" ), static_cast< int >( mFlags ) );
  map.insert( QStringLiteral( "metadata" ), mMetadata );
  return map;
}

bool QgsProcessingParameterDefinition::fromVariantMap( const QVariantMap &map )
{
  mName = map.value( QStringLiteral( "name" ) ).toString();
  mDescription = map.value( QStringLiteral( "description" ) ).toString();
  mHelp = map.value( QStringLiteral( "help" ) ).toString();
  mDefault = map.value( QStringLiteral( "default" ) );
  mGuiDefault = map.value( QStringLiteral( "defaultGui" ) );
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
  QString text = QStringLiteral( "<p><b>%1</b></p>" ).arg( description() );
  if ( !help().isEmpty() )
  {
    text += QStringLiteral( "<p>%1</p>" ).arg( help() );
  }
  text += QStringLiteral( "<p>%1</p>" ).arg( QObject::tr( "Python identifier: ‘%1’" ).arg( QStringLiteral( "<i>%1</i>" ).arg( name() ) ) );
  return text;
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
    code += QLatin1String( "optional " );
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

QgsProcessingParameterMapLayer::QgsProcessingParameterMapLayer( const QString &name, const QString &description, const QVariant &defaultValue, bool optional, const QList<int> &types )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , QgsProcessingParameterLimitedDataTypes( types )
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

QString createAllMapLayerFileFilter()
{
  QStringList vectors = QgsProviderRegistry::instance()->fileVectorFilters().split( QStringLiteral( ";;" ) );
  const QStringList rasters = QgsProviderRegistry::instance()->fileRasterFilters().split( QStringLiteral( ";;" ) );
  for ( const QString &raster : rasters )
  {
    if ( !vectors.contains( raster ) )
      vectors << raster;
  }
  const QStringList meshFilters = QgsProviderRegistry::instance()->fileMeshFilters().split( QStringLiteral( ";;" ) );
  for ( const QString &mesh : meshFilters )
  {
    if ( !vectors.contains( mesh ) )
      vectors << mesh;
  }
  const QStringList pointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters().split( QStringLiteral( ";;" ) );
  for ( const QString &pointCloud : pointCloudFilters )
  {
    if ( !vectors.contains( pointCloud ) )
      vectors << pointCloud;
  }
  vectors.removeAll( QObject::tr( "All files (*.*)" ) );
  std::sort( vectors.begin(), vectors.end() );

  return QObject::tr( "All files (*.*)" ) + QStringLiteral( ";;" ) + vectors.join( QLatin1String( ";;" ) );
}

QString QgsProcessingParameterMapLayer::createFileFilter() const
{
  return createAllMapLayerFileFilter();
}

QString QgsProcessingParameterMapLayer::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "layer " );

  for ( const int type : mDataTypes )
  {
    switch ( type )
    {
      case QgsProcessing::TypeVectorAnyGeometry:
        code += QLatin1String( "hasgeometry " );
        break;

      case QgsProcessing::TypeVectorPoint:
        code += QLatin1String( "point " );
        break;

      case QgsProcessing::TypeVectorLine:
        code += QLatin1String( "line " );
        break;

      case QgsProcessing::TypeVectorPolygon:
        code += QLatin1String( "polygon " );
        break;

      case QgsProcessing::TypeRaster:
        code += QLatin1String( "raster " );
        break;

      case QgsProcessing::TypeMesh:
        code += QLatin1String( "mesh " );
        break;

      case QgsProcessing::TypePlugin:
        code += QLatin1String( "plugin " );
        break;

      case QgsProcessing::TypePointCloud:
        code += QLatin1String( "pointcloud " );
        break;

      case QgsProcessing::TypeAnnotation:
        code += QLatin1String( "annotation " );
        break;
    }
  }

  code += mDefault.toString();
  return code.trimmed();
}

QgsProcessingParameterMapLayer *QgsProcessingParameterMapLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QList< int > types;
  QString def = definition;
  while ( true )
  {
    if ( def.startsWith( QLatin1String( "hasgeometry" ), Qt::CaseInsensitive ) )
    {
      types << QgsProcessing::TypeVectorAnyGeometry;
      def = def.mid( 12 );
      continue;
    }
    else if ( def.startsWith( QLatin1String( "point" ), Qt::CaseInsensitive ) )
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
    else if ( def.startsWith( QLatin1String( "raster" ), Qt::CaseInsensitive ) )
    {
      types << QgsProcessing::TypeRaster;
      def = def.mid( 7 );
      continue;
    }
    else if ( def.startsWith( QLatin1String( "mesh" ), Qt::CaseInsensitive ) )
    {
      types << QgsProcessing::TypeMesh;
      def = def.mid( 5 );
      continue;
    }
    else if ( def.startsWith( QLatin1String( "plugin" ), Qt::CaseInsensitive ) )
    {
      types << QgsProcessing::TypePlugin;
      def = def.mid( 7 );
      continue;
    }
    else if ( def.startsWith( QLatin1String( "pointcloud" ), Qt::CaseInsensitive ) )
    {
      types << QgsProcessing::TypePointCloud;
      def = def.mid( 11 );
      continue;
    }
    else if ( def.startsWith( QLatin1String( "annotation" ), Qt::CaseInsensitive ) )
    {
      types << QgsProcessing::TypeAnnotation;
      def = def.mid( 11 );
      continue;
    }
    break;
  }

  return new QgsProcessingParameterMapLayer( name, description, def, isOptional, types );
}

QString QgsProcessingParameterMapLayer::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterMapLayer('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1" ).arg( valueAsPythonString( mDefault, c ) );

      if ( !mDataTypes.empty() )
      {
        QStringList options;
        options.reserve( mDataTypes.size() );
        for ( const int t : mDataTypes )
          options << QStringLiteral( "QgsProcessing.%1" ).arg( QgsProcessing::sourceTypeToString( static_cast< QgsProcessing::SourceType >( t ) ) );
        code += QStringLiteral( ", types=[%1])" ).arg( options.join( ',' ) );
      }
      else
      {
        code += QLatin1Char( ')' );
      }

      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterMapLayer::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  QVariantList types;
  for ( const int type : mDataTypes )
  {
    types << type;
  }
  map.insert( QStringLiteral( "data_types" ), types );
  return map;
}

bool QgsProcessingParameterMapLayer::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mDataTypes.clear();
  const QVariantList values = map.value( QStringLiteral( "data_types" ) ).toList();
  for ( const QVariant &val : values )
  {
    mDataTypes << val.toInt();
  }
  return true;
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
    const QgsRectangle r = input.value<QgsRectangle>();
    return !r.isNull();
  }
  if ( input.canConvert< QgsGeometry >() )
  {
    return true;
  }
  if ( input.canConvert< QgsReferencedRectangle >() )
  {
    const QgsReferencedRectangle r = input.value<QgsReferencedRectangle>();
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

  const QRegularExpression rx( QStringLiteral( "^(.*?)\\s*,\\s*(.*?)\\s*,\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$" ) );
  const QRegularExpressionMatch match = rx.match( input.toString() );
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
    const QgsRectangle r = value.value<QgsRectangle>();
    return QStringLiteral( "'%1, %3, %2, %4'" ).arg( qgsDoubleToString( r.xMinimum() ),
           qgsDoubleToString( r.yMinimum() ),
           qgsDoubleToString( r.xMaximum() ),
           qgsDoubleToString( r.yMaximum() ) );
  }
  else if ( value.canConvert< QgsReferencedRectangle >() )
  {
    const QgsReferencedRectangle r = value.value<QgsReferencedRectangle>();
    return QStringLiteral( "'%1, %3, %2, %4 [%5]'" ).arg( qgsDoubleToString( r.xMinimum() ),
           qgsDoubleToString( r.yMinimum() ),
           qgsDoubleToString( r.xMaximum() ),
           qgsDoubleToString( r.yMaximum() ),                                                                                                                             r.crs().authid() );
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

  const QRegularExpression rx( QStringLiteral( "^\\s*\\(?\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*\\)?\\s*$" ) );

  const QRegularExpressionMatch match = rx.match( input.toString() );
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
    const QgsPointXY r = value.value<QgsPointXY>();
    return QStringLiteral( "'%1,%2'" ).arg( qgsDoubleToString( r.x() ),
                                            qgsDoubleToString( r.y() ) );
  }
  else if ( value.canConvert< QgsReferencedPointXY >() )
  {
    const QgsReferencedPointXY r = value.value<QgsReferencedPointXY>();
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

QgsProcessingParameterGeometry::QgsProcessingParameterGeometry( const QString &name, const QString &description,
    const QVariant &defaultValue, bool optional, const QList<int> &geometryTypes, bool allowMultipart )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional ),
    mGeomTypes( geometryTypes ),
    mAllowMultipart( allowMultipart )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterGeometry::clone() const
{
  return new QgsProcessingParameterGeometry( *this );
}

bool QgsProcessingParameterGeometry::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  const bool anyTypeAllowed = mGeomTypes.isEmpty() || mGeomTypes.contains( QgsWkbTypes::UnknownGeometry );

  if ( input.canConvert< QgsGeometry >() )
  {
    return ( anyTypeAllowed || mGeomTypes.contains( input.value<QgsGeometry>().type() ) ) &&
           ( mAllowMultipart || !input.value<QgsGeometry>().isMultipart() );
  }

  if ( input.canConvert< QgsReferencedGeometry >() )
  {
    return ( anyTypeAllowed || mGeomTypes.contains( input.value<QgsReferencedGeometry>().type() ) ) &&
           ( mAllowMultipart || !input.value<QgsReferencedGeometry>().isMultipart() );
  }

  if ( input.canConvert< QgsPointXY >() )
  {
    return anyTypeAllowed || mGeomTypes.contains( QgsWkbTypes::PointGeometry );
  }

  if ( input.canConvert< QgsRectangle >() )
  {
    return anyTypeAllowed || mGeomTypes.contains( QgsWkbTypes::PolygonGeometry );
  }

  if ( input.canConvert< QgsReferencedPointXY >() )
  {
    return anyTypeAllowed || mGeomTypes.contains( QgsWkbTypes::PointGeometry );
  }

  if ( input.canConvert< QgsReferencedRectangle >() )
  {
    return anyTypeAllowed || mGeomTypes.contains( QgsWkbTypes::PolygonGeometry );
  }

  if ( input.type() == QVariant::String )
  {
    if ( input.toString().isEmpty() )
      return mFlags & FlagOptional;
  }

  // Match against EWKT
  const QRegularExpression rx( QStringLiteral( "^\\s*(?:CRS=(.*);)?(.*?)$" ) );

  const QRegularExpressionMatch match = rx.match( input.toString() );
  if ( match.hasMatch() )
  {
    const QgsGeometry g = QgsGeometry::fromWkt( match.captured( 2 ) );
    if ( ! g.isNull() )
    {
      return ( anyTypeAllowed || mGeomTypes.contains( g.type() ) ) && ( mAllowMultipart || !g.isMultipart() );
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "Error creating geometry: \"%1\"" ).arg( g.lastError() ), QObject::tr( "Processing" ) );
    }
  }
  return false;
}

QString QgsProcessingParameterGeometry::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  auto asPythonString = []( const QgsGeometry & g, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() )
  {
    if ( !crs.isValid() )
      return QgsProcessingUtils::stringToPythonLiteral( g.asWkt() );
    else
      return QgsProcessingUtils::stringToPythonLiteral( QStringLiteral( "CRS=%1;%2" ).arg( crs.authid().isEmpty() ? crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ) : crs.authid(), g.asWkt() ) );
  };

  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression(%1)" ).arg( QgsProcessingUtils::stringToPythonLiteral( value.value< QgsProperty >().asExpression() ) );

  if ( value.canConvert< QgsGeometry >() )
  {
    const QgsGeometry g = value.value<QgsGeometry>();
    if ( !g.isNull() )
      return asPythonString( g );
  }

  if ( value.canConvert< QgsReferencedGeometry >() )
  {
    const QgsReferencedGeometry g = value.value<QgsReferencedGeometry>();
    if ( !g.isNull() )
      return asPythonString( g, g.crs() );
  }

  if ( value.canConvert< QgsPointXY >() )
  {
    const QgsGeometry g = QgsGeometry::fromPointXY( value.value<QgsPointXY>() );
    if ( !g.isNull() )
      return asPythonString( g );
  }

  if ( value.canConvert< QgsReferencedPointXY >() )
  {
    const QgsReferencedGeometry g = QgsReferencedGeometry::fromReferencedPointXY( value.value<QgsReferencedPointXY>() );
    if ( !g.isNull() )
      return asPythonString( g, g.crs() );
  }

  if ( value.canConvert< QgsRectangle >() )
  {
    const QgsGeometry g = QgsGeometry::fromRect( value.value<QgsRectangle>() );
    if ( !g.isNull() )
      return asPythonString( g );
  }

  if ( value.canConvert< QgsReferencedRectangle >() )
  {
    const QgsReferencedGeometry g = QgsReferencedGeometry::fromReferencedRect( value.value<QgsReferencedRectangle>() );
    if ( !g.isNull() )
      return asPythonString( g, g.crs() );
  }

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QString QgsProcessingParameterGeometry::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += type() + ' ';

  for ( const int type : mGeomTypes )
  {
    switch ( static_cast<QgsWkbTypes::GeometryType>( type ) )
    {
      case QgsWkbTypes::PointGeometry:
        code += QLatin1String( "point " );
        break;

      case QgsWkbTypes::LineGeometry:
        code += QLatin1String( "line " );
        break;

      case QgsWkbTypes::PolygonGeometry:
        code += QLatin1String( "polygon " );
        break;

      default:
        code += QLatin1String( "unknown " );
        break;
    }
  }

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterGeometry::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterGeometry('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      if ( !mGeomTypes.empty() )
      {
        auto geomTypeToString = []( QgsWkbTypes::GeometryType t ) -> QString
        {
          switch ( t )
          {
            case QgsWkbTypes::PointGeometry:
              return QStringLiteral( "PointGeometry" );

            case QgsWkbTypes::LineGeometry:
              return QStringLiteral( "LineGeometry" );

            case QgsWkbTypes::PolygonGeometry:
              return QStringLiteral( "PolygonGeometry" );

            case QgsWkbTypes::UnknownGeometry:
              return QStringLiteral( "UnknownGeometry" );

            case QgsWkbTypes::NullGeometry:
              return QStringLiteral( "NullGeometry" );
          }
          return QString();
        };

        QStringList options;
        options.reserve( mGeomTypes.size() );
        for ( const int type : mGeomTypes )
        {
          options << QStringLiteral( " QgsWkbTypes.%1" ).arg( geomTypeToString( static_cast<QgsWkbTypes::GeometryType>( type ) ) );
        }
        code += QStringLiteral( ", geometryTypes=[%1 ]" ).arg( options.join( ',' ) );
      }

      if ( ! mAllowMultipart )
      {
        code += QLatin1String( ", allowMultipart=False" );
      }

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterGeometry::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  QVariantList types;
  for ( const int type : mGeomTypes )
  {
    types << type;
  }
  map.insert( QStringLiteral( "geometrytypes" ), types );
  map.insert( QStringLiteral( "multipart" ), mAllowMultipart );
  return map;
}

bool QgsProcessingParameterGeometry::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mGeomTypes.clear();
  const QVariantList values = map.value( QStringLiteral( "geometrytypes" ) ).toList();
  for ( const QVariant &val : values )
  {
    mGeomTypes << val.toInt();
  }
  mAllowMultipart = map.value( QStringLiteral( "multipart" ) ).toBool();
  return true;
}

QgsProcessingParameterGeometry *QgsProcessingParameterGeometry::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterGeometry( name, description, definition, isOptional );
}

QgsProcessingParameterFile::QgsProcessingParameterFile( const QString &name, const QString &description, Behavior behavior, const QString &extension, const QVariant &defaultValue, bool optional, const QString &fileFilter )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mBehavior( behavior )
  , mExtension( fileFilter.isEmpty() ? extension : QString() )
  , mFileFilter( fileFilter.isEmpty() && extension.isEmpty() ? QObject::tr( "All files (*.*)" ) : fileFilter )
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

  const QString string = input.toString().trimmed();

  if ( input.type() != QVariant::String || string.isEmpty() )
    return mFlags & FlagOptional;

  switch ( mBehavior )
  {
    case File:
    {
      if ( !mExtension.isEmpty() )
      {
        return string.endsWith( mExtension, Qt::CaseInsensitive );
      }
      else if ( !mFileFilter.isEmpty() )
      {
        return QgsFileUtils::fileMatchesFilter( string, mFileFilter );
      }
      else
      {
        return true;
      }
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
    code += QLatin1String( "optional " );
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

      QString code = QStringLiteral( "QgsProcessingParameterFile('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );
      code += QStringLiteral( ", behavior=%1" ).arg( mBehavior == File ? QStringLiteral( "QgsProcessingParameterFile.File" ) : QStringLiteral( "QgsProcessingParameterFile.Folder" ) );
      if ( !mExtension.isEmpty() )
        code += QStringLiteral( ", extension='%1'" ).arg( mExtension );
      if ( !mFileFilter.isEmpty() )
        code += QStringLiteral( ", fileFilter='%1'" ).arg( mFileFilter );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterFile::createFileFilter() const
{
  switch ( mBehavior )
  {
    case File:
    {
      if ( !mFileFilter.isEmpty() )
        return mFileFilter != QObject::tr( "All files (*.*)" ) ? mFileFilter + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" ) : mFileFilter;
      else if ( !mExtension.isEmpty() )
        return QObject::tr( "%1 files" ).arg( mExtension.toUpper() ) + QStringLiteral( " (*." ) + mExtension.toLower() +  QStringLiteral( ");;" ) + QObject::tr( "All files (*.*)" );
      else
        return QObject::tr( "All files (*.*)" );
    }

    case Folder:
      return QString();
  }
  return QString();
}

void QgsProcessingParameterFile::setExtension( const QString &extension )
{
  mExtension = extension;
  mFileFilter.clear();
}

QString QgsProcessingParameterFile::fileFilter() const
{
  return mFileFilter;
}

void QgsProcessingParameterFile::setFileFilter( const QString &filter )
{
  mFileFilter = filter;
  mExtension.clear();
}

QVariantMap QgsProcessingParameterFile::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "behavior" ), mBehavior );
  map.insert( QStringLiteral( "extension" ), mExtension );
  map.insert( QStringLiteral( "filefilter" ), mFileFilter );
  return map;
}

bool QgsProcessingParameterFile::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mBehavior = static_cast< Behavior >( map.value( QStringLiteral( "behavior" ) ).toInt() );
  mExtension = map.value( QStringLiteral( "extension" ) ).toString();
  mFileFilter = map.value( QStringLiteral( "filefilter" ) ).toString();
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
  const QVariantList list = QgsProcessingParameters::parameterAsMatrix( this, p, context );

  return QgsProcessingUtils::variantToPythonLiteral( list );
}

QString QgsProcessingParameterMatrix::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterMatrix('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );
      code += QStringLiteral( ", numberRows=%1" ).arg( mNumberRows );
      code += QStringLiteral( ", hasFixedNumberRows=%1" ).arg( mFixedNumberRows ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

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

  if ( mLayerType != QgsProcessing::TypeFile )
  {
    if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( input ) ) )
    {
      return true;
    }
  }

  if ( input.type() == QVariant::String )
  {
    if ( input.toString().isEmpty() )
      return mFlags & FlagOptional;

    if ( mMinimumNumberInputs > 1 )
      return false;

    if ( !context )
      return true;

    if ( mLayerType != QgsProcessing::TypeFile )
      return QgsProcessingUtils::mapLayerFromString( input.toString(), *context );
    else
      return true;
  }
  else if ( input.type() == QVariant::List )
  {
    if ( input.toList().count() < mMinimumNumberInputs )
      return mFlags & FlagOptional;

    if ( mMinimumNumberInputs > input.toList().count() )
      return false;

    if ( !context )
      return true;

    if ( mLayerType != QgsProcessing::TypeFile )
    {
      const auto constToList = input.toList();
      for ( const QVariant &v : constToList )
      {
        if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( v ) ) )
          continue;

        if ( !QgsProcessingUtils::mapLayerFromString( v.toString(), *context ) )
          return false;
      }
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

    if ( mLayerType != QgsProcessing::TypeFile )
    {
      const auto constToStringList = input.toStringList();
      for ( const QString &v : constToStringList )
      {
        if ( !QgsProcessingUtils::mapLayerFromString( v, *context ) )
          return false;
      }
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

  if ( mLayerType == QgsProcessing::TypeFile )
  {
    QStringList parts;
    if ( value.type() == QVariant::StringList )
    {
      const QStringList list = value.toStringList();
      parts.reserve( list.count() );
      for ( const QString &v : list )
        parts <<  QgsProcessingUtils::stringToPythonLiteral( v );
    }
    else if ( value.type() == QVariant::List )
    {
      const QVariantList list = value.toList();
      parts.reserve( list.count() );
      for ( const QVariant &v : list )
        parts <<  QgsProcessingUtils::stringToPythonLiteral( v.toString() );
    }
    if ( !parts.isEmpty() )
      return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else
  {
    QVariantMap p;
    p.insert( name(), value );
    const QList<QgsMapLayer *> list = QgsProcessingParameters::parameterAsLayerList( this, p, context );
    if ( !list.isEmpty() )
    {
      QStringList parts;
      parts.reserve( list.count() );
      for ( const QgsMapLayer *layer : list )
      {
        parts << QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer->source() ) );
      }
      return parts.join( ',' ).prepend( '[' ).append( ']' );
    }
  }

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QString QgsProcessingParameterMultipleLayers::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  switch ( mLayerType )
  {
    case QgsProcessing::TypeRaster:
      code += QLatin1String( "multiple raster" );
      break;

    case QgsProcessing::TypeFile:
      code += QLatin1String( "multiple file" );
      break;

    default:
      code += QLatin1String( "multiple vector" );
      break;
  }
  code += ' ';
  if ( mDefault.type() == QVariant::List )
  {
    QStringList parts;
    const auto constToList = mDefault.toList();
    for ( const QVariant &var : constToList )
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
      QString code = QStringLiteral( "QgsProcessingParameterMultipleLayers('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      const QString layerType = QStringLiteral( "QgsProcessing.%1" ).arg( QgsProcessing::sourceTypeToString( mLayerType ) );

      code += QStringLiteral( ", layerType=%1" ).arg( layerType );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterMultipleLayers::createFileFilter() const
{
  const QStringList exts;
  switch ( mLayerType )
  {
    case QgsProcessing::TypeFile:
      return QObject::tr( "All files (*.*)" );

    case QgsProcessing::TypeRaster:
      return QgsProviderRegistry::instance()->fileRasterFilters() + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );

    case QgsProcessing::TypeVector:
    case QgsProcessing::TypeVectorAnyGeometry:
    case QgsProcessing::TypeVectorPoint:
    case QgsProcessing::TypeVectorLine:
    case QgsProcessing::TypeVectorPolygon:
      return QgsProviderRegistry::instance()->fileVectorFilters() + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );

    case QgsProcessing::TypeMesh:
      return QgsProviderRegistry::instance()->fileMeshFilters() + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );

    case QgsProcessing::TypePointCloud:
      return QgsProviderRegistry::instance()->filePointCloudFilters() + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );

    case QgsProcessing::TypeMapLayer:
    case QgsProcessing::TypePlugin:
    case QgsProcessing::TypeAnnotation:
      return createAllMapLayerFileFilter();
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
  const QRegularExpression re( QStringLiteral( "(.*?)\\s+(.*)" ) );
  const QRegularExpressionMatch m = re.match( definition );
  if ( m.hasMatch() )
  {
    type = m.captured( 1 ).toLower().trimmed();
    defaultVal = m.captured( 2 );
  }
  QgsProcessing::SourceType layerType = QgsProcessing::TypeVectorAnyGeometry;
  if ( type == QLatin1String( "vector" ) )
    layerType = QgsProcessing::TypeVectorAnyGeometry;
  else if ( type == QLatin1String( "raster" ) )
    layerType = QgsProcessing::TypeRaster;
  else if ( type == QLatin1String( "file" ) )
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
  const double res = input.toDouble( &ok );
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
  const QString extra = parts.join( QLatin1String( "<br />" ) );
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
      QString code = QStringLiteral( "QgsProcessingParameterNumber('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

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
         : ( definition.toLower().trimmed() == QLatin1String( "none" ) ? QVariant() : definition ), isOptional );
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
    const QStringList list = input.toString().split( ',' );
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
  const QList< double > parts = QgsProcessingParameters::parameterAsRange( this, p, context );

  QStringList stringParts;
  const auto constParts = parts;
  for ( const double v : constParts )
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
      QString code = QStringLiteral( "QgsProcessingParameterRange('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

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
  return new QgsProcessingParameterRange( name, description, QgsProcessingParameterNumber::Double, definition.isEmpty() ? QVariant()
                                          : ( definition.toLower().trimmed() == QLatin1String( "none" ) ? QVariant() : definition ), isOptional );
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
  if ( QgsProcessingUtils::mapLayerFromString( input.toString(), *context, true, QgsProcessingUtils::LayerHint::Raster ) )
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

QString QgsProcessingParameterRasterLayer::createFileFilter() const
{
  return QgsProviderRegistry::instance()->fileRasterFilters() + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );
}

QgsProcessingParameterRasterLayer *QgsProcessingParameterRasterLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterRasterLayer( name, description, definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterEnum::QgsProcessingParameterEnum( const QString &name, const QString &description, const QStringList &options, bool allowMultiple, const QVariant &defaultValue, bool optional, bool usesStaticStrings )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mOptions( options )
  , mAllowMultiple( allowMultiple )
  , mUsesStaticStrings( usesStaticStrings )
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

  if ( mUsesStaticStrings )
  {
    if ( input.type() == QVariant::List )
    {
      if ( !mAllowMultiple )
        return false;

      const QVariantList values = input.toList();
      if ( values.empty() && !( mFlags & FlagOptional ) )
        return false;

      for ( const QVariant &val : values )
      {
        if ( !mOptions.contains( val.toString() ) )
          return false;
      }

      return true;
    }
    else if ( input.type() == QVariant::StringList )
    {
      if ( !mAllowMultiple )
        return false;

      const QStringList values = input.toStringList();

      if ( values.empty() && !( mFlags & FlagOptional ) )
        return false;

      if ( values.count() > 1 && !mAllowMultiple )
        return false;

      for ( const QString &val : values )
      {
        if ( !mOptions.contains( val ) )
          return false;
      }
      return true;
    }
    else if ( input.type() == QVariant::String )
    {
      const QStringList parts = input.toString().split( ',' );
      if ( parts.count() > 1 && !mAllowMultiple )
        return false;

      const auto constParts = parts;
      for ( const QString &part : constParts )
      {
        if ( !mOptions.contains( part ) )
          return false;
      }
      return true;
    }
  }
  else
  {
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
        const int res = val.toInt( &ok );
        if ( !ok )
          return false;
        else if ( res < 0 || res >= mOptions.count() )
          return false;
      }

      return true;
    }
    else if ( input.type() == QVariant::String )
    {
      const QStringList parts = input.toString().split( ',' );
      if ( parts.count() > 1 && !mAllowMultiple )
        return false;

      const auto constParts = parts;
      for ( const QString &part : constParts )
      {
        bool ok = false;
        const int res = part.toInt( &ok );
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
      const int res = input.toInt( &ok );
      if ( !ok )
        return false;
      else if ( res >= 0 && res < mOptions.count() )
        return true;
    }
  }

  return false;
}

QString QgsProcessingParameterEnum::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( mUsesStaticStrings )
  {
    if ( value.type() == QVariant::StringList )
    {
      QStringList parts;
      const QStringList constList = value.toStringList();
      for ( const QString &val : constList )
      {
        parts << QgsProcessingUtils::stringToPythonLiteral( val );
      }
      return parts.join( ',' ).prepend( '[' ).append( ']' );
    }
    else if ( value.type() == QVariant::String )
    {
      QStringList parts;
      const QStringList constList = value.toString().split( ',' );
      if ( constList.count() > 1 )
      {
        for ( const QString &val : constList )
        {
          parts << QgsProcessingUtils::stringToPythonLiteral( val );
        }
        return parts.join( ',' ).prepend( '[' ).append( ']' );
      }
    }

    return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
  }
  else
  {
    if ( value.type() == QVariant::List )
    {
      QStringList parts;
      const auto constToList = value.toList();
      for ( const QVariant &val : constToList )
      {
        parts << QString::number( static_cast< int >( val.toDouble() ) );
      }
      return parts.join( ',' ).prepend( '[' ).append( ']' );
    }
    else if ( value.type() == QVariant::String )
    {
      const QStringList parts = value.toString().split( ',' );
      if ( parts.count() > 1 )
      {
        return parts.join( ',' ).prepend( '[' ).append( ']' );
      }
    }

    return QString::number( static_cast< int >( value.toDouble() ) );
  }
}

QString QgsProcessingParameterEnum::valueAsPythonComment( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QString();

  if ( value.canConvert<QgsProperty>() )
    return QString();

  if ( mUsesStaticStrings )
  {
    return QString();
  }
  else
  {
    if ( value.type() == QVariant::List )
    {
      QStringList parts;
      const QVariantList toList = value.toList();
      parts.reserve( toList.size() );
      for ( const QVariant &val : toList )
      {
        parts << mOptions.value( static_cast< int >( val.toDouble() ) );
      }
      return parts.join( ',' );
    }
    else if ( value.type() == QVariant::String )
    {
      const QStringList parts = value.toString().split( ',' );
      QStringList comments;
      if ( parts.count() > 1 )
      {
        for ( const QString &part : parts )
        {
          bool ok = false;
          const int val = part.toInt( &ok );
          if ( ok )
            comments << mOptions.value( val );
        }
        return comments.join( ',' );
      }
    }

    return mOptions.value( static_cast< int >( value.toDouble() ) );
  }
}

QString QgsProcessingParameterEnum::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "enum " );

  if ( mAllowMultiple )
    code += QLatin1String( "multiple " );

  if ( mUsesStaticStrings )
    code += QLatin1String( "static " );

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
      QString code = QStringLiteral( "QgsProcessingParameterEnum('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      QStringList options;
      options.reserve( mOptions.size() );
      for ( const QString &o : mOptions )
        options << QgsProcessingUtils::stringToPythonLiteral( o );
      code += QStringLiteral( ", options=[%1]" ).arg( options.join( ',' ) );

      code += QStringLiteral( ", allowMultiple=%1" ).arg( mAllowMultiple ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      code += QStringLiteral( ", usesStaticStrings=%1" ).arg( mUsesStaticStrings ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

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

bool QgsProcessingParameterEnum::usesStaticStrings() const
{
  return mUsesStaticStrings;
}

void QgsProcessingParameterEnum::setUsesStaticStrings( bool usesStaticStrings )
{
  mUsesStaticStrings = usesStaticStrings;
}

QVariantMap QgsProcessingParameterEnum::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "options" ), mOptions );
  map.insert( QStringLiteral( "allow_multiple" ), mAllowMultiple );
  map.insert( QStringLiteral( "uses_static_strings" ), mUsesStaticStrings );
  return map;
}

bool QgsProcessingParameterEnum::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mOptions = map.value( QStringLiteral( "options" ) ).toStringList();
  mAllowMultiple = map.value( QStringLiteral( "allow_multiple" ) ).toBool();
  mUsesStaticStrings = map.value( QStringLiteral( "uses_static_strings" ) ).toBool();
  return true;
}

QgsProcessingParameterEnum *QgsProcessingParameterEnum::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString defaultVal;
  QString def = definition;

  bool multiple = false;
  if ( def.startsWith( QLatin1String( "multiple" ), Qt::CaseInsensitive ) )
  {
    multiple = true;
    def = def.mid( 9 );
  }

  bool staticStrings = false;
  if ( def.startsWith( QLatin1String( "static" ), Qt::CaseInsensitive ) )
  {
    staticStrings = true;
    def = def.mid( 7 );
  }

  const QRegularExpression re( QStringLiteral( "(.*)\\s+(.*?)$" ) );
  const QRegularExpressionMatch m = re.match( def );
  QString values = def;
  if ( m.hasMatch() )
  {
    values = m.captured( 1 ).trimmed();
    defaultVal = m.captured( 2 );
  }

  return new QgsProcessingParameterEnum( name, description, values.split( ';' ), multiple, defaultVal.isEmpty() ? QVariant() : defaultVal, isOptional, staticStrings );
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

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterString::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "string " );

  if ( mMultiLine )
    code += QLatin1String( "long " );

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterString::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterString('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );
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
  if ( def == QLatin1String( "None" ) )
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

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterAuthConfig::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "authcfg " );

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
  if ( def == QLatin1String( "None" ) )
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

  const QString s = value.toString();
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
      QString code = QStringLiteral( "QgsProcessingParameterExpression('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

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
    const QgsProperty p = var.value< QgsProperty >();
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
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context, true, QgsProcessingUtils::LayerHint::Vector ) )
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
      QString code = QStringLiteral( "QgsProcessingParameterVectorLayer('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      if ( !mDataTypes.empty() )
      {
        QStringList options;
        for ( const int t : mDataTypes )
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

QString QgsProcessingParameterVectorLayer::createFileFilter() const
{
  return QgsProviderRegistry::instance()->fileVectorFilters() + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );
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
  for ( const int type : mDataTypes )
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
  const QVariantList values = map.value( QStringLiteral( "data_types" ) ).toList();
  for ( const QVariant &val : values )
  {
    mDataTypes << val.toInt();
  }
  return true;
}

QgsProcessingParameterVectorLayer *QgsProcessingParameterVectorLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterVectorLayer( name, description, QList< int>(),  definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterMeshLayer::QgsProcessingParameterMeshLayer( const QString &name, const QString &description,
    const QVariant &defaultValue, bool optional )
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
    const QgsProperty p = var.value< QgsProperty >();
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
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context, true, QgsProcessingUtils::LayerHint::Mesh ) )
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

QString QgsProcessingParameterMeshLayer::createFileFilter() const
{
  return QgsProviderRegistry::instance()->fileMeshFilters() + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );
}

QgsProcessingParameterMeshLayer *QgsProcessingParameterMeshLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterMeshLayer( name, description,  definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterField::QgsProcessingParameterField( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, DataType type, bool allowMultiple, bool optional, bool defaultToAllFields )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameterName( parentLayerParameterName )
  , mDataType( type )
  , mAllowMultiple( allowMultiple )
  , mDefaultToAllFields( defaultToAllFields )
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

    const QStringList parts = input.toString().split( ';' );
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
    const auto constToList = value.toList();
    for ( const QVariant &val : constToList )
    {
      parts << QgsProcessingUtils::stringToPythonLiteral( val.toString() );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else if ( value.type() == QVariant::StringList )
  {
    QStringList parts;
    const auto constToStringList = value.toStringList();
    for ( const QString &s : constToStringList )
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
    code += QLatin1String( "optional " );
  code += QLatin1String( "field " );

  switch ( mDataType )
  {
    case Numeric:
      code += QLatin1String( "numeric " );
      break;

    case String:
      code += QLatin1String( "string " );
      break;

    case DateTime:
      code += QLatin1String( "datetime " );
      break;

    case Any:
      break;
  }

  if ( mAllowMultiple )
    code += QLatin1String( "multiple " );

  if ( mDefaultToAllFields )
    code += QLatin1String( "default_to_all_fields " );

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
      QString code = QStringLiteral( "QgsProcessingParameterField('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

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
      code += QStringLiteral( ", defaultValue=%1" ).arg( valueAsPythonString( mDefault, c ) );

      if ( mDefaultToAllFields )
        code += QLatin1String( ", defaultToAllFields=True" );

      code += ')';

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

bool QgsProcessingParameterField::defaultToAllFields() const
{
  return mDefaultToAllFields;
}

void QgsProcessingParameterField::setDefaultToAllFields( bool enabled )
{
  mDefaultToAllFields = enabled;
}

QVariantMap QgsProcessingParameterField::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "parent_layer" ), mParentLayerParameterName );
  map.insert( QStringLiteral( "data_type" ), mDataType );
  map.insert( QStringLiteral( "allow_multiple" ), mAllowMultiple );
  map.insert( QStringLiteral( "default_to_all_fields" ), mDefaultToAllFields );
  return map;
}

bool QgsProcessingParameterField::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( QStringLiteral( "parent_layer" ) ).toString();
  mDataType = static_cast< DataType >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  mAllowMultiple = map.value( QStringLiteral( "allow_multiple" ) ).toBool();
  mDefaultToAllFields = map.value( QStringLiteral( "default_to_all_fields" ) ).toBool();
  return true;
}

QgsProcessingParameterField *QgsProcessingParameterField::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString parent;
  DataType type = Any;
  bool allowMultiple = false;
  bool defaultToAllFields = false;
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

  if ( def.startsWith( QLatin1String( "default_to_all_fields" ), Qt::CaseInsensitive ) )
  {
    defaultToAllFields = true;
    def = def.mid( 21 ).trimmed();
  }

  const QRegularExpression re( QStringLiteral( "(.*?)\\s+(.*)$" ) );
  const QRegularExpressionMatch m = re.match( def );
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

  return new QgsProcessingParameterField( name, description, def.isEmpty() ? QVariant() : def, parent, type, allowMultiple, isOptional, defaultToAllFields );
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
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( var );
    var = fromVar.source;
  }
  else if ( var.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
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
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context, true, QgsProcessingUtils::LayerHint::Vector ) )
    return true;

  return false;
}

QString QgsProcessingParameterFeatureSource::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression(%1)" ).arg( QgsProcessingUtils::stringToPythonLiteral( value.value< QgsProperty >().asExpression() ) );

  if ( value.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );
    QString geometryCheckString;
    switch ( fromVar.geometryCheck )
    {
      case QgsFeatureRequest::GeometryNoCheck:
        geometryCheckString = QStringLiteral( "QgsFeatureRequest.GeometryNoCheck" );
        break;

      case QgsFeatureRequest::GeometrySkipInvalid:
        geometryCheckString = QStringLiteral( "QgsFeatureRequest.GeometrySkipInvalid" );
        break;

      case QgsFeatureRequest::GeometryAbortOnInvalid:
        geometryCheckString = QStringLiteral( "QgsFeatureRequest.GeometryAbortOnInvalid" );
        break;
    }

    QStringList flags;
    QString flagString;
    if ( fromVar.flags & QgsProcessingFeatureSourceDefinition::Flag::FlagOverrideDefaultGeometryCheck )
      flags << QStringLiteral( "QgsProcessingFeatureSourceDefinition.FlagOverrideDefaultGeometryCheck" );
    if ( fromVar.flags & QgsProcessingFeatureSourceDefinition::Flag::FlagCreateIndividualOutputPerInputFeature )
      flags << QStringLiteral( "QgsProcessingFeatureSourceDefinition.FlagCreateIndividualOutputPerInputFeature" );
    if ( !flags.empty() )
      flagString = flags.join( QLatin1String( " | " ) );

    if ( fromVar.source.propertyType() == QgsProperty::StaticProperty )
    {
      QString layerString = fromVar.source.staticValue().toString();
      // prefer to use layer source instead of id if possible (since it's persistent)
      if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( layerString, context, true, QgsProcessingUtils::LayerHint::Vector ) ) )
        layerString = layer->source();

      if ( fromVar.selectedFeaturesOnly || fromVar.featureLimit != -1 || fromVar.flags )
      {
        return QStringLiteral( "QgsProcessingFeatureSourceDefinition(%1, selectedFeaturesOnly=%2, featureLimit=%3%4, geometryCheck=%5)" ).arg( QgsProcessingUtils::stringToPythonLiteral( layerString ),
               fromVar.selectedFeaturesOnly ? QStringLiteral( "True" ) : QStringLiteral( "False" ),
               QString::number( fromVar.featureLimit ),
               flagString.isEmpty() ? QString() : ( QStringLiteral( ", flags=%1" ).arg( flagString ) ),
               geometryCheckString );
      }
      else
      {
        return QgsProcessingUtils::stringToPythonLiteral( layerString );
      }
    }
    else
    {
      if ( fromVar.selectedFeaturesOnly || fromVar.featureLimit != -1 || fromVar.flags )
      {
        return QStringLiteral( "QgsProcessingFeatureSourceDefinition(QgsProperty.fromExpression(%1), selectedFeaturesOnly=%2, featureLimit=%3%4, geometryCheck=%5)" )
               .arg( QgsProcessingUtils::stringToPythonLiteral( fromVar.source.asExpression() ),
                     fromVar.selectedFeaturesOnly ? QStringLiteral( "True" ) : QStringLiteral( "False" ),
                     QString::number( fromVar.featureLimit ),
                     flagString.isEmpty() ? QString() : ( QStringLiteral( ", flags=%1" ).arg( flagString ) ),
                     geometryCheckString );
      }
      else
      {
        return QStringLiteral( "QgsProperty.fromExpression(%1)" ).arg( QgsProcessingUtils::stringToPythonLiteral( fromVar.source.asExpression() ) );
      }
    }
  }
  else if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( value ) ) )
  {
    return QgsProcessingUtils::stringToPythonLiteral( layer->source() );
  }

  QString layerString = value.toString();

  // prefer to use layer source if possible (since it's persistent)
  if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( layerString, context, true, QgsProcessingUtils::LayerHint::Vector ) ) )
    layerString = layer->providerType() != QLatin1String( "ogr" ) && layer->providerType() != QLatin1String( "gdal" ) && layer->providerType() != QLatin1String( "mdal" ) ? QgsProcessingUtils::encodeProviderKeyAndUri( layer->providerType(), layer->source() ) : layer->source();

  return QgsProcessingUtils::stringToPythonLiteral( layerString );
}

QString QgsProcessingParameterFeatureSource::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "source " );

  for ( const int type : mDataTypes )
  {
    switch ( type )
    {
      case QgsProcessing::TypeVectorPoint:
        code += QLatin1String( "point " );
        break;

      case QgsProcessing::TypeVectorLine:
        code += QLatin1String( "line " );
        break;

      case QgsProcessing::TypeVectorPolygon:
        code += QLatin1String( "polygon " );
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
      QString code = QStringLiteral( "QgsProcessingParameterFeatureSource('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      if ( !mDataTypes.empty() )
      {
        QStringList options;
        options.reserve( mDataTypes.size() );
        for ( const int t : mDataTypes )
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

QString QgsProcessingParameterFeatureSource::createFileFilter() const
{
  return QgsProviderRegistry::instance()->fileVectorFilters() + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );
}

QgsProcessingParameterLimitedDataTypes::QgsProcessingParameterLimitedDataTypes( const QList<int> &types )
  : mDataTypes( types )
{

}

QVariantMap QgsProcessingParameterFeatureSource::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  QVariantList types;
  for ( const int type : mDataTypes )
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
  const QVariantList values = map.value( QStringLiteral( "data_types" ) ).toList();
  for ( const QVariant &val : values )
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

QgsProcessingParameterFeatureSink::QgsProcessingParameterFeatureSink( const QString &name, const QString &description, QgsProcessing::SourceType type, const QVariant &defaultValue, bool optional, bool createByDefault, bool supportsAppend )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional, createByDefault )
  , mDataType( type )
  , mSupportsAppend( supportsAppend )
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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
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
    code += QLatin1String( "optional " );
  code += QLatin1String( "sink " );

  switch ( mDataType )
  {
    case QgsProcessing::TypeVectorPoint:
      code += QLatin1String( "point " );
      break;

    case QgsProcessing::TypeVectorLine:
      code += QLatin1String( "line " );
      break;

    case QgsProcessing::TypeVectorPolygon:
      code += QLatin1String( "polygon " );
      break;

    case QgsProcessing::TypeVector:
      code += QLatin1String( "table " );
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
  if ( auto *lOriginalProvider = originalProvider() )
  {
    return lOriginalProvider->defaultVectorFileExtension( hasGeometry() );
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->defaultVectorFileExtension( hasGeometry() );
  }
  else
  {
    if ( hasGeometry() )
    {
      return QgsProcessingUtils::defaultVectorExtension();
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
      QString code = QStringLiteral( "QgsProcessingParameterFeatureSink('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      code += QStringLiteral( ", type=QgsProcessing.%1" ).arg( QgsProcessing::sourceTypeToString( mDataType ) );

      code += QStringLiteral( ", createByDefault=%1" ).arg( createByDefault() ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );
      if ( mSupportsAppend )
        code += QLatin1String( ", supportsAppend=True" );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterFeatureSink::createFileFilter() const
{
  const QStringList exts = supportedOutputVectorLayerExtensions();
  QStringList filters;
  for ( const QString &ext : exts )
  {
    filters << QObject::tr( "%1 files (*.%2)" ).arg( ext.toUpper(), ext.toLower() );
  }
  return filters.join( QLatin1String( ";;" ) ) + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );

}

QStringList QgsProcessingParameterFeatureSink::supportedOutputVectorLayerExtensions() const
{
  if ( auto *lOriginalProvider = originalProvider() )
  {
    if ( hasGeometry() )
      return lOriginalProvider->supportedOutputVectorLayerExtensions();
    else
      return lOriginalProvider->supportedOutputTableExtensions();
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
    case QgsProcessing::TypePlugin:
    case QgsProcessing::TypePointCloud:
    case QgsProcessing::TypeAnnotation:
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
  map.insert( QStringLiteral( "supports_append" ), mSupportsAppend );
  return map;
}

bool QgsProcessingParameterFeatureSink::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingDestinationParameter::fromVariantMap( map );
  mDataType = static_cast< QgsProcessing::SourceType >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  mSupportsAppend = map.value( QStringLiteral( "supports_append" ), false ).toBool();
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

bool QgsProcessingParameterFeatureSink::supportsAppend() const
{
  return mSupportsAppend;
}

void QgsProcessingParameterFeatureSink::setSupportsAppend( bool supportsAppend )
{
  mSupportsAppend = supportsAppend;
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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
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
  if ( auto *lOriginalProvider = originalProvider() )
  {
    return lOriginalProvider->defaultRasterFileExtension();
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->defaultRasterFileExtension();
  }
  else
  {
    return QgsProcessingUtils::defaultRasterExtension();
  }
}

QString QgsProcessingParameterRasterDestination::createFileFilter() const
{
  const QStringList exts = supportedOutputRasterLayerExtensions();
  QStringList filters;
  for ( const QString &ext : exts )
  {
    filters << QObject::tr( "%1 files (*.%2)" ).arg( ext.toUpper(), ext.toLower() );
  }
  return filters.join( QLatin1String( ";;" ) ) + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );
}

QStringList QgsProcessingParameterRasterDestination::supportedOutputRasterLayerExtensions() const
{
  if ( auto *lOriginalProvider = originalProvider() )
  {
    return lOriginalProvider->supportedOutputRasterLayerExtensions();
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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
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
  const QRegularExpression rx( QStringLiteral( ".*?\\(\\*\\.([a-zA-Z0-9._]+).*" ) );
  const QRegularExpressionMatch match = rx.match( mFileFilter );
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
      QString code = QStringLiteral( "QgsProcessingParameterFileDestination('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      code += QStringLiteral( ", fileFilter=%1" ).arg( QgsProcessingUtils::stringToPythonLiteral( mFileFilter ) );

      code += QStringLiteral( ", createByDefault=%1" ).arg( createByDefault() ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterFileDestination::createFileFilter() const
{
  return ( fileFilter().isEmpty() ? QString() : fileFilter() + QStringLiteral( ";;" ) ) + QObject::tr( "All files (*.*)" );
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

QgsProcessingParameterFolderDestination::QgsProcessingParameterFolderDestination( const QString &name, const QString &description, const QVariant &defaultValue, bool optional, bool createByDefault )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional, createByDefault )
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
    const QgsProperty p = var.value< QgsProperty >();
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
        QString code = t->className() + QStringLiteral( "('%1', %2" )
                       .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
        if ( mFlags & FlagOptional )
          code += QLatin1String( ", optional=True" );

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

QString QgsProcessingDestinationParameter::createFileFilter() const
{
  return QObject::tr( "Default extension" ) + QStringLiteral( " (*." ) + defaultFileExtension() + ')';
}

QString QgsProcessingDestinationParameter::generateTemporaryDestination() const
{
  // sanitize name to avoid multiple . in the filename. E.g. when name() contain
  // backend command name having a "." inside as in case of grass commands
  const QRegularExpression rx( QStringLiteral( "[.]" ) );
  QString sanitizedName = name();
  sanitizedName.replace( rx, QStringLiteral( "_" ) );

  if ( defaultFileExtension().isEmpty() )
  {
    return QgsProcessingUtils::generateTempFilename( sanitizedName );
  }
  else
  {
    return QgsProcessingUtils::generateTempFilename( sanitizedName + '.' + defaultFileExtension() );
  }
}

bool QgsProcessingDestinationParameter::isSupportedOutputValue( const QVariant &value, QgsProcessingContext &context, QString &error ) const
{
  if ( auto *lOriginalProvider = originalProvider() )
    return lOriginalProvider->isSupportedOutputValue( value, this, context, error );
  else if ( provider() )
    return provider()->isSupportedOutputValue( value, this, context, error );

  return true;
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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
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
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
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
    code += QLatin1String( "optional " );
  code += QLatin1String( "vectorDestination " );

  switch ( mDataType )
  {
    case QgsProcessing::TypeVectorPoint:
      code += QLatin1String( "point " );
      break;

    case QgsProcessing::TypeVectorLine:
      code += QLatin1String( "line " );
      break;

    case QgsProcessing::TypeVectorPolygon:
      code += QLatin1String( "polygon " );
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
  if ( auto *lOriginalProvider = originalProvider() )
  {
    return lOriginalProvider->defaultVectorFileExtension( hasGeometry() );
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->defaultVectorFileExtension( hasGeometry() );
  }
  else
  {
    if ( hasGeometry() )
    {
      return QgsProcessingUtils::defaultVectorExtension();
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
      QString code = QStringLiteral( "QgsProcessingParameterVectorDestination('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      code += QStringLiteral( ", type=QgsProcessing.%1" ).arg( QgsProcessing::sourceTypeToString( mDataType ) );

      code += QStringLiteral( ", createByDefault=%1" ).arg( createByDefault() ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterVectorDestination::createFileFilter() const
{
  const QStringList exts = supportedOutputVectorLayerExtensions();
  QStringList filters;
  for ( const QString &ext : exts )
  {
    filters << QObject::tr( "%1 files (*.%2)" ).arg( ext.toUpper(), ext.toLower() );
  }
  return filters.join( QLatin1String( ";;" ) ) + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );
}

QStringList QgsProcessingParameterVectorDestination::supportedOutputVectorLayerExtensions() const
{
  if ( auto *lOriginalProvider = originalProvider() )
  {
    if ( hasGeometry() )
      return lOriginalProvider->supportedOutputVectorLayerExtensions();
    else
      return lOriginalProvider->supportedOutputTableExtensions();
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
    case QgsProcessing::TypePlugin:
    case QgsProcessing::TypePointCloud:
    case QgsProcessing::TypeAnnotation:
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
    const double res = input.toInt( &ok );
    Q_UNUSED( res )
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
    const QVariantList values = value.toList();
    for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
    {
      parts << QString::number( static_cast< int >( it->toDouble() ) );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else if ( value.type() == QVariant::StringList )
  {
    QStringList parts;
    const QStringList values = value.toStringList();
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
    code += QLatin1String( "optional " );
  code += QLatin1String( "band " );

  if ( mAllowMultiple )
    code += QLatin1String( "multiple " );

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
      QString code = QStringLiteral( "QgsProcessingParameterBand('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

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

  const QRegularExpression re( QStringLiteral( "(.*?)\\s+(.*)$" ) );
  const QRegularExpressionMatch m = re.match( def );
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
      QString code = QStringLiteral( "QgsProcessingParameterDistance('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

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
// QgsProcessingParameterDuration
//

QgsProcessingParameterDuration::QgsProcessingParameterDuration( const QString &name, const QString &description, const QVariant &defaultValue, bool optional, double minValue, double maxValue )
  : QgsProcessingParameterNumber( name, description, Double, defaultValue, optional, minValue, maxValue )
{
}

QgsProcessingParameterDuration *QgsProcessingParameterDuration::clone() const
{
  return new QgsProcessingParameterDuration( *this );
}

QString QgsProcessingParameterDuration::type() const
{
  return typeName();
}

QString QgsProcessingParameterDuration::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterDuration('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

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

QVariantMap QgsProcessingParameterDuration::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterNumber::toVariantMap();
  map.insert( QStringLiteral( "default_unit" ), static_cast< int >( mDefaultUnit ) );
  return map;
}

bool QgsProcessingParameterDuration::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterNumber::fromVariantMap( map );
  mDefaultUnit = static_cast< QgsUnitTypes::TemporalUnit>( map.value( QStringLiteral( "default_unit" ), QgsUnitTypes::TemporalDays ).toInt() );
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
      QString code = QStringLiteral( "QgsProcessingParameterScale('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );
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
                                          : ( definition.toLower().trimmed() == QLatin1String( "none" ) ? QVariant() : definition ), isOptional );
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

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterLayout::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "layout " );

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterLayout::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterLayout('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );
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
  if ( def == QLatin1String( "None" ) )
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

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterLayoutItem::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "layoutitem " );
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
      QString code = QStringLiteral( "QgsProcessingParameterLayoutItem('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

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
  const QRegularExpression re( QStringLiteral( "(\\d+)?\\s*(.*?)\\s+(.*)$" ) );
  const QRegularExpressionMatch m = re.match( def );
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

//
// QgsProcessingParameterColor
//

QgsProcessingParameterColor::QgsProcessingParameterColor( const QString &name, const QString &description, const QVariant &defaultValue, bool opacityEnabled, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mAllowOpacity( opacityEnabled )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterColor::clone() const
{
  return new QgsProcessingParameterColor( *this );
}

QString QgsProcessingParameterColor::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() || value.isNull() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert< QColor >() && !value.value< QColor >().isValid() )
    return QStringLiteral( "QColor()" );

  if ( value.canConvert< QColor >() )
  {
    const QColor c = value.value< QColor >();
    if ( !mAllowOpacity || c.alpha() == 255 )
      return QStringLiteral( "QColor(%1, %2, %3)" ).arg( c.red() ).arg( c.green() ).arg( c.blue() );
    else
      return QStringLiteral( "QColor(%1, %2, %3, %4)" ).arg( c.red() ).arg( c.green() ).arg( c.blue() ).arg( c.alpha() );
  }

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterColor::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "color " );

  if ( mAllowOpacity )
    code += QLatin1String( "withopacity " );

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterColor::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterColor('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      code += QStringLiteral( ", opacityEnabled=%1" ).arg( mAllowOpacity ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

bool QgsProcessingParameterColor::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() && ( mDefault.isValid() && ( !mDefault.toString().isEmpty() || mDefault.value< QColor >().isValid() ) ) )
    return true;

  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.type() == QVariant::Color )
  {
    return true;
  }
  else if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() != QVariant::String || input.toString().isEmpty() )
    return mFlags & FlagOptional;

  bool containsAlpha = false;
  return QgsSymbolLayerUtils::parseColorWithAlpha( input.toString(), containsAlpha ).isValid();
}

QVariantMap QgsProcessingParameterColor::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "opacityEnabled" ), mAllowOpacity );
  return map;
}

bool QgsProcessingParameterColor::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mAllowOpacity = map.value( QStringLiteral( "opacityEnabled" ) ).toBool();
  return true;
}

bool QgsProcessingParameterColor::opacityEnabled() const
{
  return mAllowOpacity;
}

void QgsProcessingParameterColor::setOpacityEnabled( bool enabled )
{
  mAllowOpacity = enabled;
}

QgsProcessingParameterColor *QgsProcessingParameterColor::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString def = definition;

  bool allowOpacity = false;
  if ( def.startsWith( QLatin1String( "withopacity" ), Qt::CaseInsensitive ) )
  {
    allowOpacity = true;
    def = def.mid( 12 );
  }

  if ( def.startsWith( '"' ) || def.startsWith( '\'' ) )
    def = def.mid( 1 );
  if ( def.endsWith( '"' ) || def.endsWith( '\'' ) )
    def.chop( 1 );

  QVariant defaultValue = def;
  if ( def == QLatin1String( "None" ) )
    defaultValue = QVariant();

  return new QgsProcessingParameterColor( name, description, defaultValue, allowOpacity, isOptional );
}

//
// QgsProcessingParameterCoordinateOperation
//
QgsProcessingParameterCoordinateOperation::QgsProcessingParameterCoordinateOperation( const QString &name, const QString &description, const QVariant &defaultValue, const QString &sourceCrsParameterName, const QString &destinationCrsParameterName, const QVariant &staticSourceCrs, const QVariant &staticDestinationCrs, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mSourceParameterName( sourceCrsParameterName )
  , mDestParameterName( destinationCrsParameterName )
  , mSourceCrs( staticSourceCrs )
  , mDestCrs( staticDestinationCrs )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterCoordinateOperation::clone() const
{
  return new QgsProcessingParameterCoordinateOperation( * this );
}

QString QgsProcessingParameterCoordinateOperation::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() || value.isNull() )
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

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterCoordinateOperation::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "coordinateoperation " );

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterCoordinateOperation::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QgsProcessingContext c;
      QString code = QStringLiteral( "QgsProcessingParameterCoordinateOperation('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );
      if ( !mSourceParameterName.isEmpty() )
        code += QStringLiteral( ", sourceCrsParameterName=%1" ).arg( valueAsPythonString( mSourceParameterName, c ) );
      if ( !mDestParameterName.isEmpty() )
        code += QStringLiteral( ", destinationCrsParameterName=%1" ).arg( valueAsPythonString( mDestParameterName, c ) );

      if ( mSourceCrs.isValid() )
        code += QStringLiteral( ", staticSourceCrs=%1" ).arg( valueAsPythonString( mSourceCrs, c ) );
      if ( mDestCrs.isValid() )
        code += QStringLiteral( ", staticDestinationCrs=%1" ).arg( valueAsPythonString( mDestCrs, c ) );

      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterCoordinateOperation::dependsOnOtherParameters() const
{
  QStringList res;
  if ( !mSourceParameterName.isEmpty() )
    res << mSourceParameterName;
  if ( !mDestParameterName.isEmpty() )
    res << mDestParameterName;
  return res;
}

QVariantMap QgsProcessingParameterCoordinateOperation::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "source_crs_parameter_name" ), mSourceParameterName );
  map.insert( QStringLiteral( "dest_crs_parameter_name" ), mDestParameterName );
  map.insert( QStringLiteral( "static_source_crs" ), mSourceCrs );
  map.insert( QStringLiteral( "static_dest_crs" ), mDestCrs );
  return map;
}

bool QgsProcessingParameterCoordinateOperation::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mSourceParameterName = map.value( QStringLiteral( "source_crs_parameter_name" ) ).toString();
  mDestParameterName = map.value( QStringLiteral( "dest_crs_parameter_name" ) ).toString();
  mSourceCrs = map.value( QStringLiteral( "static_source_crs" ) );
  mDestCrs = map.value( QStringLiteral( "static_dest_crs" ) );
  return true;
}

QgsProcessingParameterCoordinateOperation *QgsProcessingParameterCoordinateOperation::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString def = definition;

  if ( def.startsWith( '"' ) || def.startsWith( '\'' ) )
    def = def.mid( 1 );
  if ( def.endsWith( '"' ) || def.endsWith( '\'' ) )
    def.chop( 1 );

  QVariant defaultValue = def;
  if ( def == QLatin1String( "None" ) )
    defaultValue = QVariant();

  return new QgsProcessingParameterCoordinateOperation( name, description, defaultValue, QString(), QString(), QVariant(), QVariant(), isOptional );
}


//
// QgsProcessingParameterMapTheme
//

QgsProcessingParameterMapTheme::QgsProcessingParameterMapTheme( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}


QgsProcessingParameterDefinition *QgsProcessingParameterMapTheme::clone() const
{
  return new QgsProcessingParameterMapTheme( *this );
}

bool QgsProcessingParameterMapTheme::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() && !mDefault.isValid() )
    return mFlags & FlagOptional;

  if ( ( input.type() == QVariant::String && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.type() == QVariant::String && mDefault.toString().isEmpty() ) )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterMapTheme::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterMapTheme::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "maptheme " );

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterMapTheme::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterMapTheme('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );

      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterMapTheme::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  return map;
}

bool QgsProcessingParameterMapTheme::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  return true;
}

QgsProcessingParameterMapTheme *QgsProcessingParameterMapTheme::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString def = definition;
  if ( def.startsWith( '"' ) || def.startsWith( '\'' ) )
    def = def.mid( 1 );
  if ( def.endsWith( '"' ) || def.endsWith( '\'' ) )
    def.chop( 1 );

  QVariant defaultValue = def;

  if ( defaultValue == QLatin1String( "None" ) || defaultValue.toString().isEmpty() )
    defaultValue = QVariant();

  return new QgsProcessingParameterMapTheme( name, description, defaultValue, isOptional );
}


//
// QgsProcessingParameterDateTime
//

QgsProcessingParameterDateTime::QgsProcessingParameterDateTime( const QString &name, const QString &description, Type type, const QVariant &defaultValue, bool optional, const QDateTime &minValue, const QDateTime &maxValue )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mMin( minValue )
  , mMax( maxValue )
  , mDataType( type )
{
  if ( mMin.isValid() && mMax.isValid() && mMin >= mMax )
  {
    QgsMessageLog::logMessage( QObject::tr( "Invalid datetime parameter \"%1\": min value %2 is >= max value %3!" ).arg( name, mMin.toString(), mMax.toString() ), QObject::tr( "Processing" ) );
  }
}

QgsProcessingParameterDefinition *QgsProcessingParameterDateTime::clone() const
{
  return new QgsProcessingParameterDateTime( *this );
}

bool QgsProcessingParameterDateTime::checkValueIsAcceptable( const QVariant &value, QgsProcessingContext * ) const
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

  if ( input.type() != QVariant::DateTime && input.type() != QVariant::Date && input.type() != QVariant::Time && input.type() != QVariant::String )
    return false;

  if ( ( input.type() == QVariant::DateTime || input.type() == QVariant::Date ) && mDataType == Time )
    return false;

  if ( input.type() == QVariant::String )
  {
    const QString s = input.toString();
    if ( s.isEmpty() )
      return mFlags & FlagOptional;

    input = QDateTime::fromString( s, Qt::ISODate );
    if ( mDataType == Time )
    {
      if ( !input.toDateTime().isValid() )
        input = QTime::fromString( s );
      else
        input = input.toDateTime().time();
    }
  }

  if ( mDataType != Time )
  {
    const QDateTime res = input.toDateTime();
    return res.isValid() && ( res >= mMin || !mMin.isValid() ) && ( res <= mMax || !mMax.isValid() );
  }
  else
  {
    const QTime res = input.toTime();
    return res.isValid() && ( res >= mMin.time() || !mMin.isValid() ) && ( res <= mMax.time() || !mMax.isValid() );
  }
}

QString QgsProcessingParameterDateTime::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.type() == QVariant::DateTime )
  {
    const QDateTime dt = value.toDateTime();
    if ( !dt.isValid() )
      return QStringLiteral( "QDateTime()" );
    else
      return QStringLiteral( "QDateTime(QDate(%1, %2, %3), QTime(%4, %5, %6))" ).arg( dt.date().year() )
             .arg( dt.date().month() )
             .arg( dt.date().day() )
             .arg( dt.time().hour() )
             .arg( dt.time().minute() )
             .arg( dt.time().second() );
  }
  else if ( value.type() == QVariant::Date )
  {
    const QDate dt = value.toDate();
    if ( !dt.isValid() )
      return QStringLiteral( "QDate()" );
    else
      return QStringLiteral( "QDate(%1, %2, %3)" ).arg( dt.year() )
             .arg( dt.month() )
             .arg( dt.day() );
  }
  else if ( value.type() == QVariant::Time )
  {
    const QTime dt = value.toTime();
    if ( !dt.isValid() )
      return QStringLiteral( "QTime()" );
    else
      return QStringLiteral( "QTime(%4, %5, %6)" )
             .arg( dt.hour() )
             .arg( dt.minute() )
             .arg( dt.second() );
  }
  return value.toString();
}

QString QgsProcessingParameterDateTime::toolTip() const
{
  QString text = QgsProcessingParameterDefinition::toolTip();
  QStringList parts;
  if ( mMin.isValid() )
    parts << QObject::tr( "Minimum value: %1" ).arg( mMin.toString( Qt::ISODate ) );
  if ( mMax.isValid() )
    parts << QObject::tr( "Maximum value: %1" ).arg( mMax.toString( Qt::ISODate ) );
  if ( mDefault.isValid() )
    parts << QObject::tr( "Default value: %1" ).arg( mDataType == DateTime ? mDefault.toDateTime().toString( Qt::ISODate ) :
          ( mDataType == Date ? mDefault.toDate().toString( Qt::ISODate ) : mDefault.toTime( ).toString() ) );
  const QString extra = parts.join( QLatin1String( "<br />" ) );
  if ( !extra.isEmpty() )
    text += QStringLiteral( "<p>%1</p>" ).arg( extra );
  return text;
}

QString QgsProcessingParameterDateTime::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterDateTime('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      code += QStringLiteral( ", type=%1" ).arg( mDataType == DateTime ? QStringLiteral( "QgsProcessingParameterDateTime.DateTime" )
              : mDataType == Date ? QStringLiteral( "QgsProcessingParameterDateTime.Date" )
              : QStringLiteral( "QgsProcessingParameterDateTime.Time" ) );

      QgsProcessingContext c;
      if ( mMin.isValid() )
        code += QStringLiteral( ", minValue=%1" ).arg( valueAsPythonString( mMin, c ) );
      if ( mMax.isValid() )
        code += QStringLiteral( ", maxValue=%1" ).arg( valueAsPythonString( mMax, c ) );
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QDateTime QgsProcessingParameterDateTime::minimum() const
{
  return mMin;
}

void QgsProcessingParameterDateTime::setMinimum( const QDateTime &min )
{
  mMin = min;
}

QDateTime QgsProcessingParameterDateTime::maximum() const
{
  return mMax;
}

void QgsProcessingParameterDateTime::setMaximum( const QDateTime &max )
{
  mMax = max;
}

QgsProcessingParameterDateTime::Type QgsProcessingParameterDateTime::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterDateTime::setDataType( Type dataType )
{
  mDataType = dataType;
}

QVariantMap QgsProcessingParameterDateTime::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "min" ), mMin );
  map.insert( QStringLiteral( "max" ), mMax );
  map.insert( QStringLiteral( "data_type" ), mDataType );
  return map;
}

bool QgsProcessingParameterDateTime::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mMin = map.value( QStringLiteral( "min" ) ).toDateTime();
  mMax = map.value( QStringLiteral( "max" ) ).toDateTime();
  mDataType = static_cast< Type >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  return true;
}

QgsProcessingParameterDateTime *QgsProcessingParameterDateTime::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterDateTime( name, description, DateTime, definition.isEmpty() ? QVariant()
         : ( definition.toLower().trimmed() == QLatin1String( "none" ) ? QVariant() : definition ), isOptional );
}



//
// QgsProcessingParameterProviderConnection
//

QgsProcessingParameterProviderConnection::QgsProcessingParameterProviderConnection( const QString &name, const QString &description, const QString &provider, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mProviderId( provider )
{

}


QgsProcessingParameterDefinition *QgsProcessingParameterProviderConnection::clone() const
{
  return new QgsProcessingParameterProviderConnection( *this );
}

bool QgsProcessingParameterProviderConnection::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() && !mDefault.isValid() )
    return mFlags & FlagOptional;

  if ( ( input.type() == QVariant::String && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.type() == QVariant::String && mDefault.toString().isEmpty() ) )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterProviderConnection::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterProviderConnection::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "providerconnection " );
  code += mProviderId + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterProviderConnection::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterProviderConnection('%1', %2, '%3'" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ), mProviderId );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1)" ).arg( valueAsPythonString( mDefault, c ) );

      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterProviderConnection::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "provider" ), mProviderId );
  return map;
}

bool QgsProcessingParameterProviderConnection::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mProviderId = map.value( QStringLiteral( "provider" ) ).toString();
  return true;
}

QgsProcessingParameterProviderConnection *QgsProcessingParameterProviderConnection::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString def = definition;
  QString provider;
  if ( def.contains( ' ' ) )
  {
    provider = def.left( def.indexOf( ' ' ) );
    def = def.mid( def.indexOf( ' ' ) + 1 );
  }
  else
  {
    provider = def;
    def.clear();
  }

  if ( def.startsWith( '"' ) || def.startsWith( '\'' ) )
    def = def.mid( 1 );
  if ( def.endsWith( '"' ) || def.endsWith( '\'' ) )
    def.chop( 1 );

  QVariant defaultValue = def;

  if ( defaultValue == QLatin1String( "None" ) || defaultValue.toString().isEmpty() )
    defaultValue = QVariant();

  return new QgsProcessingParameterProviderConnection( name, description, provider, defaultValue, isOptional );
}


//
// QgsProcessingParameterDatabaseSchema
//

QgsProcessingParameterDatabaseSchema::QgsProcessingParameterDatabaseSchema( const QString &name, const QString &description, const QString &parentLayerParameterName, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentConnectionParameterName( parentLayerParameterName )
{

}


QgsProcessingParameterDefinition *QgsProcessingParameterDatabaseSchema::clone() const
{
  return new QgsProcessingParameterDatabaseSchema( *this );
}

bool QgsProcessingParameterDatabaseSchema::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() && !mDefault.isValid() )
    return mFlags & FlagOptional;

  if ( ( input.type() == QVariant::String && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.type() == QVariant::String && mDefault.toString().isEmpty() ) )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterDatabaseSchema::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterDatabaseSchema::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "databaseschema " );

  code += mParentConnectionParameterName + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterDatabaseSchema::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterDatabaseSchema('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      code += QStringLiteral( ", connectionParameterName='%1'" ).arg( mParentConnectionParameterName );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1" ).arg( valueAsPythonString( mDefault, c ) );

      code += ')';

      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterDatabaseSchema::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentConnectionParameterName.isEmpty() )
    depends << mParentConnectionParameterName;
  return depends;
}

QString QgsProcessingParameterDatabaseSchema::parentConnectionParameterName() const
{
  return mParentConnectionParameterName;
}

void QgsProcessingParameterDatabaseSchema::setParentConnectionParameterName( const QString &name )
{
  mParentConnectionParameterName = name;
}

QVariantMap QgsProcessingParameterDatabaseSchema::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "mParentConnectionParameterName" ), mParentConnectionParameterName );
  return map;
}

bool QgsProcessingParameterDatabaseSchema::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentConnectionParameterName = map.value( QStringLiteral( "mParentConnectionParameterName" ) ).toString();
  return true;
}

QgsProcessingParameterDatabaseSchema *QgsProcessingParameterDatabaseSchema::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString parent;
  QString def = definition;

  const QRegularExpression re( QStringLiteral( "(.*?)\\s+(.*)$" ) );
  const QRegularExpressionMatch m = re.match( def );
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

  return new QgsProcessingParameterDatabaseSchema( name, description, parent, def.isEmpty() ? QVariant() : def, isOptional );
}

//
// QgsProcessingParameterDatabaseTable
//

QgsProcessingParameterDatabaseTable::QgsProcessingParameterDatabaseTable( const QString &name, const QString &description,
    const QString &connectionParameterName,
    const QString &schemaParameterName,
    const QVariant &defaultValue, bool optional, bool allowNewTableNames )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentConnectionParameterName( connectionParameterName )
  , mParentSchemaParameterName( schemaParameterName )
  , mAllowNewTableNames( allowNewTableNames )
{

}


QgsProcessingParameterDefinition *QgsProcessingParameterDatabaseTable::clone() const
{
  return new QgsProcessingParameterDatabaseTable( *this );
}

bool QgsProcessingParameterDatabaseTable::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() && !mDefault.isValid() )
    return mFlags & FlagOptional;

  if ( ( input.type() == QVariant::String && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.type() == QVariant::String && mDefault.toString().isEmpty() ) )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterDatabaseTable::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterDatabaseTable::asScriptCode() const
{
  QString code = QStringLiteral( "##%1=" ).arg( mName );
  if ( mFlags & FlagOptional )
    code += QLatin1String( "optional " );
  code += QLatin1String( "databasetable " );

  code += ( mParentConnectionParameterName.isEmpty() ? QStringLiteral( "none" ) : mParentConnectionParameterName ) + ' ';
  code += ( mParentSchemaParameterName.isEmpty() ? QStringLiteral( "none" ) : mParentSchemaParameterName ) + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterDatabaseTable::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterDatabaseTable('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );

      if ( mAllowNewTableNames )
        code += QLatin1String( ", allowNewTableNames=True" );

      code += QStringLiteral( ", connectionParameterName='%1'" ).arg( mParentConnectionParameterName );
      code += QStringLiteral( ", schemaParameterName='%1'" ).arg( mParentSchemaParameterName );
      QgsProcessingContext c;
      code += QStringLiteral( ", defaultValue=%1" ).arg( valueAsPythonString( mDefault, c ) );

      code += ')';

      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterDatabaseTable::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentConnectionParameterName.isEmpty() )
    depends << mParentConnectionParameterName;
  if ( !mParentSchemaParameterName.isEmpty() )
    depends << mParentSchemaParameterName;
  return depends;
}

QString QgsProcessingParameterDatabaseTable::parentConnectionParameterName() const
{
  return mParentConnectionParameterName;
}

void QgsProcessingParameterDatabaseTable::setParentConnectionParameterName( const QString &name )
{
  mParentConnectionParameterName = name;
}

QString QgsProcessingParameterDatabaseTable::parentSchemaParameterName() const
{
  return mParentSchemaParameterName;
}

void QgsProcessingParameterDatabaseTable::setParentSchemaParameterName( const QString &name )
{
  mParentSchemaParameterName = name;
}

QVariantMap QgsProcessingParameterDatabaseTable::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "mParentConnectionParameterName" ), mParentConnectionParameterName );
  map.insert( QStringLiteral( "mParentSchemaParameterName" ), mParentSchemaParameterName );
  map.insert( QStringLiteral( "mAllowNewTableNames" ), mAllowNewTableNames );
  return map;
}

bool QgsProcessingParameterDatabaseTable::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentConnectionParameterName = map.value( QStringLiteral( "mParentConnectionParameterName" ) ).toString();
  mParentSchemaParameterName = map.value( QStringLiteral( "mParentSchemaParameterName" ) ).toString();
  mAllowNewTableNames = map.value( QStringLiteral( "mAllowNewTableNames" ), false ).toBool();
  return true;
}

QgsProcessingParameterDatabaseTable *QgsProcessingParameterDatabaseTable::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString connection;
  QString schema;
  QString def = definition;

  const QRegularExpression re( QStringLiteral( "(.*?)\\s+(.*+)\\b\\s*(.*)$" ) );
  const QRegularExpressionMatch m = re.match( def );
  if ( m.hasMatch() )
  {
    connection = m.captured( 1 ).trimmed();
    if ( connection == QLatin1String( "none" ) )
      connection.clear();
    schema = m.captured( 2 ).trimmed();
    if ( schema == QLatin1String( "none" ) )
      schema.clear();
    def = m.captured( 3 );
  }

  return new QgsProcessingParameterDatabaseTable( name, description, connection, schema, def.isEmpty() ? QVariant() : def, isOptional );
}

bool QgsProcessingParameterDatabaseTable::allowNewTableNames() const
{
  return mAllowNewTableNames;
}

void QgsProcessingParameterDatabaseTable::setAllowNewTableNames( bool allowNewTableNames )
{
  mAllowNewTableNames = allowNewTableNames;
}

//
// QgsProcessingParameterPointCloudLayer
//

QgsProcessingParameterPointCloudLayer::QgsProcessingParameterPointCloudLayer( const QString &name, const QString &description,
    const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterPointCloudLayer::clone() const
{
  return new QgsProcessingParameterPointCloudLayer( *this );
}

bool QgsProcessingParameterPointCloudLayer::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext *context ) const
{
  if ( !v.isValid() )
    return mFlags & FlagOptional;

  QVariant var = v;

  if ( var.canConvert<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( qobject_cast< QgsPointCloudLayer * >( qvariant_cast<QObject *>( var ) ) )
    return true;

  if ( var.type() != QVariant::String || var.toString().isEmpty() )
    return mFlags & FlagOptional;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  // try to load as layer
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context, true, QgsProcessingUtils::LayerHint::PointCloud ) )
    return true;

  return false;
}

QString QgsProcessingParameterPointCloudLayer::valueAsPythonString( const QVariant &val, QgsProcessingContext &context ) const
{
  if ( !val.isValid() )
    return QStringLiteral( "None" );

  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsPointCloudLayer *layer = QgsProcessingParameters::parameterAsPointCloudLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::normalizeLayerSource( layer->source() ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QString QgsProcessingParameterPointCloudLayer::createFileFilter() const
{
  return QgsProviderRegistry::instance()->filePointCloudFilters() + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );
}

QgsProcessingParameterPointCloudLayer *QgsProcessingParameterPointCloudLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterPointCloudLayer( name, description,  definition.isEmpty() ? QVariant() : definition, isOptional );
}

//
// QgsProcessingParameterAnnotationLayer
//

QgsProcessingParameterAnnotationLayer::QgsProcessingParameterAnnotationLayer( const QString &name, const QString &description,
    const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterAnnotationLayer::clone() const
{
  return new QgsProcessingParameterAnnotationLayer( *this );
}

bool QgsProcessingParameterAnnotationLayer::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext *context ) const
{
  if ( !v.isValid() )
    return mFlags & FlagOptional;

  QVariant var = v;

  if ( var.canConvert<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == QgsProperty::StaticProperty )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( qobject_cast< QgsAnnotationLayer * >( qvariant_cast<QObject *>( var ) ) )
    return true;

  if ( var.type() != QVariant::String || var.toString().isEmpty() )
    return mFlags & FlagOptional;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  // try to load as layer
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context, true, QgsProcessingUtils::LayerHint::Annotation ) )
    return true;

  return false;
}

QString QgsProcessingParameterAnnotationLayer::valueAsPythonString( const QVariant &val, QgsProcessingContext &context ) const
{
  if ( !val.isValid() )
    return QStringLiteral( "None" );

  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsAnnotationLayer *layer = QgsProcessingParameters::parameterAsAnnotationLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( layer == context.project()->mainAnnotationLayer() ? QStringLiteral( "main" ) : layer->id() )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QgsProcessingParameterAnnotationLayer *QgsProcessingParameterAnnotationLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterAnnotationLayer( name, description,  definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterPointCloudDestination::QgsProcessingParameterPointCloudDestination( const QString &name, const QString &description, const QVariant &defaultValue, bool optional, bool createByDefault )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional, createByDefault )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterPointCloudDestination::clone() const
{
  return new QgsProcessingParameterPointCloudDestination( *this );
}

bool QgsProcessingParameterPointCloudDestination::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  QVariant var = input;
  if ( !var.isValid() )
    return mFlags & FlagOptional;

  if ( var.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.canConvert<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
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

QString QgsProcessingParameterPointCloudDestination::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return QStringLiteral( "None" );

  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
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

QgsProcessingOutputDefinition *QgsProcessingParameterPointCloudDestination::toOutputDefinition() const
{
  return new QgsProcessingOutputPointCloudLayer( name(), description() );
}

QString QgsProcessingParameterPointCloudDestination::defaultFileExtension() const
{
  if ( auto *lOriginalProvider = originalProvider() )
  {
    return lOriginalProvider->defaultPointCloudFileExtension();
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->defaultPointCloudFileExtension();
  }
  else
  {
    return QgsProcessingUtils::defaultPointCloudExtension();
  }
}

QString QgsProcessingParameterPointCloudDestination::createFileFilter() const
{
  const QStringList exts = supportedOutputPointCloudLayerExtensions();
  QStringList filters;
  for ( const QString &ext : exts )
  {
    filters << QObject::tr( "%1 files (*.%2)" ).arg( ext.toUpper(), ext.toLower() );
  }
  return filters.join( QLatin1String( ";;" ) ) + QStringLiteral( ";;" ) + QObject::tr( "All files (*.*)" );
}

QStringList QgsProcessingParameterPointCloudDestination::supportedOutputPointCloudLayerExtensions() const
{
  if ( auto *lOriginalProvider = originalProvider() )
  {
    return lOriginalProvider->supportedOutputPointCloudLayerExtensions();
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->supportedOutputPointCloudLayerExtensions();
  }
  else
  {
    QString ext = QgsProcessingUtils::defaultPointCloudExtension();
    return QStringList() << QObject::tr( "%1 files (*.%2)" ).arg( ext.toUpper(), ext.toLower() );
  }
}

QgsProcessingParameterPointCloudDestination *QgsProcessingParameterPointCloudDestination::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterPointCloudDestination( name, description, definition.isEmpty() ? QVariant() : definition, isOptional );
}
