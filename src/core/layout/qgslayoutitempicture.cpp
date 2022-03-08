/***************************************************************************
                             qgslayoutitempicture.cpp
                             ------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitempicture.h"
#include "qgslayoutitemregistry.h"
#include "qgslayout.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgslayoutitemmap.h"
#include "qgslayoututils.h"
#include "qgsproject.h"
#include "qgsexpression.h"
#include "qgsvectorlayer.h"
#include "qgsmessagelog.h"
#include "qgspathresolver.h"
#include "qgsproperty.h"
#include "qgsnetworkcontentfetcher.h"
#include "qgssymbollayerutils.h"
#include "qgssvgcache.h"
#include "qgslogger.h"
#include "qgsbearingutils.h"
#include "qgsmapsettings.h"
#include "qgsreadwritecontext.h"
#include "qgsimagecache.h"
#include "qgslayoutnortharrowhandler.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QImageReader>
#include <QPainter>
#include <QSvgRenderer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QCoreApplication>
#include <QUrl>

QgsLayoutItemPicture::QgsLayoutItemPicture( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mNorthArrowHandler( new QgsLayoutNorthArrowHandler( this ) )
{
  //default to no background
  setBackgroundEnabled( false );

  //connect some signals

  //connect to atlas feature changing
  //to update the picture source expression
  connect( &layout->reportContext(), &QgsLayoutReportContext::changed, this, [ = ] { refreshPicture(); } );

  //connect to layout print resolution changing
  connect( &layout->renderContext(), &QgsLayoutRenderContext::dpiChanged, this, &QgsLayoutItemPicture::recalculateSize );

  connect( this, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutItemPicture::shapeChanged );
  connect( mNorthArrowHandler, &QgsLayoutNorthArrowHandler::arrowRotationChanged, this, &QgsLayoutItemPicture::updateNorthArrowRotation );
}

int QgsLayoutItemPicture::type() const
{
  return QgsLayoutItemRegistry::LayoutPicture;
}

QIcon QgsLayoutItemPicture::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemPicture.svg" ) );
}

QgsLayoutItemPicture *QgsLayoutItemPicture::create( QgsLayout *layout )
{
  return new QgsLayoutItemPicture( layout );
}

void QgsLayoutItemPicture::draw( QgsLayoutItemRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  const QgsScopedQPainterState painterState( painter );
  // painter is scaled to dots, so scale back to layout units
  painter->scale( context.renderContext().scaleFactor(), context.renderContext().scaleFactor() );

  const bool prevSmoothTransform = painter->testRenderHint( QPainter::RenderHint::SmoothPixmapTransform );
  if ( mLayout->renderContext().testFlag( QgsLayoutRenderContext::FlagAntialiasing ) )
    painter->setRenderHint( QPainter::RenderHint::SmoothPixmapTransform, true );

  //picture resizing
  if ( mMode != FormatUnknown )
  {
    double boundRectWidthMM;
    double boundRectHeightMM;
    QRect imageRect;
    if ( mResizeMode == QgsLayoutItemPicture::Zoom || mResizeMode == QgsLayoutItemPicture::ZoomResizeFrame )
    {
      boundRectWidthMM = mPictureWidth;
      boundRectHeightMM = mPictureHeight;
      imageRect = QRect( 0, 0, mImage.width(), mImage.height() );
    }
    else if ( mResizeMode == QgsLayoutItemPicture::Stretch )
    {
      boundRectWidthMM = rect().width();
      boundRectHeightMM = rect().height();
      imageRect = QRect( 0, 0, mImage.width(), mImage.height() );
    }
    else if ( mResizeMode == QgsLayoutItemPicture::Clip )
    {
      boundRectWidthMM = rect().width();
      boundRectHeightMM = rect().height();
      const int imageRectWidthPixels = mImage.width();
      const int imageRectHeightPixels = mImage.height();
      imageRect = clippedImageRect( boundRectWidthMM, boundRectHeightMM,
                                    QSize( imageRectWidthPixels, imageRectHeightPixels ) );
    }
    else
    {
      boundRectWidthMM = rect().width();
      boundRectHeightMM = rect().height();
      imageRect = QRect( 0, 0, mLayout->convertFromLayoutUnits( rect().width(), QgsUnitTypes::LayoutMillimeters ).length() * mLayout->renderContext().dpi() / 25.4,
                         mLayout->convertFromLayoutUnits( rect().height(), QgsUnitTypes::LayoutMillimeters ).length() * mLayout->renderContext().dpi() / 25.4 );
    }

    //zoom mode - calculate anchor point and rotation
    if ( mResizeMode == Zoom )
    {
      //TODO - allow placement modes with rotation set. for now, setting a rotation
      //always places picture in center of frame
      if ( !qgsDoubleNear( mPictureRotation, 0.0 ) )
      {
        painter->translate( rect().width() / 2.0, rect().height() / 2.0 );
        painter->rotate( mPictureRotation );
        painter->translate( -boundRectWidthMM / 2.0, -boundRectHeightMM / 2.0 );
      }
      else
      {
        //shift painter to edge/middle of frame depending on placement
        const double diffX = rect().width() - boundRectWidthMM;
        const double diffY = rect().height() - boundRectHeightMM;

        double dX = 0;
        double dY = 0;
        switch ( mPictureAnchor )
        {
          case UpperLeft:
          case MiddleLeft:
          case LowerLeft:
            //nothing to do
            break;
          case UpperMiddle:
          case Middle:
          case LowerMiddle:
            dX = diffX / 2.0;
            break;
          case UpperRight:
          case MiddleRight:
          case LowerRight:
            dX = diffX;
            break;
        }
        switch ( mPictureAnchor )
        {
          case UpperLeft:
          case UpperMiddle:
          case UpperRight:
            //nothing to do
            break;
          case MiddleLeft:
          case Middle:
          case MiddleRight:
            dY = diffY / 2.0;
            break;
          case LowerLeft:
          case LowerMiddle:
          case LowerRight:
            dY = diffY;
            break;
        }
        painter->translate( dX, dY );
      }
    }
    else if ( mResizeMode == ZoomResizeFrame )
    {
      if ( !qgsDoubleNear( mPictureRotation, 0.0 ) )
      {
        painter->translate( rect().width() / 2.0, rect().height() / 2.0 );
        painter->rotate( mPictureRotation );
        painter->translate( -boundRectWidthMM / 2.0, -boundRectHeightMM / 2.0 );
      }
    }

    if ( mMode == FormatSVG )
    {
      mSVG.render( painter, QRectF( 0, 0, boundRectWidthMM, boundRectHeightMM ) );
    }
    else if ( mMode == FormatRaster )
    {
      painter->drawImage( QRectF( 0, 0, boundRectWidthMM, boundRectHeightMM ), mImage, imageRect );
    }
  }
  painter->setRenderHint( QPainter::RenderHint::SmoothPixmapTransform, prevSmoothTransform );
}

QSizeF QgsLayoutItemPicture::applyItemSizeConstraint( const QSizeF targetSize )
{
  const QSizeF currentPictureSize = pictureSize();
  QSizeF newSize = targetSize;
  if ( mResizeMode == QgsLayoutItemPicture::Clip )
  {
    mPictureWidth = targetSize.width();
    mPictureHeight = targetSize.height();
  }
  else
  {
    if ( mResizeMode == ZoomResizeFrame && !rect().isEmpty() && !( currentPictureSize.isEmpty() ) )
    {
      QSizeF targetImageSize;
      if ( qgsDoubleNear( mPictureRotation, 0.0 ) )
      {
        targetImageSize = currentPictureSize;
      }
      else
      {
        //calculate aspect ratio of bounds of rotated image
        QTransform tr;
        tr.rotate( mPictureRotation );
        const QRectF rotatedBounds = tr.mapRect( QRectF( 0, 0, currentPictureSize.width(), currentPictureSize.height() ) );
        targetImageSize = QSizeF( rotatedBounds.width(), rotatedBounds.height() );
      }

      //if height has changed more than width, then fix width and set height correspondingly
      //else, do the opposite
      if ( std::fabs( rect().width() - targetSize.width() ) <
           std::fabs( rect().height() - targetSize.height() ) )
      {
        newSize.setHeight( targetImageSize.height() * newSize.width() / targetImageSize.width() );
      }
      else
      {
        newSize.setWidth( targetImageSize.width() * newSize.height() / targetImageSize.height() );
      }
    }
    else if ( mResizeMode == FrameToImageSize )
    {
      if ( !( currentPictureSize.isEmpty() ) )
      {
        const QgsLayoutSize sizeMM = mLayout->convertFromLayoutUnits( currentPictureSize, QgsUnitTypes::LayoutMillimeters );
        newSize.setWidth( sizeMM.width() * 25.4 / mLayout->renderContext().dpi() );
        newSize.setHeight( sizeMM.height() * 25.4 / mLayout->renderContext().dpi() );
      }
    }

    //find largest scaling of picture with this rotation which fits in item
    if ( mResizeMode == Zoom || mResizeMode == ZoomResizeFrame )
    {
      const QRectF rotatedImageRect = QgsLayoutUtils::largestRotatedRectWithinBounds( QRectF( 0, 0, currentPictureSize.width(), currentPictureSize.height() ),
                                      QRectF( 0, 0, newSize.width(), newSize.height() ), mPictureRotation );
      mPictureWidth = rotatedImageRect.width();
      mPictureHeight = rotatedImageRect.height();
    }
    else
    {
      mPictureWidth = newSize.width();
      mPictureHeight = newSize.height();
    }

    if ( newSize != targetSize )
    {
      emit changed();
    }
  }

  return newSize;
}

QRect QgsLayoutItemPicture::clippedImageRect( double &boundRectWidthMM, double &boundRectHeightMM, QSize imageRectPixels )
{
  const int boundRectWidthPixels = boundRectWidthMM * mLayout->renderContext().dpi() / 25.4;
  const int boundRectHeightPixels = boundRectHeightMM * mLayout->renderContext().dpi() / 25.4;

  //update boundRectWidth/Height so that they exactly match pixel bounds
  boundRectWidthMM = boundRectWidthPixels * 25.4 / mLayout->renderContext().dpi();
  boundRectHeightMM = boundRectHeightPixels * 25.4 / mLayout->renderContext().dpi();

  //calculate part of image which fits in bounds
  int leftClip = 0;
  int topClip = 0;

  //calculate left crop
  switch ( mPictureAnchor )
  {
    case UpperLeft:
    case MiddleLeft:
    case LowerLeft:
      leftClip = 0;
      break;
    case UpperMiddle:
    case Middle:
    case LowerMiddle:
      leftClip = ( imageRectPixels.width() - boundRectWidthPixels ) / 2;
      break;
    case UpperRight:
    case MiddleRight:
    case LowerRight:
      leftClip = imageRectPixels.width() - boundRectWidthPixels;
      break;
  }

  //calculate top crop
  switch ( mPictureAnchor )
  {
    case UpperLeft:
    case UpperMiddle:
    case UpperRight:
      topClip = 0;
      break;
    case MiddleLeft:
    case Middle:
    case MiddleRight:
      topClip = ( imageRectPixels.height() - boundRectHeightPixels ) / 2;
      break;
    case LowerLeft:
    case LowerMiddle:
    case LowerRight:
      topClip = imageRectPixels.height() - boundRectHeightPixels;
      break;
  }

  return QRect( leftClip, topClip, boundRectWidthPixels, boundRectHeightPixels );
}

void QgsLayoutItemPicture::refreshPicture( const QgsExpressionContext *context )
{
  const QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  mDataDefinedProperties.prepare( *evalContext );

  QVariant source( mSourcePath );

  //data defined source set?
  mHasExpressionError = false;
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::PictureSource ) )
  {
    mMode = FormatUnknown;
    bool ok = false;
    const QgsProperty &sourceProperty = mDataDefinedProperties.property( QgsLayoutObject::PictureSource );
    source = sourceProperty.value( *evalContext, source, &ok );
    if ( !ok || !source.canConvert( QMetaType::QString ) )
    {
      mHasExpressionError = true;
      source = QString();
      QgsMessageLog::logMessage( tr( "Picture expression eval error" ) );
    }
    else if ( source.type() != QVariant::ByteArray )
    {
      source = source.toString().trimmed();
      QgsDebugMsgLevel( QStringLiteral( "exprVal PictureSource:%1" ).arg( source.toString() ), 2 );
    }
  }

  loadPicture( source );
}

void QgsLayoutItemPicture::loadRemotePicture( const QString &url )
{
  //remote location

  QgsNetworkContentFetcher fetcher;
  QEventLoop loop;
  connect( &fetcher, &QgsNetworkContentFetcher::finished, &loop, &QEventLoop::quit );
  fetcher.fetchContent( QUrl( url ) );

  //wait until picture fetched
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  QNetworkReply *reply = fetcher.reply();
  if ( reply )
  {
    QImageReader imageReader( reply );
    imageReader.setAutoTransform( true );

    if ( imageReader.format() == "pdf" )
    {
      // special handling for this format -- we need to pass the desired target size onto the image reader
      // so that it can correctly render the (vector) pdf content at the desired dpi. Otherwise it returns
      // a very low resolution image (the driver assumes points == pixels!)
      // For other image formats, we read the original image size only and defer resampling to later in this
      // function. That gives us more control over the resampling method used.

      // driver assumes points == pixels, so driver image size is reported assuming 72 dpi.
      const QSize sizeAt72Dpi = imageReader.size();
      const QSize sizeAtTargetDpi = sizeAt72Dpi * mLayout->renderContext().dpi() / 72;
      imageReader.setScaledSize( sizeAtTargetDpi );
    }

    mImage = imageReader.read();
    mMode = FormatRaster;
  }
  else
  {
    mMode = FormatUnknown;
  }
}

void QgsLayoutItemPicture::loadLocalPicture( const QString &path )
{
  QFile pic;
  pic.setFileName( path );

  if ( !pic.exists() )
  {
    mMode = FormatUnknown;
  }
  else
  {
    const QFileInfo sourceFileInfo( pic );
    const QString sourceFileSuffix = sourceFileInfo.suffix();
    if ( sourceFileSuffix.compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
    {
      //try to open svg
      const QgsExpressionContext context = createExpressionContext();
      const QColor fillColor = mDataDefinedProperties.valueAsColor( QgsLayoutObject::PictureSvgBackgroundColor, context, mSvgFillColor );
      const QColor strokeColor = mDataDefinedProperties.valueAsColor( QgsLayoutObject::PictureSvgStrokeColor, context, mSvgStrokeColor );
      const double strokeWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::PictureSvgStrokeWidth, context, mSvgStrokeWidth );
      const QgsStringMap evaluatedParameters = QgsSymbolLayerUtils::evaluatePropertiesMap( svgDynamicParameters(), context );

      const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( path, rect().width(), fillColor, strokeColor, strokeWidth,
                                     1.0, 0, false, evaluatedParameters );
      mSVG.load( svgContent );
      if ( mSVG.isValid() )
      {
        mMode = FormatSVG;
        const QRect viewBox = mSVG.viewBox(); //take width/height ratio from view box instead of default size
        mDefaultSvgSize.setWidth( viewBox.width() );
        mDefaultSvgSize.setHeight( viewBox.height() );
      }
      else
      {
        mMode = FormatUnknown;
      }
    }
    else
    {
      //try to open raster with QImageReader
      QImageReader imageReader( pic.fileName() );
      imageReader.setAutoTransform( true );

      if ( imageReader.format() == "pdf" )
      {
        // special handling for this format -- we need to pass the desired target size onto the image reader
        // so that it can correctly render the (vector) pdf content at the desired dpi. Otherwise it returns
        // a very low resolution image (the driver assumes points == pixels!)
        // For other image formats, we read the original image size only and defer resampling to later in this
        // function. That gives us more control over the resampling method used.

        // driver assumes points == pixels, so driver image size is reported assuming 72 dpi.
        const QSize sizeAt72Dpi = imageReader.size();
        const QSize sizeAtTargetDpi = sizeAt72Dpi * mLayout->renderContext().dpi() / 72;
        imageReader.setScaledSize( sizeAtTargetDpi );
      }

      if ( imageReader.read( &mImage ) )
      {
        mMode = FormatRaster;
      }
      else
      {
        mMode = FormatUnknown;
      }
    }
  }
}

void QgsLayoutItemPicture::loadPictureUsingCache( const QString &path )
{
  if ( path.isEmpty() )
  {
    mImage = QImage();
    return;
  }

  switch ( mMode )
  {
    case FormatUnknown:
      break;

    case FormatRaster:
    {
      bool fitsInCache = false;
      bool isMissing = false;
      mImage = QgsApplication::imageCache()->pathAsImage( path, QSize(), true, 1, fitsInCache, true, mLayout->renderContext().dpi(), -1, &isMissing );
      if ( mImage.isNull() || isMissing )
        mMode = FormatUnknown;
      break;
    }

    case FormatSVG:
    {
      const QgsExpressionContext context = createExpressionContext();
      const QColor fillColor = mDataDefinedProperties.valueAsColor( QgsLayoutObject::PictureSvgBackgroundColor, context, mSvgFillColor );
      const QColor strokeColor = mDataDefinedProperties.valueAsColor( QgsLayoutObject::PictureSvgStrokeColor, context, mSvgStrokeColor );
      const double strokeWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::PictureSvgStrokeWidth, context, mSvgStrokeWidth );

      const QgsStringMap evaluatedParameters = QgsSymbolLayerUtils::evaluatePropertiesMap( svgDynamicParameters(), context );

      bool isMissingImage = false;
      const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( path, rect().width(), fillColor, strokeColor, strokeWidth,
                                     1.0, 0, false, evaluatedParameters, &isMissingImage );
      mSVG.load( svgContent );
      if ( mSVG.isValid() && !isMissingImage )
      {
        mMode = FormatSVG;
        const QRect viewBox = mSVG.viewBox(); //take width/height ratio from view box instead of default size
        mDefaultSvgSize.setWidth( viewBox.width() );
        mDefaultSvgSize.setHeight( viewBox.height() );
      }
      else
      {
        mMode = FormatUnknown;
      }
      break;
    }
  }
}

void QgsLayoutItemPicture::updateNorthArrowRotation( double rotation )
{
  setPictureRotation( rotation );
  emit pictureRotationChanged( rotation );
}

void QgsLayoutItemPicture::loadPicture( const QVariant &data )
{
  const Format origFormat = mMode;
  mIsMissingImage = false;
  QVariant imageData( data );
  mEvaluatedPath = data.toString();

  if ( mEvaluatedPath.startsWith( QLatin1String( "base64:" ), Qt::CaseInsensitive ) && mMode == FormatUnknown )
  {
    const QByteArray base64 = mEvaluatedPath.mid( 7 ).toLocal8Bit(); // strip 'base64:' prefix
    imageData = QByteArray::fromBase64( base64, QByteArray::OmitTrailingEquals );
  }

  if ( imageData.type() == QVariant::ByteArray )
  {
    if ( mImage.loadFromData( imageData.toByteArray() ) )
    {
      mMode = FormatRaster;
    }
  }
  else if ( mMode == FormatUnknown  && mEvaluatedPath.startsWith( QLatin1String( "http" ) ) )
  {
    //remote location (unsafe way, uses QEventLoop) - for old API/project compatibility only!!
    loadRemotePicture( mEvaluatedPath );
  }
  else if ( mMode == FormatUnknown )
  {
    //local location - for old API/project compatibility only!!
    loadLocalPicture( mEvaluatedPath );
  }
  else
  {
    loadPictureUsingCache( mEvaluatedPath );
  }

  if ( mMode != FormatUnknown ) //make sure we start with a new QImage
  {
    recalculateSize();
  }
  else if ( mHasExpressionError || !mEvaluatedPath.isEmpty() )
  {
    //trying to load an invalid file or bad expression, show cross picture
    mIsMissingImage = true;
    if ( origFormat == FormatRaster )
    {
      const QString badFile( QStringLiteral( ":/images/composer/missing_image.png" ) );
      QImageReader imageReader( badFile );
      if ( imageReader.read( &mImage ) )
        mMode = FormatRaster;
    }
    else
    {
      const QString badFile( QStringLiteral( ":/images/composer/missing_image.svg" ) );
      mSVG.load( badFile );
      if ( mSVG.isValid() )
      {
        mMode = FormatSVG;
        const QRect viewBox = mSVG.viewBox(); //take width/height ratio from view box instead of default size
        mDefaultSvgSize.setWidth( viewBox.width() );
        mDefaultSvgSize.setHeight( viewBox.height() );
      }
    }
    recalculateSize();
  }

  update();
  emit changed();
}

QRectF QgsLayoutItemPicture::boundedImageRect( double deviceWidth, double deviceHeight )
{
  double imageToDeviceRatio;
  if ( mImage.width() / deviceWidth > mImage.height() / deviceHeight )
  {
    imageToDeviceRatio = deviceWidth / mImage.width();
    const double height = imageToDeviceRatio * mImage.height();
    return QRectF( 0, 0, deviceWidth, height );
  }
  else
  {
    imageToDeviceRatio = deviceHeight / mImage.height();
    const double width = imageToDeviceRatio * mImage.width();
    return QRectF( 0, 0, width, deviceHeight );
  }
}

QRectF QgsLayoutItemPicture::boundedSVGRect( double deviceWidth, double deviceHeight )
{
  double imageToSvgRatio;
  if ( deviceWidth / mDefaultSvgSize.width() > deviceHeight / mDefaultSvgSize.height() )
  {
    imageToSvgRatio = deviceHeight / mDefaultSvgSize.height();
    const double width = mDefaultSvgSize.width() * imageToSvgRatio;
    return QRectF( 0, 0, width, deviceHeight );
  }
  else
  {
    imageToSvgRatio = deviceWidth / mDefaultSvgSize.width();
    const double height = mDefaultSvgSize.height() * imageToSvgRatio;
    return QRectF( 0, 0, deviceWidth, height );
  }
}

QSizeF QgsLayoutItemPicture::pictureSize()
{
  if ( mMode == FormatSVG )
  {
    return mDefaultSvgSize;
  }
  else if ( mMode == FormatRaster )
  {
    return QSizeF( mImage.width(), mImage.height() );
  }
  else
  {
    return QSizeF( 0, 0 );
  }
}

bool QgsLayoutItemPicture::isMissingImage() const
{
  return mIsMissingImage;
}

QString QgsLayoutItemPicture::evaluatedPath() const
{
  return mEvaluatedPath;
}

QMap<QString, QgsProperty> QgsLayoutItemPicture::svgDynamicParameters() const
{
  const QVariantMap parameters = mCustomProperties.value( QStringLiteral( "svg-dynamic-parameters" ), QVariantMap() ).toMap();
  return QgsProperty::variantMapToPropertyMap( parameters );
}

void QgsLayoutItemPicture::setSvgDynamicParameters( const QMap<QString, QgsProperty> &parameters )
{
  const QVariantMap variantMap = QgsProperty::propertyMapToVariantMap( parameters );
  mCustomProperties.setValue( QStringLiteral( "svg-dynamic-parameters" ), variantMap );
  refreshPicture();
}

void QgsLayoutItemPicture::shapeChanged()
{
  if ( mMode == FormatSVG && !mLoadingSvg )
  {
    mLoadingSvg = true;
    refreshPicture();
    mLoadingSvg = false;
  }
}

void QgsLayoutItemPicture::setPictureRotation( double rotation )
{
  const double oldRotation = mPictureRotation;
  mPictureRotation = rotation;

  if ( mResizeMode == Zoom )
  {
    //find largest scaling of picture with this rotation which fits in item
    const QSizeF currentPictureSize = pictureSize();
    const QRectF rotatedImageRect = QgsLayoutUtils::largestRotatedRectWithinBounds( QRectF( 0, 0, currentPictureSize.width(), currentPictureSize.height() ), rect(), mPictureRotation );
    mPictureWidth = rotatedImageRect.width();
    mPictureHeight = rotatedImageRect.height();
    update();
  }
  else if ( mResizeMode == ZoomResizeFrame )
  {
    const QSizeF currentPictureSize = pictureSize();
    const QRectF oldRect = QRectF( pos().x(), pos().y(), rect().width(), rect().height() );

    //calculate actual size of image inside frame
    const QRectF rotatedImageRect = QgsLayoutUtils::largestRotatedRectWithinBounds( QRectF( 0, 0, currentPictureSize.width(), currentPictureSize.height() ), rect(), oldRotation );

    //rotate image rect by new rotation and get bounding box
    QTransform tr;
    tr.rotate( mPictureRotation );
    QRectF newRect = tr.mapRect( QRectF( 0, 0, rotatedImageRect.width(), rotatedImageRect.height() ) );

    //keep the center in the same location
    newRect.moveCenter( oldRect.center() );
    attemptSetSceneRect( newRect );
    emit changed();
  }

  emit pictureRotationChanged( mPictureRotation );
}

void QgsLayoutItemPicture::setLinkedMap( QgsLayoutItemMap *map )
{
  mNorthArrowHandler->setLinkedMap( map );
}

void QgsLayoutItemPicture::setResizeMode( QgsLayoutItemPicture::ResizeMode mode )
{
  mResizeMode = mode;
  if ( mode == QgsLayoutItemPicture::ZoomResizeFrame || mode == QgsLayoutItemPicture::FrameToImageSize
       || ( mode == QgsLayoutItemPicture::Zoom && !qgsDoubleNear( mPictureRotation, 0.0 ) ) )
  {
    //call set scene rect to force item to resize to fit picture
    recalculateSize();
  }
  update();
}

void QgsLayoutItemPicture::recalculateSize()
{
  //call set scene rect with current position/size, as this will trigger the
  //picture item to recalculate its frame and image size
  attemptSetSceneRect( QRectF( pos().x(), pos().y(), rect().width(), rect().height() ) );
}

void QgsLayoutItemPicture::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property )
{
  if ( property == QgsLayoutObject::PictureSource || property == QgsLayoutObject::PictureSvgBackgroundColor
       || property == QgsLayoutObject::PictureSvgStrokeColor || property == QgsLayoutObject::PictureSvgStrokeWidth
       || property == QgsLayoutObject::AllProperties )
  {
    const QgsExpressionContext context = createExpressionContext();
    refreshPicture( &context );
  }

  QgsLayoutItem::refreshDataDefinedProperty( property );
}

void QgsLayoutItemPicture::setPicturePath( const QString &path, Format format )
{
  mMode = format;
  mSourcePath = path;
  refreshPicture();
}

QString QgsLayoutItemPicture::picturePath() const
{
  return mSourcePath;
}

bool QgsLayoutItemPicture::writePropertiesToElement( QDomElement &elem, QDomDocument &, const QgsReadWriteContext &context ) const
{
  QString imagePath = mSourcePath;

  // convert from absolute path to relative. For SVG we also need to consider system SVG paths
  const QgsPathResolver pathResolver = context.pathResolver();
  if ( imagePath.endsWith( QLatin1String( ".svg" ), Qt::CaseInsensitive ) )
    imagePath = QgsSymbolLayerUtils::svgSymbolPathToName( imagePath, pathResolver );
  else
    imagePath = pathResolver.writePath( imagePath );

  elem.setAttribute( QStringLiteral( "file" ), imagePath );
  elem.setAttribute( QStringLiteral( "pictureWidth" ), QString::number( mPictureWidth ) );
  elem.setAttribute( QStringLiteral( "pictureHeight" ), QString::number( mPictureHeight ) );
  elem.setAttribute( QStringLiteral( "resizeMode" ), QString::number( static_cast< int >( mResizeMode ) ) );
  elem.setAttribute( QStringLiteral( "anchorPoint" ), QString::number( static_cast< int >( mPictureAnchor ) ) );
  elem.setAttribute( QStringLiteral( "svgFillColor" ), QgsSymbolLayerUtils::encodeColor( mSvgFillColor ) );
  elem.setAttribute( QStringLiteral( "svgBorderColor" ), QgsSymbolLayerUtils::encodeColor( mSvgStrokeColor ) );
  elem.setAttribute( QStringLiteral( "svgBorderWidth" ), QString::number( mSvgStrokeWidth ) );
  elem.setAttribute( QStringLiteral( "mode" ), mMode );

  //rotation
  elem.setAttribute( QStringLiteral( "pictureRotation" ), QString::number( mPictureRotation ) );
  if ( !mNorthArrowHandler->linkedMap() )
  {
    elem.setAttribute( QStringLiteral( "mapUuid" ), QString() );
  }
  else
  {
    elem.setAttribute( QStringLiteral( "mapUuid" ), mNorthArrowHandler->linkedMap()->uuid() );
  }
  elem.setAttribute( QStringLiteral( "northMode" ), mNorthArrowHandler->northMode() );
  elem.setAttribute( QStringLiteral( "northOffset" ), mNorthArrowHandler->northOffset() );
  return true;
}

bool QgsLayoutItemPicture::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext &context )
{
  mPictureWidth = itemElem.attribute( QStringLiteral( "pictureWidth" ), QStringLiteral( "10" ) ).toDouble();
  mPictureHeight = itemElem.attribute( QStringLiteral( "pictureHeight" ), QStringLiteral( "10" ) ).toDouble();
  mResizeMode = QgsLayoutItemPicture::ResizeMode( itemElem.attribute( QStringLiteral( "resizeMode" ), QStringLiteral( "0" ) ).toInt() );
  //when loading from xml, default to anchor point of middle to match pre 2.4 behavior
  mPictureAnchor = static_cast< QgsLayoutItem::ReferencePoint >( itemElem.attribute( QStringLiteral( "anchorPoint" ), QString::number( QgsLayoutItem::Middle ) ).toInt() );

  mSvgFillColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "svgFillColor" ), QgsSymbolLayerUtils::encodeColor( QColor( 255, 255, 255 ) ) ) );
  mSvgStrokeColor = QgsSymbolLayerUtils::decodeColor( itemElem.attribute( QStringLiteral( "svgBorderColor" ), QgsSymbolLayerUtils::encodeColor( QColor( 0, 0, 0 ) ) ) );
  mSvgStrokeWidth = itemElem.attribute( QStringLiteral( "svgBorderWidth" ), QStringLiteral( "0.2" ) ).toDouble();
  mMode = static_cast< Format >( itemElem.attribute( QStringLiteral( "mode" ), QString::number( FormatUnknown ) ).toInt() );

  const QDomNodeList composerItemList = itemElem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  if ( !composerItemList.isEmpty() )
  {
    const QDomElement composerItemElem = composerItemList.at( 0 ).toElement();

    if ( !qgsDoubleNear( composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
    {
      //in versions prior to 2.1 picture rotation was stored in the rotation attribute
      mPictureRotation = composerItemElem.attribute( QStringLiteral( "rotation" ), QStringLiteral( "0" ) ).toDouble();
    }
  }

  mDefaultSvgSize = QSize( 0, 0 );

  if ( itemElem.hasAttribute( QStringLiteral( "sourceExpression" ) ) )
  {
    //update pre 2.5 picture expression to use data defined expression
    const QString sourceExpression = itemElem.attribute( QStringLiteral( "sourceExpression" ), QString() );
    const QString useExpression = itemElem.attribute( QStringLiteral( "useExpression" ) );
    bool expressionActive;
    expressionActive = ( useExpression.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 );

    mDataDefinedProperties.setProperty( QgsLayoutObject::PictureSource, QgsProperty::fromExpression( sourceExpression, expressionActive ) );
  }

  QString imagePath = itemElem.attribute( QStringLiteral( "file" ) );

  // convert from relative path to absolute. For SVG we also need to consider system SVG paths
  const QgsPathResolver pathResolver = context.pathResolver();
  if ( imagePath.endsWith( QLatin1String( ".svg" ), Qt::CaseInsensitive ) )
    imagePath = QgsSymbolLayerUtils::svgSymbolNameToPath( imagePath, pathResolver );
  else
    imagePath = pathResolver.readPath( imagePath );

  mSourcePath = imagePath;

  //picture rotation
  if ( !qgsDoubleNear( itemElem.attribute( QStringLiteral( "pictureRotation" ), QStringLiteral( "0" ) ).toDouble(), 0.0 ) )
  {
    mPictureRotation = itemElem.attribute( QStringLiteral( "pictureRotation" ), QStringLiteral( "0" ) ).toDouble();
  }

  //rotation map
  mNorthArrowHandler->setNorthMode( static_cast< QgsLayoutNorthArrowHandler::NorthMode >( itemElem.attribute( QStringLiteral( "northMode" ), QStringLiteral( "0" ) ).toInt() ) );
  mNorthArrowHandler->setNorthOffset( itemElem.attribute( QStringLiteral( "northOffset" ), QStringLiteral( "0" ) ).toDouble() );

  mNorthArrowHandler->setLinkedMap( nullptr );
  mRotationMapUuid = itemElem.attribute( QStringLiteral( "mapUuid" ) );

  return true;
}

QgsLayoutItemMap *QgsLayoutItemPicture::linkedMap() const
{
  return mNorthArrowHandler->linkedMap();
}

QgsLayoutItemPicture::NorthMode QgsLayoutItemPicture::northMode() const
{
  return static_cast< QgsLayoutItemPicture::NorthMode >( mNorthArrowHandler->northMode() );
}

void QgsLayoutItemPicture::setNorthMode( QgsLayoutItemPicture::NorthMode mode )
{
  mNorthArrowHandler->setNorthMode( static_cast< QgsLayoutNorthArrowHandler::NorthMode >( mode ) );
}

double QgsLayoutItemPicture::northOffset() const
{
  return mNorthArrowHandler->northOffset();
}

void QgsLayoutItemPicture::setNorthOffset( double offset )
{
  mNorthArrowHandler->setNorthOffset( offset );
}

void QgsLayoutItemPicture::setPictureAnchor( ReferencePoint anchor )
{
  mPictureAnchor = anchor;
  update();
}

void QgsLayoutItemPicture::setSvgFillColor( const QColor &color )
{
  mSvgFillColor = color;
  refreshPicture();
}

void QgsLayoutItemPicture::setSvgStrokeColor( const QColor &color )
{
  mSvgStrokeColor = color;
  refreshPicture();
}

void QgsLayoutItemPicture::setSvgStrokeWidth( double width )
{
  mSvgStrokeWidth = width;
  refreshPicture();
}

void QgsLayoutItemPicture::setMode( QgsLayoutItemPicture::Format mode )
{
  if ( mMode == mode )
    return;

  mMode = mode;
  refreshPicture();
}

void QgsLayoutItemPicture::finalizeRestoreFromXml()
{
  if ( !mLayout || mRotationMapUuid.isEmpty() )
  {
    mNorthArrowHandler->setLinkedMap( nullptr );
  }
  else
  {
    mNorthArrowHandler->setLinkedMap( qobject_cast< QgsLayoutItemMap * >( mLayout->itemByUuid( mRotationMapUuid, true ) ) );
  }

  refreshPicture();
}
