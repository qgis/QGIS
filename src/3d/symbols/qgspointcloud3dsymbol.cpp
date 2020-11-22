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


QgsPointCloud3DSymbol::QgsPointCloud3DSymbol()
  : QgsAbstract3DSymbol()
{

}

QgsPointCloud3DSymbol::~QgsPointCloud3DSymbol() {  }

QgsAbstract3DSymbol *QgsPointCloud3DSymbol::clone() const
{
  QgsPointCloud3DSymbol *result = new QgsPointCloud3DSymbol;
  result->mEnabled = mEnabled;
  result->mPointSize = mPointSize;
  result->mRenderingStyle = mRenderingStyle;
  result->mRenderingParameter = mRenderingParameter;
  result->mSingleColor = mSingleColor;
  result->mColorRampShader = mColorRampShader;
  result->mColorRampShaderMin = mColorRampShaderMin;
  result->mColorRampShaderMax = mColorRampShaderMax;
  copyBaseSettings( result );
  return result;
}

void QgsPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  elem.setAttribute( QStringLiteral( "enabled" ), mEnabled );
  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
  elem.setAttribute( QStringLiteral( "rendering-style" ), mRenderingStyle );
  elem.setAttribute( QStringLiteral( "single-color-red" ), mSingleColor.redF() );
  elem.setAttribute( QStringLiteral( "single-color-green" ), mSingleColor.greenF() );
  elem.setAttribute( QStringLiteral( "single-color-blue" ), mSingleColor.blueF() );
  elem.setAttribute( QStringLiteral( "rendering-parameter" ), mRenderingParameter );
  elem.setAttribute( QStringLiteral( "color-ramp-shader-min" ), mColorRampShaderMin );
  elem.setAttribute( QStringLiteral( "color-ramp-shader-max" ), mColorRampShaderMax );
  QDomDocument doc = elem.ownerDocument();
  QDomElement elemColorRampShader = mColorRampShader.writeXml( doc );
  elem.appendChild( elemColorRampShader );
}

void QgsPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mEnabled = elem.attribute( "enabled", QStringLiteral( "0" ) ).toInt();
  mPointSize = elem.attribute( "point-size", QStringLiteral( "2.0" ) ).toFloat();
  mRenderingStyle = static_cast< QgsPointCloud3DSymbol::RenderingStyle >( elem.attribute( "rendering-style", QStringLiteral( "0" ) ).toInt() );
  mSingleColor.setRedF( elem.attribute( "single-color-red", QStringLiteral( "0.0" ) ).toFloat() );
  mSingleColor.setGreenF( elem.attribute( "single-color-green", QStringLiteral( "0.0" ) ).toFloat() );
  mSingleColor.setBlueF( elem.attribute( "single-color-blue", QStringLiteral( "1.0" ) ).toFloat() );
  mRenderingParameter = static_cast< QgsPointCloud3DSymbol::RenderingParameter >( elem.attribute( "rendering-parameter", QStringLiteral( "0" ) ).toInt() );
  mColorRampShaderMin = elem.attribute( QStringLiteral( "color-ramp-shader-min" ), QStringLiteral( "0.0" ) ).toDouble();
  mColorRampShaderMax = elem.attribute( QStringLiteral( "color-ramp-shader-max" ), QStringLiteral( "1.0" ) ).toDouble();
  mColorRampShader.readXml( elem );
}

void QgsPointCloud3DSymbol::setIsEnabled( bool enabled )
{
  mEnabled = enabled;
}

void QgsPointCloud3DSymbol::setRenderingStyle( QgsPointCloud3DSymbol::RenderingStyle style )
{
  mRenderingStyle = style;
}

QgsPointCloud3DSymbol::RenderingStyle QgsPointCloud3DSymbol::renderingStyle() const
{
  return mRenderingStyle;
}

QgsPointCloud3DSymbol::RenderingParameter QgsPointCloud3DSymbol::renderingParameter() const
{
  return mRenderingParameter;
}

void QgsPointCloud3DSymbol::setRenderingParameter( QgsPointCloud3DSymbol::RenderingParameter parameter )
{
  mRenderingParameter = parameter;
}

void QgsPointCloud3DSymbol::setPointSize( float size )
{
  mPointSize = size;
}

void QgsPointCloud3DSymbol::setSingleColor( QColor color )
{
  mSingleColor = color;
}

QgsColorRampShader QgsPointCloud3DSymbol::colorRampShader() const
{
  return mColorRampShader;
}

void QgsPointCloud3DSymbol::setColorRampShader( const QgsColorRampShader &colorRampShader )
{
  mColorRampShader = colorRampShader;
}

void QgsPointCloud3DSymbol::setColorRampShaderMinMax( double min, double max )
{
  mColorRampShaderMin = min;
  mColorRampShaderMax = max;
}
