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

#include <functional>
#include <memory>

#include "qgsannotationlayer.h"
#include "qgsapplication.h"
#include "qgsfileutils.h"
#include "qgslayoutmanager.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"
#include "qgspointcloudlayer.h"
#include "qgsprintlayout.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingoutputs.h"
#include "qgsprocessingparametertype.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingutils.h"
#include "qgsproviderregistry.h"
#include "qgsrasterfilewriter.h"
#include "qgsreferencedgeometry.h"
#include "qgssettings.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"
#include "qgsvariantutils.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

#include <QRegularExpression>

QVariant QgsProcessingFeatureSourceDefinition::toVariant() const
{
  QVariantMap map;
  map.insert( u"source"_s, source.toVariant() );
  map.insert( u"selected_only"_s, selectedFeaturesOnly );
  map.insert( u"feature_limit"_s, featureLimit );
  map.insert( u"filter"_s, filterExpression );
  map.insert( u"flags"_s, static_cast< int >( flags ) );
  map.insert( u"geometry_check"_s, static_cast< int >( geometryCheck ) );
  return map;
}

bool QgsProcessingFeatureSourceDefinition::loadVariant( const QVariantMap &map )
{
  source.loadVariant( map.value( u"source"_s ) );
  selectedFeaturesOnly = map.value( u"selected_only"_s, false ).toBool();
  featureLimit = map.value( u"feature_limit"_s, -1 ).toLongLong();
  filterExpression = map.value( u"filter"_s ).toString();
  flags = static_cast< Qgis::ProcessingFeatureSourceDefinitionFlags >( map.value( u"flags"_s, 0 ).toInt() );
  geometryCheck = static_cast< Qgis::InvalidGeometryCheck >( map.value( u"geometry_check"_s, static_cast< int >( Qgis::InvalidGeometryCheck::AbortOnInvalid ) ).toInt() );
  return true;
}

//
// QgsProcessingRasterLayerDefinition
//

QVariant QgsProcessingRasterLayerDefinition::toVariant() const
{
  QVariantMap map;
  map.insert( u"source"_s, source.toVariant() );
  map.insert( u"reference_scale"_s, referenceScale );
  map.insert( u"dpi"_s, dpi );
  return map;
}

bool QgsProcessingRasterLayerDefinition::loadVariant( const QVariantMap &map )
{
  source.loadVariant( map.value( u"source"_s ) );
  referenceScale = map.value( u"reference_scale"_s, 0 ).toDouble();
  dpi = map.value( u"dpi"_s, 0 ).toInt();
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
  map.insert( u"sink"_s, sink.toVariant() );
  map.insert( u"create_options"_s, createOptions );
  if ( mUseRemapping )
    map.insert( u"remapping"_s, QVariant::fromValue( mRemappingDefinition ) );
  return map;
}

bool QgsProcessingOutputLayerDefinition::loadVariant( const QVariantMap &map )
{
  sink.loadVariant( map.value( u"sink"_s ) );
  createOptions = map.value( u"create_options"_s ).toMap();
  if ( map.contains( u"remapping"_s ) )
  {
    mUseRemapping = true;
    mRemappingDefinition = map.value( u"remapping"_s ).value< QgsRemappingSinkDefinition >();
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return val.value< QgsProperty >().propertyType() != Qgis::PropertyType::Static;
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );

  if ( !val.isValid() )
  {
    // fall back to default
    val = definition->defaultValue();
  }

  if ( val == QgsProcessing::TEMPORARY_OUTPUT )
  {
    if ( const QgsProcessingDestinationParameter *destParam = dynamic_cast< const QgsProcessingDestinationParameter * >( definition ) )
      return destParam->generateTemporaryDestination( &context );
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
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
    if ( val.userType() == qMetaTypeId<QgsProperty>() )
      resultList << val.value< QgsProperty >().valueAsInt( context.expressionContext(), definition->defaultValue().toInt() );
    else if ( val.userType() == QMetaType::Type::QVariantList )
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
      if ( definition->defaultValue().userType() == QMetaType::Type::QVariantList )
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    val = val.value< QgsProperty >().value( context.expressionContext(), definition->defaultValue() );

  QDateTime d = val.toDateTime();
  if ( !d.isValid() && val.userType() == QMetaType::Type::QString )
  {
    d = QDateTime::fromString( val.toString() );
  }

  if ( !d.isValid() )
  {
    // fall back to default
    val = definition->defaultValue();
    d = val.toDateTime();
  }
  if ( !d.isValid() && val.userType() == QMetaType::Type::QString )
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    val = val.value< QgsProperty >().value( context.expressionContext(), definition->defaultValue() );

  QDate d = val.toDate();
  if ( !d.isValid() && val.userType() == QMetaType::Type::QString )
  {
    d = QDate::fromString( val.toString() );
  }

  if ( !d.isValid() )
  {
    // fall back to default
    val = definition->defaultValue();
    d = val.toDate();
  }
  if ( !d.isValid() && val.userType() == QMetaType::Type::QString )
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    val = val.value< QgsProperty >().value( context.expressionContext(), definition->defaultValue() );

  QTime d;

  if ( val.userType() == QMetaType::Type::QDateTime )
    d = val.toDateTime().time();
  else
    d = val.toTime();

  if ( !d.isValid() && val.userType() == QMetaType::Type::QString )
  {
    d = QTime::fromString( val.toString() );
  }

  if ( !d.isValid() )
  {
    // fall back to default
    val = definition->defaultValue();
    d = val.toTime();
  }
  if ( !d.isValid() && val.userType() == QMetaType::Type::QString )
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    resultList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else if ( val.userType() == QMetaType::Type::QVariantList )
  {
    const auto constToList = val.toList();
    for ( const QVariant &var : constToList )
      resultList << var;
  }
  else if ( val.userType() == QMetaType::Type::QString )
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
    if ( definition->defaultValue().userType() == QMetaType::Type::QVariantList )
    {
      const auto constToList = definition->defaultValue().toList();
      for ( const QVariant &var : constToList )
        resultList << var;
    }
    else if ( definition->defaultValue().userType() == QMetaType::Type::QString )
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
  if ( const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( definition );
       enumDef && (
         enumText.isEmpty() || !enumDef->options().contains( enumText )
       )
     )
  {
    enumText = definition->defaultValue().toString();
  }

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
    if ( var.userType() == QMetaType::Type::QVariantList )
    {
      const auto constToList = var.toList();
      for ( const QVariant &listVar : constToList )
      {
        processVariant( listVar );
      }
    }
    else if ( var.userType() == QMetaType::Type::QStringList )
    {
      const auto constToStringList = var.toStringList();
      for ( const QString &s : constToStringList )
      {
        processVariant( s );
      }
    }
    else if ( var.userType() == qMetaTypeId<QgsProperty>() )
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

  if ( const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( definition ) )
  {
    // check that values are valid enum values. The resulting set will be empty
    // if all values are present in the enumDef->options(), otherwise it will contain
    // values which are invalid
    const QStringList options = enumDef->options();
    const QSet<QString> subtraction = QSet<QString>( enumValues.begin(), enumValues.end() ).subtract( QSet<QString>( options.begin(), options.end() ) );

    if ( enumValues.isEmpty() || !subtraction.isEmpty() )
    {
      enumValues.clear();
      // cppcheck-suppress invalidContainer
      processVariant( definition->defaultValue() );
    }
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return val.value< QgsProperty >().valueAsBool( context.expressionContext(), def.toBool() );
  else if ( val.isValid() )
    return val.toBool();
  else
    return def.toBool();
}

QgsFeatureSink *QgsProcessingParameters::parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsFields &fields,
    Qgis::WkbType geometryType, const QgsCoordinateReferenceSystem &crs,
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

QgsFeatureSink *QgsProcessingParameters::parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariant &value, const QgsFields &fields, Qgis::WkbType geometryType, const QgsCoordinateReferenceSystem &crs, QgsProcessingContext &context, QString &destinationIdentifier, QgsFeatureSink::SinkFlags sinkFlags, const QVariantMap &createOptions, const QStringList &datasourceOptions, const QStringList &layerOptions )
{
  QVariantMap options = createOptions;
  QVariant val = value;

  QgsProject *destinationProject = nullptr;
  QString destName;
  QgsRemappingSinkDefinition remapDefinition;
  bool useRemapDefinition = false;
  if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
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
  if ( definition && val.userType() == qMetaTypeId<QgsProperty>() )
  {
    dest = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }
  else if ( !val.isValid() || val.toString().isEmpty() )
  {
    if ( definition && definition->flags() & Qgis::ProcessingParameterFlag::Optional && !definition->defaultValue().isValid() )
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
      dest = destParam->generateTemporaryDestination( &context );
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
  QString filterExpression;
  if ( val.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    selectedFeaturesOnly = fromVar.selectedFeaturesOnly;
    featureLimit = fromVar.featureLimit;
    filterExpression = fromVar.filterExpression;
    val = fromVar.source;
  }
  else if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.userType() == qMetaTypeId<QgsProperty>() )
  {
    val = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }

  QgsVectorLayer *vl = nullptr;
  vl = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( val ) );

  if ( !vl )
  {
    QString layerRef;
    if ( val.userType() == qMetaTypeId<QgsProperty>() )
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
           compatibleFormats, preferredFormat, context, feedback, *layerName, featureLimit, filterExpression );
  else
    return QgsProcessingUtils::convertToCompatibleFormat( vl, selectedFeaturesOnly, definition->name(),
           compatibleFormats, preferredFormat, context, feedback, featureLimit, filterExpression );
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

QgsMapLayer *QgsProcessingParameters::parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingUtils::LayerHint layerHint, QgsProcessing::LayerOptionsFlags flags )
{
  if ( !definition )
    return nullptr;

  return parameterAsLayer( definition, parameters.value( definition->name() ), context, layerHint, flags );
}

QgsMapLayer *QgsProcessingParameters::parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, QgsProcessingUtils::LayerHint layerHint, QgsProcessing::LayerOptionsFlags flags )
{
  if ( !definition )
    return nullptr;

  QVariant val = value;
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
  {
    val = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }

  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) ) )
  {
    return layer;
  }

  if ( val.userType() == qMetaTypeId<QgsProcessingRasterLayerDefinition>() )
  {
    const QgsProcessingRasterLayerDefinition fromVar = qvariant_cast<QgsProcessingRasterLayerDefinition>( val );
    val = fromVar.source;
  }

  if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.userType() == qMetaTypeId<QgsProperty>() && val.value< QgsProperty >().propertyType() == Qgis::PropertyType::Static )
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

  return QgsProcessingUtils::mapLayerFromString( layerRef, context, true, layerHint, flags );
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

QString QgsProcessingParameters::parameterAsOutputFormat( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext & )
{
  QString format;
  QVariant val;
  if ( definition )
  {
    val = parameters.value( definition->name() );
    if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
    {
      // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
      const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
      format = fromVar.format();
    }
  }
  return format;
}

