/***************************************************************************
                qgsrastertransparency.cpp -  description
                       -------------------
begin                : Mon Nov 30 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastertransparency.h"

#include "qgis.h"
#include "qgsrasterinterface.h"

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
    return qgsDoubleNear( p.red, redValue, p.fuzzyToleranceRed )
           && qgsDoubleNear( p.green, greenValue, p.fuzzyToleranceGreen )
           && qgsDoubleNear( p.blue, blueValue, p.fuzzyToleranceBlue );
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
  QDomElement rasterTransparencyElem = doc.createElement( u"rasterTransparency"_s );
  if ( !mTransparentSingleValuePixelList.isEmpty() )
  {
    QDomElement singleValuePixelListElement = doc.createElement( u"singleValuePixelList"_s );
    auto it = mTransparentSingleValuePixelList.constBegin();
    for ( ; it != mTransparentSingleValuePixelList.constEnd(); ++it )
    {
      QDomElement pixelListElement = doc.createElement( u"pixelListEntry"_s );
      pixelListElement.setAttribute( u"min"_s, QgsRasterBlock::printValue( it->min ) );
      pixelListElement.setAttribute( u"max"_s, QgsRasterBlock::printValue( it->max ) );
      pixelListElement.setAttribute( u"percentTransparent"_s, QString::number( 100.0 * ( 1 - it->opacity ) ) );
      singleValuePixelListElement.appendChild( pixelListElement );
    }
    rasterTransparencyElem.appendChild( singleValuePixelListElement );

  }
  if ( !mTransparentThreeValuePixelList.isEmpty() )
  {
    QDomElement threeValuePixelListElement = doc.createElement( u"threeValuePixelList"_s );
    auto it = mTransparentThreeValuePixelList.constBegin();
    for ( ; it != mTransparentThreeValuePixelList.constEnd(); ++it )
    {
      QDomElement pixelListElement = doc.createElement( u"pixelListEntry"_s );
      pixelListElement.setAttribute( u"red"_s, QgsRasterBlock::printValue( it->red ) );
      pixelListElement.setAttribute( u"green"_s, QgsRasterBlock::printValue( it->green ) );
      pixelListElement.setAttribute( u"blue"_s, QgsRasterBlock::printValue( it->blue ) );
      pixelListElement.setAttribute( u"percentTransparent"_s, QString::number( 100.0 * ( 1 - it->opacity ) ) );
      if ( !qgsDoubleNear( it->fuzzyToleranceRed, 0 ) )
        pixelListElement.setAttribute( u"toleranceRed"_s, QString::number( it->fuzzyToleranceRed ) );
      if ( !qgsDoubleNear( it->fuzzyToleranceGreen, 0 ) )
        pixelListElement.setAttribute( u"toleranceGreen"_s, QString::number( it->fuzzyToleranceGreen ) );
      if ( !qgsDoubleNear( it->fuzzyToleranceBlue, 0 ) )
        pixelListElement.setAttribute( u"toleranceBlue"_s, QString::number( it->fuzzyToleranceBlue ) );
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

  const QDomElement singlePixelListElem = elem.firstChildElement( u"singleValuePixelList"_s );
  if ( !singlePixelListElem.isNull() )
  {
    const QDomNodeList entryList = singlePixelListElem.elementsByTagName( u"pixelListEntry"_s );
    for ( int i = 0; i < entryList.size(); ++i )
    {
      currentEntryElem = entryList.at( i ).toElement();
      double min = 0;
      double max = 0;
      const double opacity = 1.0 - currentEntryElem.attribute( u"percentTransparent"_s ).toDouble() / 100.0;
      // Backward compoatibility < 1.9 : pixelValue (before ranges)
      if ( currentEntryElem.hasAttribute( u"pixelValue"_s ) )
      {
        min = max = currentEntryElem.attribute( u"pixelValue"_s ).toDouble();
      }
      else
      {
        min = currentEntryElem.attribute( u"min"_s ).toDouble();
        max = currentEntryElem.attribute( u"max"_s ).toDouble();
      }
      mTransparentSingleValuePixelList.append( TransparentSingleValuePixel( min, max, opacity ) );
    }
  }
  const QDomElement threeValuePixelListElem = elem.firstChildElement( u"threeValuePixelList"_s );
  if ( !threeValuePixelListElem.isNull() )
  {
    const QDomNodeList entryList = threeValuePixelListElem.elementsByTagName( u"pixelListEntry"_s );
    for ( int i = 0; i < entryList.size(); ++i )
    {
      currentEntryElem = entryList.at( i ).toElement();
      const double red = currentEntryElem.attribute( u"red"_s ).toDouble();
      const double green = currentEntryElem.attribute( u"green"_s ).toDouble();
      const double blue = currentEntryElem.attribute( u"blue"_s ).toDouble();
      const double opacity = 1.0 - currentEntryElem.attribute( u"percentTransparent"_s ).toDouble() / 100.0;
      bool redOk = false;
      const double toleranceRed = currentEntryElem.attribute( u"toleranceRed"_s ).toDouble( &redOk );
      bool greenOk = false;
      const double toleranceGreen = currentEntryElem.attribute( u"toleranceGreen"_s ).toDouble( &greenOk );
      bool blueOk = false;
      const double toleranceBlue = currentEntryElem.attribute( u"toleranceBlue"_s ).toDouble( &blueOk );
      mTransparentThreeValuePixelList.append( TransparentThreeValuePixel( red, green, blue, opacity,
                                              redOk ? toleranceRed : 4 * std::numeric_limits<double>::epsilon(),
                                              greenOk ? toleranceGreen : 4 * std::numeric_limits<double>::epsilon(),
                                              blueOk ? toleranceBlue : 4 * std::numeric_limits<double>::epsilon() ) );
    }
  }
}
