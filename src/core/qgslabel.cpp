/***************************************************************************
                         qgslabel.cpp - render vector labels
                             -------------------
    begin                : August 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cmath>
#include <limits>

#include <QString>
#include <QFont>
#include <QFontMetrics>

#include <QPainter>
#include <QDomNode>
#include <QDomElement>

#include "qgis.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsrect.h"
#include "qgsmaptopixel.h"
#include "qgscoordinatetransform.h"

#include "qgslabelattributes.h"
#include "qgslabel.h"

// use M_PI define PI 3.141592654
#ifdef WIN32
#undef M_PI
#define M_PI 4*atan(1.0)
#endif

static const char * const ident_ =
  "$Id$";

QgsLabel::QgsLabel( const QgsFieldMap & fields )
{
  mField = fields;
  mLabelFieldIdx.resize( LabelFieldCount );
  for ( int i = 0; i < LabelFieldCount; i++ )
  {
    mLabelFieldIdx[i] = -1;
  }
  mLabelAttributes = new QgsLabelAttributes( true );
}

QgsLabel::~QgsLabel()
{
  delete mLabelAttributes;
}

QString QgsLabel::fieldValue( int attr, QgsFeature &feature )
{
  if ( mLabelFieldIdx[attr] == -1 )
  {
    return QString();
  }

  const QgsAttributeMap& attrs = feature.attributeMap();
  QgsAttributeMap::const_iterator it = attrs.find( mLabelFieldIdx[attr] );

  if ( it != attrs.end() )
  {
    return it->toString();
  }
  else
  {
    return QString();
  }
}

void QgsLabel::renderLabel( QPainter * painter, const QgsRect& viewExtent,
                            const QgsCoordinateTransform* coordinateTransform,
                            const QgsMapToPixel *transform,
                            QgsFeature &feature, bool selected, QgsLabelAttributes *classAttributes,
                            double sizeScale, double rasterScaleFactor )
{

  QPen pen;
  QFont font;
  QString value;
  QString text;

  /* Calc scale (not nice) */
  QgsPoint point;
  point = transform->transform( 0, 0 );
  double x1 = point.x();
  point = transform->transform( 1000, 0 );
  double x2 = point.x();
  double scale = ( x2 - x1 ) * 0.001;

  /* Text */
  value = fieldValue( Text, feature );
  if ( value.isEmpty() )
  {
    text = mLabelAttributes->text();
  }
  else
  {
    text = value;
  }

  /* Font */
  value = fieldValue( Family, feature );
  if ( value.isEmpty() )
  {
    font.setFamily( mLabelAttributes->family() );
  }
  else
  {
    font.setFamily( value );
  }

  double size;
  value = fieldValue( Size, feature );
  if ( value.isEmpty() )
  {
    size =  mLabelAttributes->size();
  }
  else
  {
    size =  value.toDouble();
  }
  int sizeType;
  value = fieldValue( SizeType, feature );
  if ( value.isEmpty() )
    sizeType = mLabelAttributes->sizeType();
  else
  {
    value = value.toLower();
    if ( value.compare( "mapunits" ) == 0 )
      sizeType = QgsLabelAttributes::MapUnits;
    else
      sizeType = QgsLabelAttributes::PointUnits;
  }
  if ( sizeType == QgsLabelAttributes::MapUnits )
  {
    size *= scale;
  }
  else //point units
  {
    double sizeMM = size * 0.3527;
    size = sizeMM * sizeScale;
  }
  
  //Request font larger (multiplied by rasterScaleFactor) as a workaround for the Qt font bug
  //and scale the painter down by rasterScaleFactor when drawing the label
  size *= rasterScaleFactor;

  if ( size > 0.0 )
  {
    font.setPixelSize( size );
  }

  value = fieldValue( Color, feature );
  if ( value.isEmpty() )
  {
    pen.setColor( mLabelAttributes->color() );
  }
  else
  {
    pen.setColor( QColor( value ) );
  }

  value = fieldValue( Bold, feature );
  if ( value.isEmpty() )
  {
    font.setBold( mLabelAttributes->bold() );
  }
  else
  {
    font.setBold(( bool ) value.toInt() );
  }

  value = fieldValue( Italic, feature );
  if ( value.isEmpty() )
  {
    font.setItalic( mLabelAttributes->italic() );
  }
  else
  {
    font.setItalic(( bool ) value.toInt() );
  }

  value = fieldValue( Underline, feature );
  if ( value.isEmpty() )
  {
    font.setUnderline( mLabelAttributes->underline() );
  }
  else
  {
    font.setUnderline(( bool ) value.toInt() );
  }

  //
  QgsPoint overridePoint;
  bool useOverridePoint = false;
  value = fieldValue( XCoordinate, feature );
  if ( !value.isEmpty() )
  {
    overridePoint.setX( value.toDouble() );
    useOverridePoint = true;
  }
  value = fieldValue( YCoordinate, feature );
  if ( !value.isEmpty() )
  {
    overridePoint.setY( value.toDouble() );
    useOverridePoint = true;
  }

  /* Alignment */
  int alignment;
  QFontMetrics fm( font );
  int width, height;

  if ( mLabelAttributes->multilineEnabled() )
  {
    QStringList texts = text.split( "\n" );

    width = 0;
    for ( int i = 0; i < texts.size(); i++ )
    {
      int w = fm.width( texts[i] );
      if ( w > width )
        width = w;
    }

    height = fm.height() * texts.size();
  }
  else
  {
    width = fm.width( text );
    height = fm.height();
  }

  int dx = 0;
  int dy = 0;

  value = fieldValue( Alignment, feature );
  if ( value.isEmpty() )
  {
    alignment = mLabelAttributes->alignment();
  }
  else
  {
    value = value.toLower();

    alignment = 0;

    if ( value.contains( "left" ) )
      alignment |= Qt::AlignLeft;
    else if ( value.contains( "right" ) )
      alignment |= Qt::AlignRight;
    else
      alignment |= Qt::AlignHCenter;

    if ( value.contains( "bottom" ) )
      alignment |= Qt::AlignBottom;
    else if ( value.contains( "top" ) )
      alignment |= Qt::AlignTop;
    else
      alignment |= Qt::AlignVCenter;
  }

  if ( alignment & Qt::AlignLeft )
  {
    dx = 0;
  }
  else if ( alignment & Qt::AlignHCenter )
  {
    dx = -width / 2;
  }
  else if ( alignment & Qt::AlignRight )
  {
    dx = -width;
  }

  if ( alignment & Qt::AlignBottom )
  {
    dy = 0;
  }
  else if ( alignment & Qt::AlignVCenter )
  {
    dy = height / 2;
  }
  else if ( alignment & Qt::AlignTop )
  {
    dy = height;
  }

  // Offset
  double xoffset, yoffset;
  value = fieldValue( XOffset, feature );
  if ( value.isEmpty() )
  {
    xoffset = mLabelAttributes->xOffset();
  }
  else
  {
    xoffset = value.toDouble();
  }
  value = fieldValue( YOffset, feature );
  if ( value.isEmpty() )
  {
    yoffset = mLabelAttributes->yOffset();
  }
  else
  {
    yoffset = value.toDouble();
  }

  // recalc offset to pixels
  if ( mLabelAttributes->offsetType() == QgsLabelAttributes::MapUnits )
  {
    xoffset *= scale;
    yoffset *= scale;
  }
  else
  {
    xoffset = xoffset * 0.3527 * sizeScale;
    yoffset = yoffset * 0.3527 * sizeScale;
  }

  // Angle
  double ang;
  value = fieldValue( Angle, feature );
  if ( value.isEmpty() )
  {
    ang = mLabelAttributes->angle();
  }
  else
  {
    ang = value.toDouble();
  }


  // Work out a suitable position to put the label for the
  // feature. For multi-geometries, put the same label on each
  // part.
  if ( useOverridePoint )
  {
    renderLabel( painter, overridePoint, coordinateTransform,
                 transform, text, font, pen, dx, dy,
                 xoffset, yoffset, ang, width, height, alignment, sizeScale, rasterScaleFactor );
  }
  else
  {
    std::vector<QgsPoint> points;
    labelPoint( points, feature );
    for ( uint i = 0; i < points.size(); ++i )
    {
      renderLabel( painter, points[i], coordinateTransform,
                   transform, text, font, pen, dx, dy,
                   xoffset, yoffset, ang, width, height, alignment, sizeScale, rasterScaleFactor );
    }
  }
}

