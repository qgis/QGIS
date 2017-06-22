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
#include "qgsprocessingcontext.h"
#include "qgsprocessingutils.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsprocessingoutputs.h"
#include "qgssettings.h"

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

  QVariant val = parameters.value( definition->name() );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );

  if ( !val.isValid() || val.toString().isEmpty() )
  {
    // fall back to default
    val = definition->defaultValue();
  }

  return val.toString();
}

QString QgsProcessingParameters::parameterAsExpression( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return QString();

  QVariant val = parameters.value( definition->name() );
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

  QVariant val = parameters.value( definition->name() );
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

  QVariant val = parameters.value( definition->name() );
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
    double round = qgsRound( dbl );
    if ( round  > INT_MAX || round < -INT_MAX )
    {
      //double too large to fit in int
      return 0;
    }
    return qRound( dbl );
  }

  return val.toInt();
}

int QgsProcessingParameters::parameterAsEnum( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsProcessingContext &context )
{
  if ( !definition )
    return 0;

  int val = parameterAsInt( definition, parameters, context );
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

  QVariantList resultList;
  QVariant val = parameters.value( definition->name() );
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

  QVariant def = definition->defaultValue();

  QVariant val = parameters.value( definition->name() );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsBool( context.expressionContext(), def.toBool() );
  else if ( val.isValid() )
    return val.toBool();
  else
    return def.toBool();
}

QgsFeatureSink *QgsProcessingParameters::parameterAsSink( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QgsFields &fields,
    QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs,
    QgsProcessingContext &context, QString &destinationIdentifier )
{
  QVariant val;
  if ( definition )
  {
    val = parameters.value( definition->name() );
  }

  QgsProject *destinationProject = nullptr;
  QVariantMap createOptions;
  if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    destinationProject = fromVar.destinationProject;
    createOptions = fromVar.createOptions;
    val = fromVar.sink;
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

  if ( dest.isEmpty() )
    return nullptr;

  std::unique_ptr< QgsFeatureSink > sink( QgsProcessingUtils::createFeatureSink( dest, context, fields, geometryType, crs, createOptions ) );
  destinationIdentifier = dest;

  if ( destinationProject )
    context.addLayerToLoadOnCompletion( destinationIdentifier, QgsProcessingContext::LayerDetails( definition ? definition->description() : QString(), destinationProject ) );

  return sink.release();
}

QgsFeatureSource *QgsProcessingParameters::parameterAsSource( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return nullptr;

  QVariant val = parameters.value( definition->name() );

  bool selectedFeaturesOnly = false;
  if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
    selectedFeaturesOnly = fromVar.selectedFeaturesOnly;
    val = fromVar.source;
  }

  QString layerRef;
  if ( val.canConvert<QgsProperty>() )
  {
    layerRef = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }
  else if ( !val.isValid() || val.toString().isEmpty() )
  {
    // fall back to default
    layerRef = definition->defaultValue().toString();
  }
  else
  {
    layerRef = val.toString();
  }

  if ( layerRef.isEmpty() )
    return nullptr;

  QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( layerRef, context ) );
  if ( !vl )
    return nullptr;

  if ( selectedFeaturesOnly )
  {
    return new QgsProcessingFeatureSource( new QgsVectorLayerSelectedFeatureSource( vl ), context, true );
  }
  else
  {
    return new QgsProcessingFeatureSource( vl, context );
  }
}

