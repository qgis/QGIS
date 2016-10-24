/***************************************************************************
    qgssymbologyconversion.cpp
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
#include "qgssymbologyconversion.h"

#include "qgslogger.h"

#include "qgsmarkersymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgscategorizedsymbolrenderer.h"



struct QgsOldSymbolMeta
{
  QString lowerValue;
  QString upperValue;
  QString label;
};


static QgsOldSymbolMeta readSymbolMeta( const QDomNode& synode )
{
  QgsOldSymbolMeta meta;

  QDomNode lvalnode = synode.namedItem( QStringLiteral( "lowervalue" ) );
  if ( ! lvalnode.isNull() )
  {
    QDomElement lvalelement = lvalnode.toElement();
    if ( lvalelement.attribute( QStringLiteral( "null" ) ).toInt() == 1 )
    {
      meta.lowerValue = QString::null;
    }
    else
    {
      meta.lowerValue = lvalelement.text();
    }
  }

  QDomNode uvalnode = synode.namedItem( QStringLiteral( "uppervalue" ) );
  if ( ! uvalnode.isNull() )
  {
    QDomElement uvalelement = uvalnode.toElement();
    meta.upperValue = uvalelement.text();
  }

  QDomNode labelnode = synode.namedItem( QStringLiteral( "label" ) );
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
  int red = celement.attribute( QStringLiteral( "red" ) ).toInt();
  int green = celement.attribute( QStringLiteral( "green" ) ).toInt();
  int blue = celement.attribute( QStringLiteral( "blue" ) ).toInt();
  return QColor( red, green, blue );
}

static double readOutlineWidth( const QDomNode& synode )
{
  QDomNode outlwnode = synode.namedItem( QStringLiteral( "outlinewidth" ) );
  QDomElement outlwelement = outlwnode.toElement();
  return outlwelement.text().toDouble();
}


static Qt::PenStyle readOutlineStyle( const QDomNode& synode )
{
  QDomNode outlstnode = synode.namedItem( QStringLiteral( "outlinestyle" ) );
  QDomElement outlstelement = outlstnode.toElement();
  return QgsSymbologyConversion::qString2PenStyle( outlstelement.text() );
}

static Qt::BrushStyle readBrushStyle( const QDomNode& synode )
{
  QDomNode fillpnode = synode.namedItem( QStringLiteral( "fillpattern" ) );
  QDomElement fillpelement = fillpnode.toElement();
  return QgsSymbologyConversion::qString2BrushStyle( fillpelement.text() );
}

static QString readMarkerSymbolName( const QDomNode& synode )
{
  QDomNode psymbnode = synode.namedItem( QStringLiteral( "pointsymbol" ) );
  if ( ! psymbnode.isNull() )
  {
    QDomElement psymbelement = psymbnode.toElement();
    return psymbelement.text();
  }
  return QStringLiteral( "hard:circle" );
}

static float readMarkerSymbolSize( const QDomNode& synode )
{
  QDomNode psizenode = synode.namedItem( QStringLiteral( "pointsize" ) );
  if ( ! psizenode.isNull() )
  {
    QDomElement psizeelement = psizenode.toElement();
    return psizeelement.text().toFloat();
  }
  return DEFAULT_POINT_SIZE;
}



static QgsSymbol* readOldSymbol( const QDomNode& synode, QgsWkbTypes::GeometryType geomType )
{
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
    {
      QgsMarkerSymbolLayer* sl = nullptr;
      double size = readMarkerSymbolSize( synode );
      double angle = 0; // rotation only from classification field
      QString symbolName = readMarkerSymbolName( synode );
      if ( symbolName.startsWith( QLatin1String( "hard:" ) ) )
      {
        // simple symbol marker
        QColor color = readSymbolColor( synode, true );
        QColor borderColor = readSymbolColor( synode, false );
        QgsSimpleMarkerSymbolLayerBase::Shape shape = QgsSimpleMarkerSymbolLayerBase::decodeShape( symbolName.mid( 5 ) );
        sl = new QgsSimpleMarkerSymbolLayer( shape, size, angle );
        sl->setColor( color );
        sl->setOutlineColor( borderColor );
      }
      else
      {
        // svg symbol marker
        QString name = symbolName.mid( 4 );
        sl = new QgsSvgMarkerSymbolLayer( name, size, angle );
      }
      QgsSymbolLayerList layers;
      layers.append( sl );
      return new QgsMarkerSymbol( layers );
    }

    case QgsWkbTypes::LineGeometry:
    {
      QColor color = readSymbolColor( synode, false );
      double width = readOutlineWidth( synode );
      Qt::PenStyle penStyle = readOutlineStyle( synode );
      QgsLineSymbolLayer* sl = new QgsSimpleLineSymbolLayer( color, width, penStyle );

      QgsSymbolLayerList layers;
      layers.append( sl );
      return new QgsLineSymbol( layers );
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      QColor color = readSymbolColor( synode, true );
      QColor borderColor = readSymbolColor( synode, false );
      Qt::BrushStyle brushStyle = readBrushStyle( synode );
      Qt::PenStyle borderStyle = readOutlineStyle( synode );
      double borderWidth = readOutlineWidth( synode );
      QgsFillSymbolLayer* sl = new QgsSimpleFillSymbolLayer( color, brushStyle, borderColor, borderStyle, borderWidth );

      QgsSymbolLayerList layers;
      layers.append( sl );
      return new QgsFillSymbol( layers );
    }

    default:
      return nullptr;
  }
}



static QgsFeatureRenderer* readOldSingleSymbolRenderer( const QDomNode& rnode, QgsWkbTypes::GeometryType geomType )
{
  QDomNode synode = rnode.namedItem( QStringLiteral( "symbol" ) );
  if ( synode.isNull() )
    return nullptr;

  QgsSymbol* sy2 = readOldSymbol( synode, geomType );
  QgsSingleSymbolRenderer* r = new QgsSingleSymbolRenderer( sy2 );
  return r;
}


static QgsFeatureRenderer* readOldGraduatedSymbolRenderer( const QDomNode& rnode, QgsWkbTypes::GeometryType geomType )
{
  QDomNode modeNode = rnode.namedItem( QStringLiteral( "mode" ) );
  QString modeValue = modeNode.toElement().text();
  QDomNode classnode = rnode.namedItem( QStringLiteral( "classificationfield" ) );
  QString classificationField = classnode.toElement().text();

  QgsGraduatedSymbolRenderer::Mode m = QgsGraduatedSymbolRenderer::Custom;
  if ( modeValue == QLatin1String( "Empty" ) )
  {
    m = QgsGraduatedSymbolRenderer::Custom;
  }
  else if ( modeValue == QLatin1String( "Quantile" ) )
  {
    m = QgsGraduatedSymbolRenderer::Quantile;
  }
  else //default
  {
    m = QgsGraduatedSymbolRenderer::EqualInterval;
  }

  // load ranges and symbols
  QgsRangeList ranges;
  QDomNode symbolnode = rnode.namedItem( QStringLiteral( "symbol" ) );
  while ( !symbolnode.isNull() )
  {
    QgsSymbol* symbol = readOldSymbol( symbolnode, geomType );
    if ( symbol )
    {
      QgsOldSymbolMeta meta = readSymbolMeta( symbolnode );
      double lowerValue = meta.lowerValue.toDouble();
      double upperValue = meta.upperValue.toDouble();
      QString label = meta.label;
      if ( label.isEmpty() )
        label = QStringLiteral( "%1 - %2" ).arg( lowerValue, -1, 'f', 3 ).arg( upperValue, -1, 'f', 3 );
      ranges.append( QgsRendererRange( lowerValue, upperValue, symbol, label ) );
    }

    symbolnode = symbolnode.nextSibling();
  }

  // create renderer
  QgsGraduatedSymbolRenderer* r = new QgsGraduatedSymbolRenderer( classificationField, ranges );
  r->setMode( m );
  return r;
}



static QgsFeatureRenderer* readOldUniqueValueRenderer( const QDomNode& rnode, QgsWkbTypes::GeometryType geomType )
{
  QDomNode classnode = rnode.namedItem( QStringLiteral( "classificationfield" ) );
  QString classificationField = classnode.toElement().text();

  // read categories and symbols
  QgsCategoryList cats;
  QDomNode symbolnode = rnode.namedItem( QStringLiteral( "symbol" ) );
  while ( !symbolnode.isNull() )
  {
    QgsSymbol* symbol = readOldSymbol( symbolnode, geomType );
    if ( symbol )
    {
      QgsOldSymbolMeta meta = readSymbolMeta( symbolnode );
      QVariant value = QVariant( meta.lowerValue );
      QString label = meta.label;
      if ( label.isEmpty() )
        label = value.toString();
      cats.append( QgsRendererCategory( value, symbol, label, true ) );
    }

    symbolnode = symbolnode.nextSibling();
  }

  QgsCategorizedSymbolRenderer* r = new QgsCategorizedSymbolRenderer( classificationField, cats );
  // source symbol and color ramp are not set (unknown)
  return r;
}




QgsFeatureRenderer* QgsSymbologyConversion::readOldRenderer( const QDomNode& layerNode, QgsWkbTypes::GeometryType geomType )
{
  QDomNode singlenode = layerNode.namedItem( QStringLiteral( "singlesymbol" ) );
  QDomNode graduatednode = layerNode.namedItem( QStringLiteral( "graduatedsymbol" ) );
  QDomNode continuousnode = layerNode.namedItem( QStringLiteral( "continuoussymbol" ) );
  QDomNode uniquevaluenode = layerNode.namedItem( QStringLiteral( "uniquevalue" ) );

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
    return nullptr;
  }
  else if ( !uniquevaluenode.isNull() )
  {
    return readOldUniqueValueRenderer( uniquevaluenode, geomType );
  }

  return nullptr;
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





QString QgsSymbologyConversion::penStyle2QString( Qt::PenStyle penstyle )
{
  if ( penstyle == Qt::NoPen )
  {
    return QStringLiteral( "NoPen" );
  }
  else if ( penstyle == Qt::SolidLine )
  {
    return QStringLiteral( "SolidLine" );
  }
  else if ( penstyle == Qt::DashLine )
  {
    return QStringLiteral( "DashLine" );
  }
  else if ( penstyle == Qt::DotLine )
  {
    return QStringLiteral( "DotLine" );
  }
  else if ( penstyle == Qt::DashDotLine )
  {
    return QStringLiteral( "DashDotLine" );
  }
  else if ( penstyle == Qt::DashDotDotLine )
  {
    return QStringLiteral( "DashDotDotLine" );
  }
  else if ( penstyle == Qt::MPenStyle )
  {
    return QStringLiteral( "MPenStyle" );
  }
  else                        //return a null string
  {
    return QString();
  }
}

Qt::PenStyle QgsSymbologyConversion::qString2PenStyle( const QString& penString )
{
  if ( penString == QLatin1String( "NoPen" ) )
  {
    return Qt::NoPen;
  }
  else if ( penString == QLatin1String( "SolidLine" ) )
  {
    return Qt::SolidLine;
  }
  else if ( penString == QLatin1String( "DashLine" ) )
  {
    return Qt::DashLine;
  }
  else if ( penString == QLatin1String( "DotLine" ) )
  {
    return Qt::DotLine;
  }
  else if ( penString == QLatin1String( "DashDotLine" ) )
  {
    return Qt::DashDotLine;
  }
  else if ( penString == QLatin1String( "DashDotDotLine" ) )
  {
    return Qt::DashDotDotLine;
  }
  else if ( penString == QLatin1String( "MPenStyle" ) )
  {
    return Qt::MPenStyle;
  }
  else
  {
    return Qt::NoPen;
  }
}

QString QgsSymbologyConversion::brushStyle2QString( Qt::BrushStyle brushstyle )
{
  if ( brushstyle == Qt::NoBrush )
  {
    return QStringLiteral( "NoBrush" );
  }
  else if ( brushstyle == Qt::SolidPattern )
  {
    return QStringLiteral( "SolidPattern" );
  }
  else if ( brushstyle == Qt::Dense1Pattern )
  {
    return QStringLiteral( "Dense1Pattern" );
  }
  else if ( brushstyle == Qt::Dense2Pattern )
  {
    return QStringLiteral( "Dense2Pattern" );
  }
  else if ( brushstyle == Qt::Dense3Pattern )
  {
    return QStringLiteral( "Dense3Pattern" );
  }
  else if ( brushstyle == Qt::Dense4Pattern )
  {
    return QStringLiteral( "Dense4Pattern" );
  }
  else if ( brushstyle == Qt::Dense5Pattern )
  {
    return QStringLiteral( "Dense5Pattern" );
  }
  else if ( brushstyle == Qt::Dense6Pattern )
  {
    return QStringLiteral( "Dense6Pattern" );
  }
  else if ( brushstyle == Qt::Dense7Pattern )
  {
    return QStringLiteral( "Dense7Pattern" );
  }
  else if ( brushstyle == Qt::HorPattern )
  {
    return QStringLiteral( "HorPattern" );
  }
  else if ( brushstyle == Qt::VerPattern )
  {
    return QStringLiteral( "VerPattern" );
  }
  else if ( brushstyle == Qt::CrossPattern )
  {
    return QStringLiteral( "CrossPattern" );
  }
  else if ( brushstyle == Qt::BDiagPattern )
  {
    return QStringLiteral( "BDiagPattern" );
  }
  else if ( brushstyle == Qt::FDiagPattern )
  {
    return QStringLiteral( "FDiagPattern" );
  }
  else if ( brushstyle == Qt::DiagCrossPattern )
  {
    return QStringLiteral( "DiagCrossPattern" );
  }
  else if ( brushstyle == Qt::TexturePattern )
  {
    return QStringLiteral( "TexturePattern" );
  }
  else                        //return a null string
  {
    QgsDebugMsg( "no matching pattern found" );
    return QStringLiteral( " " );
  }
}

Qt::BrushStyle QgsSymbologyConversion::qString2BrushStyle( const QString& brushString )
{
  if ( brushString == QLatin1String( "NoBrush" ) )
  {
    return Qt::NoBrush;
  }
  else if ( brushString == QLatin1String( "SolidPattern" ) )
  {
    return Qt::SolidPattern;
  }
  else if ( brushString == QLatin1String( "Dense1Pattern" ) )
  {
    return Qt::Dense1Pattern;
  }
  else if ( brushString == QLatin1String( "Dense2Pattern" ) )
  {
    return Qt::Dense2Pattern;
  }
  else if ( brushString == QLatin1String( "Dense3Pattern" ) )
  {
    return Qt::Dense3Pattern;
  }
  else if ( brushString == QLatin1String( "Dense4Pattern" ) )
  {
    return Qt::Dense4Pattern;
  }
  else if ( brushString == QLatin1String( "Dense5Pattern" ) )
  {
    return Qt::Dense5Pattern;
  }
  else if ( brushString == QLatin1String( "Dense6Pattern" ) )
  {
    return Qt::Dense6Pattern;
  }
  else if ( brushString == QLatin1String( "Dense7Pattern" ) )
  {
    return Qt::Dense7Pattern;
  }
  else if ( brushString == QLatin1String( "HorPattern" ) )
  {
    return Qt::HorPattern;
  }
  else if ( brushString == QLatin1String( "VerPattern" ) )
  {
    return Qt::VerPattern;
  }
  else if ( brushString == QLatin1String( "CrossPattern" ) )
  {
    return Qt::CrossPattern;
  }
  else if ( brushString == QLatin1String( "BDiagPattern" ) )
  {
    return Qt::BDiagPattern;
  }
  else if ( brushString == QLatin1String( "FDiagPattern" ) )
  {
    return Qt::FDiagPattern;
  }
  else if ( brushString == QLatin1String( "DiagCrossPattern" ) )
  {
    return Qt::DiagCrossPattern;
  }
  else if ( brushString == QLatin1String( "TexturePattern" ) )
  {
    return Qt::TexturePattern;
  }
  else                        //return a null string
  {
    QgsDebugMsg( QString( "Brush style \"%1\" not found" ).arg( brushString ) );
    return Qt::NoBrush;
  }
}
