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

#include "qgsdecorationnortharrowdialog.h"

#include "qgisapp.h"
#include "qgsbearingutils.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgssvgcache.h"

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


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsDecorationNorthArrow::QgsDecorationNorthArrow( QObject *parent )
  : QgsDecorationItem( parent )
{
  mPlacement = BottomLeft;
  mMarginUnit = QgsUnitTypes::RenderMillimeters;

  setName( "North Arrow" );
  projectRead();
}

void QgsDecorationNorthArrow::projectRead()
{
  QgsDecorationItem::projectRead();
  mColor = QgsSymbolLayerUtils::decodeColor( QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/Color" ), QStringLiteral( "#000000" ) ) );
  mOutlineColor = QgsSymbolLayerUtils::decodeColor( QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/OutlineColor" ), QStringLiteral( "#FFFFFF" ) ) );
  mRotationInt = QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/Rotation" ), 0 );
  mAutomatic = QgsProject::instance()->readBoolEntry( mNameConfig, QStringLiteral( "/Automatic" ), true );
  mMarginHorizontal = QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/MarginH" ), 0 );
  mMarginVertical = QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/MarginV" ), 0 );
}

void QgsDecorationNorthArrow::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/OutlineColor" ), QgsSymbolLayerUtils::encodeColor( mOutlineColor ) );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Rotation" ), mRotationInt );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Automatic" ), mAutomatic );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/MarginH" ), mMarginHorizontal );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/MarginV" ), mMarginVertical );
}

// Slot called when the buffer menu item is activated
void QgsDecorationNorthArrow::run()
{
  QgsDecorationNorthArrowDialog dlg( *this, QgisApp::instance() );
  dlg.exec();
}

void QgsDecorationNorthArrow::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{

  //Large IF statement controlled by enable checkbox
  if ( enabled() )
  {
    QSize size( 64, 64 );
    QSvgRenderer svg;

    const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( QStringLiteral( ":/images/north_arrows/default.svg" ), size.width(), mColor, mOutlineColor, 1.0, 1.0 );
    svg.load( svgContent );

    if ( svg.isValid() )
    {
      double centerXDouble = size.width() / 2.0;
      double centerYDouble = size.width() / 2.0;

      //save the current canvas rotation
      context.painter()->save();
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
          mRotationInt = QgsBearingUtils:: bearingTrueNorth( mapSettings.destinationCrs(), context.extent().center() );
        }
        catch ( QgsException & )
        {
          mRotationInt = 0.0;
          //QgsDebugMsg( "Can not get direction to true north. Probably project CRS is not defined." );
        }
        mRotationInt += mapSettings.rotation();
      }

      double myRadiansDouble = mRotationInt * M_PI / 180.0;
      int xShift = static_cast<int>( (
                                       ( centerXDouble * std::cos( myRadiansDouble ) ) +
                                       ( centerYDouble * std::sin( myRadiansDouble ) )
                                     ) - centerXDouble );
      int yShift = static_cast<int>( (
                                       ( -centerXDouble * std::sin( myRadiansDouble ) ) +
                                       ( centerYDouble * std::cos( myRadiansDouble ) )
                                     ) - centerYDouble );

      // need width/height of paint device
      int myHeight = context.painter()->device()->height();
      int myWidth = context.painter()->device()->width();

      //QgsDebugMsg("Rendering north arrow at " + mPlacementLabels.at(mPlacementIndex));

      // Set  margin according to selected units
      int myXOffset = 0;
      int myYOffset = 0;
      switch ( mMarginUnit )
      {
        case QgsUnitTypes::RenderMillimeters:
        {
          int myPixelsInchX = context.painter()->device()->logicalDpiX();
          int myPixelsInchY = context.painter()->device()->logicalDpiY();
          myXOffset = myPixelsInchX * INCHES_TO_MM * mMarginHorizontal;
          myYOffset = myPixelsInchY * INCHES_TO_MM * mMarginVertical;
          break;
        }

        case QgsUnitTypes::RenderPixels:
          myXOffset = mMarginHorizontal - 5; // Minus 5 to shift tight into corner
          myYOffset = mMarginVertical - 5;
          break;

        case QgsUnitTypes::RenderPercentage:
          myXOffset = ( ( myWidth - size.width() ) / 100. ) * mMarginHorizontal;
          myYOffset = ( ( myHeight - size.width() ) / 100. ) * mMarginVertical;
          break;

        default:  // Use default of top left
          break;
      }
      //Determine placement of label from form combo box
      switch ( mPlacement )
      {
        case BottomLeft:
          context.painter()->translate( myXOffset, myHeight - myYOffset - size.width() );
          break;
        case TopLeft:
          context.painter()->translate( myXOffset, myYOffset );
          break;
        case TopRight:
          context.painter()->translate( myWidth - myXOffset - size.width(), myYOffset );
          break;
        case BottomRight:
          context.painter()->translate( myWidth - myXOffset - size.width(),
                                        myHeight - myYOffset - size.width() );
          break;
        default:
        {
          //QgsDebugMsg("Unable to determine where to put north arrow so defaulting to top left");
        }
      }

      //rotate the canvas by the north arrow rotation amount
      context.painter()->rotate( mRotationInt );
      //Now we can actually do the drawing, and draw a smooth north arrow even when rotated

      context.painter()->translate( xShift, yShift );
      svg.render( context.painter(), QRectF( 0, 0, size.width(), size.height() ) );

      //unrotate the canvas again
      context.painter()->restore();
    }
    else
    {
      QFont myQFont( QStringLiteral( "time" ), 12, QFont::Bold );
      context.painter()->setFont( myQFont );
      context.painter()->setPen( Qt::black );
      context.painter()->drawText( 10, 20, tr( "North arrow pixmap not found" ) );
    }
  }
}
