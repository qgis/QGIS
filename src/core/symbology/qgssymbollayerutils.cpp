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

#include "qgssymbollayer.h"
#include "qgssymbollayerregistry.h"
#include "qgssymbol.h"
#include "qgscolorramp.h"
#include "qgsexpression.h"
#include "qgsexpressionnode.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsapplication.h"
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgsogcutils.h"
#include "qgslogger.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgsunittypes.h"

#include <QColor>
#include <QFont>
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QIcon>
#include <QPainter>
#include <QSettings>
#include <QRegExp>
#include <QPicture>

#define POINTS_TO_MM 2.83464567

QString QgsSymbolLayerUtils::encodeColor( const QColor &color )
{
  return QStringLiteral( "%1,%2,%3,%4" ).arg( color.red() ).arg( color.green() ).arg( color.blue() ).arg( color.alpha() );
}

QColor QgsSymbolLayerUtils::decodeColor( const QString &str )
{
  QStringList lst = str.split( ',' );
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
  QString result;
  result.sprintf( "%.2g", alpha / 255.0 );
  return result;
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
      return QStringLiteral( "normal" );
    case QFont::StyleItalic:
      return QStringLiteral( "italic" );
    case QFont::StyleOblique:
      return QStringLiteral( "oblique" );
    default:
      return QString();
  }
}

QFont::Style QgsSymbolLayerUtils::decodeSldFontStyle( const QString &str )
{
  if ( str == QLatin1String( "normal" ) ) return QFont::StyleNormal;
  if ( str == QLatin1String( "italic" ) ) return QFont::StyleItalic;
  if ( str == QLatin1String( "oblique" ) ) return QFont::StyleOblique;
  return QFont::StyleNormal;
}

QString QgsSymbolLayerUtils::encodeSldFontWeight( int weight )
{
  if ( weight == 50 ) return QStringLiteral( "normal" );
  if ( weight == 75 ) return QStringLiteral( "bold" );

  // QFont::Weight is between 0 and 99
  // CSS font-weight is between 100 and 900
  if ( weight < 0 ) return QStringLiteral( "100" );
  if ( weight > 99 ) return QStringLiteral( "900" );
  return QString::number( weight * 800 / 99 + 100 );
}

int QgsSymbolLayerUtils::decodeSldFontWeight( const QString &str )
{
  bool ok;
  int weight = str.toInt( &ok );
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
      return QStringLiteral( "no" );
    case Qt::SolidLine:
      return QStringLiteral( "solid" );
    case Qt::DashLine:
      return QStringLiteral( "dash" );
    case Qt::DotLine:
      return QStringLiteral( "dot" );
    case Qt::DashDotLine:
      return QStringLiteral( "dash dot" );
    case Qt::DashDotDotLine:
      return QStringLiteral( "dash dot dot" );
    default:
      return QStringLiteral( "???" );
  }
}

Qt::PenStyle QgsSymbolLayerUtils::decodePenStyle( const QString &str )
{
  if ( str == QLatin1String( "no" ) ) return Qt::NoPen;
  if ( str == QLatin1String( "solid" ) ) return Qt::SolidLine;
  if ( str == QLatin1String( "dash" ) ) return Qt::DashLine;
  if ( str == QLatin1String( "dot" ) ) return Qt::DotLine;
  if ( str == QLatin1String( "dash dot" ) ) return Qt::DashDotLine;
  if ( str == QLatin1String( "dash dot dot" ) ) return Qt::DashDotDotLine;
  return Qt::SolidLine;
}

QString QgsSymbolLayerUtils::encodePenJoinStyle( Qt::PenJoinStyle style )
{
  switch ( style )
  {
    case Qt::BevelJoin:
      return QStringLiteral( "bevel" );
    case Qt::MiterJoin:
      return QStringLiteral( "miter" );
    case Qt::RoundJoin:
      return QStringLiteral( "round" );
    default:
      return QStringLiteral( "???" );
  }
}

Qt::PenJoinStyle QgsSymbolLayerUtils::decodePenJoinStyle( const QString &str )
{
  if ( str == QLatin1String( "bevel" ) ) return Qt::BevelJoin;
  if ( str == QLatin1String( "miter" ) ) return Qt::MiterJoin;
  if ( str == QLatin1String( "round" ) ) return Qt::RoundJoin;
  return Qt::BevelJoin;
}

QString QgsSymbolLayerUtils::encodeSldLineJoinStyle( Qt::PenJoinStyle style )
{
  switch ( style )
  {
    case Qt::BevelJoin:
      return QStringLiteral( "bevel" );
    case Qt::MiterJoin:
      return QStringLiteral( "mitre" );  //#spellok
    case Qt::RoundJoin:
      return QStringLiteral( "round" );
    default:
      return QString();
  }
}

Qt::PenJoinStyle QgsSymbolLayerUtils::decodeSldLineJoinStyle( const QString &str )
{
  if ( str == QLatin1String( "bevel" ) ) return Qt::BevelJoin;
  if ( str == QLatin1String( "mitre" ) ) return Qt::MiterJoin;  //#spellok
  if ( str == QLatin1String( "round" ) ) return Qt::RoundJoin;
  return Qt::BevelJoin;
}

QString QgsSymbolLayerUtils::encodePenCapStyle( Qt::PenCapStyle style )
{
  switch ( style )
  {
    case Qt::SquareCap:
      return QStringLiteral( "square" );
    case Qt::FlatCap:
      return QStringLiteral( "flat" );
    case Qt::RoundCap:
      return QStringLiteral( "round" );
    default:
      return QStringLiteral( "???" );
  }
}

Qt::PenCapStyle QgsSymbolLayerUtils::decodePenCapStyle( const QString &str )
{
  if ( str == QLatin1String( "square" ) ) return Qt::SquareCap;
  if ( str == QLatin1String( "flat" ) ) return Qt::FlatCap;
  if ( str == QLatin1String( "round" ) ) return Qt::RoundCap;
  return Qt::SquareCap;
}

QString QgsSymbolLayerUtils::encodeSldLineCapStyle( Qt::PenCapStyle style )
{
  switch ( style )
  {
    case Qt::SquareCap:
      return QStringLiteral( "square" );
    case Qt::FlatCap:
      return QStringLiteral( "butt" );
    case Qt::RoundCap:
      return QStringLiteral( "round" );
    default:
      return QString();
  }
}

Qt::PenCapStyle QgsSymbolLayerUtils::decodeSldLineCapStyle( const QString &str )
{
  if ( str == QLatin1String( "square" ) ) return Qt::SquareCap;
  if ( str == QLatin1String( "butt" ) ) return Qt::FlatCap;
  if ( str == QLatin1String( "round" ) ) return Qt::RoundCap;
  return Qt::SquareCap;
}

QString QgsSymbolLayerUtils::encodeBrushStyle( Qt::BrushStyle style )
{
  switch ( style )
  {
    case Qt::SolidPattern :
      return QStringLiteral( "solid" );
    case Qt::HorPattern :
      return QStringLiteral( "horizontal" );
    case Qt::VerPattern :
      return QStringLiteral( "vertical" );
    case Qt::CrossPattern :
      return QStringLiteral( "cross" );
    case Qt::BDiagPattern :
      return QStringLiteral( "b_diagonal" );
    case Qt::FDiagPattern :
      return  QStringLiteral( "f_diagonal" );
    case Qt::DiagCrossPattern :
      return QStringLiteral( "diagonal_x" );
    case Qt::Dense1Pattern  :
      return QStringLiteral( "dense1" );
    case Qt::Dense2Pattern  :
      return QStringLiteral( "dense2" );
    case Qt::Dense3Pattern  :
      return QStringLiteral( "dense3" );
    case Qt::Dense4Pattern  :
      return QStringLiteral( "dense4" );
    case Qt::Dense5Pattern  :
      return QStringLiteral( "dense5" );
    case Qt::Dense6Pattern  :
      return QStringLiteral( "dense6" );
    case Qt::Dense7Pattern  :
      return QStringLiteral( "dense7" );
    case Qt::NoBrush :
      return QStringLiteral( "no" );
    default:
      return QStringLiteral( "???" );
  }
}

Qt::BrushStyle QgsSymbolLayerUtils::decodeBrushStyle( const QString &str )
{
  if ( str == QLatin1String( "solid" ) ) return Qt::SolidPattern;
  if ( str == QLatin1String( "horizontal" ) ) return Qt::HorPattern;
  if ( str == QLatin1String( "vertical" ) ) return Qt::VerPattern;
  if ( str == QLatin1String( "cross" ) ) return Qt::CrossPattern;
  if ( str == QLatin1String( "b_diagonal" ) ) return Qt::BDiagPattern;
  if ( str == QLatin1String( "f_diagonal" ) ) return Qt::FDiagPattern;
  if ( str == QLatin1String( "diagonal_x" ) ) return Qt::DiagCrossPattern;
  if ( str == QLatin1String( "dense1" ) ) return Qt::Dense1Pattern;
  if ( str == QLatin1String( "dense2" ) ) return Qt::Dense2Pattern;
  if ( str == QLatin1String( "dense3" ) ) return Qt::Dense3Pattern;
  if ( str == QLatin1String( "dense4" ) ) return Qt::Dense4Pattern;
  if ( str == QLatin1String( "dense5" ) ) return Qt::Dense5Pattern;
  if ( str == QLatin1String( "dense6" ) ) return Qt::Dense6Pattern;
  if ( str == QLatin1String( "dense7" ) ) return Qt::Dense7Pattern;
  if ( str == QLatin1String( "no" ) ) return Qt::NoBrush;
  return Qt::SolidPattern;
}

QString QgsSymbolLayerUtils::encodeSldBrushStyle( Qt::BrushStyle style )
{
  switch ( style )
  {
    case Qt::CrossPattern:
      return QStringLiteral( "cross" );
    case Qt::DiagCrossPattern:
      return QStringLiteral( "x" );

    /* The following names are taken from the presentation "GeoServer
     * Cartographic Rendering" by Andrea Aime at the FOSS4G 2010.
     * (see http://2010.foss4g.org/presentations/3588.pdf)
     */
    case Qt::HorPattern:
      return QStringLiteral( "horline" );
    case Qt::VerPattern:
      return QStringLiteral( "line" );
    case Qt::BDiagPattern:
      return QStringLiteral( "slash" );
    case Qt::FDiagPattern:
      return QStringLiteral( "backslash" );

    /* define the other names following the same pattern used above */
    case Qt::Dense1Pattern:
    case Qt::Dense2Pattern:
    case Qt::Dense3Pattern:
    case Qt::Dense4Pattern:
    case Qt::Dense5Pattern:
    case Qt::Dense6Pattern:
    case Qt::Dense7Pattern:
      return QStringLiteral( "brush://%1" ).arg( encodeBrushStyle( style ) );

    default:
      return QString();
  }
}

Qt::BrushStyle QgsSymbolLayerUtils::decodeSldBrushStyle( const QString &str )
{
  if ( str == QLatin1String( "horline" ) ) return Qt::HorPattern;
  if ( str == QLatin1String( "line" ) ) return Qt::VerPattern;
  if ( str == QLatin1String( "cross" ) ) return Qt::CrossPattern;
  if ( str == QLatin1String( "slash" ) ) return Qt::BDiagPattern;
  if ( str == QLatin1String( "backshash" ) ) return Qt::FDiagPattern;
  if ( str == QLatin1String( "x" ) ) return Qt::DiagCrossPattern;

  if ( str.startsWith( QLatin1String( "brush://" ) ) )
    return decodeBrushStyle( str.mid( 8 ) );

  return Qt::NoBrush;
}