void QgsLabel::renderLabel( QPainter* painter, QgsPoint point,
                            const QgsCoordinateTransform* coordinateTransform,
                            const QgsMapToPixel* transform,
                            QString text, QFont font, QPen pen,
                            int dx, int dy,
                            double xoffset, double yoffset,
                            double ang,
                            int width, int height, int alignment, double sizeScale, double rasterScaleFactor )
{
  // Convert point to projected units
  if ( coordinateTransform )
  {
    try
    {
      point = coordinateTransform->transform( point );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse ); // unused otherwise
      QgsDebugMsg( "Caught transform error. Skipping rendering this label" );
      return;
    }
  }

  // and then to canvas units
  transform->transform( &point );
  double x = point.x();
  double y = point.y();

  static const double rad = ang * M_PI / 180;

  x = x + xoffset * cos( rad ) - yoffset * sin( rad );
  y = y - xoffset * sin( rad ) - yoffset * cos( rad );
  

  painter->save();
  painter->setFont( font );
  painter->translate( x, y );
  //correct oversampled font size back by scaling painter down
  painter->scale(1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor);
  painter->rotate( -ang );

  //
  // Draw a buffer behind the text if one is desired
  //
  if ( mLabelAttributes->bufferSizeIsSet() && mLabelAttributes->bufferEnabled() )
  {
    double myBufferSize = mLabelAttributes->bufferSize() * 0.3527 * sizeScale * rasterScaleFactor;
    QPen bufferPen;
    if ( mLabelAttributes->bufferColorIsSet() )
    {
      bufferPen.setColor( mLabelAttributes->bufferColor() );
    }
    else //default to a white buffer
    {
      bufferPen.setColor( Qt::white );
    }
    painter->setPen( bufferPen );

    double bufferStepSize; //hack to distinguish pixel devices from logical devices
    if (( sizeScale - 1 ) > 1.5 )
    {
      bufferStepSize = 1;
    }
    else //draw more dense in case of logical devices
    {
      bufferStepSize = 1 / rasterScaleFactor;
    }

    for ( double i = dx - myBufferSize; i <= dx + myBufferSize; i += bufferStepSize )
    {
      for ( double j = dy - myBufferSize; j <= dy + myBufferSize; j += bufferStepSize )
      {
        if ( mLabelAttributes->multilineEnabled() )
          painter->drawText( QRectF( i, j - height, width, height ), alignment, text );
        else
          painter->drawText( QPointF( i, j ), text );
      }
    }
  }

  painter->setPen( pen );
  if ( mLabelAttributes->multilineEnabled() )
    painter->drawText( dx, dy - height, width, height, alignment, text );
  else
    painter->drawText( dx, dy, text );
  painter->restore();
}

