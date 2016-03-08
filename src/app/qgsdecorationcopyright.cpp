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

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgisapp.h"

#include <QPainter>
#include <QMenu>
#include <QDate>
#include <QTextDocument>
#include <QMatrix>
#include <QFile>

//non qt includes
#include <cmath>


QgsDecorationCopyright::QgsDecorationCopyright( QObject* parent )
    : QgsDecorationItem( parent )
    , mMarginHorizontal( 0 )
    , mMarginVertical( 0 )
{
  mPlacement = BottomRight;
  mMarginUnit = QgsSymbolV2::MM;

  setName( "Copyright Label" );
  // initialise default values in the gui
  projectRead();
}

QgsDecorationCopyright::~QgsDecorationCopyright()
{}

void QgsDecorationCopyright::projectRead()
{
  QgsDecorationItem::projectRead();

  QDate now = QDate::currentDate();
  QString defString = "&copy; QGIS " + now.toString( "yyyy" );

  // there is no font setting in the UI, so just use the Qt/QGIS default font (what mQFont gets when created)
  //  mQFont.setFamily( QgsProject::instance()->readEntry( "CopyrightLabel", "/FontName", "Sans Serif" ) );
  //  mQFont.setPointSize( QgsProject::instance()->readNumEntry( "CopyrightLabel", "/FontSize", 9 ) );
  QgsProject* prj = QgsProject::instance();
  mLabelQString = prj->readEntry( mNameConfig, "/Label", defString );
  mMarginHorizontal = QgsProject::instance()->readNumEntry( mNameConfig, "/MarginH", 0 );
  mMarginVertical = QgsProject::instance()->readNumEntry( mNameConfig, "/MarginV", 0 );
  mLabelQColor.setNamedColor( prj->readEntry( mNameConfig, "/Color", "#000000" ) ); // default color is black
}

void QgsDecorationCopyright::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject* prj = QgsProject::instance();
  prj->writeEntry( mNameConfig, "/FontName", mQFont.family() );
  prj->writeEntry( mNameConfig, "/FontSize", mQFont.pointSize() );
  prj->writeEntry( mNameConfig, "/Label", mLabelQString );
  prj->writeEntry( mNameConfig, "/Color", mLabelQColor.name() );
  prj->writeEntry( mNameConfig, "/MarginH", mMarginHorizontal );
  prj->writeEntry( mNameConfig, "/MarginV", mMarginVertical );
}

// Slot called when the buffer menu item is activated
void QgsDecorationCopyright::run()
{
  QgsDecorationCopyrightDialog dlg( *this, QgisApp::instance() );
  dlg.exec();
}


void QgsDecorationCopyright::render( QPainter * theQPainter )
{
  //Large IF statement to enable/disable copyright label
  if ( enabled() )
  {
    // need width/height of paint device
    int myHeight = theQPainter->device()->height();
    int myWidth = theQPainter->device()->width();

    QTextDocument text;
    text.setDefaultFont( mQFont );
    // To set the text color in a QTextDocument we use a CSS style
    QString style = "<style type=\"text/css\"> p {color: " +
                    mLabelQColor.name() + "}</style>";
    text.setHtml( style + "<p>" + mLabelQString + "</p>" );
    QSizeF size = text.size();

    float myXOffset( 0 ), myYOffset( 0 );

    // Set  margin according to selected units
    switch ( mMarginUnit )
    {
      case QgsSymbolV2::MM:
      {
        int myPixelsInchX = theQPainter->device()->logicalDpiX();
        int myPixelsInchY = theQPainter->device()->logicalDpiY();
        myXOffset = myPixelsInchX * INCHES_TO_MM * mMarginHorizontal;
        myYOffset = myPixelsInchY * INCHES_TO_MM * mMarginVertical;
        break;
      }

      case QgsSymbolV2::Pixel:
        myXOffset = mMarginHorizontal;
        myYOffset = mMarginVertical;
        break;

      case QgsSymbolV2::Percentage:
        myXOffset = (( myWidth - size.width() ) / 100. ) * mMarginHorizontal;
        myYOffset = (( myHeight - size.height() ) / 100. ) * mMarginVertical;
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
    QMatrix worldMatrix = theQPainter->worldMatrix();
    theQPainter->translate( myXOffset, myYOffset );
    text.drawContents( theQPainter );
    // Put things back how they were
    theQPainter->setWorldMatrix( worldMatrix );
  }
}

