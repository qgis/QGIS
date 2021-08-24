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
#include "qgslogger.h"

#include <QDomDocument>
#include <QDomElement>

QList<QgsRasterTransparency::TransparentSingleValuePixel> QgsRasterTransparency::transparentSingleValuePixelList() const
{
  return mTransparentSingleValuePixelList;
}

QList<QgsRasterTransparency::TransparentThreeValuePixel> QgsRasterTransparency::transparentThreeValuePixelList() const
{
  return mTransparentThreeValuePixelList;
}

void QgsRasterTransparency::initializeTransparentPixelList( double value )
{
  //clear the existing list
  mTransparentSingleValuePixelList.clear();

  //add the initial value
  TransparentSingleValuePixel myTransparentSingleValuePixel;
  myTransparentSingleValuePixel.min = value;
  myTransparentSingleValuePixel.max = value;
  myTransparentSingleValuePixel.percentTransparent = 100.0;
  mTransparentSingleValuePixelList.append( myTransparentSingleValuePixel );
}

void QgsRasterTransparency::initializeTransparentPixelList( double redValue, double greenValue, double blueValue )
{
  //clearn the existing list
  mTransparentThreeValuePixelList.clear();

  //add the initial values
  TransparentThreeValuePixel myTransparentThreeValuePixel;
  myTransparentThreeValuePixel.red = redValue;
  myTransparentThreeValuePixel.green = greenValue;
  myTransparentThreeValuePixel.blue = blueValue;
  myTransparentThreeValuePixel.percentTransparent = 100.0;
  mTransparentThreeValuePixelList.append( myTransparentThreeValuePixel );
}

void QgsRasterTransparency::setTransparentSingleValuePixelList( const QList<QgsRasterTransparency::TransparentSingleValuePixel> &newList )
{
  mTransparentSingleValuePixelList = newList;
}

void QgsRasterTransparency::setTransparentThreeValuePixelList( const QList<QgsRasterTransparency::TransparentThreeValuePixel> &newList )
{
  mTransparentThreeValuePixelList = newList;
}

int QgsRasterTransparency::alphaValue( double value, int globalTransparency ) const
{
  //if NaN return 0, transparent
  if ( std::isnan( value ) )
  {
    return 0;
  }

  //Search through the transparency list looking for a match
  bool myTransparentPixelFound = false;
  TransparentSingleValuePixel myTransparentPixel = {0, 0, 100};
  for ( int myListRunner = 0; myListRunner < mTransparentSingleValuePixelList.count(); myListRunner++ )
  {
    myTransparentPixel = mTransparentSingleValuePixelList[myListRunner];
    if ( ( value >= myTransparentPixel.min && value <= myTransparentPixel.max ) ||
         qgsDoubleNear( value, myTransparentPixel.min ) ||
         qgsDoubleNear( value, myTransparentPixel.max ) )
    {
      myTransparentPixelFound = true;
      break;
    }
  }

  //if a match was found use the stored transparency percentage
  if ( myTransparentPixelFound )
  {
    return static_cast< int >( static_cast< float >( globalTransparency ) * ( 1.0 - ( myTransparentPixel.percentTransparent / 100.0 ) ) );
  }

  return globalTransparency;
}