void QgsLabel::addRequiredFields( QgsAttributeList& fields )
{
  for ( uint i = 0; i < LabelFieldCount; i++ )
  {
    if ( mLabelFieldIdx[i] == -1 )
      continue;
    bool found = false;
    for ( QgsAttributeList::iterator it = fields.begin(); it != fields.end(); ++it )
    {
      if ( *it == mLabelFieldIdx[i] )
      {
        found = true;
        break;
      }
    }
    if ( !found )
    {
      fields.append( mLabelFieldIdx[i] );
    }
  }
}

void QgsLabel::setFields( const QgsFieldMap & fields )
{
  mField = fields;
}

QgsFieldMap & QgsLabel::fields( void )
{
  return mField;
}

void QgsLabel::setLabelField( int attr, int fieldIndex )
{
  if ( attr >= LabelFieldCount )
    return;

  mLabelFieldIdx[attr] = fieldIndex;
}

QString QgsLabel::labelField( int attr )
{
  if ( attr > LabelFieldCount )
    return QString();

  int fieldIndex = mLabelFieldIdx[attr];
  return mField[fieldIndex].name();
}

QgsLabelAttributes *QgsLabel::layerAttributes( void )
{
  return mLabelAttributes;
}

void QgsLabel::labelPoint( std::vector<QgsPoint>& points, QgsFeature & feature )
{
  QgsGeometry *geometry = feature.geometry();
  unsigned char *geom = geometry->asWkb();
  size_t geomlen = geometry->wkbSize();
  QGis::WkbType wkbType = geometry->wkbType();
  QgsPoint point;

  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    case QGis::WKBLineString25D:
    case QGis::WKBLineString:
    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon:
    {
      labelPoint( point, geom, geomlen );
      points.push_back( point );
    }
    break;

    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
      // Return a position for each individual in the multi-feature
    {
      Q_ASSERT( 1 + sizeof( wkbType ) + sizeof( int ) <= geomlen );
      geom += 1 + sizeof( wkbType );
      int nFeatures = *( unsigned int * )geom;
      geom += sizeof( int );

      unsigned char *feature = geom;
      for ( int i = 0; i < nFeatures && feature; ++i )
      {
        feature = labelPoint( point, feature, geom + geomlen - feature );
        points.push_back( point );
      }
    }
    break;
    default:
      QgsDebugMsg( "Unknown geometry type of " + QString::number( wkbType ) );
  }
}

