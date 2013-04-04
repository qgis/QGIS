/***************************************************************************
                              qgsfilter.cpp
                              -------------
  begin                : Jan 31, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfilter.h"
#include "qgsbetweenfilter.h"
#include "qgscomparisonfilter.h"
#include "qgslogicalfilter.h"
#include "qgsspatialfilter.h"
#include "qgsvectordataprovider.h"
#include "qgsogcutils.h"

#include <QDomElement>
#include <QStringList>
#include "qgsvectorlayer.h"

QgsFilter::QgsFilter(): mPropertyIndex( -1 )
{
}

QgsFilter::QgsFilter( int propertyIndex ): mPropertyIndex( propertyIndex )
{
}

QgsFilter::~QgsFilter()
{
}

QVariant QgsFilter::propertyIndexValue( const QgsFeature& f ) const
{
  return f.attribute( mPropertyIndex );
}

QList<int> QgsFilter::attributeIndices() const
{
  QList<int> resultList;
  resultList.push_back( mPropertyIndex );
  return resultList;
}

QgsFilter* QgsFilter::createFilterFromXml( const QDomElement& filterElem, QgsVectorLayer* vl )
{
  if ( filterElem.isNull() || !vl )
  {
    return 0;
  }

  //Name of the element indicates the type of filter
  //using tagName and to upper for interoperability
  QString filterName = filterElem.tagName().toUpper();

  if ( filterName == "AND" || filterName == "OR" || filterName == "NOT" )
  {
    //find the first filter subelement and create a filter from it
    QDomElement firstChild = filterElem.firstChild().toElement();
    if ( firstChild.isNull() )
    {
      return 0;
    }
    QgsFilter* subFilter1 = QgsFilter::createFilterFromXml( firstChild, vl );
    if ( subFilter1 && filterName == "NOT" )
    {
      return new QgsLogicalFilter( QgsLogicalFilter::NOT, subFilter1, 0 );
    }

    QDomElement secondChild = firstChild.nextSibling().toElement();
    if ( secondChild.isNull() )
    {
      delete subFilter1; return 0;
    }
    QgsFilter* subFilter2 = QgsFilter::createFilterFromXml( secondChild, vl );
    if ( subFilter1 && subFilter2 )
    {
      if ( filterName == "AND" )
      {
        return new QgsLogicalFilter( QgsLogicalFilter::AND, subFilter1, subFilter2 );
      }
      else if ( filterName == "OR" )
      {
        return new QgsLogicalFilter( QgsLogicalFilter::OR, subFilter1, subFilter2 );
      }
    }
    delete subFilter1; delete subFilter2; return 0;
  }
  // if the filter is spatial
  if ( filterName == "BBOX" || filterName == "CONTAINS" || filterName == "CROSSES" || filterName == "DISJOINT" || filterName == "EQUALS" || filterName == "INTERSECTS" || filterName == "OVERLAPS" || filterName == "TOUCHES" || filterName == "WITHIN" )
  {
    QgsGeometry *geom = 0;

    QDomNodeList bNodes = filterElem.elementsByTagName( "Box" );
    if ( bNodes.size() > 0 )
    {
      QDomElement bElem = bNodes.at( 0 ).toElement().firstChild().toElement();
      QString coordSeparator = ",";
      QString tupelSeparator = " ";
      if ( bElem.hasAttribute( "cs" ) )
      {
        coordSeparator = bElem.attribute( "cs" );
      }
      if ( bElem.hasAttribute( "ts" ) )
      {
        tupelSeparator = bElem.attribute( "ts" );
      }

      QString bString = bElem.text();
      bool conversionSuccess;
      double minx = bString.section( tupelSeparator, 0, 0 ).section( coordSeparator, 0, 0 ).toDouble( &conversionSuccess );
      double miny = bString.section( tupelSeparator, 0, 0 ).section( coordSeparator, 1, 1 ).toDouble( &conversionSuccess );
      double maxx = bString.section( tupelSeparator, 1, 1 ).section( coordSeparator, 0, 0 ).toDouble( &conversionSuccess );
      double maxy = bString.section( tupelSeparator, 1, 1 ).section( coordSeparator, 1, 1 ).toDouble( &conversionSuccess );
      QgsRectangle* rect = new QgsRectangle( minx, miny, maxx, maxy );
      geom = QgsGeometry::fromRect( *rect );
    }

    if ( !geom )
    {
      QDomNodeList gNodes = filterElem.elementsByTagName( "Point" );
      if ( gNodes.size() > 0 )
      {
        QDomElement gElem = gNodes.at( 0 ).toElement();
        geom = QgsOgcUtils::geometryFromGML( gElem );
      }
    }

    if ( !geom )
    {
      QDomNodeList gNodes = filterElem.elementsByTagName( "LineString" );
      if ( gNodes.size() > 0 )
      {
        QDomElement gElem = gNodes.at( 0 ).toElement();
        geom = QgsOgcUtils::geometryFromGML( gElem );
      }
    }

    if ( !geom )
    {
      QDomNodeList gNodes = filterElem.elementsByTagName( "Polygon" );
      if ( gNodes.size() > 0 )
      {
        QDomElement gElem = gNodes.at( 0 ).toElement();
        geom = QgsOgcUtils::geometryFromGML( gElem );
      }
    }

    if ( !geom )
    {
      QDomNodeList gNodes = filterElem.elementsByTagName( "MultiPoint" );
      if ( gNodes.size() > 0 )
      {
        QDomElement gElem = gNodes.at( 0 ).toElement();
        geom = QgsOgcUtils::geometryFromGML( gElem );
      }
    }

    if ( !geom )
    {
      QDomNodeList gNodes = filterElem.elementsByTagName( "MultiLineString" );
      if ( gNodes.size() > 0 )
      {
        QDomElement gElem = gNodes.at( 0 ).toElement();
        geom = QgsOgcUtils::geometryFromGML( gElem );
      }
    }

    if ( !geom )
    {
      QDomNodeList gNodes = filterElem.elementsByTagName( "MultiPolygon" );
      if ( gNodes.size() > 0 )
      {
        QDomElement gElem = gNodes.at( 0 ).toElement();
        geom = QgsOgcUtils::geometryFromGML( gElem );
      }
    }

    if ( !geom )
      return 0;

    if ( filterName == "BBOX" )
      return new QgsSpatialFilter( QgsSpatialFilter::BBOX, geom );
    if ( filterName == "CONTAINS" )
      return new QgsSpatialFilter( QgsSpatialFilter::CONTAINS, geom );
    if ( filterName == "CROSSES" )
      return new QgsSpatialFilter( QgsSpatialFilter::CROSSES, geom );
    if ( filterName == "DISJOINT" )
      return new QgsSpatialFilter( QgsSpatialFilter::DISJOINT, geom );
    if ( filterName == "EQUALS" )
      return new QgsSpatialFilter( QgsSpatialFilter::EQUALS, geom );
    if ( filterName == "INTERSECTS" )
      return new QgsSpatialFilter( QgsSpatialFilter::INTERSECTS, geom );
    if ( filterName == "OVERLAPS" )
      return new QgsSpatialFilter( QgsSpatialFilter::OVERLAPS, geom );
    if ( filterName == "TOUCHES" )
      return new QgsSpatialFilter( QgsSpatialFilter::TOUCHES, geom );
    if ( filterName == "WITHIN" )
      return new QgsSpatialFilter( QgsSpatialFilter::WITHIN, geom );
    return 0;
  }

  //assume it must be a comparison filter

  //get property name
  QString attributeName;
  QDomNodeList propertyNameList = filterElem.elementsByTagName( "PropertyName" );
  if ( propertyNameList.size() > 0 )
  {
    attributeName = propertyNameList.item( 0 ).toElement().text();
    //remove namespace prefixes from the beginning of the attribute name
    if ( attributeName.contains( ":" ) )
    {
      attributeName = attributeName.section( ":", 1, 1 );
    }
  }
  else
  {
    return 0;
  }

  //get property index
  int attributeIndex = -1;
  if ( vl->dataProvider() )
  {
    attributeIndex = vl->dataProvider()->fieldNameIndex( attributeName );
  }

  if ( attributeIndex == -1 )
  {
    return 0;
  }

  //get literal value(s)
  QStringList literalList;

  //Literal elemement
  QDomNodeList literalNodeList = filterElem.elementsByTagName( "Literal" );
  if ( literalNodeList.size() == 1 ) //property is a BinaryComparisonOperatorType (only one literal)
  {
    literalList.push_back( literalNodeList.item( 0 ).toElement().text() );
  }
  else //is property ogc:PropertyIsBetween and has <LowerBoundary> and <UpperBoundary> ?
  {
    QDomNodeList lowerBoundaryList = filterElem.elementsByTagName( "LowerBoundary" );
    QDomNodeList upperBoundaryList = filterElem.elementsByTagName( "UpperBoundary" );
    if ( lowerBoundaryList.size() > 0 && upperBoundaryList.size() > 0 )
    {
      QDomElement lowerBoundaryElement = lowerBoundaryList.item( 0 ).toElement();
      QDomNodeList lowerLiteralList = lowerBoundaryElement.elementsByTagName( "Literal" );
      QDomElement upperBoundaryElement = upperBoundaryList.item( 0 ).toElement();
      QDomNodeList upperLiteralList = upperBoundaryElement.elementsByTagName( "Literal" );
      if ( lowerLiteralList.size() > 0 && upperLiteralList.size() > 0 )
      {
        literalList.push_back( lowerLiteralList.item( 0 ).toElement().text() );
        literalList.push_back( upperLiteralList.item( 0 ).toElement().text() );
      }
    }
  }

  //now create the filter

  //comparison filter?
  if ( filterName == "PROPERTYISEQUALTO" )
  {
    return new QgsComparisonFilter( attributeIndex, QgsComparisonFilter::EQUAL, literalList.at( 0 ) );
  }
  else if ( filterName == "PROPERTYISNOTEQUALTO" )
  {
    return new QgsComparisonFilter( attributeIndex, QgsComparisonFilter::NOT_EQUAL, literalList.at( 0 ) );
  }
  else if ( filterName == "PROPERTYISLESSTHAN" )
  {
    return new QgsComparisonFilter( attributeIndex, QgsComparisonFilter::LESSER, literalList.at( 0 ) );
  }
  else if ( filterName == "PROPERTYISGREATERTHAN" )
  {
    return new QgsComparisonFilter( attributeIndex, QgsComparisonFilter::GREATER, literalList.at( 0 ) );
  }
  else if ( filterName == "PROPERTYISLESSTHANOREQUALTO" )
  {
    return new QgsComparisonFilter( attributeIndex, QgsComparisonFilter::LESSER_OR_EQUAL, literalList.at( 0 ) );
  }
  else if ( filterName == "PROPERTYISGREATERTHANOREQUALTO" )
  {
    return new QgsComparisonFilter( attributeIndex, QgsComparisonFilter::GREATER_OR_EQUAL, literalList.at( 0 ) );
  }

  //between filter?
  else if ( filterName == "PROPERTYISBETWEEN" )
  {
    return new QgsBetweenFilter( attributeIndex, literalList.at( 0 ), literalList.at( 1 ) );
  }

  return 0;
}
