/***************************************************************************
  qgsdecorationtitle.cpp
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationtitle.h"
#include "qgsdecorationtitledialog.h"

#include "qgisapp.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"
#include "qgstextrenderer.h"

#include <QPainter>
#include <QMenu>
#include <QDate>
#include <QDomDocument>
#include <QFile>

//non qt includes
#include <cmath>


QgsDecorationTitle::QgsDecorationTitle( QObject *parent )
  : QgsDecorationItem( parent )
{
  mPlacement = TopCenter;
  mMarginUnit = QgsUnitTypes::RenderMillimeters;

  setDisplayName( tr( "Title Label" ) );
  mConfigurationName = QStringLiteral( "TitleLabel" );

  projectRead();
}

void QgsDecorationTitle::projectRead()
{
  QgsDecorationItem::projectRead();

  mLabelText = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/Label" ), QString() );
  mBackgroundColor = QgsSymbolLayerUtils::decodeColor( QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/BackgroundColor" ), QStringLiteral( "0,0,0,99" ) ) );

  mMarginHorizontal = QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/MarginH" ), 0 );
  mMarginVertical = QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/MarginV" ), 0 );

  QDomDocument doc;
  QDomElement elem;
  const QString textXml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/Font" ) );
  if ( !textXml.isEmpty() )
  {
    doc.setContent( textXml );
    elem = doc.documentElement();
    QgsReadWriteContext rwContext;
    rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
    mTextFormat.readXml( elem, rwContext );
  }
}

void QgsDecorationTitle::saveToProject()
{
  QgsDecorationItem::saveToProject();

  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Label" ), mLabelText );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/BackgroundColor" ), QgsSymbolLayerUtils::encodeColor( mBackgroundColor ) );

  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/MarginH" ), mMarginHorizontal );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/MarginV" ), mMarginVertical );

  QDomDocument textDoc;
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
  const QDomElement textElem = mTextFormat.writeXml( textDoc, rwContext );
  textDoc.appendChild( textElem );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Font" ), textDoc.toString() );
}

// Slot called when the buffer menu item is activated
void QgsDecorationTitle::run()
{
  QgsDecorationTitleDialog dlg( *this, QgisApp::instance() );
  dlg.exec();
}


void QgsDecorationTitle::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{
  Q_UNUSED( mapSettings )
  if ( !enabled() )
    return;

  const QgsScopedQPainterState painterState( context.painter() );
  context.setPainterFlagsUsingContext();

  const QString displayString = QgsExpression::replaceExpressionText( mLabelText, &context.expressionContext() );
  const QStringList displayStringList = displayString.split( '\n' );

  const QFontMetricsF textMetrics = QgsTextRenderer::fontMetrics( context, mTextFormat );
  const double textDescent = textMetrics.descent();
  const double textWidth = QgsTextRenderer::textWidth( context, mTextFormat, displayStringList );
  const double textHeight = QgsTextRenderer::textHeight( context, mTextFormat, displayStringList, Qgis::TextLayoutMode::Point );

  QPaintDevice *device = context.painter()->device();
  const int deviceHeight = device->height() / device->devicePixelRatioF();
  const int deviceWidth = device->width() / device->devicePixelRatioF();

  float xOffset( 0 ), yOffset( 0 );

  // Set  margin according to selected units
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
    {
      xOffset = mMarginHorizontal;
      yOffset = mMarginVertical;
      break;
    }
    case QgsUnitTypes::RenderPercentage:
    {
      xOffset = ( ( deviceWidth - textWidth ) / 100. ) * mMarginHorizontal;
      yOffset = ( ( deviceHeight - textHeight ) / 100. ) * mMarginVertical;
      break;
    }
    case QgsUnitTypes::RenderMapUnits:
    case QgsUnitTypes::RenderPoints:
    case QgsUnitTypes::RenderInches:
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderMetersInMapUnits:
      break;
  }

  QPolygonF backgroundBar;

  // Determine placement of label from form combo box
  Qgis::TextHorizontalAlignment horizontalAlignment = Qgis::TextHorizontalAlignment::Left;
  switch ( mPlacement )
  {
    case BottomLeft: // Bottom Left, xOffset is set above
      backgroundBar << QPointF( 0, deviceHeight )
                    << QPointF( deviceWidth, deviceHeight )
                    << QPointF( deviceWidth, deviceHeight - ( yOffset * 2 + textHeight ) )
                    << QPointF( 0, deviceHeight - ( yOffset * 2 + textHeight ) );
      yOffset = deviceHeight - yOffset - textDescent;
      break;
    case TopLeft: // Top left, xOffset is set above
      backgroundBar << QPointF( 0, 0 )
                    << QPointF( deviceWidth, 0 )
                    << QPointF( deviceWidth, yOffset * 2 + textHeight )
                    << QPointF( 0, yOffset * 2 + textHeight );
      yOffset = yOffset + textHeight - textDescent;
      break;
    case TopRight: // Top Right
      backgroundBar << QPointF( 0, 0 )
                    << QPointF( deviceWidth, 0 )
                    << QPointF( deviceWidth, yOffset * 2 + textHeight )
                    << QPointF( 0, yOffset * 2 + textHeight );
      yOffset = yOffset + textHeight - textDescent;
      xOffset = deviceWidth - xOffset;
      horizontalAlignment = Qgis::TextHorizontalAlignment::Right;
      break;
    case BottomRight: // Bottom Right
      backgroundBar << QPointF( 0, deviceHeight )
                    << QPointF( deviceWidth, deviceHeight )
                    << QPointF( deviceWidth, deviceHeight - ( yOffset * 2 + textHeight ) )
                    << QPointF( 0, deviceHeight - ( yOffset * 2 + textHeight ) );
      yOffset = deviceHeight - yOffset - textDescent;
      xOffset = deviceWidth - xOffset;
      horizontalAlignment = Qgis::TextHorizontalAlignment::Right;
      break;
    case TopCenter: // Top Center
      backgroundBar << QPointF( 0, 0 )
                    << QPointF( deviceWidth, 0 )
                    << QPointF( deviceWidth, yOffset * 2 + textHeight )
                    << QPointF( 0, yOffset * 2 + textHeight );
      yOffset = yOffset + textHeight - textDescent;
      xOffset = deviceWidth / 2 + xOffset;
      horizontalAlignment = Qgis::TextHorizontalAlignment::Center;
      break;
    case BottomCenter: // Bottom Center
      backgroundBar << QPointF( 0, deviceHeight )
                    << QPointF( deviceWidth, deviceHeight )
                    << QPointF( deviceWidth, deviceHeight - ( yOffset * 2 + textHeight ) )
                    << QPointF( 0, deviceHeight - ( yOffset * 2 + textHeight ) );
      yOffset = deviceHeight - yOffset - textDescent;
      xOffset = deviceWidth / 2 + xOffset;
      horizontalAlignment = Qgis::TextHorizontalAlignment::Center;
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported placement index of %1" ).arg( static_cast<int>( mPlacement ) ) );
  }

  // Draw background bar
  context.painter()->setPen( Qt::NoPen );
  context.painter()->setBrush( QBrush( mBackgroundColor, Qt::SolidPattern ) );
  context.painter()->drawPolygon( backgroundBar );

  // Paint label to canvas
  QgsTextRenderer::drawText( QPointF( xOffset, yOffset ), 0.0, horizontalAlignment, displayStringList, context, mTextFormat );
}