unsigned char* QgsLabel::labelPoint( QgsPoint& point, unsigned char *geom, size_t geomlen )
{
  // verify that local types match sizes as WKB spec
  Q_ASSERT( sizeof( int ) == 4 );
  Q_ASSERT( sizeof( QGis::WkbType ) == 4 );
  Q_ASSERT( sizeof( double ) == 8 );

  if ( geom == NULL )
  {
    QgsDebugMsg( "empty wkb" );
    return NULL;
  }

  QGis::WkbType wkbType;
#ifndef QT_NO_DEBUG
  unsigned char *geomend = geom + geomlen;
#endif
  Q_ASSERT( geom + 1 + sizeof( wkbType ) <= geomend );

  geom++; // skip endianess
  memcpy( &wkbType, geom, sizeof( wkbType ) );
  geom += sizeof( wkbType );

  int dims = 2;

  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      Q_ASSERT( geom + 2*sizeof( double ) <= geomend );
      double *pts = ( double * )geom;
      point.set( pts[0], pts[1] );
      geom += 2 * sizeof( double );
    }
    break;

    case QGis::WKBLineString25D:
      dims = 3;
    case QGis::WKBLineString: // Line center
    {
      Q_ASSERT( geom + sizeof( int ) <= geomend );
      int nPoints = *( unsigned int * )geom;
      geom += sizeof( int );

      Q_ASSERT( geom + nPoints*sizeof( double )*dims <= geomend );

      // get line center
      double *pts = ( double * )geom;
      double tl = 0.0;
      for ( int i = 1; i < nPoints; i++ )
      {
        double dx = pts[dims*i]   - pts[dims*( i-1 )];
        double dy = pts[dims*i+1] - pts[dims*( i-1 )+1];
        tl += sqrt( dx * dx + dy * dy );
      }
      tl /= 2.0;

      // find line center
      double l = 0.0;
      for ( int i = 1; i < nPoints; i++ )
      {
        double dx = pts[dims*i]   - pts[dims*( i-1 )];
        double dy = pts[dims*i+1] - pts[dims*( i-1 )+1];
        double dl = sqrt( dx * dx + dy * dy );

        if ( l + dl > tl )
        {
          double k = ( tl - l ) / dl;

          point.set( pts[dims*( i-1 )]   + k * dx,
                     pts[dims*( i-1 )+1] + k * dy );
          break;
        }

        l += dl;
      }

      geom += nPoints * sizeof( double ) * dims;
    }
    break;

    case QGis::WKBPolygon25D:
      dims = 3;
    case QGis::WKBPolygon: // centroid of outer ring
    {
      Q_ASSERT( geom + sizeof( int ) <= geomend );
      int nRings = *( unsigned int * )geom;
      geom += sizeof( int );

      for ( int i = 0; i < nRings; ++i )
      {
        Q_ASSERT( geom + sizeof( int ) <= geomend );
        int nPoints = *( unsigned int * )geom;
        geom += sizeof( int );

        Q_ASSERT( geom + nPoints*sizeof( double )*dims <= geomend );

        if ( i == 0 )
        {
          double sx = 0.0, sy = 0.0;
          double *pts = ( double* ) geom;
          for ( int j = 0; j < nPoints - 1; j++ )
          {
            sx += pts[dims*j];
            sy += pts[dims*j+1];
          }
          point.set( sx / ( nPoints - 1 ),
                     sy / ( nPoints - 1 ) );
        }

        geom += nPoints * sizeof( double ) * dims;
      }
    }
    break;

    default:
      // To get here is a bug because our caller should be filtering
      // on wkb type.
      QgsDebugMsg( "unsupported wkb type" );
      return NULL;
  }

  return geom;
}