QgsMapLayer *QgsProcessingParameters::parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return nullptr;

  QVariant val = parameters.value( definition->name() );
  if ( val.canConvert<QgsProperty>() )
  {
    val = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  }

  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) ) )
  {
    return layer;
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

QString QgsProcessingParameters::parameterAsRasterOutputLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  QVariant val;
  if ( definition )
  {
    val = parameters.value( definition->name() );
  }

  QgsProject *destinationProject = nullptr;
  QVariantMap createOptions;
  if ( val.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    // input is a QgsProcessingOutputLayerDefinition - get extra properties from it
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( val );
    destinationProject = fromVar.destinationProject;
    createOptions = fromVar.createOptions;
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

  if ( destinationProject )
    context.addLayerToLoadOnCompletion( dest, QgsProcessingContext::LayerDetails( definition ? definition->description() : QString(), destinationProject ) );

  return dest;
}

QString QgsProcessingParameters::parameterAsFileOutput( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  QVariant val;
  if ( definition )
  {
    val = parameters.value( definition->name() );
  }

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

  return dest;
}

QgsVectorLayer *QgsProcessingParameters::parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  return qobject_cast< QgsVectorLayer *>( parameterAsLayer( definition, parameters, context ) );
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QgsCoordinateReferenceSystem();

  QString crsText = parameterAsString( definition, parameters, context );
  if ( crsText.isEmpty() )
    crsText = definition->defaultValue().toString();

  if ( crsText.isEmpty() )
    return QgsCoordinateReferenceSystem();

  // maybe special string
  if ( context.project() && crsText.compare( QStringLiteral( "ProjectCrs" ), Qt::CaseInsensitive ) == 0 )
    return context.project()->crs();

  // maybe a map layer reference
  if ( QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( crsText, context ) )
    return layer->crs();

  // else CRS from string
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( crsText );
  return crs;
}

QgsRectangle QgsProcessingParameters::parameterAsExtent( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QgsRectangle();

  QVariant val = parameters.value( definition->name() );
  QString rectText;
  if ( val.canConvert<QgsProperty>() )
    rectText = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else
    rectText = val.toString();

  if ( rectText.isEmpty() )
    return QgsRectangle();

  QStringList parts = rectText.split( ',' );
  if ( parts.count() == 4 )
  {
    bool xMinOk = false;
    double xMin = parts.at( 0 ).toDouble( &xMinOk );
    bool xMaxOk = false;
    double xMax = parts.at( 1 ).toDouble( &xMaxOk );
    bool yMinOk = false;
    double yMin = parts.at( 2 ).toDouble( &yMinOk );
    bool yMaxOk = false;
    double yMax = parts.at( 3 ).toDouble( &yMaxOk );
    if ( xMinOk && xMaxOk && yMinOk && yMaxOk )
      return QgsRectangle( xMin, yMin, xMax, yMax );
  }

  // try as layer extent
  if ( QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( rectText, context ) )
    return layer->extent();

  return QgsRectangle();
}

QgsPointXY QgsProcessingParameters::parameterAsPoint( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QgsPointXY();

  QString pointText = parameterAsString( definition, parameters, context );
  if ( pointText.isEmpty() )
    pointText = definition->defaultValue().toString();

  if ( pointText.isEmpty() )
    return QgsPointXY();

  QStringList parts = pointText.split( ',' );
  if ( parts.count() == 2 )
  {
    bool xOk = false;
    double x = parts.at( 0 ).toDouble( &xOk );
    bool yOk = false;
    double y = parts.at( 1 ).toDouble( &yOk );
    if ( xOk && yOk )
      return QgsPointXY( x, y );
  }

  return QgsPointXY();
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

QVariantList QgsProcessingParameters::parameterAsMatrix( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QVariantList();

  QString resultString;
  QVariant val = parameters.value( definition->name() );
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

  QVariant val = parameters.value( definition->name() );
  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( val ) ) )
  {
    return QList<QgsMapLayer *>() << layer;
  }

  QList<QgsMapLayer *> layers;

  QStringList resultStringList;

  if ( val.canConvert<QgsProperty>() )
    resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition->defaultValue().toString() );
  else if ( val.type() == QVariant::List )
  {
    Q_FOREACH ( const QVariant &var, val.toList() )
    {
      if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( var ) ) )
      {
        layers << layer;
      }
      else
      {
        resultStringList << var.toString();
      }
    }
  }
  else if ( val.type() == QVariant::StringList )
  {
    Q_FOREACH ( const QString &s, val.toStringList() )
    {
      resultStringList << s;
    }
  }
  else
    resultStringList << val.toString();

  if ( ( resultStringList.isEmpty() || resultStringList.at( 0 ).isEmpty() ) )
  {
    resultStringList.clear();
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
          resultStringList << var.toString();
        }
      }
    }
    else
      resultStringList << definition->defaultValue().toString();
  }

  Q_FOREACH ( const QString &s, resultStringList )
  {
    QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( s, context );
    if ( layer )
      layers << layer;
  }

  return layers;
}