QString QgsProcessingParameters::parameterAsOutputLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, bool testOnly )
{
  QVariant val = value;

  QgsProject *destinationProject = nullptr;
  QString destName;
  if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    destinationProject = fromVar.destinationProject;
    val = fromVar.sink;
    destName = fromVar.destinationName;
  }

  QString dest;
  if ( definition && val.userType() == qMetaTypeId<QgsProperty>() )
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
      dest = destParam->generateTemporaryDestination( &context );
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
    else if ( definition && definition->type() == QgsProcessingParameterVectorTileDestination::typeName() )
      layerTypeHint = QgsProcessingUtils::LayerHint::VectorTile;

    if ( !testOnly )
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

  if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  QString dest;
  if ( definition && val.userType() == qMetaTypeId<QgsProperty>() )
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
      dest = destParam->generateTemporaryDestination( &context );
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

  if ( val.userType() == qMetaTypeId<QgsRectangle>() )
  {
    return val.value<QgsRectangle>();
  }
  if ( val.userType() == qMetaTypeId< QgsGeometry>() )
  {
    const QgsGeometry geom = val.value<QgsGeometry>();
    if ( !geom.isNull() )
      return geom.boundingBox();
  }
  if ( val.userType() == qMetaTypeId<QgsReferencedRectangle>() )
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

  if ( val.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.userType() == qMetaTypeId<QgsProperty>() && val.value< QgsProperty >().propertyType() == Qgis::PropertyType::Static )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  // maybe parameter is a direct layer value?
  QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) );

  QString rectText;
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    rectText = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else
    rectText = val.toString();

  if ( rectText.isEmpty() && !layer )
    return QgsRectangle();

  const thread_local QRegularExpression rx( u"^(.*?)\\s*,\\s*(.*?),\\s*(.*?),\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$"_s );
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

  if ( val.userType() == qMetaTypeId<QgsReferencedRectangle>() )
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

  if ( val.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.userType() == qMetaTypeId<QgsProperty>() && val.value< QgsProperty >().propertyType() == Qgis::PropertyType::Static )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  QString rectText;
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    rectText = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else
    rectText = val.toString();

  if ( !rectText.isEmpty() )
  {
    const thread_local QRegularExpression rx( u"^(.*?)\\s*,\\s*(.*?),\\s*(.*?),\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$"_s );
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
        else
        {
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
  if ( val.userType() == qMetaTypeId<QgsReferencedRectangle>() )
  {
    const QgsReferencedRectangle rr = val.value<QgsReferencedRectangle>();
    if ( rr.crs().isValid() )
    {
      return rr.crs();
    }
  }

  if ( val.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.userType() == qMetaTypeId<QgsProperty>() && val.value< QgsProperty >().propertyType() == Qgis::PropertyType::Static )
  {
    val = val.value< QgsProperty >().staticValue();
  }

  QString valueAsString;
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    valueAsString = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else
    valueAsString = val.toString();

  const thread_local QRegularExpression rx( u"^(.*?)\\s*,\\s*(.*?),\\s*(.*?),\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$"_s );

  const QRegularExpressionMatch match = rx.match( valueAsString );
  if ( match.hasMatch() )
  {
    const QgsCoordinateReferenceSystem crs( match.captured( 5 ) );
    if ( crs.isValid() )
      return crs;
  }

  if ( val.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    val = fromVar.source;
  }
  else if ( val.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    val = fromVar.sink;
  }

  if ( val.userType() == qMetaTypeId<QgsProperty>() && val.value< QgsProperty >().propertyType() == Qgis::PropertyType::Static )
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
  if ( val.userType() == qMetaTypeId<QgsPointXY>() )
  {
    return val.value<QgsPointXY>();
  }
  if ( val.userType() == qMetaTypeId< QgsGeometry>() )
  {
    const QgsGeometry geom = val.value<QgsGeometry>();
    if ( !geom.isNull() )
      return geom.centroid().asPoint();
  }
  if ( val.userType() == qMetaTypeId<QgsReferencedPointXY>() )
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

  const thread_local QRegularExpression rx( u"^\\s*\\(?\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*\\)?\\s*$"_s );

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
  if ( value.userType() == qMetaTypeId<QgsReferencedPointXY>() )
  {
    const QgsReferencedPointXY rr = value.value<QgsReferencedPointXY>();
    if ( rr.crs().isValid() )
    {
      return rr.crs();
    }
  }

  const thread_local QRegularExpression rx( u"^\\s*\\(?\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*\\)?\\s*$"_s );

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
  if ( val.userType() == qMetaTypeId< QgsGeometry>() )
  {
    return val.value<QgsGeometry>();
  }

  if ( val.userType() == qMetaTypeId<QgsPointXY>() )
  {
    return QgsGeometry::fromPointXY( val.value<QgsPointXY>() );
  }

  if ( val.userType() == qMetaTypeId<QgsRectangle>() )
  {
    return QgsGeometry::fromRect( val.value<QgsRectangle>() );
  }

  if ( val.userType() == qMetaTypeId<QgsReferencedPointXY>() )
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

  if ( val.userType() == qMetaTypeId<QgsReferencedRectangle>() )
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

  if ( val.userType() == qMetaTypeId<QgsReferencedGeometry>() )
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

  const thread_local QRegularExpression rx( u"^\\s*(?:CRS=(.*);)?(.*?)$"_s );

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
  if ( value.userType() == qMetaTypeId<QgsReferencedGeometry>() )
  {
    const QgsReferencedGeometry rg = value.value<QgsReferencedGeometry>();
    if ( rg.crs().isValid() )
    {
      return rg.crs();
    }
  }

  if ( value.userType() == qMetaTypeId<QgsReferencedPointXY>() )
  {
    const QgsReferencedPointXY rp = value.value<QgsReferencedPointXY>();
    if ( rp.crs().isValid() )
    {
      return rp.crs();
    }
  }

  if ( value.userType() == qMetaTypeId<QgsReferencedRectangle>() )
  {
    const QgsReferencedRectangle rr = value.value<QgsReferencedRectangle>();
    if ( rr.crs().isValid() )
    {
      return rr.crs();
    }
  }

  // Match against EWKT
  const QRegularExpression rx( u"^\\s*(?:CRS=(.*);)?(.*?)$"_s );

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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    resultString = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else if ( val.userType() == QMetaType::Type::QVariantList )
    return val.toList();
  else
    resultString = val.toString();

  if ( resultString.isEmpty() )
  {
    // check default
    if ( definition->defaultValue().userType() == QMetaType::Type::QVariantList )
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

QList<QgsMapLayer *> QgsProcessingParameters::parameterAsLayerList( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessing::LayerOptionsFlags flags )
{
  if ( !definition )
    return QList<QgsMapLayer *>();

  return parameterAsLayerList( definition, parameters.value( definition->name() ), context, flags );
}

QList<QgsMapLayer *> QgsProcessingParameters::parameterAsLayerList( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, QgsProcessing::LayerOptionsFlags flags )
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
  processVariant = [ &layers, &context, &definition, flags, &processVariant]( const QVariant & var )
  {
    if ( var.userType() == QMetaType::Type::QVariantList )
    {
      const auto constToList = var.toList();
      for ( const QVariant &listVar : constToList )
      {
        processVariant( listVar );
      }
    }
    else if ( var.userType() == QMetaType::Type::QStringList )
    {
      const auto constToStringList = var.toStringList();
      for ( const QString &s : constToStringList )
      {
        processVariant( s );
      }
    }
    else if ( var.userType() == qMetaTypeId<QgsProperty>() )
      processVariant( var.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() ) );
    else if ( var.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
    {
      // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
      const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
      const QVariant sink = fromVar.sink;
      if ( sink.userType() == qMetaTypeId<QgsProperty>() )
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
      QgsMapLayer *alayer = QgsProcessingUtils::mapLayerFromString( var.toString(), context, true, QgsProcessingUtils::LayerHint::UnknownType, flags );
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
    else if ( definition->defaultValue().userType() == QMetaType::Type::QVariantList )
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
    if ( var.userType() == QMetaType::Type::QVariantList )
    {
      const auto constToList = var.toList();
      for ( const QVariant &listVar : constToList )
      {
        processVariant( listVar );
      }
    }
    else if ( var.userType() == QMetaType::Type::QStringList )
    {
      const auto constToStringList = var.toStringList();
      for ( const QString &s : constToStringList )
      {
        processVariant( s );
      }
    }
    else if ( var.userType() == qMetaTypeId<QgsProperty>() )
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

  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else if ( val.userType() == QMetaType::Type::QVariantList )
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
    if ( definition->defaultValue().userType() == QMetaType::Type::QVariantList )
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

  return parameterAsStrings( definition, parameters.value( definition->name() ), context );
}

QStringList QgsProcessingParameters::parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  return parameterAsStrings( definition, value, context );
}

QStringList QgsProcessingParameters::parameterAsStrings( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  return parameterAsStrings( definition, parameters.value( definition->name() ), context );
}

QStringList QgsProcessingParameters::parameterAsStrings( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context )
{
  if ( !definition )
    return QStringList();

  QStringList resultStringList;
  const QVariant val = value;
  if ( val.isValid() )
  {
    if ( val.userType() == qMetaTypeId<QgsProperty>() )
      resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
    else if ( val.userType() == QMetaType::Type::QVariantList )
    {
      const auto constToList = val.toList();
      for ( const QVariant &var : constToList )
        resultStringList << var.toString();
    }
    else if ( val.userType() == QMetaType::Type::QStringList )
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
      if ( definition->defaultValue().userType() == QMetaType::Type::QVariantList )
      {
        const auto constToList = definition->defaultValue().toList();
        for ( const QVariant &var : constToList )
          resultStringList << var.toString();
      }
      else if ( definition->defaultValue().userType() == QMetaType::Type::QStringList )
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
  if ( val.userType() == qMetaTypeId<QgsProperty>() )
  {
    val = val.value< QgsProperty >().value( context.expressionContext(), definition->defaultValue() );
  }
  if ( val.userType() == QMetaType::Type::QColor )
  {
    QColor c = val.value< QColor >();
    if ( const QgsProcessingParameterColor *colorParam = dynamic_cast< const QgsProcessingParameterColor * >( definition ) )
      if ( !colorParam->opacityEnabled() )
        c.setAlpha( 255 );
    return c;
  }

  QString colorText = parameterAsString( definition, value, context );
  if ( colorText.isEmpty() && !( definition->flags() & Qgis::ProcessingParameterFlag::Optional ) )
  {
    if ( definition->defaultValue().userType() == QMetaType::Type::QColor )
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

QgsPointCloudLayer *QgsProcessingParameters::parameterAsPointCloudLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessing::LayerOptionsFlags flags )
{
  return qobject_cast< QgsPointCloudLayer *>( parameterAsLayer( definition, parameters, context, QgsProcessingUtils::LayerHint::PointCloud, flags ) );
}

QgsPointCloudLayer *QgsProcessingParameters::parameterAsPointCloudLayer( const QgsProcessingParameterDefinition *definition, const QVariant &value, QgsProcessingContext &context, QgsProcessing::LayerOptionsFlags flags )
{
  return qobject_cast< QgsPointCloudLayer *>( parameterAsLayer( definition, value, context, QgsProcessingUtils::LayerHint::PointCloud, flags ) );
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
  const QString type = map.value( u"parameter_type"_s ).toString();
  const QString name = map.value( u"name"_s ).toString();
  std::unique_ptr< QgsProcessingParameterDefinition > def;

  // probably all these hardcoded values aren't required anymore, and we could
  // always resort to the registry lookup...
  // TODO: confirm
  if ( type == QgsProcessingParameterBoolean::typeName() )
    def = std::make_unique<QgsProcessingParameterBoolean>( name );
  else if ( type == QgsProcessingParameterCrs::typeName() )
    def = std::make_unique<QgsProcessingParameterCrs>( name );
  else if ( type == QgsProcessingParameterMapLayer::typeName() )
    def = std::make_unique<QgsProcessingParameterMapLayer>( name );
  else if ( type == QgsProcessingParameterExtent::typeName() )
    def = std::make_unique<QgsProcessingParameterExtent>( name );
  else if ( type == QgsProcessingParameterPoint::typeName() )
    def = std::make_unique<QgsProcessingParameterPoint>( name );
  else if ( type == QgsProcessingParameterFile::typeName() )
    def = std::make_unique<QgsProcessingParameterFile>( name );
  else if ( type == QgsProcessingParameterMatrix::typeName() )
    def = std::make_unique<QgsProcessingParameterMatrix>( name );
  else if ( type == QgsProcessingParameterMultipleLayers::typeName() )
    def = std::make_unique<QgsProcessingParameterMultipleLayers>( name );
  else if ( type == QgsProcessingParameterNumber::typeName() )
    def = std::make_unique<QgsProcessingParameterNumber>( name );
  else if ( type == QgsProcessingParameterRange::typeName() )
    def = std::make_unique<QgsProcessingParameterRange>( name );
  else if ( type == QgsProcessingParameterRasterLayer::typeName() )
    def = std::make_unique<QgsProcessingParameterRasterLayer>( name );
  else if ( type == QgsProcessingParameterEnum::typeName() )
    def = std::make_unique<QgsProcessingParameterEnum>( name );
  else if ( type == QgsProcessingParameterString::typeName() )
    def = std::make_unique<QgsProcessingParameterString>( name );
  else if ( type == QgsProcessingParameterAuthConfig::typeName() )
    def = std::make_unique<QgsProcessingParameterAuthConfig>( name );
  else if ( type == QgsProcessingParameterExpression::typeName() )
    def = std::make_unique<QgsProcessingParameterExpression>( name );
  else if ( type == QgsProcessingParameterVectorLayer::typeName() )
    def = std::make_unique<QgsProcessingParameterVectorLayer>( name );
  else if ( type == QgsProcessingParameterField::typeName() )
    def = std::make_unique<QgsProcessingParameterField>( name );
  else if ( type == QgsProcessingParameterFeatureSource::typeName() )
    def = std::make_unique<QgsProcessingParameterFeatureSource>( name );
  else if ( type == QgsProcessingParameterFeatureSink::typeName() )
    def = std::make_unique<QgsProcessingParameterFeatureSink>( name );
  else if ( type == QgsProcessingParameterVectorDestination::typeName() )
    def = std::make_unique<QgsProcessingParameterVectorDestination>( name );
  else if ( type == QgsProcessingParameterRasterDestination::typeName() )
    def = std::make_unique<QgsProcessingParameterRasterDestination>( name );
  else if ( type == QgsProcessingParameterPointCloudDestination::typeName() )
    def = std::make_unique<QgsProcessingParameterPointCloudDestination>( name );
  else if ( type == QgsProcessingParameterFileDestination::typeName() )
    def = std::make_unique<QgsProcessingParameterFileDestination>( name );
  else if ( type == QgsProcessingParameterFolderDestination::typeName() )
    def = std::make_unique<QgsProcessingParameterFolderDestination>( name );
  else if ( type == QgsProcessingParameterBand::typeName() )
    def = std::make_unique<QgsProcessingParameterBand>( name );
  else if ( type == QgsProcessingParameterMeshLayer::typeName() )
    def = std::make_unique<QgsProcessingParameterMeshLayer>( name );
  else if ( type == QgsProcessingParameterLayout::typeName() )
    def = std::make_unique<QgsProcessingParameterLayout>( name );
  else if ( type == QgsProcessingParameterLayoutItem::typeName() )
    def = std::make_unique<QgsProcessingParameterLayoutItem>( name );
  else if ( type == QgsProcessingParameterColor::typeName() )
    def = std::make_unique<QgsProcessingParameterColor>( name );
  else if ( type == QgsProcessingParameterCoordinateOperation::typeName() )
    def = std::make_unique<QgsProcessingParameterCoordinateOperation>( name );
  else if ( type == QgsProcessingParameterPointCloudLayer::typeName() )
    def = std::make_unique<QgsProcessingParameterPointCloudLayer>( name );
  else if ( type == QgsProcessingParameterAnnotationLayer::typeName() )
    def = std::make_unique<QgsProcessingParameterAnnotationLayer>( name );
  else if ( type == QgsProcessingParameterPointCloudAttribute::typeName() )
    def = std::make_unique<QgsProcessingParameterPointCloudAttribute>( name );
  else if ( type == QgsProcessingParameterVectorTileDestination::typeName() )
    def = std::make_unique<QgsProcessingParameterVectorTileDestination>( name );
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

  if ( type == "boolean"_L1 )
    return QgsProcessingParameterBoolean::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "crs"_L1 )
    return QgsProcessingParameterCrs::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "layer"_L1 )
    return QgsProcessingParameterMapLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "extent"_L1 )
    return QgsProcessingParameterExtent::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "point"_L1 )
    return QgsProcessingParameterPoint::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "geometry"_L1 )
    return QgsProcessingParameterGeometry::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "file"_L1 )
    return QgsProcessingParameterFile::fromScriptCode( name, description, isOptional, definition, Qgis::ProcessingFileParameterBehavior::File );
  else if ( type == "folder"_L1 )
    return QgsProcessingParameterFile::fromScriptCode( name, description, isOptional, definition, Qgis::ProcessingFileParameterBehavior::Folder );
  else if ( type == "matrix"_L1 )
    return QgsProcessingParameterMatrix::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "multiple"_L1 )
    return QgsProcessingParameterMultipleLayers::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "number"_L1 )
    return QgsProcessingParameterNumber::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "distance"_L1 )
    return QgsProcessingParameterDistance::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "area"_L1 )
    return QgsProcessingParameterArea::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "volume"_L1 )
    return QgsProcessingParameterVolume::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "duration"_L1 )
    return QgsProcessingParameterDuration::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "scale"_L1 )
    return QgsProcessingParameterScale::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "range"_L1 )
    return QgsProcessingParameterRange::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "raster"_L1 )
    return QgsProcessingParameterRasterLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "enum"_L1 )
    return QgsProcessingParameterEnum::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "string"_L1 )
    return QgsProcessingParameterString::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "authcfg"_L1 )
    return QgsProcessingParameterAuthConfig::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "expression"_L1 )
    return QgsProcessingParameterExpression::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "field"_L1 )
    return QgsProcessingParameterField::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "vector"_L1 )
    return QgsProcessingParameterVectorLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "source"_L1 )
    return QgsProcessingParameterFeatureSource::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "sink"_L1 )
    return QgsProcessingParameterFeatureSink::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "vectordestination"_L1 )
    return QgsProcessingParameterVectorDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "rasterdestination"_L1 )
    return QgsProcessingParameterRasterDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "pointclouddestination"_L1 )
    return QgsProcessingParameterPointCloudDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "filedestination"_L1 )
    return QgsProcessingParameterFileDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "folderdestination"_L1 )
    return QgsProcessingParameterFolderDestination::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "band"_L1 )
    return QgsProcessingParameterBand::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "mesh"_L1 )
    return QgsProcessingParameterMeshLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "layout"_L1 )
    return QgsProcessingParameterLayout::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "layoutitem"_L1 )
    return QgsProcessingParameterLayoutItem::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "color"_L1 )
    return QgsProcessingParameterColor::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "coordinateoperation"_L1 )
    return QgsProcessingParameterCoordinateOperation::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "maptheme"_L1 )
    return QgsProcessingParameterMapTheme::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "datetime"_L1 )
    return QgsProcessingParameterDateTime::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "providerconnection"_L1 )
    return QgsProcessingParameterProviderConnection::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "databaseschema"_L1 )
    return QgsProcessingParameterDatabaseSchema::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "databasetable"_L1 )
    return QgsProcessingParameterDatabaseTable::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "pointcloud"_L1 )
    return QgsProcessingParameterPointCloudLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "annotation"_L1 )
    return QgsProcessingParameterAnnotationLayer::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "attribute"_L1 )
    return QgsProcessingParameterPointCloudAttribute::fromScriptCode( name, description, isOptional, definition );
  else if ( type == "vectortiledestination"_L1 )
    return QgsProcessingParameterVectorTileDestination::fromScriptCode( name, description, isOptional, definition );

  return nullptr;
}

bool QgsProcessingParameters::parseScriptCodeParameterOptions( const QString &code, bool &isOptional, QString &name, QString &type, QString &definition )
{
  const thread_local QRegularExpression re( u"(?:#*)(.*?)=\\s*(.*)"_s );
  QRegularExpressionMatch m = re.match( code );
  if ( !m.hasMatch() )
    return false;

  name = m.captured( 1 );
  QString tokens = m.captured( 2 );
  if ( tokens.startsWith( "optional"_L1, Qt::CaseInsensitive ) )
  {
    isOptional = true;
    tokens.remove( 0, 8 ); // length "optional" = 8
  }
  else
  {
    isOptional = false;
  }

  tokens = tokens.trimmed();

  const thread_local QRegularExpression re2( u"(.*?)\\s+(.*)"_s );
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
  , mFlags( optional ? Qgis::ProcessingParameterFlag::Optional : Qgis::ProcessingParameterFlag() )
{}

QVariant QgsProcessingParameterDefinition::guiDefaultValueOverride() const
{
  QVariant defaultSettingsValue = defaultGuiValueFromSetting();
  if ( defaultSettingsValue.isValid() )
  {
    return defaultSettingsValue;
  }
  return mGuiDefault;
}

QVariant QgsProcessingParameterDefinition::defaultValueForGui() const
{
  QVariant defaultSettingsValue = defaultGuiValueFromSetting();
  if ( defaultSettingsValue.isValid() )
  {
    return defaultSettingsValue;
  }
  return mGuiDefault.isValid() ? mGuiDefault : mDefault;
}

QVariant QgsProcessingParameterDefinition::defaultGuiValueFromSetting() const
{
  if ( mAlgorithm )
  {
    QgsSettings s;
    QVariant settingValue = s.value( u"/Processing/DefaultGuiParam/%1/%2"_s.arg( mAlgorithm->id() ).arg( mName ) );
    if ( settingValue.isValid() )
    {
      return settingValue;
    }
  }
  return QVariant();
}

bool QgsProcessingParameterDefinition::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() && !mDefault.isValid() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( ( input.userType() == QMetaType::Type::QString && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.userType() == QMetaType::Type::QString && mDefault.toString().isEmpty() ) )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterDefinition::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QVariant QgsProcessingParameterDefinition::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlags() );
}

QVariant QgsProcessingParameterDefinition::valueAsJsonObjectPrivate( const QVariant &value, QgsProcessingContext &context, ValueAsStringFlags flags ) const
{
  if ( !value.isValid() )
    return value;

  // dive into map and list types and convert each value
  if ( value.userType() == QMetaType::Type::QVariantMap )
  {
    const QVariantMap sourceMap = value.toMap();
    QVariantMap resultMap;
    for ( auto it = sourceMap.constBegin(); it != sourceMap.constEnd(); it++ )
    {
      resultMap[ it.key() ] = valueAsJsonObject( it.value(), context );
    }
    return resultMap;
  }
  else if ( value.userType() == QMetaType::Type::QVariantList || value.userType() == QMetaType::Type::QStringList )
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

    if ( value.userType() == qMetaTypeId<QgsProperty>() )
    {
      const QgsProperty prop = value.value< QgsProperty >();
      switch ( prop.propertyType() )
      {
        case Qgis::PropertyType::Invalid:
          return QVariant();
        case Qgis::PropertyType::Static:
          return valueAsJsonObject( prop.staticValue(), context );
        case Qgis::PropertyType::Field:
          return QVariantMap( {{u"type"_s, u"data_defined"_s}, {u"field"_s, prop.field() }} );
        case Qgis::PropertyType::Expression:
          return QVariantMap( {{u"type"_s, u"data_defined"_s}, {u"expression"_s, prop.expressionString() }} );
      }
    }

    // value may be a CRS
    if ( value.userType() == qMetaTypeId<QgsCoordinateReferenceSystem>() )
    {
      const QgsCoordinateReferenceSystem crs = value.value< QgsCoordinateReferenceSystem >();
      if ( !crs.isValid() )
        return QString();
      else if ( !crs.authid().isEmpty() )
        return crs.authid();
      else
        return crs.toWkt( Qgis::CrsWktVariant::Preferred );
    }
    else if ( value.userType() == qMetaTypeId<QgsRectangle>() )
    {
      const QgsRectangle r = value.value<QgsRectangle>();
      return u"%1, %3, %2, %4"_s.arg( qgsDoubleToString( r.xMinimum() ),
                                      qgsDoubleToString( r.yMinimum() ),
                                      qgsDoubleToString( r.xMaximum() ),
                                      qgsDoubleToString( r.yMaximum() ) );
    }
    else if ( value.userType() == qMetaTypeId<QgsReferencedRectangle>() )
    {
      const QgsReferencedRectangle r = value.value<QgsReferencedRectangle>();
      return u"%1, %3, %2, %4 [%5]"_s.arg( qgsDoubleToString( r.xMinimum() ),
                                           qgsDoubleToString( r.yMinimum() ),
                                           qgsDoubleToString( r.xMaximum() ),
                                           qgsDoubleToString( r.yMaximum() ),
                                           r.crs().authid() );
    }
    else if ( value.userType() == qMetaTypeId< QgsGeometry>() )
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
    else if ( value.userType() == qMetaTypeId<QgsReferencedGeometry>() )
    {
      const QgsReferencedGeometry g = value.value<QgsReferencedGeometry>();
      if ( !g.isNull() )
      {
        if ( !g.crs().isValid() )
          return g.asWkt();
        else
          return u"CRS=%1;%2"_s.arg( g.crs().authid().isEmpty() ? g.crs().toWkt( Qgis::CrsWktVariant::Preferred ) : g.crs().authid(), g.asWkt() );
      }
      else
      {
        return QString();
      }
    }
    else if ( value.userType() == qMetaTypeId<QgsPointXY>() )
    {
      const QgsPointXY r = value.value<QgsPointXY>();
      return u"%1,%2"_s.arg( qgsDoubleToString( r.x() ),
                             qgsDoubleToString( r.y() ) );
    }
    else if ( value.userType() == qMetaTypeId<QgsReferencedPointXY>() )
    {
      const QgsReferencedPointXY r = value.value<QgsReferencedPointXY>();
      return u"%1,%2 [%3]"_s.arg( qgsDoubleToString( r.x() ),
                                  qgsDoubleToString( r.y() ),
                                  r.crs().authid() );
    }
    else if ( value.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
    {
      const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );

      // TODO -- we could consider also serializating the additional properties like invalid feature handling, limits, etc
      return valueAsJsonObject( fromVar.source, context );
    }
    else if ( value.userType() == qMetaTypeId<QgsProcessingRasterLayerDefinition>() )
    {
      const QgsProcessingRasterLayerDefinition fromVar = qvariant_cast<QgsProcessingRasterLayerDefinition>( value );

      // TODO -- we could consider also serializating the additional properties like reference scale, dpi, etc
      return valueAsJsonObject( fromVar.source, context );
    }
    else if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
    {
      const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
      return valueAsJsonObject( fromVar.sink, context );
    }
    else if ( value.userType() == qMetaTypeId<QColor>() )
    {
      const QColor fromVar = value.value< QColor >();
      if ( !fromVar.isValid() )
        return QString();

      return u"rgba( %1, %2, %3, %4 )"_s.arg( fromVar.red() ).arg( fromVar.green() ).arg( fromVar.blue() ).arg( QString::number( fromVar.alphaF(), 'f', 2 ) );
    }
    else if ( value.userType() == qMetaTypeId<QDateTime>() )
    {
      const QDateTime fromVar = value.toDateTime();
      if ( !fromVar.isValid() )
        return QString();

      return fromVar.toString( Qt::ISODate );
    }
    else if ( value.userType() == qMetaTypeId<QDate>() )
    {
      const QDate fromVar = value.toDate();
      if ( !fromVar.isValid() )
        return QString();

      return fromVar.toString( Qt::ISODate );
    }
    else if ( value.userType() == qMetaTypeId<QTime>() )
    {
      const QTime fromVar = value.toTime();
      if ( !fromVar.isValid() )
        return QString();

      return fromVar.toString( Qt::ISODate );
    }

    if ( flags & ValueAsStringFlag::AllowMapLayerValues )
    {
      // value may be a map layer
      QVariantMap p;
      p.insert( name(), value );
      if ( QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context ) )
      {
        return QgsProcessingUtils::layerToStringIdentifier( layer, value.toString() );
      }
    }

    // now we handle strings, after any other specific logic has already been applied
    if ( value.userType() == QMetaType::QString )
      return value;
  }

  // unhandled type
  Q_ASSERT_X( false, "QgsProcessingParameterDefinition::valueAsJsonObject", u"unsupported variant type %1"_s.arg( QMetaType::typeName( value.userType() ) ).toLocal8Bit() );
  return value;
}

QString QgsProcessingParameterDefinition::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlags() );
}

