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
  QgsDebugMsgLevel( QStringLiteral( "called." ), 4 );
}

bool QgsRasterShader::shade( double value, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlpha )
{
  if ( mRasterShaderFunction )
  {
    return mRasterShaderFunction->shade( value, returnRedValue, returnGreenValue, returnBlueValue, returnAlpha );
  }

  return false;
}

bool QgsRasterShader::shade( double redValue, double greenValue, double blueValue, double alphaValue, int *returnRedValue, int *returnGreenValue, int *returnBlueValue, int *returnAlphaValue )
{
  if ( mRasterShaderFunction )
  {
    return mRasterShaderFunction->shade( redValue, greenValue, blueValue, alphaValue, returnRedValue, returnGreenValue, returnBlueValue, returnAlphaValue );
  }

  return false;
}

void QgsRasterShader::setRasterShaderFunction( QgsRasterShaderFunction *function )
{
  QgsDebugMsgLevel( QStringLiteral( "called." ), 4 );

  if ( mRasterShaderFunction.get() == function )
    return;

  if ( function )
  {
    mRasterShaderFunction.reset( function );
  }
}

void QgsRasterShader::setMaximumValue( double value )
{
  QgsDebugMsgLevel( "Value = " + QString::number( value ), 4 );

  mMaximumValue = value;
  if ( mRasterShaderFunction )
  {
    mRasterShaderFunction->setMaximumValue( value );
  }
}

void QgsRasterShader::setMinimumValue( double value )
{
  QgsDebugMsgLevel( "Value = " + QString::number( value ), 4 );

  mMinimumValue = value;
  if ( mRasterShaderFunction )
  {
    mRasterShaderFunction->setMinimumValue( value );
  }
}

void QgsRasterShader::writeXml( QDomDocument &doc, QDomElement &parent, const QgsReadWriteContext &context ) const
{
  if ( parent.isNull() || !mRasterShaderFunction )
  {
    return;
  }

  QDomElement rasterShaderElem = doc.createElement( QStringLiteral( "rastershader" ) );
  QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( mRasterShaderFunction.get() );
  if ( colorRampShader )
  {
    rasterShaderElem.appendChild( colorRampShader->writeXml( doc, context ) );
  }
  parent.appendChild( rasterShaderElem );
}

void QgsRasterShader::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  //only colorrampshader
  const QDomElement colorRampShaderElem = elem.firstChildElement( QStringLiteral( "colorrampshader" ) );
  if ( !colorRampShaderElem.isNull() )
  {
    QgsColorRampShader *colorRampShader = new QgsColorRampShader();
    colorRampShader->readXml( colorRampShaderElem, context );
    setRasterShaderFunction( colorRampShader );
  }
}

