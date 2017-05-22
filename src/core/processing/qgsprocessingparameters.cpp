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

bool QgsProcessingParameters::isDynamic( const QVariantMap &parameters, const QString &name )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().propertyType() != QgsProperty::StaticProperty;
  else
    return false;
}

QString QgsProcessingParameters::parameterAsString( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsString( context.expressionContext(), definition ? definition->defaultValue().toString() : QString() );

  if ( !val.isValid() || val.toString().isEmpty() )
  {
    // fall back to default
    if ( definition )
      val = definition->defaultValue();
  }

  return val.toString();
}

QString QgsProcessingParameters::parameterAsExpression( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsString( context.expressionContext(), definition ? definition->defaultValue().toString() : QString() );

  if ( val.isValid() && !val.toString().isEmpty() )
  {
    QgsExpression e( val.toString() );
    if ( e.isValid() )
      return val.toString();
  }

  // fall back to default
  if ( definition )
  {
    return definition->defaultValue().toString();
  }
  else
  {
    // it's invalid, but what else are we going to do...?
    return val.toString();
  }
}

double QgsProcessingParameters::parameterAsDouble( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsDouble( context.expressionContext(), definition ? definition->defaultValue().toDouble() : 0 );

  bool ok = false;
  double res = val.toDouble( &ok );
  if ( ok )
    return res;

  // fall back to default
  if ( definition )
    val = definition->defaultValue();
  return val.toDouble();
}

int QgsProcessingParameters::parameterAsInt( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsInt( context.expressionContext(), definition ? definition->defaultValue().toInt() : 0 );

  bool ok = false;
  double dbl = val.toDouble( &ok );
  if ( !ok )
  {
    // fall back to default
    if ( definition )
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

int QgsProcessingParameters::parameterAsEnum( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  int val = parameterAsInt( definition, parameters, name, context );
  const QgsProcessingParameterEnum *enumDef = dynamic_cast< const QgsProcessingParameterEnum *>( definition );
  if ( enumDef && val >= enumDef->options().size() )
  {
    return enumDef->defaultValue().toInt();
  }
  return val;
}

QList<int> QgsProcessingParameters::parameterAsEnums( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariantList resultList;
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    resultList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition ? definition->defaultValue().toString() : QString() );
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

bool QgsProcessingParameters::parameterAsBool( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context )
{
  QVariant def;
  if ( definition )
    def = definition->defaultValue();

  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    return val.value< QgsProperty >().valueAsBool( context.expressionContext(), def.toBool() );
  else if ( val.isValid() )
    return val.toBool();
  else
    return def.toBool();
}

QgsMapLayer *QgsProcessingParameters::parameterAsLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QString layerRef = parameterAsString( definition, parameters, name, context );
  if ( layerRef.isEmpty() )
    layerRef = definition->defaultValue().toString();

  if ( layerRef.isEmpty() )
    return nullptr;

  return QgsProcessingUtils::mapLayerFromString( layerRef, context );
}

QgsRasterLayer *QgsProcessingParameters::parameterAsRasterLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  return qobject_cast< QgsRasterLayer *>( parameterAsLayer( definition, parameters, name, context ) );
}

QgsVectorLayer *QgsProcessingParameters::parameterAsVectorLayer( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  return qobject_cast< QgsVectorLayer *>( parameterAsLayer( definition, parameters, name, context ) );
}