QString QgsProcessingParameterDefinition::valueAsStringPrivate( const QVariant &value, QgsProcessingContext &context, bool &ok, ValueAsStringFlags flags ) const
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

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty prop = value.value< QgsProperty >();
    switch ( prop.propertyType() )
    {
      case Qgis::PropertyType::Invalid:
        return QString();
      case Qgis::PropertyType::Static:
        return valueAsString( prop.staticValue(), context, ok );
      case Qgis::PropertyType::Field:
        return u"field:%1"_s.arg( prop.field() );
      case Qgis::PropertyType::Expression:
        return u"expression:%1"_s.arg( prop.expressionString() );
    }
  }

  // value may be a CRS
  if ( value.userType() == qMetaTypeId<QgsCoordinateReferenceSystem>() )
  {
    const QgsCoordinateReferenceSystem crs = value.value< QgsCoordinateReferenceSystem >();
    if ( !crs.isValid() )
      return QString();
    else if ( !crs.authid().isEmpty() )
      return crs.authid();
    else
      return crs.toWkt( Qgis::CrsWktVariant::Preferred );
  }
  else if ( value.userType() == qMetaTypeId<QgsRectangle>() )
  {
    const QgsRectangle r = value.value<QgsRectangle>();
    return u"%1, %3, %2, %4"_s.arg( qgsDoubleToString( r.xMinimum() ),
                                    qgsDoubleToString( r.yMinimum() ),
                                    qgsDoubleToString( r.xMaximum() ),
                                    qgsDoubleToString( r.yMaximum() ) );
  }
  else if ( value.userType() == qMetaTypeId<QgsReferencedRectangle>() )
  {
    const QgsReferencedRectangle r = value.value<QgsReferencedRectangle>();
    return u"%1, %3, %2, %4 [%5]"_s.arg( qgsDoubleToString( r.xMinimum() ),
                                         qgsDoubleToString( r.yMinimum() ),
                                         qgsDoubleToString( r.xMaximum() ),
                                         qgsDoubleToString( r.yMaximum() ), r.crs().authid() );
  }
  else if ( value.userType() == qMetaTypeId< QgsGeometry>() )
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
  else if ( value.userType() == qMetaTypeId<QgsReferencedGeometry>() )
  {
    const QgsReferencedGeometry g = value.value<QgsReferencedGeometry>();
    if ( !g.isNull() )
    {
      if ( !g.crs().isValid() )
        return g.asWkt();
      else
        return u"CRS=%1;%2"_s.arg( g.crs().authid().isEmpty() ? g.crs().toWkt( Qgis::CrsWktVariant::Preferred ) : g.crs().authid(), g.asWkt() );
    }
    else
    {
      return QString();
    }
  }
  else if ( value.userType() == qMetaTypeId<QgsPointXY>() )
  {
    const QgsPointXY r = value.value<QgsPointXY>();
    return u"%1,%2"_s.arg( qgsDoubleToString( r.x() ),
                           qgsDoubleToString( r.y() ) );
  }
  else if ( value.userType() == qMetaTypeId<QgsReferencedPointXY>() )
  {
    const QgsReferencedPointXY r = value.value<QgsReferencedPointXY>();
    return u"%1,%2 [%3]"_s.arg( qgsDoubleToString( r.x() ),
                                qgsDoubleToString( r.y() ),
                                r.crs().authid() );
  }
  else if ( value.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );
    return valueAsString( fromVar.source, context, ok );
  }
  else if ( value.userType() == qMetaTypeId<QgsProcessingRasterLayerDefinition>() )
  {
    const QgsProcessingRasterLayerDefinition fromVar = qvariant_cast<QgsProcessingRasterLayerDefinition>( value );
    return valueAsString( fromVar.source, context, ok );
  }
  else if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    return valueAsString( fromVar.sink, context, ok );
  }
  else if ( value.userType() == qMetaTypeId<QColor>() )
  {
    const QColor fromVar = value.value< QColor >();
    if ( !fromVar.isValid() )
      return QString();

    return u"rgba( %1, %2, %3, %4 )"_s.arg( fromVar.red() ).arg( fromVar.green() ).arg( fromVar.blue() ).arg( QString::number( fromVar.alphaF(), 'f', 2 ) );
  }
  else if ( value.userType() == qMetaTypeId<QDateTime>() )
  {
    const QDateTime fromVar = value.toDateTime();
    if ( !fromVar.isValid() )
      return QString();

    return fromVar.toString( Qt::ISODate );
  }
  else if ( value.userType() == qMetaTypeId<QDate>() )
  {
    const QDate fromVar = value.toDate();
    if ( !fromVar.isValid() )
      return QString();

    return fromVar.toString( Qt::ISODate );
  }
  else if ( value.userType() == qMetaTypeId<QTime>() )
  {
    const QTime fromVar = value.toTime();
    if ( !fromVar.isValid() )
      return QString();

    return fromVar.toString( Qt::ISODate );
  }

  if ( flags & ValueAsStringFlag::AllowMapLayerValues )
  {
    // value may be a map layer
    QVariantMap p;
    p.insert( name(), value );
    if ( QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context ) )
    {
      return QgsProcessingUtils::layerToStringIdentifier( layer, value.toString() );
    }
  }

  // now we handle strings, after any other specific logic has already been applied
  if ( value.userType() == QMetaType::QString )
    return value.toString();

  // unhandled type
  QgsDebugError( u"unsupported variant type %1"_s.arg( QMetaType::typeName( value.userType() ) ) );
  ok = false;
  return value.toString();
}

QStringList QgsProcessingParameterDefinition::valueAsStringList( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  ok = true;
  if ( !value.isValid( ) )
    return QStringList();

  if ( value.userType() == QMetaType::Type::QVariantList || value.userType() == QMetaType::Type::QStringList )
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
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
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
      case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
      {
        QString code = t->className() + u"('%1', %2"_s
                       .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
        if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
          code += ", optional=True"_L1;

        QgsProcessingContext c;
        code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  map.insert( u"parameter_type"_s, type() );
  map.insert( u"name"_s, mName );
  map.insert( u"description"_s, mDescription );
  map.insert( u"help"_s, mHelp );
  map.insert( u"default"_s, mDefault );
  map.insert( u"defaultGui"_s, mGuiDefault );
  map.insert( u"flags"_s, static_cast< int >( mFlags ) );
  map.insert( u"metadata"_s, mMetadata );
  return map;
}

bool QgsProcessingParameterDefinition::fromVariantMap( const QVariantMap &map )
{
  mName = map.value( u"name"_s ).toString();
  mDescription = map.value( u"description"_s ).toString();
  mHelp = map.value( u"help"_s ).toString();
  mDefault = map.value( u"default"_s );
  mGuiDefault = map.value( u"defaultGui"_s );
  mFlags = static_cast< Qgis::ProcessingParameterFlags >( map.value( u"flags"_s ).toInt() );
  mMetadata = map.value( u"metadata"_s ).toMap();
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
  QString text = u"<p><b>%1</b></p>"_s.arg( description() );
  if ( !help().isEmpty() )
  {
    text += u"<p>%1</p>"_s.arg( help() );
  }
  text += u"<p>%1</p>"_s.arg( QObject::tr( "Python identifier: ‘%1’" ).arg( u"<i>%1</i>"_s.arg( name() ) ) );
  return text;
}

QgsProcessingParameterBoolean::QgsProcessingParameterBoolean( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{}

QgsProcessingParameterDefinition *QgsProcessingParameterBoolean::clone() const
{
  return new QgsProcessingParameterBoolean( *this );
}

QColor QgsProcessingParameterDefinition::modelColor() const
{
  QgsProcessingParameterType *paramType = QgsApplication::processingRegistry()->parameterType( type() );
  if ( paramType )
  {
    return paramType->modelColor();
  }

  return QgsProcessingParameterType::defaultModelColor();
}

QString QgsProcessingParameterDefinition::userFriendlyString( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  if ( value.userType() == qMetaTypeId<QgsPointXY>() )
  {
    const QgsPointXY r = value.value<QgsPointXY>();
    return u"%1, %2"_s.arg( qgsDoubleToString( r.x(), 4 ),
                            qgsDoubleToString( r.y(), 4 ) );
  }

  else if ( value.userType() == qMetaTypeId<QgsReferencedPointXY>() )
  {
    const QgsReferencedPointXY r = value.value<QgsReferencedPointXY>();
    return u"%1, %2 [%3]"_s.arg(
             qgsDoubleToString( r.x(), 4 ),
             qgsDoubleToString( r.y(), 4 ),
             r.crs().authid()
           );
  }

  else if ( value.userType() == qMetaTypeId<QgsRectangle>() )
  {
    const QgsGeometry g = QgsGeometry::fromRect( value.value<QgsRectangle>() );
    return QgsWkbTypes::geometryDisplayString( g.type() );
  }

  else if ( value.userType() == qMetaTypeId<QgsReferencedRectangle>() )
  {
    const QgsReferencedGeometry g = QgsReferencedGeometry::fromReferencedRect( value.value<QgsReferencedRectangle>() );
    if ( !g.isNull() )
    {

      return u"%1 [%2]"_s.arg( QgsWkbTypes::geometryDisplayString( g.type() ), g.crs().userFriendlyIdentifier( Qgis::CrsIdentifierType::ShortString ) );
    }
    return QgsWkbTypes::geometryDisplayString( g.type() );
  }

  else if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == Qgis::PropertyType::Static )
    {
      return fromVar.sink.staticValue().toString();
    }
    else
    {
      return fromVar.sink.asExpression();
    }
  }

  return value.toString();
}


QString QgsProcessingParameterBoolean::valueAsPythonString( const QVariant &val, QgsProcessingContext & ) const
{
  if ( !val.isValid() )
    return u"None"_s;

  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( val.value< QgsProperty >().asExpression() );
  return val.toBool() ? u"True"_s : u"False"_s;
}

QString QgsProcessingParameterBoolean::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += type() + ' ';
  code += mDefault.toBool() ? u"true"_s : u"false"_s;
  return code.trimmed();
}

QgsProcessingParameterBoolean *QgsProcessingParameterBoolean::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterBoolean( name, description, definition.toLower().trimmed() != u"false"_s, isOptional );
}

QgsProcessingParameterCrs::QgsProcessingParameterCrs( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterCrs::clone() const
{
  return new QgsProcessingParameterCrs( *this );
}

bool QgsProcessingParameterCrs::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext * ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsCoordinateReferenceSystem>() )
  {
    return true;
  }
  else if ( input.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    return true;
  }
  else if ( input.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    return true;
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() == QVariant::String )
  {
    const QString string = input.toString();
    if ( string.compare( "ProjectCrs"_L1, Qt::CaseInsensitive ) == 0 )
      return true;

    const QgsCoordinateReferenceSystem crs( string );
    if ( crs.isValid() )
      return true;
  }

  // direct map layer value
  if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( input ) ) )
    return true;

  if ( input.userType() != QMetaType::Type::QString || input.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterCrs::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsCoordinateReferenceSystem>() )
  {
    if ( !value.value< QgsCoordinateReferenceSystem >().isValid() )
      return u"QgsCoordinateReferenceSystem()"_s;
    else
      return u"QgsCoordinateReferenceSystem('%1')"_s.arg( value.value< QgsCoordinateReferenceSystem >().authid() );
  }

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.type() == QVariant::String )
  {
    const QString string = value.toString();
    if ( string.compare( "ProjectCrs"_L1, Qt::CaseInsensitive ) == 0 )
      return QgsProcessingUtils::stringToPythonLiteral( string );

    const QgsCoordinateReferenceSystem crs( string );
    if ( crs.isValid() )
      return QgsProcessingUtils::stringToPythonLiteral( string );
  }

  QVariantMap p;
  p.insert( name(), value );
  QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
  if ( layer )
    return QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::layerToStringIdentifier( layer ) );

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QString QgsProcessingParameterCrs::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  if ( value.type() == QVariant::String )
  {
    const QString string = value.toString();
    if ( string.compare( "ProjectCrs"_L1, Qt::CaseInsensitive ) == 0 )
      return string;

    const QgsCoordinateReferenceSystem crs( string );
    if ( crs.isValid() )
      return string;
  }

  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterCrs::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( value.type() == QVariant::String )
  {
    const QString string = value.toString();
    if ( string.compare( "ProjectCrs"_L1, Qt::CaseInsensitive ) == 0 )
      return string;

    const QgsCoordinateReferenceSystem crs( string );
    if ( crs.isValid() )
      return string;
  }

  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QgsProcessingParameterCrs *QgsProcessingParameterCrs::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterCrs( name, description, definition.compare( "none"_L1, Qt::CaseInsensitive ) == 0 ? QVariant() : definition, isOptional );
}


QString QgsProcessingParameterCrs::userFriendlyString( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  QgsCoordinateReferenceSystem crs( value.toString() );
  if ( crs.isValid() )
    return crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::ShortString );

  return QObject::tr( "Invalid CRS" );
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

bool QgsProcessingParameterMapLayer::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext *context ) const
{
  QVariant input = v;

  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( input ) ) )
  {
    return true;
  }

  if ( input.userType() != QMetaType::Type::QString || input.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

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
    return u"None"_s;

  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::layerToStringIdentifier( layer ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QString QgsProcessingParameterMapLayer::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterMapLayer::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QString createAllMapLayerFileFilter()
{
  QStringList vectors = QgsProviderRegistry::instance()->fileVectorFilters().split( u";;"_s );
  const QStringList rasters = QgsProviderRegistry::instance()->fileRasterFilters().split( u";;"_s );
  for ( const QString &raster : rasters )
  {
    if ( !vectors.contains( raster ) )
      vectors << raster;
  }
  const QStringList meshFilters = QgsProviderRegistry::instance()->fileMeshFilters().split( u";;"_s );
  for ( const QString &mesh : meshFilters )
  {
    if ( !vectors.contains( mesh ) )
      vectors << mesh;
  }
  const QStringList pointCloudFilters = QgsProviderRegistry::instance()->filePointCloudFilters().split( u";;"_s );
  for ( const QString &pointCloud : pointCloudFilters )
  {
    if ( !vectors.contains( pointCloud ) )
      vectors << pointCloud;
  }
  vectors.removeAll( QObject::tr( "All files (*.*)" ) );
  std::sort( vectors.begin(), vectors.end() );

  return QObject::tr( "All files (*.*)" ) + u";;"_s + vectors.join( ";;"_L1 );
}

QString QgsProcessingParameterMapLayer::createFileFilter() const
{
  return createAllMapLayerFileFilter();
}

QString QgsProcessingParameterMapLayer::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "layer "_L1;

  for ( const int type : mDataTypes )
  {
    switch ( static_cast< Qgis::ProcessingSourceType >( type ) )
    {
      case Qgis::ProcessingSourceType::Vector:
        code += "table "_L1;
        break;

      case Qgis::ProcessingSourceType::VectorAnyGeometry:
        code += "hasgeometry "_L1;
        break;

      case Qgis::ProcessingSourceType::VectorPoint:
        code += "point "_L1;
        break;

      case Qgis::ProcessingSourceType::VectorLine:
        code += "line "_L1;
        break;

      case Qgis::ProcessingSourceType::VectorPolygon:
        code += "polygon "_L1;
        break;

      case Qgis::ProcessingSourceType::Raster:
        code += "raster "_L1;
        break;

      case Qgis::ProcessingSourceType::Mesh:
        code += "mesh "_L1;
        break;

      case Qgis::ProcessingSourceType::Plugin:
        code += "plugin "_L1;
        break;

      case Qgis::ProcessingSourceType::PointCloud:
        code += "pointcloud "_L1;
        break;

      case Qgis::ProcessingSourceType::Annotation:
        code += "annotation "_L1;
        break;

      case Qgis::ProcessingSourceType::VectorTile:
        code += "vectortile "_L1;
        break;

      case Qgis::ProcessingSourceType::TiledScene:
        code += "tiledscene "_L1;
        break;

      default:
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
    if ( def.startsWith( "table"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::Vector );
      def = def.mid( 6 );
      continue;
    }
    if ( def.startsWith( "hasgeometry"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::VectorAnyGeometry );
      def = def.mid( 12 );
      continue;
    }
    else if ( def.startsWith( "point"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::VectorPoint );
      def = def.mid( 6 );
      continue;
    }
    else if ( def.startsWith( "line"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::VectorLine );
      def = def.mid( 5 );
      continue;
    }
    else if ( def.startsWith( "polygon"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::VectorPolygon );
      def = def.mid( 8 );
      continue;
    }
    else if ( def.startsWith( "raster"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::Raster );
      def = def.mid( 7 );
      continue;
    }
    else if ( def.startsWith( "mesh"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::Mesh );
      def = def.mid( 5 );
      continue;
    }
    else if ( def.startsWith( "plugin"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::Plugin );
      def = def.mid( 7 );
      continue;
    }
    else if ( def.startsWith( "pointcloud"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::PointCloud );
      def = def.mid( 11 );
      continue;
    }
    else if ( def.startsWith( "annotation"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::Annotation );
      def = def.mid( 11 );
      continue;
    }
    else if ( def.startsWith( "vectortile"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::VectorTile );
      def = def.mid( 11 );
      continue;
    }
    else if ( def.startsWith( "tiledscene"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::TiledScene );
      def = def.mid( 11 );
      continue;
    }
    break;
  }

  return new QgsProcessingParameterMapLayer( name, description, def.isEmpty() ? QVariant() : def, isOptional, types );
}

QString QgsProcessingParameterMapLayer::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterMapLayer('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      QgsProcessingContext c;
      code += u", defaultValue=%1"_s.arg( valueAsPythonString( mDefault, c ) );

      if ( !mDataTypes.empty() )
      {
        QStringList options;
        options.reserve( mDataTypes.size() );
        for ( const int t : mDataTypes )
          options << u"QgsProcessing.%1"_s.arg( QgsProcessing::sourceTypeToString( static_cast< Qgis::ProcessingSourceType >( t ) ) );
        code += u", types=[%1])"_s.arg( options.join( ',' ) );
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
  map.insert( u"data_types"_s, types );
  return map;
}

bool QgsProcessingParameterMapLayer::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mDataTypes.clear();
  const QVariantList values = map.value( u"data_types"_s ).toList();
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

bool QgsProcessingParameterExtent::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext *context ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    return true;
  }
  else if ( input.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    return true;
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( input.userType() == qMetaTypeId<QgsRectangle>() )
  {
    const QgsRectangle r = input.value<QgsRectangle>();
    return !r.isNull();
  }
  if ( input.userType() == qMetaTypeId< QgsGeometry>() )
  {
    return true;
  }
  if ( input.userType() == qMetaTypeId<QgsReferencedRectangle>() )
  {
    const QgsReferencedRectangle r = input.value<QgsReferencedRectangle>();
    return !r.isNull();
  }

  // direct map layer value
  if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( input ) ) )
    return true;

  if ( input.userType() != QMetaType::Type::QString || input.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( variantIsValidStringForExtent( input ) )
    return true;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  // try as layer extent
  return QgsProcessingUtils::mapLayerFromString( input.toString(), *context );
}

bool QgsProcessingParameterExtent::variantIsValidStringForExtent( const QVariant &value )
{
  if ( value.userType() == QMetaType::Type::QString )
  {
    const thread_local QRegularExpression rx( u"^(.*?)\\s*,\\s*(.*?)\\s*,\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*$"_s );
    const QRegularExpressionMatch match = rx.match( value.toString() );
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
  }
  return false;
}

QString QgsProcessingParameterExtent::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == qMetaTypeId<QgsRectangle>() )
  {
    const QgsRectangle r = value.value<QgsRectangle>();
    return u"'%1, %3, %2, %4'"_s.arg( qgsDoubleToString( r.xMinimum() ),
                                      qgsDoubleToString( r.yMinimum() ),
                                      qgsDoubleToString( r.xMaximum() ),
                                      qgsDoubleToString( r.yMaximum() ) );
  }
  else if ( value.userType() == qMetaTypeId<QgsReferencedRectangle>() )
  {
    const QgsReferencedRectangle r = value.value<QgsReferencedRectangle>();
    return u"'%1, %3, %2, %4 [%5]'"_s.arg( qgsDoubleToString( r.xMinimum() ),
                                           qgsDoubleToString( r.yMinimum() ),
                                           qgsDoubleToString( r.xMaximum() ),
                                           qgsDoubleToString( r.yMaximum() ),
                                           r.crs().authid() );
  }
  else if ( value.userType() == qMetaTypeId< QgsGeometry>() )
  {
    const QgsGeometry g = value.value<QgsGeometry>();
    if ( !g.isNull() )
    {
      const QString wkt = g.asWkt();
      return u"QgsGeometry.fromWkt('%1')"_s.arg( wkt );
    }
  }
  else if ( variantIsValidStringForExtent( value ) )
  {
    return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
  }

  QVariantMap p;
  p.insert( name(), value );
  QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
  if ( layer )
    return QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::layerToStringIdentifier( layer ) );

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QString QgsProcessingParameterExtent::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  if ( variantIsValidStringForExtent( value ) )
  {
    return value.toString();
  }

  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterExtent::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( variantIsValidStringForExtent( value ) )
  {
    return value;
  }

  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
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

