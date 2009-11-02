/***************************************************************************
                           qgscomposerpicture.cpp
                             -------------------
    begin                : September 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
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

#include "qgscomposerpicture.h"
#include "qgscomposermap.h"
#include "qgsproject.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QImageReader>
#include <QPainter>
#include <QSvgRenderer>

#ifndef Q_OS_MACX
#include <cmath>
#else
#include <math.h>
#endif

QgsComposerPicture::QgsComposerPicture( QgsComposition *composition ): QObject( 0 ), QgsComposerItem( composition ), mRotation( 0.0 ), mMode( Unknown ), \
    mSvgCacheUpToDate( false ), mCachedDpi( 0 ), mRotationMap( 0 )
{
}

QgsComposerPicture::QgsComposerPicture(): QgsComposerItem( 0 ), mRotation( 0.0 ), mMode( Unknown ), mSvgCacheUpToDate( false ), mRotationMap( 0 )
{

}

QgsComposerPicture::~QgsComposerPicture()
{

}

void QgsComposerPicture::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( !painter )
  {
    return;
  }

  drawBackground( painter );

  int newDpi = ( painter->device()->logicalDpiX() + painter->device()->logicalDpiY() ) / 2;
  if ( mMode != Unknown )
  {
    double rectPixelWidth = rect().width() * newDpi / 25.4;
    double rectPixelHeight = rect().height() * newDpi / 25.4;
    QRectF boundRect;
    if ( mMode == SVG )
    {
      boundRect = boundedSVGRect( rectPixelWidth, rectPixelHeight );
    }
    else if ( mMode == RASTER )
    {
      boundRect = boundedImageRect( rectPixelWidth, rectPixelHeight );
    }

    double boundRectWidthMM = boundRect.width() / newDpi * 25.4;
    double boundRectHeightMM = boundRect.height() / newDpi * 25.4;
    double unrotatedBoundImageWidth = boundRect.width();
    double unrotatedBoundImageHeight = boundRect.height();
    double unrotatedBoundImageWidthMM = unrotatedBoundImageWidth / newDpi * 25.4;
    double unrotatedBoundImageHeightMM = unrotatedBoundImageHeight / newDpi * 25.4;
    double rotatedBoundImageWidth = boundRect.width();
    double rotatedBoundImageHeight = boundRect.height();
    imageSizeConsideringRotation( rotatedBoundImageWidth, rotatedBoundImageHeight );
    double rotatedBoundImageWidthMM = rotatedBoundImageWidth / newDpi * 25.4;
    double rotatedBoundImageHeightMM = rotatedBoundImageHeight / newDpi * 25.4;

    if ( mMode == SVG )
    {
      if ( !mSvgCacheUpToDate )
      {
        mImage = QImage( rotatedBoundImageWidth, rotatedBoundImageHeight, QImage::Format_ARGB32 );
        updateImageFromSvg();
      }
    }

    painter->save();
    painter->translate( boundRectWidthMM / 2.0, boundRectHeightMM / 2.0 );
    painter->rotate( mRotation );
    painter->translate( -rotatedBoundImageWidthMM / 2.0, -rotatedBoundImageHeightMM / 2.0 );
    painter->drawImage( QRectF( 0, 0, rotatedBoundImageWidthMM,  rotatedBoundImageHeightMM ), mImage, QRectF( 0, 0, mImage.width(), mImage.height() ) );

    painter->restore();
  }

  mCachedDpi = newDpi;

  //frame and selection boxes
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerPicture::setPictureFile( const QString& path )
{
  mSourceFile.setFileName( path );
  if ( !mSourceFile.exists() )
  {
    mMode = Unknown;
  }

  QFileInfo sourceFileInfo( mSourceFile );
  QString sourceFileSuffix = sourceFileInfo.suffix();
  if ( sourceFileSuffix.compare( "svg", Qt::CaseInsensitive ) == 0 )
  {
    //try to open svg
    QSvgRenderer validTestRenderer( mSourceFile.fileName() );
    if ( validTestRenderer.isValid() )
    {
      mMode = SVG;
      QRect viewBox = validTestRenderer.viewBox(); //take width/height ratio from view box instead of default size
      mDefaultSvgSize.setWidth( viewBox.width() );
      mDefaultSvgSize.setHeight( viewBox.height() );
      mSvgCacheUpToDate = false;
    }
    else
    {
      mMode = Unknown;
    }
  }
  else
  {
    //try to open raster with QImageReader
    QImageReader imageReader( mSourceFile.fileName() );
    if ( imageReader.read( &mImage ) )
    {
      mMode = RASTER;
    }
    else
    {
      mMode = Unknown;
    }
  }

  if ( mMode != Unknown ) //make sure we start with a new QImage
  {
    setSceneRect( QRectF( transform().dx(), transform().dy(), rect().width(), rect().height() ) );
  }
  emit settingsChanged();
}

QRectF QgsComposerPicture::boundedImageRect( double deviceWidth, double deviceHeight )
{
  double imageToDeviceRatio;
  if ( mImage.width() / deviceWidth > mImage.height() / deviceHeight )
  {
    imageToDeviceRatio =  deviceWidth / mImage.width();
    double height = imageToDeviceRatio * mImage.height();
    return QRectF( 0, 0, deviceWidth, height );
  }
  else
  {
    imageToDeviceRatio = deviceHeight / mImage.height();
    double width = imageToDeviceRatio * mImage.width();
    return QRectF( 0, 0, width, deviceHeight );
  }
}

QRectF QgsComposerPicture::boundedSVGRect( double deviceWidth, double deviceHeight )
{
  double imageToSvgRatio;
  if ( deviceWidth / mDefaultSvgSize.width() < deviceHeight / mDefaultSvgSize.height() )
  {
    imageToSvgRatio = deviceWidth / mDefaultSvgSize.width();
    double height = mDefaultSvgSize.height() * imageToSvgRatio;
    return QRectF( 0, 0, deviceWidth, height );
  }
  else
  {
    imageToSvgRatio = deviceHeight / mDefaultSvgSize.height();
    double width = mDefaultSvgSize.width() * imageToSvgRatio;
    return QRectF( 0, 0, width, deviceHeight );
  }
}

void QgsComposerPicture::updateImageFromSvg()
{
  mImage.fill( 0 );
  QPainter p( &mImage );
  p.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing, true );
  QSvgRenderer theRenderer( mSourceFile.fileName() );
  theRenderer.render( &p );
  mSvgCacheUpToDate = true;
}

bool QgsComposerPicture::imageSizeConsideringRotation( double& width, double& height ) const
{
  double x1 = 0;
  double y1 = 0;
  double x2 = width;
  double y2 = 0;
  double x3 = width;
  double y3 = height;
  double x4 = 0;
  double y4 = height;

  if ( !cornerPointOnRotatedAndScaledRect( x1, y1, width, height ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x2, y2, width, height ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x3, y3, width, height ) )
  {
    return false;
  }
  /*
  if(!cornerPointOnRotatedAndScaledRect(x4, y4, width, height))
  {
    return false;
  }*/

  width = sqrt(( x2 - x1 ) * ( x2 - x1 ) + ( y2 - y1 ) * ( y2 - y1 ) );
  height = sqrt(( x3 - x2 ) * ( x3 - x2 ) + ( y3 - y2 ) * ( y3 - y2 ) );
  return true;
}

