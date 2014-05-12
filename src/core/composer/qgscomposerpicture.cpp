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
#include "qgsatlascomposition.h"
#include "qgsproject.h"
#include "qgsexpression.h"
#include "qgsvectorlayer.h"
#include "qgsmessagelog.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QImageReader>
#include <QPainter>
#include <QSvgRenderer>


QgsComposerPicture::QgsComposerPicture( QgsComposition *composition ) :
    QgsComposerItem( composition ),
    mMode( Unknown ),
    mUseSourceExpression( false ),
    mPictureRotation( 0 ),
    mRotationMap( 0 ),
    mResizeMode( QgsComposerPicture::Zoom ),
    mPictureExpr( 0 )
{
  mPictureWidth = rect().width();
  init();
}

QgsComposerPicture::QgsComposerPicture() : QgsComposerItem( 0 ),
    mMode( Unknown ),
    mUseSourceExpression( false ),
    mPictureRotation( 0 ),
    mRotationMap( 0 ),
    mResizeMode( QgsComposerPicture::Zoom ),
    mPictureExpr( 0 )
{
  mPictureHeight = rect().height();
  init();
}

void QgsComposerPicture::init()
{
  //connect some signals

  //connect to atlas toggling on/off and coverage layer changes
  //to update the picture source expression
  connect( &mComposition->atlasComposition(), SIGNAL( toggled( bool ) ), this, SLOT( updatePictureExpression() ) );
  connect( &mComposition->atlasComposition(), SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ), this, SLOT( updatePictureExpression() ) );

  //connect to atlas feature changing
  connect( &mComposition->atlasComposition(), SIGNAL( featureChanged( QgsFeature* ) ), this, SLOT( refreshPicture() ) );

  //connect to composer print resolution changing
  connect( mComposition, SIGNAL( printResolutionChanged() ), this, SLOT( recalculateSize() ) );
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
    double boundRectWidthMM;
    double boundRectHeightMM;
    double imageRectWidthMM;
    double imageRectHeightMM;
    if ( mResizeMode == QgsComposerPicture::Zoom || mResizeMode == QgsComposerPicture::ZoomResizeFrame )
    {
      boundRectWidthMM = mPictureWidth;
      boundRectHeightMM = mPictureHeight;
      imageRectWidthMM = mImage.width();
      imageRectHeightMM = mImage.height();
    }
    else if ( mResizeMode == QgsComposerPicture::Stretch )
    {
      boundRectWidthMM = rect().width();
      boundRectHeightMM = rect().height();
      imageRectWidthMM = mImage.width();
      imageRectHeightMM = mImage.height();
    }
    else
    {
      boundRectWidthMM = rect().width();
      boundRectHeightMM = rect().height();
      imageRectWidthMM = rect().width() * mComposition->printResolution() / 25.4;
      imageRectHeightMM = rect().height() * mComposition->printResolution() / 25.4;
    }
    painter->save();

    if ( mResizeMode == Zoom )
    {
      painter->translate( rect().width() / 2.0, rect().height() / 2.0 );
      painter->rotate( mPictureRotation );
      painter->translate( -boundRectWidthMM / 2.0, -boundRectHeightMM / 2.0 );
    }

    if ( mMode == SVG )
    {
      mSVG.render( painter, QRectF( 0, 0, boundRectWidthMM,  boundRectHeightMM ) );
    }
    else if ( mMode == RASTER )
    {
      painter->drawImage( QRectF( 0, 0, boundRectWidthMM,  boundRectHeightMM ), mImage, QRectF( 0, 0, imageRectWidthMM, imageRectHeightMM ) );
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
  refreshPicture();
}

void QgsComposerPicture::updatePictureExpression()
{
  QgsVectorLayer * vl = 0;
  if ( mComposition->atlasComposition().enabled() )
  {
    vl = mComposition->atlasComposition().coverageLayer();
  }

  if ( mSourceExpression.size() > 0 )
  {
    if ( mPictureExpr )
    {
      delete mPictureExpr;
    }
    mPictureExpr = new QgsExpression( mSourceExpression );
    // expression used to evaluate picture source
    // test for evaluation errors
    if ( mPictureExpr->hasParserError() )
    {
      QgsMessageLog::logMessage( tr( "Picture expression parsing error: %1" ).arg( mPictureExpr->parserErrorString() ), tr( "Composer" ) );
    }

    if ( vl )
    {
      const QgsFields& fields = vl->pendingFields();
      mPictureExpr->prepare( fields );
    }
  }
}

QString QgsComposerPicture::evalPictureExpression()
{
  //generate filename for picture
  if ( mSourceExpression.size() > 0 && mUseSourceExpression )
  {
    if ( ! mPictureExpr )
    {
      return QString();
    }

    QVariant filenameRes;
    QgsAtlasComposition* atlas = &( mComposition->atlasComposition() );
    if ( atlas->enabled() )
    {
      //expression needs to be evaluated considering the current atlas feature
      filenameRes = mPictureExpr->evaluate( atlas->currentFeature(),
                                            atlas->coverageLayer()->pendingFields() );
    }
    else
    {
      filenameRes = mPictureExpr->evaluate();
    }

    if ( mPictureExpr->hasEvalError() )
    {
      QgsMessageLog::logMessage( tr( "Picture expression eval error: %1" ).arg( mPictureExpr->evalErrorString() ), tr( "Composer" ) );
    }

    return filenameRes.toString();
  }
  else
  {
    return QString();
  }
}

