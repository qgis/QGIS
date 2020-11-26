/***************************************************************************
  qgspointcloud3dsymbol.h
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloud3dsymbol.h"

#include "qgscolorramptexture.h"

#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>

// QgsPointCloud3DSymbol

QgsPointCloud3DSymbol::QgsPointCloud3DSymbol( QgsPointCloudLayer *layer, QgsPointCloud3DSymbol::RenderingStyle style )
  : QgsAbstract3DSymbol()
  , mRenderingStyle( style )
  , mLayer( layer )
{
}

QgsPointCloud3DSymbol::~QgsPointCloud3DSymbol() {  }

void QgsPointCloud3DSymbol::setLayer( QgsPointCloudLayer *layer )
{
  mLayer = layer;
}

void QgsPointCloud3DSymbol::setPointSize( float size )
{
  mPointSize = size;
}

// QgsSingleColorPointCloud3DSymbol

QgsSingleColorPointCloud3DSymbol::QgsSingleColorPointCloud3DSymbol( QgsPointCloudLayer *layer )
  : QgsPointCloud3DSymbol( layer, QgsPointCloud3DSymbol::RenderingStyle::SingleColor )
{

}

QgsAbstract3DSymbol *QgsSingleColorPointCloud3DSymbol::clone() const
{
  QgsSingleColorPointCloud3DSymbol *result = new QgsSingleColorPointCloud3DSymbol( mLayer );
  result->mPointSize = mPointSize;
  result->mSingleColor = mSingleColor;
  copyBaseSettings( result );
  return result;
}

void QgsSingleColorPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
  elem.setAttribute( QStringLiteral( "rendering-style" ), mRenderingStyle );
  elem.setAttribute( QStringLiteral( "single-color-red" ), mSingleColor.redF() );
  elem.setAttribute( QStringLiteral( "single-color-green" ), mSingleColor.greenF() );
  elem.setAttribute( QStringLiteral( "single-color-blue" ), mSingleColor.blueF() );
}

void QgsSingleColorPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mPointSize = elem.attribute( "point-size", QStringLiteral( "2.0" ) ).toFloat();
  mRenderingStyle = static_cast< QgsPointCloud3DSymbol::RenderingStyle >( elem.attribute( "rendering-style", QStringLiteral( "1" ) ).toInt() );
  mSingleColor.setRedF( elem.attribute( "single-color-red", QStringLiteral( "0.0" ) ).toFloat() );
  mSingleColor.setGreenF( elem.attribute( "single-color-green", QStringLiteral( "0.0" ) ).toFloat() );
  mSingleColor.setBlueF( elem.attribute( "single-color-blue", QStringLiteral( "1.0" ) ).toFloat() );
}

void QgsSingleColorPointCloud3DSymbol::setSingleColor( QColor color )
{
  mSingleColor = color;
}

void QgsSingleColorPointCloud3DSymbol::fillMaterial( Qt3DRender::QMaterial *mat )
{
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", mRenderingStyle );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( mPointSize ) );
  mat->addParameter( pointSizeParameter );
  Qt3DRender::QParameter *singleColorParameter = new Qt3DRender::QParameter( "u_singleColor", QVector3D( mSingleColor.redF(), mSingleColor.greenF(), mSingleColor.blueF() ) );
  mat->addParameter( singleColorParameter );
}

// QgsColorRampPointCloud3DSymbol

QgsColorRampPointCloud3DSymbol::QgsColorRampPointCloud3DSymbol( QgsPointCloudLayer *layer )
  : QgsPointCloud3DSymbol( layer, QgsPointCloud3DSymbol::RenderingStyle::ColorRamp )
{

}

QgsAbstract3DSymbol *QgsColorRampPointCloud3DSymbol::clone() const
{
  QgsColorRampPointCloud3DSymbol *result = new QgsColorRampPointCloud3DSymbol( mLayer );
  result->mPointSize = mPointSize;
  result->mRenderingParameter = mRenderingParameter;
  result->mColorRampShader = mColorRampShader;
  result->mColorRampShaderMin = mColorRampShaderMin;
  result->mColorRampShaderMax = mColorRampShaderMax;
  copyBaseSettings( result );
  return result;
}

void QgsColorRampPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
  elem.setAttribute( QStringLiteral( "rendering-style" ), mRenderingStyle );
  elem.setAttribute( QStringLiteral( "rendering-parameter" ), mRenderingParameter );
  elem.setAttribute( QStringLiteral( "color-ramp-shader-min" ), mColorRampShaderMin );
  elem.setAttribute( QStringLiteral( "color-ramp-shader-max" ), mColorRampShaderMax );
  QDomDocument doc = elem.ownerDocument();
  QDomElement elemColorRampShader = mColorRampShader.writeXml( doc );
  elem.appendChild( elemColorRampShader );
}

void QgsColorRampPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mPointSize = elem.attribute( "point-size", QStringLiteral( "2.0" ) ).toFloat();
  mRenderingStyle = static_cast< QgsPointCloud3DSymbol::RenderingStyle >( elem.attribute( "rendering-style", QStringLiteral( "2" ) ).toInt() );
  mRenderingParameter = elem.attribute( "rendering-parameter", QString() );
  mColorRampShaderMin = elem.attribute( QStringLiteral( "color-ramp-shader-min" ), QStringLiteral( "0.0" ) ).toDouble();
  mColorRampShaderMax = elem.attribute( QStringLiteral( "color-ramp-shader-max" ), QStringLiteral( "1.0" ) ).toDouble();
  mColorRampShader.readXml( elem );
}

QString QgsColorRampPointCloud3DSymbol::renderingParameter() const
{
  return mRenderingParameter;
}

void QgsColorRampPointCloud3DSymbol::setRenderingParameter( const QString &parameter )
{
  mRenderingParameter = parameter;
}

QgsColorRampShader QgsColorRampPointCloud3DSymbol::colorRampShader() const
{
  return mColorRampShader;
}

void QgsColorRampPointCloud3DSymbol::setColorRampShader( const QgsColorRampShader &colorRampShader )
{
  mColorRampShader = colorRampShader;
}

void QgsColorRampPointCloud3DSymbol::setColorRampShaderMinMax( double min, double max )
{
  mColorRampShaderMin = min;
  mColorRampShaderMax = max;
}

void QgsColorRampPointCloud3DSymbol::fillMaterial( Qt3DRender::QMaterial *mat )
{
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", mRenderingStyle );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( mPointSize ) );
  mat->addParameter( pointSizeParameter );
  // Create the texture to pass the color ramp
  Qt3DRender::QTexture1D *colorRampTexture = nullptr;
  if ( mColorRampShader.colorRampItemList().count() > 0 )
  {
    colorRampTexture = new Qt3DRender::QTexture1D( mat );
    colorRampTexture->addTextureImage( new QgsColorRampTexture( mColorRampShader, 1 ) );
    colorRampTexture->setMinificationFilter( Qt3DRender::QTexture1D::Linear );
    colorRampTexture->setMagnificationFilter( Qt3DRender::QTexture1D::Linear );
  }

  // Parameters
  Qt3DRender::QParameter *colorRampTextureParameter = new Qt3DRender::QParameter( "u_colorRampTexture", colorRampTexture );
  mat->addParameter( colorRampTextureParameter );
  Qt3DRender::QParameter *colorRampCountParameter = new Qt3DRender::QParameter( "u_colorRampCount", mColorRampShader.colorRampItemList().count() );
  mat->addParameter( colorRampCountParameter );
  int colorRampType = mColorRampShader.colorRampType();
  Qt3DRender::QParameter *colorRampTypeParameter = new Qt3DRender::QParameter( "u_colorRampType", colorRampType );
  mat->addParameter( colorRampTypeParameter );
}

// QgsRGBPointCloud3DSymbol

QgsRGBPointCloud3DSymbol::QgsRGBPointCloud3DSymbol( QgsPointCloudLayer *layer )
  : QgsPointCloud3DSymbol( layer, QgsPointCloud3DSymbol::RenderingStyle::RGBRendering )
{

}

QgsAbstract3DSymbol *QgsRGBPointCloud3DSymbol::clone() const
{
  QgsRGBPointCloud3DSymbol *result = new QgsRGBPointCloud3DSymbol( mLayer );
  result->mPointSize = mPointSize;
  copyBaseSettings( result );
  return result;
}

void QgsRGBPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  elem.setAttribute( QStringLiteral( "rendering-style" ), mRenderingStyle );
  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
}

void QgsRGBPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mRenderingStyle = static_cast< QgsPointCloud3DSymbol::RenderingStyle >( elem.attribute( "rendering-style", QStringLiteral( "3" ) ).toInt() );
  mPointSize = elem.attribute( "point-size", QStringLiteral( "2.0" ) ).toFloat();
}

void QgsRGBPointCloud3DSymbol::fillMaterial( Qt3DRender::QMaterial *mat )
{
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", mRenderingStyle );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( mPointSize ) );
  mat->addParameter( pointSizeParameter );
}
