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
#include "qgsrectangle.h"
#include "qgsmaptopixel.h"
#include "qgscoordinatetransform.h"
#include "qgsrendercontext.h"

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
    : mMinScale( 0 ),
    mMaxScale( 100000000 ),
    mScaleBasedVisibility( false )
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

void QgsLabel::renderLabel( QgsRenderContext &renderContext,
                            QgsFeature &feature, bool selected,
                            QgsLabelAttributes *classAttributes )
{
  QPen pen;
  QFont font;
  QString value;
  QString text;

  /* Calc scale (not nice) */
  QgsPoint point;
  point = renderContext.mapToPixel().transform( 0, 0 );
  double x1 = point.x();
  point = renderContext.mapToPixel().transform( 1000, 0 );
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
    size = sizeMM * renderContext.scaleFactor();
  }

  //Request font larger (multiplied by rasterScaleFactor) as a workaround for the Qt font bug
  //and scale the painter down by rasterScaleFactor when drawing the label
  size *= renderContext.rasterScaleFactor();

  if (( int )size <= 0 )
    // skip too small labels
    return;

  font.setPixelSize( size );

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

  value = fieldValue( StrikeOut, feature );
  if ( value.isEmpty() )
  {
    font.setStrikeOut( mLabelAttributes->strikeOut() );
  }
  else
  {
    font.setStrikeOut(( bool ) value.toInt() );
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
    xoffset = xoffset * 0.3527 * renderContext.scaleFactor();
    yoffset = yoffset * 0.3527 * renderContext.scaleFactor();
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
    renderLabel( renderContext, overridePoint, text, font, pen, dx, dy,
                 xoffset, yoffset, ang, width, height, alignment );
  }
  else
  {
    std::vector<labelpoint> points;
    labelPoint( points, feature );
    for ( uint i = 0; i < points.size(); ++i )
    {
      renderLabel( renderContext, points[i].p, text, font, pen, dx, dy,
                   xoffset, yoffset, mLabelAttributes->angleIsAuto() ? points[i].angle : ang, width, height, alignment );
    }
  }
}