QList<double> QgsProcessingParameters::parameterAsRange( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, QgsProcessingContext &context )
{
  if ( !definition )
    return QList<double>();

  QStringList resultStringList;
  QVariant val = parameters.value( definition->name() );
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
  QVariant val = parameters.value( definition->name() );
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

QgsProcessingParameterDefinition *QgsProcessingParameters::parameterFromVariantMap( const QVariantMap &map )
{
  QString type = map.value( QStringLiteral( "parameter_type" ) ).toString();
  QString name = map.value( QStringLiteral( "name" ) ).toString();
  std::unique_ptr< QgsProcessingParameterDefinition > def;
  if ( type == QStringLiteral( "boolean" ) )
    def.reset( new QgsProcessingParameterBoolean( name ) );
  else if ( type == QStringLiteral( "crs" ) )
    def.reset( new QgsProcessingParameterCrs( name ) );
  else if ( type == QStringLiteral( "layer" ) )
    def.reset( new QgsProcessingParameterMapLayer( name ) );
  else if ( type == QStringLiteral( "extent" ) )
    def.reset( new QgsProcessingParameterExtent( name ) );
  else if ( type == QStringLiteral( "point" ) )
    def.reset( new QgsProcessingParameterPoint( name ) );
  else if ( type == QStringLiteral( "file" ) )
    def.reset( new QgsProcessingParameterFile( name ) );
  else if ( type == QStringLiteral( "matrix" ) )
    def.reset( new QgsProcessingParameterMatrix( name ) );
  else if ( type == QStringLiteral( "multilayer" ) )
    def.reset( new QgsProcessingParameterMultipleLayers( name ) );
  else if ( type == QStringLiteral( "number" ) )
    def.reset( new QgsProcessingParameterNumber( name ) );
  else if ( type == QStringLiteral( "range" ) )
    def.reset( new QgsProcessingParameterRange( name ) );
  else if ( type == QStringLiteral( "raster" ) )
    def.reset( new QgsProcessingParameterRasterLayer( name ) );
  else if ( type == QStringLiteral( "enum" ) )
    def.reset( new QgsProcessingParameterEnum( name ) );
  else if ( type == QStringLiteral( "string" ) )
    def.reset( new QgsProcessingParameterString( name ) );
  else if ( type == QStringLiteral( "expression" ) )
    def.reset( new QgsProcessingParameterExpression( name ) );
  else if ( type == QStringLiteral( "vector" ) )
    def.reset( new QgsProcessingParameterVectorLayer( name ) );
  else if ( type == QStringLiteral( "field" ) )
    def.reset( new QgsProcessingParameterField( name ) );
  else if ( type == QStringLiteral( "source" ) )
    def.reset( new QgsProcessingParameterFeatureSource( name ) );
  else if ( type == QStringLiteral( "sink" ) )
    def.reset( new QgsProcessingParameterFeatureSink( name ) );
  else if ( type == QStringLiteral( "vectorOut" ) )
    def.reset( new QgsProcessingParameterVectorOutput( name ) );
  else if ( type == QStringLiteral( "rasterOut" ) )
    def.reset( new QgsProcessingParameterRasterOutput( name ) );
  else if ( type == QStringLiteral( "fileOut" ) )
    def.reset( new QgsProcessingParameterFileOutput( name ) );
  else if ( type == QStringLiteral( "folderOut" ) )
    def.reset( new QgsProcessingParameterFolderOutput( name ) );

  if ( !def )
    return nullptr;

  def->fromVariantMap( map );
  return def.release();
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
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.type() == QVariant::String && input.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterDefinition::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  return value.toString().prepend( '\'' ).append( '\'' );
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

QgsProcessingParameterBoolean::QgsProcessingParameterBoolean( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{}

QString QgsProcessingParameterBoolean::valueAsPythonString( const QVariant &val, QgsProcessingContext & ) const
{
  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );
  return val.toBool() ? QStringLiteral( "True" ) : QStringLiteral( "False" );
}

QgsProcessingParameterCrs::QgsProcessingParameterCrs( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

bool QgsProcessingParameterCrs::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() != QVariant::String || input.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterCrs::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), value );
  QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
  if ( layer )
    return QgsProcessingUtils::normalizeLayerSource( layer->source() ).prepend( '\'' ).append( '\'' );

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QgsProcessingParameterMapLayer::QgsProcessingParameterMapLayer( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

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
  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
  return layer ? QgsProcessingUtils::normalizeLayerSource( layer->source() ).prepend( '\'' ).append( '\'' ) : QString();
}

QgsProcessingParameterExtent::QgsProcessingParameterExtent( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

bool QgsProcessingParameterExtent::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
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

  QStringList parts = input.toString().split( ',' );
  if ( parts.count() == 4 )
  {
    bool xMinOk = false;
    ( void )parts.at( 0 ).toDouble( &xMinOk );
    bool xMaxOk = false;
    ( void )parts.at( 1 ).toDouble( &xMaxOk );
    bool yMinOk = false;
    ( void )parts.at( 2 ).toDouble( &yMinOk );
    bool yMaxOk = false;
    ( void )parts.at( 3 ).toDouble( &yMaxOk );
    if ( xMinOk && xMaxOk && yMinOk && yMaxOk )
      return true;
  }

  // try as layer extent
  if ( QgsProcessingUtils::mapLayerFromString( input.toString(), *context ) )
    return true;

  return false;
}

QString QgsProcessingParameterExtent::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), value );
  QgsMapLayer *layer = QgsProcessingParameters::parameterAsLayer( this, p, context );
  if ( layer )
    return QgsProcessingUtils::normalizeLayerSource( layer->source() ).prepend( '\'' ).append( '\'' );

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QgsProcessingParameterPoint::QgsProcessingParameterPoint( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

bool QgsProcessingParameterPoint::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() == QVariant::String )
  {
    if ( input.toString().isEmpty() )
      return mFlags & FlagOptional;
  }

  QStringList parts = input.toString().split( ',' );
  if ( parts.count() == 2 )
  {
    bool xOk = false;
    ( void )parts.at( 0 ).toDouble( &xOk );
    bool yOk = false;
    ( void )parts.at( 1 ).toDouble( &yOk );
    return xOk && yOk;
  }
  else
    return false;
}

