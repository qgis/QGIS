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
  return u"rectanglesovalsdiamonds"_s;
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
  return u"vectorgeometry"_s;
}

QString QgsRectanglesOvalsDiamondsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates rectangle, oval or diamond-shaped polygons from the input point layer using "
                      "specified width, height and (optional) rotation values. Multipart inputs should be promoted "
                      "to singleparts first." );
}

QString QgsRectanglesOvalsDiamondsAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates rectangle, oval or diamond-shaped polygons from an input point layer." );
}

QIcon QgsRectanglesOvalsDiamondsAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmRectanglesOvalsDiamonds.svg"_s );
}

QString QgsRectanglesOvalsDiamondsAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmRectanglesOvalsDiamonds.svg"_s );
}

QString QgsRectanglesOvalsDiamondsAlgorithm::outputName() const
{
  return QObject::tr( "Polygon" );
}

QList<int> QgsRectanglesOvalsDiamondsAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint );
}

Qgis::ProcessingSourceType QgsRectanglesOvalsDiamondsAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

Qgis::WkbType QgsRectanglesOvalsDiamondsAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Qgis::WkbType outputWkbType = Qgis::WkbType::Polygon;
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
  addParameter( new QgsProcessingParameterEnum( u"SHAPE"_s, QObject::tr( "Shape" ), QStringList() << QObject::tr( "Rectangle" ) << QObject::tr( "Diamond" ) << QObject::tr( "Oval" ), false, 0 ) );

  auto widthParam = std::make_unique<QgsProcessingParameterDistance>( u"WIDTH"_s, QObject::tr( "Width" ), 1.0, u"INPUT"_s, false, 0.0 );
  widthParam->setIsDynamic( true );
  widthParam->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Width"_s, QObject::tr( "Width" ), QgsPropertyDefinition::DoublePositive ) );
  widthParam->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( widthParam.release() );

  auto heightParam = std::make_unique<QgsProcessingParameterDistance>( u"HEIGHT"_s, QObject::tr( "Height" ), 1.0, u"INPUT"_s, false, 0.0 );
  heightParam->setIsDynamic( true );
  heightParam->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Height"_s, QObject::tr( "Height" ), QgsPropertyDefinition::DoublePositive ) );
  heightParam->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( heightParam.release() );

  auto rotationParam = std::make_unique<QgsProcessingParameterNumber>( u"ROTATION"_s, QObject::tr( "Rotation" ), Qgis::ProcessingNumberParameterType::Double, 0.0, true, -360.0, 360.0 );
  rotationParam->setIsDynamic( true );
  rotationParam->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Rotation"_s, QObject::tr( "Rotation" ), QgsPropertyDefinition::Double ) );
  rotationParam->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( rotationParam.release() );

  addParameter( new QgsProcessingParameterNumber( u"SEGMENTS"_s, QObject::tr( "Segments" ), Qgis::ProcessingNumberParameterType::Integer, 36, false, 1 ) );
}

bool QgsRectanglesOvalsDiamondsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mShape = parameterAsEnum( parameters, u"SHAPE"_s, context );

  mWidth = parameterAsDouble( parameters, u"WIDTH"_s, context );
  mDynamicWidth = QgsProcessingParameters::isDynamic( parameters, u"WIDTH"_s );
  if ( mDynamicWidth )
    mWidthProperty = parameters.value( u"WIDTH"_s ).value<QgsProperty>();

  mHeight = parameterAsDouble( parameters, u"HEIGHT"_s, context );
  mDynamicHeight = QgsProcessingParameters::isDynamic( parameters, u"HEIGHT"_s );
  if ( mDynamicHeight )
    mHeightProperty = parameters.value( u"HEIGHT"_s ).value<QgsProperty>();

  mRotation = parameterAsDouble( parameters, u"ROTATION"_s, context );
  mDynamicRotation = QgsProcessingParameters::isDynamic( parameters, u"ROTATION"_s );
  if ( mDynamicRotation )
    mRotationProperty = parameters.value( u"ROTATION"_s ).value<QgsProperty>();

  mSegments = parameterAsDouble( parameters, u"SEGMENTS"_s, context );

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

    QVector<double> ringX( 5 );
    QVector<double> ringY( 5 );

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
        for ( int i = 0; i < mSegments; i++ )
        {
          const double t = ( 2 * M_PI ) / mSegments * i;
          ringX[i] = xOffset * cos( t ) + x;
          ringY[i] = yOffset * sin( t ) + y;
        }
        ringX[mSegments] = ringX.at( 0 );
        ringY[mSegments] = ringY.at( 0 );
        break;
    }

    if ( phi != 0 )
    {
      for ( int i = 0; i < ringX.size(); ++i )
      {
        const double px = ringX.at( i );
        const double py = ringY.at( i );
        ringX[i] = ( px - x ) * cos( phi ) + ( py - y ) * sin( phi ) + x;
        ringY[i] = -( px - x ) * sin( phi ) + ( py - y ) * cos( phi ) + y;
      }
    }

    auto poly = std::make_unique<QgsPolygon>();
    poly->setExteriorRing( new QgsLineString( ringX, ringY ) );

    if ( geometry.constGet()->is3D() )
    {
      poly->addZValue( static_cast<const QgsPoint *>( geometry.constGet() )->z() );
    }
    if ( geometry.constGet()->isMeasure() )
    {
      poly->addMValue( static_cast<const QgsPoint *>( geometry.constGet() )->m() );
    }

    outFeature.setGeometry( std::move( poly ) );
  }

  return QgsFeatureList() << outFeature;
}

///@endcond