bool QgsProcessingParameterPoint::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext * ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( input.userType() == qMetaTypeId<QgsPointXY>() )
  {
    return true;
  }
  if ( input.userType() == qMetaTypeId<QgsReferencedPointXY>() )
  {
    return true;
  }
  if ( input.userType() == qMetaTypeId< QgsGeometry>() )
  {
    return true;
  }

  if ( input.userType() == QMetaType::Type::QString )
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
  }

  const thread_local QRegularExpression rx( u"^\\s*\\(?\\s*(.*?)\\s*,\\s*(.*?)\\s*(?:\\[(.*)\\])?\\s*\\)?\\s*$"_s );

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
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == qMetaTypeId<QgsPointXY>() )
  {
    const QgsPointXY r = value.value<QgsPointXY>();
    return u"'%1,%2'"_s.arg( qgsDoubleToString( r.x() ),
                             qgsDoubleToString( r.y() ) );
  }
  else if ( value.userType() == qMetaTypeId<QgsReferencedPointXY>() )
  {
    const QgsReferencedPointXY r = value.value<QgsReferencedPointXY>();
    return u"'%1,%2 [%3]'"_s.arg( qgsDoubleToString( r.x() ),
                                  qgsDoubleToString( r.y() ),
                                  r.crs().authid() );
  }
  else if ( value.userType() == qMetaTypeId< QgsGeometry>() )
  {
    const QgsGeometry g = value.value<QgsGeometry>();
    if ( !g.isNull() )
    {
      const QString wkt = g.asWkt();
      return u"QgsGeometry.fromWkt('%1')"_s.arg( wkt );
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

bool QgsProcessingParameterGeometry::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext * ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  const bool anyTypeAllowed = mGeomTypes.isEmpty() || mGeomTypes.contains( static_cast< int >( Qgis::GeometryType::Unknown ) );

  if ( input.userType() == qMetaTypeId< QgsGeometry>() )
  {
    return ( anyTypeAllowed || mGeomTypes.contains( static_cast< int >( input.value<QgsGeometry>().type() ) ) ) &&
           ( mAllowMultipart || !input.value<QgsGeometry>().isMultipart() );
  }

  if ( input.userType() == qMetaTypeId<QgsReferencedGeometry>() )
  {
    return ( anyTypeAllowed || mGeomTypes.contains( static_cast<int>( input.value<QgsReferencedGeometry>().type() ) ) ) &&
           ( mAllowMultipart || !input.value<QgsReferencedGeometry>().isMultipart() );
  }

  if ( input.userType() == qMetaTypeId<QgsPointXY>() )
  {
    return anyTypeAllowed || mGeomTypes.contains( static_cast< int >( Qgis::GeometryType::Point ) );
  }

  if ( input.userType() == qMetaTypeId<QgsRectangle>() )
  {
    return anyTypeAllowed || mGeomTypes.contains( static_cast< int >( Qgis::GeometryType::Polygon ) );
  }

  if ( input.userType() == qMetaTypeId<QgsReferencedPointXY>() )
  {
    return anyTypeAllowed || mGeomTypes.contains( static_cast< int >( Qgis::GeometryType::Point ) );
  }

  if ( input.userType() == qMetaTypeId<QgsReferencedRectangle>() )
  {
    return anyTypeAllowed || mGeomTypes.contains( static_cast< int >( Qgis::GeometryType::Polygon ) );
  }

  if ( input.userType() == QMetaType::Type::QString )
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
  }

  // Match against EWKT
  const thread_local QRegularExpression rx( u"^\\s*(?:CRS=(.*);)?(.*?)$"_s );

  const QRegularExpressionMatch match = rx.match( input.toString() );
  if ( match.hasMatch() )
  {
    const QgsGeometry g = QgsGeometry::fromWkt( match.captured( 2 ) );
    if ( ! g.isNull() )
    {
      return ( anyTypeAllowed || mGeomTypes.contains( static_cast< int >( g.type() ) ) ) && ( mAllowMultipart || !g.isMultipart() );
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
      return QgsProcessingUtils::stringToPythonLiteral( u"CRS=%1;%2"_s.arg( crs.authid().isEmpty() ? crs.toWkt( Qgis::CrsWktVariant::Preferred ) : crs.authid(), g.asWkt() ) );
  };

  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression(%1)"_s.arg( QgsProcessingUtils::stringToPythonLiteral( value.value< QgsProperty >().asExpression() ) );

  if ( value.userType() == qMetaTypeId< QgsGeometry>() )
  {
    const QgsGeometry g = value.value<QgsGeometry>();
    if ( !g.isNull() )
      return asPythonString( g );
  }

  if ( value.userType() == qMetaTypeId<QgsReferencedGeometry>() )
  {
    const QgsReferencedGeometry g = value.value<QgsReferencedGeometry>();
    if ( !g.isNull() )
      return asPythonString( g, g.crs() );
  }

  if ( value.userType() == qMetaTypeId<QgsPointXY>() )
  {
    const QgsGeometry g = QgsGeometry::fromPointXY( value.value<QgsPointXY>() );
    if ( !g.isNull() )
      return asPythonString( g );
  }

  if ( value.userType() == qMetaTypeId<QgsReferencedPointXY>() )
  {
    const QgsReferencedGeometry g = QgsReferencedGeometry::fromReferencedPointXY( value.value<QgsReferencedPointXY>() );
    if ( !g.isNull() )
      return asPythonString( g, g.crs() );
  }

  if ( value.userType() == qMetaTypeId<QgsRectangle>() )
  {
    const QgsGeometry g = QgsGeometry::fromRect( value.value<QgsRectangle>() );
    if ( !g.isNull() )
      return asPythonString( g );
  }

  if ( value.userType() == qMetaTypeId<QgsReferencedRectangle>() )
  {
    const QgsReferencedGeometry g = QgsReferencedGeometry::fromReferencedRect( value.value<QgsReferencedRectangle>() );
    if ( !g.isNull() )
      return asPythonString( g, g.crs() );
  }

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QString QgsProcessingParameterGeometry::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += type() + ' ';

  for ( const int type : mGeomTypes )
  {
    switch ( static_cast<Qgis::GeometryType>( type ) )
    {
      case Qgis::GeometryType::Point:
        code += "point "_L1;
        break;

      case Qgis::GeometryType::Line:
        code += "line "_L1;
        break;

      case Qgis::GeometryType::Polygon:
        code += "polygon "_L1;
        break;

      default:
        code += "unknown "_L1;
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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterGeometry('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      if ( !mGeomTypes.empty() )
      {
        auto geomTypeToString = []( Qgis::GeometryType t ) -> QString
        {
          switch ( t )
          {
            case Qgis::GeometryType::Point:
              return u"PointGeometry"_s;

            case Qgis::GeometryType::Line:
              return u"LineGeometry"_s;

            case Qgis::GeometryType::Polygon:
              return u"PolygonGeometry"_s;

            case Qgis::GeometryType::Unknown:
              return u"UnknownGeometry"_s;

            case Qgis::GeometryType::Null:
              return u"NullGeometry"_s;
          }
          return QString();
        };

        QStringList options;
        options.reserve( mGeomTypes.size() );
        for ( const int type : mGeomTypes )
        {
          options << u" QgsWkbTypes.%1"_s.arg( geomTypeToString( static_cast<Qgis::GeometryType>( type ) ) );
        }
        code += u", geometryTypes=[%1 ]"_s.arg( options.join( ',' ) );
      }

      if ( ! mAllowMultipart )
      {
        code += ", allowMultipart=False"_L1;
      }

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  map.insert( u"geometrytypes"_s, types );
  map.insert( u"multipart"_s, mAllowMultipart );
  return map;
}

bool QgsProcessingParameterGeometry::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mGeomTypes.clear();
  const QVariantList values = map.value( u"geometrytypes"_s ).toList();
  for ( const QVariant &val : values )
  {
    mGeomTypes << val.toInt();
  }
  mAllowMultipart = map.value( u"multipart"_s ).toBool();
  return true;
}

QgsProcessingParameterGeometry *QgsProcessingParameterGeometry::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterGeometry( name, description, definition, isOptional );
}

QString QgsProcessingParameterGeometry::userFriendlyString( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  if ( value.isValid() )
  {

    if ( value.userType() == qMetaTypeId< QgsGeometry>() )
    {
      const QgsGeometry g = value.value<QgsGeometry>();
      return QgsWkbTypes::geometryDisplayString( g.type() );
    }

    else if ( value.userType() == qMetaTypeId<QgsReferencedGeometry>() )
    {
      const QgsReferencedGeometry g = value.value<QgsReferencedGeometry>();
      if ( !g.isNull() )
      {
        return u"%1 [%2]"_s.arg( QgsWkbTypes::geometryDisplayString( g.type() ), g.crs().userFriendlyIdentifier( Qgis::CrsIdentifierType::ShortString ) );
      }
      return QgsWkbTypes::geometryDisplayString( g.type() );
    }

    else if ( value.userType() == QMetaType::QString )
    {
      // In the case of a WKT-(string) encoded geometry, the type of geometry is going to be displayed
      // rather than the possibly very long WKT payload
      QgsGeometry g = QgsGeometry::fromWkt( value.toString() );
      if ( !g.isNull() )
      {
        return QgsWkbTypes::geometryDisplayString( g.type() );
      }
    }
  }

  return QObject::tr( "Invalid geometry" );
}

QgsProcessingParameterFile::QgsProcessingParameterFile( const QString &name, const QString &description, Qgis::ProcessingFileParameterBehavior behavior, const QString &extension, const QVariant &defaultValue, bool optional, const QString &fileFilter )
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

bool QgsProcessingParameterFile::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext * ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  const QString string = input.toString().trimmed();

  if ( input.userType() != QMetaType::Type::QString || string.isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  switch ( mBehavior )
  {
    case Qgis::ProcessingFileParameterBehavior::File:
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

    case Qgis::ProcessingFileParameterBehavior::Folder:
      return true;
  }
  return true;
}

QString QgsProcessingParameterFile::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += ( mBehavior == Qgis::ProcessingFileParameterBehavior::File ? u"file"_s : u"folder"_s ) + ' ';
  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterFile::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {

      QString code = u"QgsProcessingParameterFile('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;
      code += u", behavior=%1"_s.arg( mBehavior == Qgis::ProcessingFileParameterBehavior::File ? u"QgsProcessingParameterFile.File"_s : u"QgsProcessingParameterFile.Folder"_s );
      if ( !mExtension.isEmpty() )
        code += u", extension='%1'"_s.arg( mExtension );
      if ( !mFileFilter.isEmpty() )
        code += u", fileFilter='%1'"_s.arg( mFileFilter );
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterFile::createFileFilter() const
{
  switch ( mBehavior )
  {
    case Qgis::ProcessingFileParameterBehavior::File:
    {
      if ( !mFileFilter.isEmpty() )
        return mFileFilter != QObject::tr( "All files (*.*)" ) ? mFileFilter + u";;"_s + QObject::tr( "All files (*.*)" ) : mFileFilter;
      else if ( !mExtension.isEmpty() )
        return QObject::tr( "%1 files" ).arg( mExtension.toUpper() ) + u" (*."_s + mExtension.toLower() +  u");;"_s + QObject::tr( "All files (*.*)" );
      else
        return QObject::tr( "All files (*.*)" );
    }

    case Qgis::ProcessingFileParameterBehavior::Folder:
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
  map.insert( u"behavior"_s, static_cast< int >( mBehavior ) );
  map.insert( u"extension"_s, mExtension );
  map.insert( u"filefilter"_s, mFileFilter );
  return map;
}

bool QgsProcessingParameterFile::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mBehavior = static_cast< Qgis::ProcessingFileParameterBehavior >( map.value( u"behavior"_s ).toInt() );
  mExtension = map.value( u"extension"_s ).toString();
  mFileFilter = map.value( u"filefilter"_s ).toString();
  return true;
}

QgsProcessingParameterFile *QgsProcessingParameterFile::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition, Qgis::ProcessingFileParameterBehavior behavior )
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

bool QgsProcessingParameterMatrix::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext * ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == QMetaType::Type::QString )
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
    return true;
  }
  else if ( input.userType() == QMetaType::Type::QVariantList )
  {
    if ( input.toList().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
    return true;
  }
  else if ( input.userType() == QMetaType::Type::Double || input.userType() == QMetaType::Type::Int )
  {
    return true;
  }

  return false;
}

QString QgsProcessingParameterMatrix::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), value );
  const QVariantList list = QgsProcessingParameters::parameterAsMatrix( this, p, context );

  return QgsProcessingUtils::variantToPythonLiteral( list );
}

QString QgsProcessingParameterMatrix::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterMatrix('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;
      code += u", numberRows=%1"_s.arg( mNumberRows );
      code += u", hasFixedNumberRows=%1"_s.arg( mFixedNumberRows ? u"True"_s : u"False"_s );

      QStringList headers;
      headers.reserve( mHeaders.size() );
      for ( const QString &h : mHeaders )
        headers << QgsProcessingUtils::stringToPythonLiteral( h );
      code += u", headers=[%1]"_s.arg( headers.join( ',' ) );

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  map.insert( u"headers"_s, mHeaders );
  map.insert( u"rows"_s, mNumberRows );
  map.insert( u"fixed_number_rows"_s, mFixedNumberRows );
  return map;
}

bool QgsProcessingParameterMatrix::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mHeaders = map.value( u"headers"_s ).toStringList();
  mNumberRows = map.value( u"rows"_s ).toInt();
  mFixedNumberRows = map.value( u"fixed_number_rows"_s ).toBool();
  return true;
}

QgsProcessingParameterMatrix *QgsProcessingParameterMatrix::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterMatrix( name, description, 0, false, QStringList(), definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterMultipleLayers::QgsProcessingParameterMultipleLayers( const QString &name, const QString &description, Qgis::ProcessingSourceType layerType, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mLayerType( layerType )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterMultipleLayers::clone() const
{
  return new QgsProcessingParameterMultipleLayers( *this );
}

bool QgsProcessingParameterMultipleLayers::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext *context ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( mLayerType != Qgis::ProcessingSourceType::File )
  {
    if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( input ) ) )
    {
      return true;
    }
  }

  if ( input.userType() == QMetaType::Type::QString )
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    if ( mMinimumNumberInputs > 1 )
      return false;

    if ( !context )
      return true;

    if ( mLayerType != Qgis::ProcessingSourceType::File )
      return QgsProcessingUtils::mapLayerFromString( input.toString(), *context, true, QgsProcessingUtils::LayerHint::UnknownType, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
    else
      return true;
  }
  else if ( input.userType() == QMetaType::Type::QVariantList )
  {
    if ( input.toList().count() < mMinimumNumberInputs )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    if ( mMinimumNumberInputs > input.toList().count() )
      return false;

    if ( !context )
      return true;

    if ( mLayerType != Qgis::ProcessingSourceType::File )
    {
      const auto constToList = input.toList();
      for ( const QVariant &v : constToList )
      {
        if ( qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( v ) ) )
          continue;

        if ( !QgsProcessingUtils::mapLayerFromString( v.toString(), *context, true, QgsProcessingUtils::LayerHint::UnknownType, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration ) )
          return false;
      }
    }
    return true;
  }
  else if ( input.userType() == QMetaType::Type::QStringList )
  {
    if ( input.toStringList().count() < mMinimumNumberInputs )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    if ( mMinimumNumberInputs > input.toStringList().count() )
      return false;

    if ( !context )
      return true;

    if ( mLayerType != Qgis::ProcessingSourceType::File )
    {
      const auto constToStringList = input.toStringList();
      for ( const QString &v : constToStringList )
      {
        if ( !QgsProcessingUtils::mapLayerFromString( v, *context, true, QgsProcessingUtils::LayerHint::UnknownType, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration ) )
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
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( mLayerType == Qgis::ProcessingSourceType::File )
  {
    QStringList parts;
    if ( value.userType() == QMetaType::Type::QStringList )
    {
      const QStringList list = value.toStringList();
      parts.reserve( list.count() );
      for ( const QString &v : list )
        parts <<  QgsProcessingUtils::stringToPythonLiteral( v );
    }
    else if ( value.userType() == QMetaType::Type::QVariantList )
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
    const QList<QgsMapLayer *> list = QgsProcessingParameters::parameterAsLayerList( this, p, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
    if ( !list.isEmpty() )
    {
      QStringList parts;
      parts.reserve( list.count() );
      for ( const QgsMapLayer *layer : list )
      {
        parts << QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::layerToStringIdentifier( layer ) );
      }
      return parts.join( ',' ).prepend( '[' ).append( ']' );
    }
  }

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QString QgsProcessingParameterMultipleLayers::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterMultipleLayers::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QString QgsProcessingParameterMultipleLayers::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  switch ( mLayerType )
  {
    case Qgis::ProcessingSourceType::Raster:
      code += "multiple raster"_L1;
      break;

    case Qgis::ProcessingSourceType::File:
      code += "multiple file"_L1;
      break;

    default:
      code += "multiple vector"_L1;
      break;
  }
  code += ' ';
  if ( mDefault.userType() == QMetaType::Type::QVariantList )
  {
    QStringList parts;
    const auto constToList = mDefault.toList();
    for ( const QVariant &var : constToList )
    {
      parts << var.toString();
    }
    code += parts.join( ',' );
  }
  else if ( mDefault.userType() == QMetaType::Type::QStringList )
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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterMultipleLayers('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      const QString layerType = u"QgsProcessing.%1"_s.arg( QgsProcessing::sourceTypeToString( mLayerType ) );

      code += u", layerType=%1"_s.arg( layerType );
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterMultipleLayers::createFileFilter() const
{
  switch ( mLayerType )
  {
    case Qgis::ProcessingSourceType::File:
      return QObject::tr( "All files (*.*)" );

    case Qgis::ProcessingSourceType::Raster:
      return QgsProviderRegistry::instance()->fileRasterFilters() + u";;"_s + QObject::tr( "All files (*.*)" );

    case Qgis::ProcessingSourceType::Vector:
    case Qgis::ProcessingSourceType::VectorAnyGeometry:
    case Qgis::ProcessingSourceType::VectorPoint:
    case Qgis::ProcessingSourceType::VectorLine:
    case Qgis::ProcessingSourceType::VectorPolygon:
      return QgsProviderRegistry::instance()->fileVectorFilters() + u";;"_s + QObject::tr( "All files (*.*)" );

    case Qgis::ProcessingSourceType::Mesh:
      return QgsProviderRegistry::instance()->fileMeshFilters() + u";;"_s + QObject::tr( "All files (*.*)" );

    case Qgis::ProcessingSourceType::PointCloud:
      return QgsProviderRegistry::instance()->filePointCloudFilters() + u";;"_s + QObject::tr( "All files (*.*)" );

    case Qgis::ProcessingSourceType::MapLayer:
    case Qgis::ProcessingSourceType::Plugin:
    case Qgis::ProcessingSourceType::Annotation:
    case Qgis::ProcessingSourceType::VectorTile:
    case Qgis::ProcessingSourceType::TiledScene:
      return createAllMapLayerFileFilter();
  }
  return QString();
}

Qgis::ProcessingSourceType QgsProcessingParameterMultipleLayers::layerType() const
{
  return mLayerType;
}

void QgsProcessingParameterMultipleLayers::setLayerType( Qgis::ProcessingSourceType type )
{
  mLayerType = type;
}

int QgsProcessingParameterMultipleLayers::minimumNumberInputs() const
{
  return mMinimumNumberInputs;
}

void QgsProcessingParameterMultipleLayers::setMinimumNumberInputs( int minimumNumberInputs )
{
  if ( mMinimumNumberInputs >= 1 || !( flags() & Qgis::ProcessingParameterFlag::Optional ) )
    mMinimumNumberInputs = minimumNumberInputs;
}

QVariantMap QgsProcessingParameterMultipleLayers::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"layer_type"_s, static_cast< int >( mLayerType ) );
  map.insert( u"min_inputs"_s, mMinimumNumberInputs );
  return map;
}

bool QgsProcessingParameterMultipleLayers::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mLayerType = static_cast< Qgis::ProcessingSourceType >( map.value( u"layer_type"_s ).toInt() );
  mMinimumNumberInputs = map.value( u"min_inputs"_s ).toInt();
  return true;
}

QgsProcessingParameterMultipleLayers *QgsProcessingParameterMultipleLayers::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString type = definition;
  QString defaultVal;
  const thread_local QRegularExpression re( u"(.*?)\\s+(.*)"_s );
  const QRegularExpressionMatch m = re.match( definition );
  if ( m.hasMatch() )
  {
    type = m.captured( 1 ).toLower().trimmed();
    defaultVal = m.captured( 2 );
  }
  Qgis::ProcessingSourceType layerType = Qgis::ProcessingSourceType::VectorAnyGeometry;
  if ( type == "vector"_L1 )
    layerType = Qgis::ProcessingSourceType::VectorAnyGeometry;
  else if ( type == "raster"_L1 )
    layerType = Qgis::ProcessingSourceType::Raster;
  else if ( type == "file"_L1 )
    layerType = Qgis::ProcessingSourceType::File;
  return new QgsProcessingParameterMultipleLayers( name, description, layerType, defaultVal.isEmpty() ? QVariant() : defaultVal, isOptional );
}