int QgsRasterTransparency::alphaValue( double redValue, double greenValue, double blueValue, int globalTransparency ) const
{
  //if NaN return 0, transparent
  if ( std::isnan( redValue ) || std::isnan( greenValue ) || std::isnan( blueValue ) )
  {
    return 0;
  }

  //Search through the transparency list looking for a match
  bool myTransparentPixelFound = false;
  TransparentThreeValuePixel myTransparentPixel = {0, 0, 0, 100};
  for ( int myListRunner = 0; myListRunner < mTransparentThreeValuePixelList.count(); myListRunner++ )
  {
    myTransparentPixel = mTransparentThreeValuePixelList[myListRunner];
    if ( qgsDoubleNear( myTransparentPixel.red, redValue ) )
    {
      if ( qgsDoubleNear( myTransparentPixel.green, greenValue ) )
      {
        if ( qgsDoubleNear( myTransparentPixel.blue, blueValue ) )
        {
          myTransparentPixelFound = true;
          break;
        }
      }
    }
  }

  //if a match was found use the stored transparency percentage
  if ( myTransparentPixelFound )
  {
    return static_cast< int >( static_cast< float >( globalTransparency ) * ( 1.0 - ( myTransparentPixel.percentTransparent / 100.0 ) ) );
  }

  return globalTransparency;
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
    QList<QgsRasterTransparency::TransparentSingleValuePixel>::const_iterator it = mTransparentSingleValuePixelList.constBegin();
    for ( ; it != mTransparentSingleValuePixelList.constEnd(); ++it )
    {
      QDomElement pixelListElement = doc.createElement( QStringLiteral( "pixelListEntry" ) );
      pixelListElement.setAttribute( QStringLiteral( "min" ), QgsRasterBlock::printValue( it->min ) );
      pixelListElement.setAttribute( QStringLiteral( "max" ), QgsRasterBlock::printValue( it->max ) );
      pixelListElement.setAttribute( QStringLiteral( "percentTransparent" ), QString::number( it->percentTransparent ) );
      singleValuePixelListElement.appendChild( pixelListElement );
    }
    rasterTransparencyElem.appendChild( singleValuePixelListElement );

  }
  if ( !mTransparentThreeValuePixelList.isEmpty() )
  {
    QDomElement threeValuePixelListElement = doc.createElement( QStringLiteral( "threeValuePixelList" ) );
    QList<QgsRasterTransparency::TransparentThreeValuePixel>::const_iterator it = mTransparentThreeValuePixelList.constBegin();
    for ( ; it != mTransparentThreeValuePixelList.constEnd(); ++it )
    {
      QDomElement pixelListElement = doc.createElement( QStringLiteral( "pixelListEntry" ) );
      pixelListElement.setAttribute( QStringLiteral( "red" ), QgsRasterBlock::printValue( it->red ) );
      pixelListElement.setAttribute( QStringLiteral( "green" ), QgsRasterBlock::printValue( it->green ) );
      pixelListElement.setAttribute( QStringLiteral( "blue" ), QgsRasterBlock::printValue( it->blue ) );
      pixelListElement.setAttribute( QStringLiteral( "percentTransparent" ), QString::number( it->percentTransparent ) );
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
    TransparentSingleValuePixel sp;
    for ( int i = 0; i < entryList.size(); ++i )
    {
      currentEntryElem = entryList.at( i ).toElement();
      sp.percentTransparent = currentEntryElem.attribute( QStringLiteral( "percentTransparent" ) ).toDouble();
      // Backward compoatibility < 1.9 : pixelValue (before ranges)
      if ( currentEntryElem.hasAttribute( QStringLiteral( "pixelValue" ) ) )
      {
        sp.min = sp.max = currentEntryElem.attribute( QStringLiteral( "pixelValue" ) ).toDouble();
      }
      else
      {
        sp.min = currentEntryElem.attribute( QStringLiteral( "min" ) ).toDouble();
        sp.max = currentEntryElem.attribute( QStringLiteral( "max" ) ).toDouble();
      }
      mTransparentSingleValuePixelList.append( sp );
    }
  }
  const QDomElement threeValuePixelListElem = elem.firstChildElement( QStringLiteral( "threeValuePixelList" ) );
  if ( !threeValuePixelListElem.isNull() )
  {
    const QDomNodeList entryList = threeValuePixelListElem.elementsByTagName( QStringLiteral( "pixelListEntry" ) );
    TransparentThreeValuePixel tp;
    for ( int i = 0; i < entryList.size(); ++i )
    {
      currentEntryElem = entryList.at( i ).toElement();
      tp.red = currentEntryElem.attribute( QStringLiteral( "red" ) ).toDouble();
      tp.green = currentEntryElem.attribute( QStringLiteral( "green" ) ).toDouble();
      tp.blue = currentEntryElem.attribute( QStringLiteral( "blue" ) ).toDouble();
      tp.percentTransparent = currentEntryElem.attribute( QStringLiteral( "percentTransparent" ) ).toDouble();
      mTransparentThreeValuePixelList.append( tp );
    }
  }
}
