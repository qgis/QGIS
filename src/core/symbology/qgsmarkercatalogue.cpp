/***************************************************************************
  qgsmarkercatalogue.cpp
  -------------------
begin                : March 2005
copyright            : (C) 2005 by Radim Blazek
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
#include <assert.h>

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QImage>
#include <QString>
#include <QStringList>
#include <QRect>
#include <QPointF>
#include <QPolygonF>
#include <QDir>
#include <QPicture>
#include <QSvgRenderer>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmarkercatalogue.h"
#include "qgslogger.h"

// MSVC compiler doesn't have defined M_PI in math.h
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#define DEG2RAD(x)    ((x)*M_PI/180)

//#define IMAGEDEBUG

QgsMarkerCatalogue *QgsMarkerCatalogue::mMarkerCatalogue = 0;

QgsMarkerCatalogue::QgsMarkerCatalogue()
{
  refreshList();
}

void QgsMarkerCatalogue::refreshList()
{
  // Init list
  mList.clear();

  // Hardcoded markers
  mList.append( "hard:circle" );
  mList.append( "hard:rectangle" );
  mList.append( "hard:diamond" );
  mList.append( "hard:pentagon" );
  mList.append( "hard:cross" );
  mList.append( "hard:cross2" );
  mList.append( "hard:triangle" );
  mList.append( "hard:equilateral_triangle" );
  mList.append( "hard:star" );
  mList.append( "hard:regular_star" );
  mList.append( "hard:arrow" );

  // SVG
  QStringList svgPaths = QgsApplication::svgPaths();
  QgsDebugMsg( QString( "Application SVG Search paths: \n%1" ).arg( svgPaths.join( "\n" ) ) );

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir dir( svgPaths[i] );
    foreach( QString item, dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
      svgPaths.insert( i + 1, dir.path() + "/" + item );
    }

    QgsDebugMsg( QString( "Looking for svgs in %1" ).arg( dir.path() ) );

    foreach( QString item, dir.entryList( QStringList( "*.svg" ), QDir::Files ) )
    {
      // TODO test if it is correct SVG
      mList.append( "svg:" + dir.path() + "/" + item );
    }
  }

  emit markersRefreshed();
}

QStringList QgsMarkerCatalogue::list()
{
  return mList;
}

QgsMarkerCatalogue::~QgsMarkerCatalogue()
{
}

QgsMarkerCatalogue *QgsMarkerCatalogue::instance()
{
  if ( !QgsMarkerCatalogue::mMarkerCatalogue )
  {
    QgsMarkerCatalogue::mMarkerCatalogue = new QgsMarkerCatalogue();
  }

  return QgsMarkerCatalogue::mMarkerCatalogue;
}

QImage QgsMarkerCatalogue::imageMarker( QString fullName, double size, QPen pen, QBrush brush, double opacity )
{

  //
  // First prepare the paintdevice that the marker will be drawn onto
  //

  // Introduce a minimum size, we don't want it to disappear.
  if ( size < 4 )
  {
    size = 4;
  }

  QImage myImage;
  int imageSize;
  if ( fullName.startsWith( "hard:" ) )
  {
    int pw = (( pen.width() == 0 ? 1 : pen.width() ) + 1 ) / 2 * 2; // make even (round up); handle cosmetic pen
    imageSize = (( int ) size + pw ) / 2 * 2 + 1; //  make image width, height odd; account for pen width
    myImage = QImage( imageSize, imageSize, QImage::Format_ARGB32_Premultiplied );
  }
  else
  {
    // TODO Change this logic so width is size and height is same
    // proportion of scale factor as in oritignal SVG TS XXX
    //QPixmap myPixmap = QPixmap(width,height);
    imageSize = (( int ) size ) / 2 * 2 + 1; //  make image width, height odd
    myImage = QImage( imageSize, imageSize, QImage::Format_ARGB32_Premultiplied );
  }

  // starting with transparent QImage
  myImage.fill( 0 );

  QPainter myPainter;
  myPainter.begin( &myImage );
  myPainter.setRenderHint( QPainter::Antialiasing );
  myPainter.setOpacity( opacity );

  //
  // Now pass the paintdevice along to have the marker rendered on it
  //
  if ( fullName.startsWith( "svg:" ) )
  {
    if ( svgMarker( &myPainter, fullName.mid( 4 ), size ) )
      return myImage;

    QgsDebugMsg( QString( "%1 not found - replacing with hard:circle" ).arg( fullName ) );
    fullName = "hard:circle";
  }

  if ( fullName.startsWith( "font:" ) )
  {
    if ( fontMarker( &myPainter, fullName.mid( 5 ), size ) )
      return myImage;

    QgsDebugMsg( QString( "%1 not found - replacing with hard:circle" ).arg( fullName ) );
    fullName = "hard:circle";
  }

  if ( fullName.endsWith( ".svg", Qt::CaseInsensitive ) )
  {
    if ( svgMarker( &myPainter, fullName, size ) )
      return myImage;

    QgsDebugMsg( QString( "%1 not found - replacing with hard:circle" ).arg( fullName ) );
    fullName = "hard:circle";
  }

  if ( fullName.startsWith( "hard:" ) )
  {
    hardMarker( &myPainter, imageSize, fullName.mid( 5 ), size, pen, brush );
#ifdef IMAGEDEBUG
    QgsDebugMsg( "*** Saving hard marker to hardMarker.png ***" );
#ifdef QGISDEBUG
    myImage.save( "hardMarker.png" );
#endif
#endif
    return myImage;
  }

  return QImage(); // empty
}

QPicture QgsMarkerCatalogue::pictureMarker( QString fullName, double size, QPen pen, QBrush brush, double opacity )
{

  //
  // First prepare the paintdevice that the marker will be drawn onto
  //
  QPicture myPicture;
  if ( fullName.left( 5 ) == "hard:" )
  {
    //Note teh +1 offset below is required because the
    //otherwise the icons are getting clipped
    myPicture = QPicture( size + 1 );
  }
  else
  {
    // TODO Change this logic so width is size and height is same
    // proportion of scale factor as in oritignal SVG TS XXX
    if ( size < 1 ) size = 1;
    myPicture = QPicture( size );
  }

  QPainter myPainter( &myPicture );
  myPainter.setRenderHint( QPainter::Antialiasing );
  myPainter.setOpacity( opacity );

  //
  // Now pass the paintdevice along to have the marker rndered on it
  //
  if ( fullName.left( 4 ) == "svg:" )
  {
    if ( svgMarker( &myPainter, fullName.mid( 4 ), size ) )
      return myPicture;

    QgsDebugMsg( QString( "%1 not found - replacing with hard:circle" ).arg( fullName ) );
    fullName = "hard:circle";
  }

  if ( fullName.left( 5 ) == "hard:" )
  {
    hardMarker( &myPainter, ( int ) size, fullName.mid( 5 ), size, pen, brush );
    return myPicture;
  }

  return QPicture(); // empty
}

bool QgsMarkerCatalogue::fontMarker( QPainter *thepPainter, QString fullName, double scaleFactor )
{
  QStringList args = fullName.split( "," );
  if ( args.size() == 0 )
    return false;

  QChar c;

  if ( args.size() > 0 )
  {
    if ( args[0] == "#" )
    {
      c = QChar( '#' );
    }
    else if ( args[0].startsWith( "#" ) )
    {
      c = QChar( args[0].mid( 1 ).toInt() );
    }
    else
    {
      c = args[0][0];
    }
  }

  QString family = args.size() >= 2 ? args[1] : "Helvetica";
  int weight = args.size() >= 3 ? args[2].toInt() : -1;
  int italic = args.size() >= 4 ? args[3].toInt() != 0 : false;

  thepPainter->setFont( QFont( family, scaleFactor, weight, italic ) );
  thepPainter->drawText( 0, 0, c );

  return true;
}

bool QgsMarkerCatalogue::svgMarker( QPainter * thepPainter, QString fileName, double scaleFactor )
{
  QSvgRenderer mySVG;
  if ( !mySVG.load( fileName ) )
    return false;

  mySVG.render( thepPainter );

  return true;
}

void QgsMarkerCatalogue::hardMarker( QPainter * thepPainter, int imageSize, QString name, double s, QPen pen, QBrush brush )
{
  // Size of polygon symbols is calculated so that the boundingbox is circumscribed
  // around a circle with diameter mPointSize

#if 0
  s = s - pen.widthF(); // to make the overall size of the symbol at the specified size
#else
  // the size of the base symbol is at the specified size; the outline is applied additionally
#endif

  // Circle radius, is used for other figures also, when compensating for line
  // width is necessary.
  double r = s / 2; // get half the size of the figure to be rendered (the radius)

  QgsDebugMsgLevel( QString( "Hard marker size %1" ).arg( s ), 3 );

  // Find out center coordinates of the QImage to draw on.
  double x_c = ( double )( imageSize / 2 ) + 0.5; // add 1/2 pixel for proper rounding when the figure's coordinates are added
  double y_c = x_c;  // is square image

  thepPainter->setPen( pen );
  thepPainter->setBrush( brush );

  QgsDebugMsgLevel( QString( "Hard marker radius %1" ).arg( r ), 3 );

  // If radius is 0, draw a circle, so it wont disappear.
  if ( name == "circle" || r < 1 )
  {
    // "A stroked ellipse has a size of rectangle.size() plus the pen width."
    // (from Qt doc)

    thepPainter->drawEllipse( QRectF( x_c - r, y_c - r, s, s ) ); // x,y,w,h
  }
  else if ( name == "rectangle" )
  {
    thepPainter->drawRect( QRectF( x_c - r, y_c - r, s, s ) );  // x,y,w,h
  }
  else if ( name == "diamond" )
  {
    QPolygonF pa;
    pa << QPointF( x_c - r, y_c )
    << QPointF( x_c, y_c + r )
    << QPointF( x_c + r, y_c )
    << QPointF( x_c, y_c - r );
    thepPainter->drawPolygon( pa );
  }
  else if ( name == "pentagon" )
  {
    QPolygonF pa;
    pa << QPointF( x_c + ( r * sin( DEG2RAD( 288.0 ) ) ), y_c - ( r * cos( DEG2RAD( 288.0 ) ) ) )
    << QPointF( x_c + ( r * sin( DEG2RAD( 216.0 ) ) ), y_c - ( r * cos( DEG2RAD( 216.0 ) ) ) )
    << QPointF( x_c + ( r * sin( DEG2RAD( 144.0 ) ) ), y_c - ( r * cos( DEG2RAD( 144.0 ) ) ) )
    << QPointF( x_c + ( r * sin( DEG2RAD( 72.0 ) ) ), y_c - ( r * cos( DEG2RAD( 72.0 ) ) ) )
    << QPointF( x_c, y_c - r );
    thepPainter->drawPolygon( pa );
  }
  else if ( name == "cross" )
  {
    thepPainter->drawLine( QPointF( x_c - r, y_c ), QPointF( x_c + r, y_c ) ); // horizontal
    thepPainter->drawLine( QPointF( x_c, y_c - r ), QPointF( x_c, y_c + r ) ); // vertical
  }
  else if ( name == "cross2" )
  {
    thepPainter->drawLine( QPointF( x_c - r, y_c - r ), QPointF( x_c + r, y_c + r ) );
    thepPainter->drawLine( QPointF( x_c - r, y_c + r ), QPointF( x_c + r, y_c - r ) );
  }
  else if ( name == "triangle" )
  {
    QPolygonF pa;
    pa << QPointF( x_c - r, y_c + r )
    << QPointF( x_c + r, y_c + r )
    << QPointF( x_c, y_c - r );
    thepPainter->drawPolygon( pa );
  }
  else if ( name == "equilateral_triangle" )
  {
    QPolygonF pa;
    pa << QPointF( x_c + ( r * sin( DEG2RAD( 240.0 ) ) ), y_c - ( r * cos( DEG2RAD( 240.0 ) ) ) )
    << QPointF( x_c + ( r * sin( DEG2RAD( 120.0 ) ) ), y_c - ( r * cos( DEG2RAD( 120.0 ) ) ) )
    << QPointF( x_c, y_c - r ); // 0
    thepPainter->drawPolygon( pa );
  }
  else if ( name == "star" )
  {
    double oneSixth = 2 * r / 6;

    QPolygonF pa;
    pa << QPointF( x_c, y_c - r )
    << QPointF( x_c - oneSixth, y_c - oneSixth )
    << QPointF( x_c - r, y_c - oneSixth )
    << QPointF( x_c - oneSixth, y_c )
    << QPointF( x_c - r, y_c + r )
    << QPointF( x_c, y_c + oneSixth )
    << QPointF( x_c + r, y_c + r )
    << QPointF( x_c + oneSixth, y_c )
    << QPointF( x_c + r, y_c - oneSixth )
    << QPointF( x_c + oneSixth, y_c - oneSixth );
    thepPainter->drawPolygon( pa );
  }
  else if ( name == "regular_star" )
  {
    // control the 'fatness' of the star:  cos(72)/cos(36) gives the classic star shape
    double inner_r = r * cos( DEG2RAD( 72.0 ) ) / cos( DEG2RAD( 36.0 ) );

    QPolygonF pa;
    pa << QPointF( x_c + ( inner_r * sin( DEG2RAD( 324.0 ) ) ), y_c - ( inner_r * cos( DEG2RAD( 324.0 ) ) ) ) // 324
    << QPointF( x_c + ( r * sin( DEG2RAD( 288.0 ) ) ), y_c - ( r * cos( DEG2RAD( 288 ) ) ) )   // 288
    << QPointF( x_c + ( inner_r * sin( DEG2RAD( 252.0 ) ) ), y_c - ( inner_r * cos( DEG2RAD( 252.0 ) ) ) ) // 252
    << QPointF( x_c + ( r * sin( DEG2RAD( 216.0 ) ) ), y_c - ( r * cos( DEG2RAD( 216.0 ) ) ) )  // 216
    << QPointF( x_c, y_c + ( inner_r ) )         // 180
    << QPointF( x_c + ( r * sin( DEG2RAD( 144.0 ) ) ), y_c - ( r * cos( DEG2RAD( 144.0 ) ) ) )  // 144
    << QPointF( x_c + ( inner_r * sin( DEG2RAD( 108.0 ) ) ), y_c - ( inner_r * cos( DEG2RAD( 108.0 ) ) ) ) // 108
    << QPointF( x_c + ( r * sin( DEG2RAD( 72.0 ) ) ), y_c - ( r * cos( DEG2RAD( 72.0 ) ) ) )   //  72
    << QPointF( x_c + ( inner_r * sin( DEG2RAD( 36.0 ) ) ), y_c - ( inner_r * cos( DEG2RAD( 36.0 ) ) ) ) //  36
    << QPointF( x_c, y_c - r );          //   0
    thepPainter->drawPolygon( pa );
  }
  else if ( name == "arrow" )
  {
    double oneEight = r / 4;
    double quarter = r / 2;

    QPolygonF pa;
    pa << QPointF( x_c, y_c - r )
    << QPointF( x_c + quarter,  y_c - quarter )
    << QPointF( x_c + oneEight, y_c - quarter )
    << QPointF( x_c + oneEight, y_c + r )
    << QPointF( x_c - oneEight, y_c + r )
    << QPointF( x_c - oneEight, y_c - quarter )
    << QPointF( x_c - quarter,  y_c - quarter );
    thepPainter->drawPolygon( pa );
  }
  thepPainter->end();
}