QgsProcessingParameterNumber::QgsProcessingParameterNumber( const QString &name, const QString &description, Qgis::ProcessingNumberParameterType type, const QVariant &defaultValue, bool optional, double minValue, double maxValue )
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
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  bool ok = false;
  const double res = input.toDouble( &ok );
  if ( !ok )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return !( res < mMin || res > mMax );
}

QString QgsProcessingParameterNumber::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

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
    parts << QObject::tr( "Default value: %1" ).arg( mDataType == Qgis::ProcessingNumberParameterType::Integer ? mDefault.toInt() : mDefault.toDouble() );
  const QString extra = parts.join( "<br />"_L1 );
  if ( !extra.isEmpty() )
    text += u"<p>%1</p>"_s.arg( extra );
  return text;
}

QString QgsProcessingParameterNumber::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterNumber('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", type=%1"_s.arg( mDataType == Qgis::ProcessingNumberParameterType::Integer ? u"QgsProcessingParameterNumber.Integer"_s : u"QgsProcessingParameterNumber.Double"_s );

      if ( mMin != std::numeric_limits<double>::lowest() + 1 )
        code += u", minValue=%1"_s.arg( mMin );
      if ( mMax != std::numeric_limits<double>::max() )
        code += u", maxValue=%1"_s.arg( mMax );
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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

Qgis::ProcessingNumberParameterType QgsProcessingParameterNumber::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterNumber::setDataType( Qgis::ProcessingNumberParameterType dataType )
{
  mDataType = dataType;
}

QVariantMap QgsProcessingParameterNumber::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"min"_s, mMin );
  map.insert( u"max"_s, mMax );
  map.insert( u"data_type"_s, static_cast< int >( mDataType ) );
  return map;
}

bool QgsProcessingParameterNumber::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mMin = map.value( u"min"_s ).toDouble();
  mMax = map.value( u"max"_s ).toDouble();
  mDataType = static_cast< Qgis::ProcessingNumberParameterType >( map.value( u"data_type"_s ).toInt() );
  return true;
}

QgsProcessingParameterNumber *QgsProcessingParameterNumber::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterNumber( name, description, Qgis::ProcessingNumberParameterType::Double, definition.isEmpty() ? QVariant()
         : ( definition.toLower().trimmed() == "none"_L1 ? QVariant() : definition ), isOptional );
}

QgsProcessingParameterRange::QgsProcessingParameterRange( const QString &name, const QString &description, Qgis::ProcessingNumberParameterType type, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mDataType( type )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterRange::clone() const
{
  return new QgsProcessingParameterRange( *this );
}

bool QgsProcessingParameterRange::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext * ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( input.userType() == QMetaType::Type::QString )
  {
    const QStringList list = input.toString().split( ',' );
    if ( list.count() != 2 )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
    bool ok = false;
    list.at( 0 ).toDouble( &ok );
    bool ok2 = false;
    list.at( 1 ).toDouble( &ok2 );
    if ( !ok || !ok2 )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
    return true;
  }
  else if ( input.userType() == QMetaType::Type::QVariantList )
  {
    if ( input.toList().count() != 2 )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    bool ok = false;
    input.toList().at( 0 ).toDouble( &ok );
    bool ok2 = false;
    input.toList().at( 1 ).toDouble( &ok2 );
    if ( !ok || !ok2 )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
    return true;
  }

  return false;
}

QString QgsProcessingParameterRange::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterRange('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", type=%1"_s.arg( mDataType == Qgis::ProcessingNumberParameterType::Integer ? u"QgsProcessingParameterNumber.Integer"_s : u"QgsProcessingParameterNumber.Double"_s );

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

Qgis::ProcessingNumberParameterType QgsProcessingParameterRange::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterRange::setDataType( Qgis::ProcessingNumberParameterType dataType )
{
  mDataType = dataType;
}

QVariantMap QgsProcessingParameterRange::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"data_type"_s, static_cast< int >( mDataType ) );
  return map;
}

bool QgsProcessingParameterRange::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mDataType = static_cast< Qgis::ProcessingNumberParameterType >( map.value( u"data_type"_s ).toInt() );
  return true;
}

QgsProcessingParameterRange *QgsProcessingParameterRange::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterRange( name, description, Qgis::ProcessingNumberParameterType::Double, definition.isEmpty() ? QVariant()
                                          : ( definition.toLower().trimmed() == "none"_L1 ? QVariant() : definition ), isOptional );
}

