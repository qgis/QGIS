/***************************************************************************
                              qgssldrule.cpp
                              --------------
  begin                : Feb 1, 2008
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

#include "qgssldrule.h"
#include "qgsfilter.h"
#include "qgsmapserverlogger.h"
#include "qgsmsutils.h"
#include "qgssymbol.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QDomElement>
#include <QSvgRenderer>
#include <QTemporaryFile>
#include <QPainter>
#include <cmath>

QgsSLDRule::QgsSLDRule( double minDenom, double maxDenom, const QgsSymbol& s, const QgsFilter* f ): mSymbol( s ), mMinScaleDenominator( minDenom ), mMaxScaleDenominator( maxDenom )
{

}

QgsSLDRule::~QgsSLDRule()
{

}

int QgsSLDRule::applySymbology( QPainter* p, QgsFeature& f, QImage* pic, bool selected, double widthScale, double rasterScaleFactor, QGis::GeometryType type, const QColor& selectionColor )
{
  if ( pic && type == QGis::Point )
  {
    double rotation = 0.0;
    double fieldScale = 1.0;

    if ( mSymbol.rotationClassificationField() >= 0 )
    {
      const QgsAttributeMap& attrs = f.attributeMap();
      rotation = attrs[mSymbol.rotationClassificationField()].toDouble();
    }
    if ( mSymbol.scaleClassificationField() >= 0 )
    {
      //first find out the value for the scale classification attribute
      const QgsAttributeMap& attrs = f.attributeMap();
      fieldScale = sqrt( qAbs( attrs[mSymbol.scaleClassificationField()].toDouble() ) );
    }

    *pic = mSymbol.getPointSymbolAsImage( widthScale, selected, selectionColor, fieldScale, rotation, rasterScaleFactor );
  }
  else if ( type != QGis::Point )
  {
    QPen pen = mSymbol.pen();
    pen.setWidthF( widthScale * pen.widthF() );
    p->setPen( pen );
    p->setBrush( mSymbol.brush() );
  }
  return 0;
}

bool QgsSLDRule::evaluate( const QgsFeature& f, double currentScaleDenominator ) const
{
  //test against scale denominators
  if ( mMinScaleDenominator != 0 )
  {
    if ( currentScaleDenominator < mMinScaleDenominator )
    {
      return false;
    }
  }

  if ( mMaxScaleDenominator != 0 )
  {
    if ( currentScaleDenominator > mMaxScaleDenominator )
    {
      return false;
    }
  }

  //test against filter
  if ( mFilter )
  {
    if ( !mFilter->evaluate( f ) )
    {
      return false;
    }
  }

  return true;
}

int QgsSLDRule::setFromXml( const QDomElement& ruleElement, QgsVectorLayer* vl, QList<QTemporaryFile*>& filesToRemove )
{
  if ( ruleElement.isNull() )
  {
    return 1;
  }

  QString attributeName;
  QString lowerBound;
  QString upperBound;
  QString filterString;

  mFilterIndependentClassificationIndices.clear();

  //default value is to ignore the min/max scale denominators
  double minScaleDenominator = 0;
  double maxScaleDenominator = 0;
  double denomValue = 0;
  bool conversionSuccess;//flag for QString -> double conversion

  //check for min/max scale denominators
  QDomNodeList minScaleDenomList = ruleElement.elementsByTagName( "MinScaleDenominator" );
  if ( minScaleDenomList.size() > 0 )
  {
    QString minScaleText = minScaleDenomList.item( 0 ).toElement().text();
    denomValue = minScaleText.toDouble( &conversionSuccess );
    if ( conversionSuccess )
    {
      minScaleDenominator = denomValue;
    }
  }
  QDomNodeList maxScaleDenomList = ruleElement.elementsByTagName( "MaxScaleDenominator" );
  if ( maxScaleDenomList.size() > 0 )
  {
    QString maxScaleText = maxScaleDenomList.item( 0 ).toElement().text();
    denomValue = maxScaleText.toDouble( &conversionSuccess );
    if ( conversionSuccess )
    {
      maxScaleDenominator = denomValue;
    }
  }
  mMinScaleDenominator = minScaleDenominator;
  mMaxScaleDenominator = maxScaleDenominator;

  //Symbolizer
  //iterate all rule children until a symbolizer is found
  QDomNodeList ruleChildNodes = ruleElement.childNodes();
  QDomElement currentChildElem;

  bool symbologyFound = false;
  for ( int i = 0; i < ruleChildNodes.size(); ++i )
  {
    currentChildElem = ruleChildNodes.at( i ).toElement();

    //line?
    if ( currentChildElem.tagName() == ( "LineSymbolizer" ) )
    {
      QPen p;
      if ( symbologyFromLineSymbolizer( currentChildElem, p ) == 0 )
      {
        mSymbol.setPen( p );
      }
      else //error
      {
        return 2;
      }
      symbologyFound = true;
      break;
    }

    //polygon?
    if ( currentChildElem.tagName() == ( "PolygonSymbolizer" ) )
    {
      QPen p;
      QBrush b;
      if ( symbologyFromPolygonSymbolizer( currentChildElem, p, b ) == 0 )
      {
        mSymbol.setPen( p );
        mSymbol.setBrush( b );
      }
      else //error
      {
        return 3;
      }
      symbologyFound = true;
      break;
    }

    //point?
    if ( currentChildElem.tagName() == ( "PointSymbolizer" ) )
    {
      if ( symbologyFromPointSymbolizer( currentChildElem, mSymbol, vl->dataProvider(), filesToRemove ) != 0 )
      {
        return 4;
      }
      symbologyFound = true;
    }
  }

  if ( !symbologyFound ) //don't draw features with no symbology
  {
    mSymbol.setPen( Qt::NoPen );
    mSymbol.setBrush( Qt::NoBrush );
  }

  //search for filters
  //if the filter is 0, the rule will always be applied
  mFilter = 0;
  QDomNodeList filterNodeList = ruleElement.elementsByTagName( "Filter" );
  if ( filterNodeList.size() > 0 )
  {
    QDomElement filterElement = filterNodeList.item( 0 ).toElement();
    if ( !filterElement.isNull() )
    {
      mFilter = QgsFilter::createFilterFromXml( filterElement.firstChild().toElement(), vl );
    }
  }

  return 0;
}


int QgsSLDRule::symbologyFromLineSymbolizer( const QDomElement& lineSymbolizerElement, QPen& pen ) const
{
  //find the stroke subnode
  QDomNodeList strokeNodeList = lineSymbolizerElement.elementsByTagName( "Stroke" );
  if ( strokeNodeList.length() < 1 )
  {
    //from the specs: "The Stroke element ...its absence means that no stroke is to be rendered."
    pen.setStyle( Qt::NoPen );
  }
  else
  {
    penFromStrokeElement( strokeNodeList.item( 0 ).toElement(), pen );
  }
  return 0;
}

int QgsSLDRule::symbologyFromPointSymbolizer( const QDomElement& pointSymbolizerElement, QgsSymbol& symbol, const QgsVectorDataProvider* dp, QList<QTemporaryFile*>& filesToRemove )
{
  if ( pointSymbolizerElement.isNull() )
  {
    return 1;
  }

  QDomNodeList graphicNodeList = pointSymbolizerElement.elementsByTagName( "Graphic" );
  if ( graphicNodeList.size() > 0 )
  {
    QDomElement graphicElement = graphicNodeList.item( 0 ).toElement();
    QDomNodeList markNodeList = graphicElement.elementsByTagName( "Mark" );
    if ( markNodeList.size() > 0 )
    {
      QString symbolName;
      int size = -1;
      QPen pen;
      QBrush brush;

      //well known name symbol
      QDomElement markElement = markNodeList.item( 0 ).toElement();
      QDomNodeList wellKnownNameNodeList = markElement.elementsByTagName( "WellKnownName" );
      if ( wellKnownNameNodeList.size() > 0 )
      {
        //can be "square", "circle", "triangle", "star", "cross" and "x"
        QString wellKnownName = wellKnownNameNodeList.item( 0 ).toElement().text();
        if ( wellKnownName == "cross" )
        {
          symbolName = "hard:cross";
        }
        else if ( wellKnownName == "x" )
        {
          symbolName = "hard:cross2";
        }
        else if ( wellKnownName == "circle" )
        {
          symbolName = "hard:circle";
        }
        else if ( wellKnownName == "triangle" )
        {
          symbolName = "hard:triangle";
        }
        else if ( wellKnownName == "star" )
        {
          symbolName = "hard:star";
        }
        else //default is a rectangle
        {
          symbolName = "hard:rectangle";
        }

        QDomNodeList fillNodeList = markElement.elementsByTagName( "Fill" );
        if ( fillNodeList.size() > 0 )
        {
          brushFromFillElement( fillNodeList.item( 0 ).toElement(), brush );
        }

        QDomNodeList strokeNodeList = markElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Stroke" );
        if ( strokeNodeList.size() > 0 )
        {
          penFromStrokeElement( strokeNodeList.item( 0 ).toElement(), pen );
        }

        QDomNodeList opacityNodeList = pointSymbolizerElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Opacity" );
        if ( opacityNodeList.size() > 0 )
        {
          QString opacityString = opacityNodeList.item( 0 ).toElement().text();
          bool ok;
          double opacity = opacityString.toDouble( &ok );
          if ( ok )
          {
            QColor bc = brush.color();
            brush.setColor( QColor( bc.red(), bc.green(), bc.blue(), ( int )( opacity*255 ) ) );
            QColor pc = pen.color();
            pen.setColor( QColor( pc.red(), pc.green(), pc.blue(), ( int )( opacity*255 ) ) );
          }
        }


      }
      else //svg symbol?
      {
        QDomNodeList svgSymbolList = markElement.elementsByTagName( "SvgSymbol" );
        if ( svgSymbolList.size() > 0 )
        {
          QDomElement svgSymbolElem = svgSymbolList.at( 0 ).toElement();
          QDomNodeList svgNodeList = svgSymbolElem.elementsByTagName( "svg" );
          if ( svgNodeList.size() > 0 )
          {
            //create svg document
            QDomElement svgElem = svgNodeList.at( 0 ).toElement();
            QDomDocument svgDocument;
            svgDocument.appendChild( svgDocument.importNode( svgElem, true ) );

            //and store it in a temporary file
            QTemporaryFile* tmpFile = new QTemporaryFile();
            if ( !tmpFile->open() )
            {
              delete tmpFile;
            }
            QTextStream fileStream( tmpFile );
            fileStream << svgDocument.toString( 4 );
            tmpFile->flush();
            filesToRemove.push_back( tmpFile );
            symbolName = "svg:" + tmpFile->fileName();
          }
        }
      }

      //rotation
      QDomNodeList rotationList = graphicElement.elementsByTagName( "Rotation" );
      if ( rotationList.size() > 0 )
      {
        QDomNode propertyNameNode = rotationList.at( 0 ).namedItem( "PropertyName" );
        if ( !propertyNameNode.isNull() )
        {
          QString attributeName = propertyNameNode.toElement().text();
          if ( !attributeName.isEmpty() && dp )
          {
            int rotationField = dp->fieldNameIndex( attributeName );
            if ( rotationField != -1 )
            {
              symbol.setRotationClassificationField( rotationField );
              mFilterIndependentClassificationIndices.insert( rotationField );
            }
          }
        }
      }

      //size needed for wks and svg
      QDomNodeList sizeNodeList = pointSymbolizerElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Size" );
      if ( sizeNodeList.size() > 0 )
      {
        //data dependent size?
        QDomNode multiplicatorNameNode = sizeNodeList.at( 0 ).namedItem( "Multiplicator" );
        QDomNode propertyNameNode = sizeNodeList.at( 0 ).namedItem( "PropertyName" );
        if ( !propertyNameNode.isNull() )
        {
          QString attributeName = propertyNameNode.toElement().text();
          if ( !attributeName.isEmpty() && dp )
          {
            int scaleField = dp->fieldNameIndex( attributeName );
            if ( scaleField != -1 )
            {
              symbol.setScaleClassificationField( scaleField );
              mFilterIndependentClassificationIndices.insert( scaleField );
            }
          }
        }
        //fixed size
        QString sizeString = sizeNodeList.item( 0 ).toElement().text();
        bool ok;
        double markerSize = sizeString.toDouble( &ok );
        if ( ok )
        {
          size = ( int )markerSize;
        }
      }

      symbol.setNamedPointSymbol( symbolName );
      symbol.setPointSize( size );

      if ( symbolName.startsWith( "hard:" ) )
      {
        symbol.setPen( pen );
        symbol.setBrush( brush );
      }
      return 0;
    }
  }
  return 3;
}

int QgsSLDRule::symbologyFromPolygonSymbolizer( const QDomElement& polySymbolizerElement, QPen& pen, QBrush& brush ) const
{
  if ( polySymbolizerElement.isNull() )
  {
    return 1;
  }

  QDomNodeList strokeNodeList = polySymbolizerElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Stroke" );
  if ( strokeNodeList.size() < 1 )
  {
    //from the specs: " a missing Stroke element means the geometry will no be stroked"
    pen.setStyle( Qt::NoPen );
  }
  else
  {
    penFromStrokeElement( strokeNodeList.item( 0 ).toElement(), pen );
  }

  QDomNodeList fillNodeList = polySymbolizerElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Fill" );
  if ( fillNodeList.size() < 1 )
  {
    //from the specs: "If the Fill element is omitted from its parent element, then no fill will be rendered."
    brush.setStyle( Qt::NoBrush );
  }
  else
  {
    brushFromFillElement( fillNodeList.item( 0 ).toElement(), brush );
  }
  return 0;
}

int QgsSLDRule::brushFromFillElement( const QDomElement& fillElement, QBrush& brush ) const
{
  if ( fillElement.isNull() )
  {
    return 1;
  }

  //<CssParameter> or <svgPattern>?
  QDomNodeList cssNodeList = fillElement.elementsByTagName( "CssParameter" );
  if ( cssNodeList.size() > 0 )
  {
    return brushFromSvgParameters( fillElement, brush );
  }

  QDomNodeList svgPatternNodeList = fillElement.elementsByTagName( "pattern" );
  if ( svgPatternNodeList.size() > 0 )
  {
    return brushFromSvgPattern( svgPatternNodeList.at( 0 ).toElement(), brush );
  }

  brush.setStyle( Qt::NoBrush );
  return 0;
}

int QgsSLDRule::brushFromSvgParameters( const QDomElement& fillElement, QBrush& brush ) const
{
  QString elemText;
  int opacity = 255;
  int polyColorRed = 0;
  int polyColorGreen = 0;
  int polyColorBlue = 0;
  Qt::BrushStyle brushStyle = Qt::SolidPattern;

  //visit all the <CssParameter> elements and apply the settings step by step
  if ( !fillElement.isNull() )
  {
    QDomNodeList cssNodes = fillElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "CssParameter" );
    QString cssName;
    QDomElement currentElement;
    for ( int i = 0; i < cssNodes.size(); ++i )
    {
      currentElement = cssNodes.item( i ).toElement();
      if ( currentElement.isNull() )
      {
        continue;
      }
      QString elemText = currentElement.text();

      //switch depending on attribute 'name'
      cssName = currentElement.attribute/*NS*/( /*mSLDNamespace,*/ "name", "not found" );
      if ( cssName != "not found" )
      {
        if ( cssName == "fill" )
        {
          //accept input in the form of #ff00ff
          if ( elemText.length() == 7 )
          {
            bool success;
            polyColorRed = elemText.mid( 1, 2 ).toInt( &success, 16 );
            if ( !success )
            {
              polyColorRed = 0;
            }
            polyColorGreen = elemText.mid( 3, 2 ).toInt( &success, 16 );
            if ( !success )
            {
              polyColorGreen = 0;
            }
            polyColorBlue = elemText.mid( 5, 2 ).toInt( &success, 16 );
            if ( !success )
            {
              polyColorBlue = 0;
            }
          }
        }
        else if ( cssName == "fill-opacity" )
        {
          bool success;
          double op = elemText.toDouble( &success );
          if ( success )
          {
            if ( op > 1.0 )
            {
              opacity = 255;
            }
            else if ( op < 0.0 )
            {
              opacity = 0;
            }
            else
            {
              opacity = ( int )( 255 * op );
            }
          }
        }
      }
    }
  }
  brush.setColor( QColor( polyColorRed, polyColorGreen, polyColorBlue, opacity ) );
  brush.setStyle( brushStyle );
  return 0;
}