QgsProcessingParameterFile::QgsProcessingParameterFile( const QString &name, const QString &description, Behavior behavior, const QString &extension, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mBehavior( behavior )
  , mExtension( extension )
{

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

QgsProcessingParameterMatrix::QgsProcessingParameterMatrix( const QString &name, const QString &description, int numberRows, bool fixedNumberRows, const QStringList &headers, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mHeaders( headers )
  , mNumberRows( numberRows )
  , mFixedNumberRows( fixedNumberRows )
{

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
        parts2 << v2.toString();
      }
      parts << parts2.join( ',' ).prepend( '[' ).append( ']' );
    }
    else
    {
      parts << v.toString();
    }
  }

  return parts.join( ',' ).prepend( '[' ).append( ']' );
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

QgsProcessingParameterMultipleLayers::QgsProcessingParameterMultipleLayers( const QString &name, const QString &description, LayerType layerType, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mLayerType( layerType )
{

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
      parts << QgsProcessingUtils::normalizeLayerSource( layer->source() ).prepend( '\'' ).append( '\'' );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }

  return QgsProcessingParameterDefinition::valueAsPythonString( value, context );
}

QgsProcessingParameterDefinition::LayerType QgsProcessingParameterMultipleLayers::layerType() const
{
  return mLayerType;
}

void QgsProcessingParameterMultipleLayers::setLayerType( LayerType type )
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
  mLayerType = static_cast< LayerType >( map.value( QStringLiteral( "layer_type" ) ).toInt() );
  mMinimumNumberInputs = map.value( QStringLiteral( "min_inputs" ) ).toInt();
  return true;
}

