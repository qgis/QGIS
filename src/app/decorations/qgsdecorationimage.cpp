/***************************************************************************
  qgsdecorationimage.cpp
  --------------------------------------
  Date                 : August 2019
  Copyright            : (C) 2019 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationimage.h"
#include "qgsdecorationimagedialog.h"

#include "qgisapp.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgsimagecache.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgssvgcache.h"
#include "qgsmapsettings.h"

#include <QPainter>
#include <QMenu>
#include <QDir>
#include <QFile>
#include <QSvgRenderer>

#include <cmath>
#include <cassert>


QgsDecorationImage::QgsDecorationImage( QObject *parent )
  : QgsDecorationItem( parent )
{
  mPlacement = BottomLeft;
  mMarginUnit = QgsUnitTypes::RenderMillimeters;

  setDisplayName( tr( "Image" ) );
  mConfigurationName = QStringLiteral( "Image" );

  projectRead();
}

void QgsDecorationImage::projectRead()
{
  QgsDecorationItem::projectRead();
  mColor = QgsSymbolLayerUtils::decodeColor( QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/Color" ), QStringLiteral( "#000000" ) ) );
  mOutlineColor = QgsSymbolLayerUtils::decodeColor( QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/OutlineColor" ), QStringLiteral( "#FFFFFF" ) ) );
  mSize = QgsProject::instance()->readDoubleEntry( mConfigurationName, QStringLiteral( "/Size" ), 16.0 );
  setImagePath( QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/ImagePath" ), QString() ) );
  mMarginHorizontal = QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/MarginH" ), 0 );
  mMarginVertical = QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/MarginV" ), 0 );
}

void QgsDecorationImage::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/OutlineColor" ), QgsSymbolLayerUtils::encodeColor( mOutlineColor ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Size" ), mSize );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/ImagePath" ), QgsProject::instance()->pathResolver().writePath( mImagePath ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/MarginH" ), mMarginHorizontal );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/MarginV" ), mMarginVertical );
}

// Slot called when the buffer menu item is activated
void QgsDecorationImage::run()
{
  QgsDecorationImageDialog dlg( *this, QgisApp::instance() );
  dlg.exec();
}

void QgsDecorationImage::setImagePath( const QString &imagePath )
{
  mImagePath = imagePath;
  mImageFormat = FormatUnknown;

  const QString resolvedPath = QgsSymbolLayerUtils::svgSymbolNameToPath( mImagePath, QgsProject::instance()->pathResolver() );
  const QFileInfo fileInfo( resolvedPath );
  if ( fileInfo.exists() )
  {
    const QString suffix = fileInfo.suffix();
    if ( suffix.compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
    {
      mImageFormat = FormatSVG;
    }
    else
    {
      mImageFormat = FormatRaster;
    }
  }
  else
  {
    QSvgRenderer svg;
    const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( mImagePath, 1.0, mColor, mOutlineColor, 1.0, 1.0 );
    svg.load( svgContent );

    if ( svg.isValid() )
    {
      mImageFormat = FormatSVG;
      return;
    }
    bool cached;
    const QImage img = QgsApplication::imageCache()->pathAsImage( mImagePath, QSize( 1, 0 ), true, 1.0, cached );
    if ( !img.isNull() )
    {
      mImageFormat = FormatRaster;
    }
  }
}

QString QgsDecorationImage::imagePath()
{
  if ( !mImagePath.isEmpty() )
  {
    QString resolvedPath = QgsSymbolLayerUtils::svgSymbolNameToPath( mImagePath, QgsProject::instance()->pathResolver() );
    const bool validSvg = QFileInfo::exists( resolvedPath );
    if ( validSvg )
    {
      return resolvedPath;
    }
  }

  return QStringLiteral( ":/images/icons/qgis-icon-minimal-black.svg" );
}

void QgsDecorationImage::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{
  if ( !enabled() )
    return;

  const double maxLength = mSize * mapSettings.outputDpi() / 25.4;
  QSize size( maxLength, maxLength );

  QSvgRenderer svg;
  QImage img;
  switch ( mImageFormat )
  {
    case FormatSVG:
    {
      const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( imagePath(), maxLength, mColor, mOutlineColor, 1.0, 1.0 );
      svg.load( svgContent );

      if ( svg.isValid() )
      {
        const QRectF viewBox = svg.viewBoxF();
        if ( viewBox.height() > viewBox.width() )
        {
          size.setWidth( maxLength * viewBox.width() / viewBox.height() );
        }
        else
        {
          size.setHeight( maxLength * viewBox.height() / viewBox.width() );
        }
      }
      else
      {
        // SVG can't be parsed
        return;
      }
      break;
    }

    case FormatRaster:
    {
      const QSize originalSize = QgsApplication::imageCache()->originalSize( imagePath() );
      if ( originalSize.isValid() )
      {
        if ( originalSize.height() > originalSize.width() )
        {
          size.setWidth( originalSize.width() * maxLength / originalSize.height() );
          size.setHeight( maxLength );
        }
        else
        {
          size.setWidth( maxLength );
          size.setHeight( originalSize.height() * maxLength / originalSize.width() );
        }
        bool cached;
        img = QgsApplication::imageCache()->pathAsImage( imagePath(), size, true, 1.0, cached );
      }
      else
      {
        // Image can't be read
        return;
      }
      break;
    }

    case FormatUnknown:
      // Broken / unavailable image
      return;
  }

  const QgsScopedQPainterState painterState( context.painter() );

  // need width/height of paint device
  QPaintDevice *device = context.painter()->device();
  const int deviceHeight = device->height() / device->devicePixelRatioF();
  const int deviceWidth = device->width() / device->devicePixelRatioF();

  // Set  margin according to selected units
  int xOffset = 0;
  int yOffset = 0;
  switch ( mMarginUnit )
  {
    case QgsUnitTypes::RenderMillimeters:
    {
      const int pixelsInchX = context.painter()->device()->logicalDpiX();
      const int pixelsInchY = context.painter()->device()->logicalDpiY();
      xOffset = pixelsInchX * INCHES_TO_MM * mMarginHorizontal;
      yOffset = pixelsInchY * INCHES_TO_MM * mMarginVertical;
      break;
    }

    case QgsUnitTypes::RenderPixels:
      xOffset = mMarginHorizontal - 5; // Minus 5 to shift tight into corner
      yOffset = mMarginVertical - 5;
      break;

    case QgsUnitTypes::RenderPercentage:
      xOffset = ( ( deviceWidth - size.width() ) / 100. ) * mMarginHorizontal;
      yOffset = ( ( deviceHeight - size.width() ) / 100. ) * mMarginVertical;
      break;
    case QgsUnitTypes::RenderMapUnits:
    case QgsUnitTypes::RenderPoints:
    case QgsUnitTypes::RenderInches:
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderMetersInMapUnits:
      break;
  }

  //Determine placement of label from form combo box
  switch ( mPlacement )
  {
    case BottomLeft:
      context.painter()->translate( xOffset, deviceHeight - yOffset - size.height() );
      break;
    case TopLeft:
      context.painter()->translate( xOffset, yOffset );
      break;
    case TopRight:
      context.painter()->translate( deviceWidth - xOffset - size.width(), yOffset );
      break;
    case BottomRight:
      context.painter()->translate( deviceWidth - xOffset - size.width(),
                                    deviceHeight - yOffset - size.height() );
      break;
    case TopCenter:
      context.painter()->translate( deviceWidth / 2 - size.width() / 2 + xOffset, yOffset );
      break;
    case BottomCenter:
      context.painter()->translate( deviceWidth / 2 - size.width() / 2 + xOffset,
                                    deviceHeight - yOffset - size.height() );
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported placement index of %1" ).arg( static_cast<int>( mPlacement ) ) );
  }

  switch ( mImageFormat )
  {
    case FormatSVG:
      svg.render( context.painter(), QRectF( 0, 0, size.width(), size.height() ) );
      break;
    case FormatRaster:
      context.painter()->drawImage( QRectF( 0, 0, size.width(), size.height() ), img );
      break;
    case FormatUnknown:
      // nothing happening here, function already returned in the first switch
      break;
  }
}