static int _elementFieldIndex( QDomElement& el )
{
  QString str = el.attribute( "field", "" );
  if ( str == "" )
    return -1;
  else
    return str.toInt();
}

void QgsLabel::readXML( const QDomNode& node )
{
  QgsDebugMsg( " called for layer label properties, got node " + node.nodeName() );

  QDomNode scratchNode;       // Dom node re-used to get current QgsLabel attribute
  QDomElement el;

  int red, green, blue;
  int type;

  /* Text */
  scratchNode = node.namedItem( "label" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``label'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    mLabelAttributes->setText( el.attribute( "text", "" ) );
    setLabelField( Text, _elementFieldIndex( el ) );
  }

  /* Family */
  scratchNode = node.namedItem( "family" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``family'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    mLabelAttributes->setFamily( el.attribute( "name", "" ) );
    setLabelField( Family, _elementFieldIndex( el ) );
  }

  /* Size */
  scratchNode = node.namedItem( "size" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``size'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    if ( !el.hasAttribute( "unitfield" ) )
    {
      type = QgsLabelAttributes::unitsCode( el.attribute( "units", "" ) );
      mLabelAttributes->setSize( el.attribute( "value", "0.0" ).toDouble(), type );
    }
    else
    {
      QString str = el.attribute( "unitfield", "" );
      setLabelField( SizeType, str == "" ? -1 : str.toInt() );
    }
    setLabelField( Size, _elementFieldIndex( el ) );
  }

  /* Bold */
  scratchNode = node.namedItem( "bold" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``bold'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    mLabelAttributes->setBold(( bool )el.attribute( "on", "0" ).toInt() );
    setLabelField( Bold, _elementFieldIndex( el ) );
  }

  /* Italic */
  scratchNode = node.namedItem( "italic" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``italic'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    mLabelAttributes->setItalic(( bool )el.attribute( "on", "0" ).toInt() );
    setLabelField( Italic, _elementFieldIndex( el ) );
  }

  /* Underline */
  scratchNode = node.namedItem( "underline" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``underline'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    mLabelAttributes->setUnderline(( bool )el.attribute( "on", "0" ).toInt() );
    setLabelField( Underline, _elementFieldIndex( el ) );
  }

  /* Color */
  scratchNode = node.namedItem( "color" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``color'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();

    red = el.attribute( "red", "0" ).toInt();
    green = el.attribute( "green", "0" ).toInt();
    blue = el.attribute( "blue", "0" ).toInt();

    mLabelAttributes->setColor( QColor( red, green, blue ) );

    setLabelField( Color, _elementFieldIndex( el ) );
  }

  /* X */
  scratchNode = node.namedItem( "x" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``x'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    setLabelField( XCoordinate, _elementFieldIndex( el ) );
  }

  /* Y */
  scratchNode = node.namedItem( "y" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``y'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    setLabelField( YCoordinate, _elementFieldIndex( el ) );
  }


  /* X,Y offset */
  scratchNode = node.namedItem( "offset" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``offset'' attribute" );
  }
  else
  {
    double xoffset, yoffset;

    el = scratchNode.toElement();

    type = QgsLabelAttributes::unitsCode( el.attribute( "units", "" ) );
    xoffset = el.attribute( "x", "0.0" ).toDouble();
    yoffset = el.attribute( "y", "0.0" ).toDouble();

    mLabelAttributes->setOffset( xoffset, yoffset, type );
    setLabelField( XOffset, el.attribute( "xfield", "0" ).toInt() );
    setLabelField( YOffset, el.attribute( "yfield", "0" ).toInt() );
  }

  /* Angle */
  scratchNode = node.namedItem( "angle" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``angle'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    mLabelAttributes->setAngle( el.attribute( "value", "0.0" ).toDouble() );
    setLabelField( Angle, _elementFieldIndex( el ) );
  }

  /* Alignment */
  scratchNode = node.namedItem( "alignment" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``alignment'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    mLabelAttributes->setAlignment( QgsLabelAttributes::alignmentCode( el.attribute( "value", "" ) ) );
    setLabelField( Alignment, _elementFieldIndex( el ) );
  }


  // Buffer
  scratchNode = node.namedItem( "buffercolor" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``buffercolor'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();

    red = el.attribute( "red", "0" ).toInt();
    green = el.attribute( "green", "0" ).toInt();
    blue = el.attribute( "blue", "0" ).toInt();

    mLabelAttributes->setBufferColor( QColor( red, green, blue ) );
    setLabelField( BufferColor, _elementFieldIndex( el ) );
  }

  scratchNode = node.namedItem( "buffersize" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``bffersize'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();

    type = QgsLabelAttributes::unitsCode( el.attribute( "units", "" ) );
    mLabelAttributes->setBufferSize( el.attribute( "value", "0.0" ).toDouble(), type );
    setLabelField( BufferSize, _elementFieldIndex( el ) );
  }

  scratchNode = node.namedItem( "bufferenabled" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``bufferenabled'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();

    mLabelAttributes->setBufferEnabled(( bool )el.attribute( "on", "0" ).toInt() );
    setLabelField( BufferEnabled, _elementFieldIndex( el ) );
  }

  scratchNode = node.namedItem( "multilineenabled" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``multilineenabled'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();

    mLabelAttributes->setMultilineEnabled(( bool )el.attribute( "on", "0" ).toInt() );
    setLabelField( MultilineEnabled, _elementFieldIndex( el ) );
  }

} // QgsLabel::readXML()