QgsProcessingParameterNumber::QgsProcessingParameterNumber( const QString &name, const QString &description, Type type, const QVariant &defaultValue, bool optional, double minValue, double maxValue )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mMin( minValue )
  , mMax( maxValue )
  , mDataType( type )
{

}

bool QgsProcessingParameterNumber::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  bool ok = false;
  double res = input.toDouble( &ok );
  if ( !ok )
    return mFlags & FlagOptional;

  if ( res < mMin || res > mMax )
    return false;

  return true;
}

QString QgsProcessingParameterNumber::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  return value.toString();
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

void QgsProcessingParameterNumber::setDataType( const Type &dataType )
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

QgsProcessingParameterRange::QgsProcessingParameterRange( const QString &name, const QString &description, QgsProcessingParameterNumber::Type type, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mDataType( type )
{

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

QgsProcessingParameterNumber::Type QgsProcessingParameterRange::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterRange::setDataType( const QgsProcessingParameterNumber::Type &dataType )
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

QgsProcessingParameterRasterLayer::QgsProcessingParameterRasterLayer( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

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
  if ( QgsProcessingUtils::mapLayerFromString( input.toString(), *context ) )
    return true;

  return false;
}

QString QgsProcessingParameterRasterLayer::valueAsPythonString( const QVariant &val, QgsProcessingContext &context ) const
{
  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsRasterLayer *layer = QgsProcessingParameters::parameterAsRasterLayer( this, p, context );
  return layer ? QgsProcessingUtils::normalizeLayerSource( layer->source() ).prepend( '\'' ).append( '\'' ) : QString();
}

QgsProcessingParameterEnum::QgsProcessingParameterEnum( const QString &name, const QString &description, const QStringList &options, bool allowMultiple, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mOptions( options )
  , mAllowMultiple( allowMultiple )
{

}

bool QgsProcessingParameterEnum::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() == QVariant::List )
  {
    if ( !mAllowMultiple )
      return false;

    Q_FOREACH ( const QVariant &val, input.toList() )
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

QgsProcessingParameterString::QgsProcessingParameterString( const QString &name, const QString &description, const QVariant &defaultValue, bool multiLine, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mMultiLine( multiLine )
{

}

QString QgsProcessingParameterString::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QString s = value.toString();
  s.replace( '\n', QStringLiteral( "\\n" ) );
  return s.prepend( '\'' ).append( '\'' );
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

QgsProcessingParameterExpression::QgsProcessingParameterExpression( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameter( parentLayerParameterName )
{

}

QString QgsProcessingParameterExpression::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  QString s = value.toString();
  s.replace( '\n', QStringLiteral( "\\n" ) );
  return s.prepend( '\'' ).append( '\'' );
}

QString QgsProcessingParameterExpression::parentLayerParameter() const
{
  return mParentLayerParameter;
}

void QgsProcessingParameterExpression::setParentLayerParameter( const QString &parentLayerParameter )
{
  mParentLayerParameter = parentLayerParameter;
}

QVariantMap QgsProcessingParameterExpression::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "parent_layer" ), mParentLayerParameter );
  return map;
}

bool QgsProcessingParameterExpression::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameter = map.value( QStringLiteral( "parent_layer" ) ).toString();
  return true;
}

QgsProcessingParameterVectorLayer::QgsProcessingParameterVectorLayer( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

bool QgsProcessingParameterVectorLayer::checkValueIsAcceptable( const QVariant &var, QgsProcessingContext *context ) const
{
  if ( !var.isValid() )
    return mFlags & FlagOptional;

  if ( var.canConvert<QgsProperty>() )
  {
    return true;
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
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context ) )
    return true;

  return false;
}