int QgsSLDRule::brushFromSvgPattern( const QDomElement& svgPatternElement, QBrush& brush ) const
{
  QgsMSDebugMsg( "Entering QgsSLDRule::brushFromSvgPattern" );

  if ( svgPatternElement.isNull() )
  {
    return 1;
  }

  int patternWidth = svgPatternElement.attribute( "width" ).toInt();
  int patternHeight = svgPatternElement.attribute( "height" ).toInt();

  int x = svgPatternElement.attribute( "x" ).toInt();
  int y = svgPatternElement.attribute( "y" ).toInt();

  QString patternUnits = svgPatternElement.attribute( "patternUnits" );

  QSvgRenderer renderer;

  //find svg group element
  QDomNodeList svgGroupNodeList = svgPatternElement.elementsByTagName( "g" );
  if ( svgGroupNodeList.size() < 1 )
  {
    return 2;
  }

  QDomElement svgGroupElem = svgGroupNodeList.at( 0 ).toElement();

  QDomDocument svgDocument;
  QDomElement svgElem = svgDocument.createElementNS( "http://www.w3.org/2000/svg", "svg" );
  svgDocument.appendChild( svgElem );
  svgElem.appendChild( svgDocument.importNode( svgGroupElem, true ) );

  //debug
  QgsMSDebugMsg( svgDocument.toString() );

  if ( !renderer.load( svgDocument.toByteArray() ) )
  {
    QgsMSDebugMsg( "Loading of svg content into QSvgRenderer failed" );
  }

  QImage brushImage( patternWidth, patternHeight, QImage::Format_ARGB32 );
  brushImage.fill( 0 ); //white background
  QPainter p( &brushImage );
  p.translate( x, y );
  renderer.render( &p, renderer.viewBoxF() );
  p.end();

  brush.setTextureImage( brushImage );

  return 0;
}