QgsArrowSymbolLayer::HeadType QgsSymbolLayerUtils::decodeArrowHeadType( const QVariant &value, bool *ok )
{
  if ( ok )
    *ok = true;

  bool intOk = false;
  QString s = value.toString().toLower().trimmed();
  if ( s == QLatin1String( "single" ) )
    return QgsArrowSymbolLayer::HeadSingle;
  else if ( s == QLatin1String( "reversed" ) )
    return QgsArrowSymbolLayer::HeadReversed;
  else if ( s == QLatin1String( "double" ) )
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
  QString s = value.toString().toLower().trimmed();
  if ( s == QLatin1String( "plain" ) )
    return QgsArrowSymbolLayer::ArrowPlain;
  else if ( s == QLatin1String( "lefthalf" ) )
    return QgsArrowSymbolLayer::ArrowLeftHalf;
  else if ( s == QLatin1String( "righthalf" ) )
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

QString QgsSymbolLayerUtils::encodePoint( QPointF point )
{
  return QStringLiteral( "%1,%2" ).arg( qgsDoubleToString( point.x() ), qgsDoubleToString( point.y() ) );
}

QPointF QgsSymbolLayerUtils::decodePoint( const QString &str )
{
  QStringList lst = str.split( ',' );
  if ( lst.count() != 2 )
    return QPointF( 0, 0 );
  return QPointF( lst[0].toDouble(), lst[1].toDouble() );
}

QString QgsSymbolLayerUtils::encodeSize( QSizeF size )
{
  return QStringLiteral( "%1,%2" ).arg( qgsDoubleToString( size.width() ), qgsDoubleToString( size.height() ) );
}

QSizeF QgsSymbolLayerUtils::decodeSize( const QString &string )
{
  QStringList lst = string.split( ',' );
  if ( lst.count() != 2 )
    return QSizeF( 0, 0 );
  return QSizeF( lst[0].toDouble(), lst[1].toDouble() );
}

QString QgsSymbolLayerUtils::encodeMapUnitScale( const QgsMapUnitScale &mapUnitScale )
{
  return QStringLiteral( "3x:%1,%2,%3,%4,%5,%6" ).arg( qgsDoubleToString( mapUnitScale.minScale ),
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
  if ( str.startsWith( QLatin1String( "3x:" ) ) )
  {
    v3 = true;
    QString chopped = str.mid( 3 );
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

QString QgsSymbolLayerUtils::encodeSldUom( QgsUnitTypes::RenderUnit unit, double *scaleFactor )
{
  switch ( unit )
  {
    case QgsUnitTypes::RenderMapUnits:
      if ( scaleFactor )
        *scaleFactor = 0.001; // from millimeters to meters
      return QStringLiteral( "http://www.opengeospatial.org/se/units/metre" );

    case QgsUnitTypes::RenderMillimeters:
    default:
      // pixel is the SLD default uom. The "standardized rendering pixel
      // size" is defined to be 0.28mm × 0.28mm (millimeters).
      if ( scaleFactor )
        *scaleFactor = 1 / 0.28;  // from millimeters to pixels

      // http://www.opengeospatial.org/sld/units/pixel
      return QString();
  }
}

QgsUnitTypes::RenderUnit QgsSymbolLayerUtils::decodeSldUom( const QString &str, double *scaleFactor )
{
  if ( str == QLatin1String( "http://www.opengeospatial.org/se/units/metre" ) )
  {
    if ( scaleFactor )
      *scaleFactor = 1000.0;  // from meters to millimeters
    return QgsUnitTypes::RenderMapUnits;
  }
  else if ( str == QLatin1String( "http://www.opengeospatial.org/se/units/foot" ) )
  {
    if ( scaleFactor )
      *scaleFactor = 304.8; // from feet to meters
    return QgsUnitTypes::RenderMapUnits;
  }

  // pixel is the SLD default uom. The "standardized rendering pixel
  // size" is defined to be 0.28mm x 0.28mm (millimeters).
  if ( scaleFactor )
    *scaleFactor = 1 / 0.00028; // from pixels to millimeters
  return QgsUnitTypes::RenderMillimeters;
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

  QStringList realList = s.split( ';' );
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

  QStringList realList = s.split( ' ' );
  QStringList::const_iterator it = realList.constBegin();
  for ( ; it != realList.constEnd(); ++it )
  {
    resultVector.append( it->toDouble() );
  }

  return resultVector;
}

QString QgsSymbolLayerUtils::encodeScaleMethod( QgsSymbol::ScaleMethod scaleMethod )
{
  QString encodedValue;

  switch ( scaleMethod )
  {
    case QgsSymbol::ScaleDiameter:
      encodedValue = QStringLiteral( "diameter" );
      break;
    case QgsSymbol::ScaleArea:
      encodedValue = QStringLiteral( "area" );
      break;
  }
  return encodedValue;
}

QgsSymbol::ScaleMethod QgsSymbolLayerUtils::decodeScaleMethod( const QString &str )
{
  QgsSymbol::ScaleMethod scaleMethod;

  if ( str == QLatin1String( "diameter" ) )
  {
    scaleMethod = QgsSymbol::ScaleDiameter;
  }
  else
  {
    scaleMethod = QgsSymbol::ScaleArea;
  }

  return scaleMethod;
}

QPainter::CompositionMode QgsSymbolLayerUtils::decodeBlendMode( const QString &s )
{
  if ( s.compare( QLatin1String( "Lighten" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Lighten;
  if ( s.compare( QLatin1String( "Screen" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Screen;
  if ( s.compare( QLatin1String( "Dodge" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_ColorDodge;
  if ( s.compare( QLatin1String( "Addition" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Plus;
  if ( s.compare( QLatin1String( "Darken" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Darken;
  if ( s.compare( QLatin1String( "Multiply" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Multiply;
  if ( s.compare( QLatin1String( "Burn" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_ColorBurn;
  if ( s.compare( QLatin1String( "Overlay" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Overlay;
  if ( s.compare( QLatin1String( "SoftLight" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_SoftLight;
  if ( s.compare( QLatin1String( "HardLight" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_HardLight;
  if ( s.compare( QLatin1String( "Difference" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Difference;
  if ( s.compare( QLatin1String( "Subtract" ), Qt::CaseInsensitive ) == 0 ) return QPainter::CompositionMode_Exclusion;
  return QPainter::CompositionMode_SourceOver; // "Normal"
}

QIcon QgsSymbolLayerUtils::symbolPreviewIcon( const QgsSymbol *symbol, QSize size, int padding )
{
  return QIcon( symbolPreviewPixmap( symbol, size, padding ) );
}

QPixmap QgsSymbolLayerUtils::symbolPreviewPixmap( const QgsSymbol *symbol, QSize size, int padding, QgsRenderContext *customContext )
{
  Q_ASSERT( symbol );
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );
  painter.setRenderHint( QPainter::Antialiasing );

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
    symbol_noDD->drawPreviewIcon( &painter, size, customContext );
  }
  else
  {
    std::unique_ptr<QgsSymbol> symbolClone( symbol->clone( ) );
    symbolClone->drawPreviewIcon( &painter, size, customContext );
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
    double layerMaxBleed = layer->estimateMaxBleed( context );
    maxBleed = layerMaxBleed > maxBleed ? layerMaxBleed : maxBleed;
  }

  return maxBleed;
}

QPicture QgsSymbolLayerUtils::symbolLayerPreviewPicture( const QgsSymbolLayer *layer, QgsUnitTypes::RenderUnit units, QSize size, const QgsMapUnitScale &scale )
{
  QPicture picture;
  QPainter painter;
  painter.begin( &picture );
  painter.setRenderHint( QPainter::Antialiasing );
  QgsRenderContext renderContext = QgsRenderContext::fromQPainter( &painter );
  renderContext.setForceVectorOutput( true );
  QgsSymbolRenderContext symbolContext( renderContext, units, 1.0, false, nullptr, nullptr, QgsFields(), scale );
  std::unique_ptr< QgsSymbolLayer > layerClone( layer->clone() );
  layerClone->drawPreviewIcon( symbolContext, size );
  painter.end();
  return picture;
}

QIcon QgsSymbolLayerUtils::symbolLayerPreviewIcon( const QgsSymbolLayer *layer, QgsUnitTypes::RenderUnit u, QSize size, const QgsMapUnitScale &scale )
{
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );
  painter.setRenderHint( QPainter::Antialiasing );
  QgsRenderContext renderContext = QgsRenderContext::fromQPainter( &painter );
  // build a minimal expression context
  QgsExpressionContext expContext;
  expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
  renderContext.setExpressionContext( expContext );

  QgsSymbolRenderContext symbolContext( renderContext, u, 1.0, false, nullptr, nullptr, QgsFields(), scale );
  std::unique_ptr< QgsSymbolLayer > layerClone( layer->clone() );
  layerClone->drawPreviewIcon( symbolContext, size );
  painter.end();
  return QIcon( pixmap );
}

QIcon QgsSymbolLayerUtils::colorRampPreviewIcon( QgsColorRamp *ramp, QSize size, int padding )
{
  return QIcon( colorRampPreviewPixmap( ramp, size, padding ) );
}

QPixmap QgsSymbolLayerUtils::colorRampPreviewPixmap( QgsColorRamp *ramp, QSize size, int padding )
{
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  // pixmap.fill( Qt::white ); // this makes the background white instead of transparent
  QPainter painter;
  painter.begin( &pixmap );

  //draw stippled background, for transparent images
  drawStippledBackground( &painter, QRect( padding, padding, size.width() - padding * 2, size.height() - padding  * 2 ) );

  // antialising makes the colors duller, and no point in antialiasing a color ramp
  // painter.setRenderHint( QPainter::Antialiasing );
  for ( int i = 0; i < size.width(); i++ )
  {
    QPen pen( ramp->color( static_cast< double >( i ) / size.width() ) );
    painter.setPen( pen );
    painter.drawLine( i, 0 + padding, i, size.height() - 1 - padding );
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
  QImage img( pixDataRGB, 2, 2, 8, QImage::Format_ARGB32 );
  // scale it to rect so at least 5 patterns are shown
  int width = ( rect.width() < rect.height() ) ?
              rect.width() / 2.5 : rect.height() / 2.5;
  QPixmap pix = QPixmap::fromImage( img.scaled( width, width ) );
  // fill rect with texture
  QBrush brush;
  brush.setTexture( pix );
  painter->fillRect( rect, brush );
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

QList<QPolygonF> offsetLine( QPolygonF polyline, double dist, QgsWkbTypes::GeometryType geometryType )
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

  QgsGeometry tempGeometry = geometryType == QgsWkbTypes::PolygonGeometry ? QgsGeometry::fromPolygonXY( QgsPolygonXY() << tempPolyline ) : QgsGeometry::fromPolylineXY( tempPolyline );
  if ( !tempGeometry.isNull() )
  {
    int quadSegments = 0; // we want miter joins, not round joins
    double miterLimit = 2.0; // the default value in GEOS (5.0) allows for fairly sharp endings
    QgsGeometry offsetGeom;
    if ( geometryType == QgsWkbTypes::PolygonGeometry )
      offsetGeom = tempGeometry.buffer( -dist, quadSegments, QgsGeometry::CapFlat,
                                        QgsGeometry::JoinStyleMiter, miterLimit );
    else
      offsetGeom = tempGeometry.offsetCurve( dist, quadSegments, QgsGeometry::JoinStyleMiter, miterLimit );

    if ( !offsetGeom.isNull() )
    {
      tempGeometry = offsetGeom;

      if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == QgsWkbTypes::LineString )
      {
        QgsPolylineXY line = tempGeometry.asPolyline();
        resultLine.append( makeOffsetGeometry( line ) );
        return resultLine;
      }
      else if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == QgsWkbTypes::Polygon )
      {
        resultLine.append( makeOffsetGeometry( tempGeometry.asPolygon() ) );
        return resultLine;
      }
      else if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == QgsWkbTypes::MultiLineString )
      {
        QgsMultiPolylineXY tempMPolyline = tempGeometry.asMultiPolyline();
        resultLine.reserve( tempMPolyline.count() );
        for ( int part = 0; part < tempMPolyline.count(); ++part )
        {
          resultLine.append( makeOffsetGeometry( tempMPolyline[ part ] ) );
        }
        return resultLine;
      }
      else if ( QgsWkbTypes::flatType( tempGeometry.wkbType() ) == QgsWkbTypes::MultiPolygon )
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


QgsSymbol *QgsSymbolLayerUtils::loadSymbol( const QDomElement &element, const QgsReadWriteContext &context )
{
  QgsSymbolLayerList layers;
  QDomNode layerNode = element.firstChild();

  while ( !layerNode.isNull() )
  {
    QDomElement e = layerNode.toElement();
    if ( !e.isNull() )
    {
      if ( e.tagName() != QLatin1String( "layer" ) )
      {
        QgsDebugMsg( "unknown tag " + e.tagName() );
      }
      else
      {
        QgsSymbolLayer *layer = loadSymbolLayer( e, context );

        if ( layer )
        {
          // Dealing with sub-symbols nested into a layer
          QDomElement s = e.firstChildElement( QStringLiteral( "symbol" ) );
          if ( !s.isNull() )
          {
            QgsSymbol *subSymbol = loadSymbol( s, context );
            bool res = layer->setSubSymbol( subSymbol );
            if ( !res )
            {
              QgsDebugMsg( "symbol layer refused subsymbol: " + s.attribute( "name" ) );
            }
          }
          layers.append( layer );
        }
      }
    }
    layerNode = layerNode.nextSibling();
  }

  if ( layers.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "no layers for symbol" ) );
    return nullptr;
  }

  QString symbolType = element.attribute( QStringLiteral( "type" ) );

  QgsSymbol *symbol = nullptr;
  if ( symbolType == QLatin1String( "line" ) )
    symbol = new QgsLineSymbol( layers );
  else if ( symbolType == QLatin1String( "fill" ) )
    symbol = new QgsFillSymbol( layers );
  else if ( symbolType == QLatin1String( "marker" ) )
    symbol = new QgsMarkerSymbol( layers );
  else
  {
    QgsDebugMsg( "unknown symbol type " + symbolType );
    return nullptr;
  }

  if ( element.hasAttribute( QStringLiteral( "outputUnit" ) ) )
  {
    symbol->setOutputUnit( QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "outputUnit" ) ) ) );
  }
  if ( element.hasAttribute( ( QStringLiteral( "mapUnitScale" ) ) ) )
  {
    QgsMapUnitScale mapUnitScale;
    double oldMin = element.attribute( QStringLiteral( "mapUnitMinScale" ), QStringLiteral( "0.0" ) ).toDouble();
    mapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = element.attribute( QStringLiteral( "mapUnitMaxScale" ), QStringLiteral( "0.0" ) ).toDouble();
    mapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
    symbol->setMapUnitScale( mapUnitScale );
  }
  symbol->setOpacity( element.attribute( QStringLiteral( "alpha" ), QStringLiteral( "1.0" ) ).toDouble() );
  symbol->setClipFeaturesToExtent( element.attribute( QStringLiteral( "clip_to_extent" ), QStringLiteral( "1" ) ).toInt() );
  symbol->setForceRHR( element.attribute( QStringLiteral( "force_rhr" ), QStringLiteral( "0" ) ).toInt() );
  return symbol;
}

QgsSymbolLayer *QgsSymbolLayerUtils::loadSymbolLayer( QDomElement &element, const QgsReadWriteContext &context )
{
  QString layerClass = element.attribute( QStringLiteral( "class" ) );
  bool locked = element.attribute( QStringLiteral( "locked" ) ).toInt();
  bool enabled = element.attribute( QStringLiteral( "enabled" ), QStringLiteral( "1" ) ).toInt();
  int pass = element.attribute( QStringLiteral( "pass" ) ).toInt();

  // parse properties
  QgsStringMap props = parseProperties( element );

  // if there are any paths stored in properties, convert them from relative to absolute
  QgsApplication::symbolLayerRegistry()->resolvePaths( layerClass, props, context.pathResolver(), false );

  QgsSymbolLayer *layer = nullptr;
  layer = QgsApplication::symbolLayerRegistry()->createSymbolLayer( layerClass, props );
  if ( layer )
  {
    layer->setLocked( locked );
    layer->setRenderingPass( pass );
    layer->setEnabled( enabled );

    //restore layer effect
    QDomElement effectElem = element.firstChildElement( QStringLiteral( "effect" ) );
    if ( !effectElem.isNull() )
    {
      layer->setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
    }

    // restore data defined properties
    QDomElement ddProps = element.firstChildElement( QStringLiteral( "data_defined_properties" ) );
    if ( !ddProps.isNull() )
    {
      layer->dataDefinedProperties().readXml( ddProps, QgsSymbolLayer::propertyDefinitions() );
    }

    return layer;
  }
  else
  {
    QgsDebugMsg( "unknown class " + layerClass );
    return nullptr;
  }
}

static QString _nameForSymbolType( QgsSymbol::SymbolType type )
{
  switch ( type )
  {
    case QgsSymbol::Line:
      return QStringLiteral( "line" );
    case QgsSymbol::Marker:
      return QStringLiteral( "marker" );
    case QgsSymbol::Fill:
      return QStringLiteral( "fill" );
    default:
      return QString();
  }
}

QDomElement QgsSymbolLayerUtils::saveSymbol( const QString &name, QgsSymbol *symbol, QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_ASSERT( symbol );
  QDomElement symEl = doc.createElement( QStringLiteral( "symbol" ) );
  symEl.setAttribute( QStringLiteral( "type" ), _nameForSymbolType( symbol->type() ) );
  symEl.setAttribute( QStringLiteral( "name" ), name );
  symEl.setAttribute( QStringLiteral( "alpha" ), QString::number( symbol->opacity() ) );
  symEl.setAttribute( QStringLiteral( "clip_to_extent" ), symbol->clipFeaturesToExtent() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  symEl.setAttribute( QStringLiteral( "force_rhr" ), symbol->forceRHR() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  //QgsDebugMsg( "num layers " + QString::number( symbol->symbolLayerCount() ) );

  for ( int i = 0; i < symbol->symbolLayerCount(); i++ )
  {
    QgsSymbolLayer *layer = symbol->symbolLayer( i );

    QDomElement layerEl = doc.createElement( QStringLiteral( "layer" ) );
    layerEl.setAttribute( QStringLiteral( "class" ), layer->layerType() );
    layerEl.setAttribute( QStringLiteral( "enabled" ), layer->enabled() );
    layerEl.setAttribute( QStringLiteral( "locked" ), layer->isLocked() );
    layerEl.setAttribute( QStringLiteral( "pass" ), layer->renderingPass() );

    QgsStringMap props = layer->properties();

    // if there are any paths in properties, convert them from absolute to relative
    QgsApplication::symbolLayerRegistry()->resolvePaths( layer->layerType(), props, context.pathResolver(), true );

    saveProperties( props, doc, layerEl );
    if ( !QgsPaintEffectRegistry::isDefaultStack( layer->paintEffect() ) )
      layer->paintEffect()->saveProperties( doc, layerEl );

    QDomElement ddProps = doc.createElement( QStringLiteral( "data_defined_properties" ) );
    layer->dataDefinedProperties().writeXml( ddProps, QgsSymbolLayer::propertyDefinitions() );
    layerEl.appendChild( ddProps );

    if ( layer->subSymbol() )
    {
      QString subname = QStringLiteral( "@%1@%2" ).arg( name ).arg( i );
      QDomElement subEl = saveSymbol( subname, layer->subSymbol(), doc, context );
      layerEl.appendChild( subEl );
    }
    symEl.appendChild( layerEl );
  }

  return symEl;
}

QString QgsSymbolLayerUtils::symbolProperties( QgsSymbol *symbol )
{
  QDomDocument doc( QStringLiteral( "qgis-symbol-definition" ) );
  QDomElement symbolElem = saveSymbol( QStringLiteral( "symbol" ), symbol, doc, QgsReadWriteContext() );
  QString props;
  QTextStream stream( &props );
  symbolElem.save( stream, -1 );
  return props;
}

bool QgsSymbolLayerUtils::createSymbolLayerListFromSld( QDomElement &element,
    QgsWkbTypes::GeometryType geomType,
    QgsSymbolLayerList &layers )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  if ( element.isNull() )
    return false;

  QgsSymbolLayer *l = nullptr;

  QString symbolizerName = element.localName();

  if ( symbolizerName == QLatin1String( "PointSymbolizer" ) )
  {
    // first check for Graphic element, nothing will be rendered if not found
    QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
    if ( graphicElem.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "Graphic element not found in PointSymbolizer" ) );
    }
    else
    {
      switch ( geomType )
      {
        case QgsWkbTypes::PolygonGeometry:
          // polygon layer and point symbolizer: draw poligon centroid
          l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "CentroidFill" ), element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::PointGeometry:
          // point layer and point symbolizer: use markers
          l = createMarkerLayerFromSld( element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::LineGeometry:
          // line layer and point symbolizer: draw central point
          l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "SimpleMarker" ), element );
          if ( l )
            layers.append( l );

          break;

        default:
          break;
      }
    }
  }

  if ( symbolizerName == QLatin1String( "LineSymbolizer" ) )
  {
    // check for Stroke element, nothing will be rendered if not found
    QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
    if ( strokeElem.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "Stroke element not found in LineSymbolizer" ) );
    }
    else
    {
      switch ( geomType )
      {
        case QgsWkbTypes::PolygonGeometry:
        case QgsWkbTypes::LineGeometry:
          // polygon layer and line symbolizer: draw polygon stroke
          // line layer and line symbolizer: draw line
          l = createLineLayerFromSld( element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::PointGeometry:
          // point layer and line symbolizer: draw a little line marker
          l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "MarkerLine" ), element );
          if ( l )
            layers.append( l );

          break;

        default:
          break;
      }
    }
  }

  if ( symbolizerName == QLatin1String( "PolygonSymbolizer" ) )
  {
    // get Fill and Stroke elements, nothing will be rendered if both are missing
    QDomElement fillElem = element.firstChildElement( QStringLiteral( "Fill" ) );
    QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
    if ( fillElem.isNull() && strokeElem.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "neither Fill nor Stroke element not found in PolygonSymbolizer" ) );
    }
    else
    {
      QgsSymbolLayer *l = nullptr;

      switch ( geomType )
      {
        case QgsWkbTypes::PolygonGeometry:
          // polygon layer and polygon symbolizer: draw fill

          l = createFillLayerFromSld( element );
          if ( l )
          {
            layers.append( l );

            // SVGFill and SimpleFill symbolLayerV2 supports stroke internally,
            // so don't go forward to create a different symbolLayerV2 for stroke
            if ( l->layerType() == QLatin1String( "SimpleFill" ) || l->layerType() == QLatin1String( "SVGFill" ) )
              break;
          }

          // now create polygon stroke
          // polygon layer and polygon symbolizer: draw polygon stroke
          l = createLineLayerFromSld( element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::LineGeometry:
          // line layer and polygon symbolizer: draw line
          l = createLineLayerFromSld( element );
          if ( l )
            layers.append( l );

          break;

        case QgsWkbTypes::PointGeometry:
          // point layer and polygon symbolizer: draw a square marker
          convertPolygonSymbolizerToPointMarker( element, layers );
          break;

        default:
          break;
      }
    }
  }

  return true;
}

QgsSymbolLayer *QgsSymbolLayerUtils::createFillLayerFromSld( QDomElement &element )
{
  QDomElement fillElem = element.firstChildElement( QStringLiteral( "Fill" ) );
  if ( fillElem.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Fill element not found" ) );
    return nullptr;
  }

  QgsSymbolLayer *l = nullptr;

  if ( needLinePatternFill( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "LinePatternFill" ), element );
  else if ( needPointPatternFill( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "PointPatternFill" ), element );
  else if ( needSvgFill( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "SVGFill" ), element );
  else
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "SimpleFill" ), element );

  return l;
}

QgsSymbolLayer *QgsSymbolLayerUtils::createLineLayerFromSld( QDomElement &element )
{
  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
  if ( strokeElem.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Stroke element not found" ) );
    return nullptr;
  }

  QgsSymbolLayer *l = nullptr;

  if ( needMarkerLine( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "MarkerLine" ), element );
  else
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "SimpleLine" ), element );

  return l;
}

QgsSymbolLayer *QgsSymbolLayerUtils::createMarkerLayerFromSld( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Graphic element not found" ) );
    return nullptr;
  }

  QgsSymbolLayer *l = nullptr;

  if ( needFontMarker( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "FontMarker" ), element );
  else if ( needSvgMarker( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "SvgMarker" ), element );
  else if ( needEllipseMarker( element ) )
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "EllipseMarker" ), element );
  else
    l = QgsApplication::symbolLayerRegistry()->createSymbolLayerFromSld( QStringLiteral( "SimpleMarker" ), element );

  return l;
}

bool QgsSymbolLayerUtils::hasExternalGraphic( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return false;

  QDomElement externalGraphicElem = graphicElem.firstChildElement( QStringLiteral( "ExternalGraphic" ) );
  if ( externalGraphicElem.isNull() )
    return false;

  // check for format
  QDomElement formatElem = externalGraphicElem.firstChildElement( QStringLiteral( "Format" ) );
  if ( formatElem.isNull() )
    return false;

  QString format = formatElem.firstChild().nodeValue();
  if ( format != QLatin1String( "image/svg+xml" ) )
  {
    QgsDebugMsg( "unsupported External Graphic format found: " + format );
    return false;
  }

  // check for a valid content
  QDomElement onlineResourceElem = externalGraphicElem.firstChildElement( QStringLiteral( "OnlineResource" ) );
  QDomElement inlineContentElem = externalGraphicElem.firstChildElement( QStringLiteral( "InlineContent" ) );
  if ( !onlineResourceElem.isNull() )
  {
    return true;
  }
#if 0
  else if ( !inlineContentElem.isNull() )
  {
    return false; // not implemented yet
  }
#endif
  else
  {
    return false;
  }
}

bool QgsSymbolLayerUtils::hasWellKnownMark( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return false;

  QDomElement markElem = graphicElem.firstChildElement( QStringLiteral( "Mark" ) );
  if ( markElem.isNull() )
    return false;

  QDomElement wellKnownNameElem = markElem.firstChildElement( QStringLiteral( "WellKnownName" ) );
  return !wellKnownNameElem.isNull();
}


bool QgsSymbolLayerUtils::needFontMarker( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return false;

  QDomElement markElem = graphicElem.firstChildElement( QStringLiteral( "Mark" ) );
  if ( markElem.isNull() )
    return false;

  // check for format
  QDomElement formatElem = markElem.firstChildElement( QStringLiteral( "Format" ) );
  if ( formatElem.isNull() )
    return false;

  QString format = formatElem.firstChild().nodeValue();
  if ( format != QLatin1String( "ttf" ) )
  {
    QgsDebugMsg( "unsupported Graphic Mark format found: " + format );
    return false;
  }

  // check for a valid content
  QDomElement onlineResourceElem = markElem.firstChildElement( QStringLiteral( "OnlineResource" ) );
  QDomElement inlineContentElem = markElem.firstChildElement( QStringLiteral( "InlineContent" ) );
  if ( !onlineResourceElem.isNull() )
  {
    // mark with ttf format has a markIndex element
    QDomElement markIndexElem = markElem.firstChildElement( QStringLiteral( "MarkIndex" ) );
    if ( !markIndexElem.isNull() )
      return true;
  }
  else if ( !inlineContentElem.isNull() )
  {
    return false; // not implemented yet
  }

  return false;
}

bool QgsSymbolLayerUtils::needSvgMarker( QDomElement &element )
{
  return hasExternalGraphic( element );
}

bool QgsSymbolLayerUtils::needEllipseMarker( QDomElement &element )
{
  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return false;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( graphicElem );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == QLatin1String( "widthHeightFactor" ) )
    {
      return true;
    }
  }

  return false;
}

bool QgsSymbolLayerUtils::needMarkerLine( QDomElement &element )
{
  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
  if ( strokeElem.isNull() )
    return false;

  QDomElement graphicStrokeElem = strokeElem.firstChildElement( QStringLiteral( "GraphicStroke" ) );
  if ( graphicStrokeElem.isNull() )
    return false;

  return hasWellKnownMark( graphicStrokeElem );
}

bool QgsSymbolLayerUtils::needLinePatternFill( QDomElement &element )
{
  QDomElement fillElem = element.firstChildElement( QStringLiteral( "Fill" ) );
  if ( fillElem.isNull() )
    return false;

  QDomElement graphicFillElem = fillElem.firstChildElement( QStringLiteral( "GraphicFill" ) );
  if ( graphicFillElem.isNull() )
    return false;

  QDomElement graphicElem = graphicFillElem.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return false;

  // line pattern fill uses horline wellknown marker with an angle

  QString name;
  QColor fillColor, strokeColor;
  double size, strokeWidth;
  Qt::PenStyle strokeStyle;
  if ( !wellKnownMarkerFromSld( graphicElem, name, fillColor, strokeColor, strokeStyle, strokeWidth, size ) )
    return false;

  if ( name != QLatin1String( "horline" ) )
    return false;

  QString angleFunc;
  if ( !rotationFromSldElement( graphicElem, angleFunc ) )
    return false;

  bool ok;
  double angle = angleFunc.toDouble( &ok );
  return !( !ok || qgsDoubleNear( angle, 0.0 ) );
}

bool QgsSymbolLayerUtils::needPointPatternFill( QDomElement &element )
{
  Q_UNUSED( element );
  return false;
}

bool QgsSymbolLayerUtils::needSvgFill( QDomElement &element )
{
  QDomElement fillElem = element.firstChildElement( QStringLiteral( "Fill" ) );
  if ( fillElem.isNull() )
    return false;

  QDomElement graphicFillElem = fillElem.firstChildElement( QStringLiteral( "GraphicFill" ) );
  if ( graphicFillElem.isNull() )
    return false;

  return hasExternalGraphic( graphicFillElem );
}


bool QgsSymbolLayerUtils::convertPolygonSymbolizerToPointMarker( QDomElement &element, QgsSymbolLayerList &layerList )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  /* SE 1.1 says about PolygonSymbolizer:
  if a point geometry is referenced instead of a polygon,
  then a small, square, ortho-normal polygon should be
  constructed for rendering.
   */

  QgsSymbolLayerList layers;

  // retrieve both Fill and Stroke elements
  QDomElement fillElem = element.firstChildElement( QStringLiteral( "Fill" ) );
  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );

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
      QgsStringMap map;
      map[QStringLiteral( "name" )] = QStringLiteral( "square" );
      map[QStringLiteral( "color" )] = encodeColor( validFill ? fillColor : Qt::transparent );
      map[QStringLiteral( "color_border" )] = encodeColor( validStroke ? strokeColor : Qt::transparent );
      map[QStringLiteral( "size" )] = QString::number( 6 );
      map[QStringLiteral( "angle" )] = QString::number( 0 );
      map[QStringLiteral( "offset" )] = encodePoint( QPointF( 0, 0 ) );
      layers.append( QgsApplication::symbolLayerRegistry()->createSymbolLayer( QStringLiteral( "SimpleMarker" ), map ) );
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
    QDomElement graphicFillElem = fillElem.firstChildElement( QStringLiteral( "GraphicFill" ) );
    if ( !graphicFillElem.isNull() )
    {
      // GraphicFill element must contain a Graphic element
      QDomElement graphicElem = graphicFillElem.firstChildElement( QStringLiteral( "Graphic" ) );
      if ( !graphicElem.isNull() )
      {
        // Graphic element can contains some ExternalGraphic and Mark element
        // search for the first supported one and use it
        bool found = false;

        QDomElement graphicChildElem = graphicElem.firstChildElement();
        while ( !graphicChildElem.isNull() )
        {
          if ( graphicChildElem.localName() == QLatin1String( "Mark" ) )
          {
            // check for a well known name
            QDomElement wellKnownNameElem = graphicChildElem.firstChildElement( QStringLiteral( "WellKnownName" ) );
            if ( !wellKnownNameElem.isNull() )
            {
              name = wellKnownNameElem.firstChild().nodeValue();
              found = true;
              break;
            }
          }

          if ( graphicChildElem.localName() == QLatin1String( "ExternalGraphic" ) || graphicChildElem.localName() == QLatin1String( "Mark" ) )
          {
            // check for external graphic format
            QDomElement formatElem = graphicChildElem.firstChildElement( QStringLiteral( "Format" ) );
            if ( formatElem.isNull() )
              continue;

            format = formatElem.firstChild().nodeValue();

            // TODO: remove this check when more formats will be supported
            // only SVG external graphics are supported in this moment
            if ( graphicChildElem.localName() == QLatin1String( "ExternalGraphic" ) && format != QLatin1String( "image/svg+xml" ) )
              continue;

            // TODO: remove this check when more formats will be supported
            // only ttf marks are supported in this moment
            if ( graphicChildElem.localName() == QLatin1String( "Mark" ) && format != QLatin1String( "ttf" ) )
              continue;

            // check for a valid content
            QDomElement onlineResourceElem = graphicChildElem.firstChildElement( QStringLiteral( "OnlineResource" ) );
            QDomElement inlineContentElem = graphicChildElem.firstChildElement( QStringLiteral( "InlineContent" ) );

            if ( !onlineResourceElem.isNull() )
            {
              name = onlineResourceElem.attributeNS( QStringLiteral( "http://www.w3.org/1999/xlink" ), QStringLiteral( "href" ) );

              if ( graphicChildElem.localName() == QLatin1String( "Mark" ) && format == QLatin1String( "ttf" ) )
              {
                // mark with ttf format may have a name like ttf://fontFamily
                if ( name.startsWith( QLatin1String( "ttf://" ) ) )
                  name = name.mid( 6 );

                // mark with ttf format has a markIndex element
                QDomElement markIndexElem = graphicChildElem.firstChildElement( QStringLiteral( "MarkIndex" ) );
                if ( markIndexElem.isNull() )
                  continue;

                bool ok;
                int v = markIndexElem.firstChild().nodeValue().toInt( &ok );
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
          if ( graphicChildElem.localName() == QLatin1String( "Mark" ) )
          {
            name = QStringLiteral( "square" );
            found = true;
            break;
          }
        }

        // if found a valid Mark, check for its Fill and Stroke element
        if ( found && graphicChildElem.localName() == QLatin1String( "Mark" ) )
        {
          // XXX: recursive definition!?! couldn't be dangerous???
          // to avoid recursion we handle only simple fill and simple stroke

          // check for simple fill
          // Fill element can contain some SvgParameter elements
          Qt::BrushStyle markFillStyle;

          QDomElement markFillElem = graphicChildElem.firstChildElement( QStringLiteral( "Fill" ) );
          if ( fillFromSld( markFillElem, markFillStyle, fillColor ) )
            validFill = true;

          // check for simple stroke
          // Stroke element can contain some SvgParameter elements
          Qt::PenStyle strokeStyle;
          double strokeWidth = 1.0, dashOffset = 0.0;
          QVector<qreal> customDashPattern;

          QDomElement markStrokeElem = graphicChildElem.firstChildElement( QStringLiteral( "Stroke" ) );
          if ( lineFromSld( markStrokeElem, strokeStyle, strokeColor, strokeWidth,
                            nullptr, nullptr, &customDashPattern, &dashOffset ) )
            validStroke = true;
        }

        if ( found )
        {
          // check for Opacity, Size, Rotation, AnchorPoint, Displacement
          QDomElement opacityElem = graphicElem.firstChildElement( QStringLiteral( "Opacity" ) );
          if ( !opacityElem.isNull() )
            fillColor.setAlpha( decodeSldAlpha( opacityElem.firstChild().nodeValue() ) );

          QDomElement sizeElem = graphicElem.firstChildElement( QStringLiteral( "Size" ) );
          if ( !sizeElem.isNull() )
          {
            bool ok;
            double v = sizeElem.firstChild().nodeValue().toDouble( &ok );
            if ( ok && v > 0 )
              size = v;
          }

          QString angleFunc;
          if ( rotationFromSldElement( graphicElem, angleFunc ) && !angleFunc.isEmpty() )
          {
            bool ok;
            double v = angleFunc.toDouble( &ok );
            if ( ok )
              angle = v;
          }

          displacementFromSldElement( graphicElem, offset );
        }
      }
    }

    if ( validFill || validStroke )
    {
      if ( format == QLatin1String( "image/svg+xml" ) )
      {
        QgsStringMap map;
        map[QStringLiteral( "name" )] = name;
        map[QStringLiteral( "fill" )] = fillColor.name();
        map[QStringLiteral( "outline" )] = strokeColor.name();
        map[QStringLiteral( "outline-width" )] = QString::number( strokeWidth );
        if ( !qgsDoubleNear( size, 0.0 ) )
          map[QStringLiteral( "size" )] = QString::number( size );
        if ( !qgsDoubleNear( angle, 0.0 ) )
          map[QStringLiteral( "angle" )] = QString::number( angle );
        if ( !offset.isNull() )
          map[QStringLiteral( "offset" )] = encodePoint( offset );
        layers.append( QgsApplication::symbolLayerRegistry()->createSymbolLayer( QStringLiteral( "SvgMarker" ), map ) );
      }
      else if ( format == QLatin1String( "ttf" ) )
      {
        QgsStringMap map;
        map[QStringLiteral( "font" )] = name;
        map[QStringLiteral( "chr" )] = markIndex;
        map[QStringLiteral( "color" )] = encodeColor( validFill ? fillColor : Qt::transparent );
        if ( size > 0 )
          map[QStringLiteral( "size" )] = QString::number( size );
        if ( !qgsDoubleNear( angle, 0.0 ) )
          map[QStringLiteral( "angle" )] = QString::number( angle );
        if ( !offset.isNull() )
          map[QStringLiteral( "offset" )] = encodePoint( offset );
        layers.append( QgsApplication::symbolLayerRegistry()->createSymbolLayer( QStringLiteral( "FontMarker" ), map ) );
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
  QString patternName;
  switch ( brushStyle )
  {
    case Qt::NoBrush:
      return;

    case Qt::SolidPattern:
      if ( color.isValid() )
      {
        element.appendChild( createSvgParameterElement( doc, QStringLiteral( "fill" ), color.name() ) );
        if ( color.alpha() < 255 )
          element.appendChild( createSvgParameterElement( doc, QStringLiteral( "fill-opacity" ), encodeSldAlpha( color.alpha() ) ) );
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
      element.appendChild( doc.createComment( QStringLiteral( "Qt::BrushStyle '%1'' not supported yet" ).arg( brushStyle ) ) );
      return;
  }

  QDomElement graphicFillElem = doc.createElement( QStringLiteral( "se:GraphicFill" ) );
  element.appendChild( graphicFillElem );

  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  graphicFillElem.appendChild( graphicElem );

  QColor fillColor = patternName.startsWith( QLatin1String( "brush://" ) ) ? color : QColor();
  QColor strokeColor = !patternName.startsWith( QLatin1String( "brush://" ) ) ? color : QColor();

  /* Use WellKnownName tag to handle QT brush styles. */
  wellKnownMarkerToSld( doc, graphicElem, patternName, fillColor, strokeColor, Qt::SolidLine, -1, -1 );
}

bool QgsSymbolLayerUtils::fillFromSld( QDomElement &element, Qt::BrushStyle &brushStyle, QColor &color )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  brushStyle = Qt::SolidPattern;
  color = QColor( 128, 128, 128 );

  if ( element.isNull() )
  {
    brushStyle = Qt::NoBrush;
    color = QColor();
    return true;
  }

  QDomElement graphicFillElem = element.firstChildElement( QStringLiteral( "GraphicFill" ) );
  // if no GraphicFill element is found, it's a solid fill
  if ( graphicFillElem.isNull() )
  {
    QgsStringMap svgParams = getSvgParameterList( element );
    for ( QgsStringMap::iterator it = svgParams.begin(); it != svgParams.end(); ++it )
    {
      QgsDebugMsg( QStringLiteral( "found SvgParameter %1: %2" ).arg( it.key(), it.value() ) );

      if ( it.key() == QLatin1String( "fill" ) )
        color = QColor( it.value() );
      else if ( it.key() == QLatin1String( "fill-opacity" ) )
        color.setAlpha( decodeSldAlpha( it.value() ) );
    }
  }
  else  // wellKnown marker
  {
    QDomElement graphicElem = graphicFillElem.firstChildElement( QStringLiteral( "Graphic" ) );
    if ( graphicElem.isNull() )
      return false; // Graphic is required within GraphicFill

    QString patternName = QStringLiteral( "square" );
    QColor fillColor, strokeColor;
    double strokeWidth, size;
    Qt::PenStyle strokeStyle;
    if ( !wellKnownMarkerFromSld( graphicElem, patternName, fillColor, strokeColor, strokeStyle, strokeWidth, size ) )
      return false;

    brushStyle = decodeSldBrushStyle( patternName );
    if ( brushStyle == Qt::NoBrush )
      return false; // unable to decode brush style

    QColor c = patternName.startsWith( QLatin1String( "brush://" ) ) ? fillColor : strokeColor;
    if ( c.isValid() )
      color = c;
  }

  return true;
}

void QgsSymbolLayerUtils::lineToSld( QDomDocument &doc, QDomElement &element,
                                     Qt::PenStyle penStyle, const QColor &color, double width,
                                     const Qt::PenJoinStyle *penJoinStyle, const Qt::PenCapStyle *penCapStyle,
                                     const QVector<qreal> *customDashPattern, double dashOffset )
{
  QVector<qreal> dashPattern;
  const QVector<qreal> *pattern = &dashPattern;

  if ( penStyle == Qt::CustomDashLine && !customDashPattern )
  {
    element.appendChild( doc.createComment( QStringLiteral( "WARNING: Custom dash pattern required but not provided. Using default dash pattern." ) ) );
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
      element.appendChild( doc.createComment( QStringLiteral( "Qt::BrushStyle '%1'' not supported yet" ).arg( penStyle ) ) );
      return;
  }

  if ( color.isValid() )
  {
    element.appendChild( createSvgParameterElement( doc, QStringLiteral( "stroke" ), color.name() ) );
    if ( color.alpha() < 255 )
      element.appendChild( createSvgParameterElement( doc, QStringLiteral( "stroke-opacity" ), encodeSldAlpha( color.alpha() ) ) );
  }
  if ( width > 0 )
  {
    element.appendChild( createSvgParameterElement( doc, QStringLiteral( "stroke-width" ), qgsDoubleToString( width ) ) );
  }
  else if ( width == 0 )
  {
    // hairline, yet not zero. it's actually painted in qgis
    element.appendChild( createSvgParameterElement( doc, QStringLiteral( "stroke-width" ), QStringLiteral( "0.5" ) ) );
  }
  if ( penJoinStyle )
    element.appendChild( createSvgParameterElement( doc, QStringLiteral( "stroke-linejoin" ), encodeSldLineJoinStyle( *penJoinStyle ) ) );
  if ( penCapStyle )
    element.appendChild( createSvgParameterElement( doc, QStringLiteral( "stroke-linecap" ), encodeSldLineCapStyle( *penCapStyle ) ) );

  if ( !pattern->isEmpty() )
  {
    element.appendChild( createSvgParameterElement( doc, QStringLiteral( "stroke-dasharray" ), encodeSldRealVector( *pattern ) ) );
    if ( !qgsDoubleNear( dashOffset, 0.0 ) )
      element.appendChild( createSvgParameterElement( doc, QStringLiteral( "stroke-dashoffset" ), qgsDoubleToString( dashOffset ) ) );
  }
}


bool QgsSymbolLayerUtils::lineFromSld( QDomElement &element,
                                       Qt::PenStyle &penStyle, QColor &color, double &width,
                                       Qt::PenJoinStyle *penJoinStyle, Qt::PenCapStyle *penCapStyle,
                                       QVector<qreal> *customDashPattern, double *dashOffset )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

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
    QgsDebugMsg( QStringLiteral( "found SvgParameter %1: %2" ).arg( it.key(), it.value() ) );

    if ( it.key() == QLatin1String( "stroke" ) )
    {
      color = QColor( it.value() );
    }
    else if ( it.key() == QLatin1String( "stroke-opacity" ) )
    {
      color.setAlpha( decodeSldAlpha( it.value() ) );
    }
    else if ( it.key() == QLatin1String( "stroke-width" ) )
    {
      bool ok;
      double w = it.value().toDouble( &ok );
      if ( ok )
        width = w;
    }
    else if ( it.key() == QLatin1String( "stroke-linejoin" ) && penJoinStyle )
    {
      *penJoinStyle = decodeSldLineJoinStyle( it.value() );
    }
    else if ( it.key() == QLatin1String( "stroke-linecap" ) && penCapStyle )
    {
      *penCapStyle = decodeSldLineCapStyle( it.value() );
    }
    else if ( it.key() == QLatin1String( "stroke-dasharray" ) )
    {
      QVector<qreal> dashPattern = decodeSldRealVector( it.value() );
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
            QgsDebugMsg( QStringLiteral( "custom dash pattern required but not provided. Using default dash pattern." ) );
            penStyle = Qt::DashLine;
          }
        }
      }
    }
    else if ( it.key() == QLatin1String( "stroke-dashoffset" ) && dashOffset )
    {
      bool ok;
      double d = it.value().toDouble( &ok );
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
  QDomElement externalGraphicElem = doc.createElement( QStringLiteral( "se:ExternalGraphic" ) );
  element.appendChild( externalGraphicElem );

  createOnlineResourceElement( doc, externalGraphicElem, path, mime );

  //TODO: missing a way to handle svg color. Should use <se:ColorReplacement>
  Q_UNUSED( color );

  if ( size >= 0 )
  {
    QDomElement sizeElem = doc.createElement( QStringLiteral( "se:Size" ) );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    element.appendChild( sizeElem );
  }
}

void QgsSymbolLayerUtils::parametricSvgToSld( QDomDocument &doc, QDomElement &graphicElem,
    const QString &path, const QColor &fillColor, double size, const QColor &strokeColor, double strokeWidth )
{
  // Parametric SVG paths are an extension that few systems will understand, but se:Graphic allows for fallback
  // symbols, this encodes the full parametric path first, the pure shape second, and a mark with the right colors as
  // a last resort for systems that cannot do SVG at all

  // encode parametric version with all coloring details (size is going to be encoded by the last fallback)
  graphicElem.appendChild( doc.createComment( QStringLiteral( "Parametric SVG" ) ) );
  QString parametricPath = getSvgParametricPath( path, fillColor, strokeColor, strokeWidth );
  QgsSymbolLayerUtils::externalGraphicToSld( doc, graphicElem, parametricPath, QStringLiteral( "image/svg+xml" ), fillColor, -1 );
  // also encode a fallback version without parameters, in case a renderer gets confused by the parameters
  graphicElem.appendChild( doc.createComment( QStringLiteral( "Plain SVG fallback, no parameters" ) ) );
  QgsSymbolLayerUtils::externalGraphicToSld( doc, graphicElem, path, QStringLiteral( "image/svg+xml" ), fillColor, -1 );
  // finally encode a simple mark with the right colors/outlines for renderers that cannot do SVG at all
  graphicElem.appendChild( doc.createComment( QStringLiteral( "Well known marker fallback" ) ) );
  QgsSymbolLayerUtils::wellKnownMarkerToSld( doc, graphicElem, QStringLiteral( "square" ), fillColor, strokeColor, Qt::PenStyle::SolidLine, strokeWidth, -1 );

  // size is encoded here, it's part of se:Graphic, not attached to the single symbol
  if ( size >= 0 )
  {
    QDomElement sizeElem = doc.createElement( QStringLiteral( "se:Size" ) );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    graphicElem.appendChild( sizeElem );
  }
}


QString QgsSymbolLayerUtils::getSvgParametricPath( const QString &basePath, const QColor &fillColor, const QColor &strokeColor, double strokeWidth )
{
  QUrl url = QUrl();
  if ( fillColor.isValid() )
  {
    url.addQueryItem( QStringLiteral( "fill" ), fillColor.name() );
    url.addQueryItem( QStringLiteral( "fill-opacity" ), encodeSldAlpha( fillColor.alpha() ) );
  }
  else
  {
    url.addQueryItem( QStringLiteral( "fill" ), QStringLiteral( "#000000" ) );
    url.addQueryItem( QStringLiteral( "fill-opacity" ), QStringLiteral( "1" ) );
  }
  if ( strokeColor.isValid() )
  {
    url.addQueryItem( QStringLiteral( "outline" ), strokeColor.name() );
    url.addQueryItem( QStringLiteral( "outline-opacity" ), encodeSldAlpha( strokeColor.alpha() ) );
  }
  else
  {
    url.addQueryItem( QStringLiteral( "outline" ), QStringLiteral( "#000000" ) );
    url.addQueryItem( QStringLiteral( "outline-opacity" ), QStringLiteral( "1" ) );
  }
  url.addQueryItem( QStringLiteral( "outline-width" ), QString::number( strokeWidth ) );
  QString params = url.encodedQuery();
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
  QgsDebugMsg( QStringLiteral( "Entered." ) );
  Q_UNUSED( color );

  QDomElement externalGraphicElem = element.firstChildElement( QStringLiteral( "ExternalGraphic" ) );
  if ( externalGraphicElem.isNull() )
    return false;

  onlineResourceFromSldElement( externalGraphicElem, path, mime );

  QDomElement sizeElem = element.firstChildElement( QStringLiteral( "Size" ) );
  if ( !sizeElem.isNull() )
  {
    bool ok;
    double s = sizeElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      size = s;
  }

  return true;
}

void QgsSymbolLayerUtils::externalMarkerToSld( QDomDocument &doc, QDomElement &element,
    const QString &path, const QString &format, int *markIndex,
    const QColor &color, double size )
{
  QDomElement markElem = doc.createElement( QStringLiteral( "se:Mark" ) );
  element.appendChild( markElem );

  createOnlineResourceElement( doc, markElem, path, format );

  if ( markIndex )
  {
    QDomElement markIndexElem = doc.createElement( QStringLiteral( "se:MarkIndex" ) );
    markIndexElem.appendChild( doc.createTextNode( QString::number( *markIndex ) ) );
    markElem.appendChild( markIndexElem );
  }

  // <Fill>
  QDomElement fillElem = doc.createElement( QStringLiteral( "se:Fill" ) );
  fillToSld( doc, fillElem, Qt::SolidPattern, color );
  markElem.appendChild( fillElem );

  // <Size>
  if ( !qgsDoubleNear( size, 0.0 ) && size > 0 )
  {
    QDomElement sizeElem = doc.createElement( QStringLiteral( "se:Size" ) );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    element.appendChild( sizeElem );
  }
}

bool QgsSymbolLayerUtils::externalMarkerFromSld( QDomElement &element,
    QString &path, QString &format, int &markIndex,
    QColor &color, double &size )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  color = QColor();
  markIndex = -1;
  size = -1;

  QDomElement markElem = element.firstChildElement( QStringLiteral( "Mark" ) );
  if ( markElem.isNull() )
    return false;

  onlineResourceFromSldElement( markElem, path, format );

  QDomElement markIndexElem = markElem.firstChildElement( QStringLiteral( "MarkIndex" ) );
  if ( !markIndexElem.isNull() )
  {
    bool ok;
    int i = markIndexElem.firstChild().nodeValue().toInt( &ok );
    if ( ok )
      markIndex = i;
  }

  // <Fill>
  QDomElement fillElem = markElem.firstChildElement( QStringLiteral( "Fill" ) );
  Qt::BrushStyle b = Qt::SolidPattern;
  fillFromSld( fillElem, b, color );
  // ignore brush style, solid expected

  // <Size>
  QDomElement sizeElem = element.firstChildElement( QStringLiteral( "Size" ) );
  if ( !sizeElem.isNull() )
  {
    bool ok;
    double s = sizeElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      size = s;
  }

  return true;
}

void QgsSymbolLayerUtils::wellKnownMarkerToSld( QDomDocument &doc, QDomElement &element,
    const QString &name, const QColor &color, const QColor &strokeColor, Qt::PenStyle strokeStyle,
    double strokeWidth, double size )
{
  QDomElement markElem = doc.createElement( QStringLiteral( "se:Mark" ) );
  element.appendChild( markElem );

  QDomElement wellKnownNameElem = doc.createElement( QStringLiteral( "se:WellKnownName" ) );
  wellKnownNameElem.appendChild( doc.createTextNode( name ) );
  markElem.appendChild( wellKnownNameElem );

  // <Fill>
  if ( color.isValid() )
  {
    QDomElement fillElem = doc.createElement( QStringLiteral( "se:Fill" ) );
    fillToSld( doc, fillElem, Qt::SolidPattern, color );
    markElem.appendChild( fillElem );
  }

  // <Stroke>
  if ( strokeColor.isValid() )
  {
    QDomElement strokeElem = doc.createElement( QStringLiteral( "se:Stroke" ) );
    lineToSld( doc, strokeElem, strokeStyle, strokeColor, strokeWidth );
    markElem.appendChild( strokeElem );
  }

  // <Size>
  if ( !qgsDoubleNear( size, 0.0 ) && size > 0 )
  {
    QDomElement sizeElem = doc.createElement( QStringLiteral( "se:Size" ) );
    sizeElem.appendChild( doc.createTextNode( qgsDoubleToString( size ) ) );
    element.appendChild( sizeElem );
  }
}

bool QgsSymbolLayerUtils::wellKnownMarkerFromSld( QDomElement &element,
    QString &name, QColor &color, QColor &strokeColor, Qt::PenStyle &strokeStyle,
    double &strokeWidth, double &size )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  name = QStringLiteral( "square" );
  color = QColor();
  strokeColor = QColor( 0, 0, 0 );
  strokeWidth = 1;
  size = 6;

  QDomElement markElem = element.firstChildElement( QStringLiteral( "Mark" ) );
  if ( markElem.isNull() )
    return false;

  QDomElement wellKnownNameElem = markElem.firstChildElement( QStringLiteral( "WellKnownName" ) );
  if ( !wellKnownNameElem.isNull() )
  {
    name = wellKnownNameElem.firstChild().nodeValue();
    QgsDebugMsg( "found Mark with well known name: " + name );
  }

  // <Fill>
  QDomElement fillElem = markElem.firstChildElement( QStringLiteral( "Fill" ) );
  Qt::BrushStyle b = Qt::SolidPattern;
  fillFromSld( fillElem, b, color );
  // ignore brush style, solid expected

  // <Stroke>
  QDomElement strokeElem = markElem.firstChildElement( QStringLiteral( "Stroke" ) );
  lineFromSld( strokeElem, strokeStyle, strokeColor, strokeWidth );
  // ignore stroke style, solid expected

  // <Size>
  QDomElement sizeElem = element.firstChildElement( QStringLiteral( "Size" ) );
  if ( !sizeElem.isNull() )
  {
    bool ok;
    double s = sizeElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      size = s;
  }

  return true;
}

void QgsSymbolLayerUtils::createRotationElement( QDomDocument &doc, QDomElement &element, const QString &rotationFunc )
{
  if ( !rotationFunc.isEmpty() )
  {
    QDomElement rotationElem = doc.createElement( QStringLiteral( "se:Rotation" ) );
    createExpressionElement( doc, rotationElem, rotationFunc );
    element.appendChild( rotationElem );
  }
}

bool QgsSymbolLayerUtils::rotationFromSldElement( QDomElement &element, QString &rotationFunc )
{
  QDomElement rotationElem = element.firstChildElement( QStringLiteral( "Rotation" ) );
  if ( !rotationElem.isNull() )
  {
    return functionFromSldElement( rotationElem, rotationFunc );
  }
  return true;
}


void QgsSymbolLayerUtils::createOpacityElement( QDomDocument &doc, QDomElement &element, const QString &alphaFunc )
{
  if ( !alphaFunc.isEmpty() )
  {
    QDomElement opacityElem = doc.createElement( QStringLiteral( "se:Opacity" ) );
    createExpressionElement( doc, opacityElem, alphaFunc );
    element.appendChild( opacityElem );
  }
}

bool QgsSymbolLayerUtils::opacityFromSldElement( QDomElement &element, QString &alphaFunc )
{
  QDomElement opacityElem = element.firstChildElement( QStringLiteral( "Opacity" ) );
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

  QDomElement displacementElem = doc.createElement( QStringLiteral( "se:Displacement" ) );
  element.appendChild( displacementElem );

  QDomElement dispXElem = doc.createElement( QStringLiteral( "se:DisplacementX" ) );
  dispXElem.appendChild( doc.createTextNode( qgsDoubleToString( offset.x(), 2 ) ) );

  QDomElement dispYElem = doc.createElement( QStringLiteral( "se:DisplacementY" ) );
  dispYElem.appendChild( doc.createTextNode( qgsDoubleToString( offset.y(), 2 ) ) );

  displacementElem.appendChild( dispXElem );
  displacementElem.appendChild( dispYElem );
}

void QgsSymbolLayerUtils::createAnchorPointElement( QDomDocument &doc, QDomElement &element, QPointF anchor )
{
  // anchor is not tested for null, (0,0) is _not_ the default value (0.5, 0) is.

  QDomElement anchorElem = doc.createElement( QStringLiteral( "se:AnchorPoint" ) );
  element.appendChild( anchorElem );

  QDomElement anchorXElem = doc.createElement( QStringLiteral( "se:AnchorPointX" ) );
  anchorXElem.appendChild( doc.createTextNode( qgsDoubleToString( anchor.x() ) ) );

  QDomElement anchorYElem = doc.createElement( QStringLiteral( "se:AnchorPointY" ) );
  anchorYElem.appendChild( doc.createTextNode( qgsDoubleToString( anchor.y() ) ) );

  anchorElem.appendChild( anchorXElem );
  anchorElem.appendChild( anchorYElem );
}

bool QgsSymbolLayerUtils::displacementFromSldElement( QDomElement &element, QPointF &offset )
{
  offset = QPointF( 0, 0 );

  QDomElement displacementElem = element.firstChildElement( QStringLiteral( "Displacement" ) );
  if ( displacementElem.isNull() )
    return true;

  QDomElement dispXElem = displacementElem.firstChildElement( QStringLiteral( "DisplacementX" ) );
  if ( !dispXElem.isNull() )
  {
    bool ok;
    double offsetX = dispXElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset.setX( offsetX );
  }

  QDomElement dispYElem = displacementElem.firstChildElement( QStringLiteral( "DisplacementY" ) );
  if ( !dispYElem.isNull() )
  {
    bool ok;
    double offsetY = dispYElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset.setY( offsetY );
  }

  return true;
}

void QgsSymbolLayerUtils::labelTextToSld( QDomDocument &doc, QDomElement &element,
    const QString &label, const QFont &font,
    const QColor &color, double size )
{
  QDomElement labelElem = doc.createElement( QStringLiteral( "se:Label" ) );
  labelElem.appendChild( doc.createTextNode( label ) );
  element.appendChild( labelElem );

  QDomElement fontElem = doc.createElement( QStringLiteral( "se:Font" ) );
  element.appendChild( fontElem );

  fontElem.appendChild( createSvgParameterElement( doc, QStringLiteral( "font-family" ), font.family() ) );
#if 0
  fontElem.appendChild( createSldParameterElement( doc, "font-style", encodeSldFontStyle( font.style() ) ) );
  fontElem.appendChild( createSldParameterElement( doc, "font-weight", encodeSldFontWeight( font.weight() ) ) );
#endif
  fontElem.appendChild( createSvgParameterElement( doc, QStringLiteral( "font-size" ), QString::number( size ) ) );

  // <Fill>
  if ( color.isValid() )
  {
    QDomElement fillElem = doc.createElement( QStringLiteral( "Fill" ) );
    fillToSld( doc, fillElem, Qt::SolidPattern, color );
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
  if ( geomFunc.isEmpty() )
    return;

  QDomElement geometryElem = doc.createElement( QStringLiteral( "Geometry" ) );
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

  createExpressionElement( doc, geometryElem, geomFunc );
}

bool QgsSymbolLayerUtils::geometryFromSldElement( QDomElement &element, QString &geomFunc )
{
  QDomElement geometryElem = element.firstChildElement( QStringLiteral( "Geometry" ) );
  if ( geometryElem.isNull() )
    return true;

  return functionFromSldElement( geometryElem, geomFunc );
}

bool QgsSymbolLayerUtils::createExpressionElement( QDomDocument &doc, QDomElement &element, const QString &function )
{
  // let's use QgsExpression to generate the SLD for the function
  QgsExpression expr( function );
  if ( expr.hasParserError() )
  {
    element.appendChild( doc.createComment( "Parser Error: " + expr.parserErrorString() + " - Expression was: " + function ) );
    return false;
  }
  QDomElement filterElem = QgsOgcUtils::expressionToOgcExpression( expr, doc );
  if ( !filterElem.isNull() )
    element.appendChild( filterElem );
  return true;
}


bool QgsSymbolLayerUtils::createFunctionElement( QDomDocument &doc, QDomElement &element, const QString &function )
{
  // let's use QgsExpression to generate the SLD for the function
  QgsExpression expr( function );
  if ( expr.hasParserError() )
  {
    element.appendChild( doc.createComment( "Parser Error: " + expr.parserErrorString() + " - Expression was: " + function ) );
    return false;
  }
  QDomElement filterElem = QgsOgcUtils::expressionToOgcFilter( expr, doc );
  if ( !filterElem.isNull() )
    element.appendChild( filterElem );
  return true;
}

bool QgsSymbolLayerUtils::functionFromSldElement( QDomElement &element, QString &function )
{
  // check if ogc:Filter or containe ogc:Filters
  QDomElement elem = element;
  if ( element.tagName() != QLatin1String( "Filter" ) )
  {
    QDomNodeList filterNodes = element.elementsByTagName( QStringLiteral( "Filter" ) );
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

  bool valid = !expr->hasParserError();
  if ( !valid )
  {
    QgsDebugMsg( "parser error: " + expr->parserErrorString() );
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
  QString url = svgSymbolPathToName( path, QgsPathResolver() );
  QDomElement onlineResourceElem = doc.createElement( QStringLiteral( "se:OnlineResource" ) );
  onlineResourceElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
  onlineResourceElem.setAttribute( QStringLiteral( "xlink:href" ), url );
  element.appendChild( onlineResourceElem );

  QDomElement formatElem = doc.createElement( QStringLiteral( "se:Format" ) );
  formatElem.appendChild( doc.createTextNode( format ) );
  element.appendChild( formatElem );
}

bool QgsSymbolLayerUtils::onlineResourceFromSldElement( QDomElement &element, QString &path, QString &format )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  QDomElement onlineResourceElem = element.firstChildElement( QStringLiteral( "OnlineResource" ) );
  if ( onlineResourceElem.isNull() )
    return false;

  path = onlineResourceElem.attributeNS( QStringLiteral( "http://www.w3.org/1999/xlink" ), QStringLiteral( "href" ) );

  QDomElement formatElem = element.firstChildElement( QStringLiteral( "Format" ) );
  if ( formatElem.isNull() )
    return false; // OnlineResource requires a Format sibling element

  format = formatElem.firstChild().nodeValue();
  return true;
}


QDomElement QgsSymbolLayerUtils::createSvgParameterElement( QDomDocument &doc, const QString &name, const QString &value )
{
  QDomElement nodeElem = doc.createElement( QStringLiteral( "se:SvgParameter" ) );
  nodeElem.setAttribute( QStringLiteral( "name" ), name );
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
    if ( paramElem.localName() == QLatin1String( "SvgParameter" ) || paramElem.localName() == QLatin1String( "CssParameter" ) )
    {
      QString name = paramElem.attribute( QStringLiteral( "name" ) );
      if ( paramElem.firstChild().nodeType() == QDomNode::TextNode )
      {
        value = paramElem.firstChild().nodeValue();
      }
      else
      {
        if ( paramElem.firstChild().nodeType() == QDomNode::ElementNode &&
             paramElem.firstChild().localName() == QLatin1String( "Literal" ) )
        {
          QgsDebugMsg( paramElem.firstChild().localName() );
          value = paramElem.firstChild().firstChild().nodeValue();
        }
        else
        {
          QgsDebugMsg( QStringLiteral( "unexpected child of %1" ).arg( paramElem.localName() ) );
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
  QDomElement nodeElem = doc.createElement( QStringLiteral( "se:VendorOption" ) );
  nodeElem.setAttribute( QStringLiteral( "name" ), name );
  nodeElem.appendChild( doc.createTextNode( value ) );
  return nodeElem;
}

QgsStringMap QgsSymbolLayerUtils::getVendorOptionList( QDomElement &element )
{
  QgsStringMap params;

  QDomElement paramElem = element.firstChildElement( QStringLiteral( "VendorOption" ) );
  while ( !paramElem.isNull() )
  {
    QString name = paramElem.attribute( QStringLiteral( "name" ) );
    QString value = paramElem.firstChild().nodeValue();

    if ( !name.isEmpty() && !value.isEmpty() )
      params[ name ] = value;

    paramElem = paramElem.nextSiblingElement( QStringLiteral( "VendorOption" ) );
  }

  return params;
}


QgsStringMap QgsSymbolLayerUtils::parseProperties( QDomElement &element )
{
  QgsStringMap props;
  QDomElement e = element.firstChildElement();
  while ( !e.isNull() )
  {
    if ( e.tagName() == QLatin1String( "prop" ) )
    {
      QString propKey = e.attribute( QStringLiteral( "k" ) );
      QString propValue = e.attribute( QStringLiteral( "v" ) );
      props[propKey] = propValue;
    }
    e = e.nextSiblingElement();
  }
  return props;
}


void QgsSymbolLayerUtils::saveProperties( QgsStringMap props, QDomDocument &doc, QDomElement &element )
{
  for ( QgsStringMap::iterator it = props.begin(); it != props.end(); ++it )
  {
    QDomElement propEl = doc.createElement( QStringLiteral( "prop" ) );
    propEl.setAttribute( QStringLiteral( "k" ), it.key() );
    propEl.setAttribute( QStringLiteral( "v" ), it.value() );
    element.appendChild( propEl );
  }
}

QgsSymbolMap QgsSymbolLayerUtils::loadSymbols( QDomElement &element, const QgsReadWriteContext &context )
{
  // go through symbols one-by-one and load them

  QgsSymbolMap symbols;
  QDomElement e = element.firstChildElement();

  while ( !e.isNull() )
  {
    if ( e.tagName() == QLatin1String( "symbol" ) )
    {
      QgsSymbol *symbol = QgsSymbolLayerUtils::loadSymbol( e, context );
      if ( symbol )
        symbols.insert( e.attribute( QStringLiteral( "name" ) ), symbol );
    }
    else
    {
      QgsDebugMsg( "unknown tag: " + e.tagName() );
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
      QgsDebugMsg( "found subsymbol with invalid name: " + it.key() );
      delete it.value(); // we must delete it
      continue; // some invalid syntax
    }
    QString symname = parts[1];
    int symlayer = parts[2].toInt();

    if ( !symbols.contains( symname ) )
    {
      QgsDebugMsg( "subsymbol references invalid symbol: " + symname );
      delete it.value(); // we must delete it
      continue;
    }

    QgsSymbol *sym = symbols[symname];
    if ( symlayer < 0 || symlayer >= sym->symbolLayerCount() )
    {
      QgsDebugMsg( "subsymbol references invalid symbol layer: " + QString::number( symlayer ) );
      delete it.value(); // we must delete it
      continue;
    }

    // set subsymbol takes ownership
    bool res = sym->symbolLayer( symlayer )->setSubSymbol( it.value() );
    if ( !res )
    {
      QgsDebugMsg( "symbol layer refused subsymbol: " + it.key() );
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
    QDomElement symEl = saveSymbol( its.key(), its.value(), doc, context );
    symbolsElem.appendChild( symEl );
  }

  return symbolsElem;
}

void QgsSymbolLayerUtils::clearSymbolMap( QgsSymbolMap &symbols )
{
  qDeleteAll( symbols );
  symbols.clear();
}

QMimeData *QgsSymbolLayerUtils::symbolToMimeData( QgsSymbol *symbol )
{
  if ( !symbol )
    return nullptr;

  std::unique_ptr< QMimeData >mimeData( new QMimeData );

  QDomDocument symbolDoc;
  QDomElement symbolElem = saveSymbol( QStringLiteral( "symbol" ), symbol, symbolDoc, QgsReadWriteContext() );
  symbolDoc.appendChild( symbolElem );
  mimeData->setText( symbolDoc.toString() );

  mimeData->setImageData( symbolPreviewPixmap( symbol, QSize( 100, 100 ), 18 ).toImage() );
  mimeData->setColorData( symbol->color() );

  return mimeData.release();
}

QgsSymbol *QgsSymbolLayerUtils::symbolFromMimeData( const QMimeData *data )
{
  if ( !data )
    return nullptr;

  QString text = data->text();
  if ( !text.isEmpty() )
  {
    QDomDocument doc;
    QDomElement elem;

    if ( doc.setContent( text ) )
    {
      elem = doc.documentElement();

      if ( elem.nodeName() != QStringLiteral( "symbol" ) )
        elem = elem.firstChildElement( QStringLiteral( "symbol" ) );

      return loadSymbol( elem, QgsReadWriteContext() );
    }
  }
  return nullptr;
}


QgsColorRamp *QgsSymbolLayerUtils::loadColorRamp( QDomElement &element )
{
  QString rampType = element.attribute( QStringLiteral( "type" ) );

  // parse properties
  QgsStringMap props = QgsSymbolLayerUtils::parseProperties( element );

  if ( rampType == QLatin1String( "gradient" ) )
    return QgsGradientColorRamp::create( props );
  else if ( rampType == QLatin1String( "random" ) )
    return QgsLimitedRandomColorRamp::create( props );
  else if ( rampType == QLatin1String( "colorbrewer" ) )
    return QgsColorBrewerColorRamp::create( props );
  else if ( rampType == QLatin1String( "cpt-city" ) )
    return QgsCptCityColorRamp::create( props );
  else if ( rampType == QLatin1String( "preset" ) )
    return QgsPresetSchemeColorRamp::create( props );
  else
  {
    QgsDebugMsg( "unknown colorramp type " + rampType );
    return nullptr;
  }
}


QDomElement QgsSymbolLayerUtils::saveColorRamp( const QString &name, QgsColorRamp *ramp, QDomDocument &doc )
{
  QDomElement rampEl = doc.createElement( QStringLiteral( "colorramp" ) );
  rampEl.setAttribute( QStringLiteral( "type" ), ramp->type() );
  rampEl.setAttribute( QStringLiteral( "name" ), name );

  QgsSymbolLayerUtils::saveProperties( ramp->properties(), doc, rampEl );
  return rampEl;
}

QVariant QgsSymbolLayerUtils::colorRampToVariant( const QString &name, QgsColorRamp *ramp )
{
  QVariantMap rampMap;

  rampMap.insert( QStringLiteral( "type" ), ramp->type() );
  rampMap.insert( QStringLiteral( "name" ), name );

  QgsStringMap properties = ramp->properties();

  QVariantMap propertyMap;
  for ( auto property = properties.constBegin(); property != properties.constEnd(); ++property )
  {
    propertyMap.insert( property.key(), property.value() );
  }

  rampMap.insert( QStringLiteral( "properties" ), propertyMap );
  return rampMap;
}

QgsColorRamp *QgsSymbolLayerUtils::loadColorRamp( const QVariant &value )
{
  QVariantMap rampMap = value.toMap();

  QString rampType = rampMap.value( QStringLiteral( "type" ) ).toString();

  // parse properties
  QVariantMap propertyMap = rampMap.value( QStringLiteral( "properties" ) ).toMap();
  QgsStringMap props;

  for ( auto property = propertyMap.constBegin(); property != propertyMap.constEnd(); ++property )
  {
    props.insert( property.key(), property.value().toString() );
  }

  if ( rampType == QLatin1String( "gradient" ) )
    return QgsGradientColorRamp::create( props );
  else if ( rampType == QLatin1String( "random" ) )
    return QgsLimitedRandomColorRamp::create( props );
  else if ( rampType == QLatin1String( "colorbrewer" ) )
    return QgsColorBrewerColorRamp::create( props );
  else if ( rampType == QLatin1String( "cpt-city" ) )
    return QgsCptCityColorRamp::create( props );
  else if ( rampType == QLatin1String( "preset" ) )
    return QgsPresetSchemeColorRamp::create( props );
  else
  {
    QgsDebugMsg( "unknown colorramp type " + rampType );
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
  QStringList components = colorStr.simplified().split( QRegExp( "(,|\\s)" ) );
  QStringList::iterator it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    QColor result = parseColor( *it, true );
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
  components = colorStr.split( QRegExp( "(,|\n)" ) );
  it = components.begin();
  for ( ; it != components.end(); ++it )
  {
    QColor result = parseColor( *it, true );
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
    QColor result = parseColor( *it, true );
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
    QColor result = parseColor( *it, true );
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
  //attempt to read color data directly from mime
  QColor mimeColor = mimeData->colorData().value<QColor>();
  if ( mimeColor.isValid() )
  {
    hasAlpha = true;
    return mimeColor;
  }

  //attempt to intrepret a color from mime text data
  hasAlpha = false;
  QColor textColor = QgsSymbolLayerUtils::parseColorWithAlpha( mimeData->text(), hasAlpha );
  if ( textColor.isValid() )
  {
    return textColor;
  }

  //could not get color from mime data
  return QColor();
}

QgsNamedColorList QgsSymbolLayerUtils::colorListFromMimeData( const QMimeData *data )
{
  QgsNamedColorList mimeColors;

  //prefer xml format
  if ( data->hasFormat( QStringLiteral( "text/xml" ) ) )
  {
    //get XML doc
    QByteArray encodedData = data->data( QStringLiteral( "text/xml" ) );
    QDomDocument xmlDoc;
    xmlDoc.setContent( encodedData );

    QDomElement dragDataElem = xmlDoc.documentElement();
    if ( dragDataElem.tagName() == QLatin1String( "ColorSchemeModelDragData" ) )
    {
      QDomNodeList nodeList = dragDataElem.childNodes();
      int nChildNodes = nodeList.size();
      QDomElement currentElem;

      for ( int i = 0; i < nChildNodes; ++i )
      {
        currentElem = nodeList.at( i ).toElement();
        if ( currentElem.isNull() )
        {
          continue;
        }

        QPair< QColor, QString> namedColor;
        namedColor.first = QgsSymbolLayerUtils::decodeColor( currentElem.attribute( QStringLiteral( "color" ), QStringLiteral( "255,255,255,255" ) ) );
        namedColor.second = currentElem.attribute( QStringLiteral( "label" ), QString() );

        mimeColors << namedColor;
      }
    }
  }

  if ( mimeColors.length() == 0 && data->hasFormat( QStringLiteral( "application/x-colorobject-list" ) ) )
  {
    //get XML doc
    QByteArray encodedData = data->data( QStringLiteral( "application/x-colorobject-list" ) );
    QDomDocument xmlDoc;
    xmlDoc.setContent( encodedData );

    QDomNodeList colorsNodes = xmlDoc.elementsByTagName( QStringLiteral( "colors" ) );
    if ( colorsNodes.length() > 0 )
    {
      QDomElement colorsElem = colorsNodes.at( 0 ).toElement();
      QDomNodeList colorNodeList = colorsElem.childNodes();
      int nChildNodes = colorNodeList.size();
      QDomElement currentElem;

      for ( int i = 0; i < nChildNodes; ++i )
      {
        //li element
        currentElem = colorNodeList.at( i ).toElement();
        if ( currentElem.isNull() )
        {
          continue;
        }

        QDomNodeList colorNodes = currentElem.elementsByTagName( QStringLiteral( "color" ) );
        QDomNodeList nameNodes = currentElem.elementsByTagName( QStringLiteral( "name" ) );

        if ( colorNodes.length() > 0 )
        {
          QDomElement colorElem = colorNodes.at( 0 ).toElement();

          QStringList colorParts = colorElem.text().simplified().split( ' ' );
          if ( colorParts.length() < 3 )
          {
            continue;
          }

          int red = colorParts.at( 0 ).toDouble() * 255;
          int green = colorParts.at( 1 ).toDouble() * 255;
          int blue = colorParts.at( 2 ).toDouble() * 255;
          QPair< QColor, QString> namedColor;
          namedColor.first = QColor( red, green, blue );
          if ( nameNodes.length() > 0 )
          {
            QDomElement nameElem = nameNodes.at( 0 ).toElement();
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
    QColor mimeColor = data->colorData().value<QColor>();
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
  QDomElement xmlRootElement = xmlDoc.createElement( QStringLiteral( "ColorSchemeModelDragData" ) );
  xmlDoc.appendChild( xmlRootElement );

  QgsNamedColorList::const_iterator colorIt = colorList.constBegin();
  for ( ; colorIt != colorList.constEnd(); ++colorIt )
  {
    QDomElement namedColor = xmlDoc.createElement( QStringLiteral( "NamedColor" ) );
    namedColor.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( ( *colorIt ).first ) );
    namedColor.setAttribute( QStringLiteral( "label" ), ( *colorIt ).second );
    xmlRootElement.appendChild( namedColor );
  }
  mimeData->setData( QStringLiteral( "text/xml" ), xmlDoc.toByteArray() );

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
  mimeData->setText( colorListString.join( QStringLiteral( "\n" ) ) );

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
  stream << "GIMP Palette" << endl;
  if ( paletteName.isEmpty() )
  {
    stream << "Name: QGIS Palette" << endl;
  }
  else
  {
    stream << "Name: " << paletteName << endl;
  }
  stream << "Columns: 4" << endl;
  stream << '#' << endl;

  for ( QgsNamedColorList::ConstIterator colorIt = colors.constBegin(); colorIt != colors.constEnd(); ++colorIt )
  {
    QColor color = ( *colorIt ).first;
    if ( !color.isValid() )
    {
      continue;
    }
    stream << QStringLiteral( "%1 %2 %3" ).arg( color.red(), 3 ).arg( color.green(), 3 ).arg( color.blue(), 3 );
    stream << "\t" << ( ( *colorIt ).second.isEmpty() ? color.name() : ( *colorIt ).second ) << endl;
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
  if ( !line.startsWith( QLatin1String( "GIMP Palette" ) ) )
  {
    ok = false;
    return importedColors;
  }

  //find name line
  while ( !in.atEnd() && !line.startsWith( QLatin1String( "Name:" ) ) && !line.startsWith( '#' ) )
  {
    line = in.readLine();
  }
  if ( line.startsWith( QLatin1String( "Name:" ) ) )
  {
    QRegExp nameRx( "Name:\\s*(\\S.*)$" );
    if ( nameRx.indexIn( line ) != -1 )
    {
      name = nameRx.cap( 1 );
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
  QRegExp rx( "^\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)(\\s.*)?$" );
  while ( !in.atEnd() )
  {
    line = in.readLine();
    if ( rx.indexIn( line ) == -1 )
    {
      continue;
    }
    int red = rx.cap( 1 ).toInt();
    int green = rx.cap( 2 ).toInt();
    int blue = rx.cap( 3 ).toInt();
    QColor color = QColor( red, green, blue );
    if ( !color.isValid() )
    {
      continue;
    }

    //try to read color name
    QString label;
    if ( rx.captureCount() > 3 )
    {
      label = rx.cap( 4 ).simplified();
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

  QRegExp hexColorAlphaRx( "^\\s*#?([0-9a-fA-F]{6})([0-9a-fA-F]{2})\\s*$" );
  int hexColorIndex = hexColorAlphaRx.indexIn( colorStr );

  //color in hex format "#aabbcc", but not #aabbccdd
  if ( hexColorIndex == -1 && QColor::isValidColor( colorStr ) )
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
  if ( hexColorIndex > -1 )
  {
    QString hexColor = hexColorAlphaRx.cap( 1 );
    parsedColor.setNamedColor( QStringLiteral( "#" ) + hexColor );
    bool alphaOk;
    int alphaHex = hexColorAlphaRx.cap( 2 ).toInt( &alphaOk, 16 );

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
    QRegExp hexColorRx2( "^\\s*(?:[0-9a-fA-F]{3}){1,2}\\s*$" );
    if ( hexColorRx2.indexIn( colorStr ) != -1 )
    {
      //add "#" and parse
      parsedColor.setNamedColor( QStringLiteral( "#" ) + colorStr );
      if ( parsedColor.isValid() )
      {
        containsAlpha = false;
        return parsedColor;
      }
    }
  }

  //color in (rrr,ggg,bbb) format, brackets and rgb prefix optional
  QRegExp rgbFormatRx( "^\\s*(?:rgb)?\\(?\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*\\)?\\s*;?\\s*$" );
  if ( rgbFormatRx.indexIn( colorStr ) != -1 )
  {
    int r = rgbFormatRx.cap( 1 ).toInt();
    int g = rgbFormatRx.cap( 2 ).toInt();
    int b = rgbFormatRx.cap( 3 ).toInt();
    parsedColor.setRgb( r, g, b );
    if ( parsedColor.isValid() )
    {
      containsAlpha = false;
      return parsedColor;
    }
  }

  //color in (r%,g%,b%) format, brackets and rgb prefix optional
  QRegExp rgbPercentFormatRx( "^\\s*(?:rgb)?\\(?\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(100|0*\\d{1,2})\\s*%\\s*\\)?\\s*;?\\s*$" );
  if ( rgbPercentFormatRx.indexIn( colorStr ) != -1 )
  {
    int r = std::round( rgbPercentFormatRx.cap( 1 ).toDouble() * 2.55 );
    int g = std::round( rgbPercentFormatRx.cap( 2 ).toDouble() * 2.55 );
    int b = std::round( rgbPercentFormatRx.cap( 3 ).toDouble() * 2.55 );
    parsedColor.setRgb( r, g, b );
    if ( parsedColor.isValid() )
    {
      containsAlpha = false;
      return parsedColor;
    }
  }

  //color in (r,g,b,a) format, brackets and rgba prefix optional
  QRegExp rgbaFormatRx( "^\\s*(?:rgba)?\\(?\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\s*,\\s*(0|0?\\.\\d*|1(?:\\.0*)?)\\s*\\)?\\s*;?\\s*$" );
  if ( rgbaFormatRx.indexIn( colorStr ) != -1 )
  {
    int r = rgbaFormatRx.cap( 1 ).toInt();
    int g = rgbaFormatRx.cap( 2 ).toInt();
    int b = rgbaFormatRx.cap( 3 ).toInt();
    int a = std::round( rgbaFormatRx.cap( 4 ).toDouble() * 255.0 );
    parsedColor.setRgb( r, g, b, a );
    if ( parsedColor.isValid() )
    {
      containsAlpha = true;
      return parsedColor;
    }
  }

  //color in (r%,g%,b%,a) format, brackets and rgba prefix optional
  QRegExp rgbaPercentFormatRx( "^\\s*(?:rgba)?\\(?\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(100|0*\\d{1,2})\\s*%\\s*,\\s*(0|0?\\.\\d*|1(?:\\.0*)?)\\s*\\)?\\s*;?\\s*$" );
  if ( rgbaPercentFormatRx.indexIn( colorStr ) != -1 )
  {
    int r = std::round( rgbaPercentFormatRx.cap( 1 ).toDouble() * 2.55 );
    int g = std::round( rgbaPercentFormatRx.cap( 2 ).toDouble() * 2.55 );
    int b = std::round( rgbaPercentFormatRx.cap( 3 ).toDouble() * 2.55 );
    int a = std::round( rgbaPercentFormatRx.cap( 4 ).toDouble() * 255.0 );
    parsedColor.setRgb( r, g, b, a );
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
  QImage::Format format = image->format();
  if ( format != QImage::Format_ARGB32_Premultiplied && format != QImage::Format_ARGB32 )
  {
    QgsDebugMsg( QStringLiteral( "no alpha channel." ) );
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
  int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
  int alpha = ( radius < 1 )  ? 16 : ( radius > 17 ) ? 1 : tab[radius - 1];

  if ( image.format() != QImage::Format_ARGB32_Premultiplied
       && image.format() != QImage::Format_RGB32 )
  {
    image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied );
  }

  int r1 = rect.top();
  int r2 = rect.bottom();
  int c1 = rect.left();
  int c2 = rect.right();

  int bpl = image.bytesPerLine();
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
    double alphaFactor = alpha / 255.;
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
  double dx = directionPoint.x() - startPoint.x();
  double dy = directionPoint.y() - startPoint.y();
  double length = std::sqrt( dx * dx + dy * dy );
  double scaleFactor = distance / length;
  return QPointF( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}


QStringList QgsSymbolLayerUtils::listSvgFiles()
{
  // copied from QgsMarkerCatalogue - TODO: unify  //#spellok
  QStringList list;
  QStringList svgPaths = QgsApplication::svgPaths();

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir dir( svgPaths[i] );
    Q_FOREACH ( const QString &item, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
      svgPaths.insert( i + 1, dir.path() + '/' + item );
    }

    Q_FOREACH ( const QString &item, dir.entryList( QStringList( "*.svg" ), QDir::Files ) )
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
    QDir dir( svgPaths[i] );
    Q_FOREACH ( const QString &item, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
      svgPaths.insert( i + 1, dir.path() + '/' + item );
    }

    Q_FOREACH ( const QString &item, dir.entryList( QStringList( "*.svg" ), QDir::Files ) )
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

  // we might have a full path...
  if ( QFileInfo::exists( n ) )
    return QFileInfo( n ).canonicalFilePath();

  QString name = n;
  // or it might be an url...
  if ( name.contains( QLatin1String( "://" ) ) )
  {
    QUrl url( name );
    if ( url.isValid() && !url.scheme().isEmpty() )
    {
      if ( url.scheme().compare( QLatin1String( "file" ), Qt::CaseInsensitive ) == 0 )
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
    QString myLocalPath = svgPath + QDir::separator() + name;

    QgsDebugMsgLevel( "Alternative svg path: " + myLocalPath, 3 );
    if ( QFile( myLocalPath ).exists() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Svg found in alternative path" ), 3 );
      return QFileInfo( myLocalPath ).canonicalFilePath();
    }
  }

  return pathResolver.readPath( name );
}

QString QgsSymbolLayerUtils::svgSymbolPathToName( const QString &p, const QgsPathResolver &pathResolver )
{
  if ( p.isEmpty() )
    return QString();

  if ( !QFileInfo::exists( p ) )
    return p;

  QString path = QFileInfo( p ).canonicalFilePath();

  QStringList svgPaths = QgsApplication::svgPaths();

  bool isInSvgPaths = false;
  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QString dir = QFileInfo( svgPaths[i] ).canonicalFilePath();

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

QPointF QgsSymbolLayerUtils::polygonPointOnSurface( const QPolygonF &points )
{
  QPointF centroid = QgsSymbolLayerUtils::polygonCentroid( points );

  // check if centroid inside in polygon
  if ( !QgsSymbolLayerUtils::pointInPolygon( points, centroid ) )
  {
    unsigned int i, pointCount = points.count();

    QgsPolylineXY polyline( pointCount );
    for ( i = 0; i < pointCount; ++i ) polyline[i] = QgsPointXY( points[i].x(), points[i].y() );

    QgsGeometry geom = QgsGeometry::fromPolygonXY( QgsPolygonXY() << polyline );
    if ( !geom.isNull() )
    {
      QgsGeometry pointOnSurfaceGeom = geom.pointOnSurface();

      if ( !pointOnSurfaceGeom.isNull() )
      {
        QgsPointXY point = pointOnSurfaceGeom.asPoint();

        return QPointF( point.x(), point.y() );
      }
    }
  }
  return centroid;
}

bool QgsSymbolLayerUtils::pointInPolygon( const QPolygonF &points, QPointF point )
{
  bool inside = false;

  double x = point.x();
  double y = point.y();

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

QgsExpression *QgsSymbolLayerUtils::fieldOrExpressionToExpression( const QString &fieldOrExpression )
{
  if ( fieldOrExpression.isEmpty() )
    return nullptr;

  QgsExpression *expr = new QgsExpression( fieldOrExpression );
  if ( !expr->hasParserError() )
    return expr;

  // now try with quoted field name
  delete expr;
  QgsExpression *expr2 = new QgsExpression( QgsExpression::quotedColumnRef( fieldOrExpression ) );
  Q_ASSERT( !expr2->hasParserError() );
  return expr2;
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

  int minimumCount = static_cast< int >( classes ) / 3;
  double shrink = 0.75;
  double highBias = 1.5;
  double adjustBias = 0.5 + 1.5 * highBias;
  int divisions = classes;
  double h = highBias;
  double cell;
  bool small = false;
  double dx = maximum - minimum;

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

  double base = std::pow( 10.0, std::floor( std::log10( cell ) ) );
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
  QgsDebugMsg( QStringLiteral( "pretty classes: %1" ).arg( end ) );

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
  double minimumBreak = start * unit;
  //double maximumBreak = end * unit;
  int count = end - start;

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

double QgsSymbolLayerUtils::rescaleUom( double size, QgsUnitTypes::RenderUnit unit, const QgsStringMap &props )
{
  double scale = 1;
  bool roundToUnit = false;
  if ( unit == QgsUnitTypes::RenderUnknownUnit )
  {
    if ( props.contains( QStringLiteral( "uomScale" ) ) )
    {
      bool ok;
      scale = props.value( QStringLiteral( "uomScale" ) ).toDouble( &ok );
      if ( !ok )
      {
        return size;
      }
    }
  }
  else
  {
    if ( props.value( QStringLiteral( "uom" ) ) == QLatin1String( "http://www.opengeospatial.org/se/units/metre" ) )
    {
      switch ( unit )
      {
        case QgsUnitTypes::RenderMillimeters:
          scale = 0.001;
          break;
        case QgsUnitTypes::RenderPixels:
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
        case QgsUnitTypes::RenderMillimeters:
          scale = 1 / 0.28;
          roundToUnit = true;
          break;
        case QgsUnitTypes::RenderInches:
          scale = 1 / 0.28 * 25.4;
          roundToUnit = true;
          break;
        case QgsUnitTypes::RenderPoints:
          scale = 90. /* dots per inch according to OGC SLD */ / 72. /* points per inch */;
          roundToUnit = true;
          break;
        case QgsUnitTypes::RenderPixels:
          // pixel is pixel
          scale = 1;
          break;
        case QgsUnitTypes::RenderMapUnits:
        case QgsUnitTypes::RenderMetersInMapUnits:
          // already handed via uom
          scale = 1;
          break;
        case QgsUnitTypes::RenderPercentage:
        case QgsUnitTypes::RenderUnknownUnit:
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

QPointF QgsSymbolLayerUtils::rescaleUom( QPointF point, QgsUnitTypes::RenderUnit unit, const QgsStringMap &props )
{
  double x = rescaleUom( point.x(), unit, props );
  double y = rescaleUom( point.y(), unit, props );
  return QPointF( x, y );
}

QVector<qreal> QgsSymbolLayerUtils::rescaleUom( const QVector<qreal> &array, QgsUnitTypes::RenderUnit unit, const QgsStringMap &props )
{
  QVector<qreal> result;
  QVector<qreal>::const_iterator it = array.constBegin();
  for ( ; it != array.constEnd(); ++it )
  {
    result.append( rescaleUom( *it, unit, props ) );
  }
  return result;
}

void QgsSymbolLayerUtils::applyScaleDependency( QDomDocument &doc, QDomElement &ruleElem, QgsStringMap &props )
{
  if ( !props.value( QStringLiteral( "scaleMinDenom" ), QString() ).isEmpty() )
  {
    QDomElement scaleMinDenomElem = doc.createElement( QStringLiteral( "se:MinScaleDenominator" ) );
    scaleMinDenomElem.appendChild( doc.createTextNode( qgsDoubleToString( props.value( QStringLiteral( "scaleMinDenom" ) ).toDouble() ) ) );
    ruleElem.appendChild( scaleMinDenomElem );
  }

  if ( !props.value( QStringLiteral( "scaleMaxDenom" ), QString() ).isEmpty() )
  {
    QDomElement scaleMaxDenomElem = doc.createElement( QStringLiteral( "se:MaxScaleDenominator" ) );
    scaleMaxDenomElem.appendChild( doc.createTextNode( qgsDoubleToString( props.value( QStringLiteral( "scaleMaxDenom" ) ).toDouble() ) ) );
    ruleElem.appendChild( scaleMaxDenomElem );
  }
}

void QgsSymbolLayerUtils::mergeScaleDependencies( double mScaleMinDenom, double mScaleMaxDenom, QgsStringMap &props )
{
  if ( !qgsDoubleNear( mScaleMinDenom, 0 ) )
  {
    bool ok;
    double parentScaleMinDenom = props.value( QStringLiteral( "scaleMinDenom" ), QStringLiteral( "0" ) ).toDouble( &ok );
    if ( !ok || parentScaleMinDenom <= 0 )
      props[ QStringLiteral( "scaleMinDenom" )] = QString::number( mScaleMinDenom );
    else
      props[ QStringLiteral( "scaleMinDenom" )] = QString::number( std::max( parentScaleMinDenom, mScaleMinDenom ) );
  }

  if ( !qgsDoubleNear( mScaleMaxDenom, 0 ) )
  {
    bool ok;
    double parentScaleMaxDenom = props.value( QStringLiteral( "scaleMaxDenom" ), QStringLiteral( "0" ) ).toDouble( &ok );
    if ( !ok || parentScaleMaxDenom <= 0 )
      props[ QStringLiteral( "scaleMaxDenom" )] = QString::number( mScaleMaxDenom );
    else
      props[ QStringLiteral( "scaleMaxDenom" )] = QString::number( std::min( parentScaleMaxDenom, mScaleMaxDenom ) );
  }
}

double QgsSymbolLayerUtils::sizeInPixelsFromSldUom( const QString &uom, double size )
{
  double scale = 1.0;

  if ( uom == QLatin1String( "http://www.opengeospatial.org/se/units/metre" ) )
  {
    scale = 1.0 / 0.00028; // from meters to pixels
  }
  else if ( uom == QLatin1String( "http://www.opengeospatial.org/se/units/foot" ) )
  {
    scale = 304.8 / 0.28; // from feet to pixels
  }
  else
  {
    scale = 1.0; // from pixels to pixels (default unit)
  }

  return size * scale;
}
