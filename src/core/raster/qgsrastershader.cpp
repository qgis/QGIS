/* **************************************************************************
                qgsrastershader.cpp -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgscolorrampshader.h"
#include "qgsrastershader.h"
#include "qgsrasterblock.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>
#include <QDomElement>

QgsRasterShader::QgsRasterShader( double minimumValue, double maximumValue )
  : mMinimumValue( minimumValue )
  , mMaximumValue( maximumValue )
  , mRasterShaderFunction( new QgsRasterShaderFunction( mMinimumValue, mMaximumValue ) )
{
  QgsDebugMsgLevel( "called.", 4 );
}

/**
  Generates and new RGBA value based on one input value

  @param value The original value to base a new RGBA value on
  @param returnRedValue  The red component of the new RGBA value
  @param returnGreenValue  The green component of the new RGBA value
  @param returnBlueValue  The blue component of the new RGBA value
  @param returnAlpha  The alpha component of the new RGBA value
  @return True if the return values are valid otherwise false
*/
bool QgsRasterShader::shade( double value, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlpha )
{
  if ( mRasterShaderFunction )
  {
    return mRasterShaderFunction->shade( value, returnRedValue, returnGreenValue, returnBlueValue, returnAlpha );
  }

  return false;
}

/**
  Generates and new RGBA value based on an original RGBA value


  @param redValue The red component of the original value to base a new RGBA value on
  @param greenValue The green component of the original value to base a new RGBA value on
  @param blueValue The blue component of the original value to base a new RGBA value on
  @param alphaValue  The alpha component of the original value to base a new RGBA value on
  @param returnRedValue  The red component of the new RGBA value
  @param returnGreenValue  The green component of the new RGBA value
  @param returnBlueValue  The blue component of the new RGBA value
  @param returnAlphaValue  The alpha component of the new RGBA value
  @return True if the return values are valid otherwise false
*/
bool QgsRasterShader::shade( double redValue, double greenValue, double blueValue, double alphaValue, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlphaValue )
{
  if ( mRasterShaderFunction )
  {
    return mRasterShaderFunction->shade( redValue, greenValue, blueValue, alphaValue, returnRedValue, returnGreenValue, returnBlueValue, returnAlphaValue );
  }

  return false;
}

/**
    A public function that allows the user to set their own custom shader function.

    @param function A pointer to the new raster shader function
*/
void QgsRasterShader::setRasterShaderFunction( QgsRasterShaderFunction *function )
{
  QgsDebugMsgLevel( "called.", 4 );

  if ( mRasterShaderFunction.get() == function )
    return;

  if ( function )
  {
    mRasterShaderFunction.reset( function );
  }
}

/**
    Set the maximum value for the raster shader.

    @param value The new maximum value
*/
void QgsRasterShader::setMaximumValue( double value )
{
  QgsDebugMsgLevel( "Value = " + QString::number( value ), 4 );

  mMaximumValue = value;
  if ( mRasterShaderFunction )
  {
    mRasterShaderFunction->setMaximumValue( value );
  }
}

/**
    Set the maximum value for the raster shader

    @param value The new minimum value
*/
void QgsRasterShader::setMinimumValue( double value )
{
  QgsDebugMsgLevel( "Value = " + QString::number( value ), 4 );

  mMinimumValue = value;
  if ( mRasterShaderFunction )
  {
    mRasterShaderFunction->setMinimumValue( value );
  }
}

void QgsRasterShader::writeXml( QDomDocument &doc, QDomElement &parent ) const
{
  if ( parent.isNull() || !mRasterShaderFunction )
  {
    return;
  }

  QDomElement rasterShaderElem = doc.createElement( QStringLiteral( "rastershader" ) );
  QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( mRasterShaderFunction.get() );
  if ( colorRampShader )
  {
    QDomElement colorRampShaderElem = doc.createElement( QStringLiteral( "colorrampshader" ) );
    colorRampShaderElem.setAttribute( QStringLiteral( "colorRampType" ), colorRampShader->colorRampTypeAsQString() );
    colorRampShaderElem.setAttribute( QStringLiteral( "classificationMode" ), colorRampShader->classificationMode() );
    colorRampShaderElem.setAttribute( QStringLiteral( "clip" ), colorRampShader->clip() );

    // save source color ramp
    if ( colorRampShader->sourceColorRamp() )
    {
      QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QStringLiteral( "[source]" ), colorRampShader->sourceColorRamp(), doc );
      colorRampShaderElem.appendChild( colorRampElem );
    }

    //items
    QList<QgsColorRampShader::ColorRampItem> itemList = colorRampShader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem>::const_iterator itemIt = itemList.constBegin();
    for ( ; itemIt != itemList.constEnd(); ++itemIt )
    {
      QDomElement itemElem = doc.createElement( QStringLiteral( "item" ) );
      itemElem.setAttribute( QStringLiteral( "label" ), itemIt->label );
      itemElem.setAttribute( QStringLiteral( "value" ), QgsRasterBlock::printValue( itemIt->value ) );
      itemElem.setAttribute( QStringLiteral( "color" ), itemIt->color.name() );
      itemElem.setAttribute( QStringLiteral( "alpha" ), itemIt->color.alpha() );
      colorRampShaderElem.appendChild( itemElem );
    }
    rasterShaderElem.appendChild( colorRampShaderElem );
  }
  parent.appendChild( rasterShaderElem );
}

void QgsRasterShader::readXml( const QDomElement &elem )
{
  //only colorrampshader
  QDomElement colorRampShaderElem = elem.firstChildElement( QStringLiteral( "colorrampshader" ) );
  if ( !colorRampShaderElem.isNull() )
  {
    QgsColorRampShader *colorRampShader = new QgsColorRampShader();

    // try to load color ramp (optional)
    QDomElement sourceColorRampElem = colorRampShaderElem.firstChildElement( QStringLiteral( "colorramp" ) );
    if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "[source]" ) )
    {
      colorRampShader->setSourceColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
    }

    colorRampShader->setColorRampType( colorRampShaderElem.attribute( QStringLiteral( "colorRampType" ), QStringLiteral( "INTERPOLATED" ) ) );
    colorRampShader->setClassificationMode( static_cast< QgsColorRampShader::ClassificationMode >( colorRampShaderElem.attribute( QStringLiteral( "classificationMode" ), QStringLiteral( "1" ) ).toInt() ) );
    colorRampShader->setClip( colorRampShaderElem.attribute( QStringLiteral( "clip" ), QStringLiteral( "0" ) ) == QLatin1String( "1" ) );

    QList<QgsColorRampShader::ColorRampItem> itemList;
    QDomElement itemElem;
    QString itemLabel;
    double itemValue;
    QColor itemColor;

    QDomNodeList itemNodeList = colorRampShaderElem.elementsByTagName( QStringLiteral( "item" ) );
    itemList.reserve( itemNodeList.size() );
    for ( int i = 0; i < itemNodeList.size(); ++i )
    {
      itemElem = itemNodeList.at( i ).toElement();
      itemValue = itemElem.attribute( QStringLiteral( "value" ) ).toDouble();
      itemLabel = itemElem.attribute( QStringLiteral( "label" ) );
      itemColor.setNamedColor( itemElem.attribute( QStringLiteral( "color" ) ) );
      itemColor.setAlpha( itemElem.attribute( QStringLiteral( "alpha" ), QStringLiteral( "255" ) ).toInt() );

      itemList.push_back( QgsColorRampShader::ColorRampItem( itemValue, itemColor, itemLabel ) );
    }
    colorRampShader->setColorRampItemList( itemList );
    setRasterShaderFunction( colorRampShader );
  }
}
