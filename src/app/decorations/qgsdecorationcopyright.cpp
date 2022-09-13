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

#include "qgsdecorationcopyright.h"
#include "qgsdecorationcopyrightdialog.h"

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


QgsDecorationCopyright::QgsDecorationCopyright( QObject *parent )
  : QgsDecorationItem( parent )
{
  mPlacement = BottomRight;
  mMarginUnit = QgsUnitTypes::RenderMillimeters;

  setDisplayName( tr( "Copyright Label" ) );
  mConfigurationName = QStringLiteral( "CopyrightLabel" );

  projectRead();
}

void QgsDecorationCopyright::projectRead()
{
  QgsDecorationItem::projectRead();

  mLabelText = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/Label" ), QString() );
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

  // Migration for pre QGIS 3.2 settings
  const QColor oldColor = QgsSymbolLayerUtils::decodeColor( QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/Color" ) ) );
  if ( oldColor.isValid() )
  {
    mTextFormat.setColor( oldColor );
    QgsProject::instance()->removeEntry( mConfigurationName, QStringLiteral( "/Color" ) );
  }
}

void QgsDecorationCopyright::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Label" ), mLabelText );
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
void QgsDecorationCopyright::run()
{
  QgsDecorationCopyrightDialog dlg( *this, QgisApp::instance() );
  dlg.exec();
}


void QgsDecorationCopyright::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
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

  // Determine placement of label from form combo box
  Qgis::TextHorizontalAlignment horizontalAlignment = Qgis::TextHorizontalAlignment::Left;
  switch ( mPlacement )
  {
    case BottomLeft: // Bottom Left, xOffset is set above
      yOffset = deviceHeight - yOffset - textDescent;
      break;
    case TopLeft: // Top left, xOffset is set above
      yOffset = yOffset + textHeight - textDescent;
      break;
    case TopRight: // Top Right
      yOffset = yOffset + textHeight - textDescent;
      xOffset = deviceWidth - xOffset;
      horizontalAlignment = Qgis::TextHorizontalAlignment::Right;
      break;
    case BottomRight: // Bottom Right
      yOffset = deviceHeight - yOffset - textDescent;
      xOffset = deviceWidth - xOffset;
      horizontalAlignment = Qgis::TextHorizontalAlignment::Right;
      break;
    case TopCenter: // Top Center
      yOffset = yOffset + textHeight - textDescent;
      xOffset = deviceWidth / 2 + xOffset;
      horizontalAlignment = Qgis::TextHorizontalAlignment::Center;
      break;
    case BottomCenter: // Bottom Center
      yOffset = deviceHeight - yOffset - textDescent;
      xOffset = deviceWidth / 2 + xOffset;
      horizontalAlignment = Qgis::TextHorizontalAlignment::Center;
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported placement index of %1" ).arg( static_cast<int>( mPlacement ) ) );
  }

  //Paint label to canvas
  QgsTextRenderer::drawText( QPointF( xOffset, yOffset ), 0.0, horizontalAlignment, displayStringList, context, mTextFormat );
}

