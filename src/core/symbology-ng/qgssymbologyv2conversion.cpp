/***************************************************************************
    qgssymbologyv2conversion.cpp
    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssymbologyv2conversion.h"

#include "qgslogger.h"

#include "qgsmarkersymbollayerv2.h"
#include "qgslinesymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2.h"



struct QgsOldSymbolMeta
{
  QString lowerValue;
  QString upperValue;
  QString label;
};


static QgsOldSymbolMeta readSymbolMeta( const QDomNode& synode )
{
  QgsOldSymbolMeta meta;

  QDomNode lvalnode = synode.namedItem( "lowervalue" );
  if ( ! lvalnode.isNull() )
  {
    QDomElement lvalelement = lvalnode.toElement();
    if ( lvalelement.attribute( "null" ).toInt() == 1 )
    {
      meta.lowerValue = QString::null;
    }
    else
    {
      meta.lowerValue = lvalelement.text();
    }
  }

  QDomNode uvalnode = synode.namedItem( "uppervalue" );
  if ( ! uvalnode.isNull() )
  {
    QDomElement uvalelement = uvalnode.toElement();
    meta.upperValue = uvalelement.text();
  }

  QDomNode labelnode = synode.namedItem( "label" );
  if ( ! labelnode.isNull() )
  {
    QDomElement labelelement = labelnode.toElement();
    meta.label = labelelement.text();
  }

  return meta;
}


static QColor readSymbolColor( const QDomNode& synode, bool fillColor )
{
  QDomNode cnode = synode.namedItem( fillColor ? "fillcolor" : "outlinecolor" );
  QDomElement celement = cnode.toElement();
  int red = celement.attribute( "red" ).toInt();
  int green = celement.attribute( "green" ).toInt();
  int blue = celement.attribute( "blue" ).toInt();
  return QColor( red, green, blue );
}

static double readOutlineWidth( const QDomNode& synode )
{
  QDomNode outlwnode = synode.namedItem( "outlinewidth" );
  QDomElement outlwelement = outlwnode.toElement();
  return outlwelement.text().toDouble();
}


static Qt::PenStyle readOutlineStyle( const QDomNode& synode )
{
  QDomNode outlstnode = synode.namedItem( "outlinestyle" );
  QDomElement outlstelement = outlstnode.toElement();
  return QgsSymbologyV2Conversion::qString2PenStyle( outlstelement.text() );
}

static Qt::BrushStyle readBrushStyle( const QDomNode& synode )
{
  QDomNode fillpnode = synode.namedItem( "fillpattern" );
  QDomElement fillpelement = fillpnode.toElement();
  return QgsSymbologyV2Conversion::qString2BrushStyle( fillpelement.text() );
}

static QString readMarkerSymbolName( const QDomNode& synode )
{
  QDomNode psymbnode = synode.namedItem( "pointsymbol" );
  if ( ! psymbnode.isNull() )
  {
    QDomElement psymbelement = psymbnode.toElement();
    return psymbelement.text();
  }
  return QString( "hard:circle" );
}

static float readMarkerSymbolSize( const QDomNode& synode )
{
  QDomNode psizenode = synode.namedItem( "pointsize" );
  if ( ! psizenode.isNull() )
  {
    QDomElement psizeelement = psizenode.toElement();
    return psizeelement.text().toFloat();
  }
  return DEFAULT_POINT_SIZE;
}



static QgsSymbolV2* readOldSymbol( const QDomNode& synode, QGis::GeometryType geomType )
{
  switch ( geomType )
  {
    case QGis::Point:
    {
      QgsMarkerSymbolLayerV2* sl = NULL;
      double size = readMarkerSymbolSize( synode );
      double angle = 0; // rotation only from classification field
      QString symbolName = readMarkerSymbolName( synode );
      if ( symbolName.startsWith( "hard:" ) )
      {
        // simple symbol marker
        QColor color = readSymbolColor( synode, true );
        QColor borderColor = readSymbolColor( synode, false );
        QString name = symbolName.mid( 5 );
        sl = new QgsSimpleMarkerSymbolLayerV2( name, color, borderColor, size, angle );
      }
      else
      {
        // svg symbol marker
        QString name = symbolName.mid( 4 );
        sl = new QgsSvgMarkerSymbolLayerV2( name, size, angle );
      }
      QgsSymbolLayerV2List layers;
      layers.append( sl );
      return new QgsMarkerSymbolV2( layers );
    }

    case QGis::Line:
    {
      QColor color = readSymbolColor( synode, false );
      double width = readOutlineWidth( synode );
      Qt::PenStyle penStyle = readOutlineStyle( synode );
      QgsLineSymbolLayerV2* sl = new QgsSimpleLineSymbolLayerV2( color, width, penStyle );

      QgsSymbolLayerV2List layers;
      layers.append( sl );
      return new QgsLineSymbolV2( layers );
    }

    case QGis::Polygon:
    {
      QColor color = readSymbolColor( synode, true );
      QColor borderColor = readSymbolColor( synode, false );
      Qt::BrushStyle brushStyle = readBrushStyle( synode );
      Qt::PenStyle borderStyle = readOutlineStyle( synode );
      double borderWidth = readOutlineWidth( synode );
      QgsFillSymbolLayerV2* sl = new QgsSimpleFillSymbolLayerV2( color, brushStyle, borderColor, borderStyle, borderWidth );

      QgsSymbolLayerV2List layers;
      layers.append( sl );
      return new QgsFillSymbolV2( layers );
    }

    default:
      return NULL;
  }
}



static QgsFeatureRendererV2* readOldSingleSymbolRenderer( const QDomNode& rnode, QGis::GeometryType geomType )
{
  QDomNode synode = rnode.namedItem( "symbol" );
  if ( synode.isNull() )
    return 0;

  QgsSymbolV2* sy2 = readOldSymbol( synode, geomType );
  QgsSingleSymbolRendererV2* r = new QgsSingleSymbolRendererV2( sy2 );
  return r;
}


static QgsFeatureRendererV2* readOldGraduatedSymbolRenderer( const QDomNode& rnode, QGis::GeometryType geomType )
{
  QDomNode modeNode = rnode.namedItem( "mode" );
  QString modeValue = modeNode.toElement().text();
  QDomNode classnode = rnode.namedItem( "classificationfield" );
  QString classificationField = classnode.toElement().text();

  QgsGraduatedSymbolRendererV2::Mode m = QgsGraduatedSymbolRendererV2::Custom;
  if ( modeValue == "Empty" )
  {
    m = QgsGraduatedSymbolRendererV2::Custom;
  }
  else if ( modeValue == "Quantile" )
  {
    m = QgsGraduatedSymbolRendererV2::Quantile;
  }
  else //default
  {
    m = QgsGraduatedSymbolRendererV2::EqualInterval;
  }

  // load ranges and symbols
  QgsRangeList ranges;
  QDomNode symbolnode = rnode.namedItem( "symbol" );
  while ( !symbolnode.isNull() )
  {
    QgsSymbolV2* symbolv2 = readOldSymbol( symbolnode, geomType );
    if ( symbolv2 )
    {
      QgsOldSymbolMeta meta = readSymbolMeta( symbolnode );
      double lowerValue = meta.lowerValue.toDouble();
      double upperValue = meta.upperValue.toDouble();
      QString label = meta.label;
      if ( label.isEmpty() )
        label = QString( "%1 - %2" ).arg( lowerValue, -1, 'f', 3 ).arg( upperValue, -1, 'f', 3 );
      ranges.append( QgsRendererRangeV2( lowerValue, upperValue, symbolv2, label ) );
    }

    symbolnode = symbolnode.nextSibling();
  }

  // create renderer
  QgsGraduatedSymbolRendererV2* r = new QgsGraduatedSymbolRendererV2( classificationField, ranges );
  r->setMode( m );
  return r;
}



static QgsFeatureRendererV2* readOldUniqueValueRenderer( const QDomNode& rnode, QGis::GeometryType geomType )
{
  QDomNode classnode = rnode.namedItem( "classificationfield" );
  QString classificationField = classnode.toElement().text();

  // read categories and symbols
  QgsCategoryList cats;
  QDomNode symbolnode = rnode.namedItem( "symbol" );
  while ( !symbolnode.isNull() )
  {
    QgsSymbolV2* symbolv2 = readOldSymbol( symbolnode, geomType );
    if ( symbolv2 )
    {
      QgsOldSymbolMeta meta = readSymbolMeta( symbolnode );
      QVariant value = QVariant( meta.lowerValue );
      QString label = meta.label;
      if ( label.isEmpty() )
        label = value.toString();
      cats.append( QgsRendererCategoryV2( value, symbolv2, label, true ) );
    }

    symbolnode = symbolnode.nextSibling();
  }

  QgsCategorizedSymbolRendererV2* r = new QgsCategorizedSymbolRendererV2( classificationField, cats );
  // source symbol and color ramp are not set (unknown)
  return r;
}




QgsFeatureRendererV2* QgsSymbologyV2Conversion::readOldRenderer( const QDomNode& layerNode, QGis::GeometryType geomType )
{
  QDomNode singlenode = layerNode.namedItem( "singlesymbol" );
  QDomNode graduatednode = layerNode.namedItem( "graduatedsymbol" );
  QDomNode continuousnode = layerNode.namedItem( "continuoussymbol" );
  QDomNode uniquevaluenode = layerNode.namedItem( "uniquevalue" );

  if ( !singlenode.isNull() )
  {
    return readOldSingleSymbolRenderer( singlenode, geomType );
  }
  else if ( !graduatednode.isNull() )
  {
    return readOldGraduatedSymbolRenderer( graduatednode, geomType );
  }
  else if ( !continuousnode.isNull() )
  {
    return 0;
  }
  else if ( !uniquevaluenode.isNull() )
  {
    return readOldUniqueValueRenderer( uniquevaluenode, geomType );
  }

  return 0;
}


/*
UNSUPPORTED RENDERER: continuous color

  QDomNode classnode = rnode.namedItem( "classificationfield" );
  QDomNode polyoutlinenode = rnode.namedItem( "polygonoutline" );
  QString polyoutline = polyoutlinenode.toElement().text();
  if ( polyoutline == "0" )
    drawPolygonOutline = false;
  else if ( polyoutline == "1" )
    drawPolygonOutline = true;
  QDomNode lowernode = rnode.namedItem( "lowestsymbol" );
  lowSymbol = readOldSymbol( lowernode.namedItem( "symbol" ), geomType );
  QDomNode uppernode = rnode.namedItem( "highestsymbol" );
  highSymbol = readOldSymbol( uppernode.namedItem( "symbol" ), geomType );

UNSUPPORTED SYMBOL PROPERTY: point size units

  QDomNode psizeunitnodes = synode.namedItem( "pointsizeunits" );
  if ( ! psizeunitnodes.isNull() )
  {
    QDomElement psizeunitelement = psizeunitnodes.toElement();
    QgsDebugMsg( QString( "psizeunitelement:%1" ).arg( psizeunitelement.text() ) );
    setPointSizeUnits( psizeunitelement.text().compare( "mapunits", Qt::CaseInsensitive ) == 0 );
  }

UNSUPPORTED SYMBOL PROPERTY: data-defined rotation / scale / symbol name

  rotationClassificationFieldName = synode.namedItem( "rotationclassificationfieldname" ).toElement().text();
  scaleClassificationFieldName = synode.namedItem( "scaleclassificationfield" ).toElement().text();
  symbolFieldName = synode.namedItem( "symbolfieldname" ).toElement().text();

UNSUPPORTED SYMBOL PROPERTY: texture

  QDomNode texturepathnode = synode.namedItem( "texturepath" );
  QDomElement texturepathelement = texturepathnode.toElement();
  setCustomTexture( QgsProject::instance()->readPath( texturepathelement.text() ) );
*/





