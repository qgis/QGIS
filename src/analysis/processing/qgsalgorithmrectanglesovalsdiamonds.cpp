/***************************************************************************
                         qgsalgorithmrectanglesovalsdiamonds.cpp
                         ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmrectanglesovalsdiamonds.h"
#include "qgsapplication.h"
#include "qgslinestring.h"
#include "qgspolygon.h"

///@cond PRIVATE

QString QgsRectanglesOvalsDiamondsAlgorithm::name() const
{
  return QStringLiteral( "rectanglesovalsdiamonds" );
}

QString QgsRectanglesOvalsDiamondsAlgorithm::displayName() const
{
  return QObject::tr( "Rectangles, ovals, diamonds" );
}

QStringList QgsRectanglesOvalsDiamondsAlgorithm::tags() const
{
  return QObject::tr( "buffer,grow,fixed,variable,distance,rectangle,oval,diamond,point" ).split( ',' );
}

QString QgsRectanglesOvalsDiamondsAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsRectanglesOvalsDiamondsAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsRectanglesOvalsDiamondsAlgorithm::shortHelpString() const
{
  return QObject::tr( "Creates rectangle, oval or diamond-shaped polygons from the input point layer using "
                      "specified width, height and (optional) rotation values. Multipart inputs should be promoted "
                      "to singleparts first." );
}

QString QgsRectanglesOvalsDiamondsAlgorithm::outputName() const
{
  return QObject::tr( "Polygon" );
}

QList<int> QgsRectanglesOvalsDiamondsAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPoint;
}

QgsProcessing::SourceType QgsRectanglesOvalsDiamondsAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsWkbTypes::Type QgsRectanglesOvalsDiamondsAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  QgsWkbTypes::Type outputWkbType = QgsWkbTypes::Polygon;
  if ( QgsWkbTypes::hasM( inputWkbType ) )
  {
    outputWkbType = QgsWkbTypes::addM( outputWkbType );
  }
  if ( QgsWkbTypes::hasZ( inputWkbType ) )
  {
    outputWkbType = QgsWkbTypes::addZ( outputWkbType );
  }

  return outputWkbType;
}

QgsRectanglesOvalsDiamondsAlgorithm *QgsRectanglesOvalsDiamondsAlgorithm::createInstance() const
{
  return new QgsRectanglesOvalsDiamondsAlgorithm();
}

void QgsRectanglesOvalsDiamondsAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "SHAPE" ), QObject::tr( "Shape" ), QStringList() << QObject::tr( "Rectangle" ) << QObject::tr( "Diamond" ) << QObject::tr( "Oval" ), false, 0 ) );

  auto widthParam = std::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "WIDTH" ), QObject::tr( "Width" ), 1.0, QStringLiteral( "INPUT" ), false, 0.0 );
  widthParam->setIsDynamic( true );
  widthParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Width" ), QObject::tr( "Width" ), QgsPropertyDefinition::DoublePositive ) );
  widthParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( widthParam.release() );

  auto heightParam = std::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "HEIGHT" ), QObject::tr( "Height" ), 1.0, QStringLiteral( "INPUT" ), false, 0.0 );
  heightParam->setIsDynamic( true );
  heightParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Height" ), QObject::tr( "Height" ), QgsPropertyDefinition::DoublePositive ) );
  heightParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( heightParam.release() );

  auto rotationParam = std::make_unique < QgsProcessingParameterNumber >( QStringLiteral( "ROTATION" ), QObject::tr( "Rotation" ), QgsProcessingParameterNumber::Double, 0.0, true, -360.0, 360.0 );
  rotationParam->setIsDynamic( true );
  rotationParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Rotation" ), QObject::tr( "Rotation" ), QgsPropertyDefinition::Double ) );
  rotationParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( rotationParam.release() );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer, 5, false, 1 ) );
}

bool QgsRectanglesOvalsDiamondsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mShape = parameterAsEnum( parameters, QStringLiteral( "SHAPE" ), context );

  mWidth = parameterAsDouble( parameters, QStringLiteral( "WIDTH" ), context );
  mDynamicWidth = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "WIDTH" ) );
  if ( mDynamicWidth )
    mWidthProperty = parameters.value( QStringLiteral( "WIDTH" ) ).value< QgsProperty >();

  mHeight = parameterAsDouble( parameters, QStringLiteral( "HEIGHT" ), context );
  mDynamicHeight = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "HEIGHT" ) );
  if ( mDynamicHeight )
    mHeightProperty = parameters.value( QStringLiteral( "HEIGHT" ) ).value< QgsProperty >();

  mRotation = parameterAsDouble( parameters, QStringLiteral( "ROTATION" ), context );
  mDynamicRotation = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ROTATION" ) );
  if ( mDynamicRotation )
    mRotationProperty = parameters.value( QStringLiteral( "ROTATION" ) ).value< QgsProperty >();

  mSegments = parameterAsDouble( parameters, QStringLiteral( "SEGMENTS" ), context );

  return true;
}

QgsFeatureList QgsRectanglesOvalsDiamondsAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature outFeature = feature;
  if ( outFeature.hasGeometry() )
  {
    const QgsGeometry geometry = outFeature.geometry();
    if ( geometry.isMultipart() )
    {
      throw QgsProcessingException( QObject::tr( "Multipart geometry. Please promote input layer to singleparts first." ) );
    }

    double width = mWidth;
    if ( mDynamicWidth )
      width = mWidthProperty.valueAsDouble( context.expressionContext(), width );

    double height = mHeight;
    if ( mDynamicHeight )
      height = mHeightProperty.valueAsDouble( context.expressionContext(), height );

    double rotation = mRotation;
    if ( mDynamicRotation )
      rotation = mRotationProperty.valueAsDouble( context.expressionContext(), rotation );

    if ( width == 0 || height == 0 )
    {
      throw QgsProcessingException( QObject::tr( "Width and height should be greater than 0." ) );
    }

    const double phi = rotation * M_PI / 180;
    const double xOffset = width / 2.0;
    const double yOffset = height / 2.0;
    const QgsPointXY point = geometry.asPoint();
    const double x = point.x();
    const double y = point.y();

    QVector< double > ringX( 5 );
    QVector< double > ringY( 5 );

    switch ( mShape )
    {
      case 0:
        // rectangle
        ringX = { -xOffset + x, -xOffset + x, xOffset + x, xOffset + x, -xOffset + x };
        ringY = { -yOffset + y, yOffset + y, yOffset + y, -yOffset + y, -yOffset + y };
        break;
      case 1:
        // diamond
        ringX = { x, -xOffset + x, x, xOffset + x, x };
        ringY = { -yOffset + y, y, yOffset + y, y, -yOffset + y };
        break;
      case 2:
        // oval
        ringX.resize( mSegments + 1 );
        ringY.resize( mSegments + 1 );
        for ( int i = 0; i < mSegments; i ++ )
        {
          const double t = ( 2 * M_PI ) / mSegments * i;
          ringX[ i ] = xOffset * cos( t ) + x;
          ringY[ i ] = yOffset * sin( t ) + y;
        }
        ringX[ mSegments ] = ringX.at( 0 );
        ringY[ mSegments ] = ringY.at( 0 );
        break;
    }

    if ( phi != 0 )
    {
      for ( int i = 0; i < ringX.size(); ++i )
      {
        const double px = ringX.at( i );
        const double py = ringY.at( i );
        ringX[ i ] = ( px - x ) * cos( phi ) + ( py - y ) * sin( phi ) + x;
        ringY[ i ] = -( px - x ) * sin( phi ) + ( py - y ) * cos( phi ) + y;
      }
    }

    std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();
    poly->setExteriorRing( new QgsLineString( ringX, ringY ) );

    if ( geometry.constGet()->is3D() )
    {
      poly->addZValue( static_cast< const QgsPoint * >( geometry.constGet() )->z() );
    }
    if ( geometry.constGet()->isMeasure() )
    {
      poly->addMValue( static_cast< const QgsPoint * >( geometry.constGet() )->m() );
    }

    outFeature.setGeometry( std::move( poly ) );
  }

  return QgsFeatureList() << outFeature;
}

///@endcond