void QgsComposerPicture::refreshPicture()
{
  if ( mUseSourceExpression )
  {
    //using expression for picture source file

    //evaluate expression
    QFile path;
    path.setFileName( evalPictureExpression() );
    loadPicture( path );
  }
  else
  {
    //using a static picture path
    loadPicture( mSourceFile );
  }
}

void QgsComposerPicture::loadPicture( const QFile& file )
{
  if ( !file.exists() )
  {
    mMode = Unknown;
  }

  QFileInfo sourceFileInfo( file );
  QString sourceFileSuffix = sourceFileInfo.suffix();
  if ( sourceFileSuffix.compare( "svg", Qt::CaseInsensitive ) == 0 )
  {
    //try to open svg
    mSVG.load( file.fileName() );
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
    QImageReader imageReader( file.fileName() );
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
    recalculateSize();
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

  QSizeF currentPictureSize = pictureSize();

  if ( mResizeMode == QgsComposerPicture::Clip )
  {
    QgsComposerItem::setSceneRect( rectangle );
    mPictureWidth = rectangle.width();
    mPictureHeight = rectangle.height();
    return;
  }

  QRectF newRect = rectangle;

  if ( mResizeMode == ZoomResizeFrame && !rect().isEmpty() && !( currentPictureSize.isEmpty() ) )
  {
    //if width has changed less than height, then fix width and set height correspondingly
    //else, do the opposite
    if ( qAbs( rect().width() - rectangle.width() ) <
         qAbs( rect().height() - rectangle.height() ) )
    {
      newRect.setHeight( currentPictureSize.height() * newRect.width() / currentPictureSize.width() );
    }
    else
    {
      newRect.setWidth( currentPictureSize.width() * newRect.height() / currentPictureSize.height() );
    }
  }
  else if ( mResizeMode == FrameToImageSize )
  {
    if ( !( currentPictureSize.isEmpty() ) )
    {
      newRect.setWidth( currentPictureSize.width() * 25.4 / mComposition->printResolution() );
      newRect.setHeight( currentPictureSize.height() * 25.4 / mComposition->printResolution() );
    }
  }

  //find largest scaling of picture with this rotation which fits in item
  if ( mResizeMode == Zoom )
  {
    QRectF rotatedImageRect = largestRotatedRectWithinBounds( QRectF( 0, 0, currentPictureSize.width(), currentPictureSize.height() ), newRect, mPictureRotation );
    mPictureWidth = rotatedImageRect.width();
    mPictureHeight = rotatedImageRect.height();
  }
  else
  {
    mPictureWidth = newRect.width();
    mPictureHeight = newRect.height();
  }

  QgsComposerItem::setSceneRect( newRect );
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

  if ( mResizeMode == Zoom )
  {
    //find largest scaling of picture with this rotation which fits in item
    QSizeF currentPictureSize = pictureSize();
    QRectF rotatedImageRect = largestRotatedRectWithinBounds( QRectF( 0, 0, currentPictureSize.width(), currentPictureSize.height() ), rect(), mPictureRotation );
    mPictureWidth = rotatedImageRect.width();
    mPictureHeight = rotatedImageRect.height();
    update();
  }

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

void QgsComposerPicture::setResizeMode( QgsComposerPicture::ResizeMode mode )
{
  mResizeMode = mode;
  if ( mode == QgsComposerPicture::ZoomResizeFrame || mode == QgsComposerPicture::FrameToImageSize
       || ( mode == QgsComposerPicture::Zoom && mPictureRotation != 0 ) )
  {
    //call set scene rect to force item to resize to fit picture
    recalculateSize();
  }
  update();
}

void QgsComposerPicture::recalculateSize()
{
  //call set scene rect with current position/size, as this will trigger the
  //picture item to recalculate its frame and image size
  setSceneRect( QRectF( pos().x(), pos().y(), rect().width(), rect().height() ) );
}

void QgsComposerPicture::setUsePictureExpression( bool useExpression )
{
  if ( useExpression == mUseSourceExpression )
  {
    return;
  }

  mUseSourceExpression = useExpression;
  if ( useExpression )
  {
    updatePictureExpression();
  }

  refreshPicture();
}

void QgsComposerPicture::setPictureExpression( QString expression )
{
  if ( expression == mSourceExpression )
  {
    return;
  }

  mSourceExpression = expression;

  if ( mUseSourceExpression )
  {
    updatePictureExpression();
    refreshPicture();
  }
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
  composerPictureElem.setAttribute( "resizeMode", QString::number(( int )mResizeMode ) );
  if ( mUseSourceExpression )
  {
    composerPictureElem.setAttribute( "useExpression", "true" );
  }
  else
  {
    composerPictureElem.setAttribute( "useExpression", "false" );
  }
  composerPictureElem.setAttribute( "sourceExpression", mSourceExpression );

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
  mResizeMode = QgsComposerPicture::ResizeMode( itemElem.attribute( "resizeMode", "0" ).toInt() );

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


  mSourceExpression = itemElem.attribute( "sourceExpression", "" );
  QString useExpression = itemElem.attribute( "useExpression" );
  if ( useExpression.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    mUseSourceExpression = true;
    updatePictureExpression();
  }
  else
  {
    mUseSourceExpression = false;
  }

  QString fileName = QgsProject::instance()->readPath( itemElem.attribute( "file" ) );
  mSourceFile.setFileName( fileName );

  refreshPicture();

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
