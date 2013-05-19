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
#include <QDomDocument>
#include <QDomElement>

QgsRasterShader::QgsRasterShader( double theMinimumValue, double theMaximumValue )
{
  QgsDebugMsg( "called." );

  mMinimumValue = theMinimumValue;
  mMaximumValue = theMaximumValue;
  mRasterShaderFunction = new QgsRasterShaderFunction( mMinimumValue, mMaximumValue );
}

QgsRasterShader::~QgsRasterShader()
{
  delete mRasterShaderFunction;
}

/**
  Generates and new RGBA value based on one input value

  @param theValue The original value to base a new RGBA value on
  @param theReturnRedValue  The red component of the new RGBA value
  @param theReturnGreenValue  The green component of the new RGBA value
  @param theReturnBlueValue  The blue component of the new RGBA value
  @param theReturnAlpha  The alpha component of the new RGBA value
  @return True if the return values are valid otherwise false
*/
bool QgsRasterShader::shade( double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue , int *theReturnAlpha )
{
  if ( 0 != mRasterShaderFunction )
  {
    return mRasterShaderFunction->shade( theValue, theReturnRedValue, theReturnGreenValue, theReturnBlueValue, theReturnAlpha );
  }

  return false;
}
/**
  Generates and new RGBA value based on an original RGBA value


  @param theRedValue The red component of the original value to base a new RGBA value on
  @param theGreenValue The green component of the original value to base a new RGBA value on
  @param theBlueValue The blue component of the original value to base a new RGBA value on
  @param theAlphaValue  The alpha component of the original value to base a new RGBA value on
  @param theReturnRedValue  The red component of the new RGBA value
  @param theReturnGreenValue  The green component of the new RGBA value
  @param theReturnBlueValue  The blue component of the new RGBA value
  @param theReturnAlphaValue  The alpha component of the new RGBA value
  @return True if the return values are valid otherwise false
*/
bool QgsRasterShader::shade( double theRedValue, double theGreenValue, double theBlueValue, double theAlphaValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue, int* theReturnAlphaValue )
{
  if ( 0 != mRasterShaderFunction )
  {
    return mRasterShaderFunction->shade( theRedValue, theGreenValue, theBlueValue, theAlphaValue, theReturnRedValue, theReturnGreenValue, theReturnBlueValue, theReturnAlphaValue );
  }

  return false;
}

/**
    A public function that allows the user to set their own custom shader function.

    @param theFunction A pointer to the new raster shader function
*/
void QgsRasterShader::setRasterShaderFunction( QgsRasterShaderFunction* theFunction )
{
  QgsDebugMsg( "called." );

  if ( mRasterShaderFunction == theFunction )
    return;

  if ( 0 != theFunction )
  {
    delete mRasterShaderFunction;
    mRasterShaderFunction = theFunction;
  }
}

/**
    Set the maximum value for the raster shader.

    @param theValue The new maximum value
*/
void QgsRasterShader::setMaximumValue( double theValue )
{
  QgsDebugMsg( "Value = " + QString::number( theValue ) );

  mMaximumValue = theValue;
  if ( 0 != mRasterShaderFunction )
  {
    mRasterShaderFunction->setMaximumValue( theValue );
  }
}

/**
    Set the maximum value for the raster shader

    @param theValue The new minimum value
*/
void QgsRasterShader::setMinimumValue( double theValue )
{
  QgsDebugMsg( "Value = " + QString::number( theValue ) );

  mMinimumValue = theValue;
  if ( 0 != mRasterShaderFunction )
  {
    mRasterShaderFunction->setMinimumValue( theValue );
  }
}

void QgsRasterShader::writeXML( QDomDocument& doc, QDomElement& parent ) const
{
  if ( parent.isNull() || !mRasterShaderFunction )
  {
    return;
  }

  QDomElement rasterShaderElem = doc.createElement( "rastershader" );
  QgsColorRampShader* colorRampShader = dynamic_cast<QgsColorRampShader*>( mRasterShaderFunction );
  if ( colorRampShader )
  {
    QDomElement colorRampShaderElem = doc.createElement( "colorrampshader" );
    colorRampShaderElem.setAttribute( "colorRampType", colorRampShader->colorRampTypeAsQString() );
    colorRampShaderElem.setAttribute( "clip", colorRampShader->clip() );
    //items
    QList<QgsColorRampShader::ColorRampItem> itemList = colorRampShader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem>::const_iterator itemIt = itemList.constBegin();
    for ( ; itemIt != itemList.constEnd(); ++itemIt )
    {
      QDomElement itemElem = doc.createElement( "item" );
      itemElem.setAttribute( "label", itemIt->label );
      itemElem.setAttribute( "value", QString::number( itemIt->value ) );
      itemElem.setAttribute( "color", itemIt->color.name() );
      itemElem.setAttribute( "alpha", itemIt->color.alpha() );
      colorRampShaderElem.appendChild( itemElem );
    }
    rasterShaderElem.appendChild( colorRampShaderElem );
  }
  parent.appendChild( rasterShaderElem );
}

void QgsRasterShader::readXML( const QDomElement& elem )
{
  //only colorrampshader
  QDomElement colorRampShaderElem = elem.firstChildElement( "colorrampshader" );
  if ( !colorRampShaderElem.isNull() )
  {
    QgsColorRampShader* colorRampShader = new QgsColorRampShader();
    colorRampShader->setColorRampType( colorRampShaderElem.attribute( "colorRampType", "INTERPOLATED" ) );
    colorRampShader->setClip( colorRampShaderElem.attribute( "clip", "0" ) == "1" );

    QList<QgsColorRampShader::ColorRampItem> itemList;
    QDomElement itemElem;
    QString itemLabel;
    double itemValue;
    QColor itemColor;

    QDomNodeList itemNodeList = colorRampShaderElem.elementsByTagName( "item" );
    for ( int i = 0; i < itemNodeList.size(); ++i )
    {
      itemElem = itemNodeList.at( i ).toElement();
      itemValue = itemElem.attribute( "value" ).toDouble();
      itemLabel = itemElem.attribute( "label" );
      itemColor.setNamedColor( itemElem.attribute( "color" ) );
      itemColor.setAlpha( itemElem.attribute( "alpha", "255" ).toInt() );

      itemList.push_back( QgsColorRampShader::ColorRampItem( itemValue, itemColor, itemLabel ) );
    }
    colorRampShader->setColorRampItemList( itemList );
    setRasterShaderFunction( colorRampShader );
  }
}