QgsProcessingParameterRasterLayer::QgsProcessingParameterRasterLayer( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterRasterLayer::clone() const
{
  return new QgsProcessingParameterRasterLayer( *this );
}

bool QgsProcessingParameterRasterLayer::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext *context ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProcessingRasterLayerDefinition>() )
  {
    const QgsProcessingRasterLayerDefinition fromVar = qvariant_cast<QgsProcessingRasterLayerDefinition>( input );
    input = fromVar.source;
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = input.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
    {
      input = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( qobject_cast< QgsRasterLayer * >( qvariant_cast<QObject *>( input ) ) )
    return true;

  if ( input.userType() != QMetaType::Type::QString || input.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

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
    return u"None"_s;

  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( val.value< QgsProperty >().asExpression() );

  if ( val.userType() == qMetaTypeId<QgsProcessingRasterLayerDefinition>() )
  {
    const QgsProcessingRasterLayerDefinition fromVar = qvariant_cast<QgsProcessingRasterLayerDefinition>( val );

    if ( fromVar.source.propertyType() == Qgis::PropertyType::Static )
    {
      QString layerString = fromVar.source.staticValue().toString();
      // prefer to use layer source instead of id if possible (since it's persistent)
      if ( QgsRasterLayer *layer = qobject_cast< QgsRasterLayer * >( QgsProcessingUtils::mapLayerFromString( layerString, context, true, QgsProcessingUtils::LayerHint::Raster ) ) )
        layerString = layer->source();

      if ( fromVar.referenceScale > 0 )
      {
        return u"QgsProcessingRasterLayerDefinition(%1, referenceScale=%2, dpi=%3)"_s
               .arg( QgsProcessingUtils::stringToPythonLiteral( layerString ),
                     QString::number( fromVar.referenceScale ),
                     QString::number( fromVar.dpi ) );
      }
      else
      {
        return QgsProcessingUtils::stringToPythonLiteral( layerString );
      }
    }
    else
    {
      if ( fromVar.referenceScale > 0 )
      {
        return u"QgsProcessingRasterLayerDefinition(QgsProperty.fromExpression(%1), referenceScale=%2, dpi=%3)"_s
               .arg( QgsProcessingUtils::stringToPythonLiteral( fromVar.source.asExpression() ),
                     QString::number( fromVar.referenceScale ),
                     QString::number( fromVar.dpi ) );
      }
      else
      {
        return u"QgsProperty.fromExpression(%1)"_s.arg( QgsProcessingUtils::stringToPythonLiteral( fromVar.source.asExpression() ) );
      }
    }
  }

  QVariantMap p;
  p.insert( name(), val );
  QgsRasterLayer *layer = QgsProcessingParameters::parameterAsRasterLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::layerToStringIdentifier( layer ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QString QgsProcessingParameterRasterLayer::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterRasterLayer::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QString QgsProcessingParameterRasterLayer::createFileFilter() const
{
  return QgsProviderRegistry::instance()->fileRasterFilters() + u";;"_s + QObject::tr( "All files (*.*)" );
}

QgsProcessingParameterRasterLayer *QgsProcessingParameterRasterLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterRasterLayer( name, description, definition.isEmpty() ? QVariant() : definition, isOptional );
}

void QgsProcessingParameterRasterLayer::setParameterCapabilities( Qgis::RasterProcessingParameterCapabilities capabilities )
{
  mCapabilities = capabilities;
}

Qgis::RasterProcessingParameterCapabilities QgsProcessingParameterRasterLayer::parameterCapabilities() const
{
  return mCapabilities;
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
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( mUsesStaticStrings )
  {
    if ( input.userType() == QMetaType::Type::QVariantList )
    {
      if ( !mAllowMultiple )
        return false;

      const QVariantList values = input.toList();
      if ( values.empty() && !( mFlags & Qgis::ProcessingParameterFlag::Optional ) )
        return false;

      for ( const QVariant &val : values )
      {
        if ( !mOptions.contains( val.toString() ) )
          return false;
      }

      return true;
    }
    else if ( input.userType() == QMetaType::Type::QStringList )
    {
      if ( !mAllowMultiple )
        return false;

      const QStringList values = input.toStringList();

      if ( values.empty() && !( mFlags & Qgis::ProcessingParameterFlag::Optional ) )
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
    else if ( input.userType() == QMetaType::Type::QString )
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
    if ( input.userType() == QMetaType::Type::QVariantList )
    {
      if ( !mAllowMultiple )
        return false;

      const QVariantList values = input.toList();
      if ( values.empty() && !( mFlags & Qgis::ProcessingParameterFlag::Optional ) )
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
    else if ( input.userType() == QMetaType::Type::QString )
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
    else if ( input.userType() == QMetaType::Type::Int || input.userType() == QMetaType::Type::Double )
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
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( mUsesStaticStrings )
  {
    if ( value.userType() == QMetaType::Type::QVariantList || value.userType() == QMetaType::Type::QStringList )
    {
      QStringList parts;
      const QStringList constList = value.toStringList();
      for ( const QString &val : constList )
      {
        parts << QgsProcessingUtils::stringToPythonLiteral( val );
      }
      return parts.join( ',' ).prepend( '[' ).append( ']' );
    }
    else if ( value.userType() == QMetaType::Type::QString )
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
    if ( value.userType() == QMetaType::Type::QVariantList )
    {
      QStringList parts;
      const auto constToList = value.toList();
      for ( const QVariant &val : constToList )
      {
        parts << QString::number( static_cast< int >( val.toDouble() ) );
      }
      return parts.join( ',' ).prepend( '[' ).append( ']' );
    }
    else if ( value.userType() == QMetaType::Type::QString )
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

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return QString();

  if ( mUsesStaticStrings )
  {
    return QString();
  }
  else
  {
    if ( value.userType() == QMetaType::Type::QVariantList )
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
    else if ( value.userType() == QMetaType::Type::QString )
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
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "enum "_L1;

  if ( mAllowMultiple )
    code += "multiple "_L1;

  if ( mUsesStaticStrings )
    code += "static "_L1;

  code += mOptions.join( ';' ) + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterEnum::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterEnum('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      QStringList options;
      options.reserve( mOptions.size() );
      for ( const QString &o : mOptions )
        options << QgsProcessingUtils::stringToPythonLiteral( o );
      code += u", options=[%1]"_s.arg( options.join( ',' ) );

      code += u", allowMultiple=%1"_s.arg( mAllowMultiple ? u"True"_s : u"False"_s );

      code += u", usesStaticStrings=%1"_s.arg( mUsesStaticStrings ? u"True"_s : u"False"_s );

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );

      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterEnum::userFriendlyString( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  return options().value( value.toInt() );
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
  map.insert( u"options"_s, mOptions );
  map.insert( u"allow_multiple"_s, mAllowMultiple );
  map.insert( u"uses_static_strings"_s, mUsesStaticStrings );
  return map;
}

bool QgsProcessingParameterEnum::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mOptions = map.value( u"options"_s ).toStringList();
  mAllowMultiple = map.value( u"allow_multiple"_s ).toBool();
  mUsesStaticStrings = map.value( u"uses_static_strings"_s ).toBool();
  return true;
}

QgsProcessingParameterEnum *QgsProcessingParameterEnum::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString defaultVal;
  QString def = definition;

  bool multiple = false;
  if ( def.startsWith( "multiple"_L1, Qt::CaseInsensitive ) )
  {
    multiple = true;
    def = def.mid( 9 );
  }

  bool staticStrings = false;
  if ( def.startsWith( "static"_L1, Qt::CaseInsensitive ) )
  {
    staticStrings = true;
    def = def.mid( 7 );
  }

  const thread_local QRegularExpression re( u"(.*)\\s+(.*?)$"_s );
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
  if ( QgsVariantUtils::isNull( value ) )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterString::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "string "_L1;

  if ( mMultiLine )
    code += "long "_L1;

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterString::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterString('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;
      code += u", multiLine=%1"_s.arg( mMultiLine ? u"True"_s : u"False"_s );

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  map.insert( u"multiline"_s, mMultiLine );
  return map;
}

bool QgsProcessingParameterString::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mMultiLine = map.value( u"multiline"_s ).toBool();
  return true;
}

QgsProcessingParameterString *QgsProcessingParameterString::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString def = definition;
  bool multiLine = false;
  if ( def.startsWith( "long"_L1, Qt::CaseInsensitive ) )
  {
    multiLine = true;
    def = def.mid( 5 );
  }

  if ( def.startsWith( '"' ) || def.startsWith( '\'' ) )
    def = def.mid( 1 );
  if ( def.endsWith( '"' ) || def.endsWith( '\'' ) )
    def.chop( 1 );

  QVariant defaultValue = def;
  if ( def == "None"_L1 )
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
    return u"None"_s;

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterAuthConfig::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "authcfg "_L1;

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
  if ( def == "None"_L1 )
    defaultValue = QVariant();

  return new QgsProcessingParameterAuthConfig( name, description, defaultValue, isOptional );
}


//
// QgsProcessingParameterExpression
//

QgsProcessingParameterExpression::QgsProcessingParameterExpression( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, bool optional, Qgis::ExpressionType type )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameterName( parentLayerParameterName )
  , mExpressionType( type )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterExpression::clone() const
{
  return new QgsProcessingParameterExpression( *this );
}

QString QgsProcessingParameterExpression::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterExpression('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", parentLayerParameterName='%1'"_s.arg( mParentLayerParameterName );

      QgsProcessingContext c;
      code += u", defaultValue=%1"_s.arg( valueAsPythonString( mDefault, c ) );


      switch ( mExpressionType )
      {
        case Qgis::ExpressionType::PointCloud:
          code += ", type=Qgis.ExpressionType.PointCloud)"_L1;
          break;
        case Qgis::ExpressionType::RasterCalculator:
          code += ", type=Qgis.ExpressionType.RasterCalculator)"_L1;
          break;
        default:
          code += QLatin1Char( ')' );
          break;
      }
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

Qgis::ExpressionType QgsProcessingParameterExpression::expressionType() const
{
  return mExpressionType;
}

void QgsProcessingParameterExpression::setExpressionType( Qgis::ExpressionType expressionType )
{
  mExpressionType = expressionType;
}

QVariantMap QgsProcessingParameterExpression::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"parent_layer"_s, mParentLayerParameterName );
  map.insert( u"expression_type"_s, static_cast< int >( mExpressionType ) );
  return map;
}

bool QgsProcessingParameterExpression::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( u"parent_layer"_s ).toString();
  mExpressionType = static_cast< Qgis::ExpressionType >( map.value( u"expression_type"_s ).toInt() );
  return true;
}

QgsProcessingParameterExpression *QgsProcessingParameterExpression::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterExpression( name, description, definition, QString(), isOptional, Qgis::ExpressionType::Qgis );
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
  QVariant var = v;
  if ( !var.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
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

  if ( var.userType() != QMetaType::Type::QString || var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

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
    return u"None"_s;

  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsVectorLayer *layer = QgsProcessingParameters::parameterAsVectorLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::layerToStringIdentifier( layer ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QString QgsProcessingParameterVectorLayer::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterVectorLayer::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QString QgsProcessingParameterVectorLayer::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterVectorLayer('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      if ( !mDataTypes.empty() )
      {
        QStringList options;
        for ( const int t : mDataTypes )
          options << u"QgsProcessing.%1"_s.arg( QgsProcessing::sourceTypeToString( static_cast< Qgis::ProcessingSourceType >( t ) ) );
        code += u", types=[%1]"_s.arg( options.join( ',' ) );
      }

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterVectorLayer::createFileFilter() const
{
  return QgsProviderRegistry::instance()->fileVectorFilters() + u";;"_s + QObject::tr( "All files (*.*)" );
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
  map.insert( u"data_types"_s, types );
  return map;
}

bool QgsProcessingParameterVectorLayer::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mDataTypes.clear();
  const QVariantList values = map.value( u"data_types"_s ).toList();
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
  QVariant var = v;

  if ( !var.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
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

  if ( var.userType() != QMetaType::Type::QString || var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

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
    return u"None"_s;

  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsMeshLayer *layer = QgsProcessingParameters::parameterAsMeshLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::layerToStringIdentifier( layer ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QString QgsProcessingParameterMeshLayer::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterMeshLayer::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QString QgsProcessingParameterMeshLayer::createFileFilter() const
{
  return QgsProviderRegistry::instance()->fileMeshFilters() + u";;"_s + QObject::tr( "All files (*.*)" );
}

QgsProcessingParameterMeshLayer *QgsProcessingParameterMeshLayer::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterMeshLayer( name, description,  definition.isEmpty() ? QVariant() : definition, isOptional );
}

QgsProcessingParameterField::QgsProcessingParameterField( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, Qgis::ProcessingFieldParameterDataType type, bool allowMultiple, bool optional, bool defaultToAllFields )
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

bool QgsProcessingParameterField::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext * ) const
{
  QVariant input = v;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( input.userType() == QMetaType::Type::QVariantList || input.userType() == QMetaType::Type::QStringList )
  {
    if ( !mAllowMultiple )
      return false;

    if ( input.toList().isEmpty() && !( mFlags & Qgis::ProcessingParameterFlag::Optional ) )
      return false;
  }
  else if ( input.userType() == QMetaType::Type::QString )
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    const QStringList parts = input.toString().split( ';' );
    if ( parts.count() > 1 && !mAllowMultiple )
      return false;
  }
  else
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
  }
  return true;
}

QString QgsProcessingParameterField::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == QMetaType::Type::QVariantList )
  {
    QStringList parts;
    const auto constToList = value.toList();
    for ( const QVariant &val : constToList )
    {
      parts << QgsProcessingUtils::stringToPythonLiteral( val.toString() );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else if ( value.userType() == QMetaType::Type::QStringList )
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
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "field "_L1;

  switch ( mDataType )
  {
    case Qgis::ProcessingFieldParameterDataType::Numeric:
      code += "numeric "_L1;
      break;

    case Qgis::ProcessingFieldParameterDataType::String:
      code += "string "_L1;
      break;

    case Qgis::ProcessingFieldParameterDataType::DateTime:
      code += "datetime "_L1;
      break;

    case Qgis::ProcessingFieldParameterDataType::Binary:
      code += "binary "_L1;
      break;

    case Qgis::ProcessingFieldParameterDataType::Boolean:
      code += "boolean "_L1;
      break;

    case Qgis::ProcessingFieldParameterDataType::Any:
      break;
  }

  if ( mAllowMultiple )
    code += "multiple "_L1;

  if ( mDefaultToAllFields )
    code += "default_to_all_fields "_L1;

  code += mParentLayerParameterName + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterField::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterField('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      QString dataType;
      switch ( mDataType )
      {
        case Qgis::ProcessingFieldParameterDataType::Any:
          dataType = u"QgsProcessingParameterField.Any"_s;
          break;

        case Qgis::ProcessingFieldParameterDataType::Numeric:
          dataType = u"QgsProcessingParameterField.Numeric"_s;
          break;

        case Qgis::ProcessingFieldParameterDataType::String:
          dataType = u"QgsProcessingParameterField.String"_s;
          break;

        case Qgis::ProcessingFieldParameterDataType::DateTime:
          dataType = u"QgsProcessingParameterField.DateTime"_s;
          break;

        case Qgis::ProcessingFieldParameterDataType::Binary:
          dataType = u"QgsProcessingParameterField.Binary"_s;
          break;

        case Qgis::ProcessingFieldParameterDataType::Boolean:
          dataType = u"QgsProcessingParameterField.Boolean"_s;
          break;
      }
      code += u", type=%1"_s.arg( dataType );

      code += u", parentLayerParameterName='%1'"_s.arg( mParentLayerParameterName );
      code += u", allowMultiple=%1"_s.arg( mAllowMultiple ? u"True"_s : u"False"_s );
      QgsProcessingContext c;
      code += u", defaultValue=%1"_s.arg( valueAsPythonString( mDefault, c ) );

      if ( mDefaultToAllFields )
        code += ", defaultToAllFields=True"_L1;

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

Qgis::ProcessingFieldParameterDataType QgsProcessingParameterField::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterField::setDataType( Qgis::ProcessingFieldParameterDataType dataType )
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
  map.insert( u"parent_layer"_s, mParentLayerParameterName );
  map.insert( u"data_type"_s, static_cast< int >( mDataType ) );
  map.insert( u"allow_multiple"_s, mAllowMultiple );
  map.insert( u"default_to_all_fields"_s, mDefaultToAllFields );
  return map;
}

bool QgsProcessingParameterField::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( u"parent_layer"_s ).toString();
  mDataType = static_cast< Qgis::ProcessingFieldParameterDataType >( map.value( u"data_type"_s ).toInt() );
  mAllowMultiple = map.value( u"allow_multiple"_s ).toBool();
  mDefaultToAllFields = map.value( u"default_to_all_fields"_s ).toBool();
  return true;
}

QgsProcessingParameterField *QgsProcessingParameterField::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString parent;
  Qgis::ProcessingFieldParameterDataType type = Qgis::ProcessingFieldParameterDataType::Any;
  bool allowMultiple = false;
  bool defaultToAllFields = false;
  QString def = definition;

  if ( def.startsWith( "numeric "_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingFieldParameterDataType::Numeric;
    def = def.mid( 8 );
  }
  else if ( def.startsWith( "string "_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingFieldParameterDataType::String;
    def = def.mid( 7 );
  }
  else if ( def.startsWith( "datetime "_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingFieldParameterDataType::DateTime;
    def = def.mid( 9 );
  }
  else if ( def.startsWith( "binary "_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingFieldParameterDataType::Binary;
    def = def.mid( 7 );
  }
  else if ( def.startsWith( "boolean "_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingFieldParameterDataType::Boolean;
    def = def.mid( 8 );
  }

  if ( def.startsWith( "multiple"_L1, Qt::CaseInsensitive ) )
  {
    allowMultiple = true;
    def = def.mid( 8 ).trimmed();
  }

  if ( def.startsWith( "default_to_all_fields"_L1, Qt::CaseInsensitive ) )
  {
    defaultToAllFields = true;
    def = def.mid( 21 ).trimmed();
  }

  const thread_local QRegularExpression re( u"(.*?)\\s+(.*)$"_s );
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
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( var );
    var = fromVar.source;
  }
  else if ( var.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
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

  if ( var.userType() != QMetaType::Type::QString || var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

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
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression(%1)"_s.arg( QgsProcessingUtils::stringToPythonLiteral( value.value< QgsProperty >().asExpression() ) );

  if ( value.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
  {
    const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );
    QString geometryCheckString;
    switch ( fromVar.geometryCheck )
    {
      case Qgis::InvalidGeometryCheck::NoCheck:
        geometryCheckString = u"QgsFeatureRequest.GeometryNoCheck"_s;
        break;

      case Qgis::InvalidGeometryCheck::SkipInvalid:
        geometryCheckString = u"QgsFeatureRequest.GeometrySkipInvalid"_s;
        break;

      case Qgis::InvalidGeometryCheck::AbortOnInvalid:
        geometryCheckString = u"QgsFeatureRequest.GeometryAbortOnInvalid"_s;
        break;
    }

    QStringList flags;
    QString flagString;
    if ( fromVar.flags & Qgis::ProcessingFeatureSourceDefinitionFlag::OverrideDefaultGeometryCheck )
      flags << u"QgsProcessingFeatureSourceDefinition.FlagOverrideDefaultGeometryCheck"_s;
    if ( fromVar.flags & Qgis::ProcessingFeatureSourceDefinitionFlag::CreateIndividualOutputPerInputFeature )
      flags << u"QgsProcessingFeatureSourceDefinition.FlagCreateIndividualOutputPerInputFeature"_s;
    if ( !flags.empty() )
      flagString = flags.join( " | "_L1 );

    if ( fromVar.source.propertyType() == Qgis::PropertyType::Static )
    {
      QString layerString = fromVar.source.staticValue().toString();
      // prefer to use layer source instead of id if possible (since it's persistent)
      if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( QgsProcessingUtils::mapLayerFromString( layerString, context, true, QgsProcessingUtils::LayerHint::Vector ) ) )
        layerString = layer->source();

      if ( fromVar.selectedFeaturesOnly || fromVar.featureLimit != -1 || fromVar.flags || !fromVar.filterExpression.isEmpty() )
      {
        return u"QgsProcessingFeatureSourceDefinition(%1, selectedFeaturesOnly=%2, featureLimit=%3%4%6, geometryCheck=%5)"_s.arg( QgsProcessingUtils::stringToPythonLiteral( layerString ),
               fromVar.selectedFeaturesOnly ? u"True"_s : u"False"_s,
               QString::number( fromVar.featureLimit ),
               flagString.isEmpty() ? QString() : ( u", flags=%1"_s.arg( flagString ) ),
               geometryCheckString,
               fromVar.filterExpression.isEmpty() ? QString() : ( u", filterExpression=%1"_s.arg( QgsProcessingUtils::stringToPythonLiteral( fromVar.filterExpression ) ) ) );
      }
      else
      {
        return QgsProcessingUtils::stringToPythonLiteral( layerString );
      }
    }
    else
    {
      if ( fromVar.selectedFeaturesOnly || fromVar.featureLimit != -1 || fromVar.flags || !fromVar.filterExpression.isEmpty() )
      {
        return u"QgsProcessingFeatureSourceDefinition(QgsProperty.fromExpression(%1), selectedFeaturesOnly=%2, featureLimit=%3%4%6, geometryCheck=%5)"_s
               .arg( QgsProcessingUtils::stringToPythonLiteral( fromVar.source.asExpression() ),
                     fromVar.selectedFeaturesOnly ? u"True"_s : u"False"_s,
                     QString::number( fromVar.featureLimit ),
                     flagString.isEmpty() ? QString() : ( u", flags=%1"_s.arg( flagString ) ),
                     geometryCheckString,
                     fromVar.filterExpression.isEmpty() ? QString() : ( u", filterExpression=%1"_s.arg( QgsProcessingUtils::stringToPythonLiteral( fromVar.filterExpression ) ) ) );
      }
      else
      {
        return u"QgsProperty.fromExpression(%1)"_s.arg( QgsProcessingUtils::stringToPythonLiteral( fromVar.source.asExpression() ) );
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
    layerString = layer->providerType() != "ogr"_L1 && layer->providerType() != "gdal"_L1 && layer->providerType() != "mdal"_L1 ? QgsProcessingUtils::encodeProviderKeyAndUri( layer->providerType(), layer->source() ) : layer->source();

  return QgsProcessingUtils::stringToPythonLiteral( layerString );
}

QString QgsProcessingParameterFeatureSource::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterFeatureSource::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QString QgsProcessingParameterFeatureSource::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "source "_L1;

  for ( const int type : mDataTypes )
  {
    switch ( static_cast< Qgis::ProcessingSourceType >( type ) )
    {
      case Qgis::ProcessingSourceType::VectorPoint:
        code += "point "_L1;
        break;

      case Qgis::ProcessingSourceType::VectorLine:
        code += "line "_L1;
        break;

      case Qgis::ProcessingSourceType::VectorPolygon:
        code += "polygon "_L1;
        break;

      default:
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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterFeatureSource('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      if ( !mDataTypes.empty() )
      {
        QStringList options;
        options.reserve( mDataTypes.size() );
        for ( const int t : mDataTypes )
          options << u"QgsProcessing.%1"_s.arg( QgsProcessing::sourceTypeToString( static_cast< Qgis::ProcessingSourceType >( t ) ) );
        code += u", types=[%1]"_s.arg( options.join( ',' ) );
      }

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterFeatureSource::createFileFilter() const
{
  return QgsProviderRegistry::instance()->fileVectorFilters() + u";;"_s + QObject::tr( "All files (*.*)" );
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
  map.insert( u"data_types"_s, types );
  return map;
}

bool QgsProcessingParameterFeatureSource::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mDataTypes.clear();
  const QVariantList values = map.value( u"data_types"_s ).toList();
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
    if ( def.startsWith( "point"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::VectorPoint );
      def = def.mid( 6 );
      continue;
    }
    else if ( def.startsWith( "line"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::VectorLine );
      def = def.mid( 5 );
      continue;
    }
    else if ( def.startsWith( "polygon"_L1, Qt::CaseInsensitive ) )
    {
      types << static_cast< int >( Qgis::ProcessingSourceType::VectorPolygon );
      def = def.mid( 8 );
      continue;
    }
    break;
  }

  return new QgsProcessingParameterFeatureSource( name, description, types, def.isEmpty() ? QVariant() : def, isOptional );
}

QgsProcessingParameterFeatureSink::QgsProcessingParameterFeatureSink( const QString &name, const QString &description, Qgis::ProcessingSourceType type, const QVariant &defaultValue, bool optional, bool createByDefault, bool supportsAppend )
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
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.userType() != QMetaType::Type::QString )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterFeatureSink::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == Qgis::PropertyType::Static )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return u"QgsProperty.fromExpression('%1')"_s.arg( fromVar.sink.asExpression() );
    }
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterFeatureSink::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "sink "_L1;

  switch ( mDataType )
  {
    case Qgis::ProcessingSourceType::VectorPoint:
      code += "point "_L1;
      break;

    case Qgis::ProcessingSourceType::VectorLine:
      code += "line "_L1;
      break;

    case Qgis::ProcessingSourceType::VectorPolygon:
      code += "polygon "_L1;
      break;

    case Qgis::ProcessingSourceType::Vector:
      code += "table "_L1;
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
      return u"dbf"_s;
    }
  }
}

QString QgsProcessingParameterFeatureSink::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterFeatureSink('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", type=QgsProcessing.%1"_s.arg( QgsProcessing::sourceTypeToString( mDataType ) );

      code += u", createByDefault=%1"_s.arg( createByDefault() ? u"True"_s : u"False"_s );
      if ( mSupportsAppend )
        code += ", supportsAppend=True"_L1;

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  return filters.join( ";;"_L1 ) + u";;"_s + QObject::tr( "All files (*.*)" );

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

Qgis::ProcessingSourceType QgsProcessingParameterFeatureSink::dataType() const
{
  return mDataType;
}

bool QgsProcessingParameterFeatureSink::hasGeometry() const
{
  switch ( mDataType )
  {
    case Qgis::ProcessingSourceType::MapLayer:
    case Qgis::ProcessingSourceType::VectorAnyGeometry:
    case Qgis::ProcessingSourceType::VectorPoint:
    case Qgis::ProcessingSourceType::VectorLine:
    case Qgis::ProcessingSourceType::VectorPolygon:
    case Qgis::ProcessingSourceType::VectorTile:
      return true;

    case Qgis::ProcessingSourceType::Raster:
    case Qgis::ProcessingSourceType::File:
    case Qgis::ProcessingSourceType::Vector:
    case Qgis::ProcessingSourceType::Mesh:
    case Qgis::ProcessingSourceType::Plugin:
    case Qgis::ProcessingSourceType::PointCloud:
    case Qgis::ProcessingSourceType::Annotation:
    case Qgis::ProcessingSourceType::TiledScene:
      return false;
  }
  return true;
}

void QgsProcessingParameterFeatureSink::setDataType( Qgis::ProcessingSourceType type )
{
  mDataType = type;
}

QVariantMap QgsProcessingParameterFeatureSink::toVariantMap() const
{
  QVariantMap map = QgsProcessingDestinationParameter::toVariantMap();
  map.insert( u"data_type"_s, static_cast< int >( mDataType ) );
  map.insert( u"supports_append"_s, mSupportsAppend );
  return map;
}

bool QgsProcessingParameterFeatureSink::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingDestinationParameter::fromVariantMap( map );
  mDataType = static_cast< Qgis::ProcessingSourceType >( map.value( u"data_type"_s ).toInt() );
  mSupportsAppend = map.value( u"supports_append"_s, false ).toBool();
  return true;
}

QString QgsProcessingParameterFeatureSink::generateTemporaryDestination( const QgsProcessingContext *context ) const
{
  if ( supportsNonFileBasedOutput() )
    return u"memory:%1"_s.arg( description() );
  else
    return QgsProcessingDestinationParameter::generateTemporaryDestination( context );
}

QgsProcessingParameterFeatureSink *QgsProcessingParameterFeatureSink::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  Qgis::ProcessingSourceType type = Qgis::ProcessingSourceType::VectorAnyGeometry;
  QString def = definition;
  if ( def.startsWith( "point"_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingSourceType::VectorPoint;
    def = def.mid( 6 );
  }
  else if ( def.startsWith( "line"_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingSourceType::VectorLine;
    def = def.mid( 5 );
  }
  else if ( def.startsWith( "polygon"_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingSourceType::VectorPolygon;
    def = def.mid( 8 );
  }
  else if ( def.startsWith( "table"_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingSourceType::Vector;
    def = def.mid( 6 );
  }

  return new QgsProcessingParameterFeatureSink( name, description, type, definition.trimmed().isEmpty() ? QVariant() : definition, isOptional );
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
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.userType() != QMetaType::Type::QString )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterRasterDestination::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == Qgis::PropertyType::Static )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return u"QgsProperty.fromExpression('%1')"_s.arg( fromVar.sink.asExpression() );
    }
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QgsProcessingOutputDefinition *QgsProcessingParameterRasterDestination::toOutputDefinition() const
{
  return new QgsProcessingOutputRasterLayer( name(), description() );
}

QString QgsProcessingParameterRasterDestination::defaultFileFormat() const
{
  if ( auto *lOriginalProvider = originalProvider() )
  {
    return lOriginalProvider->defaultRasterFileFormat();
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->defaultRasterFileFormat();
  }
  else
  {
    return QgsProcessingUtils::defaultRasterFormat();
  }
}

QString QgsProcessingParameterRasterDestination::defaultFileExtension() const
{
  QString format = defaultFileFormat();
  QStringList extensions = QgsRasterFileWriter::extensionsForFormat( format );
  if ( !extensions.isEmpty() )
    return extensions[0];

  return u"tif"_s;
}

QString QgsProcessingParameterRasterDestination::createFileFilter() const
{
  QStringList filters;
  const QList<QPair<QString, QString>> formatAndExtensions = supportedOutputRasterLayerFormatAndExtensions();
  for ( const QPair<QString, QString> &formatAndExt : std::as_const( formatAndExtensions ) )
  {
    QString format = formatAndExt.first;
    const QString &extension = formatAndExt.second;
    if ( format.isEmpty() )
      format = extension;
    filters << QObject::tr( "%1 files (*.%2)" ).arg( format.toUpper(), extension.toLower() );
  }

  return filters.join( ";;"_L1 ) + u";;"_s + QObject::tr( "All files (*.*)" );
}

QStringList QgsProcessingParameterRasterDestination::supportedOutputRasterLayerExtensions() const
{
  const QList<QPair<QString, QString>> formatAndExtensions = supportedOutputRasterLayerFormatAndExtensions();
  QSet< QString > extensions;
  for ( const QPair<QString, QString> &formatAndExt : std::as_const( formatAndExtensions ) )
  {
    extensions.insert( formatAndExt.second );
  }
  return QStringList( extensions.constBegin(), extensions.constEnd() );
}

QList<QPair<QString, QString>>  QgsProcessingParameterRasterDestination::supportedOutputRasterLayerFormatAndExtensions() const
{
  if ( auto *lOriginalProvider = originalProvider() )
  {
    return lOriginalProvider->supportedOutputRasterLayerFormatAndExtensions();
  }
  else if ( QgsProcessingProvider *p = provider() )
  {
    return p->supportedOutputRasterLayerFormatAndExtensions();
  }
  else
  {
    return QgsProcessingProvider::supportedOutputRasterLayerFormatAndExtensionsDefault();
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
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.userType() != QMetaType::Type::QString )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  // possible enhancement - check that value is compatible with file filter?

  return true;
}

QString QgsProcessingParameterFileDestination::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == Qgis::PropertyType::Static )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return u"QgsProperty.fromExpression('%1')"_s.arg( fromVar.sink.asExpression() );
    }
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QgsProcessingOutputDefinition *QgsProcessingParameterFileDestination::toOutputDefinition() const
{
  if ( !mFileFilter.isEmpty() && mFileFilter.contains( u"htm"_s, Qt::CaseInsensitive ) )
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
    return u"file"_s;

  // get first extension from filter
  const thread_local QRegularExpression rx( u".*?\\(\\*\\.([a-zA-Z0-9._]+).*"_s );
  const QRegularExpressionMatch match = rx.match( mFileFilter );
  if ( !match.hasMatch() )
    return u"file"_s;

  return match.captured( 1 );
}

QString QgsProcessingParameterFileDestination::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterFileDestination('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", fileFilter=%1"_s.arg( QgsProcessingUtils::stringToPythonLiteral( mFileFilter ) );

      code += u", createByDefault=%1"_s.arg( createByDefault() ? u"True"_s : u"False"_s );

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterFileDestination::createFileFilter() const
{
  return ( fileFilter().isEmpty() ? QString() : fileFilter() + u";;"_s ) + QObject::tr( "All files (*.*)" );
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
  map.insert( u"file_filter"_s, mFileFilter );
  return map;
}

bool QgsProcessingParameterFileDestination::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingDestinationParameter::fromVariantMap( map );
  mFileFilter = map.value( u"file_filter"_s ).toString();
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
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.userType() != QMetaType::Type::QString )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

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
  map.insert( u"supports_non_file_outputs"_s, mSupportsNonFileBasedOutputs );
  map.insert( u"create_by_default"_s, mCreateByDefault );
  return map;
}

bool QgsProcessingDestinationParameter::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mSupportsNonFileBasedOutputs = map.value( u"supports_non_file_outputs"_s ).toBool();
  mCreateByDefault = map.value( u"create_by_default"_s, u"1"_s ).toBool();
  return true;
}

QString QgsProcessingDestinationParameter::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      // base class method is probably not much use
      if ( QgsProcessingParameterType *t = QgsApplication::processingRegistry()->parameterType( type() ) )
      {
        QString code = t->className() + u"('%1', %2"_s
                       .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
        if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
          code += ", optional=True"_L1;

        code += u", createByDefault=%1"_s.arg( mCreateByDefault ? u"True"_s : u"False"_s );

        QgsProcessingContext c;
        code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  return QObject::tr( "Default extension" ) + u" (*."_s + defaultFileExtension() + ')';
}

QString QgsProcessingDestinationParameter::generateTemporaryDestination( const QgsProcessingContext *context ) const
{
  // sanitize name to avoid multiple . in the filename. E.g. when name() contain
  // backend command name having a "." inside as in case of grass commands
  const thread_local QRegularExpression rx( u"[.]"_s );
  QString sanitizedName = name();
  sanitizedName.replace( rx, u"_"_s );

  if ( defaultFileExtension().isEmpty() )
  {
    return QgsProcessingUtils::generateTempFilename( sanitizedName, context );
  }
  else
  {
    return QgsProcessingUtils::generateTempFilename( sanitizedName + '.' + defaultFileExtension(), context );
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

QgsProcessingParameterVectorDestination::QgsProcessingParameterVectorDestination( const QString &name, const QString &description, Qgis::ProcessingSourceType type, const QVariant &defaultValue, bool optional, bool createByDefault )
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
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.userType() != QMetaType::Type::QString )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterVectorDestination::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == Qgis::PropertyType::Static )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return u"QgsProperty.fromExpression('%1')"_s.arg( fromVar.sink.asExpression() );
    }
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterVectorDestination::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "vectorDestination "_L1;

  switch ( mDataType )
  {
    case Qgis::ProcessingSourceType::VectorPoint:
      code += "point "_L1;
      break;

    case Qgis::ProcessingSourceType::VectorLine:
      code += "line "_L1;
      break;

    case Qgis::ProcessingSourceType::VectorPolygon:
      code += "polygon "_L1;
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
      return u"dbf"_s;
    }
  }
}

QString QgsProcessingParameterVectorDestination::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterVectorDestination('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", type=QgsProcessing.%1"_s.arg( QgsProcessing::sourceTypeToString( mDataType ) );

      code += u", createByDefault=%1"_s.arg( createByDefault() ? u"True"_s : u"False"_s );

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  return filters.join( ";;"_L1 ) + u";;"_s + QObject::tr( "All files (*.*)" );
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

Qgis::ProcessingSourceType QgsProcessingParameterVectorDestination::dataType() const
{
  return mDataType;
}

bool QgsProcessingParameterVectorDestination::hasGeometry() const
{
  switch ( mDataType )
  {
    case Qgis::ProcessingSourceType::MapLayer:
    case Qgis::ProcessingSourceType::VectorAnyGeometry:
    case Qgis::ProcessingSourceType::VectorPoint:
    case Qgis::ProcessingSourceType::VectorLine:
    case Qgis::ProcessingSourceType::VectorPolygon:
    case Qgis::ProcessingSourceType::VectorTile:
      return true;

    case Qgis::ProcessingSourceType::Raster:
    case Qgis::ProcessingSourceType::File:
    case Qgis::ProcessingSourceType::Vector:
    case Qgis::ProcessingSourceType::Mesh:
    case Qgis::ProcessingSourceType::Plugin:
    case Qgis::ProcessingSourceType::PointCloud:
    case Qgis::ProcessingSourceType::Annotation:
    case Qgis::ProcessingSourceType::TiledScene:
      return false;
  }
  return true;
}

void QgsProcessingParameterVectorDestination::setDataType( Qgis::ProcessingSourceType type )
{
  mDataType = type;
}

QVariantMap QgsProcessingParameterVectorDestination::toVariantMap() const
{
  QVariantMap map = QgsProcessingDestinationParameter::toVariantMap();
  map.insert( u"data_type"_s, static_cast< int >( mDataType ) );
  return map;
}

bool QgsProcessingParameterVectorDestination::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingDestinationParameter::fromVariantMap( map );
  mDataType = static_cast< Qgis::ProcessingSourceType >( map.value( u"data_type"_s ).toInt() );
  return true;
}

QgsProcessingParameterVectorDestination *QgsProcessingParameterVectorDestination::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  Qgis::ProcessingSourceType type = Qgis::ProcessingSourceType::VectorAnyGeometry;
  QString def = definition;
  if ( def.startsWith( "point"_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingSourceType::VectorPoint;
    def = def.mid( 6 );
  }
  else if ( def.startsWith( "line"_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingSourceType::VectorLine;
    def = def.mid( 5 );
  }
  else if ( def.startsWith( "polygon"_L1, Qt::CaseInsensitive ) )
  {
    type = Qgis::ProcessingSourceType::VectorPolygon;
    def = def.mid( 8 );
  }

  return new QgsProcessingParameterVectorDestination( name, description, type, definition.isEmpty() ? QVariant() : definition, isOptional );
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

bool QgsProcessingParameterBand::checkValueIsAcceptable( const QVariant &value, QgsProcessingContext * ) const
{
  QVariant input = value;
  if ( !input.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( input.userType() == QMetaType::Type::QVariantList || input.userType() == QMetaType::Type::QStringList )
  {
    if ( !mAllowMultiple )
      return false;

    if ( input.toList().isEmpty() && !( mFlags & Qgis::ProcessingParameterFlag::Optional ) )
      return false;
  }
  else
  {
    bool ok = false;
    const double res = input.toInt( &ok );
    Q_UNUSED( res )
    if ( !ok )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
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
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == QMetaType::Type::QVariantList )
  {
    QStringList parts;
    const QVariantList values = value.toList();
    for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
    {
      parts << QString::number( static_cast< int >( it->toDouble() ) );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else if ( value.userType() == QMetaType::Type::QStringList )
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
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "band "_L1;

  if ( mAllowMultiple )
    code += "multiple "_L1;

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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterBand('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", parentLayerParameterName='%1'"_s.arg( mParentLayerParameterName );
      code += u", allowMultiple=%1"_s.arg( mAllowMultiple ? u"True"_s : u"False"_s );

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  map.insert( u"parent_layer"_s, mParentLayerParameterName );
  map.insert( u"allow_multiple"_s, mAllowMultiple );
  return map;
}

bool QgsProcessingParameterBand::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( u"parent_layer"_s ).toString();
  mAllowMultiple = map.value( u"allow_multiple"_s ).toBool();
  return true;
}

QgsProcessingParameterBand *QgsProcessingParameterBand::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString parent;
  QString def = definition;
  bool allowMultiple = false;

  if ( def.startsWith( "multiple"_L1, Qt::CaseInsensitive ) )
  {
    allowMultiple = true;
    def = def.mid( 8 ).trimmed();
  }

  const thread_local QRegularExpression re( u"(.*?)\\s+(.*)$"_s );
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
  : QgsProcessingParameterNumber( name, description, Qgis::ProcessingNumberParameterType::Double, defaultValue, optional, minValue, maxValue )
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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterDistance('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", parentParameterName='%1'"_s.arg( mParentParameterName );

      if ( minimum() != std::numeric_limits<double>::lowest() + 1 )
        code += u", minValue=%1"_s.arg( minimum() );
      if ( maximum() != std::numeric_limits<double>::max() )
        code += u", maxValue=%1"_s.arg( maximum() );
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  map.insert( u"parent"_s, mParentParameterName );
  map.insert( u"default_unit"_s, static_cast< int >( mDefaultUnit ) );
  return map;
}

bool QgsProcessingParameterDistance::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterNumber::fromVariantMap( map );
  mParentParameterName = map.value( u"parent"_s ).toString();
  mDefaultUnit = static_cast< Qgis::DistanceUnit>( map.value( u"default_unit"_s, static_cast< int >( Qgis::DistanceUnit::Unknown ) ).toInt() );
  return true;
}


QString QgsProcessingParameterDistance::userFriendlyString( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  return u"%1 %2"_s.arg( value.toString(), QgsUnitTypes::toAbbreviatedString( defaultUnit() ) );
}


//
// QgsProcessingParameterArea
//

QgsProcessingParameterArea::QgsProcessingParameterArea( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentParameterName, bool optional, double minValue, double maxValue )
  : QgsProcessingParameterNumber( name, description, Qgis::ProcessingNumberParameterType::Double, defaultValue, optional, minValue, maxValue )
  , mParentParameterName( parentParameterName )
{

}

QgsProcessingParameterArea *QgsProcessingParameterArea::clone() const
{
  return new QgsProcessingParameterArea( *this );
}

QString QgsProcessingParameterArea::type() const
{
  return typeName();
}

QStringList QgsProcessingParameterArea::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentParameterName.isEmpty() )
    depends << mParentParameterName;
  return depends;
}

QString QgsProcessingParameterArea::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterArea('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", parentParameterName='%1'"_s.arg( mParentParameterName );

      if ( minimum() != 0 )
        code += u", minValue=%1"_s.arg( minimum() );
      if ( maximum() != std::numeric_limits<double>::max() )
        code += u", maxValue=%1"_s.arg( maximum() );
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterArea::parentParameterName() const
{
  return mParentParameterName;
}

void QgsProcessingParameterArea::setParentParameterName( const QString &parentParameterName )
{
  mParentParameterName = parentParameterName;
}

QVariantMap QgsProcessingParameterArea::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterNumber::toVariantMap();
  map.insert( u"parent"_s, mParentParameterName );
  map.insert( u"default_unit"_s, qgsEnumValueToKey( mDefaultUnit ) );
  return map;
}

bool QgsProcessingParameterArea::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterNumber::fromVariantMap( map );
  mParentParameterName = map.value( u"parent"_s ).toString();
  mDefaultUnit = qgsEnumKeyToValue( map.value( u"default_unit"_s ).toString(), Qgis::AreaUnit::Unknown );
  return true;
}


QString QgsProcessingParameterArea::userFriendlyString( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  return u"%1 %2"_s.arg( value.toString(), QgsUnitTypes::toAbbreviatedString( defaultUnit() ) );
}


//
// QgsProcessingParameterVolume
//

QgsProcessingParameterVolume::QgsProcessingParameterVolume( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentParameterName, bool optional, double minValue, double maxValue )
  : QgsProcessingParameterNumber( name, description, Qgis::ProcessingNumberParameterType::Double, defaultValue, optional, minValue, maxValue )
  , mParentParameterName( parentParameterName )
{

}

QgsProcessingParameterVolume *QgsProcessingParameterVolume::clone() const
{
  return new QgsProcessingParameterVolume( *this );
}

QString QgsProcessingParameterVolume::type() const
{
  return typeName();
}

QStringList QgsProcessingParameterVolume::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentParameterName.isEmpty() )
    depends << mParentParameterName;
  return depends;
}

QString QgsProcessingParameterVolume::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterVolume('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", parentParameterName='%1'"_s.arg( mParentParameterName );

      if ( minimum() != 0 )
        code += u", minValue=%1"_s.arg( minimum() );
      if ( maximum() != std::numeric_limits<double>::max() )
        code += u", maxValue=%1"_s.arg( maximum() );
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QString QgsProcessingParameterVolume::parentParameterName() const
{
  return mParentParameterName;
}

void QgsProcessingParameterVolume::setParentParameterName( const QString &parentParameterName )
{
  mParentParameterName = parentParameterName;
}

QVariantMap QgsProcessingParameterVolume::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterNumber::toVariantMap();
  map.insert( u"parent"_s, mParentParameterName );
  map.insert( u"default_unit"_s, qgsEnumValueToKey( mDefaultUnit ) );
  return map;
}

bool QgsProcessingParameterVolume::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterNumber::fromVariantMap( map );
  mParentParameterName = map.value( u"parent"_s ).toString();
  mDefaultUnit = qgsEnumKeyToValue( map.value( u"default_unit"_s ).toString(), Qgis::VolumeUnit::Unknown );
  return true;
}

QString QgsProcessingParameterVolume::userFriendlyString( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  return u"%1 %2"_s.arg( value.toString(), QgsUnitTypes::toAbbreviatedString( defaultUnit() ) );
}

//
// QgsProcessingParameterDuration
//

QgsProcessingParameterDuration::QgsProcessingParameterDuration( const QString &name, const QString &description, const QVariant &defaultValue, bool optional, double minValue, double maxValue )
  : QgsProcessingParameterNumber( name, description, Qgis::ProcessingNumberParameterType::Double, defaultValue, optional, minValue, maxValue )
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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterDuration('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      if ( minimum() != std::numeric_limits<double>::lowest() + 1 )
        code += u", minValue=%1"_s.arg( minimum() );
      if ( maximum() != std::numeric_limits<double>::max() )
        code += u", maxValue=%1"_s.arg( maximum() );
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterDuration::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterNumber::toVariantMap();
  map.insert( u"default_unit"_s, static_cast< int >( mDefaultUnit ) );
  return map;
}

bool QgsProcessingParameterDuration::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterNumber::fromVariantMap( map );
  mDefaultUnit = static_cast< Qgis::TemporalUnit>( map.value( u"default_unit"_s, static_cast< int >( Qgis::TemporalUnit::Days ) ).toInt() );
  return true;
}

QString QgsProcessingParameterDuration::userFriendlyString( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  return u"%1 %2"_s.arg( value.toString(), QgsUnitTypes::toAbbreviatedString( defaultUnit() ) );
}


//
// QgsProcessingParameterScale
//

QgsProcessingParameterScale::QgsProcessingParameterScale( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterNumber( name, description, Qgis::ProcessingNumberParameterType::Double, defaultValue, optional )
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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterScale('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QgsProcessingParameterScale *QgsProcessingParameterScale::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition ) // cppcheck-suppress duplInheritedMember
{
  return new QgsProcessingParameterScale( name, description, definition.isEmpty() ? QVariant()
                                          : ( definition.toLower().trimmed() == "none"_L1 ? QVariant() : definition ), isOptional );
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
  if ( QgsVariantUtils::isNull( value ) )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterLayout::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "layout "_L1;

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterLayout::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterLayout('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;
      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
  if ( def == "None"_L1 )
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
  if ( QgsVariantUtils::isNull( value ) )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterLayoutItem::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "layoutitem "_L1;
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
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterLayoutItem('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      if ( mItemType >= 0 )
        code += u", itemType=%1"_s.arg( mItemType );

      code += u", parentLayoutParameterName='%1'"_s.arg( mParentLayoutParameterName );

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterLayoutItem::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"parent_layout"_s, mParentLayoutParameterName );
  map.insert( u"item_type"_s, mItemType );
  return map;
}

bool QgsProcessingParameterLayoutItem::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayoutParameterName = map.value( u"parent_layout"_s ).toString();
  mItemType = map.value( u"item_type"_s ).toInt();
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
  const thread_local QRegularExpression re( u"(\\d+)?\\s*(.*?)\\s+(.*)$"_s );
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
  if ( QgsVariantUtils::isNull( value ) )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert< QColor >() && !value.value< QColor >().isValid() )
    return u"QColor()"_s;

  if ( value.canConvert< QColor >() )
  {
    const QColor c = value.value< QColor >();
    if ( !mAllowOpacity || c.alpha() == 255 )
      return u"QColor(%1, %2, %3)"_s.arg( c.red() ).arg( c.green() ).arg( c.blue() );
    else
      return u"QColor(%1, %2, %3, %4)"_s.arg( c.red() ).arg( c.green() ).arg( c.blue() ).arg( c.alpha() );
  }

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterColor::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "color "_L1;

  if ( mAllowOpacity )
    code += "withopacity "_L1;

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterColor::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterColor('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", opacityEnabled=%1"_s.arg( mAllowOpacity ? u"True"_s : u"False"_s );

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( input.userType() == QMetaType::Type::QColor )
  {
    return true;
  }
  else if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( input.userType() != QMetaType::Type::QString || input.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  bool containsAlpha = false;
  return QgsSymbolLayerUtils::parseColorWithAlpha( input.toString(), containsAlpha ).isValid();
}

QVariantMap QgsProcessingParameterColor::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"opacityEnabled"_s, mAllowOpacity );
  return map;
}

bool QgsProcessingParameterColor::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mAllowOpacity = map.value( u"opacityEnabled"_s ).toBool();
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
  if ( def.startsWith( "withopacity"_L1, Qt::CaseInsensitive ) )
  {
    allowOpacity = true;
    def = def.mid( 12 );
  }

  if ( def.startsWith( '"' ) || def.startsWith( '\'' ) )
    def = def.mid( 1 );
  if ( def.endsWith( '"' ) || def.endsWith( '\'' ) )
    def.chop( 1 );

  QVariant defaultValue = def;
  if ( def == "None"_L1 || def.isEmpty() )
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
  return valueAsPythonStringPrivate( value, context, false );
}

QString QgsProcessingParameterCoordinateOperation::valueAsPythonStringPrivate( const QVariant &value, QgsProcessingContext &context, bool allowNonStringValues ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return u"None"_s;

  if ( allowNonStringValues && value.userType() == qMetaTypeId<QgsCoordinateReferenceSystem>() )
  {
    if ( !value.value< QgsCoordinateReferenceSystem >().isValid() )
      return u"QgsCoordinateReferenceSystem()"_s;
    else
      return u"QgsCoordinateReferenceSystem('%1')"_s.arg( value.value< QgsCoordinateReferenceSystem >().authid() );
  }

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( allowNonStringValues )
  {
    QVariantMap p;
    p.insert( name(), value );
    QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
    if ( layer )
      return QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::layerToStringIdentifier( layer ) );
  }

  const QString s = value.toString();
  return QgsProcessingUtils::stringToPythonLiteral( s );
}