QString QgsProcessingParameterVectorLayer::valueAsPythonString( const QVariant &val, QgsProcessingContext &context ) const
{
  if ( val.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( val.value< QgsProperty >().asExpression() );

  QVariantMap p;
  p.insert( name(), val );
  QgsVectorLayer *layer = QgsProcessingParameters::parameterAsVectorLayer( this, p, context );
  return layer ? QgsProcessingUtils::normalizeLayerSource( layer->source() ).prepend( '\'' ).append( '\'' ) : QString();
}

QgsProcessingParameterField::QgsProcessingParameterField( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, DataType type, bool allowMultiple, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameter( parentLayerParameterName )
  , mDataType( type )
  , mAllowMultiple( allowMultiple )
{

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
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.type() == QVariant::List )
  {
    QStringList parts;
    Q_FOREACH ( const QVariant &val, value.toList() )
    {
      parts << val.toString().prepend( '\'' ).append( '\'' );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }
  else if ( value.type() == QVariant::StringList )
  {
    QStringList parts;
    Q_FOREACH ( QString s, value.toStringList() )
    {
      parts << s.prepend( '\'' ).append( '\'' );
    }
    return parts.join( ',' ).prepend( '[' ).append( ']' );
  }

  return value.toString().prepend( '\'' ).append( '\'' );
}

QString QgsProcessingParameterField::parentLayerParameter() const
{
  return mParentLayerParameter;
}

void QgsProcessingParameterField::setParentLayerParameter( const QString &parentLayerParameter )
{
  mParentLayerParameter = parentLayerParameter;
}

QgsProcessingParameterField::DataType QgsProcessingParameterField::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterField::setDataType( const DataType &dataType )
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
  map.insert( QStringLiteral( "parent_layer" ), mParentLayerParameter );
  map.insert( QStringLiteral( "data_type" ), mDataType );
  map.insert( QStringLiteral( "allow_multiple" ), mAllowMultiple );
  return map;
}

bool QgsProcessingParameterField::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mParentLayerParameter = map.value( QStringLiteral( "parent_layer" ) ).toString();
  mDataType = static_cast< DataType >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  mAllowMultiple = map.value( QStringLiteral( "allow_multiple" ) ).toBool();
  return true;
}

QgsProcessingParameterFeatureSource::QgsProcessingParameterFeatureSource( const QString &name, const QString &description, const QList<int> &types, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mDataTypes( types )
{

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

  if ( var.canConvert<QgsProperty>() )
  {
    return true;
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
  if ( QgsProcessingUtils::mapLayerFromString( var.toString(), *context ) )
    return true;

  return false;
}

QString QgsProcessingParameterFeatureSource::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingFeatureSourceDefinition>() )
  {
    QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );
    if ( fromVar.source.propertyType() == QgsProperty::StaticProperty )
    {
      return QStringLiteral( "QgsProcessingFeatureSourceDefinition('%1', %2)" ).arg( fromVar.source.staticValue().toString(),
             fromVar.selectedFeaturesOnly ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );
    }
    else
    {
      return QStringLiteral( "QgsProcessingFeatureSourceDefinition(QgsProperty.fromExpression('%1'), %2)" ).arg( fromVar.source.asExpression(),
             fromVar.selectedFeaturesOnly ? QStringLiteral( "True" ) : QStringLiteral( "False" ) );
    }
  }

  return value.toString().prepend( '\'' ).append( '\'' );
}

QList< int > QgsProcessingParameterFeatureSource::dataTypes() const
{
  return mDataTypes;
}

void QgsProcessingParameterFeatureSource::setDataTypes( const QList<int> &types )
{
  mDataTypes = types;
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


QgsProcessingParameterFeatureSink::QgsProcessingParameterFeatureSink( const QString &name, const QString &description, QgsProcessingParameterDefinition::LayerType type, const QVariant &defaultValue, bool optional )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional )
  , mDataType( type )
{

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
    return true;
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterFeatureSink::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == QgsProperty::StaticProperty )
    {
      return QStringLiteral( "QgsProcessingOutputLayerDefinition('%1')" ).arg( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return QStringLiteral( "QgsProcessingOutputLayerDefinition(QgsProperty.fromExpression('%1'))" ).arg( fromVar.sink.asExpression() );
    }
  }

  return value.toString().prepend( '\'' ).append( '\'' );
}

QgsProcessingOutputDefinition *QgsProcessingParameterFeatureSink::toOutputDefinition() const
{
  return new QgsProcessingOutputVectorLayer( name(), description(), mDataType );
}

QString QgsProcessingParameterFeatureSink::defaultFileExtension() const
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