bool QgsComposerPicture::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const
{
  //first rotate point clockwise
  double rotToRad = mRotation * M_PI / 180.0;
  QPointF midpoint( width / 2.0, height / 2.0 );
  double xVector = x - midpoint.x();
  double yVector = y - midpoint.y();
  //double xRotated = cos(rotToRad) * xVector + sin(rotToRad) * yVector;
  //double yRotated = -sin(rotToRad) * xVector + cos(rotToRad) * yVector;
  double xRotated = cos( rotToRad ) * xVector - sin( rotToRad ) * yVector;
  double yRotated = sin( rotToRad ) * xVector + cos( rotToRad ) * yVector;

  //create line from midpoint to rotated point
  QLineF line( midpoint.x(), midpoint.y(), midpoint.x() + xRotated, midpoint.y() + yRotated );

  //intersect with all four borders and return result
  QList<QLineF> borders;
  borders << QLineF( 0, 0, width, 0 );
  borders << QLineF( width, 0, width, height );
  borders << QLineF( width, height, 0, height );
  borders << QLineF( 0, height, 0, 0 );

  QList<QLineF>::const_iterator it = borders.constBegin();
  QPointF intersectionPoint;

  for ( ; it != borders.constEnd(); ++it )
  {
    if ( line.intersect( *it, &intersectionPoint ) == QLineF::BoundedIntersection )
    {
      x = intersectionPoint.x();
      y = intersectionPoint.y();
      return true;
    }
  }
  return false;
}