int QgsSLDRule::penFromStrokeElement( const QDomElement& strokeElement, QPen& pen ) const
{
  //default setting is a solid black line, width 1
  double lineWidth = 1.0;
  int lineColorRed = 0;
  int lineColorGreen = 0;
  int lineColorBlue = 0;
  Qt::PenStyle lineStyle = Qt::SolidLine;
  QVector<qreal> dashPattern;
  Qt::PenCapStyle lineCapStyle = Qt::FlatCap;
  Qt::PenJoinStyle lineJoinStyle = Qt::MiterJoin;
  double miterLimit = 2.0;
  int opacity = 255;
  double dashOffset = 0.0;

  //visit all the <CssParameter> elements and apply the settings step by step
  if ( !strokeElement.isNull() )
  {
    QDomNodeList cssNodes = strokeElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "CssParameter" );
    QString cssName;
    QDomElement currentElement;
    for ( int i = 0; i < cssNodes.size(); ++i )
    {
      currentElement = cssNodes.item( i ).toElement();
      if ( currentElement.isNull() )
      {
        continue;
      }
      QString elemText = currentElement.text();

      //switch depending on attribute 'name'
      cssName = currentElement.attribute/*NS*/( /*mSLDNamespace,*/ "name", "not found" );
      if ( cssName != "not found" )
      {
        if ( cssName == "stroke" )
        {
          //accept input in the form of #ff00ff
          if ( elemText.length() == 7 )
          {
            bool success;
            lineColorRed = elemText.mid( 1, 2 ).toInt( &success, 16 );
            if ( !success )
            {
              lineColorRed = 0;
            }
            lineColorGreen = elemText.mid( 3, 2 ).toInt( &success, 16 );
            if ( !success )
            {
              lineColorGreen = 0;
            }
            lineColorBlue = elemText.mid( 5, 2 ).toInt( &success, 16 );
            if ( !success )
            {
              lineColorBlue = 0;
            }
          }
        }
        else if ( cssName == "stroke-width" )
        {
          bool success;
          double width = elemText.toDouble( &success );
          if ( success )
          {
            lineWidth = width;
          }
        }
        else if ( cssName == "stroke-opacity" )
        {
          bool success;
          double op = elemText.toDouble( &success );
          if ( success )
          {
            if ( op > 1.0 )
            {
              opacity = 255;
            }
            else if ( op < 0.0 )
            {
              opacity = 0;
            }
            else
            {
              opacity = ( int )( 255 * op );
            }
          }
        }
        else if ( cssName == "stroke-linejoin" )
        {
          if ( elemText == "mitre" )
          {
            lineJoinStyle = Qt::MiterJoin;
          }
          else if ( elemText == "round" )
          {
            lineJoinStyle = Qt::RoundJoin;
          }
          else if ( elemText == "bevel" )
          {
            lineJoinStyle = Qt::BevelJoin;
          }
        }
        else if ( cssName == "stroke-linecap" )
        {
          if ( elemText == "butt" )
          {
            lineCapStyle = Qt::FlatCap;
          }
          else if ( elemText == "round" )
          {
            lineCapStyle = Qt::RoundCap;
          }
          else if ( elemText == "square" )
          {
            lineCapStyle = Qt::SquareCap;
          }
        }
        else if ( cssName == "stroke-dasharray" )
        {
          QStringList dashList = elemText.split( " ", QString::SkipEmptyParts );
          if ( dashList.size() % 2 != 0 )//if number of entries is odd, the list has to be doubled
          {
            dashList += dashList;
          }
          bool success; //flag if conversion to qreal was successful
          float f;
          for ( QStringList::const_iterator it = dashList.constBegin(); it != dashList.constEnd(); ++it )
          {
            f = ( *it ).toFloat( &success );
            if ( success )
            {
              dashPattern << f;
            }
          }
        }
        else if ( cssName == "stroke-dashoffset" )
        {
          dashOffset = elemText.toDouble();
        }
        else if ( cssName == "stroke-miterlimit" )
        {
          bool success;
          double ml = elemText.toDouble( &success );
          if ( success )
          {
            miterLimit = ml;
          }
        }
      }
    }
  }

  pen.setWidthF( lineWidth );
  pen.setColor( QColor( lineColorRed, lineColorGreen, lineColorBlue, opacity ) );
  if ( dashPattern.isEmpty() )
  {
    pen.setStyle( lineStyle );
  }
  else
  {
    pen.setDashPattern( dashPattern );
    pen.setDashOffset( dashOffset );
  }
  pen.setCapStyle( lineCapStyle );
  pen.setJoinStyle( lineJoinStyle );
  pen.setMiterLimit( miterLimit );
  return 0;
}

QSet<int> QgsSLDRule::attributeIndices() const
{
  QSet<int> resultSet = mFilterIndependentClassificationIndices;
  if ( mFilter )
  {
    QList<int> filterIndices = mFilter->attributeIndices();
    QList<int>::const_iterator filterIt = filterIndices.constBegin();
    for ( ; filterIt != filterIndices.constEnd(); ++filterIt )
    {
      resultSet.insert( *filterIt );
    }
  }
  return resultSet;
}