QString QgsSymbologyV2Conversion::penStyle2QString( Qt::PenStyle penstyle )
{
  if ( penstyle == Qt::NoPen )
  {
    return "NoPen";
  }
  else if ( penstyle == Qt::SolidLine )
  {
    return "SolidLine";
  }
  else if ( penstyle == Qt::DashLine )
  {
    return "DashLine";
  }
  else if ( penstyle == Qt::DotLine )
  {
    return "DotLine";
  }
  else if ( penstyle == Qt::DashDotLine )
  {
    return "DashDotLine";
  }
  else if ( penstyle == Qt::DashDotDotLine )
  {
    return "DashDotDotLine";
  }
  else if ( penstyle == Qt::MPenStyle )
  {
    return "MPenStyle";
  }
  else                        //return a null string
  {
    return QString();
  }
}

Qt::PenStyle QgsSymbologyV2Conversion::qString2PenStyle( QString penString )
{
  if ( penString == "NoPen" )
  {
    return Qt::NoPen;
  }
  else if ( penString == "SolidLine" )
  {
    return Qt::SolidLine;
  }
  else if ( penString == "DashLine" )
  {
    return Qt::DashLine;
  }
  else if ( penString == "DotLine" )
  {
    return Qt::DotLine;
  }
  else if ( penString == "DashDotLine" )
  {
    return Qt::DashDotLine;
  }
  else if ( penString == "DashDotDotLine" )
  {
    return Qt::DashDotDotLine;
  }
  else if ( penString == "MPenStyle" )
  {
    return Qt::MPenStyle;
  }
  else
  {
    return Qt::NoPen;
  }
}

