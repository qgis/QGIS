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
{
  mPlacementLabels << tr( "Bottom Left" ) << tr( "Top Left" )
  << tr( "Top Right" ) << tr( "Bottom Right" );

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
  mPlacementIndex = prj->readNumEntry( mNameConfig, "/Placement", 3 );
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
  prj->writeEntry( mNameConfig, "/Placement", mPlacementIndex );
}

// Slot called when the buffer menu item is activated
void QgsDecorationCopyright::run()
{
  QgsDecorationCopyrightDialog dlg( *this, QgisApp::instance() );

  if ( dlg.exec() )
  {
    update();
  }
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
    //Determine placement of label from form combo box
    switch ( mPlacementIndex )
    {
      case 0: // Bottom Left
        //Define bottom left hand corner start point
        myYOffset = myHeight - ( size.height() + 5 );
        myXOffset = 5;
        break;
      case 1: // Top left
        //Define top left hand corner start point
        myYOffset = 0;;
        myXOffset = 5;
        break;
      case 2: // Top Right
        //Define top right hand corner start point
        myYOffset = 0;
        myXOffset = myWidth - ( size.width() + 5 );
        break;
      case 3: // Bottom Right
        //Define bottom right hand corner start point
        myYOffset = myHeight - ( size.height() + 5 );
        myXOffset = myWidth - ( size.width() + 5 );
        break;
      default:
        QgsDebugMsg( QString( "Unknown placement index of %1" ).arg( mPlacementIndex ) );
    }

    //Paint label to canvas
    QMatrix worldMatrix = theQPainter->worldMatrix();
    theQPainter->translate( myXOffset, myYOffset );
    text.drawContents( theQPainter );
    // Put things back how they were
    theQPainter->setWorldMatrix( worldMatrix );
  }
}

