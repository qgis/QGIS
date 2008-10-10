/***************************************************************************
                          QgsSymbol.cpp  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include <cmath>

#include "qgssymbol.h"
#include "qgslogger.h"
#include "qgssymbologyutils.h"
#include "qgsmarkercatalogue.h"
#include "qgsapplication.h"

#include <QPainter>
#include <QDomNode>
#include <QDomDocument>
#include <QImage>
#include <QDir>
#include <QFileInfo>
//#include <QString>
#include "qgslogger.h"
//do we have to include qstring?

QgsSymbol::QgsSymbol( QGis::GeometryType t, QString lvalue, QString uvalue, QString label ) :
    mLowerValue( lvalue ),
    mUpperValue( uvalue ),
    mLabel( label ),
    mType( t ),
    mPointSymbolName( "hard:circle" ),
    mPointSize( DEFAULT_POINT_SIZE ),
    mPointSymbolImage( 1, 1, QImage::Format_ARGB32_Premultiplied ),
    mWidthScale( 1.0 ),
    mCacheUpToDate( false ),
    mCacheUpToDate2( false ),
    mRotationClassificationField( -1 ),
    mScaleClassificationField( -1 )
{}


QgsSymbol::QgsSymbol( QGis::GeometryType t, QString lvalue, QString uvalue, QString label, QColor c ) :
    mLowerValue( lvalue ),
    mUpperValue( uvalue ),
    mLabel( label ),
    mType( t ),
    mPen( c ),
    mBrush( c ),
    mPointSymbolName( "hard:circle" ),
    mPointSize( DEFAULT_POINT_SIZE ),
    mPointSymbolImage( 1, 1, QImage::Format_ARGB32_Premultiplied ),
    mWidthScale( 1.0 ),
    mCacheUpToDate( false ),
    mCacheUpToDate2( false ),
    mRotationClassificationField( -1 ),
    mScaleClassificationField( -1 )
{}

QgsSymbol::QgsSymbol()
    : mPointSymbolName( "hard:circle" ),
    mPointSize( DEFAULT_POINT_SIZE ),
    mPointSymbolImage( 1, 1, QImage::Format_ARGB32_Premultiplied ),
    mWidthScale( 1.0 ),
    mCacheUpToDate( false ),
    mCacheUpToDate2( false ),
    mRotationClassificationField( -1 ),
    mScaleClassificationField( -1 )
{}


QgsSymbol::QgsSymbol( QColor c )
    : mPen( c ),
    mBrush( c ),
    mPointSymbolName( "hard:circle" ),
    mPointSize( DEFAULT_POINT_SIZE ),
    mPointSymbolImage( 1, 1, QImage::Format_ARGB32_Premultiplied ),
    mWidthScale( 1.0 ),
    mCacheUpToDate( false ),
    mCacheUpToDate2( false ),
    mRotationClassificationField( -1 ),
    mScaleClassificationField( -1 )
{}

QgsSymbol::QgsSymbol( const QgsSymbol& s )
{
  if ( this != &s )
  {
    mLowerValue = s.mLowerValue;
    mUpperValue = s.mUpperValue;
    mLabel = s.mLabel;
    mType = s.mType;
    mPen = s.mPen;
    mBrush = s.mBrush;
    mTextureFilePath = s.mTextureFilePath;
    mPointSymbolName = s.mPointSymbolName;
    mPointSize = s.mPointSize;
    mPointSymbolImage = s.mPointSymbolImage;
    mPointSymbolImageSelected = s.mPointSymbolImageSelected;
    mWidthScale = s.mWidthScale;
    mPointSymbolImage2 = s.mPointSymbolImage2;
    mPointSymbolImageSelected2 = s.mPointSymbolImageSelected2;
    mCacheUpToDate = s.mCacheUpToDate;
    mCacheUpToDate2 = s.mCacheUpToDate2;
    mSelectionColor = s.mSelectionColor;
    mSelectionColor2 = s.mSelectionColor2;
    mRotationClassificationField = s.mRotationClassificationField;
    mScaleClassificationField = s.mScaleClassificationField;
  }
}

QgsSymbol::~QgsSymbol()
{

}


QColor QgsSymbol::color() const
{
  return mPen.color();
}

void QgsSymbol::setColor( QColor c )
{
  mPen.setColor( c );
  mCacheUpToDate = mCacheUpToDate2 = false;
}

QColor QgsSymbol::fillColor() const
{
  return mBrush.color();
}

void QgsSymbol::setFillColor( QColor c )
{
  mBrush.setColor( c );
  mCacheUpToDate = mCacheUpToDate2 = false;
}

double QgsSymbol::lineWidth() const
{
  return mPen.widthF();
}

void QgsSymbol::setLineWidth( double w )
{
  mPen.setWidthF( w );
  mCacheUpToDate = mCacheUpToDate2 = false;
}

void QgsSymbol::setLineStyle( Qt::PenStyle s )
{
  mPen.setStyle( s );
  mCacheUpToDate = mCacheUpToDate2 = false;
}

void QgsSymbol::setFillStyle( Qt::BrushStyle s )
{
  mBrush.setStyle( s );
  mCacheUpToDate = mCacheUpToDate2 = false;
}

QString QgsSymbol::customTexture() const
{
  return mTextureFilePath;
}

void QgsSymbol::setCustomTexture( QString path )
{
  mTextureFilePath = path;
  mBrush.setTextureImage( QImage( path ) );
  mCacheUpToDate = mCacheUpToDate2 = false;
}

//should we set the path independently of setting the texture?

void QgsSymbol::setNamedPointSymbol( QString name )
{
  // do some sanity checking for svgs...
  QString myTempName = name;
  myTempName.replace( "svg:", "" );
  QFile myFile( myTempName );
  if ( !myFile.exists() )
  {
    QgsDebugMsg( "\n\n\n *** Svg Symbol not found on fs ***" );
    QgsDebugMsg( "Name: " + name );
    //see if we can resolve the problem...
    //by using the qgis svg dir from this local machine
    //one day when user specified svg are allowed we need
    //to adjust this logic probably...
    QString svgPath = QgsApplication::svgPath();
    QgsDebugMsg( "SvgPath: " + svgPath );
    QFileInfo myInfo( myTempName );
    QString myFileName = myInfo.fileName(); // foo.svg
    QString myLowestDir = myInfo.dir().dirName();
    QString myLocalPath = svgPath + QDir::separator() +
                          myLowestDir + QDir::separator() +
                          myFileName;
    QgsDebugMsg( "Alternative svg path: " + myLocalPath );
    if ( QFile( myLocalPath ).exists() )
    {
      name = "svg:" + myLocalPath;
      QgsDebugMsg( "Svg found in alternative path" );
    }
    else
    {
      //couldnt find the file, no happy ending :-(
      QgsDebugMsg( "Computed alternate path but no svg there either" );
    }
  }
  mPointSymbolName = name;
  mCacheUpToDate = mCacheUpToDate2 = false;
}

QString QgsSymbol::pointSymbolName() const
{
  return mPointSymbolName;
}

void QgsSymbol::setPointSize( double s )
{
  if ( s < MINIMUM_POINT_SIZE )
    mPointSize = MINIMUM_POINT_SIZE;
  else
    mPointSize = s;

  mCacheUpToDate = mCacheUpToDate2 = false;
}

double QgsSymbol::pointSize() const
{
  return mPointSize;
}


QImage QgsSymbol::getLineSymbolAsImage()
{
  //Note by Tim: dont use premultiplied - it causes
  //artifacts on the output icon!
  QImage img( 15, 15, QImage::Format_ARGB32 );//QImage::Format_ARGB32_Premultiplied);
  img.fill( QColor( 255, 255, 255, 255 ).rgba() );
  QPainter p( &img );
  p.setRenderHints( QPainter::Antialiasing );
  p.setPen( mPen );


  QPainterPath myPath;
  myPath.moveTo( 0, 0 );
  myPath.cubicTo( 15, 0, 5, 7, 15, 15 );
  p.drawPath( myPath );
  //p.drawLine(0, 0, 15, 15);
  return img; //this is ok because of qts sharing mechanism
}

QImage QgsSymbol::getPolygonSymbolAsImage()
{
  //Note by Tim: dont use premultiplied - it causes
  //artifacts on the output icon!
  QImage img( 15, 15, QImage::Format_ARGB32 ); //, QImage::Format_ARGB32_Premultiplied);
  img.fill( QColor( 255, 255, 255, 255 ).rgba() );
  QPainter p( &img );
  p.setRenderHints( QPainter::Antialiasing );
  p.setPen( mPen );
  p.setBrush( mBrush );
  QPolygon myPolygon;
  //leave a little white space around so
  //dont draw at 0,0,15,15
  myPolygon << QPoint( 2, 2 )
  << QPoint( 1, 5 )
  << QPoint( 1, 10 )
  << QPoint( 2, 12 )
  << QPoint( 5, 13 )
  << QPoint( 6, 13 )
  << QPoint( 8, 12 )
  << QPoint( 8, 12 )
  << QPoint( 10, 12 )
  << QPoint( 12, 13 )
  << QPoint( 13, 11 )
  << QPoint( 12, 8 )
  << QPoint( 11, 6 )
  << QPoint( 12, 5 )
  << QPoint( 13, 2 )
  << QPoint( 11, 1 )
  << QPoint( 10, 1 )
  << QPoint( 8, 2 )
  << QPoint( 6, 4 )
  << QPoint( 4, 2 )
  ;
  p.drawPolygon( myPolygon );
  //p.drawRect(1, 1, 14, 14);
  return img; //this is ok because of qts sharing mechanism
}

QImage QgsSymbol::getCachedPointSymbolAsImage( double widthScale,
    bool selected, QColor selectionColor )
{
  if ( !mCacheUpToDate2
       || ( selected && mSelectionColor != selectionColor ) )
  {
    if ( selected )
    {
      cache2( widthScale, selectionColor );
    }
    else
    {
      cache2( widthScale, mSelectionColor );
    }
  }

  if ( selected )
  {
    return mPointSymbolImageSelected2;
  }
  else
  {
    return mPointSymbolImage2;
  }
}

QImage QgsSymbol::getPointSymbolAsImage( double widthScale, bool selected, QColor selectionColor, double scale,
    double rotation, double rasterScaleFactor )
{
  //QgsDebugMsg(QString("Symbol scale = %1, and rotation = %2").arg(scale).arg(rotation));
  if ( 1.0 == ( scale * rasterScaleFactor ) && 0 == rotation )
  {
    // If scale is 1.0 and rotation 0.0, use cached image.
    return getCachedPointSymbolAsImage( widthScale, selected, selectionColor );
  }

  QImage preRotateImage;
  QPen pen = mPen;
  double newWidth = mPen.widthF() * widthScale * rasterScaleFactor;
  pen.setWidth( newWidth );

  if ( selected )
  {
    pen.setColor( selectionColor );
    QBrush brush = mBrush;
    preRotateImage = QgsMarkerCatalogue::instance()->imageMarker( mPointSymbolName, ( float )( mPointSize * scale * widthScale * rasterScaleFactor ),
                     pen, mBrush );
  }
  else
  {
    preRotateImage = QgsMarkerCatalogue::instance()->imageMarker( mPointSymbolName, ( float )( mPointSize * scale * widthScale * rasterScaleFactor ),
                     pen, mBrush );
  }

  QMatrix rotationMatrix;
  rotationMatrix = rotationMatrix.rotate( rotation );

  return preRotateImage.transformed( rotationMatrix, Qt::SmoothTransformation );
}


void QgsSymbol::cache( QColor selectionColor )
{
  QPen pen = mPen;
  pen.setColor( selectionColor );
  QBrush brush = mBrush;
  // For symbols that have a different coloured border, the line
  // below causes the fill colour to be wrong for the print
  // composer. Not sure why...
  // brush.setColor ( selectionColor );

  mPointSymbolImage = QgsMarkerCatalogue::instance()->imageMarker( mPointSymbolName, mPointSize,
                      mPen, mBrush );

  mPointSymbolImageSelected = QgsMarkerCatalogue::instance()->imageMarker(
                                mPointSymbolName, mPointSize, pen, brush );

  mSelectionColor = selectionColor;
  mCacheUpToDate = true;
}

void QgsSymbol::cache2( double widthScale, QColor selectionColor )
{
// QgsDebugMsg(QString("widthScale = %1").arg(widthScale));

  QPen pen = mPen;
  pen.setWidthF( widthScale * pen.widthF() );

  mPointSymbolImage2 = QgsMarkerCatalogue::instance()->imageMarker( mPointSymbolName, mPointSize * widthScale,
                       pen, mBrush, false );

  QBrush brush = mBrush;
  brush.setColor( selectionColor );
  pen.setColor( selectionColor );

  mPointSymbolImageSelected2 = QgsMarkerCatalogue::instance()->imageMarker(
                                 mPointSymbolName, mPointSize * widthScale, pen, brush,  false );

  mSelectionColor2 = selectionColor;

  mWidthScale = widthScale;
  mCacheUpToDate2 = true;
}

bool QgsSymbol::writeXML( QDomNode & item, QDomDocument & document ) const
{
  bool returnval = false;
  returnval = true; // no error checking yet
  QDomElement symbol = document.createElement( "symbol" );
  item.appendChild( symbol );

  QDomElement lowervalue = document.createElement( "lowervalue" );
  QDomText lowervaluetxt = document.createTextNode( mLowerValue );
  symbol.appendChild( lowervalue );
  lowervalue.appendChild( lowervaluetxt );

  QDomElement uppervalue = document.createElement( "uppervalue" );
  QDomText uppervaluetxt = document.createTextNode( mUpperValue );
  symbol.appendChild( uppervalue );
  uppervalue.appendChild( uppervaluetxt );

  QDomElement label = document.createElement( "label" );
  QDomText labeltxt = document.createTextNode( mLabel );
  symbol.appendChild( label );
  label.appendChild( labeltxt );

  QDomElement pointsymbol = document.createElement( "pointsymbol" );
  QDomText pointsymboltxt = document.createTextNode( pointSymbolName() );
  symbol.appendChild( pointsymbol );
  pointsymbol.appendChild( pointsymboltxt );

  QDomElement pointsize = document.createElement( "pointsize" );
  QDomText pointsizetxt = document.createTextNode( QString::number( pointSize() ) );
  symbol.appendChild( pointsize );
  pointsize.appendChild( pointsizetxt );

  QDomElement rotationclassificationfield = document.createElement( "rotationclassificationfield" );
  QDomText rotationclassificationfieldtxt = document.createTextNode( QString::number( mRotationClassificationField ) );
  rotationclassificationfield.appendChild( rotationclassificationfieldtxt );
  symbol.appendChild( rotationclassificationfield );

  QDomElement scaleclassificationfield = document.createElement( "scaleclassificationfield" );
  QDomText scaleclassificationfieldtxt = document.createTextNode( QString::number( mScaleClassificationField ) );
  scaleclassificationfield.appendChild( scaleclassificationfieldtxt );
  symbol.appendChild( scaleclassificationfield );


  QDomElement outlinecolor = document.createElement( "outlinecolor" );
  outlinecolor.setAttribute( "red", QString::number( mPen.color().red() ) );
  outlinecolor.setAttribute( "green", QString::number( mPen.color().green() ) );
  outlinecolor.setAttribute( "blue", QString::number( mPen.color().blue() ) );
  symbol.appendChild( outlinecolor );
  QDomElement outlinestyle = document.createElement( "outlinestyle" );
  QDomText outlinestyletxt = document.createTextNode( QgsSymbologyUtils::penStyle2QString( mPen.style() ) );
  outlinestyle.appendChild( outlinestyletxt );
  symbol.appendChild( outlinestyle );
  QDomElement outlinewidth = document.createElement( "outlinewidth" );
  QDomText outlinewidthtxt = document.createTextNode( QString::number( mPen.widthF() ) );
  outlinewidth.appendChild( outlinewidthtxt );
  symbol.appendChild( outlinewidth );
  QDomElement fillcolor = document.createElement( "fillcolor" );
  fillcolor.setAttribute( "red", QString::number( mBrush.color().red() ) );
  fillcolor.setAttribute( "green", QString::number( mBrush.color().green() ) );
  fillcolor.setAttribute( "blue", QString::number( mBrush.color().blue() ) );
  symbol.appendChild( fillcolor );
  QDomElement fillpattern = document.createElement( "fillpattern" );
  QDomText fillpatterntxt = document.createTextNode( QgsSymbologyUtils::brushStyle2QString( mBrush.style() ) );
  fillpattern.appendChild( fillpatterntxt );
  symbol.appendChild( fillpattern );
  fillpattern.appendChild( fillpatterntxt );

  QDomElement texturepath = document.createElement( "texturepath" );
  QDomText texturepathtxt = document.createTextNode( mTextureFilePath );
  symbol.appendChild( texturepath );
  texturepath.appendChild( texturepathtxt );

  return returnval;
}

bool QgsSymbol::readXML( QDomNode & synode )
{
  // Legacy project file formats didn't have support for pointsymbol nor
  // pointsize Dom elements.  Therefore we should check whether these
  // actually exist.

  QDomNode lvalnode = synode.namedItem( "lowervalue" );
  if ( ! lvalnode.isNull() )
  {
    QDomElement lvalelement = lvalnode.toElement();
    mLowerValue = lvalelement.text();
  }

  QDomNode uvalnode = synode.namedItem( "uppervalue" );
  if ( ! uvalnode.isNull() )
  {
    QDomElement uvalelement = uvalnode.toElement();
    mUpperValue = uvalelement.text();
  }

  QDomNode labelnode = synode.namedItem( "label" );
  if ( ! labelnode.isNull() )
  {
    QDomElement labelelement = labelnode.toElement();
    mLabel = labelelement.text();
  }

  QDomNode psymbnode = synode.namedItem( "pointsymbol" );

  if ( ! psymbnode.isNull() )
  {
    QDomElement psymbelement = psymbnode.toElement();
    setNamedPointSymbol( psymbelement.text() );
  }

  QDomNode psizenode = synode.namedItem( "pointsize" );

  if ( !  psizenode.isNull() )
  {
    QDomElement psizeelement = psizenode.toElement();
    setPointSize( psizeelement.text().toFloat() );
  }

  mRotationClassificationField = -1;
  mScaleClassificationField = -1;

  QDomNode classnode = synode.namedItem( "rotationclassificationfield" );
  if ( !classnode.isNull() )
  {
    mRotationClassificationField = classnode.toElement().text().toInt();
    QgsDebugMsg( "Found rotationfield: " + QString::number( rotationClassificationField() ) );
  }

  classnode = synode.namedItem( "scaleclassificationfield" );
  if ( !classnode.isNull() )
  {
    mScaleClassificationField = classnode.toElement().text().toInt();
    QgsDebugMsg( "Found scalefield: " + QString::number( scaleClassificationField() ) );
  }


  QDomNode outlcnode = synode.namedItem( "outlinecolor" );
  QDomElement oulcelement = outlcnode.toElement();
  int red = oulcelement.attribute( "red" ).toInt();
  int green = oulcelement.attribute( "green" ).toInt();
  int blue = oulcelement.attribute( "blue" ).toInt();
  setColor( QColor( red, green, blue ) );

  QDomNode outlstnode = synode.namedItem( "outlinestyle" );
  QDomElement outlstelement = outlstnode.toElement();
  setLineStyle( QgsSymbologyUtils::qString2PenStyle( outlstelement.text() ) );

  QDomNode outlwnode = synode.namedItem( "outlinewidth" );
  QDomElement outlwelement = outlwnode.toElement();
  setLineWidth( outlwelement.text().toDouble() );

  QDomNode fillcnode = synode.namedItem( "fillcolor" );
  QDomElement fillcelement = fillcnode.toElement();
  red = fillcelement.attribute( "red" ).toInt();
  green = fillcelement.attribute( "green" ).toInt();
  blue = fillcelement.attribute( "blue" ).toInt();
  setFillColor( QColor( red, green, blue ) );

  QDomNode texturepathnode = synode.namedItem( "texturepath" );
  QDomElement texturepathelement = texturepathnode.toElement();
  setCustomTexture( texturepathelement.text() );

  //run this after setting the custom texture path, so we override the brush if it isn't the custom pattern brush.
  QDomNode fillpnode = synode.namedItem( "fillpattern" );
  QDomElement fillpelement = fillpnode.toElement();
  setFillStyle( QgsSymbologyUtils::qString2BrushStyle( fillpelement.text() ) );

  return true;
}

int QgsSymbol::rotationClassificationField() const
{
  return mRotationClassificationField;
}

void QgsSymbol::setRotationClassificationField( int field )
{
  mRotationClassificationField = field;
}

int QgsSymbol::scaleClassificationField() const
{
  return mScaleClassificationField;
}

void QgsSymbol::setScaleClassificationField( int field )
{
  mScaleClassificationField = field;
}