QgsProcessingParameterDefinition::LayerType QgsProcessingParameterFeatureSink::dataType() const
{
  return mDataType;
}

bool QgsProcessingParameterFeatureSink::hasGeometry() const
{
  switch ( mDataType )
  {
    case TypeAny:
    case TypeVectorAny:
    case TypeVectorPoint:
    case TypeVectorLine:
    case TypeVectorPolygon:
    case TypeTable:
      return true;

    case TypeRaster:
    case TypeFile:
      return false;
  }
  return true;
}

void QgsProcessingParameterFeatureSink::setDataType( QgsProcessingParameterDefinition::LayerType type )
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
  mDataType = static_cast< QgsProcessingParameterDefinition::LayerType >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  return true;
}

QString QgsProcessingParameterFeatureSink::generateTemporaryDestination() const
{
  if ( supportsNonFileBasedOutputs() )
    return QStringLiteral( "memory:" );
  else
    return QgsProcessingDestinationParameter::generateTemporaryDestination();
}

QgsProcessingParameterRasterOutput::QgsProcessingParameterRasterOutput( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional )
{}

bool QgsProcessingParameterRasterOutput::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
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
    return true;
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterRasterOutput::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == QgsProperty::StaticProperty )
    {
      return QStringLiteral( "QgsProcessingOutputLayerDefinition('%1')" ).arg( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return QStringLiteral( "QgsProcessingOutputLayerDefinition(QgsProperty.fromExpression('%1'))" ).arg( fromVar.sink.asExpression() );
    }
  }

  return value.toString().prepend( '\'' ).append( '\'' );
}

QgsProcessingOutputDefinition *QgsProcessingParameterRasterOutput::toOutputDefinition() const
{
  return new QgsProcessingOutputRasterLayer( name(), description() );
}

QString QgsProcessingParameterRasterOutput::defaultFileExtension() const
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "Processing/DefaultOutputRasterLayerExt" ), QStringLiteral( "tif" ), QgsSettings::Core ).toString();
}


QgsProcessingParameterFileOutput::QgsProcessingParameterFileOutput( const QString &name, const QString &description, const QString &fileFilter, const QVariant &defaultValue, bool optional )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional )
  , mFileFilter( fileFilter.isEmpty() ? QObject::tr( "All files (*.*)" ) : fileFilter )
{

}

bool QgsProcessingParameterFileOutput::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
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
    return true;
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  // possible enhancement - check that value is compatible with file filter?

  return true;
}

QString QgsProcessingParameterFileOutput::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == QgsProperty::StaticProperty )
    {
      return QStringLiteral( "QgsProcessingOutputLayerDefinition('%1')" ).arg( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return QStringLiteral( "QgsProcessingOutputLayerDefinition(QgsProperty.fromExpression('%1'))" ).arg( fromVar.sink.asExpression() );
    }
  }

  return value.toString().prepend( '\'' ).append( '\'' );
}

QgsProcessingOutputDefinition *QgsProcessingParameterFileOutput::toOutputDefinition() const
{
  return nullptr;
}

QString QgsProcessingParameterFileOutput::defaultFileExtension() const
{
  if ( mFileFilter.isEmpty() || mFileFilter == QObject::tr( "All files (*.*)" ) )
    return QStringLiteral( "file" );

  // get first extension from filter
  QRegularExpression rx( ".*?\\(\\*\\.([a-zA-Z0-9._]+).*" );
  QRegularExpressionMatch match = rx.match( mFileFilter );
  if ( !match.hasMatch() )
    return QStringLiteral( "file" );

  return match.captured( 1 );
}

QString QgsProcessingParameterFileOutput::fileFilter() const
{
  return mFileFilter;
}

void QgsProcessingParameterFileOutput::setFileFilter( const QString &fileFilter )
{
  mFileFilter = fileFilter;
}

QVariantMap QgsProcessingParameterFileOutput::toVariantMap() const
{
  QVariantMap map = QgsProcessingDestinationParameter::toVariantMap();
  map.insert( QStringLiteral( "file_filter" ), mFileFilter );
  return map;
}

