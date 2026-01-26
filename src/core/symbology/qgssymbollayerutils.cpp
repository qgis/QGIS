/***************************************************************************
 qgssymbollayerutils.cpp
 ---------------------
 begin                : November 2009
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

#include "qgssymbollayerutils.h"

#include "qgsapplication.h"
#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"
#include "qgscolorutils.h"
#include "qgscurvepolygon.h"
#include "qgsexpression.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionnode.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmasksymbollayer.h"
#include "qgsogcutils.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgspathresolver.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgsrenderer.h"
#include "qgssldexportcontext.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerreference.h"
#include "qgssymbollayerregistry.h"
#include "qgsunittypes.h"
#include "qgsxmlutils.h"

#include <QColor>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFont>
#include <QIcon>
#include <QMimeData>
#include <QPainter>
#include <QPicture>
#include <QRegularExpression>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>
#include <qmath.h>

#define POINTS_TO_MM 2.83464567

QString QgsSymbolLayerUtils::encodeColor( const QColor &color )
{
  return u"%1,%2,%3,%4"_s.arg( color.red() ).arg( color.green() ).arg( color.blue() ).arg( color.alpha() );
}

QColor QgsSymbolLayerUtils::decodeColor( const QString &str )
{
  const QStringList lst = str.split( ',' );
  if ( lst.count() < 3 )
  {
    return QColor( str );
  }
  int red, green, blue, alpha;
  red = lst[0].toInt();
  green = lst[1].toInt();
  blue = lst[2].toInt();
  alpha = 255;
  if ( lst.count() > 3 )
  {
    alpha = lst[3].toInt();
  }
  return QColor( red, green, blue, alpha );
}

QString QgsSymbolLayerUtils::encodeSldAlpha( int alpha )
{
  return QString::number( alpha / 255.0, 'g', 2 );
}

int QgsSymbolLayerUtils::decodeSldAlpha( const QString &str )
{
  bool ok;
  double alpha = str.toDouble( &ok );
  if ( !ok || alpha > 1 )
    alpha = 255;
  else if ( alpha < 0 )
    alpha = 0;
  return alpha * 255;
}

QString QgsSymbolLayerUtils::encodeSldFontStyle( QFont::Style style )
{
  switch ( style )
  {
    case QFont::StyleNormal:
      return u"normal"_s;
    case QFont::StyleItalic:
      return u"italic"_s;
    case QFont::StyleOblique:
      return u"oblique"_s;
    default:
      return QString();
  }
}

QFont::Style QgsSymbolLayerUtils::decodeSldFontStyle( const QString &str )
{
  if ( str == "normal"_L1 ) return QFont::StyleNormal;
  if ( str == "italic"_L1 ) return QFont::StyleItalic;
  if ( str == "oblique"_L1 ) return QFont::StyleOblique;
  return QFont::StyleNormal;
}

QString QgsSymbolLayerUtils::encodeSldFontWeight( int weight )
{
  if ( weight == 50 ) return u"normal"_s;
  if ( weight == 75 ) return u"bold"_s;

  // QFont::Weight is between 0 and 99
  // CSS font-weight is between 100 and 900
  if ( weight < 0 ) return u"100"_s;
  if ( weight > 99 ) return u"900"_s;
  return QString::number( weight * 800 / 99 + 100 );
}

int QgsSymbolLayerUtils::decodeSldFontWeight( const QString &str )
{
  bool ok;
  const int weight = str.toInt( &ok );
  if ( !ok )
    return static_cast< int >( QFont::Normal );

  // CSS font-weight is between 100 and 900
  // QFont::Weight is between 0 and 99
  if ( weight > 900 ) return 99;
  if ( weight < 100 ) return 0;
  return ( weight - 100 ) * 99 / 800;
}

QString QgsSymbolLayerUtils::encodePenStyle( Qt::PenStyle style )
{
  switch ( style )
  {
    case Qt::NoPen:
      return u"no"_s;
    case Qt::SolidLine:
      return u"solid"_s;
    case Qt::DashLine:
      return u"dash"_s;
    case Qt::DotLine:
      return u"dot"_s;
    case Qt::DashDotLine:
      return u"dash dot"_s;
    case Qt::DashDotDotLine:
      return u"dash dot dot"_s;
    default:
      return u"???"_s;
  }
}

Qt::PenStyle QgsSymbolLayerUtils::decodePenStyle( const QString &str )
{
  if ( str == "no"_L1 ) return Qt::NoPen;
  if ( str == "solid"_L1 ) return Qt::SolidLine;
  if ( str == "dash"_L1 ) return Qt::DashLine;
  if ( str == "dot"_L1 ) return Qt::DotLine;
  if ( str == "dash dot"_L1 ) return Qt::DashDotLine;
  if ( str == "dash dot dot"_L1 ) return Qt::DashDotDotLine;
  return Qt::SolidLine;
}

QString QgsSymbolLayerUtils::encodePenJoinStyle( Qt::PenJoinStyle style )
{
  switch ( style )
  {
    case Qt::BevelJoin:
      return u"bevel"_s;
    case Qt::MiterJoin:
      return u"miter"_s;
    case Qt::RoundJoin:
      return u"round"_s;
    default:
      return u"???"_s;
  }
}

Qt::PenJoinStyle QgsSymbolLayerUtils::decodePenJoinStyle( const QString &str )
{
  const QString cleaned = str.toLower().trimmed();
  if ( cleaned == "bevel"_L1 )
    return Qt::BevelJoin;
  if ( cleaned == "miter"_L1 )
    return Qt::MiterJoin;
  if ( cleaned == "round"_L1 )
    return Qt::RoundJoin;
  return Qt::BevelJoin;
}

QString QgsSymbolLayerUtils::encodeSldLineJoinStyle( Qt::PenJoinStyle style )
{
  switch ( style )
  {
    case Qt::BevelJoin:
      return u"bevel"_s;
    case Qt::MiterJoin:
      return u"mitre"_s;  //#spellok
    case Qt::RoundJoin:
      return u"round"_s;
    default:
      return QString();
  }
}

Qt::PenJoinStyle QgsSymbolLayerUtils::decodeSldLineJoinStyle( const QString &str )
{
  if ( str == "bevel"_L1 ) return Qt::BevelJoin;
  if ( str == "mitre"_L1 ) return Qt::MiterJoin;  //#spellok
  if ( str == "round"_L1 ) return Qt::RoundJoin;
  return Qt::BevelJoin;
}

QString QgsSymbolLayerUtils::encodePenCapStyle( Qt::PenCapStyle style )
{
  switch ( style )
  {
    case Qt::SquareCap:
      return u"square"_s;
    case Qt::FlatCap:
      return u"flat"_s;
    case Qt::RoundCap:
      return u"round"_s;
    default:
      return u"???"_s;
  }
}

Qt::PenCapStyle QgsSymbolLayerUtils::decodePenCapStyle( const QString &str )
{
  if ( str == "square"_L1 ) return Qt::SquareCap;
  if ( str == "flat"_L1 ) return Qt::FlatCap;
  if ( str == "round"_L1 ) return Qt::RoundCap;
  return Qt::SquareCap;
}

QString QgsSymbolLayerUtils::encodeSldLineCapStyle( Qt::PenCapStyle style )
{
  switch ( style )
  {
    case Qt::SquareCap:
      return u"square"_s;
    case Qt::FlatCap:
      return u"butt"_s;
    case Qt::RoundCap:
      return u"round"_s;
    default:
      return QString();
  }
}

Qt::PenCapStyle QgsSymbolLayerUtils::decodeSldLineCapStyle( const QString &str )
{
  if ( str == "square"_L1 ) return Qt::SquareCap;
  if ( str == "butt"_L1 ) return Qt::FlatCap;
  if ( str == "round"_L1 ) return Qt::RoundCap;
  return Qt::SquareCap;
}

QString QgsSymbolLayerUtils::encodeBrushStyle( Qt::BrushStyle style )
{
  switch ( style )
  {
    case Qt::SolidPattern :
      return u"solid"_s;
    case Qt::HorPattern :
      return u"horizontal"_s;
    case Qt::VerPattern :
      return u"vertical"_s;
    case Qt::CrossPattern :
      return u"cross"_s;
    case Qt::BDiagPattern :
      return u"b_diagonal"_s;
    case Qt::FDiagPattern :
      return  u"f_diagonal"_s;
    case Qt::DiagCrossPattern :
      return u"diagonal_x"_s;
    case Qt::Dense1Pattern  :
      return u"dense1"_s;
    case Qt::Dense2Pattern  :
      return u"dense2"_s;
    case Qt::Dense3Pattern  :
      return u"dense3"_s;
    case Qt::Dense4Pattern  :
      return u"dense4"_s;
    case Qt::Dense5Pattern  :
      return u"dense5"_s;
    case Qt::Dense6Pattern  :
      return u"dense6"_s;
    case Qt::Dense7Pattern  :
      return u"dense7"_s;
    case Qt::NoBrush :
      return u"no"_s;
    default:
      return u"???"_s;
  }
}

Qt::BrushStyle QgsSymbolLayerUtils::decodeBrushStyle( const QString &str )
{
  if ( str == "solid"_L1 ) return Qt::SolidPattern;
  if ( str == "horizontal"_L1 ) return Qt::HorPattern;
  if ( str == "vertical"_L1 ) return Qt::VerPattern;
  if ( str == "cross"_L1 ) return Qt::CrossPattern;
  if ( str == "b_diagonal"_L1 ) return Qt::BDiagPattern;
  if ( str == "f_diagonal"_L1 ) return Qt::FDiagPattern;
  if ( str == "diagonal_x"_L1 ) return Qt::DiagCrossPattern;
  if ( str == "dense1"_L1 ) return Qt::Dense1Pattern;
  if ( str == "dense2"_L1 ) return Qt::Dense2Pattern;
  if ( str == "dense3"_L1 ) return Qt::Dense3Pattern;
  if ( str == "dense4"_L1 ) return Qt::Dense4Pattern;
  if ( str == "dense5"_L1 ) return Qt::Dense5Pattern;
  if ( str == "dense6"_L1 ) return Qt::Dense6Pattern;
  if ( str == "dense7"_L1 ) return Qt::Dense7Pattern;
  if ( str == "no"_L1 ) return Qt::NoBrush;
  return Qt::SolidPattern;
}

QString QgsSymbolLayerUtils::encodeSldBrushStyle( Qt::BrushStyle style )
{
  switch ( style )
  {
    case Qt::CrossPattern:
      return u"cross"_s;
    case Qt::DiagCrossPattern:
      return u"x"_s;

    /* The following names are taken from the presentation "GeoServer
     * Cartographic Rendering" by Andrea Aime at the FOSS4G 2010.
     * (see http://2010.foss4g.org/presentations/3588.pdf)
     */
    case Qt::HorPattern:
      return u"horline"_s;
    case Qt::VerPattern:
      return u"line"_s;
    case Qt::BDiagPattern:
      return u"slash"_s;
    case Qt::FDiagPattern:
      return u"backslash"_s;

    /* define the other names following the same pattern used above */
    case Qt::Dense1Pattern:
    case Qt::Dense2Pattern:
    case Qt::Dense3Pattern:
    case Qt::Dense4Pattern:
    case Qt::Dense5Pattern:
    case Qt::Dense6Pattern:
    case Qt::Dense7Pattern:
      return u"brush://%1"_s.arg( encodeBrushStyle( style ) );

    default:
      return QString();
  }
}

Qt::BrushStyle QgsSymbolLayerUtils::decodeSldBrushStyle( const QString &str )
{
  if ( str == "horline"_L1 ) return Qt::HorPattern;
  if ( str == "line"_L1 ) return Qt::VerPattern;
  if ( str == "cross"_L1 ) return Qt::CrossPattern;
  if ( str == "slash"_L1 ) return Qt::BDiagPattern;
  if ( str == "backshash"_L1 ) return Qt::FDiagPattern;
  if ( str == "x"_L1 ) return Qt::DiagCrossPattern;

  if ( str.startsWith( "brush://"_L1 ) )
    return decodeBrushStyle( str.mid( 8 ) );

  return Qt::NoBrush;
}

Qgis::EndCapStyle QgsSymbolLayerUtils::penCapStyleToEndCapStyle( Qt::PenCapStyle style )
{
  switch ( style )
  {
    case Qt::FlatCap:
      return Qgis::EndCapStyle::Flat;
    case Qt::SquareCap:
      return Qgis::EndCapStyle::Square;
    case Qt::RoundCap:
      return Qgis::EndCapStyle::Round;
    case Qt::MPenCapStyle:
      // undocumented?
      break;
  }

  return Qgis::EndCapStyle::Round;
}

Qgis::JoinStyle QgsSymbolLayerUtils::penJoinStyleToJoinStyle( Qt::PenJoinStyle style )
{
  switch ( style )
  {
    case Qt::MiterJoin:
    case Qt::SvgMiterJoin:
      return Qgis::JoinStyle::Miter;
    case Qt::BevelJoin:
      return Qgis::JoinStyle::Bevel;
    case Qt::RoundJoin:
      return Qgis::JoinStyle::Round;
    case Qt::MPenJoinStyle:
      // undocumented?
      break;
  }
  return Qgis::JoinStyle::Round;
}

bool QgsSymbolLayerUtils::hasSldSymbolizer( const QDomElement &element )
{
  const QDomNodeList children = element.childNodes();
  for ( int i = 0; i < children.size(); ++i )
  {
    const QDomElement childElement = children.at( i ).toElement();
    if ( childElement.tagName() == "se:LineSymbolizer"_L1
         || childElement.tagName() == "se:PointSymbolizer"_L1
         || childElement.tagName() == "se:PolygonSymbolizer"_L1 )
      return true;
  }
  return false;
}

