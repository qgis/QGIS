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
    mRotationMap( 0 )
{
  mPictureWidth = rect().width();
}

QgsComposerPicture::QgsComposerPicture(): QgsComposerItem( 0 ), mMode( Unknown ), mRotationMap( 0 )
{
  mPictureHeight = rect().height();
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
    double rectPixelWidth = /*rect().width()*/mPictureWidth * newDpi / 25.4;
    double rectPixelHeight = /*rect().height()*/ mPictureHeight * newDpi / 25.4;
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

    painter->save();
    painter->translate( rect().width() / 2.0, rect().height() / 2.0 );
    painter->rotate( mRotation );
    painter->translate( -boundRectWidthMM / 2.0, -boundRectHeightMM / 2.0 );

    if ( mMode == SVG )
    {
      mSVG.render( painter, QRectF( 0, 0, boundRectWidthMM,  boundRectHeightMM ) );
    }
    else if ( mMode == RASTER )
    {
      painter->drawImage( QRectF( 0, 0, boundRectWidthMM,  boundRectHeightMM ), mImage, QRectF( 0, 0, mImage.width(), mImage.height() ) );
    }

    painter->restore();
  }

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
    mSVG.load( mSourceFile.fileName() );
    if ( mSVG.isValid() )
    {
      mMode = SVG;
      QRect viewBox = mSVG.viewBox(); //take width/height ratio from view box instead of default size
      mDefaultSvgSize.setWidth( viewBox.width() );
      mDefaultSvgSize.setHeight( viewBox.height() );
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
  emit itemChanged();
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
  if ( deviceWidth / mDefaultSvgSize.width() > deviceHeight / mDefaultSvgSize.height() )
  {
    imageToSvgRatio = deviceHeight / mDefaultSvgSize.height();
    double width = mDefaultSvgSize.width() * imageToSvgRatio;
    return QRectF( 0, 0, width, deviceHeight );
  }
  else
  {
    imageToSvgRatio = deviceWidth / mDefaultSvgSize.width();
    double height = mDefaultSvgSize.height() * imageToSvgRatio;
    return QRectF( 0, 0, deviceWidth, height );
  }
}

#if 0
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
#endif //0

void QgsComposerPicture::setSceneRect( const QRectF& rectangle )
{
  QgsComposerItem::setSceneRect( rectangle );

  //consider to change size of the shape if the rectangle changes width and/or height
  double newPictureWidth = rectangle.width();
  double newPictureHeight = rectangle.height();
  imageSizeConsideringRotation( newPictureWidth, newPictureHeight );
  mPictureWidth = newPictureWidth;
  mPictureHeight = newPictureHeight;

  emit itemChanged();
}

void QgsComposerPicture::setRotation( double r )
{
  //adapt rectangle size
  double width = mPictureWidth;
  double height = mPictureHeight;
  sizeChangedByRotation( width, height );

  //adapt scene rect to have the same center and the new width / height
  double x = transform().dx() + rect().width() / 2.0 - width / 2.0;
  double y = transform().dy() + rect().height() / 2.0 - height / 2.0;
  QgsComposerItem::setSceneRect( QRectF( x, y, width, height ) );

  QgsComposerItem::setRotation( r );
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
  composerPictureElem.setAttribute( "pictureWidth", mPictureWidth );
  composerPictureElem.setAttribute( "pictureHeight", mPictureHeight );
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

  mPictureWidth = itemElem.attribute( "pictureWidth", "10" ).toDouble();
  mPictureHeight = itemElem.attribute( "pictureHeight", "10" ).toDouble();

  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    _readXML( composerItemList.at( 0 ).toElement(), doc );
  }


  mDefaultSvgSize = QSize( 0, 0 );

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

  emit itemChanged();
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