QString QgsProcessingParameterCoordinateOperation::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "coordinateoperation "_L1;

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterCoordinateOperation::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QgsProcessingContext c;
      QString code = u"QgsProcessingParameterCoordinateOperation('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;
      if ( !mSourceParameterName.isEmpty() )
        code += u", sourceCrsParameterName=%1"_s.arg( valueAsPythonStringPrivate( mSourceParameterName, c, false ) );
      if ( !mDestParameterName.isEmpty() )
        code += u", destinationCrsParameterName=%1"_s.arg( valueAsPythonStringPrivate( mDestParameterName, c, false ) );

      if ( mSourceCrs.isValid() )
        code += u", staticSourceCrs=%1"_s.arg( valueAsPythonStringPrivate( mSourceCrs, c, true ) );
      if ( mDestCrs.isValid() )
        code += u", staticDestinationCrs=%1"_s.arg( valueAsPythonStringPrivate( mDestCrs, c, true ) );

      code += u", defaultValue=%1)"_s.arg( valueAsPythonStringPrivate( mDefault, c, false ) );
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
  map.insert( u"source_crs_parameter_name"_s, mSourceParameterName );
  map.insert( u"dest_crs_parameter_name"_s, mDestParameterName );
  map.insert( u"static_source_crs"_s, mSourceCrs );
  map.insert( u"static_dest_crs"_s, mDestCrs );
  return map;
}

bool QgsProcessingParameterCoordinateOperation::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mSourceParameterName = map.value( u"source_crs_parameter_name"_s ).toString();
  mDestParameterName = map.value( u"dest_crs_parameter_name"_s ).toString();
  mSourceCrs = map.value( u"static_source_crs"_s );
  mDestCrs = map.value( u"static_dest_crs"_s );
  return true;
}

QgsProcessingParameterCoordinateOperation *QgsProcessingParameterCoordinateOperation::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString def = definition;

  if ( def.startsWith( '"' ) )
  {
    def = def.mid( 1 );
    if ( def.endsWith( '"' ) )
      def.chop( 1 );
  }
  else if ( def.startsWith( '\'' ) )
  {
    def = def.mid( 1 );
    if ( def.endsWith( '\'' ) )
      def.chop( 1 );
  }

  QVariant defaultValue = def;
  if ( def == "None"_L1 )
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
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( ( input.userType() == QMetaType::Type::QString && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.userType() == QMetaType::Type::QString && mDefault.toString().isEmpty() ) )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterMapTheme::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterMapTheme::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "maptheme "_L1;

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterMapTheme::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterMapTheme('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );

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

  if ( defaultValue == "None"_L1 || defaultValue.toString().isEmpty() )
    defaultValue = QVariant();

  return new QgsProcessingParameterMapTheme( name, description, defaultValue, isOptional );
}


//
// QgsProcessingParameterDateTime
//