QgsCoordinateReferenceSystem QgsProcessingParameters::parameterAsCrs( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QString crsText = parameterAsString( definition, parameters, name, context );
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

QgsRectangle QgsProcessingParameters::parameterAsExtent( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QVariant val = parameters.value( name );
  QString rectText;
  if ( val.canConvert<QgsProperty>() )
    rectText = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition ? definition->defaultValue().toString() : QString() );
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

QgsPointXY QgsProcessingParameters::parameterAsPoint( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QString pointText = parameterAsString( definition, parameters, name, context );
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

QString QgsProcessingParameters::parameterAsFile( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QString fileText = parameterAsString( definition, parameters, name, context );
  if ( fileText.isEmpty() )
    fileText = definition->defaultValue().toString();
  return fileText;
}

QVariantList QgsProcessingParameters::parameterAsMatrix( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QString resultString;
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    resultString = val.value< QgsProperty >().valueAsString( context.expressionContext(), definition ? definition->defaultValue().toString() : QString() );
  else if ( val.type() == QVariant::List )
    return val.toList();
  else
    resultString = val.toString();

  if ( resultString.isEmpty() && definition )
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

QList<QgsMapLayer *> QgsProcessingParameters::parameterAsLayerList( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QStringList resultStringList;
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition ? definition->defaultValue().toString() : QString() );
  else if ( val.type() == QVariant::List )
  {
    Q_FOREACH ( const QVariant &var, val.toList() )
      resultStringList << var.toString();
  }
  else
    resultStringList << val.toString();

  if ( ( resultStringList.isEmpty() || resultStringList.at( 0 ).isEmpty() ) && definition )
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

  QList<QgsMapLayer *> layers;
  Q_FOREACH ( const QString &s, resultStringList )
  {
    QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( s, context );
    if ( layer )
      layers << layer;
  }

  return layers;
}

QList<double> QgsProcessingParameters::parameterAsRange( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QStringList resultStringList;
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition ? definition->defaultValue().toString() : QString() );
  else if ( val.type() == QVariant::List )
  {
    Q_FOREACH ( const QVariant &var, val.toList() )
      resultStringList << var.toString();
  }
  else
    resultStringList << val.toString();

  if ( ( resultStringList.isEmpty() || ( resultStringList.size() == 1 && resultStringList.at( 0 ).isEmpty() ) ) && definition )
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

QStringList QgsProcessingParameters::parameterAsFields( const QgsProcessingParameterDefinition *definition, const QVariantMap &parameters, const QString &name, QgsProcessingContext &context )
{
  QStringList resultStringList;
  QVariant val = parameters.value( name );
  if ( val.canConvert<QgsProperty>() )
    resultStringList << val.value< QgsProperty >().valueAsString( context.expressionContext(), definition ? definition->defaultValue().toString() : QString() );
  else if ( val.type() == QVariant::List )
  {
    Q_FOREACH ( const QVariant &var, val.toList() )
      resultStringList << var.toString();
  }
  else
    resultStringList.append( val.toString().split( ';' ) );

  if ( ( resultStringList.isEmpty() || resultStringList.at( 0 ).isEmpty() ) && definition )
  {
    resultStringList.clear();
    // check default
    if ( definition->defaultValue().type() == QVariant::List )
    {
      Q_FOREACH ( const QVariant &var, definition->defaultValue().toList() )
        resultStringList << var.toString();
    }
    else
      resultStringList.append( definition->defaultValue().toString().split( ';' ) );
  }

  return resultStringList;
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

QgsProcessingParameterBoolean::QgsProcessingParameterBoolean( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{}

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

QgsProcessingParameterMultipleLayers::QgsProcessingParameterMultipleLayers( const QString &name, const QString &description, LayerType layerType, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mLayerType( layerType )
{

}

bool QgsProcessingParameterMultipleLayers::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

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

QgsProcessingParameterNumber::Type QgsProcessingParameterRange::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterRange::setDataType( const QgsProcessingParameterNumber::Type &dataType )
{
  mDataType = dataType;
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

QgsProcessingParameterString::QgsProcessingParameterString( const QString &name, const QString &description, const QVariant &defaultValue, bool multiLine, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mMultiLine( multiLine )
{

}

bool QgsProcessingParameterString::multiLine() const
{
  return mMultiLine;
}

void QgsProcessingParameterString::setMultiLine( bool multiLine )
{
  mMultiLine = multiLine;
}

QgsProcessingParameterExpression::QgsProcessingParameterExpression( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameter( parentLayerParameterName )
{

}

QString QgsProcessingParameterExpression::parentLayerParameter() const
{
  return mParentLayerParameter;
}

void QgsProcessingParameterExpression::setParentLayerParameter( const QString &parentLayerParameter )
{
  mParentLayerParameter = parentLayerParameter;
}

QgsProcessingParameterTable::QgsProcessingParameterTable( const QString &name, const QString &description, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
{

}

QgsProcessingParameterTableField::QgsProcessingParameterTableField( const QString &name, const QString &description, const QVariant &defaultValue, const QString &parentLayerParameterName, DataType type, bool allowMultiple, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mParentLayerParameter( parentLayerParameterName )
  , mDataType( type )
  , mAllowMultiple( allowMultiple )
{

}

bool QgsProcessingParameterTableField::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
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

QString QgsProcessingParameterTableField::parentLayerParameter() const
{
  return mParentLayerParameter;
}

void QgsProcessingParameterTableField::setParentLayerParameter( const QString &parentLayerParameter )
{
  mParentLayerParameter = parentLayerParameter;
}

QgsProcessingParameterTableField::DataType QgsProcessingParameterTableField::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterTableField::setDataType( const DataType &dataType )
{
  mDataType = dataType;
}

bool QgsProcessingParameterTableField::allowMultiple() const
{
  return mAllowMultiple;
}

void QgsProcessingParameterTableField::setAllowMultiple( bool allowMultiple )
{
  mAllowMultiple = allowMultiple;
}

QgsProcessingParameterVectorLayer::QgsProcessingParameterVectorLayer( const QString &name, const QString &description, const QList<int> &types, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mDataTypes( types )
{

}

bool QgsProcessingParameterVectorLayer::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
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

  // try to load as layer
  if ( QgsProcessingUtils::mapLayerFromString( input.toString(), *context ) )
    return true;

  return false;
}

QList< int > QgsProcessingParameterVectorLayer::dataTypes() const
{
  return mDataTypes;
}

void QgsProcessingParameterVectorLayer::setDataTypes( const QList<int> &types )
{
  mDataTypes = types;
}


QgsProcessingParameterOutputVectorLayer::QgsProcessingParameterOutputVectorLayer( const QString &name, const QString &description, QgsProcessingParameterDefinition::LayerType type, const QVariant &defaultValue, bool optional )
  : QgsProcessingParameterDefinition( name, description, defaultValue, optional )
  , mDataType( type )
{

}

bool QgsProcessingParameterOutputVectorLayer::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() )
    return mFlags & FlagOptional;

  if ( input.canConvert<QgsProperty>() )
  {
    return true;
  }

  if ( input.type() != QVariant::String )
    return false;

  if ( input.toString().isEmpty() )
    return mFlags & FlagOptional;

  return true;
}

QgsProcessingParameterDefinition::LayerType QgsProcessingParameterOutputVectorLayer::dataType() const
{
  return mDataType;
}

void QgsProcessingParameterOutputVectorLayer::setDataType( QgsProcessingParameterDefinition::LayerType type )
{
  mDataType = type;
}