bool QgsProcessingParameterFileOutput::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingDestinationParameter::fromVariantMap( map );
  mFileFilter = map.value( QStringLiteral( "file_filter" ) ).toString();
  return true;

}

QgsProcessingParameterFolderOutput::QgsProcessingParameterFolderOutput( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional )
{}

bool QgsProcessingParameterFolderOutput::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  QVariant var = input;
  if ( !var.isValid() )
    return mFlags & FlagOptional;

  if ( var.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QgsProcessingOutputDefinition *QgsProcessingParameterFolderOutput::toOutputDefinition() const
{
  return new QgsProcessingOutputFolder( name(), description() );
}

QString QgsProcessingParameterFolderOutput::defaultFileExtension() const
{
  return QString();
}

QgsProcessingDestinationParameter::QgsProcessingDestinationParameter( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QVariantMap QgsProcessingDestinationParameter::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap();
  map.insert( QStringLiteral( "supports_non_file_outputs" ), mSupportsNonFileBasedOutputs );
  return map;
}

bool QgsProcessingDestinationParameter::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map );
  mSupportsNonFileBasedOutputs = map.value( QStringLiteral( "supports_non_file_outputs" ) ).toBool();
  return true;
}

QString QgsProcessingDestinationParameter::generateTemporaryDestination() const
{
  return QgsProcessingUtils::generateTempFilename( name() + '.' + defaultFileExtension() );
}

QgsProcessingParameterVectorOutput::QgsProcessingParameterVectorOutput( const QString &name, const QString &description, QgsProcessingParameterDefinition::LayerType type, const QVariant &defaultValue, bool optional )
  : QgsProcessingDestinationParameter( name, description, defaultValue, optional )
  , mDataType( type )
{

}

bool QgsProcessingParameterVectorOutput::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
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
    return true;
  }

  if ( var.type() != QVariant::String )
    return false;

  if ( var.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QString QgsProcessingParameterVectorOutput::valueAsPythonString( const QVariant &value, QgsProcessingContext & ) const
{
  if ( value.canConvert<QgsProperty>() )
    return QStringLiteral( "QgsProperty.fromExpression('%1')" ).arg( value.value< QgsProperty >().asExpression() );

  if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
  {
    QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
    if ( fromVar.sink.propertyType() == QgsProperty::StaticProperty )
    {
      return QStringLiteral( "QgsProcessingOutputLayerDefinition('%1')" ).arg( fromVar.sink.staticValue().toString() );
    }
    else
    {
      return QStringLiteral( "QgsProcessingOutputLayerDefinition(QgsProperty.fromExpression('%1'))" ).arg( fromVar.sink.asExpression() );
    }
  }

  return value.toString().prepend( '\'' ).append( '\'' );
}

QgsProcessingOutputDefinition *QgsProcessingParameterVectorOutput::toOutputDefinition() const
{
  return new QgsProcessingOutputVectorLayer( name(), description(), mDataType );
}

QString QgsProcessingParameterVectorOutput::defaultFileExtension() const
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

QgsProcessingParameterDefinition::LayerType QgsProcessingParameterVectorOutput::dataType() const
{
  return mDataType;
}

bool QgsProcessingParameterVectorOutput::hasGeometry() const
{
  switch ( mDataType )
  {
    case TypeAny:
    case TypeVectorAny:
    case TypeVectorPoint:
    case TypeVectorLine:
    case TypeVectorPolygon:
    case TypeTable:
      return true;

    case TypeRaster:
    case TypeFile:
      return false;
  }
  return true;
}

void QgsProcessingParameterVectorOutput::setDataType( QgsProcessingParameterDefinition::LayerType type )
{
  mDataType = type;
}

QVariantMap QgsProcessingParameterVectorOutput::toVariantMap() const
{
  QVariantMap map = QgsProcessingDestinationParameter::toVariantMap();
  map.insert( QStringLiteral( "data_type" ), mDataType );
  return map;
}

bool QgsProcessingParameterVectorOutput::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingDestinationParameter::fromVariantMap( map );
  mDataType = static_cast< QgsProcessingParameterDefinition::LayerType >( map.value( QStringLiteral( "data_type" ) ).toInt() );
  return true;
}