QgsProcessingParameterDateTime::QgsProcessingParameterDateTime( const QString &name, const QString &description, Qgis::ProcessingDateTimeParameterDataType type, const QVariant &defaultValue, bool optional, const QDateTime &minValue, const QDateTime &maxValue )
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
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( input.userType() != QMetaType::Type::QDateTime && input.userType() != QMetaType::Type::QDate && input.userType() != QMetaType::Type::QTime && input.userType() != QMetaType::Type::QString )
    return false;

  if ( ( input.userType() == QMetaType::Type::QDateTime || input.userType() == QMetaType::Type::QDate ) && mDataType == Qgis::ProcessingDateTimeParameterDataType::Time )
    return false;

  if ( input.userType() == QMetaType::Type::QString )
  {
    const QString s = input.toString();
    if ( s.isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = QDateTime::fromString( s, Qt::ISODate );
    if ( mDataType == Qgis::ProcessingDateTimeParameterDataType::Time )
    {
      if ( !input.toDateTime().isValid() )
        input = QTime::fromString( s );
      else
        input = input.toDateTime().time();
    }
  }

  if ( mDataType != Qgis::ProcessingDateTimeParameterDataType::Time )
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
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == QMetaType::Type::QDateTime )
  {
    const QDateTime dt = value.toDateTime();
    if ( !dt.isValid() )
      return u"QDateTime()"_s;
    else
      return u"QDateTime(QDate(%1, %2, %3), QTime(%4, %5, %6))"_s.arg( dt.date().year() )
             .arg( dt.date().month() )
             .arg( dt.date().day() )
             .arg( dt.time().hour() )
             .arg( dt.time().minute() )
             .arg( dt.time().second() );
  }
  else if ( value.userType() == QMetaType::Type::QDate )
  {
    const QDate dt = value.toDate();
    if ( !dt.isValid() )
      return u"QDate()"_s;
    else
      return u"QDate(%1, %2, %3)"_s.arg( dt.year() )
             .arg( dt.month() )
             .arg( dt.day() );
  }
  else if ( value.userType() == QMetaType::Type::QTime )
  {
    const QTime dt = value.toTime();
    if ( !dt.isValid() )
      return u"QTime()"_s;
    else
      return u"QTime(%4, %5, %6)"_s
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
    parts << QObject::tr( "Default value: %1" ).arg( mDataType == Qgis::ProcessingDateTimeParameterDataType::DateTime ? mDefault.toDateTime().toString( Qt::ISODate ) :
          ( mDataType == Qgis::ProcessingDateTimeParameterDataType::Date ? mDefault.toDate().toString( Qt::ISODate ) : mDefault.toTime( ).toString() ) );
  const QString extra = parts.join( "<br />"_L1 );
  if ( !extra.isEmpty() )
    text += u"<p>%1</p>"_s.arg( extra );
  return text;
}

QString QgsProcessingParameterDateTime::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterDateTime('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", type=%1"_s.arg( mDataType == Qgis::ProcessingDateTimeParameterDataType::DateTime ? u"QgsProcessingParameterDateTime.DateTime"_s
                                  : mDataType == Qgis::ProcessingDateTimeParameterDataType::Date ? u"QgsProcessingParameterDateTime.Date"_s
                                  : u"QgsProcessingParameterDateTime.Time"_s );

      QgsProcessingContext c;
      if ( mMin.isValid() )
        code += u", minValue=%1"_s.arg( valueAsPythonString( mMin, c ) );
      if ( mMax.isValid() )
        code += u", maxValue=%1"_s.arg( valueAsPythonString( mMax, c ) );
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );
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

Qgis::ProcessingDateTimeParameterDataType QgsProcessingParameterDateTime::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterDateTime::setDataType( Qgis::ProcessingDateTimeParameterDataType dataType )
{
  mDataType = dataType;
}

QVariantMap QgsProcessingParameterDateTime::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"min"_s, mMin );
  map.insert( u"max"_s, mMax );
  map.insert( u"data_type"_s, static_cast< int >( mDataType ) );
  return map;
}

bool QgsProcessingParameterDateTime::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mMin = map.value( u"min"_s ).toDateTime();
  mMax = map.value( u"max"_s ).toDateTime();
  mDataType = static_cast< Qgis::ProcessingDateTimeParameterDataType >( map.value( u"data_type"_s ).toInt() );
  return true;
}

QgsProcessingParameterDateTime *QgsProcessingParameterDateTime::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterDateTime( name, description, Qgis::ProcessingDateTimeParameterDataType::DateTime, definition.isEmpty() ? QVariant()
         : ( definition.toLower().trimmed() == "none"_L1 ? QVariant() : definition ), isOptional );
}


QString QgsProcessingParameterDateTime:: userFriendlyString( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QString();

  if ( value.userType() == QMetaType::Type::QDateTime )
  {
    const QDateTime dt = value.toDateTime();
    if ( !dt.isValid() )
      return QObject::tr( "Invalid datetime" );
    else
      return dt.toString( Qt::ISODate );
  }

  else if ( value.userType() == QMetaType::Type::QDate )
  {
    const QDate dt = value.toDate();
    if ( !dt.isValid() )
      return QObject::tr( "Invalid date" );
    else
      return dt.toString( Qt::ISODate );
  }

  else if ( value.userType() == QMetaType::Type::QTime )
  {
    const QTime dt = value.toTime();
    if ( !dt.isValid() )
      return QObject::tr( "Invalid time" );
    else
      return dt.toString( Qt::ISODate );
  }

  return value.toString();
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
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( ( input.userType() == QMetaType::Type::QString && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.userType() == QMetaType::Type::QString && mDefault.toString().isEmpty() ) )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterProviderConnection::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterProviderConnection::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "providerconnection "_L1;
  code += mProviderId + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterProviderConnection::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterProviderConnection('%1', %2, '%3'"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ), mProviderId );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      QgsProcessingContext c;
      code += u", defaultValue=%1)"_s.arg( valueAsPythonString( mDefault, c ) );

      return code;
    }
  }
  return QString();
}

QVariantMap QgsProcessingParameterProviderConnection::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"provider"_s, mProviderId );
  return map;
}

bool QgsProcessingParameterProviderConnection::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mProviderId = map.value( u"provider"_s ).toString();
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

  if ( defaultValue == "None"_L1 || defaultValue.toString().isEmpty() )
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
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( ( input.userType() == QMetaType::Type::QString && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.userType() == QMetaType::Type::QString && mDefault.toString().isEmpty() ) )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterDatabaseSchema::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterDatabaseSchema::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "databaseschema "_L1;

  code += mParentConnectionParameterName + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterDatabaseSchema::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterDatabaseSchema('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", connectionParameterName='%1'"_s.arg( mParentConnectionParameterName );
      QgsProcessingContext c;
      code += u", defaultValue=%1"_s.arg( valueAsPythonString( mDefault, c ) );

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
  map.insert( u"mParentConnectionParameterName"_s, mParentConnectionParameterName );
  return map;
}

bool QgsProcessingParameterDatabaseSchema::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentConnectionParameterName = map.value( u"mParentConnectionParameterName"_s ).toString();
  return true;
}

QgsProcessingParameterDatabaseSchema *QgsProcessingParameterDatabaseSchema::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString parent;
  QString def = definition;

  const thread_local QRegularExpression re( u"(.*?)\\s+(.*)$"_s );
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
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( ( input.userType() == QMetaType::Type::QString && input.toString().isEmpty() )
       || ( !input.isValid() && mDefault.userType() == QMetaType::Type::QString && mDefault.toString().isEmpty() ) )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterDatabaseTable::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QString QgsProcessingParameterDatabaseTable::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "databasetable "_L1;

  code += ( mParentConnectionParameterName.isEmpty() ? u"none"_s : mParentConnectionParameterName ) + ' ';
  code += ( mParentSchemaParameterName.isEmpty() ? u"none"_s : mParentSchemaParameterName ) + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterDatabaseTable::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterDatabaseTable('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      if ( mAllowNewTableNames )
        code += ", allowNewTableNames=True"_L1;

      code += u", connectionParameterName='%1'"_s.arg( mParentConnectionParameterName );
      code += u", schemaParameterName='%1'"_s.arg( mParentSchemaParameterName );
      QgsProcessingContext c;
      code += u", defaultValue=%1"_s.arg( valueAsPythonString( mDefault, c ) );

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
  map.insert( u"mParentConnectionParameterName"_s, mParentConnectionParameterName );
  map.insert( u"mParentSchemaParameterName"_s, mParentSchemaParameterName );
  map.insert( u"mAllowNewTableNames"_s, mAllowNewTableNames );
  return map;
}

bool QgsProcessingParameterDatabaseTable::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentConnectionParameterName = map.value( u"mParentConnectionParameterName"_s ).toString();
  mParentSchemaParameterName = map.value( u"mParentSchemaParameterName"_s ).toString();
  mAllowNewTableNames = map.value( u"mAllowNewTableNames"_s, false ).toBool();
  return true;
}

QgsProcessingParameterDatabaseTable *QgsProcessingParameterDatabaseTable::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString connection;
  QString schema;
  QString def = definition;

  const thread_local QRegularExpression re( u"(.*?)\\s+(.*+)\\b\\s*(.*)$"_s );
  const QRegularExpressionMatch m = re.match( def );
  if ( m.hasMatch() )
  {
    connection = m.captured( 1 ).trimmed();
    if ( connection == "none"_L1 )
      connection.clear();
    schema = m.captured( 2 ).trimmed();
    if ( schema == "none"_L1 )
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
  QVariant var = v;

  if ( !var.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
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

  if ( var.userType() != QMetaType::Type::QString || var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  if ( !context )
  {
    // that's as far as we can get without a context
    return true;
  }

  // try to load as layer
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context, true, QgsProcessingUtils::LayerHint::PointCloud, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration ) )
    return true;

  return false;
}

QString QgsProcessingParameterPointCloudLayer::valueAsPythonString( const QVariant &val, QgsProcessingContext &context ) const
{
  if ( !val.isValid() )
    return u"None"_s;

  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsPointCloudLayer *layer = QgsProcessingParameters::parameterAsPointCloudLayer( this, p, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( QgsProcessingUtils::layerToStringIdentifier( layer ) )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QString QgsProcessingParameterPointCloudLayer::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterPointCloudLayer::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
}

QString QgsProcessingParameterPointCloudLayer::createFileFilter() const
{
  return QgsProviderRegistry::instance()->filePointCloudFilters() + u";;"_s + QObject::tr( "All files (*.*)" );
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
  QVariant var = v;
  if ( !var.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
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

  if ( var.userType() != QMetaType::Type::QString || var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

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
    return u"None"_s;

  if ( val.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsAnnotationLayer *layer = QgsProcessingParameters::parameterAsAnnotationLayer( this, p, context );
  return layer ? QgsProcessingUtils::stringToPythonLiteral( layer == context.project()->mainAnnotationLayer() ? u"main"_s : layer->id() )
         : QgsProcessingUtils::stringToPythonLiteral( val.toString() );
}

QString QgsProcessingParameterAnnotationLayer::valueAsString( const QVariant &value, QgsProcessingContext &context, bool &ok ) const
{
  return valueAsStringPrivate( value, context, ok, ValueAsStringFlag::AllowMapLayerValues );
}

QVariant QgsProcessingParameterAnnotationLayer::valueAsJsonObject( const QVariant &value, QgsProcessingContext &context ) const
{
  return valueAsJsonObjectPrivate( value, context, ValueAsStringFlag::AllowMapLayerValues );
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
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.userType() != QMetaType::Type::QString )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterPointCloudDestination::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == Qgis::PropertyType::Static )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return u"QgsProperty.fromExpression('%1')"_s.arg( fromVar.sink.asExpression() );
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
  return filters.join( ";;"_L1 ) + u";;"_s + QObject::tr( "All files (*.*)" );
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
    return QStringList() << ext;
  }
}

QgsProcessingParameterPointCloudDestination *QgsProcessingParameterPointCloudDestination::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterPointCloudDestination( name, description, definition.isEmpty() ? QVariant() : definition, isOptional );
}

//
// QgsProcessingParameterPointCloudAttribute
//

QgsProcessingParameterPointCloudAttribute::QgsProcessingParameterPointCloudAttribute( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, bool allowMultiple, bool optional, bool defaultToAllAttributes )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameterName( parentLayerParameterName )
  , mAllowMultiple( allowMultiple )
  , mDefaultToAllAttributes( defaultToAllAttributes )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterPointCloudAttribute::clone() const
{
  return new QgsProcessingParameterPointCloudAttribute( *this );
}

bool QgsProcessingParameterPointCloudAttribute::checkValueIsAcceptable( const QVariant &v, QgsProcessingContext * ) const
{
  QVariant input = v;
  if ( !v.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    input = defaultValue();
  }

  if ( input.userType() == qMetaTypeId<QgsProperty>() )
  {
    return true;
  }

  if ( input.userType() == QMetaType::Type::QVariantList || input.userType() == QMetaType::Type::QStringList )
  {
    if ( !mAllowMultiple )
      return false;

    if ( input.toList().isEmpty() && !( mFlags & Qgis::ProcessingParameterFlag::Optional ) )
      return false;
  }
  else if ( input.userType() == QMetaType::Type::QString )
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    const QStringList parts = input.toString().split( ';' );
    if ( parts.count() > 1 && !mAllowMultiple )
      return false;
  }
  else
  {
    if ( input.toString().isEmpty() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;
  }
  return true;
}

QString QgsProcessingParameterPointCloudAttribute::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == QMetaType::Type::QVariantList )
  {
    QStringList parts;
    const auto constToList = value.toList();
    for ( const QVariant &val : constToList )
    {
      parts << QgsProcessingUtils::stringToPythonLiteral( val.toString() );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else if ( value.userType() == QMetaType::Type::QStringList )
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

QString QgsProcessingParameterPointCloudAttribute::asScriptCode() const
{
  QString code = u"##%1="_s.arg( mName );
  if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
    code += "optional "_L1;
  code += "attribute "_L1;

  if ( mAllowMultiple )
    code += "multiple "_L1;

  if ( mDefaultToAllAttributes )
    code += "default_to_all_attributes "_L1;

  code += mParentLayerParameterName + ' ';

  code += mDefault.toString();
  return code.trimmed();
}

QString QgsProcessingParameterPointCloudAttribute::asPythonString( const QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = u"QgsProcessingParameterPointCloudAttribute('%1', %2"_s
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;

      code += u", parentLayerParameterName='%1'"_s.arg( mParentLayerParameterName );
      code += u", allowMultiple=%1"_s.arg( mAllowMultiple ? u"True"_s : u"False"_s );
      QgsProcessingContext c;
      code += u", defaultValue=%1"_s.arg( valueAsPythonString( mDefault, c ) );

      if ( mDefaultToAllAttributes )
        code += ", defaultToAllAttributes=True"_L1;

      code += ')';

      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterPointCloudAttribute::dependsOnOtherParameters() const
{
  QStringList depends;
  if ( !mParentLayerParameterName.isEmpty() )
    depends << mParentLayerParameterName;
  return depends;
}

QString QgsProcessingParameterPointCloudAttribute::parentLayerParameterName() const
{
  return mParentLayerParameterName;
}

void QgsProcessingParameterPointCloudAttribute::setParentLayerParameterName( const QString &parentLayerParameterName )
{
  mParentLayerParameterName = parentLayerParameterName;
}

bool QgsProcessingParameterPointCloudAttribute::allowMultiple() const
{
  return mAllowMultiple;
}

void QgsProcessingParameterPointCloudAttribute::setAllowMultiple( bool allowMultiple )
{
  mAllowMultiple = allowMultiple;
}

bool QgsProcessingParameterPointCloudAttribute::defaultToAllAttributes() const
{
  return mDefaultToAllAttributes;
}

void QgsProcessingParameterPointCloudAttribute::setDefaultToAllAttributes( bool enabled )
{
  mDefaultToAllAttributes = enabled;
}

QVariantMap QgsProcessingParameterPointCloudAttribute::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( u"parent_layer"_s, mParentLayerParameterName );
  map.insert( u"allow_multiple"_s, mAllowMultiple );
  map.insert( u"default_to_all_attributes"_s, mDefaultToAllAttributes );
  return map;
}

bool QgsProcessingParameterPointCloudAttribute::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameterName = map.value( u"parent_layer"_s ).toString();
  mAllowMultiple = map.value( u"allow_multiple"_s ).toBool();
  mDefaultToAllAttributes = map.value( u"default_to_all_attributes"_s ).toBool();
  return true;
}

QgsProcessingParameterPointCloudAttribute *QgsProcessingParameterPointCloudAttribute::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  QString parent;
  bool allowMultiple = false;
  bool defaultToAllAttributes = false;
  QString def = definition;

  if ( def.startsWith( "multiple"_L1, Qt::CaseInsensitive ) )
  {
    allowMultiple = true;
    def = def.mid( 8 ).trimmed();
  }

  if ( def.startsWith( "default_to_all_attributes"_L1, Qt::CaseInsensitive ) )
  {
    defaultToAllAttributes = true;
    def = def.mid( 25 ).trimmed();
  }

  const thread_local QRegularExpression re( u"(.*?)\\s+(.*)$"_s );
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

  return new QgsProcessingParameterPointCloudAttribute( name, description, def.isEmpty() ? QVariant() : def, parent, allowMultiple, isOptional, defaultToAllAttributes );
}

//
// QgsProcessingParameterVectorTileDestination
//

QgsProcessingParameterVectorTileDestination::QgsProcessingParameterVectorTileDestination( const QString &name, const QString &description, const QVariant &defaultValue, bool optional, bool createByDefault )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional, createByDefault )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterVectorTileDestination::clone() const
{
  return new QgsProcessingParameterVectorTileDestination( *this );
}

bool QgsProcessingParameterVectorTileDestination::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  QVariant var = input;
  if ( !var.isValid() )
  {
    if ( !defaultValue().isValid() )
      return mFlags & Qgis::ProcessingParameterFlag::Optional;

    var = defaultValue();
  }

  if ( var.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( var );
    var = fromVar.sink;
  }

  if ( var.userType() == qMetaTypeId<QgsProperty>() )
  {
    const QgsProperty p = var.value< QgsProperty >();
    if ( p.propertyType() == Qgis::PropertyType::Static )
    {
      var = p.staticValue();
    }
    else
    {
      return true;
    }
  }

  if ( var.userType() != QMetaType::Type::QString )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  return true;
}

QString QgsProcessingParameterVectorTileDestination::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( !value.isValid() )
    return u"None"_s;

  if ( value.userType() == qMetaTypeId<QgsProperty>() )
    return u"QgsProperty.fromExpression('%1')"_s.arg( value.value< QgsProperty >().asExpression() );

  if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    const QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == Qgis::PropertyType::Static )
    {
      return QgsProcessingUtils::stringToPythonLiteral( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return u"QgsProperty.fromExpression('%1')"_s.arg( fromVar.sink.asExpression() );
    }
  }

  return QgsProcessingUtils::stringToPythonLiteral( value.toString() );
}

QgsProcessingOutputDefinition *QgsProcessingParameterVectorTileDestination::toOutputDefinition() const
{
  return new QgsProcessingOutputVectorTileLayer( name(), description() );
}

QString QgsProcessingParameterVectorTileDestination::defaultFileExtension() const
{
  return QgsProcessingUtils::defaultVectorTileExtension();
}

QString QgsProcessingParameterVectorTileDestination::createFileFilter() const
{
  const QStringList exts = supportedOutputVectorTileLayerExtensions();
  QStringList filters;
  for ( const QString &ext : exts )
  {
    filters << QObject::tr( "%1 files (*.%2)" ).arg( ext.toUpper(), ext.toLower() );
  }
  return filters.join( ";;"_L1 ) + u";;"_s + QObject::tr( "All files (*.*)" );
}

QStringList QgsProcessingParameterVectorTileDestination::supportedOutputVectorTileLayerExtensions() const
{
  QString ext = QgsProcessingUtils::defaultVectorTileExtension();
  return QStringList() << ext;
}

QgsProcessingParameterVectorTileDestination *QgsProcessingParameterVectorTileDestination::fromScriptCode( const QString &name, const QString &description, bool isOptional, const QString &definition )
{
  return new QgsProcessingParameterVectorTileDestination( name, description, definition.isEmpty() ? QVariant() : definition, isOptional );
}