void QgsComposerPicture::setSceneRect( const QRectF& rectangle )
{
  mSvgCacheUpToDate = false;
  QgsComposerItem::setSceneRect( rectangle );
  emit settingsChanged();
}

void QgsComposerPicture::setRotation( double rotation )
{
  if ( rotation > 360 )
  {
    mRotation = (( int )rotation ) % 360;
  }
  else
  {
    mRotation = rotation;
  }
  emit settingsChanged();
  update();
}

void QgsComposerPicture::setRotationMap( int composerMapId )
{
  if ( !mComposition )
  {
    return;
  }

  if ( composerMapId == -1 ) //disable rotation from map
  {
    QObject::disconnect( mRotationMap, SIGNAL( rotationChanged( double ) ), this, SLOT( setRotation( double ) ) );
    mRotationMap = 0;
  }

  const QgsComposerMap* map = mComposition->getComposerMapById( composerMapId );
  if ( !map )
  {
    return;
  }
  if ( mRotationMap )
  {
    QObject::disconnect( mRotationMap, SIGNAL( rotationChanged( double ) ), this, SLOT( setRotation( double ) ) );
  }
  mRotation = map->rotation();
  QObject::connect( map, SIGNAL( rotationChanged( double ) ), this, SLOT( setRotation( double ) ) );
  mRotationMap = map;
}

QString QgsComposerPicture::pictureFile() const
{
  return mSourceFile.fileName();
}

bool QgsComposerPicture::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }
  QDomElement composerPictureElem = doc.createElement( "ComposerPicture" );
  composerPictureElem.setAttribute( "file", QgsProject::instance()->writePath( mSourceFile.fileName() ) );
  composerPictureElem.setAttribute( "rotation", QString::number( mRotation ) );
  if ( !mRotationMap )
  {
    composerPictureElem.setAttribute( "mapId", -1 );
  }
  else
  {
    composerPictureElem.setAttribute( "mapId", mRotationMap->id() );
  }

  _writeXML( composerPictureElem, doc );
  elem.appendChild( composerPictureElem );
  return true;
}

bool QgsComposerPicture::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    _readXML( composerItemList.at( 0 ).toElement(), doc );
  }


  mSvgCacheUpToDate = false;
  mDefaultSvgSize = QSize( 0, 0 );
  mCachedDpi = 0;

  QString fileName = QgsProject::instance()->readPath( itemElem.attribute( "file" ) );
  setPictureFile( fileName );

  mRotation = itemElem.attribute( "rotation" ).toDouble();

  //rotation map
  int rotationMapId = itemElem.attribute( "mapId", "-1" ).toInt();
  if ( rotationMapId == -1 )
  {
    mRotationMap = 0;
  }
  else if ( mComposition )
  {

    if ( mRotationMap )
    {
      QObject::disconnect( mRotationMap, SIGNAL( rotationChanged( double ) ), this, SLOT( setRotation( double ) ) );
    }
    mRotationMap = mComposition->getComposerMapById( rotationMapId );
    QObject::connect( mRotationMap, SIGNAL( rotationChanged( double ) ), this, SLOT( setRotation( double ) ) );
  }

  return true;
}

int QgsComposerPicture::rotationMap() const
{
  if ( !mRotationMap )
  {
    return -1;
  }
  else
  {
    return mRotationMap->id();
  }
}