void QgsLabel::renderLabel( QgsRenderContext &renderContext,
                            QgsPoint point,
                            QString text, QFont font, QPen pen,
                            int dx, int dy,
                            double xoffset, double yoffset,
                            double ang,
                            int width, int height, int alignment )
{
  QPainter *painter = renderContext.painter();

  // Convert point to projected units
  if ( renderContext.coordinateTransform() )
  {
    try
    {
      point = renderContext.coordinateTransform()->transform( point );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse ); // unused otherwise
      QgsDebugMsg( "Caught transform error. Skipping rendering this label" );
      return;
    }
  }

  // and then to canvas units
  renderContext.mapToPixel().transform( &point );
  double x = point.x();
  double y = point.y();

  double rad = ang * M_PI / 180;

  x = x + xoffset * cos( rad ) - yoffset * sin( rad );
  y = y - xoffset * sin( rad ) - yoffset * cos( rad );


  painter->save();
  painter->setFont( font );
  painter->translate( x, y );
  //correct oversampled font size back by scaling painter down
  painter->scale( 1.0 / renderContext.rasterScaleFactor(), 1.0 / renderContext.rasterScaleFactor() );
  painter->rotate( -ang );

  //
  // Draw a buffer behind the text if one is desired
  //
  if ( mLabelAttributes->bufferSizeIsSet() && mLabelAttributes->bufferEnabled() )
  {
    double myBufferSize = mLabelAttributes->bufferSize() * 0.3527 * renderContext.scaleFactor() * renderContext.rasterScaleFactor();
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
    if (( renderContext.scaleFactor() - 1 ) > 1.5 )
    {
      bufferStepSize = 1;
    }
    else //draw more dense in case of logical devices
    {
      bufferStepSize = 1 / renderContext.rasterScaleFactor();
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

void QgsLabel::addRequiredFields( QgsAttributeList& fields ) const
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

QString QgsLabel::labelField( int attr ) const
{
  if ( attr > LabelFieldCount )
    return QString();

  int fieldIndex = mLabelFieldIdx[attr];
  return mField[fieldIndex].name();
}

QgsLabelAttributes *QgsLabel::labelAttributes( void )
{
  return mLabelAttributes;
}
// @note this will be deprecated use attributes rather
QgsLabelAttributes *QgsLabel::layerAttributes( void )
{
  return mLabelAttributes;
}

void QgsLabel::labelPoint( std::vector<labelpoint>& points, QgsFeature & feature )
{
  QgsGeometry *geometry = feature.geometry();
  unsigned char *geom = geometry->asWkb();
  size_t geomlen = geometry->wkbSize();
  QGis::WkbType wkbType = geometry->wkbType();
  labelpoint point;

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

unsigned char* QgsLabel::labelPoint( labelpoint& point, unsigned char *geom, size_t geomlen )
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

  geom++; // skip endianness
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
      point.p.set( pts[0], pts[1] );
      point.angle = 0.0;
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

          point.p.set( pts[dims*( i-1 )]   + k * dx,
                       pts[dims*( i-1 )+1] + k * dy );
          point.angle = atan2( dy, dx ) * 180.0 * M_1_PI;
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
          point.p.set( sx / ( nPoints - 1 ),
                       sy / ( nPoints - 1 ) );
          point.angle = 0.0;
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

bool QgsLabel::readLabelField( QDomElement &el, int attr, QString prefix = "field" )
{
  QString name = prefix + "name";

  if ( el.hasAttribute( name ) )
  {
    name = el.attribute( name );

    QgsFieldMap::const_iterator field_it = mField.constBegin();
    for ( ; field_it != mField.constEnd(); ++field_it )
    {
      if ( field_it.value().name() == name )
      {
        break;
      }
    }

    if ( field_it != mField.constEnd() )
    {
      mLabelFieldIdx[attr] = field_it.key();
      return true;
    }
  }
  else if ( el.hasAttribute( prefix ) )
  {
    QString value = el.attribute( prefix );
    mLabelFieldIdx[attr] = value.isEmpty() ? -1 : value.toInt();
    return true;
  }

  mLabelFieldIdx[attr] = -1;
  return false;
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
    readLabelField( el, Text );
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
    readLabelField( el, Family );
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
    if ( !el.hasAttribute( "unitfield" ) && !el.hasAttribute( "unitfieldname" ) )
    {
      type = QgsLabelAttributes::unitsCode( el.attribute( "units", "" ) );
      mLabelAttributes->setSize( el.attribute( "value", "0.0" ).toDouble(), type );
    }
    else
    {
      readLabelField( el, SizeType, "unitfield" );
    }
    readLabelField( el, Size );
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
    readLabelField( el, Bold );
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
    readLabelField( el, Italic );
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
    readLabelField( el, Underline );
  }

  /* Strikeout */
  scratchNode = node.namedItem( "strikeout" );

  if ( scratchNode.isNull() )
  {
    QgsDebugMsg( "couldn't find QgsLabel ``strikeout'' attribute" );
  }
  else
  {
    el = scratchNode.toElement();
    mLabelAttributes->setStrikeOut(( bool )el.attribute( "on", "0" ).toInt() );
    readLabelField( el, StrikeOut );
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

    readLabelField( el, Color );
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
    readLabelField( el, XCoordinate );
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
    readLabelField( el, YCoordinate );
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
    readLabelField( el, XOffset, "xfield" );
    readLabelField( el, YOffset, "yfield" );
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
    readLabelField( el, Angle );
    mLabelAttributes->setAutoAngle( el.attribute( "auto", "0" ) == "1" );
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
    readLabelField( el, Alignment );
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
    readLabelField( el, BufferColor );
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
    readLabelField( el, BufferSize );
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
    readLabelField( el, BufferEnabled );
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
    readLabelField( el, MultilineEnabled );
  }

} // QgsLabel::readXML()



void QgsLabel::writeXML( QDomNode & layer_node, QDomDocument & document ) const
{
  QDomElement labelattributes = document.createElement( "labelattributes" );

  // Text
  QDomElement label = document.createElement( "label" );
  label.setAttribute( "text", mLabelAttributes->text() );
  if ( mLabelAttributes->textIsSet() && mLabelFieldIdx[Text] != -1 )
  {
    label.setAttribute( "fieldname", labelField( Text ) );
  }
  else
  {
    label.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( label );

  // Family
  QDomElement family = document.createElement( "family" );
  if ( mLabelAttributes->familyIsSet() && !mLabelAttributes->family().isNull() )
  {
    if ( mLabelFieldIdx[Family] != -1 )
    {
      family.setAttribute( "name", mLabelAttributes->family() );
      family.setAttribute( "fieldname", labelField( Family ) );
    }
    else
    {
      family.setAttribute( "name", mLabelAttributes->family() );
      family.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    family.setAttribute( "name", "Arial" );
    family.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( family );

  // size and units
  QDomElement size = document.createElement( "size" );
  size.setAttribute( "value", mLabelAttributes->size() );
  if ( mLabelAttributes->sizeIsSet() )
  {
    if ( mLabelFieldIdx[Size] != -1 )
    {
      if ( mLabelFieldIdx[SizeType] != -1 )
      {
        size.setAttribute( "unitfieldname", labelField( SizeType ) );
      }
      else
      {
        size.setAttribute( "units", QgsLabelAttributes::unitsName( mLabelAttributes->sizeType() ) );
      }
      size.setAttribute( "fieldname", labelField( Size ) );
    }
    else
    {
      size.setAttribute( "units", QgsLabelAttributes::unitsName( mLabelAttributes->sizeType() ) );
      size.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    size.setAttribute( "value", "12" );
    size.setAttribute( "units", "Points" );
    size.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( size );

  // bold
  QDomElement bold = document.createElement( "bold" );
  if ( mLabelAttributes->boldIsSet() )
  {
    bold.setAttribute( "on", mLabelAttributes->bold() );
    if ( mLabelFieldIdx[Bold] != -1 )
    {
      bold.setAttribute( "fieldname", labelField( Bold ) );
    }
    else
    {
      bold.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    bold.setAttribute( "on", 0 );
    bold.setAttribute( "fieldname", 0 );
  }
  labelattributes.appendChild( bold );

  // italics
  QDomElement italic = document.createElement( "italic" );
  if ( mLabelAttributes->italicIsSet() )
  {
    italic.setAttribute( "on", mLabelAttributes->italic() );
    if ( mLabelFieldIdx[Italic] != -1 )
    {
      italic.setAttribute( "fieldname", labelField( Italic ) );
    }
    else
    {
      italic.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    italic.setAttribute( "on", "0" );
    italic.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( italic );

  // underline
  QDomElement underline = document.createElement( "underline" );
  if ( mLabelAttributes->underlineIsSet() )
  {
    underline.setAttribute( "on", mLabelAttributes->underline() );
    if ( mLabelFieldIdx[Underline] != -1 )
    {
      underline.setAttribute( "fieldname", labelField( Underline ) );
    }
    else
    {
      underline.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    underline.setAttribute( "on", 0 );
    underline.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( underline );

  // strikeout
  QDomElement strikeOut = document.createElement( "strikeout" );
  if ( mLabelAttributes->strikeOutIsSet() )
  {
    strikeOut.setAttribute( "on", mLabelAttributes->strikeOut() );
    if ( mLabelFieldIdx[StrikeOut] != -1 )
    {
      strikeOut.setAttribute( "fieldname", labelField( StrikeOut ) );
    }
    else
    {
      strikeOut.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    strikeOut.setAttribute( "on", 0 );
    strikeOut.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( strikeOut );

  // color
  QDomElement color = document.createElement( "color" );
  if ( mLabelAttributes->colorIsSet() )
  {
    color.setAttribute( "red", mLabelAttributes->color().red() );
    color.setAttribute( "green", mLabelAttributes->color().green() );
    color.setAttribute( "blue", mLabelAttributes->color().blue() );
    if ( mLabelFieldIdx[Color] != -1 )
    {
      color.setAttribute( "fieldname", labelField( Color ) );
    }
    else
    {
      color.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    color.setAttribute( "red", 0 );
    color.setAttribute( "green", 0 );
    color.setAttribute( "blue", 0 );
    color.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( color );

  /* X */
  QDomElement x = document.createElement( "x" );
  if ( mLabelFieldIdx[XCoordinate] != -1 )
  {
    x.setAttribute( "fieldname", labelField( XCoordinate ) );
  }
  else
  {
    x.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( x );

  /* Y */
  QDomElement y = document.createElement( "y" );
  if ( mLabelFieldIdx[YCoordinate] != -1 )
  {
    y.setAttribute( "fieldname", labelField( YCoordinate ) );
  }
  else
  {
    y.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( y );

  // offset
  if ( mLabelAttributes->offsetIsSet() )
  {
    QDomElement offset = document.createElement( "offset" );
    offset.setAttribute( "units", QgsLabelAttributes::unitsName( mLabelAttributes->offsetType() ) );
    offset.setAttribute( "x", mLabelAttributes->xOffset() );
    offset.setAttribute( "xfieldname", labelField( XOffset ) );
    offset.setAttribute( "y", mLabelAttributes->yOffset() );
    offset.setAttribute( "yfieldname", labelField( YOffset ) );
    labelattributes.appendChild( offset );
  }

  // Angle
  QDomElement angle = document.createElement( "angle" );
  if ( mLabelAttributes->angleIsSet() )
  {
    angle.setAttribute( "value", mLabelAttributes->angle() );
    if ( mLabelFieldIdx[Angle] != -1 )
    {
      angle.setAttribute( "fieldname", labelField( Angle ) );
    }
    else
    {
      angle.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    angle.setAttribute( "value", "" );
    angle.setAttribute( "fieldname", "" );
  }
  angle.setAttribute( "auto", mLabelAttributes->angleIsAuto() ? "1" : "0" );
  labelattributes.appendChild( angle );

  // alignment
  if ( mLabelAttributes->alignmentIsSet() )
  {
    QDomElement alignment = document.createElement( "alignment" );
    alignment.setAttribute( "value", QgsLabelAttributes::alignmentName( mLabelAttributes->alignment() ) );
    alignment.setAttribute( "fieldname", labelField( Alignment ) );
    labelattributes.appendChild( alignment );
  }

  // buffer color
  QDomElement buffercolor = document.createElement( "buffercolor" );
  if ( mLabelAttributes->bufferColorIsSet() )
  {
    buffercolor.setAttribute( "red", mLabelAttributes->bufferColor().red() );
    buffercolor.setAttribute( "green", mLabelAttributes->bufferColor().green() );
    buffercolor.setAttribute( "blue", mLabelAttributes->bufferColor().blue() );
    if ( mLabelFieldIdx[BufferColor] != -1 )
    {
      buffercolor.setAttribute( "fieldname", labelField( BufferColor ) );
    }
    else
    {
      buffercolor.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    buffercolor.setAttribute( "red", "" );
    buffercolor.setAttribute( "green", "" );
    buffercolor.setAttribute( "blue", "" );
    buffercolor.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( buffercolor );

  // buffer size
  QDomElement buffersize = document.createElement( "buffersize" );
  if ( mLabelAttributes->bufferSizeIsSet() )
  {
    buffersize.setAttribute( "value", mLabelAttributes->bufferSize() );
    buffersize.setAttribute( "units", QgsLabelAttributes::unitsName( mLabelAttributes->bufferSizeType() ) );
    if ( mLabelFieldIdx[BufferSize] != -1 )
    {
      buffersize.setAttribute( "fieldname", labelField( BufferSize ) );
    }
    else
    {
      buffersize.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    buffersize.setAttribute( "value", "" );
    buffersize.setAttribute( "units", "" );
    buffersize.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( buffersize );

  // buffer enabled
  QDomElement bufferenabled = document.createElement( "bufferenabled" );
  if ( mLabelAttributes->bufferEnabled() )
  {
    bufferenabled.setAttribute( "on", mLabelAttributes->bufferEnabled() );
    if ( mLabelFieldIdx[BufferEnabled] != -1 )
    {
      bufferenabled.setAttribute( "fieldname", labelField( BufferEnabled ) );
    }
    else
    {
      bufferenabled.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    bufferenabled.setAttribute( "on", "" );
    bufferenabled.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( bufferenabled );

  // multiline enabled
  QDomElement multilineenabled = document.createElement( "multilineenabled" );
  if ( mLabelAttributes->multilineEnabled() )
  {
    multilineenabled.setAttribute( "on", mLabelAttributes->multilineEnabled() );
    if ( mLabelFieldIdx[MultilineEnabled] != -1 )
    {
      multilineenabled.setAttribute( "fieldname", labelField( MultilineEnabled ) );
    }
    else
    {
      multilineenabled.setAttribute( "fieldname", "" );
    }
  }
  else
  {
    multilineenabled.setAttribute( "on", "" );
    multilineenabled.setAttribute( "fieldname", "" );
  }
  labelattributes.appendChild( multilineenabled );

  layer_node.appendChild( labelattributes );
}

void QgsLabel::setScaleBasedVisibility( bool theVisibilityFlag )
{
  mScaleBasedVisibility = theVisibilityFlag;
}

bool QgsLabel::scaleBasedVisibility() const
{
  return mScaleBasedVisibility;
}

void QgsLabel::setMinScale( float theMinScale )
{
  mMinScale = theMinScale;
}

float QgsLabel::minScale() const
{
  return mMinScale;
}

void QgsLabel::setMaxScale( float theMaxScale )
{
  mMaxScale = theMaxScale;
}

float QgsLabel::maxScale() const
{
  return mMaxScale;
}