QString QgsSymbologyV2Conversion::brushStyle2QString( Qt::BrushStyle brushstyle )
{
  if ( brushstyle == Qt::NoBrush )
  {
    return "NoBrush";
  }
  else if ( brushstyle == Qt::SolidPattern )
  {
    return "SolidPattern";
  }
  else if ( brushstyle == Qt::Dense1Pattern )
  {
    return "Dense1Pattern";
  }
  else if ( brushstyle == Qt::Dense2Pattern )
  {
    return "Dense2Pattern";
  }
  else if ( brushstyle == Qt::Dense3Pattern )
  {
    return "Dense3Pattern";
  }
  else if ( brushstyle == Qt::Dense4Pattern )
  {
    return "Dense4Pattern";
  }
  else if ( brushstyle == Qt::Dense5Pattern )
  {
    return "Dense5Pattern";
  }
  else if ( brushstyle == Qt::Dense6Pattern )
  {
    return "Dense6Pattern";
  }
  else if ( brushstyle == Qt::Dense7Pattern )
  {
    return "Dense7Pattern";
  }
  else if ( brushstyle == Qt::HorPattern )
  {
    return "HorPattern";
  }
  else if ( brushstyle == Qt::VerPattern )
  {
    return "VerPattern";
  }
  else if ( brushstyle == Qt::CrossPattern )
  {
    return "CrossPattern";
  }
  else if ( brushstyle == Qt::BDiagPattern )
  {
    return "BDiagPattern";
  }
  else if ( brushstyle == Qt::FDiagPattern )
  {
    return "FDiagPattern";
  }
  else if ( brushstyle == Qt::DiagCrossPattern )
  {
    return "DiagCrossPattern";
  }
  else if ( brushstyle == Qt::TexturePattern )
  {
    return "TexturePattern";
  }
  else                        //return a null string
  {
    QgsDebugMsg( "no matching pattern found" );
    return " ";
  }
}

