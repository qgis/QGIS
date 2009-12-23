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

#include "qgis.h"
#include "qgssymbol.h"
#include "qgslogger.h"
#include "qgssymbologyutils.h"
#include "qgsmarkercatalogue.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"

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
    mSize( DEFAULT_POINT_SIZE ),
    mSizeInMapUnits( false ),
    mPointSymbolImage( 1, 1, QImage::Format_ARGB32_Premultiplied ),
    mWidthScale( -1.0 ),
    mCacheUpToDate( false ),
    mCacheUpToDate2( false ),
    mRotationClassificationField( -1 ),
    mScaleClassificationField( -1 ),
    mSymbolField( -1 )
{
  mPen.setWidthF( DEFAULT_LINE_WIDTH );
}


QgsSymbol::QgsSymbol( QGis::GeometryType t, QString lvalue, QString uvalue, QString label, QColor c ) :
    mLowerValue( lvalue ),
    mUpperValue( uvalue ),
    mLabel( label ),
    mType( t ),
    mPen( c ),
    mBrush( c ),
    mPointSymbolName( "hard:circle" ),
    mSize( DEFAULT_POINT_SIZE ),
    mSizeInMapUnits( false ),
    mPointSymbolImage( 1, 1, QImage::Format_ARGB32_Premultiplied ),
    mWidthScale( -1.0 ),
    mCacheUpToDate( false ),
    mCacheUpToDate2( false ),
    mRotationClassificationField( -1 ),
    mScaleClassificationField( -1 ),
    mSymbolField( -1 )
{
  mPen.setWidthF( DEFAULT_LINE_WIDTH );
}

QgsSymbol::QgsSymbol()
    : mPointSymbolName( "hard:circle" ),
    mSize( DEFAULT_POINT_SIZE ),
    mSizeInMapUnits( false ),
    mPointSymbolImage( 1, 1, QImage::Format_ARGB32_Premultiplied ),
    mWidthScale( -1.0 ),
    mCacheUpToDate( false ),
    mCacheUpToDate2( false ),
    mRotationClassificationField( -1 ),
    mScaleClassificationField( -1 ),
    mSymbolField( -1 )
{
  mPen.setWidthF( DEFAULT_LINE_WIDTH );
}


QgsSymbol::QgsSymbol( QColor c )
    : mPen( c ),
    mBrush( c ),
    mPointSymbolName( "hard:circle" ),
    mSize( DEFAULT_POINT_SIZE ),
    mSizeInMapUnits( false ),
    mPointSymbolImage( 1, 1, QImage::Format_ARGB32_Premultiplied ),
    mWidthScale( -1.0 ),
    mCacheUpToDate( false ),
    mCacheUpToDate2( false ),
    mRotationClassificationField( -1 ),
    mScaleClassificationField( -1 ),
    mSymbolField( -1 )
{
  mPen.setWidthF( DEFAULT_LINE_WIDTH );
}

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
    mSize = s.mSize;
    mSizeInMapUnits = s.mSizeInMapUnits;
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
    mSymbolField = s.mSymbolField;
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
  if ( name.startsWith( "svg:" ) )
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
      //

      QStringList svgPaths = QgsApplication::svgPaths();
      for ( int i = 0; i < svgPaths.size(); i++ )
      {
        QgsDebugMsg( "SvgPath: " + svgPaths[i] );
        QFileInfo myInfo( myTempName );
        QString myFileName = myInfo.fileName(); // foo.svg
        QString myLowestDir = myInfo.dir().dirName();
        QString myLocalPath = svgPaths[i] + "/" + myLowestDir + "/" + myFileName;

        QgsDebugMsg( "Alternative svg path: " + myLocalPath );
        if ( QFile( myLocalPath ).exists() )
        {
          name = "svg:" + myLocalPath;
          QgsDebugMsg( "Svg found in alternative path" );
        }
        else if ( myInfo.isRelative() )
        {
          QFileInfo pfi( QgsProject::instance()->fileName() );
          if ( pfi.exists() && QFile( pfi.canonicalPath() + QDir::separator() + myTempName ).exists() )
          {
            name = "svg:" + pfi.canonicalPath() + QDir::separator() + myTempName;
            QgsDebugMsg( "Svg found in alternative path" );
            break;
          }
          else
          {
            QgsDebugMsg( "Svg not found in project path" );
          }
        }
        else
        {
          //couldnt find the file, no happy ending :-(
          QgsDebugMsg( "Computed alternate path but no svg there either" );
        }
      }
    }
  }
  mPointSymbolName = name;
  mCacheUpToDate = mCacheUpToDate2 = false;
}

