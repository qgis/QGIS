/* **************************************************************************
                qgsrastertransparency.cpp -  description
                       -------------------
begin                : Mon Nov 30 2007
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

#include "qgsrasterinterface.h"
#include "qgsrastertransparency.h"
#include "qgis.h"

#include <QDomDocument>
#include <QDomElement>

QVector<QgsRasterTransparency::TransparentSingleValuePixel> QgsRasterTransparency::transparentSingleValuePixelList() const
{
  return mTransparentSingleValuePixelList;
}

QVector<QgsRasterTransparency::TransparentThreeValuePixel> QgsRasterTransparency::transparentThreeValuePixelList() const
{
  return mTransparentThreeValuePixelList;
}

void QgsRasterTransparency::initializeTransparentPixelList( double value )
{
  //clear the existing list
  mTransparentSingleValuePixelList.clear();

  //add the initial value
  mTransparentSingleValuePixelList.append( TransparentSingleValuePixel( value, value, 0 ) );
}

void QgsRasterTransparency::initializeTransparentPixelList( double redValue, double greenValue, double blueValue )
{
  //clear the existing list
  mTransparentThreeValuePixelList.clear();

  //add the initial values
  mTransparentThreeValuePixelList.append( TransparentThreeValuePixel( redValue, greenValue, blueValue, 0 ) );
}

void QgsRasterTransparency::setTransparentSingleValuePixelList( const QVector<QgsRasterTransparency::TransparentSingleValuePixel> &newList )
{
  mTransparentSingleValuePixelList = newList;
}

void QgsRasterTransparency::setTransparentThreeValuePixelList( const QVector<QgsRasterTransparency::TransparentThreeValuePixel> &newList )
{
  mTransparentThreeValuePixelList = newList;
}

int QgsRasterTransparency::alphaValue( double value, int globalTransparency ) const
{
  return static_cast< int >( opacityForValue( value ) * globalTransparency );
}

double QgsRasterTransparency::opacityForValue( double value ) const
{
  //if NaN return 0, transparent
  if ( std::isnan( value ) )
  {
    return 0;
  }

  //Search through the transparency list looking for a match
  auto it = std::find_if( mTransparentSingleValuePixelList.constBegin(), mTransparentSingleValuePixelList.constEnd(), [value]( const TransparentSingleValuePixel & p )
  {
    return ( value > p.min && value < p.max )
           || ( p.includeMinimum && qgsDoubleNear( value, p.min ) )
           || ( p.includeMaximum && qgsDoubleNear( value, p.max ) );
  } );

  if ( it != mTransparentSingleValuePixelList.constEnd() )
  {
    return it->opacity;
  }

  return 1;
}

int QgsRasterTransparency::alphaValue( double redValue, double greenValue, double blueValue, int globalTransparency ) const
{
  return static_cast< int >( opacityForRgbValues( redValue, greenValue, blueValue ) * globalTransparency );
}

double QgsRasterTransparency::opacityForRgbValues( double redValue, double greenValue, double blueValue ) const
{
  //if NaN return 0, transparent
  if ( std::isnan( redValue ) || std::isnan( greenValue ) || std::isnan( blueValue ) )
  {
    return 0;
  }

  //Search through the transparency list looking for a match
  auto it = std::find_if( mTransparentThreeValuePixelList.constBegin(), mTransparentThreeValuePixelList.constEnd(), [redValue, greenValue, blueValue]( const TransparentThreeValuePixel & p )
  {
    return qgsDoubleNear( p.red, redValue )
           &&  qgsDoubleNear( p.green, greenValue )
           && qgsDoubleNear( p.blue, blueValue );
  } );

  if ( it != mTransparentThreeValuePixelList.constEnd() )
  {
    return it->opacity;
  }

  return 1;
}

bool QgsRasterTransparency::isEmpty() const
{
  return mTransparentSingleValuePixelList.isEmpty() && mTransparentThreeValuePixelList.isEmpty();
}

void QgsRasterTransparency::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  QDomElement rasterTransparencyElem = doc.createElement( QStringLiteral( "rasterTransparency" ) );
  if ( !mTransparentSingleValuePixelList.isEmpty() )
  {
    QDomElement singleValuePixelListElement = doc.createElement( QStringLiteral( "singleValuePixelList" ) );
    auto it = mTransparentSingleValuePixelList.constBegin();
    for ( ; it != mTransparentSingleValuePixelList.constEnd(); ++it )
    {
      QDomElement pixelListElement = doc.createElement( QStringLiteral( "pixelListEntry" ) );
      pixelListElement.setAttribute( QStringLiteral( "min" ), QgsRasterBlock::printValue( it->min ) );
      pixelListElement.setAttribute( QStringLiteral( "max" ), QgsRasterBlock::printValue( it->max ) );
      pixelListElement.setAttribute( QStringLiteral( "percentTransparent" ), QString::number( 100.0 * ( 1 - it->opacity ) ) );
      singleValuePixelListElement.appendChild( pixelListElement );
    }
    rasterTransparencyElem.appendChild( singleValuePixelListElement );

  }
  if ( !mTransparentThreeValuePixelList.isEmpty() )
  {
    QDomElement threeValuePixelListElement = doc.createElement( QStringLiteral( "threeValuePixelList" ) );
    auto it = mTransparentThreeValuePixelList.constBegin();
    for ( ; it != mTransparentThreeValuePixelList.constEnd(); ++it )
    {
      QDomElement pixelListElement = doc.createElement( QStringLiteral( "pixelListEntry" ) );
      pixelListElement.setAttribute( QStringLiteral( "red" ), QgsRasterBlock::printValue( it->red ) );
      pixelListElement.setAttribute( QStringLiteral( "green" ), QgsRasterBlock::printValue( it->green ) );
      pixelListElement.setAttribute( QStringLiteral( "blue" ), QgsRasterBlock::printValue( it->blue ) );
      pixelListElement.setAttribute( QStringLiteral( "percentTransparent" ), QString::number( 100.0 * ( 1 - it->opacity ) ) );
      threeValuePixelListElement.appendChild( pixelListElement );
    }
    rasterTransparencyElem.appendChild( threeValuePixelListElement );
  }
  parentElem.appendChild( rasterTransparencyElem );
}

void QgsRasterTransparency::readXml( const QDomElement &elem )
{
  if ( elem.isNull() )
  {
    return;
  }

  mTransparentSingleValuePixelList.clear();
  mTransparentThreeValuePixelList.clear();
  QDomElement currentEntryElem;

  const QDomElement singlePixelListElem = elem.firstChildElement( QStringLiteral( "singleValuePixelList" ) );
  if ( !singlePixelListElem.isNull() )
  {
    const QDomNodeList entryList = singlePixelListElem.elementsByTagName( QStringLiteral( "pixelListEntry" ) );
    for ( int i = 0; i < entryList.size(); ++i )
    {
      currentEntryElem = entryList.at( i ).toElement();
      double min = 0;
      double max = 0;
      const double opacity = 1.0 - currentEntryElem.attribute( QStringLiteral( "percentTransparent" ) ).toDouble() / 100.0;
      // Backward compoatibility < 1.9 : pixelValue (before ranges)
      if ( currentEntryElem.hasAttribute( QStringLiteral( "pixelValue" ) ) )
      {
        min = max = currentEntryElem.attribute( QStringLiteral( "pixelValue" ) ).toDouble();
      }
      else
      {
        min = currentEntryElem.attribute( QStringLiteral( "min" ) ).toDouble();
        max = currentEntryElem.attribute( QStringLiteral( "max" ) ).toDouble();
      }
      mTransparentSingleValuePixelList.append( TransparentSingleValuePixel( min, max, opacity ) );
    }
  }
  const QDomElement threeValuePixelListElem = elem.firstChildElement( QStringLiteral( "threeValuePixelList" ) );
  if ( !threeValuePixelListElem.isNull() )
  {
    const QDomNodeList entryList = threeValuePixelListElem.elementsByTagName( QStringLiteral( "pixelListEntry" ) );
    for ( int i = 0; i < entryList.size(); ++i )
    {
      currentEntryElem = entryList.at( i ).toElement();
      const double red = currentEntryElem.attribute( QStringLiteral( "red" ) ).toDouble();
      const double green = currentEntryElem.attribute( QStringLiteral( "green" ) ).toDouble();
      const double blue = currentEntryElem.attribute( QStringLiteral( "blue" ) ).toDouble();
      const double opacity = 1.0 - currentEntryElem.attribute( QStringLiteral( "percentTransparent" ) ).toDouble() / 100.0;
      mTransparentThreeValuePixelList.append( TransparentThreeValuePixel( red, green, blue, opacity ) );
    }
  }
}