void QgsLabel::writeXML( std::ostream& xml )
{

  xml << "\t\t<labelattributes>\n";

  // Text
  if ( mLabelAttributes->textIsSet() )
  {
    if ( mLabelFieldIdx[Text] != -1 )
    {
      xml << "\t\t\t<label text=\"" << mLabelAttributes->text().toUtf8().constData()
      << "\" field=\"" << mLabelFieldIdx[Text] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<label text=\"" << mLabelAttributes->text().toUtf8().constData()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<label text=\"" << mLabelAttributes->text().toUtf8().constData()
    << "\" field=\"\" />\n";
  }

  // Family
  if ( mLabelAttributes->familyIsSet() && !mLabelAttributes->family().isNull() )
  {
    if ( mLabelFieldIdx[Family] != -1 )
    {
      xml << "\t\t\t<family name=\"" << mLabelAttributes->family().toUtf8().constData()
      << "\" field=\"" << mLabelFieldIdx[Family] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<family name=\"" << mLabelAttributes->family().toUtf8().constData()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<family name=\"Arial\" field=\"\" />\n";
  }

  // size and units
  if ( mLabelAttributes->sizeIsSet() )
  {
    if ( mLabelFieldIdx[Size] != -1 )
    {
      if ( mLabelFieldIdx[SizeType] != -1 )
      {
        xml << "\t\t\t<size value=\"" << mLabelAttributes->size()
        << "\" unitfield=\"" << mLabelFieldIdx[SizeType]
        << "\" field=\"" << mLabelFieldIdx[Size] << "\" />\n";
      }
      else
      {
        xml << "\t\t\t<size value=\"" << mLabelAttributes->size()
        << "\" units=\"" << QgsLabelAttributes::unitsName( mLabelAttributes->sizeType() ).toUtf8().constData()
        << "\" field=\"" << mLabelFieldIdx[Size] << "\" />\n";
      }
    }
    else
    {
      xml << "\t\t\t<size value=\"" << mLabelAttributes->size()
      << "\" units=\"" << QgsLabelAttributes::unitsName( mLabelAttributes->sizeType() ).toUtf8().constData()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<size value=\"12\" units=\"Points\" field=\"\" />\n";
  }


  // bold
  if ( mLabelAttributes->boldIsSet() )
  {
    if ( mLabelFieldIdx[Bold] != -1 )
    {
      xml << "\t\t\t<bold on=\"" << mLabelAttributes->bold()
      << "\" field=\"" << mLabelFieldIdx[Bold] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<bold on=\"" << mLabelAttributes->bold()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<bold on=\"0\" field=\"0\" />\n";
  }

  // italics
  if ( mLabelAttributes->italicIsSet() )
  {
    if ( mLabelFieldIdx[Italic] != -1 )
    {
      xml << "\t\t\t<italic on=\"" << mLabelAttributes->italic()
      << "\" field=\"" << mLabelFieldIdx[Italic] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<italic on=\"" << mLabelAttributes->italic()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<italic on=\"0\" field=\"\" />\n";
  }

  // underline
  if ( mLabelAttributes->underlineIsSet() )
  {
    if ( mLabelFieldIdx[Underline] != -1 )
    {
      xml << "\t\t\t<underline on=\"" << mLabelAttributes->underline()
      << "\" field=\"" << mLabelFieldIdx[Underline] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<underline on=\"" << mLabelAttributes->underline()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<underline on=\"0\" field=\"\" />\n";
  }

  // color
  if ( mLabelAttributes->colorIsSet() )
  {
    if ( mLabelFieldIdx[Color] != -1 )
    {
      xml << "\t\t\t<color red=\"" << mLabelAttributes->color().red()
      << "\" green=\"" << mLabelAttributes->color().green()
      << "\" blue=\"" << mLabelAttributes->color().blue()
      << "\" field=\"" << mLabelFieldIdx[Color] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<color red=\"" << mLabelAttributes->color().red()
      << "\" green=\"" << mLabelAttributes->color().green()
      << "\" blue=\"" << mLabelAttributes->color().blue()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<color red=\"0\" green=\"0\" blue=\"0\" field=\"\" />\n";
  }


  /* X */
  if ( mLabelFieldIdx[XCoordinate] != -1 )
  {
    xml << "\t\t\t<x field=\"" << mLabelFieldIdx[XCoordinate] << "\" />\n";
  }
  else
  {
    xml << "\t\t\t<x field=\"\" />\n";
  }

  /* Y */
  if ( mLabelFieldIdx[YCoordinate] != -1 )
  {
    xml << "\t\t\t<y field=\"" << mLabelFieldIdx[YCoordinate] << "\" />\n";
  }
  else
  {
    xml << "\t\t\t<y field=\"\" />\n";
  }

  // offset
  if ( mLabelAttributes->offsetIsSet() )
  {
    xml << "\t\t\t<offset  units=\"" << QgsLabelAttributes::unitsName( mLabelAttributes->offsetType() ).toUtf8().constData()
    << "\" x=\"" << mLabelAttributes->xOffset() << "\" xfield=\"" << mLabelFieldIdx[XOffset]
    << "\" y=\"" << mLabelAttributes->yOffset() << "\" yfield=\"" << mLabelFieldIdx[YOffset]
    << "\" />\n";
  }

  // Angle
  if ( mLabelAttributes->angleIsSet() )
  {
    if ( mLabelFieldIdx[Angle] != -1 )
    {
      xml << "\t\t\t<angle value=\"" << mLabelAttributes->angle()
      << "\" field=\"" << mLabelFieldIdx[Angle] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<angle value=\"" << mLabelAttributes->angle() << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<angle value=\"\" field=\"\" />\n";
  }

  // alignment
  if ( mLabelAttributes->alignmentIsSet() )
  {
    xml << "\t\t\t<alignment value=\"" << QgsLabelAttributes::alignmentName( mLabelAttributes->alignment() ).toUtf8().constData()
    << "\" field=\"" << mLabelFieldIdx[Alignment] << "\" />\n";
  }

  // buffer color
  if ( mLabelAttributes->bufferColorIsSet() )
  {
    if ( mLabelFieldIdx[BufferColor] != -1 )
    {
      xml << "\t\t\t<buffercolor red=\"" << mLabelAttributes->bufferColor().red()
      << "\" green=\"" << mLabelAttributes->bufferColor().green()
      << "\" blue=\"" << mLabelAttributes->bufferColor().blue()
      << "\" field=\"" << mLabelFieldIdx[BufferColor] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<buffercolor red=\"" << mLabelAttributes->bufferColor().red()
      << "\" green=\"" << mLabelAttributes->bufferColor().green()
      << "\" blue=\"" << mLabelAttributes->bufferColor().blue()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<buffercolor red=\"\" green=\"\" blue=\"\" field=\"\" />\n";
  }

  // buffer size
  if ( mLabelAttributes->bufferSizeIsSet() )
  {
    if ( mLabelFieldIdx[BufferSize] != -1 )
    {
      xml << "\t\t\t<buffersize value=\"" << mLabelAttributes->bufferSize()
      << "\" units=\"" << QgsLabelAttributes::unitsName( mLabelAttributes->bufferSizeType() ).toUtf8().constData()
      << "\" field=\"" << mLabelFieldIdx[BufferSize] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<buffersize value=\"" << mLabelAttributes->bufferSize()
      << "\" units=\"" << QgsLabelAttributes::unitsName( mLabelAttributes->bufferSizeType() ).toUtf8().constData()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<buffersize value=\"\" units=\"\" field=\"\" />\n";
  }

  // buffer enabled
  if ( mLabelAttributes->bufferEnabled() )
  {
    if ( mLabelFieldIdx[BufferEnabled] != -1 )
    {
      xml << "\t\t\t<bufferenabled on=\"" << mLabelAttributes->bufferEnabled()
      << "\" field=\"" << mLabelFieldIdx[BufferEnabled] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<bufferenabled on=\"" << mLabelAttributes->bufferEnabled()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<bufferenabled on=\"" << "\" field=\"" << "\" />\n";
  }

  // multiline enabled
  if ( mLabelAttributes->multilineEnabled() )
  {
    if ( mLabelFieldIdx[MultilineEnabled] != -1 )
    {
      xml << "\t\t\t<multilineenabled on=\"" << mLabelAttributes->multilineEnabled()
      << "\" field=\"" << mLabelFieldIdx[MultilineEnabled] << "\" />\n";
    }
    else
    {
      xml << "\t\t\t<multilineenabled on=\"" << mLabelAttributes->multilineEnabled()
      << "\" field=\"\" />\n";
    }
  }
  else
  {
    xml << "\t\t\t<multilineenabled on=\"" << "\" field=\"" << "\" />\n";
  }

  xml << "\t\t</labelattributes>\n";
}

