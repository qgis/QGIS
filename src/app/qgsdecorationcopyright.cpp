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
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"

#include <QPainter>
#include <QMenu>
#include <QDate>
#include <QTextDocument>
#include <QMatrix>
#include <QFile>

//non qt includes
#include <cmath>


QgsDecorationCopyright::QgsDecorationCopyright( QObject *parent )
  : QgsDecorationItem( parent )
{
  mPlacement = BottomRight;
  mMarginUnit = QgsUnitTypes::RenderMillimeters;

  setName( "Copyright Label" );
  // initialize default values in the gui
  projectRead();
}

void QgsDecorationCopyright::projectRead()
{
  QgsDecorationItem::projectRead();

  QDate now = QDate::currentDate();
  QString defString = "&copy; QGIS " + now.toString( QStringLiteral( "yyyy" ) );

  // there is no font setting in the UI, so just use the Qt/QGIS default font (what mQFont gets when created)
  //  mQFont.setFamily( QgsProject::instance()->readEntry( "CopyrightLabel", "/FontName", "Sans Serif" ) );
  //  mQFont.setPointSize( QgsProject::instance()->readNumEntry( "CopyrightLabel", "/FontSize", 9 ) );

  mLabelQString = QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/Label" ), defString );
  mMarginHorizontal = QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/MarginH" ), 0 );
  mMarginVertical = QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/MarginV" ), 0 );
  mColor = QgsSymbolLayerUtils::decodeColor( QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/Color" ), QStringLiteral( "#000000" ) ) );
}

void QgsDecorationCopyright::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/FontName" ), mQFont.family() );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/FontSize" ), mQFont.pointSize() );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Label" ), mLabelQString );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/MarginH" ), mMarginHorizontal );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/MarginV" ), mMarginVertical );
}

// Slot called when the buffer menu item is activated
void QgsDecorationCopyright::run()
{
  QgsDecorationCopyrightDialog dlg( *this, QgisApp::instance() );
  dlg.exec();
}


void QgsDecorationCopyright::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{
  Q_UNUSED( mapSettings );
  //Large IF statement to enable/disable copyright label
  if ( enabled() )
  {
    // need width/height of paint device
    int myHeight = context.painter()->device()->height();
    int myWidth = context.painter()->device()->width();

    QTextDocument text;
    text.setDefaultFont( mQFont );
    // To set the text color in a QTextDocument we use a CSS style

    QString style = "<style type=\"text/css\"> p {color: " +
                    QStringLiteral( "rgba( %1, %2, %3, %4 )" ).arg( mColor.red() ).arg( mColor.green() ).arg( mColor.blue() ).arg( QString::number( mColor.alphaF(), 'f', 2 ) ) + "}</style>";
    text.setHtml( style + "<p>" + mLabelQString + "</p>" );
    QSizeF size = text.size();

    float myXOffset( 0 ), myYOffset( 0 );

    // Set  margin according to selected units
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
        myXOffset = mMarginHorizontal;
        myYOffset = mMarginVertical;
        break;

      case QgsUnitTypes::RenderPercentage:
        myXOffset = ( ( myWidth - size.width() ) / 100. ) * mMarginHorizontal;
        myYOffset = ( ( myHeight - size.height() ) / 100. ) * mMarginVertical;
        break;

      default:  // Use default of top left
        break;
    }
    //Determine placement of label from form combo box
    switch ( mPlacement )
    {
      case BottomLeft: // Bottom Left. myXOffset is set above
        myYOffset = myHeight - myYOffset - size.height();
        break;
      case TopLeft: // Top left. Already setup above
        break;
      case TopRight: // Top Right. myYOffset is set above
        myXOffset = myWidth - myXOffset - size.width();
        break;
      case BottomRight: // Bottom Right
        //Define bottom right hand corner start point
        myYOffset = myHeight - myYOffset - size.height();
        myXOffset = myWidth - myXOffset - size.width();
        break;
      default:
        QgsDebugMsg( QString( "Unknown placement index of %1" ).arg( static_cast<int>( mPlacement ) ) );
    }

    //Paint label to canvas
    QMatrix worldMatrix = context.painter()->worldMatrix();
    context.painter()->translate( myXOffset, myYOffset );
    text.drawContents( context.painter() );
    // Put things back how they were
    context.painter()->setWorldMatrix( worldMatrix );
  }
}

