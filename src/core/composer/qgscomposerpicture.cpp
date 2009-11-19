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


QgsComposerPicture::QgsComposerPicture( QgsComposition *composition ): QgsComposerItem( composition ), mMode( Unknown ), \
    mSvgCacheUpToDate( false ), mCachedDpi( 0 ), mCachedRotation( 0 ), mCachedViewScaleFactor( -1 ), mRotationMap( 0 )
{
}

QgsComposerPicture::QgsComposerPicture(): QgsComposerItem( 0 ), mMode( Unknown ), mSvgCacheUpToDate( false ), mCachedRotation( 0 ), mCachedViewScaleFactor( -1 ), mRotationMap( 0 )
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
  double viewScaleFactor = horizontalViewScaleFactor();

  if ( newDpi != mCachedDpi || mCachedRotation != mRotation || mCachedViewScaleFactor != viewScaleFactor )
  {
    mSvgCacheUpToDate = false;
  }

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
    double rotatedBoundImageWidth = boundRect.width();
    double rotatedBoundImageHeight = boundRect.height();
    imageSizeConsideringRotation( rotatedBoundImageWidth, rotatedBoundImageHeight );
    double rotatedBoundImageWidthMM = rotatedBoundImageWidth / newDpi * 25.4;
    double rotatedBoundImageHeightMM = rotatedBoundImageHeight / newDpi * 25.4;

    if ( mMode == SVG )
    {
      if ( !mSvgCacheUpToDate )
      {
        //make nicer preview
        if ( mComposition && mComposition->plotStyle() == QgsComposition::Preview )
        {
          rotatedBoundImageWidth *= std::min( viewScaleFactor, 10.0 );
          rotatedBoundImageHeight *= std::min( viewScaleFactor, 10.0 );
        }
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
  mCachedRotation = mRotation;
  mCachedViewScaleFactor = viewScaleFactor;

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



void QgsComposerPicture::setSceneRect( const QRectF& rectangle )
{
  mSvgCacheUpToDate = false;
  QgsComposerItem::setSceneRect( rectangle );
  emit settingsChanged();
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
  setRotation( map->rotation() );
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
