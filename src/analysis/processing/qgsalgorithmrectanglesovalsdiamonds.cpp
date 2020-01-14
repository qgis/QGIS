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
  return QObject::tr( "Creates a rectangle, oval or diamond-shaped polygons from the input point layer using "
                      "specified width, height and (optional) rotation values." );
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

  auto widthParam = qgis::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "WIDTH" ), QObject::tr( "Width" ), 1.0, QStringLiteral( "INPUT" ), false, 0.0 );
  widthParam->setIsDynamic( true );
  widthParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Width" ), QObject::tr( "Width" ), QgsPropertyDefinition::Double ) );
  widthParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( widthParam.release() );

  auto heightParam = qgis::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "HEIGHT" ), QObject::tr( "Height" ), 1.0, QStringLiteral( "INPUT" ), false, 0.0 );
  heightParam->setIsDynamic( true );
  heightParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Height" ), QObject::tr( "Height" ), QgsPropertyDefinition::Double ) );
  heightParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( heightParam.release() );

  auto rotationParam = qgis::make_unique < QgsProcessingParameterNumber >( QStringLiteral( "ROTATION" ), QObject::tr( "Rotation" ), QgsProcessingParameterNumber::Double, 0.0, true, 0.0, 360.0 );
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

    double phi = rotation * M_PI / 180;
    double xOffset = width / 2.0;
    double yOffset = height / 2.0;
    QgsPointXY point = outFeature.geometry().asPoint();
    double x = point.x();
    double y = point.y();

    QgsPolylineXY ring;
    switch ( mShape )
    {
      case 0:
        // rectangle
        ring << QgsPointXY( -xOffset, -yOffset )
             << QgsPointXY( -xOffset, yOffset )
             << QgsPointXY( xOffset, yOffset )
             << QgsPointXY( xOffset, -yOffset );
        break;
      case 1:
        // diamond
        ring << QgsPointXY( 0.0, -yOffset )
             << QgsPointXY( -xOffset, 0.0 )
             << QgsPointXY( 0.0, yOffset )
             << QgsPointXY( xOffset, 0.0 );
        break;
      case 2:
        // oval
        for ( int i = 0; i < mSegments; i ++ )
        {
          double t = ( 2 * M_PI ) / mSegments * i;
          ring << QgsPointXY( xOffset * cos( t ), yOffset * sin( t ) );
        }
        break;
    }

    for ( int i = 0; i < ring.size(); ++i )
    {
      QgsPointXY p = ring[ i ];
      double px = p.x();
      double py = p.y();
      ring[ i ] = QgsPointXY( px * cos( phi ) + py * sin( phi ) + x,
                              -px * sin( phi ) + py * cos( phi ) + y );
    }

    outFeature.setGeometry( QgsGeometry::fromPolygonXY( QgsPolygonXY() << ring ) );
  }

  return QgsFeatureList() << outFeature;
}

///@endcond