QString QgsSymbol::pointSymbolName() const
{
  return mPointSymbolName;
}

void QgsSymbol::setPointSizeUnits( bool sizeInMapUnits )
{
  mSizeInMapUnits = sizeInMapUnits;
}

bool QgsSymbol::pointSizeUnits() const
{
  return mSizeInMapUnits;
}

void QgsSymbol::setPointSize( double s )
{
  if ( mSizeInMapUnits )
  {
    mSize = s;
  }
  else if ( s < MINIMUM_POINT_SIZE )
    mSize = MINIMUM_POINT_SIZE;
  else
    mSize = s;

  mCacheUpToDate = mCacheUpToDate2 = false;
}

double QgsSymbol::pointSize() const
{
  return mSize;
}

QImage QgsSymbol::getLineSymbolAsImage()
{
  //Note by Tim: don't use premultiplied - it causes
  //artifacts on the output icon!
  QImage img( 15, 15, QImage::Format_ARGB32 );//QImage::Format_ARGB32_Premultiplied);
  //0 = fully transparent
  img.fill( QColor( 255, 255, 255, 0 ).rgba() );
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
  //Note by Tim: don't use premultiplied - it causes
  //artifacts on the output icon!
  QImage img( 15, 15, QImage::Format_ARGB32 ); //, QImage::Format_ARGB32_Premultiplied);
  //0 = fully transparent
  img.fill( QColor( 255, 255, 255, 0 ).rgba() );
  QPainter p( &img );
  p.setRenderHints( QPainter::Antialiasing );
  p.setPen( mPen );
  p.setBrush( mBrush );
  QPolygon myPolygon;
  //leave a little white space around so
  //don't draw at 0,0,15,15
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

QImage QgsSymbol::getCachedPointSymbolAsImage( double widthScale, bool selected, QColor selectionColor, double opacity )
{
  if ( !mCacheUpToDate2
       || ( selected && mSelectionColor != selectionColor ) || ( opacity != mOpacity ) )
  {
    if ( selected )
    {
      cache2( widthScale, selectionColor, opacity );
    }
    else
    {

      cache2( widthScale, mSelectionColor, opacity );
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
    double rotation, double rasterScaleFactor, double opacity )
{
  double scaleProduct = scale * rasterScaleFactor;

  //on systems where dpi in x- and y-direction are not the same, the scaleProduct may differ from 1.0 by a very small number
  if ( scaleProduct > 0.9 && scaleProduct < 1.1 && 0 == rotation )
  {
    if ( mWidthScale < 0 || widthScale == mWidthScale )
    {
      // If scale is 1.0, rotation 0.0  use cached image.
      return getCachedPointSymbolAsImage( widthScale, selected, selectionColor, opacity );
    }
  }

  QImage preRotateImage;
  QPen pen = mPen;
  double newWidth = mPen.widthF() * widthScale * rasterScaleFactor;
  pen.setWidthF( newWidth );

  if ( selected )
  {
    pen.setColor( selectionColor );
    QBrush brush = mBrush;
    preRotateImage = QgsMarkerCatalogue::instance()->imageMarker(
                       mPointSymbolName,
                       ( float )( mSize * scale * widthScale * rasterScaleFactor ),
                       pen, mBrush, opacity );
  }
  else
  {
    QgsDebugMsg( QString( "marker:%1 mPointSize:%2 mPointSizeUnits:%3 scale:%4 widthScale:%5 rasterScaleFactor:%6 opacity:%7" )
                 .arg( mPointSymbolName ).arg( mSize ).arg( mSizeInMapUnits ? "true" : "false" )
                 .arg( scale ).arg( widthScale ).arg( rasterScaleFactor ).arg( opacity ) );


    preRotateImage = QgsMarkerCatalogue::instance()->imageMarker(
                       mPointSymbolName,
                       ( float )( mSize * scale * widthScale * rasterScaleFactor ),
                       pen, mBrush, opacity );
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
  // For symbols that have a different colored border, the line
  // below causes the fill color to be wrong for the print
  // composer. Not sure why...
  // brush.setColor ( selectionColor );

  mPointSymbolImage = QgsMarkerCatalogue::instance()->imageMarker( mPointSymbolName, mSize,
                      mPen, mBrush );

  mPointSymbolImageSelected = QgsMarkerCatalogue::instance()->imageMarker(
                                mPointSymbolName, mSize, pen, brush );

  mSelectionColor = selectionColor;
  mCacheUpToDate = true;
}

void QgsSymbol::cache2( double widthScale, QColor selectionColor, double opacity )
{
// QgsDebugMsg(QString("widthScale = %1").arg(widthScale));

  QPen pen = mPen;
  pen.setWidthF( widthScale * pen.widthF() );

  mPointSymbolImage2 = QgsMarkerCatalogue::instance()->imageMarker( mPointSymbolName, mSize * widthScale,
                       pen, mBrush, opacity );

  QBrush brush = mBrush;
  brush.setColor( selectionColor );
  pen.setColor( selectionColor );

  mPointSymbolImageSelected2 = QgsMarkerCatalogue::instance()->imageMarker(
                                 mPointSymbolName, mSize * widthScale, pen, brush, opacity );

  mSelectionColor2 = selectionColor;

  mWidthScale = widthScale;

  mOpacity = opacity;

  mCacheUpToDate2 = true;
}

void QgsSymbol::appendField( QDomElement &symbol, QDomDocument &document, const QgsVectorLayer &vl, QString name, int idx ) const
{
  appendText( symbol, document, name, vl.pendingFields().contains( idx ) ? vl.pendingFields()[idx].name() : "" );
}

void QgsSymbol::appendText( QDomElement &symbol, QDomDocument &document, QString name, QString value ) const
{
  QDomElement node = document.createElement( name );
  QDomText txt = document.createTextNode( value );
  if ( value.isNull() )
  {
    node.setAttribute( "null", "1" );
  }
  symbol.appendChild( node );
  node.appendChild( txt );
}

bool QgsSymbol::writeXML( QDomNode & item, QDomDocument & document, const QgsVectorLayer *vl ) const
{
  bool returnval = false;
  returnval = true; // no error checking yet
  QDomElement symbol = document.createElement( "symbol" );
  item.appendChild( symbol );

  appendText( symbol, document, "lowervalue", mLowerValue );
  appendText( symbol, document, "uppervalue", mUpperValue );
  appendText( symbol, document, "label", mLabel );

  QString name = pointSymbolName();
  if ( name.startsWith( "svg:" ) )
  {
    name = name.mid( 4 );

    QFileInfo fi( name );
    if ( fi.exists() )
    {
      name = fi.canonicalFilePath();

      QStringList svgPaths = QgsApplication::svgPaths();

      for ( int i = 0; i < svgPaths.size(); i++ )
      {
        QString dir = QFileInfo( svgPaths[i] ).canonicalFilePath();

        if ( !dir.isEmpty() && name.startsWith( dir ) )
        {
          name = name.mid( dir.size() );
          break;
        }
      }
    }

    name = "svg:" + name;
  }

  appendText( symbol, document, "pointsymbol", name );
  appendText( symbol, document, "pointsize", QString::number( pointSize() ) );
  appendText( symbol, document, "pointsizeunits", pointSizeUnits() ? "mapunits" : "pixels" );

  if ( vl )
  {
    appendField( symbol, document, *vl, "rotationclassificationfieldname", mRotationClassificationField );
    appendField( symbol, document, *vl, "scaleclassificationfieldname", mScaleClassificationField );
    appendField( symbol, document, *vl, "symbolfieldname", mSymbolField );
  }

  QDomElement outlinecolor = document.createElement( "outlinecolor" );
  outlinecolor.setAttribute( "red", QString::number( mPen.color().red() ) );
  outlinecolor.setAttribute( "green", QString::number( mPen.color().green() ) );
  outlinecolor.setAttribute( "blue", QString::number( mPen.color().blue() ) );
  symbol.appendChild( outlinecolor );

  appendText( symbol, document, "outlinestyle", QgsSymbologyUtils::penStyle2QString( mPen.style() ) );
  appendText( symbol, document, "outlinewidth", QString::number( mPen.widthF() ) );

  QDomElement fillcolor = document.createElement( "fillcolor" );
  fillcolor.setAttribute( "red", QString::number( mBrush.color().red() ) );
  fillcolor.setAttribute( "green", QString::number( mBrush.color().green() ) );
  fillcolor.setAttribute( "blue", QString::number( mBrush.color().blue() ) );
  symbol.appendChild( fillcolor );

  appendText( symbol, document, "fillpattern", QgsSymbologyUtils::brushStyle2QString( mBrush.style() ) );
  appendText( symbol, document, "texturepath", QgsProject::instance()->writePath( mTextureFilePath ) );

  return returnval;
}

int QgsSymbol::readFieldName( QDomNode &synode, QString name, const QgsVectorLayer &vl )
{
  QDomNode node = synode.namedItem( name + "name" );

  if ( !node.isNull() )
  {
    const QgsFieldMap &fields = vl.pendingFields();
    QString name = node.toElement().text();

    for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); it++ )
      if ( it->name() == name )
        return it.key();

    return -1;
  }

  node = synode.namedItem( name );

  return node.isNull() ? -1 : node.toElement().text().toInt();
}

bool QgsSymbol::readXML( QDomNode &synode, const QgsVectorLayer *vl )
{
  // Legacy project file formats didn't have support for pointsymbol nor
  // pointsize Dom elements.  Therefore we should check whether these
  // actually exist.

  QDomNode lvalnode = synode.namedItem( "lowervalue" );
  if ( ! lvalnode.isNull() )
  {
    QDomElement lvalelement = lvalnode.toElement();
    if ( lvalelement.attribute( "null" ).toInt() == 1 )
    {
      mLowerValue = QString::null;
    }
    else
    {
      mLowerValue = lvalelement.text();
    }
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

  if ( ! psizenode.isNull() )
  {
    QDomElement psizeelement = psizenode.toElement();
    setPointSize( psizeelement.text().toFloat() );
  }

  QDomNode psizeunitnodes = synode.namedItem( "pointsizeunits" );
  if ( ! psizeunitnodes.isNull() )
  {
    QDomElement psizeunitelement = psizeunitnodes.toElement();
    QgsDebugMsg( QString( "psizeunitelement:%1" ).arg( psizeunitelement.text() ) );
    setPointSizeUnits( psizeunitelement.text().compare( "mapunits", Qt::CaseInsensitive ) == 0 );
  }

  if ( vl )
  {
    mRotationClassificationField = readFieldName( synode, "rotationclassificationfield", *vl );
    mScaleClassificationField = readFieldName( synode, "scaleclassificationfield", *vl );
    mSymbolField = readFieldName( synode, "symbolfield", *vl );
  }
  else
  {
    mRotationClassificationField = -1;
    mScaleClassificationField = -1;
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
  setCustomTexture( QgsProject::instance()->readPath( texturepathelement.text() ) );

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

int QgsSymbol::symbolField() const
{
  return mSymbolField;
}

void QgsSymbol::setSymbolField( int field )
{
  mSymbolField = field;
}
