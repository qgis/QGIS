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
#include "qgscomposition.h"
#include "qgsproject.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QImageReader>
#include <QPainter>
#include <QSvgRenderer>


QgsComposerPicture::QgsComposerPicture( QgsComposition *composition )
    : QgsComposerItem( composition ), mMode( Unknown ), mPictureRotation( 0 ), mRotationMap( 0 )
{
  mPictureWidth = rect().width();
}

QgsComposerPicture::QgsComposerPicture(): QgsComposerItem( 0 ), mMode( Unknown ), mPictureRotation( 0 ), mRotationMap( 0 )
{
  mPictureHeight = rect().height();
}

QgsComposerPicture::~QgsComposerPicture()
{

}

void QgsComposerPicture::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter )
  {
    return;
  }

  drawBackground( painter );

  //int newDpi = ( painter->device()->logicalDpiX() + painter->device()->logicalDpiY() ) / 2;

  if ( mMode != Unknown )
  {
    double boundRectWidthMM = mPictureWidth;
    double boundRectHeightMM = mPictureHeight;

    painter->save();
    painter->translate( rect().width() / 2.0, rect().height() / 2.0 );
    painter->rotate( mPictureRotation );
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
    setSceneRect( QRectF( pos().x(), pos().y(), rect().width(), rect().height() ) );
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

QSizeF QgsComposerPicture::pictureSize()
{
  if ( mMode == SVG )
  {
    return mDefaultSvgSize;
  }
  else if ( mMode == RASTER )
  {
    return QSizeF( mImage.width(), mImage.height() );
  }
  else
  {
    return QSizeF( 0, 0 );
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

  //find largest scaling of picture with this rotation which fits in item
  QSizeF currentPictureSize = pictureSize();
  QRectF rotatedImageRect = largestRotatedRectWithinBounds( QRectF( 0, 0, currentPictureSize.width(), currentPictureSize.height() ), rectangle, mPictureRotation );
  mPictureWidth = rotatedImageRect.width();
  mPictureHeight = rotatedImageRect.height();

  emit itemChanged();
}

void QgsComposerPicture::setRotation( double r )
{
  //kept for compatibility for QGIS2.0 api
  setPictureRotation( r );
}

void QgsComposerPicture::setPictureRotation( double r )
{
  mPictureRotation = r;

  //find largest scaling of picture with this rotation which fits in item
  QSizeF currentPictureSize = pictureSize();
  QRectF rotatedImageRect = largestRotatedRectWithinBounds( QRectF( 0, 0, currentPictureSize.width(), currentPictureSize.height() ), rect(), mPictureRotation );
  mPictureWidth = rotatedImageRect.width();
  mPictureHeight = rotatedImageRect.height();

  update();
  emit pictureRotationChanged( mPictureRotation );
}

void QgsComposerPicture::setRotationMap( int composerMapId )
{
  if ( !mComposition )
  {
    return;
  }

  if ( composerMapId == -1 ) //disable rotation from map
  {
    QObject::disconnect( mRotationMap, SIGNAL( mapRotationChanged( double ) ), this, SLOT( setPictureRotation( double ) ) );
    mRotationMap = 0;
  }

  const QgsComposerMap* map = mComposition->getComposerMapById( composerMapId );
  if ( !map )
  {
    return;
  }
  if ( mRotationMap )
  {
    QObject::disconnect( mRotationMap, SIGNAL( mapRotationChanged( double ) ), this, SLOT( setPictureRotation( double ) ) );
  }
  mPictureRotation = map->mapRotation();
  QObject::connect( map, SIGNAL( mapRotationChanged( double ) ), this, SLOT( setPictureRotation( double ) ) );
  mRotationMap = map;
  update();
  emit pictureRotationChanged( mPictureRotation );
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
  composerPictureElem.setAttribute( "pictureWidth", QString::number( mPictureWidth ) );
  composerPictureElem.setAttribute( "pictureHeight", QString::number( mPictureHeight ) );

  //rotation
  composerPictureElem.setAttribute( "pictureRotation",  QString::number( mPictureRotation ) );
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
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    if ( composerItemElem.attribute( "rotation", "0" ).toDouble() != 0 )
    {
      //in versions prior to 2.1 picture rotation was stored in the rotation attribute
      mPictureRotation = composerItemElem.attribute( "rotation", "0" ).toDouble();
    }

    _readXML( composerItemElem, doc );
  }

  mDefaultSvgSize = QSize( 0, 0 );

  QString fileName = QgsProject::instance()->readPath( itemElem.attribute( "file" ) );
  setPictureFile( fileName );

  //picture rotation
  if ( itemElem.attribute( "pictureRotation", "0" ).toDouble() != 0 )
  {
    mPictureRotation = itemElem.attribute( "pictureRotation", "0" ).toDouble();
  }

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
      QObject::disconnect( mRotationMap, SIGNAL( mapRotationChanged( double ) ), this, SLOT( setRotation( double ) ) );
    }
    mRotationMap = mComposition->getComposerMapById( rotationMapId );
    QObject::connect( mRotationMap, SIGNAL( mapRotationChanged( double ) ), this, SLOT( setRotation( double ) ) );
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

bool QgsComposerPicture::imageSizeConsideringRotation( double& width, double& height ) const
{
  //kept for api compatibility with QGIS 2.0 - use mPictureRotation
  return QgsComposerItem::imageSizeConsideringRotation( width, height, mPictureRotation );
}

bool QgsComposerPicture::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const
{
  //kept for api compatibility with QGIS 2.0 - use mPictureRotation
  return QgsComposerItem::cornerPointOnRotatedAndScaledRect( x, y, width, height, mPictureRotation );
}

void QgsComposerPicture::sizeChangedByRotation( double& width, double& height )
{
  //kept for api compatibility with QGIS 2.0 - use mPictureRotation
  return QgsComposerItem::sizeChangedByRotation( width, height, mPictureRotation );
}
