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
  : QObject(parent)
{
  mPlacementLabels << tr( "Bottom Left" ) << tr( "Top Left" )
  << tr( "Top Right" ) << tr( "Bottom Right" );

  // initialise default values in the gui
  projectRead();
}

QgsDecorationCopyright::~QgsDecorationCopyright()
{}

void QgsDecorationCopyright::projectRead()
{
  QDate now;
  QString defString;

  now = QDate::currentDate();
  defString = "&copy; QGIS " + now.toString( "yyyy" );

  // there is no font setting in the UI, so just use the Qt/QGIS default font (what mQFont gets when created)
  //  mQFont.setFamily( QgsProject::instance()->readEntry( "CopyrightLabel", "/FontName", "Sans Serif" ) );
  //  mQFont.setPointSize( QgsProject::instance()->readNumEntry( "CopyrightLabel", "/FontSize", 9 ) );
  QgsProject* prj = QgsProject::instance();
  mLabelQString = prj->readEntry( "CopyrightLabel", "/Label", defString );
  mPlacementIndex = prj->readNumEntry( "CopyrightLabel", "/Placement", 3 );
  mEnable = prj->readBoolEntry( "CopyrightLabel", "/Enabled", false );
  mLabelQColor.setNamedColor( prj->readEntry( "CopyrightLabel", "/Color", "#000000" ) ); // default color is black
}

// Slot called when the buffer menu item is activated
void QgsDecorationCopyright::run()
{
  QgsDecorationCopyrightDialog dlg( *this, QgisApp::instance() );

  if (dlg.exec())
  {
    saveToProject();
    QgisApp::instance()->mapCanvas()->refresh();
  }
}


void QgsDecorationCopyright::renderLabel( QPainter * theQPainter )
{
  //Large IF statement to enable/disable copyright label
  if ( mEnable )
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

void QgsDecorationCopyright::saveToProject()
{
  //save state to the project file.....
  QgsProject* prj = QgsProject::instance();
  prj->writeEntry( "CopyrightLabel", "/FontName", mQFont.family() );
  prj->writeEntry( "CopyrightLabel", "/FontSize", mQFont.pointSize() );
  prj->writeEntry( "CopyrightLabel", "/Label", mLabelQString );
  prj->writeEntry( "CopyrightLabel", "/Color", mLabelQColor.name() );
  prj->writeEntry( "CopyrightLabel", "/Placement", mPlacementIndex );
  prj->writeEntry( "CopyrightLabel", "/Enabled", mEnable );
}