Qt::BrushStyle QgsSymbologyV2Conversion::qString2BrushStyle( QString brushString )
{
  if ( brushString == "NoBrush" )
  {
    return Qt::NoBrush;
  }
  else if ( brushString == "SolidPattern" )
  {
    return Qt::SolidPattern;
  }
  else if ( brushString == "Dense1Pattern" )
  {
    return Qt::Dense1Pattern;
  }
  else if ( brushString == "Dense2Pattern" )
  {
    return Qt::Dense2Pattern;
  }
  else if ( brushString == "Dense3Pattern" )
  {
    return Qt::Dense3Pattern;
  }
  else if ( brushString == "Dense4Pattern" )
  {
    return Qt::Dense4Pattern;
  }
  else if ( brushString == "Dense5Pattern" )
  {
    return Qt::Dense5Pattern;
  }
  else if ( brushString == "Dense6Pattern" )
  {
    return Qt::Dense6Pattern;
  }
  else if ( brushString == "Dense7Pattern" )
  {
    return Qt::Dense7Pattern;
  }
  else if ( brushString == "HorPattern" )
  {
    return Qt::HorPattern;
  }
  else if ( brushString == "VerPattern" )
  {
    return Qt::VerPattern;
  }
  else if ( brushString == "CrossPattern" )
  {
    return Qt::CrossPattern;
  }
  else if ( brushString == "BDiagPattern" )
  {
    return Qt::BDiagPattern;
  }
  else if ( brushString == "FDiagPattern" )
  {
    return Qt::FDiagPattern;
  }
  else if ( brushString == "DiagCrossPattern" )
  {
    return Qt::DiagCrossPattern;
  }
  else if ( brushString == "TexturePattern" )
  {
    return Qt::TexturePattern;
  }
  else                        //return a null string
  {
    QgsDebugMsg( QString( "Brush style \"%1\" not found" ).arg( brushString ) );
    return Qt::NoBrush;
  }
}
