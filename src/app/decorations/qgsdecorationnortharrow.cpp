/***************************************************************************
  plugin.cpp
  Import tool for various worldmap analysis output files
Functions:

-------------------
begin                : Jan 21, 2004
copyright            : (C) 2004 by Tim Sutton
email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// includes
#include "qgsdecorationnortharrow.h"

#include "qgisapp.h"
#include "qgsbearingutils.h"
#include "qgscolorutils.h"
#include "qgsdecorationnortharrowdialog.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgsmapsettings.h"
#include "qgsproject.h"
#include "qgssvgcache.h"
#include "qgssymbollayerutils.h"

#include "moc_qgsdecorationnortharrow.cpp"

// qt includes
#include <QPainter>
#include <QMenu>
#include <QDir>
#include <QFile>
#include <QSvgRenderer>

//non qt includes
#include <cmath>
#include <cassert>

//  const double QgsNorthArrowPlugin::DEG2RAD = 0.0174532925199433;
const double QgsDecorationNorthArrow::TOL = 1e-8;


QgsDecorationNorthArrow::QgsDecorationNorthArrow( QObject *parent )
  : QgsDecorationItem( parent )
{
  mPlacement = BottomLeft;
  mMarginUnit = Qgis::RenderUnit::Millimeters;

  setDisplayName( tr( "North Arrow" ) );
  mConfigurationName = u"NorthArrow"_s;

  projectRead();
}

void QgsDecorationNorthArrow::projectRead()
{
  QgsDecorationItem::projectRead();
  mColor = QgsColorUtils::colorFromString( QgsProject::instance()->readEntry( mConfigurationName, u"/Color"_s, u"#000000"_s ) );
  mOutlineColor = QgsColorUtils::colorFromString( QgsProject::instance()->readEntry( mConfigurationName, u"/OutlineColor"_s, u"#FFFFFF"_s ) );
  mSize = QgsProject::instance()->readDoubleEntry( mConfigurationName, u"/Size"_s, 16.0 );
  mSvgPath = QgsProject::instance()->readEntry( mConfigurationName, u"/SvgPath"_s, QString() );
  mRotationInt = QgsProject::instance()->readNumEntry( mConfigurationName, u"/Rotation"_s, 0 );
  mAutomatic = QgsProject::instance()->readBoolEntry( mConfigurationName, u"/Automatic"_s, true );
  mMarginHorizontal = QgsProject::instance()->readNumEntry( mConfigurationName, u"/MarginH"_s, 0 );
  mMarginVertical = QgsProject::instance()->readNumEntry( mConfigurationName, u"/MarginV"_s, 0 );
}

void QgsDecorationNorthArrow::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mConfigurationName, u"/Color"_s, QgsColorUtils::colorToString( mColor ) );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/OutlineColor"_s, QgsColorUtils::colorToString( mOutlineColor ) );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/Size"_s, mSize );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/SvgPath"_s, mSvgPath );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/Rotation"_s, mRotationInt );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/Automatic"_s, mAutomatic );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/MarginH"_s, mMarginHorizontal );
  QgsProject::instance()->writeEntry( mConfigurationName, u"/MarginV"_s, mMarginVertical );
}

// Slot called when the buffer menu item is activated
void QgsDecorationNorthArrow::run()
{
  QgsDecorationNorthArrowDialog dlg( *this, QgisApp::instance() );
  dlg.exec();
}

QString QgsDecorationNorthArrow::svgPath()
{
  if ( mSvgPath.startsWith( "base64:"_L1, Qt::CaseInsensitive ) )
    return mSvgPath;

  if ( !mSvgPath.isEmpty() )
  {
    QString resolvedPath = QgsSymbolLayerUtils::svgSymbolNameToPath( mSvgPath, QgsProject::instance()->pathResolver() );
    const bool validSvg = QFileInfo::exists( resolvedPath );
    if ( validSvg )
    {
      return resolvedPath;
    }
  }

  return u":/images/north_arrows/default.svg"_s;
}

void QgsDecorationNorthArrow::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{
  if ( !enabled() )
    return;

  const double maxLength = mSize * mapSettings.outputDpi() / 25.4;
  QSvgRenderer svg;

  const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( svgPath(), maxLength, mColor, mOutlineColor, 1.0, 1.0 );
  svg.load( svgContent );

  if ( svg.isValid() )
  {
    QSize size( maxLength, maxLength );
    const QRectF viewBox = svg.viewBoxF();
    if ( viewBox.height() > viewBox.width() )
    {
      size.setWidth( maxLength * viewBox.width() / viewBox.height() );
    }
    else
    {
      size.setHeight( maxLength * viewBox.height() / viewBox.width() );
    }

    const double centerXDouble = size.width() / 2.0;
    const double centerYDouble = size.height() / 2.0;

    //save the current canvas rotation
    const QgsScopedQPainterState painterState( context.painter() );
    //
    //work out how to shift the image so that it rotates
    //           properly about its center
    //(x cos a + y sin a - x, -x sin a + y cos a - y)
    //
    // could move this call to somewhere else so that it is only
    // called when the projection or map extent changes
    if ( mAutomatic )
    {
      try
      {
        mRotationInt = QgsBearingUtils::bearingTrueNorth( mapSettings.destinationCrs(), mapSettings.transformContext(), context.extent().center() );
      }
      catch ( QgsException & )
      {
        mRotationInt = 0.0;
      }
      mRotationInt += mapSettings.rotation();
    }

    const double radiansDouble = mRotationInt * M_PI / 180.0;
    const int xShift = static_cast<int>( ( ( centerXDouble * std::cos( radiansDouble ) ) + ( centerYDouble * std::sin( radiansDouble ) ) ) - centerXDouble );
    const int yShift = static_cast<int>( ( ( -centerXDouble * std::sin( radiansDouble ) ) + ( centerYDouble * std::cos( radiansDouble ) ) ) - centerYDouble );
    // need width/height of paint device
    QPaintDevice *device = context.painter()->device();
    const float deviceHeight = static_cast<float>( device->height() ) / context.devicePixelRatio();
    const float deviceWidth = static_cast<float>( device->width() ) / context.devicePixelRatio();

    // Set  margin according to selected units
    int xOffset = 0;
    int yOffset = 0;
    switch ( mMarginUnit )
    {
      case Qgis::RenderUnit::Millimeters:
      {
        const int pixelsInchX = context.painter()->device()->logicalDpiX();
        const int pixelsInchY = context.painter()->device()->logicalDpiY();
        xOffset = pixelsInchX * INCHES_TO_MM * mMarginHorizontal;
        yOffset = pixelsInchY * INCHES_TO_MM * mMarginVertical;
        break;
      }

      case Qgis::RenderUnit::Pixels:
        xOffset = mMarginHorizontal - 5; // Minus 5 to shift tight into corner
        yOffset = mMarginVertical - 5;
        break;

      case Qgis::RenderUnit::Percentage:
        xOffset = ( ( deviceWidth - size.width() ) / 100. ) * mMarginHorizontal;
        yOffset = ( ( deviceHeight - size.width() ) / 100. ) * mMarginVertical;
        break;
      case Qgis::RenderUnit::MapUnits:
      case Qgis::RenderUnit::Points:
      case Qgis::RenderUnit::Inches:
      case Qgis::RenderUnit::Unknown:
      case Qgis::RenderUnit::MetersInMapUnits:
        break;
    }
    //Determine placement of label from form combo box
    switch ( mPlacement )
    {
      case BottomLeft:
        context.painter()->translate( xOffset, deviceHeight - yOffset - maxLength + ( maxLength - size.height() ) / 2 );
        break;
      case TopLeft:
        context.painter()->translate( xOffset, yOffset );
        break;
      case TopRight:
        context.painter()->translate( deviceWidth - xOffset - maxLength + ( maxLength - size.width() ) / 2, yOffset );
        break;
      case BottomRight:
        context.painter()->translate( deviceWidth - xOffset - maxLength + ( maxLength - size.width() ) / 2, deviceHeight - yOffset - maxLength + ( maxLength - size.height() ) / 2 );
        break;
      case TopCenter:
        context.painter()->translate( deviceWidth / 2 - size.width() / 2 + xOffset, yOffset );
        break;
      case BottomCenter:
        context.painter()->translate( deviceWidth / 2 - size.width() / 2 + xOffset, deviceHeight - yOffset - size.height() );
        break;
      default:
        QgsDebugError( u"Unsupported placement index of %1"_s.arg( static_cast<int>( mPlacement ) ) );
    }

    //rotate the canvas by the north arrow rotation amount
    context.painter()->rotate( mRotationInt );
    //Now we can actually do the drawing, and draw a smooth north arrow even when rotated
    context.painter()->translate( xShift, yShift );
    svg.render( context.painter(), QRectF( 0, 0, size.width(), size.height() ) );

    //unrotate the canvas again
  }
}