Qgis::SymbolCoordinateReference QgsSymbolLayerUtils::decodeCoordinateReference( const QString &string, bool *ok )
{
  const QString compareString = string.trimmed();
  if ( ok )
    *ok = true;

  if ( compareString.compare( "feature"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SymbolCoordinateReference::Feature;
  else if ( compareString.compare( "viewport"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SymbolCoordinateReference::Viewport;

  if ( ok )
    *ok = false;
  return  Qgis::SymbolCoordinateReference::Feature;
}

QString QgsSymbolLayerUtils::encodeCoordinateReference( Qgis::SymbolCoordinateReference coordinateReference )
{
  switch ( coordinateReference )
  {
    case Qgis::SymbolCoordinateReference::Feature:
      return u"feature"_s;
    case Qgis::SymbolCoordinateReference::Viewport:
      return u"viewport"_s;
  }
  return QString(); // no warnings
}

QgsArrowSymbolLayer::HeadType QgsSymbolLayerUtils::decodeArrowHeadType( const QVariant &value, bool *ok )
{
  if ( ok )
    *ok = true;

  bool intOk = false;
  const QString s = value.toString().toLower().trimmed();
  if ( s == "single"_L1 )
    return QgsArrowSymbolLayer::HeadSingle;
  else if ( s == "reversed"_L1 )
    return QgsArrowSymbolLayer::HeadReversed;
  else if ( s == "double"_L1 )
    return QgsArrowSymbolLayer::HeadDouble;
  else if ( value.toInt() == 1 )
    return QgsArrowSymbolLayer::HeadReversed;
  else if ( value.toInt() == 2 )
    return QgsArrowSymbolLayer::HeadDouble;
  else if ( value.toInt( &intOk ) == 0 && intOk )
    return QgsArrowSymbolLayer::HeadSingle;

  if ( ok )
    *ok = false;
  return QgsArrowSymbolLayer::HeadSingle;
}

QgsArrowSymbolLayer::ArrowType QgsSymbolLayerUtils::decodeArrowType( const QVariant &value, bool *ok )
{
  if ( ok )
    *ok = true;

  bool intOk = false;
  const QString s = value.toString().toLower().trimmed();
  if ( s == "plain"_L1 )
    return QgsArrowSymbolLayer::ArrowPlain;
  else if ( s == "lefthalf"_L1 )
    return QgsArrowSymbolLayer::ArrowLeftHalf;
  else if ( s == "righthalf"_L1 )
    return QgsArrowSymbolLayer::ArrowRightHalf;
  else if ( value.toInt() == 1 )
    return QgsArrowSymbolLayer::ArrowLeftHalf;
  else if ( value.toInt() == 2 )
    return QgsArrowSymbolLayer::ArrowRightHalf;
  else if ( value.toInt( &intOk ) == 0 && intOk )
    return QgsArrowSymbolLayer::ArrowPlain;

  if ( ok )
    *ok = false;
  return QgsArrowSymbolLayer::ArrowPlain;
}

Qgis::MarkerClipMode QgsSymbolLayerUtils::decodeMarkerClipMode( const QString &string, bool *ok )
{
  const QString compareString = string.trimmed();
  if ( ok )
    *ok = true;

  if ( compareString.compare( "no"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::MarkerClipMode::NoClipping;
  else if ( compareString.compare( "shape"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::MarkerClipMode::Shape;
  else if ( compareString.compare( "centroid_within"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::MarkerClipMode::CentroidWithin;
  else if ( compareString.compare( "completely_within"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::MarkerClipMode::CompletelyWithin;

  if ( ok )
    *ok = false;
  return  Qgis::MarkerClipMode::Shape;
}

QString QgsSymbolLayerUtils::encodeMarkerClipMode( Qgis::MarkerClipMode mode )
{
  switch ( mode )
  {
    case Qgis::MarkerClipMode::NoClipping:
      return u"no"_s;
    case Qgis::MarkerClipMode::Shape:
      return u"shape"_s;
    case Qgis::MarkerClipMode::CentroidWithin:
      return u"centroid_within"_s;
    case Qgis::MarkerClipMode::CompletelyWithin:
      return u"completely_within"_s;
  }
  return QString(); // no warnings
}

Qgis::LineClipMode QgsSymbolLayerUtils::decodeLineClipMode( const QString &string, bool *ok )
{
  const QString compareString = string.trimmed();
  if ( ok )
    *ok = true;

  if ( compareString.compare( "no"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::LineClipMode::NoClipping;
  else if ( compareString.compare( "during_render"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::LineClipMode::ClipPainterOnly;
  else if ( compareString.compare( "before_render"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::LineClipMode::ClipToIntersection;

  if ( ok )
    *ok = false;
  return  Qgis::LineClipMode::ClipPainterOnly;
}

QString QgsSymbolLayerUtils::encodeLineClipMode( Qgis::LineClipMode mode )
{
  switch ( mode )
  {
    case Qgis::LineClipMode::NoClipping:
      return u"no"_s;
    case Qgis::LineClipMode::ClipPainterOnly:
      return u"during_render"_s;
    case Qgis::LineClipMode::ClipToIntersection:
      return u"before_render"_s;
  }
  return QString(); // no warnings
}

QString QgsSymbolLayerUtils::encodePoint( QPointF point )
{
  return u"%1,%2"_s.arg( qgsDoubleToString( point.x() ), qgsDoubleToString( point.y() ) );
}

QPointF QgsSymbolLayerUtils::decodePoint( const QString &str )
{
  QStringList lst = str.split( ',' );
  if ( lst.count() != 2 )
    return QPointF( 0, 0 );
  return QPointF( lst[0].toDouble(), lst[1].toDouble() );
}

QPointF QgsSymbolLayerUtils::toPoint( const QVariant &value, bool *ok )
{
  if ( ok )
    *ok = false;

  if ( QgsVariantUtils::isNull( value ) )
    return QPoint();

  if ( value.userType() == QMetaType::Type::QVariantList )
  {
    const QVariantList list = value.toList();
    if ( list.size() != 2 )
    {
      return QPointF();
    }
    bool convertOk = false;
    const double x = list.at( 0 ).toDouble( &convertOk );
    if ( convertOk )
    {
      const double y = list.at( 1 ).toDouble( &convertOk );
      if ( convertOk )
      {
        if ( ok )
          *ok = true;
        return QPointF( x, y );
      }
    }
    return QPointF();
  }
  else
  {
    // can't use decodePoint here -- has no OK handling
    const QStringList list = value.toString().trimmed().split( ',' );
    if ( list.count() != 2 )
      return QPointF();
    bool convertOk = false;
    const double x = list.at( 0 ).toDouble( &convertOk );
    if ( convertOk )
    {
      const double y = list.at( 1 ).toDouble( &convertOk );
      if ( convertOk )
      {
        if ( ok )
          *ok = true;
        return QPointF( x, y );
      }
    }
    return QPointF();
  }
}

QString QgsSymbolLayerUtils::encodeSize( QSizeF size )
{
  return u"%1,%2"_s.arg( qgsDoubleToString( size.width() ), qgsDoubleToString( size.height() ) );
}

QSizeF QgsSymbolLayerUtils::decodeSize( const QString &string )
{
  QStringList lst = string.split( ',' );
  if ( lst.count() != 2 )
    return QSizeF( 0, 0 );
  return QSizeF( lst[0].toDouble(), lst[1].toDouble() );
}

QSizeF QgsSymbolLayerUtils::toSize( const QVariant &value, bool *ok )
{
  if ( ok )
    *ok = false;

  if ( QgsVariantUtils::isNull( value ) )
    return QSizeF();

  if ( value.userType() == QMetaType::Type::QVariantList )
  {
    const QVariantList list = value.toList();
    if ( list.size() != 2 )
    {
      return QSizeF();
    }
    bool convertOk = false;
    const double x = list.at( 0 ).toDouble( &convertOk );
    if ( convertOk )
    {
      const double y = list.at( 1 ).toDouble( &convertOk );
      if ( convertOk )
      {
        if ( ok )
          *ok = true;
        return QSizeF( x, y );
      }
    }
    return QSizeF();
  }
  else
  {
    // can't use decodePoint here -- has no OK handling
    const QStringList list = value.toString().trimmed().split( ',' );
    if ( list.count() != 2 )
      return QSizeF();
    bool convertOk = false;
    const double x = list.at( 0 ).toDouble( &convertOk );
    if ( convertOk )
    {
      const double y = list.at( 1 ).toDouble( &convertOk );
      if ( convertOk )
      {
        if ( ok )
          *ok = true;
        return QSizeF( x, y );
      }
    }
    return QSizeF();
  }
}

QString QgsSymbolLayerUtils::encodeMapUnitScale( const QgsMapUnitScale &mapUnitScale )
{
  return u"3x:%1,%2,%3,%4,%5,%6"_s.arg( qgsDoubleToString( mapUnitScale.minScale ),
                                        qgsDoubleToString( mapUnitScale.maxScale ) )
         .arg( mapUnitScale.minSizeMMEnabled ? 1 : 0 )
         .arg( mapUnitScale.minSizeMM )
         .arg( mapUnitScale.maxSizeMMEnabled ? 1 : 0 )
         .arg( mapUnitScale.maxSizeMM );
}

QgsMapUnitScale QgsSymbolLayerUtils::decodeMapUnitScale( const QString &str )
{
  QStringList lst;
  bool v3 = false;
  if ( str.startsWith( "3x:"_L1 ) )
  {
    v3 = true;
    const QString chopped = str.mid( 3 );
    lst = chopped.split( ',' );
  }
  else
  {
    lst = str.split( ',' );
  }
  if ( lst.count() < 2 )
    return QgsMapUnitScale();

  double minScale = lst[0].toDouble();
  if ( !v3 )
    minScale = minScale != 0 ? 1.0 / minScale : 0;
  double maxScale = lst[1].toDouble();
  if ( !v3 )
    maxScale = maxScale != 0 ? 1.0 / maxScale : 0;

  if ( lst.count() < 6 )
  {
    // old format
    return QgsMapUnitScale( minScale, maxScale );
  }

  QgsMapUnitScale s( minScale, maxScale );
  s.minSizeMMEnabled = lst[2].toInt();
  s.minSizeMM = lst[3].toDouble();
  s.maxSizeMMEnabled = lst[4].toInt();
  s.maxSizeMM = lst[5].toDouble();
  return s;
}

QString QgsSymbolLayerUtils::encodeSldUom( Qgis::RenderUnit unit, double *scaleFactor )
{
  switch ( unit )
  {
    case Qgis::RenderUnit::MapUnits:
      if ( scaleFactor )
        *scaleFactor = 0.001; // from millimeters to meters
      return u"http://www.opengeospatial.org/se/units/metre"_s;

    case Qgis::RenderUnit::MetersInMapUnits:
      if ( scaleFactor )
        *scaleFactor = 1.0; // from meters to meters
      return u"http://www.opengeospatial.org/se/units/metre"_s;

    case Qgis::RenderUnit::Millimeters:
    default:
      // pixel is the SLD default uom. The "standardized rendering pixel
      // size" is defined to be 0.28mm × 0.28mm (millimeters).
      if ( scaleFactor )
        *scaleFactor = 1 / 0.28;  // from millimeters to pixels

      // http://www.opengeospatial.org/sld/units/pixel
      return QString();
  }
}

Qgis::RenderUnit QgsSymbolLayerUtils::decodeSldUom( const QString &str, double *scaleFactor )
{
  if ( str == "http://www.opengeospatial.org/se/units/metre"_L1 )
  {
    if ( scaleFactor )
      *scaleFactor = 1.0;  // from meters to meters
    return Qgis::RenderUnit::MetersInMapUnits;
  }
  else if ( str == "http://www.opengeospatial.org/se/units/foot"_L1 )
  {
    if ( scaleFactor )
      *scaleFactor = 0.3048; // from feet to meters
    return Qgis::RenderUnit::MetersInMapUnits;
  }
  // pixel is the SLD default uom so it's used if no uom attribute is available or
  // if uom="http://www.opengeospatial.org/se/units/pixel"
  else
  {
    if ( scaleFactor )
      *scaleFactor = 1.0; // from pixels to pixels
    return Qgis::RenderUnit::Pixels;
  }
}

QString QgsSymbolLayerUtils::encodeRealVector( const QVector<qreal> &v )
{
  QString vectorString;
  QVector<qreal>::const_iterator it = v.constBegin();
  for ( ; it != v.constEnd(); ++it )
  {
    if ( it != v.constBegin() )
    {
      vectorString.append( ';' );
    }
    vectorString.append( QString::number( *it ) );
  }
  return vectorString;
}

QVector<qreal> QgsSymbolLayerUtils::decodeRealVector( const QString &s )
{
  QVector<qreal> resultVector;

  const QStringList realList = s.split( ';' );
  QStringList::const_iterator it = realList.constBegin();
  for ( ; it != realList.constEnd(); ++it )
  {
    resultVector.append( it->toDouble() );
  }

  return resultVector;
}

QString QgsSymbolLayerUtils::encodeSldRealVector( const QVector<qreal> &v )
{
  QString vectorString;
  QVector<qreal>::const_iterator it = v.constBegin();
  for ( ; it != v.constEnd(); ++it )
  {
    if ( it != v.constBegin() )
    {
      vectorString.append( ' ' );
    }
    vectorString.append( QString::number( *it ) );
  }
  return vectorString;
}

QVector<qreal> QgsSymbolLayerUtils::decodeSldRealVector( const QString &s )
{
  QVector<qreal> resultVector;

  const QStringList realList = s.split( ' ' );
  QStringList::const_iterator it = realList.constBegin();
  for ( ; it != realList.constEnd(); ++it )
  {
    resultVector.append( it->toDouble() );
  }

  return resultVector;
}

QString QgsSymbolLayerUtils::encodeScaleMethod( Qgis::ScaleMethod scaleMethod )
{
  QString encodedValue;

  switch ( scaleMethod )
  {
    case Qgis::ScaleMethod::ScaleDiameter:
      encodedValue = u"diameter"_s;
      break;
    case Qgis::ScaleMethod::ScaleArea:
      encodedValue = u"area"_s;
      break;
  }
  return encodedValue;
}

Qgis::ScaleMethod QgsSymbolLayerUtils::decodeScaleMethod( const QString &str )
{
  Qgis::ScaleMethod scaleMethod;

  if ( str == "diameter"_L1 )
  {
    scaleMethod = Qgis::ScaleMethod::ScaleDiameter;
  }
  else
  {
    scaleMethod = Qgis::ScaleMethod::ScaleArea;
  }

  return scaleMethod;
}

QPainter::CompositionMode QgsSymbolLayerUtils::decodeBlendMode( const QString &s )
{
  if ( s.compare( "Lighten"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Lighten;
  if ( s.compare( "Screen"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Screen;
  if ( s.compare( "Dodge"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_ColorDodge;
  if ( s.compare( "Addition"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Plus;
  if ( s.compare( "Darken"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Darken;
  if ( s.compare( "Multiply"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Multiply;
  if ( s.compare( "Burn"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_ColorBurn;
  if ( s.compare( "Overlay"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Overlay;
  if ( s.compare( "SoftLight"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_SoftLight;
  if ( s.compare( "HardLight"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_HardLight;
  if ( s.compare( "Difference"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Difference;
  if ( s.compare( "Subtract"_L1, Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Exclusion;
  return QPainter::CompositionMode_SourceOver; // "Normal"
}

QIcon QgsSymbolLayerUtils::symbolPreviewIcon( const QgsSymbol *symbol, QSize size, int padding, QgsLegendPatchShape *shape, const QgsScreenProperties &screen )
{
  return QIcon( symbolPreviewPixmap( symbol, size, padding, nullptr, false, nullptr, shape, screen ) );
}

QPixmap QgsSymbolLayerUtils::symbolPreviewPixmap( const QgsSymbol *symbol, QSize size, int padding, QgsRenderContext *customContext, bool selected, const QgsExpressionContext *expressionContext, const QgsLegendPatchShape *shape, const QgsScreenProperties &screen )
{
  Q_ASSERT( symbol );

  const double devicePixelRatio = screen.isValid() ? screen.devicePixelRatio() : 1;
  QPixmap pixmap( size * devicePixelRatio );
  pixmap.setDevicePixelRatio( devicePixelRatio );

  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );
  if ( customContext )
    customContext->setPainterFlagsUsingContext( &painter );
  else
  {
    painter.setRenderHint( QPainter::Antialiasing );
    painter.setRenderHint( QPainter::SmoothPixmapTransform );
  }

  if ( customContext )
  {
    customContext->setPainter( &painter );
  }

  if ( padding > 0 )
  {
    size.setWidth( size.rwidth() - ( padding * 2 ) );
    size.setHeight( size.rheight() - ( padding * 2 ) );
    painter.translate( padding, padding );
  }

  // If the context has no feature and there are DD properties,
  // use a clone and clear some DDs: see issue #19096
  // Applying a data defined size to a categorized layer hides its category symbol in the layers panel and legend
  if ( symbol->hasDataDefinedProperties() &&
       !( customContext
          && customContext->expressionContext().hasFeature( ) ) )
  {
    std::unique_ptr<QgsSymbol> symbol_noDD( symbol->clone( ) );
    const QgsSymbolLayerList layers( symbol_noDD->symbolLayers() );
    for ( const auto &layer : layers )
    {
      for ( int i = 0; i < layer->dataDefinedProperties().count(); ++i )
      {
        QgsProperty &prop = layer->dataDefinedProperties().property( i );
        // don't clear project color properties -- we want to show them in symbol previews
        if ( prop.isActive() && !prop.isProjectColor() )
          prop.setActive( false );
      }
    }
    symbol_noDD->drawPreviewIcon( &painter, size, customContext, selected, expressionContext, shape, screen );
  }
  else
  {
    std::unique_ptr<QgsSymbol> symbolClone( symbol->clone( ) );
    symbolClone->drawPreviewIcon( &painter, size, customContext, selected, expressionContext, shape, screen );
  }

  painter.end();
  return pixmap;
}

double QgsSymbolLayerUtils::estimateMaxSymbolBleed( QgsSymbol *symbol, const QgsRenderContext &context )
{
  double maxBleed = 0;
  for ( int i = 0; i < symbol->symbolLayerCount(); i++ )
  {
    QgsSymbolLayer *layer = symbol->symbolLayer( i );
    const double layerMaxBleed = layer->estimateMaxBleed( context );
    maxBleed = layerMaxBleed > maxBleed ? layerMaxBleed : maxBleed;
  }

  return maxBleed;
}

QPicture QgsSymbolLayerUtils::symbolLayerPreviewPicture( const QgsSymbolLayer *layer, Qgis::RenderUnit units, QSize size, const QgsMapUnitScale &, Qgis::SymbolType parentSymbolType )
{
  QPicture picture;
  QPainter painter;
  painter.begin( &picture );
  painter.setRenderHint( QPainter::Antialiasing );
  QgsRenderContext renderContext = QgsRenderContext::fromQPainter( &painter );
  renderContext.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
  renderContext.setFlag( Qgis::RenderContextFlag::RenderSymbolPreview, true );
  renderContext.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  renderContext.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms, true );
  renderContext.setPainterFlagsUsingContext( &painter );

  QgsSymbolRenderContext symbolContext( renderContext, units, 1.0, false, Qgis::SymbolRenderHints(), nullptr );

  switch ( parentSymbolType )
  {
    case Qgis::SymbolType::Marker:
      symbolContext.setOriginalGeometryType( Qgis::GeometryType::Point );
      break;
    case Qgis::SymbolType::Line:
      symbolContext.setOriginalGeometryType( Qgis::GeometryType::Line );
      break;
    case Qgis::SymbolType::Fill:
      symbolContext.setOriginalGeometryType( Qgis::GeometryType::Polygon );
      break;
    case Qgis::SymbolType::Hybrid:
      break;
  }

  std::unique_ptr< QgsSymbolLayer > layerClone( layer->clone() );
  layerClone->drawPreviewIcon( symbolContext, size );
  painter.end();
  return picture;
}

QIcon QgsSymbolLayerUtils::symbolLayerPreviewIcon( const QgsSymbolLayer *layer, Qgis::RenderUnit u, QSize size, const QgsMapUnitScale &, Qgis::SymbolType parentSymbolType, QgsMapLayer *mapLayer, const QgsScreenProperties &screen )
{
  const double devicePixelRatio = screen.isValid() ? screen.devicePixelRatio() : 1;
  QPixmap pixmap( size * devicePixelRatio );
  pixmap.setDevicePixelRatio( devicePixelRatio );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );
  painter.setRenderHint( QPainter::Antialiasing );
  QgsRenderContext renderContext = QgsRenderContext::fromQPainter( &painter );

  if ( screen.isValid() )
  {
    screen.updateRenderContextForScreen( renderContext );
  }

  renderContext.setFlag( Qgis::RenderContextFlag::RenderSymbolPreview );
  renderContext.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms );
  renderContext.setDevicePixelRatio( devicePixelRatio );
  // build a minimal expression context
  QgsExpressionContext expContext;
  expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mapLayer ) );
  renderContext.setExpressionContext( expContext );

  QgsSymbolRenderContext symbolContext( renderContext, u, 1.0, false, Qgis::SymbolRenderHints(), nullptr );

  switch ( parentSymbolType )
  {
    case Qgis::SymbolType::Marker:
      symbolContext.setOriginalGeometryType( Qgis::GeometryType::Point );
      break;
    case Qgis::SymbolType::Line:
      symbolContext.setOriginalGeometryType( Qgis::GeometryType::Line );
      break;
    case Qgis::SymbolType::Fill:
      symbolContext.setOriginalGeometryType( Qgis::GeometryType::Polygon );
      break;
    case Qgis::SymbolType::Hybrid:
      break;
  }

  std::unique_ptr< QgsSymbolLayer > layerClone( layer->clone() );
  layerClone->drawPreviewIcon( symbolContext, size );
  painter.end();
  return QIcon( pixmap );
}

QIcon QgsSymbolLayerUtils::colorRampPreviewIcon( QgsColorRamp *ramp, QSize size, int padding )
{
  return QIcon( colorRampPreviewPixmap( ramp, size, padding ) );
}

QPixmap QgsSymbolLayerUtils::colorRampPreviewPixmap( QgsColorRamp *ramp, QSize size, int padding, Qt::Orientation direction, bool flipDirection, bool drawTransparentBackground )
{
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  // pixmap.fill( Qt::white ); // this makes the background white instead of transparent
  QPainter painter;
  painter.begin( &pixmap );

  //draw stippled background, for transparent images
  if ( drawTransparentBackground )
    drawStippledBackground( &painter, QRect( padding, padding, size.width() - padding * 2, size.height() - padding  * 2 ) );

  // antialiasing makes the colors duller, and no point in antialiasing a color ramp
  // painter.setRenderHint( QPainter::Antialiasing );
  switch ( direction )
  {
    case Qt::Horizontal:
    {
      for ( int i = 0; i < size.width(); i++ )
      {
        const QPen pen( ramp->color( static_cast< double >( i ) / size.width() ) );
        painter.setPen( pen );
        const int x = flipDirection ? size.width() - i - 1 : i;
        painter.drawLine( x, 0 + padding, x, size.height() - 1 - padding );
      }
      break;
    }

    case Qt::Vertical:
    {
      for ( int i = 0; i < size.height(); i++ )
      {
        const QPen pen( ramp->color( static_cast< double >( i ) / size.height() ) );
        painter.setPen( pen );
        const int y = flipDirection ? size.height() - i - 1 : i;
        painter.drawLine( 0 + padding, y, size.width() - 1 - padding, y );
      }
      break;
    }
  }

  painter.end();
  return pixmap;
}

void QgsSymbolLayerUtils::drawStippledBackground( QPainter *painter, QRect rect )
{
  // create a 2x2 checker-board image
  uchar pixDataRGB[] = { 255, 255, 255, 255,
                         127, 127, 127, 255,
                         127, 127, 127, 255,
                         255, 255, 255, 255
                       };
  const QImage img( pixDataRGB, 2, 2, 8, QImage::Format_ARGB32 );
  // scale it to rect so at least 5 patterns are shown
  const int width = ( rect.width() < rect.height() ) ?
                    rect.width() / 2.5 : rect.height() / 2.5;
  const QPixmap pix = QPixmap::fromImage( img.scaled( width, width ) );
  // fill rect with texture
  QBrush brush;
  brush.setTexture( pix );
  painter->fillRect( rect, brush );
}

void QgsSymbolLayerUtils::drawVertexMarker( double x, double y, QPainter &p, Qgis::VertexMarkerType type, int markerSize )
{
  const qreal s = ( markerSize - 1 ) / 2.0;

  switch ( type )
  {
    case Qgis::VertexMarkerType::SemiTransparentCircle:
      p.setPen( QColor( 50, 100, 120, 200 ) );
      p.setBrush( QColor( 200, 200, 210, 120 ) );
      p.drawEllipse( x - s, y - s, s * 2, s * 2 );
      break;
    case Qgis::VertexMarkerType::Cross:
      p.setPen( QColor( 255, 0, 0 ) );
      p.drawLine( x - s, y + s, x + s, y - s );
      p.drawLine( x - s, y - s, x + s, y + s );
      break;
    case Qgis::VertexMarkerType::NoMarker:
      break;
  }
}

#include <QPolygonF>

#include <cmath>
#include <cfloat>

static QPolygonF makeOffsetGeometry( const QgsPolylineXY &polyline )
{
  int i, pointCount = polyline.count();

  QPolygonF resultLine;
  resultLine.resize( pointCount );

  const QgsPointXY *tempPtr = polyline.data();

  for ( i = 0; i < pointCount; ++i, tempPtr++ )
    resultLine[i] = QPointF( tempPtr->x(), tempPtr->y() );

  return resultLine;
}
static QList<QPolygonF> makeOffsetGeometry( const QgsPolygonXY &polygon )
{
  QList<QPolygonF> resultGeom;
  resultGeom.reserve( polygon.size() );
  for ( int ring = 0; ring < polygon.size(); ++ring )
    resultGeom.append( makeOffsetGeometry( polygon[ ring ] ) );
  return resultGeom;
}

QList<QPolygonF> offsetLine( QPolygonF polyline, double dist, Qgis::GeometryType geometryType )
{
  QList<QPolygonF> resultLine;

  if ( polyline.count() < 2 )
  {
    resultLine.append( polyline );
    return resultLine;
  }

  unsigned int i, pointCount = polyline.count();

  QgsPolylineXY tempPolyline( pointCount );
  QPointF *tempPtr = polyline.data();
  for ( i = 0; i < pointCount; ++i, tempPtr++ )
    tempPolyline[i] = QgsPointXY( tempPtr->rx(), tempPtr->ry() );

  QgsGeometry tempGeometry = geometryType == Qgis::GeometryType::Polygon ? QgsGeometry::fromPolygonXY( QgsPolygonXY() << tempPolyline ) : QgsGeometry::fromPolylineXY( tempPolyline );
  if ( !tempGeometry.isNull() )
  {
    const int quadSegments = 0; // we want miter joins, not round joins
    const double miterLimit = 2.0; // the default value in GEOS (5.0) allows for fairly sharp endings
    QgsGeometry offsetGeom;
    if ( geometryType == Qgis::GeometryType::Polygon )
      offsetGeom = tempGeometry.buffer( -dist, quadSegments, Qgis::EndCapStyle::Flat,
                                        Qgis::JoinStyle::Miter, miterLimit );
    else
      offsetGeom = tempGeometry.offsetCurve( dist, quadSegments, Qgis::JoinStyle::Miter, miterLimit );

    if ( !offsetGeom.isNull() )
    {
      tempGeometry = offsetGeom;

      if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == Qgis::WkbType::LineString )
      {
        const QgsPolylineXY line = tempGeometry.asPolyline();
        resultLine.append( makeOffsetGeometry( line ) );
        return resultLine;
      }
      else if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == Qgis::WkbType::Polygon )
      {
        resultLine.append( makeOffsetGeometry( tempGeometry.asPolygon() ) );
        return resultLine;
      }
      else if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == Qgis::WkbType::MultiLineString )
      {
        QgsMultiPolylineXY tempMPolyline = tempGeometry.asMultiPolyline();
        resultLine.reserve( tempMPolyline.count() );
        for ( int part = 0; part < tempMPolyline.count(); ++part )
        {
          resultLine.append( makeOffsetGeometry( tempMPolyline[ part ] ) );
        }
        return resultLine;
      }
      else if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == Qgis::WkbType::MultiPolygon )
      {
        QgsMultiPolygonXY tempMPolygon = tempGeometry.asMultiPolygon();
        resultLine.reserve( tempMPolygon.count() );
        for ( int part = 0; part < tempMPolygon.count(); ++part )
        {
          resultLine.append( makeOffsetGeometry( tempMPolygon[ part ] ) );
        }
        return resultLine;
      }
    }
  }

  // returns original polyline when 'GEOSOffsetCurve' fails!
  resultLine.append( polyline );
  return resultLine;
}

/////


std::unique_ptr< QgsSymbol > QgsSymbolLayerUtils::loadSymbol( const QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.isNull() )
    return nullptr;

  QgsSymbolLayerList layers;
  QDomNode layerNode = element.firstChild();

  while ( !layerNode.isNull() )
  {
    QDomElement e = layerNode.toElement();
    if ( !e.isNull() && e.tagName() != "data_defined_properties"_L1 && e.tagName() != "buffer"_L1 )
    {
      if ( e.tagName() != "layer"_L1 )
      {
        QgsDebugError( "unknown tag " + e.tagName() );
      }
      else
      {
        std::unique_ptr< QgsSymbolLayer > layer = loadSymbolLayer( e, context );
        if ( layer )
        {
          // Dealing with sub-symbols nested into a layer
          const QDomElement s = e.firstChildElement( u"symbol"_s );
          if ( !s.isNull() )
          {
            std::unique_ptr< QgsSymbol > subSymbol( loadSymbol( s, context ) );
            // special handling for SVG fill symbol layer -- upgrade the subsymbol which
            // was historically used for the fill stroke to be dedicated symbol layer instead
            // in order to match the behavior of all other fill symbol layer types
            if ( dynamic_cast< QgsSVGFillSymbolLayer * >( layer.get() ) )
            {
              // add the SVG fill first
              layers.append( layer.release() );
              // then add the layers from the subsymbol stroke outline on top
              for ( int i = 0; i < subSymbol->symbolLayerCount(); ++i )
              {
                layers.append( subSymbol->symbolLayer( i )->clone() );
              }
            }
            else
            {
              const bool res = layer->setSubSymbol( subSymbol.release() );
              if ( !res )
              {
                QgsDebugError( u"symbol layer refused subsymbol: "_s + s.attribute( "name" ) );
              }
              layers.append( layer.release() );
            }
          }
          else
          {
            layers.append( layer.release() );
          }
        }
      }
    }
    layerNode = layerNode.nextSibling();
  }

  if ( layers.isEmpty() )
  {
    QgsDebugError( u"no layers for symbol"_s );
    return nullptr;
  }

  const QString symbolType = element.attribute( u"type"_s );

  std::unique_ptr< QgsSymbol > symbol;
  if ( symbolType == "line"_L1 )
    symbol = std::make_unique< QgsLineSymbol >( layers );
  else if ( symbolType == "fill"_L1 )
    symbol = std::make_unique< QgsFillSymbol >( layers );
  else if ( symbolType == "marker"_L1 )
    symbol = std::make_unique< QgsMarkerSymbol >( layers );
  else
  {
    QgsDebugError( "unknown symbol type " + symbolType );
    return nullptr;
  }

  if ( element.hasAttribute( u"outputUnit"_s ) )
  {
    symbol->setOutputUnit( QgsUnitTypes::decodeRenderUnit( element.attribute( u"outputUnit"_s ) ) );
  }
  if ( element.hasAttribute( ( u"mapUnitScale"_s ) ) )
  {
    QgsMapUnitScale mapUnitScale;
    const double oldMin = element.attribute( u"mapUnitMinScale"_s, u"0.0"_s ).toDouble();
    mapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = element.attribute( u"mapUnitMaxScale"_s, u"0.0"_s ).toDouble();
    mapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
    symbol->setMapUnitScale( mapUnitScale );
  }
  symbol->setOpacity( element.attribute( u"alpha"_s, u"1.0"_s ).toDouble() );
  symbol->setExtentBuffer( element.attribute( u"extent_buffer"_s, u"0.0"_s ).toDouble() );
  symbol->setExtentBufferSizeUnit( QgsUnitTypes::decodeRenderUnit( element.attribute( u"extent_buffer_unit"_s, u"MapUnit"_s ) ) );
  symbol->setClipFeaturesToExtent( element.attribute( u"clip_to_extent"_s, u"1"_s ).toInt() );
  symbol->setForceRHR( element.attribute( u"force_rhr"_s, u"0"_s ).toInt() );
  Qgis::SymbolFlags flags;
  if ( element.attribute( u"renderer_should_use_levels"_s, u"0"_s ).toInt() )
    flags |= Qgis::SymbolFlag::RendererShouldUseSymbolLevels;
  symbol->setFlags( flags );

  symbol->animationSettings().setIsAnimated( element.attribute( u"is_animated"_s, u"0"_s ).toInt() );
  symbol->animationSettings().setFrameRate( element.attribute( u"frame_rate"_s, u"10"_s ).toDouble() );

  if ( !element.firstChildElement( u"buffer"_s ).isNull() )
  {
    auto bufferSettings = std::make_unique< QgsSymbolBufferSettings >();
    bufferSettings->readXml( element, context );
    symbol->setBufferSettings( bufferSettings.release() );
  }
  else
  {
    symbol->setBufferSettings( nullptr );
  }

  const QDomElement ddProps = element.firstChildElement( u"data_defined_properties"_s );
  if ( !ddProps.isNull() )
  {
    symbol->dataDefinedProperties().readXml( ddProps, QgsSymbol::propertyDefinitions() );
  }

  return symbol;
}

std::unique_ptr< QgsSymbolLayer > QgsSymbolLayerUtils::loadSymbolLayer( QDomElement &element, const QgsReadWriteContext &context )
{
  const QString layerClass = element.attribute( u"class"_s );
  const bool locked = element.attribute( u"locked"_s ).toInt();
  const bool enabled = element.attribute( u"enabled"_s, u"1"_s ).toInt();
  const int pass = element.attribute( u"pass"_s ).toInt();
  const QString id = element.attribute( u"id"_s );
  const Qgis::SymbolLayerUserFlags userFlags = qgsFlagKeysToValue( element.attribute( u"userFlags"_s ), Qgis::SymbolLayerUserFlags() );

  // parse properties
  QVariantMap props = parseProperties( element );

  // if there are any paths stored in properties, convert them from relative to absolute
  QgsApplication::symbolLayerRegistry()->resolvePaths( layerClass, props, context.pathResolver(), false );

  QgsApplication::symbolLayerRegistry()->resolveFonts( layerClass, props, context );

  std::unique_ptr< QgsSymbolLayer > layer = QgsApplication::symbolLayerRegistry()->createSymbolLayer( layerClass, props );
  if ( layer )
  {
    layer->setLocked( locked );
    layer->setRenderingPass( pass );
    layer->setEnabled( enabled );
    layer->setUserFlags( userFlags );

    // old project format, empty is missing, keep the actual layer one
    if ( !id.isEmpty() )
      layer->setId( id );

    //restore layer effect
    const QDomElement effectElem = element.firstChildElement( u"effect"_s );
    if ( !effectElem.isNull() )
    {
      std::unique_ptr< QgsPaintEffect > effect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
      if ( effect && !QgsPaintEffectRegistry::isDefaultStack( effect.get() ) )
        layer->setPaintEffect( effect.release() );
    }

    // restore data defined properties
    const QDomElement ddProps = element.firstChildElement( u"data_defined_properties"_s );
    if ( !ddProps.isNull() )
    {
      const QgsPropertyCollection prevProperties = layer->dataDefinedProperties();
      layer->dataDefinedProperties().readXml( ddProps, QgsSymbolLayer::propertyDefinitions() );

      // some symbol layers will be created with data defined properties by default -- we want to retain
      // these if they weren't restored from the xml
      const QSet< int > oldKeys = prevProperties.propertyKeys();
      for ( int key : oldKeys )
      {
        if ( !layer->dataDefinedProperties().propertyKeys().contains( key ) )
          layer->setDataDefinedProperty( static_cast< QgsSymbolLayer::Property >( key ), prevProperties.property( key ) );
      }
    }

    return layer;
  }
  else
  {
    QgsDebugError( "unknown class " + layerClass );
    return nullptr;
  }
}

static QString _nameForSymbolType( Qgis::SymbolType type )
{
  switch ( type )
  {
    case Qgis::SymbolType::Line:
      return u"line"_s;
    case Qgis::SymbolType::Marker:
      return u"marker"_s;
    case Qgis::SymbolType::Fill:
      return u"fill"_s;
    default:
      return QString();
  }
}

QDomElement QgsSymbolLayerUtils::saveSymbol( const QString &name, const QgsSymbol *symbol, QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_ASSERT( symbol );
  QDomElement symEl = doc.createElement( u"symbol"_s );
  symEl.setAttribute( u"type"_s, _nameForSymbolType( symbol->type() ) );
  symEl.setAttribute( u"name"_s, name );
  symEl.setAttribute( u"alpha"_s, QString::number( symbol->opacity() ) );
  symEl.setAttribute( u"clip_to_extent"_s, symbol->clipFeaturesToExtent() ? u"1"_s : u"0"_s );
  if ( !qgsDoubleNear( symbol->extentBuffer(), 0 ) )
  {
    symEl.setAttribute( u"extent_buffer"_s, QString::number( symbol->extentBuffer() ) );
    symEl.setAttribute( u"extent_buffer_unit"_s, QgsUnitTypes::encodeUnit( symbol->extentBufferSizeUnit() ) );
  }
  symEl.setAttribute( u"force_rhr"_s, symbol->forceRHR() ? u"1"_s : u"0"_s );
  if ( symbol->flags() & Qgis::SymbolFlag::RendererShouldUseSymbolLevels )
    symEl.setAttribute( u"renderer_should_use_levels"_s, u"1"_s );

  symEl.setAttribute( u"is_animated"_s, symbol->animationSettings().isAnimated() ? u"1"_s : u"0"_s );
  symEl.setAttribute( u"frame_rate"_s, qgsDoubleToString( symbol->animationSettings().frameRate() ) );

  if ( const QgsSymbolBufferSettings *bufferSettings = symbol->bufferSettings() )
    bufferSettings->writeXml( symEl, context );

  //QgsDebugMsgLevel( "num layers " + QString::number( symbol->symbolLayerCount() ), 2 );

  QDomElement ddProps = doc.createElement( u"data_defined_properties"_s );
  symbol->dataDefinedProperties().writeXml( ddProps, QgsSymbol::propertyDefinitions() );
  symEl.appendChild( ddProps );

  for ( int i = 0; i < symbol->symbolLayerCount(); i++ )
  {
    const QgsSymbolLayer *layer = symbol->symbolLayer( i );

    QDomElement layerEl = doc.createElement( u"layer"_s );
    layerEl.setAttribute( u"class"_s, layer->layerType() );
    layerEl.setAttribute( u"enabled"_s, layer->enabled() );
    layerEl.setAttribute( u"locked"_s, layer->isLocked() );
    layerEl.setAttribute( u"pass"_s, layer->renderingPass() );
    layerEl.setAttribute( u"id"_s, layer->id() );
    if ( layer->userFlags() != Qgis::SymbolLayerUserFlags() )
      layerEl.setAttribute( u"userFlags"_s, qgsFlagValueToKeys( layer->userFlags() ) );

    QVariantMap props = layer->properties();

    // if there are any paths in properties, convert them from absolute to relative
    QgsApplication::symbolLayerRegistry()->resolvePaths( layer->layerType(), props, context.pathResolver(), true );

    saveProperties( props, doc, layerEl );

    if ( layer->paintEffect() && !QgsPaintEffectRegistry::isDefaultStack( layer->paintEffect() ) )
      layer->paintEffect()->saveProperties( doc, layerEl );

    QDomElement ddProps = doc.createElement( u"data_defined_properties"_s );
    layer->dataDefinedProperties().writeXml( ddProps, QgsSymbolLayer::propertyDefinitions() );
    layerEl.appendChild( ddProps );

    if ( const QgsSymbol *subSymbol = const_cast< QgsSymbolLayer * >( layer )->subSymbol() )
    {
      const QString subname = u"@%1@%2"_s.arg( name ).arg( i );
      const QDomElement subEl = saveSymbol( subname, subSymbol, doc, context );
      layerEl.appendChild( subEl );
    }
    symEl.appendChild( layerEl );
  }

  return symEl;
}

QString QgsSymbolLayerUtils::symbolProperties( QgsSymbol *symbol )
{
  QDomDocument doc( u"qgis-symbol-definition"_s );
  const QDomElement symbolElem = saveSymbol( u"symbol"_s, symbol, doc, QgsReadWriteContext() );
  QString props;
  QTextStream stream( &props );
  symbolElem.save( stream, -1 );
  return props;
}

bool QgsSymbolLayerUtils::createSymbolLayerListFromSld( QDomElement &element,
    Qgis::GeometryType geomType,
    QList<QgsSymbolLayer *> &layers )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  if ( element.isNull() )
    return false;

  const QString symbolizerName = element.localName();

  if ( symbolizerName == "PointSymbolizer"_L1 )
  {
    // first check for Graphic element, nothing will be rendered if not found
    const QDomElement graphicElem = element.firstChildElement( u"Graphic"_s );
    if ( graphicElem.isNull() )
    {
      QgsDebugError( u"Graphic element not found in PointSymbolizer"_s );
    }
    else
    {
      switch ( geomType )
      {
        case Qgis::GeometryType::Polygon:
        {
          // polygon layer and point symbolizer: draw polygon centroid
          std::unique_ptr< QgsSymbolLayer> l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"CentroidFill"_s, element );
          if ( l )
            layers.append( l.release() );

          break;
        }

        case Qgis::GeometryType::Point:
        {
          // point layer and point symbolizer: use markers
          std::unique_ptr< QgsSymbolLayer> l( createMarkerLayerFromSld( element ) );
          if ( l )
            layers.append( l.release() );

          break;
        }

        case Qgis::GeometryType::Line:
        {
          // line layer and point symbolizer: draw central point
          std::unique_ptr< QgsSymbolLayer> l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"SimpleMarker"_s, element );
          if ( l )
            layers.append( l.release() );

          break;
        }

        case Qgis::GeometryType::Unknown:
        case Qgis::GeometryType::Null:
          break;
      }
    }
  }

  if ( symbolizerName == "LineSymbolizer"_L1 )
  {
    // check for Stroke element, nothing will be rendered if not found
    const QDomElement strokeElem = element.firstChildElement( u"Stroke"_s );
    if ( strokeElem.isNull() )
    {
      QgsDebugError( u"Stroke element not found in LineSymbolizer"_s );
    }
    else
    {
      switch ( geomType )
      {
        case Qgis::GeometryType::Polygon:
        case Qgis::GeometryType::Line:
        {
          // polygon layer and line symbolizer: draw polygon stroke
          // line layer and line symbolizer: draw line
          std::unique_ptr< QgsSymbolLayer> l = createLineLayerFromSld( element );
          if ( l )
            layers.append( l.release() );

          break;
        }

        case Qgis::GeometryType::Point:
        {
          // point layer and line symbolizer: draw a little line marker
          std::unique_ptr< QgsSymbolLayer> l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"MarkerLine"_s, element );
          if ( l )
            layers.append( l.release() );

          break;
        }

        case Qgis::GeometryType::Unknown:
        case Qgis::GeometryType::Null:
          break;
      }
    }
  }

  if ( symbolizerName == "PolygonSymbolizer"_L1 )
  {
    // get Fill and Stroke elements, nothing will be rendered if both are missing
    const QDomElement fillElem = element.firstChildElement( u"Fill"_s );
    const QDomElement strokeElem = element.firstChildElement( u"Stroke"_s );
    if ( fillElem.isNull() && strokeElem.isNull() )
    {
      QgsDebugError( u"neither Fill nor Stroke element not found in PolygonSymbolizer"_s );
    }
    else
    {
      switch ( geomType )
      {
        case Qgis::GeometryType::Polygon:
        {
          // polygon layer and polygon symbolizer: draw fill

          std::unique_ptr< QgsSymbolLayer > l = createFillLayerFromSld( element );
          if ( l )
          {
            QgsSymbolLayer *lastLayer = l.get();
            layers.append( l.release() );

            // SVGFill and SimpleFill symbolLayerV2 supports stroke internally,
            // so don't go forward to create a different symbolLayerV2 for stroke
            if ( lastLayer->layerType() == "SimpleFill"_L1 || lastLayer->layerType() == "SVGFill"_L1 )
              break;
          }

          // now create polygon stroke
          // polygon layer and polygon symbolizer: draw polygon stroke
          l = createLineLayerFromSld( element );
          if ( l )
            layers.append( l.release() );

          break;
        }

        case Qgis::GeometryType::Line:
        {
          // line layer and polygon symbolizer: draw line
          std::unique_ptr< QgsSymbolLayer > l = createLineLayerFromSld( element );
          if ( l )
            layers.append( l.release() );

          break;
        }

        case Qgis::GeometryType::Point:
        {
          // point layer and polygon symbolizer: draw a square marker
          convertPolygonSymbolizerToPointMarker( element, layers );
          break;
        }
        case Qgis::GeometryType::Unknown:
        case Qgis::GeometryType::Null:
          break;
      }
    }
  }

  return true;
}

std::unique_ptr< QgsSymbolLayer > QgsSymbolLayerUtils::createFillLayerFromSld( QDomElement &element )
{
  const QDomElement fillElem = element.firstChildElement( u"Fill"_s );
  if ( fillElem.isNull() )
  {
    QgsDebugError( u"Fill element not found"_s );
    return nullptr;
  }

  std::unique_ptr< QgsSymbolLayer > l;

  if ( needLinePatternFill( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"LinePatternFill"_s, element );
  else if ( needPointPatternFill( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"PointPatternFill"_s, element );
  else if ( needSvgFill( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"SVGFill"_s, element );
  else if ( needRasterImageFill( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"RasterFill"_s, element );
  else
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"SimpleFill"_s, element );

  return l;
}

std::unique_ptr< QgsSymbolLayer > QgsSymbolLayerUtils::createLineLayerFromSld( QDomElement &element )
{
  const QDomElement strokeElem = element.firstChildElement( u"Stroke"_s );
  if ( strokeElem.isNull() )
  {
    QgsDebugError( u"Stroke element not found"_s );
    return nullptr;
  }

  std::unique_ptr< QgsSymbolLayer > l;

  if ( needMarkerLine( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"MarkerLine"_s, element );
  else
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"SimpleLine"_s, element );

  return l;
}

std::unique_ptr< QgsSymbolLayer > QgsSymbolLayerUtils::createMarkerLayerFromSld( QDomElement &element )
{
  const QDomElement graphicElem = element.firstChildElement( u"Graphic"_s );
  if ( graphicElem.isNull() )
  {
    QgsDebugError( u"Graphic element not found"_s );
    return nullptr;
  }

  std::unique_ptr< QgsSymbolLayer > l;

  if ( needFontMarker( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"FontMarker"_s, element );
  else if ( needSvgMarker( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"SvgMarker"_s, element );
  else if ( needEllipseMarker( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"EllipseMarker"_s, element );
  else if ( needRasterMarker( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"RasterMarker"_s, element );
  else
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( u"SimpleMarker"_s, element );

  return l;
}

bool QgsSymbolLayerUtils::hasExternalGraphic( QDomElement &element )
{
  return hasExternalGraphicV2( element, u"image/svg+xml"_s );
}

bool QgsSymbolLayerUtils::hasExternalGraphicV2( const QDomElement &element, const QString format )
{
  const QDomElement graphicElem = element.firstChildElement( u"Graphic"_s );
  if ( graphicElem.isNull() )
    return false;

  const QDomElement externalGraphicElem = graphicElem.firstChildElement( u"ExternalGraphic"_s );
  if ( externalGraphicElem.isNull() )
    return false;

  // check for format
  const QDomElement formatElem = externalGraphicElem.firstChildElement( u"Format"_s );
  if ( formatElem.isNull() )
    return false;

  const QString elementFormat = formatElem.firstChild().nodeValue();
  if ( ! format.isEmpty() && elementFormat != format )
  {
    QgsDebugMsgLevel( "unsupported External Graphic format found: " + elementFormat, 4 );
    return false;
  }

  // check for a valid content
  const QDomElement onlineResourceElem = externalGraphicElem.firstChildElement( u"OnlineResource"_s );
  const QDomElement inlineContentElem = externalGraphicElem.firstChildElement( u"InlineContent"_s );
  // NOLINTBEGIN(bugprone-branch-clone)
  if ( !onlineResourceElem.isNull() )
  {
    return true;
  }
  else if ( !inlineContentElem.isNull() )
  {
    return true;
  }
  else
  {
    return false;
  }
  // NOLINTEND(bugprone-branch-clone)
}

bool QgsSymbolLayerUtils::hasWellKnownMark( QDomElement &element )
{
  const QDomElement graphicElem = element.firstChildElement( u"Graphic"_s );
  if ( graphicElem.isNull() )
    return false;

  const QDomElement markElem = graphicElem.firstChildElement( u"Mark"_s );
  if ( markElem.isNull() )
    return false;

  const QDomElement wellKnownNameElem = markElem.firstChildElement( u"WellKnownName"_s );
  return !wellKnownNameElem.isNull();
}


bool QgsSymbolLayerUtils::needFontMarker( QDomElement &element )
{
  const QDomElement graphicElem = element.firstChildElement( u"Graphic"_s );
  if ( graphicElem.isNull() )
    return false;

  const QDomElement markElem = graphicElem.firstChildElement( u"Mark"_s );
  if ( markElem.isNull() )
    return false;

  // check for format
  const QDomElement formatElem = markElem.firstChildElement( u"Format"_s );
  if ( formatElem.isNull() )
    return false;

  const QString format = formatElem.firstChild().nodeValue();
  if ( format != "ttf"_L1 )
  {
    QgsDebugError( "unsupported Graphic Mark format found: " + format );
    return false;
  }

  // check for a valid content
  const QDomElement onlineResourceElem = markElem.firstChildElement( u"OnlineResource"_s );
  const QDomElement inlineContentElem = markElem.firstChildElement( u"InlineContent"_s );
  if ( !onlineResourceElem.isNull() )
  {
    // mark with ttf format has a markIndex element
    const QDomElement markIndexElem = markElem.firstChildElement( u"MarkIndex"_s );
    if ( !markIndexElem.isNull() )
      return true;
  }
  else if ( !inlineContentElem.isNull() )
  {
    return false; // not implemented yet
  }

  return false;
}

bool QgsSymbolLayerUtils::needSvgMarker( const QDomElement &element )
{
  return hasExternalGraphicV2( element, u"image/svg+xml"_s );
}

bool QgsSymbolLayerUtils::needRasterMarker( const QDomElement &element )
{
  // any external graphic except SVGs are considered rasters
  return hasExternalGraphicV2( element, QString() ) && !needSvgMarker( element );
}

bool QgsSymbolLayerUtils::needEllipseMarker( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( u"Graphic"_s );
  if ( graphicElem.isNull() )
    return false;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( graphicElem );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == "widthHeightFactor"_L1 )
    {
      return true;
    }
  }

  return false;
}

bool QgsSymbolLayerUtils::needMarkerLine( QDomElement &element )
{
  const QDomElement strokeElem = element.firstChildElement( u"Stroke"_s );
  if ( strokeElem.isNull() )
    return false;

  QDomElement graphicStrokeElem = strokeElem.firstChildElement( u"GraphicStroke"_s );
  if ( graphicStrokeElem.isNull() )
    return false;

  return hasWellKnownMark( graphicStrokeElem );
}

bool QgsSymbolLayerUtils::needLinePatternFill( QDomElement &element )
{
  const QDomElement fillElem = element.firstChildElement( u"Fill"_s );
  if ( fillElem.isNull() )
    return false;

  const QDomElement graphicFillElem = fillElem.firstChildElement( u"GraphicFill"_s );
  if ( graphicFillElem.isNull() )
    return false;

  QDomElement graphicElem = graphicFillElem.firstChildElement( u"Graphic"_s );
  if ( graphicElem.isNull() )
    return false;

  // line pattern fill uses horline wellknown marker with an angle

  QString name;
  QColor fillColor, strokeColor;
  double size, strokeWidth;
  Qt::PenStyle strokeStyle;
  if ( !wellKnownMarkerFromSld( graphicElem, name, fillColor, strokeColor, strokeStyle, strokeWidth, size ) )
    return false;

  if ( name != "horline"_L1 )
    return false;

  QString angleFunc;
  if ( !rotationFromSldElement( graphicElem, angleFunc ) )
    return false;

  bool ok;
  const double angle = angleFunc.toDouble( &ok );
  return !( !ok || qgsDoubleNear( angle, 0.0 ) );
}

bool QgsSymbolLayerUtils::needPointPatternFill( QDomElement &element )
{
  const QDomElement fillElem = element.firstChildElement( u"Fill"_s );
  if ( fillElem.isNull() )
    return false;

  const QDomElement graphicFillElem = fillElem.firstChildElement( u"GraphicFill"_s );
  if ( graphicFillElem.isNull() )
    return false;

  const QDomElement graphicElem = graphicFillElem.firstChildElement( u"Graphic"_s );
  if ( graphicElem.isNull() )
    return false;

  const QDomElement markElem = graphicElem.firstChildElement( u"Mark"_s );
  if ( markElem.isNull() )
    return false;

  return true;
}

bool QgsSymbolLayerUtils::needSvgFill( QDomElement &element )
{
  const QDomElement fillElem = element.firstChildElement( u"Fill"_s );
  if ( fillElem.isNull() )
    return false;

  QDomElement graphicFillElem = fillElem.firstChildElement( u"GraphicFill"_s );
  if ( graphicFillElem.isNull() )
    return false;

  return hasExternalGraphicV2( graphicFillElem, u"image/svg+xml"_s );
}

bool QgsSymbolLayerUtils::needRasterImageFill( QDomElement &element )
{
  const QDomElement fillElem = element.firstChildElement( u"Fill"_s );
  if ( fillElem.isNull() )
    return false;

  QDomElement graphicFillElem = fillElem.firstChildElement( u"GraphicFill"_s );
  if ( graphicFillElem.isNull() )
    return false;

  return hasExternalGraphicV2( graphicFillElem, u"image/png"_s ) || hasExternalGraphicV2( graphicFillElem, u"image/jpeg"_s ) || hasExternalGraphicV2( graphicFillElem, u"image/gif"_s );
}


bool QgsSymbolLayerUtils::convertPolygonSymbolizerToPointMarker( QDomElement &element, QList<QgsSymbolLayer *> &layerList )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  /* SE 1.1 says about PolygonSymbolizer:
  if a point geometry is referenced instead of a polygon,
  then a small, square, ortho-normal polygon should be
  constructed for rendering.
   */

  QgsSymbolLayerList layers;

  // retrieve both Fill and Stroke elements
  QDomElement fillElem = element.firstChildElement( u"Fill"_s );
  QDomElement strokeElem = element.firstChildElement( u"Stroke"_s );

  // first symbol layer
  {
    bool validFill = false, validStroke = false;

    // check for simple fill
    // Fill element can contain some SvgParameter elements
    QColor fillColor;
    Qt::BrushStyle fillStyle;

    if ( fillFromSld( fillElem, fillStyle, fillColor ) )
      validFill = true;

    // check for simple stroke
    // Stroke element can contain some SvgParameter elements
    QColor strokeColor;
    Qt::PenStyle strokeStyle;
    double strokeWidth = 1.0, dashOffset = 0.0;
    QVector<qreal> customDashPattern;

    if ( lineFromSld( strokeElem, strokeStyle, strokeColor, strokeWidth,
                      nullptr, nullptr, &customDashPattern, &dashOffset ) )
      validStroke = true;

    if ( validFill || validStroke )
    {
      QVariantMap map;
      map[u"name"_s] = u"square"_s;
      map[u"color"_s] = QgsColorUtils::colorToString( validFill ? fillColor : Qt::transparent );
      map[u"color_border"_s] = QgsColorUtils::colorToString( validStroke ? strokeColor : Qt::transparent );
      map[u"size"_s] = QString::number( 6 );
      map[u"angle"_s] = QString::number( 0 );
      map[u"offset"_s] = encodePoint( QPointF( 0, 0 ) );
      layers.append( QgsApplication::symbolLayerRegistry()->createSymbolLayer( u"SimpleMarker"_s, map ).release() );
    }
  }

  // second symbol layer
  {
    bool validFill = false, validStroke = false;

    // check for graphic fill
    QString name, format;
    int markIndex = -1;
    QColor fillColor, strokeColor;
    double strokeWidth = 1.0, size = 0.0, angle = 0.0;
    QPointF offset;

    // Fill element can contain a GraphicFill element
    const QDomElement graphicFillElem = fillElem.firstChildElement( u"GraphicFill"_s );
    if ( !graphicFillElem.isNull() )
    {
      // GraphicFill element must contain a Graphic element
      QDomElement graphicElem = graphicFillElem.firstChildElement( u"Graphic"_s );
      if ( !graphicElem.isNull() )
      {
        // Graphic element can contains some ExternalGraphic and Mark element
        // search for the first supported one and use it
        bool found = false;

        const QDomElement graphicChildElem = graphicElem.firstChildElement();
        while ( !graphicChildElem.isNull() )
        {
          if ( graphicChildElem.localName() == "Mark"_L1 )
          {
            // check for a well known name
            const QDomElement wellKnownNameElem = graphicChildElem.firstChildElement( u"WellKnownName"_s );
            if ( !wellKnownNameElem.isNull() )
            {
              name = wellKnownNameElem.firstChild().nodeValue();
              found = true;
              break;
            }
          }

          if ( graphicChildElem.localName() == "ExternalGraphic"_L1 || graphicChildElem.localName() == "Mark"_L1 )
          {
            // check for external graphic format
            const QDomElement formatElem = graphicChildElem.firstChildElement( u"Format"_s );
            if ( formatElem.isNull() )
              continue;

            format = formatElem.firstChild().nodeValue();

            // TODO: remove this check when more formats will be supported
            // only SVG external graphics are supported in this moment
            if ( graphicChildElem.localName() == "ExternalGraphic"_L1 && format != "image/svg+xml"_L1 )
              continue;

            // TODO: remove this check when more formats will be supported
            // only ttf marks are supported in this moment
            if ( graphicChildElem.localName() == "Mark"_L1 && format != "ttf"_L1 )
              continue;

            // check for a valid content
            const QDomElement onlineResourceElem = graphicChildElem.firstChildElement( u"OnlineResource"_s );
            const QDomElement inlineContentElem = graphicChildElem.firstChildElement( u"InlineContent"_s );

            if ( !onlineResourceElem.isNull() )
            {
              name = onlineResourceElem.attributeNS( u"http://www.w3.org/1999/xlink"_s, u"href"_s );

              if ( graphicChildElem.localName() == "Mark"_L1 && format == "ttf"_L1 )
              {
                // mark with ttf format may have a name like ttf://fontFamily
                if ( name.startsWith( "ttf://"_L1 ) )
                  name = name.mid( 6 );

                // mark with ttf format has a markIndex element
                const QDomElement markIndexElem = graphicChildElem.firstChildElement( u"MarkIndex"_s );
                if ( markIndexElem.isNull() )
                  continue;

                bool ok;
                const int v = markIndexElem.firstChild().nodeValue().toInt( &ok );
                if ( !ok || v < 0 )
                  continue;

                markIndex = v;
              }

              found = true;
              break;
            }
#if 0
            else if ( !inlineContentElem.isNull() )
              continue; // TODO: not implemented yet
#endif
            else
              continue;
          }

          // if Mark element is present but it doesn't contains neither
          // WellKnownName nor OnlineResource nor InlineContent,
          // use the default mark (square)
          if ( graphicChildElem.localName() == "Mark"_L1 )
          {
            name = u"square"_s;
            found = true;
            break;
          }
        }

        // if found a valid Mark, check for its Fill and Stroke element
        if ( found && graphicChildElem.localName() == "Mark"_L1 )
        {
          // XXX: recursive definition!?! couldn't be dangerous???
          // to avoid recursion we handle only simple fill and simple stroke

          // check for simple fill
          // Fill element can contain some SvgParameter elements
          Qt::BrushStyle markFillStyle;

          QDomElement markFillElem = graphicChildElem.firstChildElement( u"Fill"_s );
          if ( fillFromSld( markFillElem, markFillStyle, fillColor ) )
            validFill = true;

          // check for simple stroke
          // Stroke element can contain some SvgParameter elements
          Qt::PenStyle strokeStyle;
          double strokeWidth = 1.0, dashOffset = 0.0;
          QVector<qreal> customDashPattern;

          QDomElement markStrokeElem = graphicChildElem.firstChildElement( u"Stroke"_s );
          if ( lineFromSld( markStrokeElem, strokeStyle, strokeColor, strokeWidth,
                            nullptr, nullptr, &customDashPattern, &dashOffset ) )
            validStroke = true;
        }

        if ( found )
        {
          // check for Opacity, Size, Rotation, AnchorPoint, Displacement
          const QDomElement opacityElem = graphicElem.firstChildElement( u"Opacity"_s );
          if ( !opacityElem.isNull() )
            fillColor.setAlpha( decodeSldAlpha( opacityElem.firstChild().nodeValue() ) );

          const QDomElement sizeElem = graphicElem.firstChildElement( u"Size"_s );
          if ( !sizeElem.isNull() )
          {
            bool ok;
            const double v = sizeElem.firstChild().nodeValue().toDouble( &ok );
            if ( ok && v > 0 )
              size = v;
          }

          QString angleFunc;
          if ( rotationFromSldElement( graphicElem, angleFunc ) && !angleFunc.isEmpty() )
          {
            bool ok;
            const double v = angleFunc.toDouble( &ok );
            if ( ok )
              angle = v;
          }

          displacementFromSldElement( graphicElem, offset );
        }
      }
    }

    if ( validFill || validStroke )
    {
      if ( format == "image/svg+xml"_L1 )
      {
        QVariantMap map;
        map[u"name"_s] = name;
        map[u"fill"_s] = fillColor.name();
        map[u"outline"_s] = strokeColor.name();
        map[u"outline-width"_s] = QString::number( strokeWidth );
        if ( !qgsDoubleNear( size, 0.0 ) )
          map[u"size"_s] = QString::number( size );
        if ( !qgsDoubleNear( angle, 0.0 ) )
          map[u"angle"_s] = QString::number( angle );
        if ( !offset.isNull() )
          map[u"offset"_s] = encodePoint( offset );
        layers.append( QgsApplication::symbolLayerRegistry()->createSymbolLayer( u"SvgMarker"_s, map ).release() );
      }
      else if ( format == "ttf"_L1 )
      {
        QVariantMap map;
        map[u"font"_s] = name;
        map[u"chr"_s] = markIndex;
        map[u"color"_s] = QgsColorUtils::colorToString( validFill ? fillColor : Qt::transparent );
        if ( size > 0 )
          map[u"size"_s] = QString::number( size );
        if ( !qgsDoubleNear( angle, 0.0 ) )
          map[u"angle"_s] = QString::number( angle );
        if ( !offset.isNull() )
          map[u"offset"_s] = encodePoint( offset );
        layers.append( QgsApplication::symbolLayerRegistry()->createSymbolLayer( u"FontMarker"_s, map ).release() );
      }
    }
  }

  if ( layers.isEmpty() )
    return false;

  layerList << layers;
  layers.clear();
  return true;
}

void QgsSymbolLayerUtils::fillToSld( QDomDocument &doc, QDomElement &element, Qt::BrushStyle brushStyle, const QColor &color )
{
  QgsSldExportContext context;
  fillToSld( doc, element, context, brushStyle, color );
}

void QgsSymbolLayerUtils::fillToSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context, Qt::BrushStyle brushStyle, const QColor &color )
{
  QString patternName;
  switch ( brushStyle )
  {
    case Qt::NoBrush:
      return;

    case Qt::SolidPattern:
      if ( color.isValid() )
      {
        element.appendChild( createSvgParameterElement( doc, u"fill"_s, color.name() ) );
        if ( color.alpha() < 255 )
          element.appendChild( createSvgParameterElement( doc, u"fill-opacity"_s, encodeSldAlpha( color.alpha() ) ) );
      }
      return;

    case Qt::CrossPattern:
    case Qt::DiagCrossPattern:
    case Qt::HorPattern:
    case Qt::VerPattern:
    case Qt::BDiagPattern:
    case Qt::FDiagPattern:
    case Qt::Dense1Pattern:
    case Qt::Dense2Pattern:
    case Qt::Dense3Pattern:
    case Qt::Dense4Pattern:
    case Qt::Dense5Pattern:
    case Qt::Dense6Pattern:
    case Qt::Dense7Pattern:
      patternName = encodeSldBrushStyle( brushStyle );
      break;

    default:
      context.pushWarning( QObject::tr( "Brush style '%1' is not supported for SLD" ).arg( brushStyle ) );
      return;
  }

  QDomElement graphicFillElem = doc.createElement( u"se:GraphicFill"_s );
  element.appendChild( graphicFillElem );

  QDomElement graphicElem = doc.createElement( u"se:Graphic"_s );
  graphicFillElem.appendChild( graphicElem );

  const QColor fillColor = patternName.startsWith( "brush://"_L1 ) ? color : QColor();
  const QColor strokeColor = !patternName.startsWith( "brush://"_L1 ) ? color : QColor();

  /* Use WellKnownName tag to handle QT brush styles. */
  wellKnownMarkerToSld( doc, graphicElem, patternName, fillColor, strokeColor, Qt::SolidLine, context, -1, -1 );
}

bool QgsSymbolLayerUtils::fillFromSld( QDomElement &element, Qt::BrushStyle &brushStyle, QColor &color )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  brushStyle = Qt::SolidPattern;
  color = QColor( 128, 128, 128 );

  if ( element.isNull() )
  {
    brushStyle = Qt::NoBrush;
    color = QColor();
    return true;
  }

  const QDomElement graphicFillElem = element.firstChildElement( u"GraphicFill"_s );
  // if no GraphicFill element is found, it's a solid fill
  if ( graphicFillElem.isNull() )
  {
    QgsStringMap svgParams = getSvgParameterList( element );
    for ( QgsStringMap::iterator it = svgParams.begin(); it != svgParams.end(); ++it )
    {
      QgsDebugMsgLevel( u"found SvgParameter %1: %2"_s.arg( it.key(), it.value() ), 2 );

      if ( it.key() == "fill"_L1 )
        color = QColor( it.value() );
      else if ( it.key() == "fill-opacity"_L1 )
        color.setAlpha( decodeSldAlpha( it.value() ) );
    }
  }
  else  // wellKnown marker
  {
    QDomElement graphicElem = graphicFillElem.firstChildElement( u"Graphic"_s );
    if ( graphicElem.isNull() )
      return false; // Graphic is required within GraphicFill

    QString patternName = u"square"_s;
    QColor fillColor, strokeColor;
    double strokeWidth, size;
    Qt::PenStyle strokeStyle;
    if ( !wellKnownMarkerFromSld( graphicElem, patternName, fillColor, strokeColor, strokeStyle, strokeWidth, size ) )
      return false;

    brushStyle = decodeSldBrushStyle( patternName );
    if ( brushStyle == Qt::NoBrush )
      return false; // unable to decode brush style

    const QColor c = patternName.startsWith( "brush://"_L1 ) ? fillColor : strokeColor;
    if ( c.isValid() )
      color = c;
  }

  return true;
}

void QgsSymbolLayerUtils::lineToSld( QDomDocument &doc, QDomElement &element,
                                     Qt::PenStyle penStyle, const QColor &color, QgsSldExportContext &context, double width,
                                     const Qt::PenJoinStyle *penJoinStyle, const Qt::PenCapStyle *penCapStyle,
                                     const QVector<qreal> *customDashPattern, double dashOffset )
{
  QVector<qreal> dashPattern;
  const QVector<qreal> *pattern = &dashPattern;

  if ( penStyle == Qt::CustomDashLine && !customDashPattern )
  {
    context.pushWarning( QObject::tr( "WARNING: Custom dash pattern required but not provided. Using default dash pattern." ) );
    penStyle = Qt::DashLine;
  }

  switch ( penStyle )
  {
    case Qt::NoPen:
      return;

    case Qt::SolidLine:
      break;

    case Qt::DashLine:
      dashPattern.push_back( 4.0 );
      dashPattern.push_back( 2.0 );
      break;
    case Qt::DotLine:
      dashPattern.push_back( 1.0 );
      dashPattern.push_back( 2.0 );
      break;
    case Qt::DashDotLine:
      dashPattern.push_back( 4.0 );
      dashPattern.push_back( 2.0 );
      dashPattern.push_back( 1.0 );
      dashPattern.push_back( 2.0 );
      break;
    case Qt::DashDotDotLine:
      dashPattern.push_back( 4.0 );
      dashPattern.push_back( 2.0 );
      dashPattern.push_back( 1.0 );
      dashPattern.push_back( 2.0 );
      dashPattern.push_back( 1.0 );
      dashPattern.push_back( 2.0 );
      break;

    case Qt::CustomDashLine:
      Q_ASSERT( customDashPattern );
      pattern = customDashPattern;
      break;

    default:
      context.pushWarning( QObject::tr( "Pen style '%1' is not supported for SLD" ).arg( penStyle ) );
      return;
  }

  if ( color.isValid() )
  {
    element.appendChild( createSvgParameterElement( doc, u"stroke"_s, color.name() ) );
    if ( color.alpha() < 255 )
      element.appendChild( createSvgParameterElement( doc, u"stroke-opacity"_s, encodeSldAlpha( color.alpha() ) ) );
  }
  if ( width > 0 )
  {
    element.appendChild( createSvgParameterElement( doc, u"stroke-width"_s, qgsDoubleToString( width ) ) );
  }
  else if ( width == 0 )
  {
    // hairline, yet not zero. it's actually painted in qgis
    element.appendChild( createSvgParameterElement( doc, u"stroke-width"_s, u"0.5"_s ) );
  }
  if ( penJoinStyle )
    element.appendChild( createSvgParameterElement( doc, u"stroke-linejoin"_s, encodeSldLineJoinStyle( *penJoinStyle ) ) );
  if ( penCapStyle )
    element.appendChild( createSvgParameterElement( doc, u"stroke-linecap"_s, encodeSldLineCapStyle( *penCapStyle ) ) );

  if ( !pattern->isEmpty() )
  {
    element.appendChild( createSvgParameterElement( doc, u"stroke-dasharray"_s, encodeSldRealVector( *pattern ) ) );
    if ( !qgsDoubleNear( dashOffset, 0.0 ) )
      element.appendChild( createSvgParameterElement( doc, u"stroke-dashoffset"_s, qgsDoubleToString( dashOffset ) ) );
  }
}


bool QgsSymbolLayerUtils::lineFromSld( QDomElement &element,
                                       Qt::PenStyle &penStyle, QColor &color, double &width,
                                       Qt::PenJoinStyle *penJoinStyle, Qt::PenCapStyle *penCapStyle,
                                       QVector<qreal> *customDashPattern, double *dashOffset )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  penStyle = Qt::SolidLine;
  color = QColor( 0, 0, 0 );
  width = 1;
  if ( penJoinStyle )
    *penJoinStyle = Qt::BevelJoin;
  if ( penCapStyle )
    *penCapStyle = Qt::SquareCap;
  if ( customDashPattern )
    customDashPattern->clear();
  if ( dashOffset )
    *dashOffset = 0;

  if ( element.isNull() )
  {
    penStyle = Qt::NoPen;
    color = QColor();
    return true;
  }

  QgsStringMap svgParams = getSvgParameterList( element );
  for ( QgsStringMap::iterator it = svgParams.begin(); it != svgParams.end(); ++it )
  {
    QgsDebugMsgLevel( u"found SvgParameter %1: %2"_s.arg( it.key(), it.value() ), 2 );

    if ( it.key() == "stroke"_L1 )
    {
      color = QColor( it.value() );
    }
    else if ( it.key() == "stroke-opacity"_L1 )
    {
      color.setAlpha( decodeSldAlpha( it.value() ) );
    }
    else if ( it.key() == "stroke-width"_L1 )
    {
      bool ok;
      const double w = it.value().toDouble( &ok );
      if ( ok )
        width = w;
    }
    else if ( it.key() == "stroke-linejoin"_L1 && penJoinStyle )
    {
      *penJoinStyle = decodeSldLineJoinStyle( it.value() );
    }
    else if ( it.key() == "stroke-linecap"_L1 && penCapStyle )
    {
      *penCapStyle = decodeSldLineCapStyle( it.value() );
    }
    else if ( it.key() == "stroke-dasharray"_L1 )
    {
      const QVector<qreal> dashPattern = decodeSldRealVector( it.value() );
      if ( !dashPattern.isEmpty() )
      {
        // convert the dasharray to one of the QT pen style,
        // if no match is found then set pen style to CustomDashLine
        bool dashPatternFound = false;

        if ( dashPattern.count() == 2 )
        {
          if ( dashPattern.at( 0 ) == 4.0 &&
               dashPattern.at( 1 ) == 2.0 )
          {
            penStyle = Qt::DashLine;
            dashPatternFound = true;
          }
          else if ( dashPattern.at( 0 ) == 1.0 &&
                    dashPattern.at( 1 ) == 2.0 )
          {
            penStyle = Qt::DotLine;
            dashPatternFound = true;
          }
        }
        else if ( dashPattern.count() == 4 )
        {
          if ( dashPattern.at( 0 ) == 4.0 &&
               dashPattern.at( 1 ) == 2.0 &&
               dashPattern.at( 2 ) == 1.0 &&
               dashPattern.at( 3 ) == 2.0 )
          {
            penStyle = Qt::DashDotLine;
            dashPatternFound = true;
          }
        }
        else if ( dashPattern.count() == 6 )
        {
          if ( dashPattern.at( 0 ) == 4.0 &&
               dashPattern.at( 1 ) == 2.0 &&
               dashPattern.at( 2 ) == 1.0 &&
               dashPattern.at( 3 ) == 2.0 &&
               dashPattern.at( 4 ) == 1.0 &&
               dashPattern.at( 5 ) == 2.0 )
          {
            penStyle = Qt::DashDotDotLine;
            dashPatternFound = true;
          }
        }

        // default case: set pen style to CustomDashLine
        if ( !dashPatternFound )
        {
          if ( customDashPattern )
          {
            penStyle = Qt::CustomDashLine;
            *customDashPattern = dashPattern;
          }
          else
          {
            QgsDebugMsgLevel( u"custom dash pattern required but not provided. Using default dash pattern."_s, 2 );
            penStyle = Qt::DashLine;
          }
        }
      }
    }
    else if ( it.key() == "stroke-dashoffset"_L1 && dashOffset )
    {
      bool ok;
      const double d = it.value().toDouble( &ok );
      if ( ok )
        *dashOffset = d;
    }
  }

  return true;
}

void QgsSymbolLayerUtils::externalGraphicToSld( QDomDocument &doc, QDomElement &element,
    const QString &path, const QString &mime,
    const QColor &color, double size )
{
  QDomElement externalGraphicElem = doc.createElement( u"se:ExternalGraphic"_s );
  element.appendChild( externalGraphicElem );

  createOnlineResourceElement( doc, externalGraphicElem, path, mime );

  //TODO: missing a way to handle svg color. Should use <se:ColorReplacement>
  Q_UNUSED( color )

  if ( size >= 0 )
  {
    QDomElement sizeElem = doc.createElement( u"se:Size"_s );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    element.appendChild( sizeElem );
  }
}

void QgsSymbolLayerUtils::parametricSvgToSld( QDomDocument &doc, QDomElement &graphicElem,
    const QString &path, const QColor &fillColor, double size, const QColor &strokeColor, double strokeWidth )
{
  QgsSldExportContext context;
  parametricSvgToSld( doc, graphicElem, path, fillColor, size, strokeColor, strokeWidth, context );
}

void QgsSymbolLayerUtils::parametricSvgToSld( QDomDocument &doc, QDomElement &graphicElem, const QString &path, const QColor &fillColor, double size, const QColor &strokeColor, double strokeWidth, QgsSldExportContext &context )
{
  // Parametric SVG paths are an extension that few systems will understand, but se:Graphic allows for fallback
  // symbols, this encodes the full parametric path first, the pure shape second, and a mark with the right colors as
  // a last resort for systems that cannot do SVG at all

  // encode parametric version with all coloring details (size is going to be encoded by the last fallback)
  graphicElem.appendChild( doc.createComment( u"Parametric SVG"_s ) );
  const QString parametricPath = getSvgParametricPath( path, fillColor, strokeColor, strokeWidth );
  QgsSymbolLayerUtils::externalGraphicToSld( doc, graphicElem, parametricPath, u"image/svg+xml"_s, fillColor, -1 );
  // also encode a fallback version without parameters, in case a renderer gets confused by the parameters
  graphicElem.appendChild( doc.createComment( u"Plain SVG fallback, no parameters"_s ) );
  QgsSymbolLayerUtils::externalGraphicToSld( doc, graphicElem, path, u"image/svg+xml"_s, fillColor, -1 );
  // finally encode a simple mark with the right colors/outlines for renderers that cannot do SVG at all
  graphicElem.appendChild( doc.createComment( u"Well known marker fallback"_s ) );
  QgsSymbolLayerUtils::wellKnownMarkerToSld( doc, graphicElem, u"square"_s, fillColor, strokeColor, Qt::PenStyle::SolidLine, context, strokeWidth, -1 );

  // size is encoded here, it's part of se:Graphic, not attached to the single symbol
  if ( size >= 0 )
  {
    QDomElement sizeElem = doc.createElement( u"se:Size"_s );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    graphicElem.appendChild( sizeElem );
  }
}

QString QgsSymbolLayerUtils::getSvgParametricPath( const QString &basePath, const QColor &fillColor, const QColor &strokeColor, double strokeWidth )
{
  QUrlQuery url;
  if ( fillColor.isValid() )
  {
    url.addQueryItem( u"fill"_s, fillColor.name() );
    url.addQueryItem( u"fill-opacity"_s, encodeSldAlpha( fillColor.alpha() ) );
  }
  else
  {
    url.addQueryItem( u"fill"_s, u"#000000"_s );
    url.addQueryItem( u"fill-opacity"_s, u"1"_s );
  }
  if ( strokeColor.isValid() )
  {
    url.addQueryItem( u"outline"_s, strokeColor.name() );
    url.addQueryItem( u"outline-opacity"_s, encodeSldAlpha( strokeColor.alpha() ) );
  }
  else
  {
    url.addQueryItem( u"outline"_s, u"#000000"_s );
    url.addQueryItem( u"outline-opacity"_s, u"1"_s );
  }
  url.addQueryItem( u"outline-width"_s, QString::number( strokeWidth ) );
  const QString params = url.toString( QUrl::FullyEncoded );
  if ( params.isEmpty() )
  {
    return basePath;
  }
  else
  {
    return basePath + "?" + params;
  }
}

bool QgsSymbolLayerUtils::externalGraphicFromSld( QDomElement &element,
    QString &path, QString &mime,
    QColor &color, double &size )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );
  Q_UNUSED( color )

  QDomElement externalGraphicElem = element.firstChildElement( u"ExternalGraphic"_s );
  if ( externalGraphicElem.isNull() )
    return false;

  onlineResourceFromSldElement( externalGraphicElem, path, mime );

  const QDomElement sizeElem = element.firstChildElement( u"Size"_s );
  if ( !sizeElem.isNull() )
  {
    bool ok;
    const double s = sizeElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      size = s;
  }

  return true;
}

void QgsSymbolLayerUtils::externalMarkerToSld( QDomDocument &doc, QDomElement &element,
    const QString &path, const QString &format, int *markIndex,
    const QColor &color, double size )
{
  QgsSldExportContext context;
  externalMarkerToSld( doc, element, path, format, context, markIndex, color, size );
}

void QgsSymbolLayerUtils::externalMarkerToSld( QDomDocument &doc, QDomElement &element, const QString &path, const QString &format, QgsSldExportContext &context, int *markIndex, const QColor &color, double size )
{
  QDomElement markElem = doc.createElement( u"se:Mark"_s );
  element.appendChild( markElem );

  createOnlineResourceElement( doc, markElem, path, format );

  if ( markIndex )
  {
    QDomElement markIndexElem = doc.createElement( u"se:MarkIndex"_s );
    markIndexElem.appendChild( doc.createTextNode( QString::number( *markIndex ) ) );
    markElem.appendChild( markIndexElem );
  }

  // <Fill>
  QDomElement fillElem = doc.createElement( u"se:Fill"_s );
  fillToSld( doc, fillElem, context, Qt::SolidPattern, color );
  markElem.appendChild( fillElem );

  // <Size>
  if ( !qgsDoubleNear( size, 0.0 ) && size > 0 )
  {
    QDomElement sizeElem = doc.createElement( u"se:Size"_s );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    element.appendChild( sizeElem );
  }
}

bool QgsSymbolLayerUtils::externalMarkerFromSld( QDomElement &element,
    QString &path, QString &format, int &markIndex,
    QColor &color, double &size )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  color = QColor();
  markIndex = -1;
  size = -1;

  QDomElement markElem = element.firstChildElement( u"Mark"_s );
  if ( markElem.isNull() )
    return false;

  onlineResourceFromSldElement( markElem, path, format );

  const QDomElement markIndexElem = markElem.firstChildElement( u"MarkIndex"_s );
  if ( !markIndexElem.isNull() )
  {
    bool ok;
    const int i = markIndexElem.firstChild().nodeValue().toInt( &ok );
    if ( ok )
      markIndex = i;
  }

  // <Fill>
  QDomElement fillElem = markElem.firstChildElement( u"Fill"_s );
  Qt::BrushStyle b = Qt::SolidPattern;
  fillFromSld( fillElem, b, color );
  // ignore brush style, solid expected

  // <Size>
  const QDomElement sizeElem = element.firstChildElement( u"Size"_s );
  if ( !sizeElem.isNull() )
  {
    bool ok;
    const double s = sizeElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      size = s;
  }

  return true;
}

void QgsSymbolLayerUtils::wellKnownMarkerToSld( QDomDocument &doc, QDomElement &element,
    const QString &name, const QColor &color, const QColor &strokeColor, Qt::PenStyle strokeStyle,
    double strokeWidth, double size )
{
  QgsSldExportContext context;
  wellKnownMarkerToSld( doc, element, name, color, strokeColor, strokeStyle, context, strokeWidth, size );
}

void QgsSymbolLayerUtils::wellKnownMarkerToSld( QDomDocument &doc, QDomElement &element, const QString &name, const QColor &color, const QColor &strokeColor, Qt::PenStyle strokeStyle, QgsSldExportContext &context, double strokeWidth, double size )
{
  QDomElement markElem = doc.createElement( u"se:Mark"_s );
  element.appendChild( markElem );

  QDomElement wellKnownNameElem = doc.createElement( u"se:WellKnownName"_s );
  wellKnownNameElem.appendChild( doc.createTextNode( name ) );
  markElem.appendChild( wellKnownNameElem );

  // <Fill>
  if ( color.isValid() )
  {
    QDomElement fillElem = doc.createElement( u"se:Fill"_s );
    fillToSld( doc, fillElem, context, Qt::SolidPattern, color );
    markElem.appendChild( fillElem );
  }

  // <Stroke>
  if ( strokeColor.isValid() )
  {
    QDomElement strokeElem = doc.createElement( u"se:Stroke"_s );
    lineToSld( doc, strokeElem, strokeStyle, strokeColor, context, strokeWidth );
    markElem.appendChild( strokeElem );
  }

  // <Size>
  if ( !qgsDoubleNear( size, 0.0 ) && size > 0 )
  {
    QDomElement sizeElem = doc.createElement( u"se:Size"_s );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    element.appendChild( sizeElem );
  }
}

bool QgsSymbolLayerUtils::wellKnownMarkerFromSld( QDomElement &element,
    QString &name, QColor &color, QColor &strokeColor, Qt::PenStyle &strokeStyle,
    double &strokeWidth, double &size )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  name = u"square"_s;
  color = QColor();
  strokeColor = QColor( 0, 0, 0 );
  strokeWidth = 1;
  size = 6;

  const QDomElement markElem = element.firstChildElement( u"Mark"_s );
  if ( markElem.isNull() )
    return false;

  const QDomElement wellKnownNameElem = markElem.firstChildElement( u"WellKnownName"_s );
  if ( !wellKnownNameElem.isNull() )
  {
    name = wellKnownNameElem.firstChild().nodeValue();
    QgsDebugMsgLevel( "found Mark with well known name: " + name, 2 );
  }

  // <Fill>
  QDomElement fillElem = markElem.firstChildElement( u"Fill"_s );
  Qt::BrushStyle b = Qt::SolidPattern;
  fillFromSld( fillElem, b, color );
  // ignore brush style, solid expected

  // <Stroke>
  QDomElement strokeElem = markElem.firstChildElement( u"Stroke"_s );
  lineFromSld( strokeElem, strokeStyle, strokeColor, strokeWidth );
  // ignore stroke style, solid expected

  // <Size>
  const QDomElement sizeElem = element.firstChildElement( u"Size"_s );
  if ( !sizeElem.isNull() )
  {
    bool ok;
    const double s = sizeElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      size = s;
  }

  return true;
}

void QgsSymbolLayerUtils::createRotationElement( QDomDocument &doc, QDomElement &element, const QString &rotationFunc )
{
  QgsSldExportContext context;
  createRotationElement( doc, element, rotationFunc, context );
}

void QgsSymbolLayerUtils::createRotationElement( QDomDocument &doc, QDomElement &element, const QString &rotationFunc, QgsSldExportContext &context )
{
  if ( !rotationFunc.isEmpty() )
  {
    QDomElement rotationElem = doc.createElement( u"se:Rotation"_s );
    createExpressionElement( doc, rotationElem, rotationFunc, context );
    element.appendChild( rotationElem );
  }
}

bool QgsSymbolLayerUtils::rotationFromSldElement( QDomElement &element, QString &rotationFunc )
{
  QDomElement rotationElem = element.firstChildElement( u"Rotation"_s );
  if ( !rotationElem.isNull() )
  {
    return functionFromSldElement( rotationElem, rotationFunc );
  }
  return true;
}

void QgsSymbolLayerUtils::createOpacityElement( QDomDocument &doc, QDomElement &element, const QString &alphaFunc )
{
  QgsSldExportContext context;
  createOpacityElement( doc, element, alphaFunc, context );
}

void QgsSymbolLayerUtils::createOpacityElement( QDomDocument &doc, QDomElement &element, const QString &alphaFunc, QgsSldExportContext &context )
{
  if ( !alphaFunc.isEmpty() )
  {
    QDomElement opacityElem = doc.createElement( u"se:Opacity"_s );
    createExpressionElement( doc, opacityElem, alphaFunc, context );
    element.appendChild( opacityElem );
  }
}

bool QgsSymbolLayerUtils::opacityFromSldElement( QDomElement &element, QString &alphaFunc )
{
  QDomElement opacityElem = element.firstChildElement( u"Opacity"_s );
  if ( !opacityElem.isNull() )
  {
    return functionFromSldElement( opacityElem, alphaFunc );
  }
  return true;
}

void QgsSymbolLayerUtils::createDisplacementElement( QDomDocument &doc, QDomElement &element, QPointF offset )
{
  if ( offset.isNull() )
    return;

  QDomElement displacementElem = doc.createElement( u"se:Displacement"_s );
  element.appendChild( displacementElem );

  QDomElement dispXElem = doc.createElement( u"se:DisplacementX"_s );
  dispXElem.appendChild( doc.createTextNode( qgsDoubleToString( offset.x(), 2 ) ) );

  QDomElement dispYElem = doc.createElement( u"se:DisplacementY"_s );
  dispYElem.appendChild( doc.createTextNode( qgsDoubleToString( offset.y(), 2 ) ) );

  displacementElem.appendChild( dispXElem );
  displacementElem.appendChild( dispYElem );
}

void QgsSymbolLayerUtils::createAnchorPointElement( QDomDocument &doc, QDomElement &element, QPointF anchor )
{
  // anchor is not tested for null, (0,0) is _not_ the default value (0.5, 0) is.

  QDomElement anchorElem = doc.createElement( u"se:AnchorPoint"_s );
  element.appendChild( anchorElem );

  QDomElement anchorXElem = doc.createElement( u"se:AnchorPointX"_s );
  anchorXElem.appendChild( doc.createTextNode( qgsDoubleToString( anchor.x() ) ) );

  QDomElement anchorYElem = doc.createElement( u"se:AnchorPointY"_s );
  anchorYElem.appendChild( doc.createTextNode( qgsDoubleToString( anchor.y() ) ) );

  anchorElem.appendChild( anchorXElem );
  anchorElem.appendChild( anchorYElem );
}

bool QgsSymbolLayerUtils::displacementFromSldElement( QDomElement &element, QPointF &offset )
{
  offset = QPointF( 0, 0 );

  const QDomElement displacementElem = element.firstChildElement( u"Displacement"_s );
  if ( displacementElem.isNull() )
    return true;

  const QDomElement dispXElem = displacementElem.firstChildElement( u"DisplacementX"_s );
  if ( !dispXElem.isNull() )
  {
    bool ok;
    const double offsetX = dispXElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset.setX( offsetX );
  }

  const QDomElement dispYElem = displacementElem.firstChildElement( u"DisplacementY"_s );
  if ( !dispYElem.isNull() )
  {
    bool ok;
    const double offsetY = dispYElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset.setY( offsetY );
  }

  return true;
}

void QgsSymbolLayerUtils::labelTextToSld( QDomDocument &doc, QDomElement &element,
    const QString &label, const QFont &font,
    const QColor &color, double size )
{
  QgsSldExportContext context;
  labelTextToSld( doc, element, label, font, context, color, size );
}

void QgsSymbolLayerUtils::labelTextToSld( QDomDocument &doc, QDomElement &element, const QString &label, const QFont &font, QgsSldExportContext &context, const QColor &color, double size )
{
  QDomElement labelElem = doc.createElement( u"se:Label"_s );
  labelElem.appendChild( doc.createTextNode( label ) );
  element.appendChild( labelElem );

  QDomElement fontElem = doc.createElement( u"se:Font"_s );
  element.appendChild( fontElem );

  fontElem.appendChild( createSvgParameterElement( doc, u"font-family"_s, font.family() ) );
#if 0
  fontElem.appendChild( createSldParameterElement( doc, "font-style", encodeSldFontStyle( font.style() ) ) );
  fontElem.appendChild( createSldParameterElement( doc, "font-weight", encodeSldFontWeight( font.weight() ) ) );
#endif
  fontElem.appendChild( createSvgParameterElement( doc, u"font-size"_s, QString::number( size ) ) );

  // <Fill>
  if ( color.isValid() )
  {
    QDomElement fillElem = doc.createElement( u"Fill"_s );
    fillToSld( doc, fillElem, context, Qt::SolidPattern, color );
    element.appendChild( fillElem );
  }
}

QString QgsSymbolLayerUtils::ogrFeatureStylePen( double width, double mmScaleFactor, double mapUnitScaleFactor, const QColor &c,
    Qt::PenJoinStyle joinStyle,
    Qt::PenCapStyle capStyle,
    double offset,
    const QVector<qreal> *dashPattern )
{
  QString penStyle;
  penStyle.append( "PEN(" );
  penStyle.append( "c:" );
  penStyle.append( c.name() );
  penStyle.append( ",w:" );
  //dxf driver writes ground units as mm? Should probably be changed in ogr
  penStyle.append( QString::number( width * mmScaleFactor ) );
  penStyle.append( "mm" );

  //dash dot vector
  if ( dashPattern && !dashPattern->isEmpty() )
  {
    penStyle.append( ",p:\"" );
    QVector<qreal>::const_iterator pIt = dashPattern->constBegin();
    for ( ; pIt != dashPattern->constEnd(); ++pIt )
    {
      if ( pIt != dashPattern->constBegin() )
      {
        penStyle.append( ' ' );
      }
      penStyle.append( QString::number( *pIt * mapUnitScaleFactor ) );
      penStyle.append( 'g' );
    }
    penStyle.append( '\"' );
  }

  //cap
  penStyle.append( ",cap:" );
  switch ( capStyle )
  {
    case Qt::SquareCap:
      penStyle.append( 'p' );
      break;
    case Qt::RoundCap:
      penStyle.append( 'r' );
      break;
    case Qt::FlatCap:
    default:
      penStyle.append( 'b' );
  }

  //join
  penStyle.append( ",j:" );
  switch ( joinStyle )
  {
    case Qt::BevelJoin:
      penStyle.append( 'b' );
      break;
    case Qt::RoundJoin:
      penStyle.append( 'r' );
      break;
    case Qt::MiterJoin:
    default:
      penStyle.append( 'm' );
  }

  //offset
  if ( !qgsDoubleNear( offset, 0.0 ) )
  {
    penStyle.append( ",dp:" );
    penStyle.append( QString::number( offset * mapUnitScaleFactor ) );
    penStyle.append( 'g' );
  }

  penStyle.append( ')' );
  return penStyle;
}

QString QgsSymbolLayerUtils::ogrFeatureStyleBrush( const QColor &fillColor )
{
  QString brushStyle;
  brushStyle.append( "BRUSH(" );
  brushStyle.append( "fc:" );
  brushStyle.append( fillColor.name() );
  brushStyle.append( ')' );
  return brushStyle;
}

void QgsSymbolLayerUtils::createGeometryElement( QDomDocument &doc, QDomElement &element, const QString &geomFunc )
{
  QgsSldExportContext context;
  createGeometryElement( doc, element, geomFunc, context );
}

void QgsSymbolLayerUtils::createGeometryElement( QDomDocument &doc, QDomElement &element, const QString &geomFunc, QgsSldExportContext &context )
{
  if ( geomFunc.isEmpty() )
    return;

  QDomElement geometryElem = doc.createElement( u"Geometry"_s );
  element.appendChild( geometryElem );

  /* About using a function within the Geometry tag.
   *
   * The SLD specification <= 1.1 is vague:
   * "In principle, a fixed geometry could be defined using GML or
   * operators could be defined for computing the geometry from
   * references or literals. However, using a feature property directly
   * is by far the most commonly useful method."
   *
   * Even if it seems that specs should take care all the possible cases,
   * looking at the XML schema fragment that encodes the Geometry element,
   * it has to be a PropertyName element:
   *   <xsd:element name="Geometry">
   *       <xsd:complexType>
   *           <xsd:sequence>
   *               <xsd:element ref="ogc:PropertyName"/>
   *           </xsd:sequence>
   *       </xsd:complexType>
   *   </xsd:element>
   *
   * Anyway we will use a ogc:Function to handle geometry transformations
   * like offset, centroid, ...
   */

  createExpressionElement( doc, geometryElem, geomFunc, context );
}

bool QgsSymbolLayerUtils::geometryFromSldElement( QDomElement &element, QString &geomFunc )
{
  QDomElement geometryElem = element.firstChildElement( u"Geometry"_s );
  if ( geometryElem.isNull() )
    return true;

  return functionFromSldElement( geometryElem, geomFunc );
}

bool QgsSymbolLayerUtils::createExpressionElement( QDomDocument &doc, QDomElement &element, const QString &function )
{
  QgsSldExportContext context;
  return createExpressionElement( doc, element, function, context );
}

bool QgsSymbolLayerUtils::createExpressionElement( QDomDocument &doc, QDomElement &element, const QString &function, QgsSldExportContext &context )
{
  // let's use QgsExpression to generate the SLD for the function
  const QgsExpression expr( function );
  if ( expr.hasParserError() )
  {
    context.pushError( QObject::tr( "Parser error encountered when converting expression to SLD: %1 - Expression was: %2" ).arg( expr.parserErrorString(), function ) );
    return false;
  }
  const QDomElement filterElem = QgsOgcUtils::expressionToOgcExpression( expr, doc );
  if ( !filterElem.isNull() )
    element.appendChild( filterElem );
  return true;
}

bool QgsSymbolLayerUtils::createFunctionElement( QDomDocument &doc, QDomElement &element, const QString &function )
{
  QgsSldExportContext context;
  return createFunctionElement( doc, element, function, context );
}

bool QgsSymbolLayerUtils::createFunctionElement( QDomDocument &doc, QDomElement &element, const QString &function, QgsSldExportContext &context )
{
  // else rule is not a valid expression
  if ( function == "ELSE"_L1 )
  {
    const QDomElement filterElem = QgsOgcUtils::elseFilterExpression( doc );
    element.appendChild( filterElem );
    return true;
  }
  else
  {
    // let's use QgsExpression to generate the SLD for the function
    const QgsExpression expr( function );
    if ( expr.hasParserError() )
    {
      context.pushError( QObject::tr( "Parser error encountered when converting expression to SLD: %1 - Expression was: %2" ).arg( expr.parserErrorString(), function ) );
      return false;
    }
    const QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( expr, doc );
    if ( !filterElem.isNull() )
      element.appendChild( filterElem );
    return true;
  }
}

bool QgsSymbolLayerUtils::functionFromSldElement( QDomElement &element, QString &function )
{
  // check if ogc:Filter or contains ogc:Filters
  QDomElement elem = element;
  if ( element.tagName() != "Filter"_L1 )
  {
    const QDomNodeList filterNodes = element.elementsByTagName( u"Filter"_s );
    if ( !filterNodes.isEmpty() )
    {
      elem = filterNodes.at( 0 ).toElement();
    }
  }

  if ( elem.isNull() )
  {
    return false;
  }

  // parse ogc:Filter
  QgsExpression *expr = QgsOgcUtils::expressionFromOgcFilter( elem );
  if ( !expr )
    return false;

  const bool valid = !expr->hasParserError();
  if ( !valid )
  {
    QgsDebugError( "parser error: " + expr->parserErrorString() );
  }
  else
  {
    function = expr->expression();
  }

  delete expr;
  return valid;
}

void QgsSymbolLayerUtils::createOnlineResourceElement( QDomDocument &doc, QDomElement &element,
    const QString &path, const QString &format )
{
  // get resource url or relative path
  const QString url = svgSymbolPathToName( path, QgsPathResolver() );
  QDomElement onlineResourceElem = doc.createElement( u"se:OnlineResource"_s );
  onlineResourceElem.setAttribute( u"xlink:type"_s, u"simple"_s );
  onlineResourceElem.setAttribute( u"xlink:href"_s, url );
  element.appendChild( onlineResourceElem );

  QDomElement formatElem = doc.createElement( u"se:Format"_s );
  formatElem.appendChild( doc.createTextNode( format ) );
  element.appendChild( formatElem );
}

bool QgsSymbolLayerUtils::onlineResourceFromSldElement( QDomElement &element, QString &path, QString &format )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  const QDomElement onlineResourceElem = element.firstChildElement( u"OnlineResource"_s );
  if ( onlineResourceElem.isNull() )
    return false;

  path = QUrl::fromPercentEncoding( onlineResourceElem.attributeNS( u"http://www.w3.org/1999/xlink"_s, u"href"_s ).toUtf8() );

  const QDomElement formatElem = element.firstChildElement( u"Format"_s );
  if ( formatElem.isNull() )
    return false; // OnlineResource requires a Format sibling element

  format = formatElem.firstChild().nodeValue();
  return true;
}


QDomElement QgsSymbolLayerUtils::createSvgParameterElement( QDomDocument &doc, const QString &name, const QString &value )
{
  QDomElement nodeElem = doc.createElement( u"se:SvgParameter"_s );
  nodeElem.setAttribute( u"name"_s, name );
  nodeElem.appendChild( doc.createTextNode( value ) );
  return nodeElem;
}

QgsStringMap QgsSymbolLayerUtils::getSvgParameterList( QDomElement &element )
{
  QgsStringMap params;
  QString value;

  QDomElement paramElem = element.firstChildElement();
  while ( !paramElem.isNull() )
  {
    if ( paramElem.localName() == "SvgParameter"_L1 || paramElem.localName() == "CssParameter"_L1 )
    {
      const QString name = paramElem.attribute( u"name"_s );
      if ( paramElem.firstChild().nodeType() == QDomNode::TextNode )
      {
        value = paramElem.firstChild().nodeValue();
      }
      else
      {
        if ( paramElem.firstChild().nodeType() == QDomNode::ElementNode &&
             paramElem.firstChild().localName() == "Literal"_L1 )
        {
          QgsDebugMsgLevel( paramElem.firstChild().localName(), 3 );
          value = paramElem.firstChild().firstChild().nodeValue();
        }
        else
        {
          QgsDebugError( u"unexpected child of %1"_s.arg( paramElem.localName() ) );
        }
      }

      if ( !name.isEmpty() && !value.isEmpty() )
        params[ name ] = value;
    }

    paramElem = paramElem.nextSiblingElement();
  }

  return params;
}

QDomElement QgsSymbolLayerUtils::createVendorOptionElement( QDomDocument &doc, const QString &name, const QString &value )
{
  QDomElement nodeElem = doc.createElement( u"se:VendorOption"_s );
  nodeElem.setAttribute( u"name"_s, name );
  nodeElem.appendChild( doc.createTextNode( value ) );
  return nodeElem;
}

QgsStringMap QgsSymbolLayerUtils::getVendorOptionList( QDomElement &element )
{
  QgsStringMap params;

  QDomElement paramElem = element.firstChildElement( u"VendorOption"_s );
  while ( !paramElem.isNull() )
  {
    const QString name = paramElem.attribute( u"name"_s );
    const QString value = paramElem.firstChild().nodeValue();

    if ( !name.isEmpty() && !value.isEmpty() )
      params[ name ] = value;

    paramElem = paramElem.nextSiblingElement( u"VendorOption"_s );
  }

  return params;
}


QVariantMap QgsSymbolLayerUtils::parseProperties( const QDomElement &element )
{
  const QVariant newSymbols = QgsXmlUtils::readVariant( element.firstChildElement( u"Option"_s ) );
  if ( newSymbols.userType() == QMetaType::Type::QVariantMap )
  {
    return newSymbols.toMap();
  }
  else
  {
    // read old style of writing properties
    // backward compatibility with project saved in <= 3.16
    QVariantMap props;
    QDomElement e = element.firstChildElement();
    while ( !e.isNull() )
    {
      if ( e.tagName() == "prop"_L1 )
      {
        const QString propKey = e.attribute( u"k"_s );
        const QString propValue = e.attribute( u"v"_s );
        props[propKey] = propValue;
      }
      e = e.nextSiblingElement();
    }
    return props;
  }
}


void QgsSymbolLayerUtils::saveProperties( QVariantMap props, QDomDocument &doc, QDomElement &element )
{
  element.appendChild( QgsXmlUtils::writeVariant( props, doc ) );
}

QgsSymbolMap QgsSymbolLayerUtils::loadSymbols( QDomElement &element, const QgsReadWriteContext &context )
{
  // go through symbols one-by-one and load them

  QgsSymbolMap symbols;
  QDomElement e = element.firstChildElement();

  while ( !e.isNull() )
  {
    if ( e.tagName() == "symbol"_L1 )
    {
      std::unique_ptr< QgsSymbol > symbol = QgsSymbolLayerUtils::loadSymbol( e, context );
      if ( symbol )
        symbols.insert( e.attribute( u"name"_s ), symbol.release() );
    }
    else
    {
      QgsDebugError( "unknown tag: " + e.tagName() );
    }
    e = e.nextSiblingElement();
  }


  // now walk through the list of symbols and find those prefixed with @
  // these symbols are sub-symbols of some other symbol layers
  // e.g. symbol named "@foo@1" is sub-symbol of layer 1 in symbol "foo"
  QStringList subsymbols;

  for ( QMap<QString, QgsSymbol *>::iterator it = symbols.begin(); it != symbols.end(); ++it )
  {
    if ( it.key()[0] != '@' )
      continue;

    // add to array (for deletion)
    subsymbols.append( it.key() );

    QStringList parts = it.key().split( '@' );
    if ( parts.count() < 3 )
    {
      QgsDebugError( "found subsymbol with invalid name: " + it.key() );
      delete it.value(); // we must delete it
      continue; // some invalid syntax
    }
    const QString symname = parts[1];
    const int symlayer = parts[2].toInt();

    if ( !symbols.contains( symname ) )
    {
      QgsDebugError( "subsymbol references invalid symbol: " + symname );
      delete it.value(); // we must delete it
      continue;
    }

    QgsSymbol *sym = symbols[symname];
    if ( symlayer < 0 || symlayer >= sym->symbolLayerCount() )
    {
      QgsDebugError( "subsymbol references invalid symbol layer: " + QString::number( symlayer ) );
      delete it.value(); // we must delete it
      continue;
    }

    // set subsymbol takes ownership
    const bool res = sym->symbolLayer( symlayer )->setSubSymbol( it.value() );
    if ( !res )
    {
      QgsDebugError( "symbol layer refused subsymbol: " + it.key() );
    }


  }

  // now safely remove sub-symbol entries (they have been already deleted or the ownership was taken away)
  for ( int i = 0; i < subsymbols.count(); i++ )
    symbols.take( subsymbols[i] );

  return symbols;
}

QDomElement QgsSymbolLayerUtils::saveSymbols( QgsSymbolMap &symbols, const QString &tagName, QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement symbolsElem = doc.createElement( tagName );

  // save symbols
  for ( QMap<QString, QgsSymbol *>::iterator its = symbols.begin(); its != symbols.end(); ++its )
  {
    const QDomElement symEl = saveSymbol( its.key(), its.value(), doc, context );
    symbolsElem.appendChild( symEl );
  }

  return symbolsElem;
}

void QgsSymbolLayerUtils::clearSymbolMap( QgsSymbolMap &symbols )
{
  qDeleteAll( symbols );
  symbols.clear();
}

QMimeData *QgsSymbolLayerUtils::symbolToMimeData( const QgsSymbol *symbol )
{
  if ( !symbol )
    return nullptr;

  std::unique_ptr< QMimeData >mimeData( new QMimeData );

  QDomDocument symbolDoc;
  const QDomElement symbolElem = saveSymbol( u"symbol"_s, symbol, symbolDoc, QgsReadWriteContext() );
  symbolDoc.appendChild( symbolElem );
  mimeData->setText( symbolDoc.toString() );

  mimeData->setImageData( symbolPreviewPixmap( symbol, QSize( 100, 100 ), 18 ).toImage() );
  mimeData->setColorData( symbol->color() );

  return mimeData.release();
}

std::unique_ptr< QgsSymbol > QgsSymbolLayerUtils::symbolFromMimeData( const QMimeData *data )
{
  if ( !data )
    return nullptr;

  const QString text = data->text();
  if ( !text.isEmpty() )
  {
    QDomDocument doc;
    QDomElement elem;

    if ( doc.setContent( text ) )
    {
      elem = doc.documentElement();

      if ( elem.nodeName() != "symbol"_L1 )
        elem = elem.firstChildElement( u"symbol"_s );

      return loadSymbol( elem, QgsReadWriteContext() );
    }
  }
  return nullptr;
}


std::unique_ptr< QgsColorRamp > QgsSymbolLayerUtils::loadColorRamp( QDomElement &element )
{
  const QString rampType = element.attribute( u"type"_s );

  // parse properties
  const QVariantMap props = QgsSymbolLayerUtils::parseProperties( element );

  if ( rampType == QgsGradientColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsGradientColorRamp::create( props ) );
  else if ( rampType == QgsLimitedRandomColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsLimitedRandomColorRamp::create( props ) );
  else if ( rampType == QgsColorBrewerColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsColorBrewerColorRamp::create( props ) );
  else if ( rampType == QgsCptCityColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsCptCityColorRamp::create( props ) );
  else if ( rampType == QgsPresetSchemeColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsPresetSchemeColorRamp::create( props ) );
  else
  {
    QgsDebugError( "unknown colorramp type " + rampType );
    return nullptr;
  }
}


QDomElement QgsSymbolLayerUtils::saveColorRamp( const QString &name, const QgsColorRamp *ramp, QDomDocument &doc )
{
  QDomElement rampEl = doc.createElement( u"colorramp"_s );
  rampEl.setAttribute( u"type"_s, ramp->type() );
  rampEl.setAttribute( u"name"_s, name );

  QgsSymbolLayerUtils::saveProperties( ramp->properties(), doc, rampEl );
  return rampEl;
}

QVariant QgsSymbolLayerUtils::colorRampToVariant( const QString &name, QgsColorRamp *ramp )
{
  QVariantMap rampMap;

  rampMap.insert( u"type"_s, ramp->type() );
  rampMap.insert( u"name"_s, name );

  const QVariantMap properties = ramp->properties();

  QVariantMap propertyMap;
  for ( auto property = properties.constBegin(); property != properties.constEnd(); ++property )
  {
    propertyMap.insert( property.key(), property.value() );
  }

  rampMap.insert( u"properties"_s, propertyMap );
  return rampMap;
}

std::unique_ptr< QgsColorRamp > QgsSymbolLayerUtils::loadColorRamp( const QVariant &value )
{
  const QVariantMap rampMap = value.toMap();

  const QString rampType = rampMap.value( u"type"_s ).toString();

  // parse properties
  const QVariantMap propertyMap = rampMap.value( u"properties"_s ).toMap();
  QVariantMap props;

  for ( auto property = propertyMap.constBegin(); property != propertyMap.constEnd(); ++property )
  {
    props.insert( property.key(), property.value().toString() );
  }

  if ( rampType == QgsGradientColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsGradientColorRamp::create( props ) );
  else if ( rampType == QgsLimitedRandomColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsLimitedRandomColorRamp::create( props ) );
  else if ( rampType == QgsColorBrewerColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsColorBrewerColorRamp::create( props ) );
  else if ( rampType == QgsCptCityColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsCptCityColorRamp::create( props ) );
  else if ( rampType == QgsPresetSchemeColorRamp::typeString() )
    return std::unique_ptr< QgsColorRamp >( QgsPresetSchemeColorRamp::create( props ) );
  else
  {
    QgsDebugError( "unknown colorramp type " + rampType );
    return nullptr;
  }
}

QString QgsSymbolLayerUtils::colorToName( const QColor &color )
{
  if ( !color.isValid() )
  {
    return QString();
  }

  //TODO - utilize a color names database (such as X11) to return nicer names
  //for now, just return hex codes
  return color.name();
}

QList<QColor> QgsSymbolLayerUtils::parseColorList( const QString &colorStr )
{
  QList<QColor> colors;

  //try splitting string at commas, spaces or newlines
  const thread_local QRegularExpression sepCommaSpaceRegExp( "(,|\\s)" );
  QStringList components = colorStr.simplified().split( sepCommaSpaceRegExp );
  QStringList::iterator it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    const QColor result = parseColor( *it, true );
    if ( result.isValid() )
    {
      colors << result;
    }
  }
  if ( colors.length() > 0 )
  {
    return colors;
  }

  //try splitting string at commas or newlines
  const thread_local QRegularExpression sepCommaRegExp( "(,|\n)" );
  components = colorStr.split( sepCommaRegExp );
  it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    const QColor result = parseColor( *it, true );
    if ( result.isValid() )
    {
      colors << result;
    }
  }
  if ( colors.length() > 0 )
  {
    return colors;
  }

  //try splitting string at whitespace or newlines
  components = colorStr.simplified().split( QString( ' ' ) );
  it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    const QColor result = parseColor( *it, true );
    if ( result.isValid() )
    {
      colors << result;
    }
  }
  if ( colors.length() > 0 )
  {
    return colors;
  }

  //try splitting string just at newlines
  components = colorStr.split( '\n' );
  it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    const QColor result = parseColor( *it, true );
    if ( result.isValid() )
    {
      colors << result;
    }
  }

  return colors;
}

QMimeData *QgsSymbolLayerUtils::colorToMimeData( const QColor &color )
{
  //set both the mime color data (which includes alpha channel), and the text (which is the color's hex
  //value, and can be used when pasting colors outside of QGIS).
  QMimeData *mimeData = new QMimeData;
  mimeData->setColorData( QVariant( color ) );
  mimeData->setText( color.name() );
  return mimeData;
}

QColor QgsSymbolLayerUtils::colorFromMimeData( const QMimeData *mimeData, bool &hasAlpha )
{
  if ( !mimeData )
    return QColor();

  //attempt to read color data directly from mime
  if ( mimeData->hasColor() )
  {
    QColor mimeColor = mimeData->colorData().value<QColor>();
    if ( mimeColor.isValid() )
    {
      hasAlpha = true;
      return mimeColor;
    }
  }

  //attempt to intrepret a color from mime text data
  if ( mimeData->hasText() )
  {
    hasAlpha = false;
    QColor textColor = QgsSymbolLayerUtils::parseColorWithAlpha( mimeData->text(), hasAlpha );
    if ( textColor.isValid() )
    {
      return textColor;
    }
  }

  //could not get color from mime data
  return QColor();
}

QgsNamedColorList QgsSymbolLayerUtils::colorListFromMimeData( const QMimeData *data )
{
  if ( !data )
    return {};

  QgsNamedColorList mimeColors;

  //prefer xml format
  if ( data->hasFormat( u"text/xml"_s ) )
  {
    //get XML doc
    const QByteArray encodedData = data->data( u"text/xml"_s );
    QDomDocument xmlDoc;
    xmlDoc.setContent( encodedData );

    const QDomElement dragDataElem = xmlDoc.documentElement();
    if ( dragDataElem.tagName() == "ColorSchemeModelDragData"_L1 )
    {
      const QDomNodeList nodeList = dragDataElem.childNodes();
      const int nChildNodes = nodeList.size();
      QDomElement currentElem;

      for ( int i = 0; i < nChildNodes; ++i )
      {
        currentElem = nodeList.at( i ).toElement();
        if ( currentElem.isNull() )
        {
          continue;
        }

        QPair< QColor, QString> namedColor;
        namedColor.first = QgsColorUtils::colorFromString( currentElem.attribute( u"color"_s, u"255,255,255,255"_s ) );
        namedColor.second = currentElem.attribute( u"label"_s, QString() );

        mimeColors << namedColor;
      }
    }
  }

  if ( mimeColors.length() == 0 && data->hasFormat( u"application/x-colorobject-list"_s ) )
  {
    //get XML doc
    const QByteArray encodedData = data->data( u"application/x-colorobject-list"_s );
    QDomDocument xmlDoc;
    xmlDoc.setContent( encodedData );

    const QDomNodeList colorsNodes = xmlDoc.elementsByTagName( u"colors"_s );
    if ( colorsNodes.length() > 0 )
    {
      const QDomElement colorsElem = colorsNodes.at( 0 ).toElement();
      const QDomNodeList colorNodeList = colorsElem.childNodes();
      const int nChildNodes = colorNodeList.size();
      QDomElement currentElem;

      for ( int i = 0; i < nChildNodes; ++i )
      {
        //li element
        currentElem = colorNodeList.at( i ).toElement();
        if ( currentElem.isNull() )
        {
          continue;
        }

        const QDomNodeList colorNodes = currentElem.elementsByTagName( u"color"_s );
        const QDomNodeList nameNodes = currentElem.elementsByTagName( u"name"_s );

        if ( colorNodes.length() > 0 )
        {
          const QDomElement colorElem = colorNodes.at( 0 ).toElement();

          const QStringList colorParts = colorElem.text().simplified().split( ' ' );
          if ( colorParts.length() < 3 )
          {
            continue;
          }

          const int red = colorParts.at( 0 ).toDouble() * 255;
          const int green = colorParts.at( 1 ).toDouble() * 255;
          const int blue = colorParts.at( 2 ).toDouble() * 255;
          QPair< QColor, QString> namedColor;
          namedColor.first = QColor( red, green, blue );
          if ( nameNodes.length() > 0 )
          {
            const QDomElement nameElem = nameNodes.at( 0 ).toElement();
            namedColor.second = nameElem.text();
          }
          mimeColors << namedColor;
        }
      }
    }
  }

  if ( mimeColors.length() == 0 && data->hasText() )
  {
    //attempt to read color data from mime text
    QList< QColor > parsedColors = QgsSymbolLayerUtils::parseColorList( data->text() );
    QList< QColor >::iterator it = parsedColors.begin();
    for ( ; it != parsedColors.end(); ++it )
    {
      mimeColors << qMakePair( *it, QString() );
    }
  }

  if ( mimeColors.length() == 0 && data->hasColor() )
  {
    //attempt to read color data directly from mime
    const QColor mimeColor = data->colorData().value<QColor>();
    if ( mimeColor.isValid() )
    {
      mimeColors << qMakePair( mimeColor, QString() );
    }
  }

  return mimeColors;
}

QMimeData *QgsSymbolLayerUtils::colorListToMimeData( const QgsNamedColorList &colorList, const bool allFormats )
{
  //native format
  QMimeData *mimeData = new QMimeData();
  QDomDocument xmlDoc;
  QDomElement xmlRootElement = xmlDoc.createElement( u"ColorSchemeModelDragData"_s );
  xmlDoc.appendChild( xmlRootElement );

  QgsNamedColorList::const_iterator colorIt = colorList.constBegin();
  for ( ; colorIt != colorList.constEnd(); ++colorIt )
  {
    QDomElement namedColor = xmlDoc.createElement( u"NamedColor"_s );
    namedColor.setAttribute( u"color"_s, QgsColorUtils::colorToString( ( *colorIt ).first ) );
    namedColor.setAttribute( u"label"_s, ( *colorIt ).second );
    xmlRootElement.appendChild( namedColor );
  }
  mimeData->setData( u"text/xml"_s, xmlDoc.toByteArray() );

  if ( !allFormats )
  {
    return mimeData;
  }

  //set mime text to list of hex values
  colorIt = colorList.constBegin();
  QStringList colorListString;
  for ( ; colorIt != colorList.constEnd(); ++colorIt )
  {
    colorListString << ( *colorIt ).first.name();
  }
  mimeData->setText( colorListString.join( QLatin1Char( '\n' ) ) );

  //set mime color data to first color
  if ( colorList.length() > 0 )
  {
    mimeData->setColorData( QVariant( colorList.at( 0 ).first ) );
  }

  return mimeData;
}

bool QgsSymbolLayerUtils::saveColorsToGpl( QFile &file, const QString &paletteName, const QgsNamedColorList &colors )
{
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return false;
  }

  QTextStream stream( &file );

  stream << "GIMP Palette" << Qt::endl;
  if ( paletteName.isEmpty() )
  {
    stream << "Name: QGIS Palette" << Qt::endl;
  }
  else
  {
    stream << "Name: " << paletteName << Qt::endl;
  }
  stream << "Columns: 4" << Qt::endl;
  stream << '#' << Qt::endl;

  for ( QgsNamedColorList::ConstIterator colorIt = colors.constBegin(); colorIt != colors.constEnd(); ++colorIt )
  {
    const QColor color = ( *colorIt ).first;
    if ( !color.isValid() )
    {
      continue;
    }
    stream << u"%1 %2 %3"_s.arg( color.red(), 3 ).arg( color.green(), 3 ).arg( color.blue(), 3 );
    stream << "\t" << ( ( *colorIt ).second.isEmpty() ? color.name() : ( *colorIt ).second ) << Qt::endl;
  }
  file.close();

  return true;
}

QgsNamedColorList QgsSymbolLayerUtils::importColorsFromGpl( QFile &file, bool &ok, QString &name )
{
  QgsNamedColorList importedColors;

  if ( !file.open( QIODevice::ReadOnly ) )
  {
    ok = false;
    return importedColors;
  }

  QTextStream in( &file );

  QString line = in.readLine();
  if ( !line.startsWith( "GIMP Palette"_L1 ) )
  {
    ok = false;
    return importedColors;
  }

  //find name line
  while ( !in.atEnd() && !line.startsWith( "Name:"_L1 ) && !line.startsWith( '#' ) )
  {
    line = in.readLine();
  }
  if ( line.startsWith( "Name:"_L1 ) )
  {
    const thread_local QRegularExpression nameRx( "Name:\\s*(\\S.*)$" );
    const QRegularExpressionMatch match = nameRx.match( line );
    if ( match.hasMatch() )
    {
      name = match.captured( 1 );
    }
  }

  //ignore lines until after "#"
  while ( !in.atEnd() && !line.startsWith( '#' ) )
  {
    line = in.readLine();
  }
  if ( in.atEnd() )
  {
    ok = false;
    return importedColors;
  }

  //ready to start reading colors
  const thread_local QRegularExpression rx( "^\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)(\\s.*)?$" );
  while ( !in.atEnd() )
  {
    line = in.readLine();
    const QRegularExpressionMatch match = rx.match( line );
    if ( !match.hasMatch() )
    {
      continue;
    }
    const int red = match.captured( 1 ).toInt();
    const int green = match.captured( 2 ).toInt();
    const int blue = match.captured( 3 ).toInt();
    const QColor color = QColor( red, green, blue );
    if ( !color.isValid() )
    {
      continue;
    }

    //try to read color name
    QString label;
    if ( rx.captureCount() > 3 )
    {
      label = match.captured( 4 ).simplified();
    }
    else
    {
      label = colorToName( color );
    }

    importedColors << qMakePair( color, label );
  }

  file.close();
  ok = true;
  return importedColors;
}

QColor QgsSymbolLayerUtils::parseColor( const QString &colorStr, bool strictEval )
{
  bool hasAlpha;
  return parseColorWithAlpha( colorStr, hasAlpha, strictEval );
}

QColor QgsSymbolLayerUtils::parseColorWithAlpha( const QString &colorStr, bool &containsAlpha, bool strictEval )
{
  QColor parsedColor;

  const thread_local QRegularExpression hexColorAlphaRx( "^\\s*#?([0-9a-fA-F]{6})([0-9a-fA-F]{2})\\s*$" );
  QRegularExpressionMatch match = hexColorAlphaRx.match( colorStr );

  //color in hex format "#aabbcc", but not #aabbccdd
  if ( !match.hasMatch() && QColor::isValidColor( colorStr ) )
  {
    //string is a valid hex color string
    parsedColor.setNamedColor( colorStr );
    if ( parsedColor.isValid() )
    {
      containsAlpha = false;
      return parsedColor;
    }
  }

  //color in hex format, with alpha
  if ( match.hasMatch() )
  {
    const QString hexColor = match.captured( 1 );
    parsedColor.setNamedColor( u"#"_s + hexColor );
    bool alphaOk;
    const int alphaHex = match.captured( 2 ).toInt( &alphaOk, 16 );

    if ( parsedColor.isValid() && alphaOk )
    {
      parsedColor.setAlpha( alphaHex );
      containsAlpha = true;
      return parsedColor;
    }
  }

  if ( !strictEval )
  {
    //color in hex format, without #
    const thread_local QRegularExpression hexColorRx2( "^\\s*(?:[0-9a-fA-F]{3}){1,2}\\s*$" );
    if ( colorStr.indexOf( hexColorRx2 ) != -1 )
    {
      //add "#" and parse
      parsedColor.setNamedColor( u"#"_s + colorStr );
      if ( parsedColor.isValid() )
      {
        containsAlpha = false;
        return parsedColor;
      }
    }
  }

  //color in (rrr,ggg,bbb) format, brackets and rgb prefix optional
  const thread_local QRegularExpression rgbFormatRx( "^\\s*(?:rgb)?\\(?\\s*((?:[01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])(?:\\.\\d*)?)\\s*,\\s*((?:[01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])(?:\\.\\d*)?)\\s*,\\s*((?:[01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])(?:\\.\\d*)?)\\s*\\)?\\s*;?\\s*$" );
  match = rgbFormatRx.match( colorStr );
  if ( match.hasMatch() )
  {
    bool rOk = false;
    bool gOk = false;
    bool bOk = false;
    const int r = match.captured( 1 ).toInt( &rOk );
    const int g = match.captured( 2 ).toInt( &gOk );
    const int b = match.captured( 3 ).toInt( &bOk );

    if ( !rOk || !gOk || !bOk )
    {
      const float rFloat = match.captured( 1 ).toFloat();
      const float gFloat = match.captured( 2 ).toFloat();
      const float bFloat = match.captured( 3 ).toFloat();
      parsedColor.setRgbF( rFloat / 255.0, gFloat / 255.0, bFloat / 255.0 );
    }
    else
    {
      parsedColor.setRgb( r, g, b );
    }

    if ( parsedColor.isValid() )
    {
      containsAlpha = false;
      return parsedColor;
    }
  }

  //color in hsl(h,s,l) format, brackets optional
  const thread_local QRegularExpression hslFormatRx( "^\\s*hsl\\(?\\s*(\\d+(?:\\.\\d*)?)\\s*,\\s*(\\d+(?:\\.\\d*)?)\\s*%\\s*,\\s*(\\d+(?:\\.\\d*)?)\\s*%\\s*\\)?\\s*;?\\s*$" );
  match = hslFormatRx.match( colorStr );
  if ( match.hasMatch() )
  {
    bool hOk = false;
    bool sOk = false;
    bool lOk = false;
    const int h = match.captured( 1 ).toInt( &hOk );
    const int s = match.captured( 2 ).toInt( &sOk );
    const int l = match.captured( 3 ).toInt( &lOk );

    if ( !hOk || !sOk || !lOk )
    {
      const float hFloat = match.captured( 1 ).toFloat();
      const float sFloat = match.captured( 2 ).toFloat();
      const float lFloat = match.captured( 3 ).toFloat();
      parsedColor.setHslF( hFloat / 360.0, sFloat / 100.0, lFloat / 100.0 );
    }
    else
    {
      parsedColor.setHsl( h, s / 100.0 * 255.0, l / 100.0 * 255.0 );
    }
    if ( parsedColor.isValid() )
    {
      containsAlpha = false;
      return parsedColor;
    }
  }

  //color in (r%,g%,b%) format, brackets and rgb prefix optional
  const thread_local QRegularExpression rgbPercentFormatRx( "^\\s*(?:rgb)?\\(?\\s*(100|0*\\d{1,2}(?:\\.\\d*)?)\\s*%\\s*,\\s*(100|0*\\d{1,2}(?:\\.\\d*)?)\\s*%\\s*,\\s*(100|0*\\d{1,2}(?:\\.\\d*)?)\\s*%\\s*\\)?\\s*;?\\s*$" );
  match = rgbPercentFormatRx.match( colorStr );
  if ( match.hasMatch() )
  {
    const double r = match.captured( 1 ).toDouble() / 100;
    const double g = match.captured( 2 ).toDouble() / 100;
    const double b = match.captured( 3 ).toDouble() / 100;
    parsedColor.setRgbF( r, g, b );
    if ( parsedColor.isValid() )
    {
      containsAlpha = false;
      return parsedColor;
    }
  }

  //color in (r,g,b,a) format, brackets and rgba prefix optional
  const thread_local QRegularExpression rgbaFormatRx( "^\\s*(?:rgba)?\\(?\\s*((?:[01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])(?:\\.\\d*)?)\\s*,\\s*((?:[01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])(?:\\.\\d*)?)\\s*,\\s*((?:[01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])(?:\\.\\d*)?)\\s*,\\s*(0|0?\\.\\d*|1(?:\\.0*)?)\\s*\\)?\\s*;?\\s*$" );
  match = rgbaFormatRx.match( colorStr );
  if ( match.hasMatch() )
  {
    bool rOk = false;
    bool gOk = false;
    bool bOk = false;
    const int r = match.captured( 1 ).toInt( &rOk );
    const int g = match.captured( 2 ).toInt( &gOk );
    const int b = match.captured( 3 ).toInt( &bOk );
    const double aDouble = match.captured( 4 ).toDouble();

    if ( !rOk || !gOk || !bOk )
    {
      const float rFloat = match.captured( 1 ).toFloat();
      const float gFloat = match.captured( 2 ).toFloat();
      const float bFloat = match.captured( 3 ).toFloat();
      parsedColor.setRgbF( rFloat / 255.0, gFloat / 255.0, bFloat / 255.0, aDouble );
    }
    else
    {
      const int a = static_cast< int >( std::round( match.captured( 4 ).toDouble() * 255.0 ) );
      parsedColor.setRgb( r, g, b, a );
    }
    if ( parsedColor.isValid() )
    {
      containsAlpha = true;
      return parsedColor;
    }
  }

  //color in (r%,g%,b%,a) format, brackets and rgba prefix optional
  const thread_local QRegularExpression rgbaPercentFormatRx( "^\\s*(?:rgba)?\\(?\\s*(100|0*\\d{1,2}(?:\\.\\d*)?)\\s*%\\s*,\\s*(100|0*\\d{1,2}(?:\\.\\d*)?)\\s*%\\s*,\\s*(100|0*\\d{1,2}(?:\\.\\d*)?)\\s*%\\s*,\\s*(0|0?\\.\\d*|1(?:\\.0*)?)\\s*\\)?\\s*;?\\s*$" );
  match = rgbaPercentFormatRx.match( colorStr );
  if ( match.hasMatch() )
  {
    const double r = match.captured( 1 ).toDouble() / 100;
    const double g = match.captured( 2 ).toDouble() / 100;
    const double b = match.captured( 3 ).toDouble() / 100;
    const double a = match.captured( 4 ).toDouble();
    parsedColor.setRgbF( r, g, b, a );
    if ( parsedColor.isValid() )
    {
      containsAlpha = true;
      return parsedColor;
    }
  }

  //color in hsla(h,s%,l%,a) format, brackets optional
  const thread_local QRegularExpression hslaPercentFormatRx( "^\\s*hsla\\(?\\s*(\\d+(?:\\.\\d*)?)\\s*,\\s*(\\d+(?:\\.\\d*)?)\\s*%\\s*,\\s*(\\d+(?:\\.\\d*)?)\\s*%\\s*,\\s*([\\d\\.]+)\\s*\\)?\\s*;?\\s*$" );
  match = hslaPercentFormatRx.match( colorStr );
  if ( match.hasMatch() )
  {
    bool hOk = false;
    bool sOk = false;
    bool lOk = false;
    const int h = match.captured( 1 ).toInt( &hOk );
    const int s = match.captured( 2 ).toInt( &sOk );
    const int l = match.captured( 3 ).toInt( &lOk );
    const double aDouble = match.captured( 4 ).toDouble();

    if ( !hOk || !sOk || !lOk )
    {
      const float hFloat = match.captured( 1 ).toFloat();
      const float sFloat = match.captured( 2 ).toFloat();
      const float lFloat = match.captured( 3 ).toFloat();
      parsedColor.setHslF( hFloat / 360.0, sFloat / 100.0, lFloat / 100.0, aDouble );
    }
    else
    {
      const int a = std::round( aDouble * 255.0 );
      parsedColor.setHsl( h, s / 100.0 * 255.0, l / 100.0 * 255.0, a );
    }

    if ( parsedColor.isValid() )
    {
      containsAlpha = true;
      return parsedColor;
    }
  }

  //couldn't parse string as color
  return QColor();
}

void QgsSymbolLayerUtils::multiplyImageOpacity( QImage *image, qreal opacity )
{
  if ( !image )
  {
    return;
  }

  QRgb myRgb;
  const QImage::Format format = image->format();
  if ( format != QImage::Format_ARGB32_Premultiplied && format != QImage::Format_ARGB32 )
  {
    QgsDebugError( u"no alpha channel."_s );
    return;
  }

  //change the alpha component of every pixel
  for ( int heightIndex = 0; heightIndex < image->height(); ++heightIndex )
  {
    QRgb *scanLine = reinterpret_cast< QRgb * >( image->scanLine( heightIndex ) );
    for ( int widthIndex = 0; widthIndex < image->width(); ++widthIndex )
    {
      myRgb = scanLine[widthIndex];
      if ( format == QImage::Format_ARGB32_Premultiplied )
        scanLine[widthIndex] = qRgba( opacity * qRed( myRgb ), opacity * qGreen( myRgb ), opacity * qBlue( myRgb ), opacity * qAlpha( myRgb ) );
      else
        scanLine[widthIndex] = qRgba( qRed( myRgb ), qGreen( myRgb ), qBlue( myRgb ), opacity * qAlpha( myRgb ) );
    }
  }
}

void QgsSymbolLayerUtils::blurImageInPlace( QImage &image, QRect rect, int radius, bool alphaOnly )
{
  // culled from Qt's qpixmapfilter.cpp, see: http://www.qtcentre.org/archive/index.php/t-26534.html
  const int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
  const int alpha = ( radius < 1 )  ? 16 : ( radius > 17 ) ? 1 : tab[radius - 1];

  if ( image.format() != QImage::Format_ARGB32_Premultiplied
       && image.format() != QImage::Format_RGB32 )
  {
    image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied );
  }

  const int r1 = rect.top();
  const int r2 = rect.bottom();
  const int c1 = rect.left();
  const int c2 = rect.right();

  const int bpl = image.bytesPerLine();
  int rgba[4];
  unsigned char *p;

  int i1 = 0;
  int i2 = 3;

  if ( alphaOnly ) // this seems to only work right for a black color
    i1 = i2 = ( QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3 );

  for ( int col = c1; col <= c2; col++ )
  {
    p = image.scanLine( r1 ) + col * 4;
    for ( int i = i1; i <= i2; i++ )
      rgba[i] = p[i] << 4;

    p += bpl;
    for ( int j = r1; j < r2; j++, p += bpl )
      for ( int i = i1; i <= i2; i++ )
        p[i] = ( rgba[i] += ( ( p[i] << 4 ) - rgba[i] ) * alpha / 16 ) >> 4;
  }

  for ( int row = r1; row <= r2; row++ )
  {
    p = image.scanLine( row ) + c1 * 4;
    for ( int i = i1; i <= i2; i++ )
      rgba[i] = p[i] << 4;

    p += 4;
    for ( int j = c1; j < c2; j++, p += 4 )
      for ( int i = i1; i <= i2; i++ )
        p[i] = ( rgba[i] += ( ( p[i] << 4 ) - rgba[i] ) * alpha / 16 ) >> 4;
  }

  for ( int col = c1; col <= c2; col++ )
  {
    p = image.scanLine( r2 ) + col * 4;
    for ( int i = i1; i <= i2; i++ )
      rgba[i] = p[i] << 4;

    p -= bpl;
    for ( int j = r1; j < r2; j++, p -= bpl )
      for ( int i = i1; i <= i2; i++ )
        p[i] = ( rgba[i] += ( ( p[i] << 4 ) - rgba[i] ) * alpha / 16 ) >> 4;
  }

  for ( int row = r1; row <= r2; row++ )
  {
    p = image.scanLine( row ) + c2 * 4;
    for ( int i = i1; i <= i2; i++ )
      rgba[i] = p[i] << 4;

    p -= 4;
    for ( int j = c1; j < c2; j++, p -= 4 )
      for ( int i = i1; i <= i2; i++ )
        p[i] = ( rgba[i] += ( ( p[i] << 4 ) - rgba[i] ) * alpha / 16 ) >> 4;
  }
}

void QgsSymbolLayerUtils::premultiplyColor( QColor &rgb, int alpha )
{
  if ( alpha != 255 && alpha > 0 )
  {
    // Semi-transparent pixel. We need to adjust the colors for ARGB32_Premultiplied images
    // where color values have to be premultiplied by alpha
    const double alphaFactor = alpha / 255.;
    int r = 0, g = 0, b = 0;
    rgb.getRgb( &r, &g, &b );

    r *= alphaFactor;
    g *= alphaFactor;
    b *= alphaFactor;
    rgb.setRgb( r, g, b, alpha );
  }
  else if ( alpha == 0 )
  {
    rgb.setRgb( 0, 0, 0, 0 );
  }
}

bool QgsSymbolLayerUtils::condenseFillAndOutline( QgsFillSymbolLayer *fill, QgsLineSymbolLayer *outline )
{
  QgsSimpleFillSymbolLayer *simpleFill = dynamic_cast< QgsSimpleFillSymbolLayer *>( fill );
  QgsSimpleLineSymbolLayer *simpleLine = dynamic_cast< QgsSimpleLineSymbolLayer *>( outline );

  if ( !simpleFill || !simpleLine )
    return false;

  if ( simpleLine->useCustomDashPattern() )
    return false;

  if ( simpleLine->dashPatternOffset() )
    return false;

  if ( simpleLine->alignDashPattern() )
    return false;

  if ( simpleLine->tweakDashPatternOnCorners() )
    return false;

  if ( simpleLine->trimDistanceStart() || simpleLine->trimDistanceEnd() )
    return false;

  if ( simpleLine->drawInsidePolygon() )
    return false;

  if ( simpleLine->ringFilter() != QgsSimpleLineSymbolLayer::AllRings )
    return false;

  if ( simpleLine->offset() )
    return false;

  if ( simpleLine->hasDataDefinedProperties() )
    return false;

  // looks good!
  simpleFill->setStrokeColor( simpleLine->color() );
  simpleFill->setStrokeWidth( simpleLine->width() );
  simpleFill->setStrokeWidthUnit( simpleLine->widthUnit() );
  simpleFill->setStrokeWidthMapUnitScale( simpleLine->widthMapUnitScale() );
  simpleFill->setStrokeStyle( simpleLine->penStyle() );
  simpleFill->setPenJoinStyle( simpleLine->penJoinStyle() );
  return true;
}

void QgsSymbolLayerUtils::sortVariantList( QList<QVariant> &list, Qt::SortOrder order )
{
  if ( order == Qt::AscendingOrder )
  {
    //std::sort( list.begin(), list.end(), _QVariantLessThan );
    std::sort( list.begin(), list.end(), qgsVariantLessThan );
  }
  else // Qt::DescendingOrder
  {
    //std::sort( list.begin(), list.end(), _QVariantGreaterThan );
    std::sort( list.begin(), list.end(), qgsVariantGreaterThan );
  }
}

QPointF QgsSymbolLayerUtils::pointOnLineWithDistance( QPointF startPoint, QPointF directionPoint, double distance )
{
  const double dx = directionPoint.x() - startPoint.x();
  const double dy = directionPoint.y() - startPoint.y();
  const double length = std::sqrt( dx * dx + dy * dy );
  const double scaleFactor = distance / length;
  return QPointF( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}


QStringList QgsSymbolLayerUtils::listSvgFiles()
{
  // copied from QgsMarkerCatalogue - TODO: unify  //#spellok
  QStringList list;
  QStringList svgPaths = QgsApplication::svgPaths();

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    const QDir dir( svgPaths[i] );
    const auto svgSubPaths = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
    for ( const QString &item : svgSubPaths )
    {
      svgPaths.insert( i + 1, dir.path() + '/' + item );
    }

    const auto svgFiles = dir.entryList( QStringList( "*.svg" ), QDir::Files );
    for ( const QString &item : svgFiles )
    {
      // TODO test if it is correct SVG
      list.append( dir.path() + '/' + item );
    }
  }
  return list;
}

// Stripped down version of listSvgFiles() for specified directory
QStringList QgsSymbolLayerUtils::listSvgFilesAt( const QString &directory )
{
  // TODO anything that applies for the listSvgFiles() applies this also

  QStringList list;
  QStringList svgPaths;
  svgPaths.append( directory );

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    const QDir dir( svgPaths[i] );
    const auto svgSubPaths = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
    for ( const QString &item : svgSubPaths )
    {
      svgPaths.insert( i + 1, dir.path() + '/' + item );
    }

    const auto svgFiles = dir.entryList( QStringList( "*.svg" ), QDir::Files );
    for ( const QString &item : svgFiles )
    {
      list.append( dir.path() + '/' + item );
    }
  }
  return list;

}

QString QgsSymbolLayerUtils::svgSymbolNameToPath( const QString &n, const QgsPathResolver &pathResolver )
{
  if ( n.isEmpty() )
    return QString();

  if ( n.startsWith( "base64:"_L1 ) )
    return n;

  // we might have a full path...
  if ( QFileInfo::exists( n ) )
    return QFileInfo( n ).canonicalFilePath();

  QString name = n;
  // or it might be an url...
  if ( name.contains( "://"_L1 ) )
  {
    const QUrl url( name );
    if ( url.isValid() && !url.scheme().isEmpty() )
    {
      if ( url.scheme().compare( "file"_L1, Qt::CaseInsensitive ) == 0 )
      {
        // it's a url to a local file
        name = url.toLocalFile();
        if ( QFile( name ).exists() )
        {
          return QFileInfo( name ).canonicalFilePath();
        }
      }
      else
      {
        // it's a url pointing to a online resource
        return name;
      }
    }
  }

  // SVG symbol not found - probably a relative path was used

  QStringList svgPaths = QgsApplication::svgPaths();
  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QString svgPath = svgPaths[i];
    if ( svgPath.endsWith( QChar( '/' ) ) )
    {
      svgPath.chop( 1 );
    }

    QgsDebugMsgLevel( "SvgPath: " + svgPath, 3 );
    // Not sure why to lowest dir was used instead of full relative path, it was causing #8664
    //QFileInfo myInfo( name );
    //QString myFileName = myInfo.fileName(); // foo.svg
    //QString myLowestDir = myInfo.dir().dirName();
    //QString myLocalPath = svgPath + QString( myLowestDir.isEmpty() ? "" : '/' + myLowestDir ) + '/' + myFileName;
    const QString myLocalPath = svgPath + QDir::separator() + name;

    QgsDebugMsgLevel( "Alternative svg path: " + myLocalPath, 3 );
    if ( QFile( myLocalPath ).exists() )
    {
      QgsDebugMsgLevel( u"Svg found in alternative path"_s, 3 );
      return QFileInfo( myLocalPath ).canonicalFilePath();
    }
  }

  return pathResolver.readPath( name );
}

QString QgsSymbolLayerUtils::svgSymbolPathToName( const QString &p, const QgsPathResolver &pathResolver )
{
  if ( p.isEmpty() )
    return QString();

  if ( p.startsWith( "base64:"_L1 ) )
    return p;

  if ( !QFileInfo::exists( p ) )
    return p;

  QString path = QFileInfo( p ).canonicalFilePath();

  QStringList svgPaths = QgsApplication::svgPaths();

  bool isInSvgPaths = false;
  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    const QString dir = QFileInfo( svgPaths[i] ).canonicalFilePath();

    if ( !dir.isEmpty() && path.startsWith( dir ) )
    {
      path = path.mid( dir.size() + 1 );
      isInSvgPaths = true;
      break;
    }
  }

  if ( isInSvgPaths )
    return path;

  return pathResolver.writePath( path );
}

QPolygonF lineStringToQPolygonF( const QgsLineString *line )
{
  const double *srcX = line->xData();
  const double *srcY = line->yData();
  const int count = line->numPoints();
  QPolygonF thisRes( count );
  QPointF *dest = thisRes.data();
  for ( int i = 0; i < count; ++i )
  {
    *dest++ = QPointF( *srcX++, *srcY++ );
  }
  return thisRes;
}

QPolygonF curveToPolygonF( const QgsCurve *curve )
{
  if ( const QgsLineString *line = qgsgeometry_cast< const QgsLineString * >( curve ) )
  {
    return lineStringToQPolygonF( line );
  }
  else
  {
    const std::unique_ptr< QgsLineString > straightened( curve->curveToLine() );
    return lineStringToQPolygonF( straightened.get() );
  }
}

QList<QList<QPolygonF> > QgsSymbolLayerUtils::toQPolygonF( const QgsGeometry &geometry, Qgis::SymbolType type )
{
  return toQPolygonF( geometry.constGet(), type );
}

QList<QList<QPolygonF> > QgsSymbolLayerUtils::toQPolygonF( const QgsAbstractGeometry *geometry, Qgis::SymbolType type )
{
  if ( !geometry )
    return {};

  switch ( type )
  {
    case Qgis::SymbolType::Marker:
    {
      QPolygonF points;

      if ( QgsWkbTypes::flatType( geometry->wkbType() ) == Qgis::WkbType::MultiPoint )
      {
        for ( auto it = geometry->vertices_begin(); it != geometry->vertices_end(); ++it )
          points << QPointF( ( *it ).x(), ( *it ).y() );
      }
      else
      {
        points << QPointF( 0, 0 );
      }
      return QList< QList<QPolygonF> >() << ( QList< QPolygonF >() << points );
    }

    case Qgis::SymbolType::Line:
    {
      QList< QList<QPolygonF> > res;
      if ( QgsWkbTypes::geometryType( geometry->wkbType() ) == Qgis::GeometryType::Line )
      {
        for ( auto it = geometry->const_parts_begin(); it != geometry->const_parts_end(); ++it )
        {
          res << ( QList< QPolygonF >() << curveToPolygonF( qgsgeometry_cast< const QgsCurve * >( *it ) ) );
        }
      }
      return res;
    }

    case Qgis::SymbolType::Fill:
    {
      QList< QList<QPolygonF> > res;

      for ( auto it = geometry->const_parts_begin(); it != geometry->const_parts_end(); ++it )
      {
        QList<QPolygonF> thisPart;
        const QgsCurvePolygon *surface = qgsgeometry_cast< const QgsCurvePolygon * >( *it );
        if ( !surface )
          continue;

        if ( !surface->exteriorRing() )
          continue;

        thisPart << curveToPolygonF( surface->exteriorRing() );

        for ( int i = 0; i < surface->numInteriorRings(); ++i )
          thisPart << curveToPolygonF( surface->interiorRing( i ) );
        res << thisPart;
      }

      return res;
    }

    case Qgis::SymbolType::Hybrid:
      return QList< QList<QPolygonF> >();
  }

  return QList< QList<QPolygonF> >();
}


QPointF QgsSymbolLayerUtils::polygonCentroid( const QPolygonF &points )
{
  //Calculate the centroid of points
  double cx = 0, cy = 0;
  double area, sum = 0;
  for ( int i = points.count() - 1, j = 0; j < points.count(); i = j++ )
  {
    const QPointF &p1 = points[i];
    const QPointF &p2 = points[j];
    area = p1.x() * p2.y() - p1.y() * p2.x();
    sum += area;
    cx += ( p1.x() + p2.x() ) * area;
    cy += ( p1.y() + p2.y() ) * area;
  }
  sum *= 3.0;
  if ( qgsDoubleNear( sum, 0.0 ) )
  {
    // the linear ring is invalid -  let's fall back to a solution that will still
    // allow us render at least something (instead of just returning point nan,nan)
    if ( points.count() >= 2 )
      return QPointF( ( points[0].x() + points[1].x() ) / 2, ( points[0].y() + points[1].y() ) / 2 );
    else if ( points.count() == 1 )
      return points[0];
    else
      return QPointF(); // hopefully we shouldn't ever get here
  }
  cx /= sum;
  cy /= sum;

  return QPointF( cx, cy );
}

QPointF QgsSymbolLayerUtils::polygonPointOnSurface( const QPolygonF &points, const QVector<QPolygonF> *rings )
{
  QPointF centroid = QgsSymbolLayerUtils::polygonCentroid( points );

  if ( ( rings && rings->count() > 0 ) || !pointInPolygon( points, centroid ) )
  {
    unsigned int i, pointCount = points.count();
    QgsPolylineXY polyline( pointCount );
    for ( i = 0; i < pointCount; ++i ) polyline[i] = QgsPointXY( points[i].x(), points[i].y() );
    QgsGeometry geom = QgsGeometry::fromPolygonXY( QgsPolygonXY() << polyline );
    if ( !geom.isNull() )
    {
      if ( rings )
      {
        for ( auto ringIt = rings->constBegin(); ringIt != rings->constEnd(); ++ringIt )
        {
          pointCount = ( *ringIt ).count();
          QgsPolylineXY polyline( pointCount );
          for ( i = 0; i < pointCount; ++i ) polyline[i] = QgsPointXY( ( *ringIt )[i].x(), ( *ringIt )[i].y() );
          geom.addRing( polyline );
        }
      }

      const QgsGeometry pointOnSurfaceGeom = geom.pointOnSurface();
      if ( !pointOnSurfaceGeom.isNull() )
      {
        const QgsPointXY point = pointOnSurfaceGeom.asPoint();
        centroid.setX( point.x() );
        centroid.setY( point.y() );
      }
    }
  }

  return QPointF( centroid.x(), centroid.y() );
}

bool QgsSymbolLayerUtils::pointInPolygon( const QPolygonF &points, QPointF point )
{
  bool inside = false;

  const double x = point.x();
  const double y = point.y();

  for ( int i = 0, j = points.count() - 1; i < points.count(); i++ )
  {
    const QPointF &p1 = points[i];
    const QPointF &p2 = points[j];

    if ( qgsDoubleNear( p1.x(), x ) && qgsDoubleNear( p1.y(), y ) )
      return true;

    if ( ( p1.y() < y && p2.y() >= y ) || ( p2.y() < y && p1.y() >= y ) )
    {
      if ( p1.x() + ( y - p1.y() ) / ( p2.y() - p1.y() ) * ( p2.x() - p1.x() ) <= x )
        inside = !inside;
    }

    j = i;
  }
  return inside;
}

double QgsSymbolLayerUtils::polylineLength( const QPolygonF &polyline )
{
  if ( polyline.size() < 2 )
    return 0;

  double totalLength = 0;
  auto it = polyline.begin();
  QPointF p1 = *it++;
  for ( ; it != polyline.end(); ++it )
  {
    const QPointF p2 = *it;
    const double segmentLength = std::sqrt( std::pow( p1.x() - p2.x(), 2.0 ) + std::pow( p1.y() - p2.y(), 2.0 ) );
    totalLength += segmentLength;
    p1 = p2;
  }
  return totalLength;
}

QPolygonF QgsSymbolLayerUtils::polylineSubstring( const QPolygonF &polyline, double startOffset, double endOffset )
{
  if ( polyline.size() < 2 )
    return QPolygonF();

  double totalLength = 0;
  auto it = polyline.begin();
  QPointF p1 = *it++;
  std::vector< double > segmentLengths( polyline.size() - 1 );
  auto segmentLengthIt = segmentLengths.begin();
  for ( ; it != polyline.end(); ++it )
  {
    const QPointF p2 = *it;
    *segmentLengthIt = std::sqrt( std::pow( p1.x() - p2.x(), 2.0 ) + std::pow( p1.y() - p2.y(), 2.0 ) );
    totalLength += *segmentLengthIt;

    segmentLengthIt++;
    p1 = p2;
  }

  if ( startOffset >= 0 && totalLength <= startOffset )
    return QPolygonF();
  if ( endOffset < 0 && totalLength <= -endOffset )
    return QPolygonF();

  const double startDistance = startOffset < 0 ? totalLength + startOffset : startOffset;
  const double endDistance = endOffset <= 0 ? totalLength + endOffset : endOffset;
  QPolygonF substringPoints;
  substringPoints.reserve( polyline.size() );

  it = polyline.begin();
  segmentLengthIt = segmentLengths.begin();

  p1 = *it++;
  bool foundStart = false;
  if ( qgsDoubleNear( startDistance, 0.0 ) || startDistance < 0 )
  {
    substringPoints << p1;
    foundStart = true;
  }

  double distanceTraversed = 0;
  for ( ; it != polyline.end(); ++it )
  {
    const QPointF p2 = *it;
    if ( distanceTraversed < startDistance && distanceTraversed + *segmentLengthIt > startDistance )
    {
      // start point falls on this segment
      const double distanceToStart = startDistance - distanceTraversed;
      double startX, startY;
      QgsGeometryUtilsBase::pointOnLineWithDistance( p1.x(), p1.y(), p2.x(), p2.y(), distanceToStart, startX, startY );
      substringPoints << QPointF( startX, startY );
      foundStart = true;
    }
    if ( foundStart && ( distanceTraversed + *segmentLengthIt > endDistance ) )
    {
      // end point falls on this segment
      const double distanceToEnd = endDistance - distanceTraversed;
      double endX, endY;
      QgsGeometryUtilsBase::pointOnLineWithDistance( p1.x(), p1.y(), p2.x(), p2.y(), distanceToEnd, endX, endY );
      if ( substringPoints.last() != QPointF( endX, endY ) )
        substringPoints << QPointF( endX, endY );
    }
    else if ( foundStart )
    {
      if ( substringPoints.last() != QPointF( p2.x(), p2.y() ) )
        substringPoints << QPointF( p2.x(), p2.y() );
    }

    distanceTraversed += *segmentLengthIt;
    if ( distanceTraversed > endDistance )
      break;

    p1 = p2;
    segmentLengthIt++;
  }

  if ( ( substringPoints.size() < 2 ) || ( substringPoints.size() == 2 && substringPoints.at( 0 ) == substringPoints.at( 1 ) ) )
    return QPolygonF();

  return substringPoints;
}

bool QgsSymbolLayerUtils::isSharpCorner( QPointF p1, QPointF p2, QPointF p3 )
{
  double vertexAngle = M_PI - ( std::atan2( p3.y() - p2.y(), p3.x() - p2.x() ) - std::atan2( p2.y() - p1.y(), p2.x() - p1.x() ) );
  vertexAngle = QgsGeometryUtilsBase::normalizedAngle( vertexAngle );

  // extreme angles form more than 45 degree angle at a node
  return vertexAngle < M_PI * 135.0 / 180.0 || vertexAngle > M_PI * 225.0 / 180.0;
}

void QgsSymbolLayerUtils::appendPolyline( QPolygonF &target, const QPolygonF &line )
{
  target.reserve( target.size() + line.size() );
  for ( const QPointF &pt : line )
  {
    if ( !target.empty() && target.last() == pt )
      continue;

    target << pt;
  }
}

std::unique_ptr< QgsExpression > QgsSymbolLayerUtils::fieldOrExpressionToExpression( const QString &fieldOrExpression )
{
  if ( fieldOrExpression.isEmpty() )
    return nullptr;

  auto expr = std::make_unique< QgsExpression >( fieldOrExpression );
  if ( !expr->hasParserError() )
    return expr;

  // now try with quoted field name
  expr = std::make_unique< QgsExpression >( QgsExpression::quotedColumnRef( fieldOrExpression ) );
  Q_ASSERT( !expr->hasParserError() );
  return expr;
}

QString QgsSymbolLayerUtils::fieldOrExpressionFromExpression( QgsExpression *expression )
{
  const QgsExpressionNode *n = expression->rootNode();

  if ( n && n->nodeType() == QgsExpressionNode::ntColumnRef )
    return static_cast<const QgsExpressionNodeColumnRef *>( n )->name();

  return expression->expression();
}

QList<double> QgsSymbolLayerUtils::prettyBreaks( double minimum, double maximum, int classes )
{
  // C++ implementation of R's pretty algorithm
  // Based on code for determining optimal tick placement for statistical graphics
  // from the R statistical programming language.
  // Code ported from R implementation from 'labeling' R package
  //
  // Computes a sequence of about 'classes' equally spaced round values
  // which cover the range of values from 'minimum' to 'maximum'.
  // The values are chosen so that they are 1, 2 or 5 times a power of 10.

  QList<double> breaks;
  if ( classes < 1 )
  {
    breaks.append( maximum );
    return breaks;
  }

  const int minimumCount = static_cast< int >( classes ) / 3;
  const double shrink = 0.75;
  const double highBias = 1.5;
  const double adjustBias = 0.5 + 1.5 * highBias;
  const int divisions = classes;
  const double h = highBias;
  double cell;
  bool small = false;
  const double dx = maximum - minimum;

  if ( qgsDoubleNear( dx, 0.0 ) && qgsDoubleNear( maximum, 0.0 ) )
  {
    cell = 1.0;
    small = true;
  }
  else
  {
    int U = 1;
    cell = std::max( std::fabs( minimum ), std::fabs( maximum ) );
    if ( adjustBias >= 1.5 * h + 0.5 )
    {
      U = 1 + ( 1.0 / ( 1 + h ) );
    }
    else
    {
      U = 1 + ( 1.5 / ( 1 + adjustBias ) );
    }
    small = dx < ( cell * U * std::max( 1, divisions ) * 1e-07 * 3.0 );
  }

  if ( small )
  {
    if ( cell > 10 )
    {
      cell = 9 + cell / 10;
      cell = cell * shrink;
    }
    if ( minimumCount > 1 )
    {
      cell = cell / minimumCount;
    }
  }
  else
  {
    cell = dx;
    if ( divisions > 1 )
    {
      cell = cell / divisions;
    }
  }
  if ( cell < 20 * 1e-07 )
  {
    cell = 20 * 1e-07;
  }

  const double base = std::pow( 10.0, std::floor( std::log10( cell ) ) );
  double unit = base;
  if ( ( 2 * base ) - cell < h * ( cell - unit ) )
  {
    unit = 2.0 * base;
    if ( ( 5 * base ) - cell < adjustBias * ( cell - unit ) )
    {
      unit = 5.0 * base;
      if ( ( 10.0 * base ) - cell < h * ( cell - unit ) )
      {
        unit = 10.0 * base;
      }
    }
  }
  // Maybe used to correct for the epsilon here??
  int start = std::floor( minimum / unit + 1e-07 );
  int end = std::ceil( maximum / unit - 1e-07 );

  // Extend the range out beyond the data. Does this ever happen??
  while ( start * unit > minimum + ( 1e-07 * unit ) )
  {
    start = start - 1;
  }
  while ( end * unit < maximum - ( 1e-07 * unit ) )
  {
    end = end + 1;
  }
  QgsDebugMsgLevel( u"pretty classes: %1"_s.arg( end ), 3 );

  // If we don't have quite enough labels, extend the range out
  // to make more (these labels are beyond the data :()
  int k = std::floor( 0.5 + end - start );
  if ( k < minimumCount )
  {
    k = minimumCount - k;
    if ( start >= 0 )
    {
      end = end + k / 2;
      start = start - k / 2 + k % 2;
    }
    else
    {
      start = start - k / 2;
      end = end + k / 2 + k % 2;
    }
  }
  const double minimumBreak = start * unit;
  //double maximumBreak = end * unit;
  const int count = end - start;

  breaks.reserve( count );
  for ( int i = 1; i < count + 1; i++ )
  {
    breaks.append( minimumBreak + i * unit );
  }

  if ( breaks.isEmpty() )
    return breaks;

  if ( breaks.first() < minimum )
  {
    breaks[0] = minimum;
  }
  if ( breaks.last() > maximum )
  {
    breaks[breaks.count() - 1] = maximum;
  }

  // because sometimes when number of classes is big,
  // break supposed to be at zero is something like -2.22045e-16
  if ( minimum < 0.0 && maximum > 0.0 ) //then there should be a zero somewhere
  {
    QList<double> breaksMinusZero; // compute difference "each break - 0"
    for ( int i = 0; i < breaks.count(); i++ )
    {
      breaksMinusZero.append( breaks[i] - 0.0 );
    }
    int posOfMin = 0;
    for ( int i = 1; i < breaks.count(); i++ ) // find position of minimal difference
    {
      if ( std::abs( breaksMinusZero[i] ) < std::abs( breaksMinusZero[i - 1] ) )
        posOfMin = i;
    }
    breaks[posOfMin] = 0.0;
  }

  return breaks;
}

double QgsSymbolLayerUtils::rescaleUom( double size, Qgis::RenderUnit unit, const QVariantMap &props )
{
  double scale = 1;
  bool roundToUnit = false;
  if ( unit == Qgis::RenderUnit::Unknown )
  {
    if ( props.contains( u"uomScale"_s ) )
    {
      bool ok;
      scale = props.value( u"uomScale"_s ).toDouble( &ok );
      if ( !ok )
      {
        return size;
      }
    }
  }
  else
  {
    if ( props.value( u"uom"_s ) == "http://www.opengeospatial.org/se/units/metre"_L1 )
    {
      switch ( unit )
      {
        case Qgis::RenderUnit::Millimeters:
          scale = 0.001;
          break;
        case Qgis::RenderUnit::Pixels:
          scale = 0.00028;
          roundToUnit = true;
          break;
        default:
          scale = 1;
      }
    }
    else
    {
      // target is pixels
      switch ( unit )
      {
        case Qgis::RenderUnit::Millimeters:
          scale = 1 / 0.28;
          roundToUnit = true;
          break;
        case Qgis::RenderUnit::Inches:
          scale = 1 / 0.28 * 25.4;
          roundToUnit = true;
          break;
        case Qgis::RenderUnit::Points:
          scale = 90. /* dots per inch according to OGC SLD */ / 72. /* points per inch */;
          roundToUnit = true;
          break;
        case Qgis::RenderUnit::Pixels:
          // pixel is pixel
          scale = 1;
          break;
        case Qgis::RenderUnit::MapUnits:
        case Qgis::RenderUnit::MetersInMapUnits:
          // already handed via uom
          scale = 1;
          break;
        case Qgis::RenderUnit::Percentage:
        case Qgis::RenderUnit::Unknown:
          // these do not make sense and should not really reach here
          scale = 1;
      }
    }

  }
  double rescaled = size * scale;
  // round to unit if the result is pixels to avoid a weird looking SLD (people often think
  // of pixels as integers, even if SLD allows for float values in there
  if ( roundToUnit )
  {
    rescaled = std::round( rescaled );
  }
  return rescaled;
}

QPointF QgsSymbolLayerUtils::rescaleUom( QPointF point, Qgis::RenderUnit unit, const QVariantMap &props )
{
  const double x = rescaleUom( point.x(), unit, props );
  const double y = rescaleUom( point.y(), unit, props );
  return QPointF( x, y );
}

QVector<qreal> QgsSymbolLayerUtils::rescaleUom( const QVector<qreal> &array, Qgis::RenderUnit unit, const QVariantMap &props )
{
  QVector<qreal> result;
  QVector<qreal>::const_iterator it = array.constBegin();
  for ( ; it != array.constEnd(); ++it )
  {
    result.append( rescaleUom( *it, unit, props ) );
  }
  return result;
}

void QgsSymbolLayerUtils::applyScaleDependency( QDomDocument &doc, QDomElement &ruleElem, QVariantMap &props )
{
  if ( !props.value( u"scaleMinDenom"_s, QString() ).toString().isEmpty() )
  {
    QDomElement scaleMinDenomElem = doc.createElement( u"se:MinScaleDenominator"_s );
    scaleMinDenomElem.appendChild( doc.createTextNode( qgsDoubleToString( props.value( u"scaleMinDenom"_s ).toString().toDouble() ) ) );
    ruleElem.appendChild( scaleMinDenomElem );
  }

  if ( !props.value( u"scaleMaxDenom"_s, QString() ).toString().isEmpty() )
  {
    QDomElement scaleMaxDenomElem = doc.createElement( u"se:MaxScaleDenominator"_s );
    scaleMaxDenomElem.appendChild( doc.createTextNode( qgsDoubleToString( props.value( u"scaleMaxDenom"_s ).toString().toDouble() ) ) );
    ruleElem.appendChild( scaleMaxDenomElem );
  }
}

void QgsSymbolLayerUtils::mergeScaleDependencies( double mScaleMinDenom, double mScaleMaxDenom, QVariantMap &props )
{
  if ( !qgsDoubleNear( mScaleMinDenom, 0 ) )
  {
    bool ok;
    const double parentScaleMinDenom = props.value( u"scaleMinDenom"_s, u"0"_s ).toString().toDouble( &ok );
    if ( !ok || parentScaleMinDenom <= 0 )
      props[ u"scaleMinDenom"_s] = QString::number( mScaleMinDenom );
    else
      props[ u"scaleMinDenom"_s] = QString::number( std::max( parentScaleMinDenom, mScaleMinDenom ) );
  }

  if ( !qgsDoubleNear( mScaleMaxDenom, 0 ) )
  {
    bool ok;
    const double parentScaleMaxDenom = props.value( u"scaleMaxDenom"_s, u"0"_s ).toString().toDouble( &ok );
    if ( !ok || parentScaleMaxDenom <= 0 )
      props[ u"scaleMaxDenom"_s] = QString::number( mScaleMaxDenom );
    else
      props[ u"scaleMaxDenom"_s] = QString::number( std::min( parentScaleMaxDenom, mScaleMaxDenom ) );
  }
}

double QgsSymbolLayerUtils::sizeInPixelsFromSldUom( const QString &uom, double size )
{
  double scale = 1.0;

  if ( uom == "http://www.opengeospatial.org/se/units/metre"_L1 )
  {
    scale = 1.0 / 0.00028; // from meters to pixels
  }
  else if ( uom == "http://www.opengeospatial.org/se/units/foot"_L1 )
  {
    scale = 304.8 / 0.28; // from feet to pixels
  }
  else
  {
    scale = 1.0; // from pixels to pixels (default unit)
  }

  return size * scale;
}

QSet<const QgsSymbolLayer *> QgsSymbolLayerUtils::toSymbolLayerPointers( const QgsFeatureRenderer *renderer, const QSet<QgsSymbolLayerId> &symbolLayerIds )
{
  Q_NOWARN_DEPRECATED_PUSH
  class SymbolLayerVisitor : public QgsStyleEntityVisitorInterface
  {
    public:
      SymbolLayerVisitor( const QSet<QgsSymbolLayerId> &layerIds )
        : mSymbolLayerIds( layerIds )
      {}

      bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
      {
        if ( node.type == QgsStyleEntityVisitorInterface::NodeType::SymbolRule )
        {
          mCurrentRuleKey = node.identifier;
          return true;
        }
        return false;
      }

      void visitSymbol( const QgsSymbol *symbol, const QString &identifier, QVector<int> rootPath )
      {
        for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
        {
          QVector<int> indexPath = rootPath;
          indexPath.append( idx );
          const QgsSymbolLayer *sl = symbol->symbolLayer( idx );
          Q_NOWARN_DEPRECATED_PUSH
          if ( mSymbolLayerIds.contains( QgsSymbolLayerId( mCurrentRuleKey + identifier, indexPath ) ) )
          {
            mSymbolLayers.insert( sl );
          }
          Q_NOWARN_DEPRECATED_POP

          const QgsSymbol *subSymbol = const_cast<QgsSymbolLayer *>( sl )->subSymbol();
          if ( subSymbol )
            visitSymbol( subSymbol, identifier, indexPath );
        }
      }

      bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
      {
        if ( leaf.entity && leaf.entity->type() == QgsStyle::SymbolEntity )
        {
          auto symbolEntity = static_cast<const QgsStyleSymbolEntity *>( leaf.entity );
          if ( symbolEntity->symbol() )
          {
            visitSymbol( symbolEntity->symbol(), leaf.identifier, {} );
          }
        }
        return true;
      }

      QString mCurrentRuleKey;
      const QSet<QgsSymbolLayerId> &mSymbolLayerIds;
      QSet<const QgsSymbolLayer *> mSymbolLayers;
  };
  Q_NOWARN_DEPRECATED_POP

  SymbolLayerVisitor visitor( symbolLayerIds );
  renderer->accept( &visitor );
  return visitor.mSymbolLayers;
}

double QgsSymbolLayerUtils::rendererFrameRate( const QgsFeatureRenderer *renderer )
{
  class SymbolRefreshRateVisitor : public QgsStyleEntityVisitorInterface
  {
    public:
      SymbolRefreshRateVisitor()
      {}

      bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
      {
        if ( node.type == QgsStyleEntityVisitorInterface::NodeType::SymbolRule )
        {
          return true;
        }
        return false;
      }

      void visitSymbol( const QgsSymbol *symbol )
      {
        // symbol may be marked as animated on a symbol level (e.g. when it implements animation
        // via data defined properties)
        if ( symbol->animationSettings().isAnimated() )
        {
          if ( symbol->animationSettings().frameRate() > refreshRate )
            refreshRate = symbol->animationSettings().frameRate();
        }
        for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
        {
          const QgsSymbolLayer *sl = symbol->symbolLayer( idx );
          if ( const QgsAnimatedMarkerSymbolLayer *animatedMarker = dynamic_cast< const QgsAnimatedMarkerSymbolLayer *>( sl ) )
          {
            // this is a bit of a short cut -- if a symbol has multiple layers with different frame rates,
            // there's no guarantee that they will be even multiples of each other! But given we are looking for
            // a single frame rate for a whole renderer, it's an acceptable compromise...
            if ( refreshRate == -1 || ( animatedMarker->frameRate() > refreshRate ) )
              refreshRate = animatedMarker->frameRate();
          }

          if ( const QgsSymbol *subSymbol = const_cast<QgsSymbolLayer *>( sl )->subSymbol() )
            visitSymbol( subSymbol );
        }
      }

      bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
      {
        if ( leaf.entity && leaf.entity->type() == QgsStyle::SymbolEntity )
        {
          if ( QgsSymbol *symbol = qgis::down_cast<const QgsStyleSymbolEntity *>( leaf.entity )->symbol() )
          {
            visitSymbol( symbol );
          }
        }
        return true;
      }

      double refreshRate = -1;
  };

  SymbolRefreshRateVisitor visitor;
  renderer->accept( &visitor );
  return visitor.refreshRate;
}

std::unique_ptr< QgsSymbol > QgsSymbolLayerUtils::restrictedSizeSymbol( const QgsSymbol *s, double minSize, double maxSize, QgsRenderContext *context, double &width, double &height, bool *ok )
{
  if ( !s || !context )
  {
    return nullptr;
  }

  if ( ok )
    *ok = true;

  const QgsSymbolLayerList sls = s->symbolLayers();
  for ( const QgsSymbolLayer *sl : std::as_const( sls ) )
  {
    // geometry generators involved, there is no way to get a restricted size symbol
    if ( sl->type() == Qgis::SymbolType::Hybrid )
    {
      if ( ok )
        *ok = false;

      return nullptr;
    }
  }

  double size;
  const QgsMarkerSymbol *markerSymbol = dynamic_cast<const QgsMarkerSymbol *>( s );
  const QgsLineSymbol *lineSymbol = dynamic_cast<const QgsLineSymbol *>( s );
  if ( markerSymbol )
  {
    size = markerSymbol->size( *context );
  }
  else if ( lineSymbol )
  {
    size = lineSymbol->width( *context );
  }
  else
  {
    // cannot return a size restricted symbol but we assume there is no need
    // for one as the rendering will be done in the given size (different from geometry
    // generator where rendering will bleed outside the given area
    return nullptr;
  }

  size /= context->scaleFactor();

  if ( minSize > 0 && size < minSize )
  {
    size = minSize;
  }
  else if ( maxSize > 0 && size > maxSize )
  {
    size = maxSize;
  }
  else
  {
    // no need to restricted size symbol
    return nullptr;
  }

  if ( markerSymbol )
  {
    std::unique_ptr< QgsMarkerSymbol > ms( markerSymbol->clone() );
    ms->setSize( size );
    ms->setSizeUnit( Qgis::RenderUnit::Millimeters );
    width = size;
    height = size;
    return ms;
  }
  else if ( lineSymbol )
  {
    std::unique_ptr< QgsLineSymbol > ls( lineSymbol->clone() );
    ls->setWidth( size );
    ls->setWidthUnit( Qgis::RenderUnit::Millimeters );
    height = size;
    return ls;
  }

  return nullptr;
}

QgsStringMap QgsSymbolLayerUtils::evaluatePropertiesMap( const QMap<QString, QgsProperty> &propertiesMap, const QgsExpressionContext &context )
{
  QgsStringMap properties;
  QMap<QString, QgsProperty>::const_iterator paramIt = propertiesMap.constBegin();
  for ( ; paramIt != propertiesMap.constEnd(); ++paramIt )
  {
    properties.insert( paramIt.key(), paramIt.value().valueAsString( context ) );
  }
  return properties;
}

QSize QgsSymbolLayerUtils::tileSize( int width, int height, double &angleRad )
{

  angleRad = std::fmod( angleRad, M_PI * 2 );

  if ( angleRad < 0 )
  {
    angleRad += M_PI * 2;
  }

  // tan with rational sin/cos
  struct rationalTangent
  {
    int p; // numerator
    int q; // denominator
    double angle; // "good" angle
  };

#if 0

  // This list is more granular (approx 1 degree steps) but some
  // values can lead to huge tiles
  // List of "good" angles from 0 to PI/2
  static const QList<rationalTangent> __rationalTangents
  {
    { 1, 57, 0.01754206006 },
    { 3, 86, 0.03486958155 },
    { 1, 19, 0.05258306161 },
    { 3, 43, 0.06965457373 },
    { 7, 80, 0.08727771295 },
    { 2, 19, 0.1048769387 },
    { 7, 57, 0.1221951707 },
    { 9, 64, 0.1397088743 },
    { 13, 82, 0.157228051 },
    { 3, 17, 0.174672199 },
    { 7, 36, 0.1920480172 },
    { 17, 80, 0.209385393 },
    { 3, 13, 0.2267988481 },
    { 1, 4, 0.2449786631 },
    { 26, 97, 0.2618852647 },
    { 27, 94, 0.2797041525 },
    { 26, 85, 0.2968446734 },
    { 13, 40, 0.3142318991 },
    { 21, 61, 0.3315541619 },
    { 4, 11, 0.3487710036 },
    { 38, 99, 0.3664967859 },
    { 40, 99, 0.383984624 },
    { 31, 73, 0.4015805401 },
    { 41, 92, 0.4192323938 },
    { 7, 15, 0.4366271598 },
    { 20, 41, 0.4538440015 },
    { 27, 53, 0.4711662643 },
    { 42, 79, 0.4886424026 },
    { 51, 92, 0.5061751436 },
    { 56, 97, 0.5235757641 },
    { 3, 5, 0.5404195003 },
    { 5, 8, 0.5585993153 },
    { 50, 77, 0.5759185996 },
    { 29, 43, 0.5933501462 },
    { 7, 10, 0.6107259644 },
    { 69, 95, 0.6281701124 },
    { 52, 69, 0.6458159195 },
    { 25, 32, 0.6632029927 },
    { 17, 21, 0.6805212247 },
    { 73, 87, 0.6981204504 },
    { 73, 84, 0.7154487784 },
    { 9, 10, 0.7328151018 },
    { 83, 89, 0.7505285818 },
    { 28, 29, 0.7678561033 },
    { 1, 1, 0.7853981634 },
    { 29, 28, 0.8029402235 },
    { 89, 83, 0.820267745 },
    { 10, 9, 0.837981225 },
    { 107, 93, 0.855284165 },
    { 87, 73, 0.8726758763 },
    { 121, 98, 0.8900374031 },
    { 32, 25, 0.9075933341 },
    { 69, 52, 0.9249804073 },
    { 128, 93, 0.9424647244 },
    { 10, 7, 0.9600703624 },
    { 43, 29, 0.9774461806 },
    { 77, 50, 0.9948777272 },
    { 8, 5, 1.012197011 },
    { 163, 98, 1.029475114 },
    { 168, 97, 1.047174539 },
    { 175, 97, 1.064668696 },
    { 126, 67, 1.082075603 },
    { 157, 80, 1.099534652 },
    { 203, 99, 1.117049384 },
    { 193, 90, 1.134452855 },
    { 146, 65, 1.151936673 },
    { 139, 59, 1.169382787 },
    { 99, 40, 1.186811703 },
    { 211, 81, 1.204257817 },
    { 272, 99, 1.221730164 },
    { 273, 94, 1.239188479 },
    { 277, 90, 1.25664606 },
    { 157, 48, 1.274088705 },
    { 279, 80, 1.291550147 },
    { 362, 97, 1.308990773 },
    { 373, 93, 1.326448578 },
    { 420, 97, 1.343823596 },
    { 207, 44, 1.361353157 },
    { 427, 83, 1.378810994 },
    { 414, 73, 1.396261926 },
    { 322, 51, 1.413716057 },
    { 185, 26, 1.431170275 },
    { 790, 97, 1.448623034 },
    { 333, 35, 1.466075711 },
    { 1063, 93, 1.483530284 },
    { 1330, 93, 1.500985147 },
    { 706, 37, 1.518436297 },
    { 315, 11, 1.535889876 },
    { 3953, 69, 1.553343002 },
  };
#endif

  // Optimized "good" angles list, it produces small tiles but
  // it has approximately 10 degrees steps
  static const QList<rationalTangent> rationalTangents
  {
    { 1, 10, qDegreesToRadians( 5.71059 ) },
    { 1, 5, qDegreesToRadians( 11.3099 ) },
    { 1, 4, qDegreesToRadians( 14.0362 ) },
    { 1, 4, qDegreesToRadians( 18.4349 ) },
    { 1, 2, qDegreesToRadians( 26.5651 ) },
    { 2, 3, qDegreesToRadians( 33.6901 ) },
    { 1, 1, qDegreesToRadians( 45.0 ) },
    { 3, 2, qDegreesToRadians( 56.3099 ) },
    { 2, 1, qDegreesToRadians( 63.4349 ) },
    { 3, 1, qDegreesToRadians( 71.5651 ) },
    { 4, 1, qDegreesToRadians( 75.9638 ) },
    { 10, 1, qDegreesToRadians( 84.2894 ) },
  };

  const int quadrant { static_cast<int>( angleRad / M_PI_2 ) };
  Q_ASSERT( quadrant >= 0 && quadrant <= 3 );

  QSize tileSize;

  switch ( quadrant )
  {
    case 0:
    {
      break;
    }
    case 1:
    {
      angleRad -= M_PI / 2;
      break;
    }
    case 2:
    {
      angleRad -= M_PI;
      break;
    }
    case 3:
    {
      angleRad -= M_PI + M_PI_2;
      break;
    }
  }

  if ( qgsDoubleNear( angleRad, 0, 10E-3 ) )
  {
    angleRad = 0;
    tileSize.setWidth( width );
    tileSize.setHeight( height );
  }
  else if ( qgsDoubleNear( angleRad, M_PI_2, 10E-3 ) )
  {
    angleRad = M_PI_2;
    tileSize.setWidth( height );
    tileSize.setHeight( width );
  }
  else
  {

    int rTanIdx = 0;

    for ( int idx = 0; idx < rationalTangents.count(); ++idx )
    {
      const auto item = rationalTangents.at( idx );
      if ( qgsDoubleNear( item.angle, angleRad, 10E-3 ) || item.angle > angleRad )
      {
        rTanIdx = idx;
        break;
      }
    }

    const rationalTangent bTan { rationalTangents.at( rTanIdx ) };
    angleRad = bTan.angle;
    const double k { bTan.q *height *width / std::cos( angleRad ) };
    const int hcfH { std::gcd( bTan.p * height, bTan.q * width ) };
    const int hcfW { std::gcd( bTan.q * height, bTan.p * width ) };
    const int W1 { static_cast<int>( std::round( k / hcfW ) ) };
    const int H1 { static_cast<int>( std::round( k / hcfH ) ) };
    tileSize.setWidth( W1 );
    tileSize.setHeight( H1 );
  }

  switch ( quadrant )
  {
    case 0:
    {
      break;
    }
    case 1:
    {
      angleRad += M_PI / 2;
      const int h { tileSize.height() };
      tileSize.setHeight( tileSize.width() );
      tileSize.setWidth( h );
      break;
    }
    case 2:
    {
      angleRad += M_PI;
      break;
    }
    case 3:
    {
      angleRad += M_PI + M_PI_2;
      const int h { tileSize.height() };
      tileSize.setHeight( tileSize.width() );
      tileSize.setWidth( h );
      break;
    }
  }

  return tileSize;
}

template <typename Functor>
void changeSymbolLayerIds( QgsSymbolLayer *sl, Functor &&generateId )
{
  sl->setId( generateId() );

  // recurse over sub symbols
  QgsSymbol *subSymbol = sl->subSymbol();
  if ( subSymbol )
    changeSymbolLayerIds( subSymbol, generateId );
}

template <typename Functor>
void changeSymbolLayerIds( QgsSymbol *symbol, Functor &&generateId )
{
  if ( !symbol )
    return;

  for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
    changeSymbolLayerIds( symbol->symbolLayer( idx ), generateId );
}

void QgsSymbolLayerUtils::clearSymbolLayerIds( QgsSymbol *symbol )
{
  changeSymbolLayerIds( symbol, []() { return QString(); } );
}

void QgsSymbolLayerUtils::clearSymbolLayerIds( QgsSymbolLayer *symbolLayer )
{
  changeSymbolLayerIds( symbolLayer, []() { return QString(); } );
}

void QgsSymbolLayerUtils::resetSymbolLayerIds( QgsSymbolLayer *symbolLayer )
{
  changeSymbolLayerIds( symbolLayer, []() { return QUuid::createUuid().toString(); } );
}

void QgsSymbolLayerUtils::clearSymbolLayerMasks( QgsSymbol *symbol )
{
  if ( !symbol )
    return;

  for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
  {
    if ( QgsMaskMarkerSymbolLayer *maskSl = dynamic_cast<QgsMaskMarkerSymbolLayer *>( symbol->symbolLayer( idx ) ) )
    {
      maskSl->clearMasks();

      // recurse over sub symbols
      if ( QgsSymbol *subSymbol = maskSl->subSymbol() )
      {
        clearSymbolLayerMasks( subSymbol );
      }
    }
  }
}

QVector<QgsGeometry> QgsSymbolLayerUtils::collectSymbolLayerClipGeometries( const QgsRenderContext &context, const QString &symbolLayerId, const QRectF &bounds )
{
  QVector<QgsGeometry> clipGeometries = context.symbolLayerClipGeometries( symbolLayerId );
  if ( clipGeometries.empty() )
    return {};

  if ( bounds.isNull() )
    return clipGeometries;

  const QgsRectangle boundsRect = QgsRectangle( bounds );

  clipGeometries.erase(
    std::remove_if( clipGeometries.begin(), clipGeometries.end(), [&boundsRect]( const QgsGeometry & geometry )
  {
    return !geometry.boundingBoxIntersects( boundsRect );
  } ), clipGeometries.end() );

  return clipGeometries;
}

void QgsSymbolLayerUtils::resetSymbolLayerIds( QgsSymbol *symbol )
{
  changeSymbolLayerIds( symbol, []() { return QUuid::createUuid().toString(); } );
}
