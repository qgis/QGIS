/***************************************************************************
  qgsquickprint.cpp 
  A class to quickly print a map with minimal effort.
  -------------------
         begin                : Jan 2008
         copyright            : (c) Tim Sutton, 2008
         email                : tim@linfiniti.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: plugin.cpp 7796 2007-12-16 22:11:38Z homann $ */

//
// QGIS Specific includes
//

#include <qgisinterface.h>
#include <qgisgui.h>
#include "qgsquickprint.h"
#include <qgsapplication.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectorlayer.h>
#include <qgssymbol.h>
#include <qgsmapcanvas.h>
#include <qgsrenderer.h>
#include <qgslogger.h>
#include <qgslabelattributes.h>
#include <qgslabel.h>

//
// Qt4 Related Includes
//

#include <QAction>
#include <QToolBar>
#include <QColor>
#include <QPainter>
#include <QDate>
#include <QPixmap>
#include <QString>
#include <QSettings>
#include <QSvgRenderer>
#include <QPrinter>

//other includes
#include <cmath>

#ifdef _MSC_VER
#define round(x)  ((x) >= 0 ? floor((x)+0.5) : floor((x)-0.5))
#endif

QgsQuickPrint::QgsQuickPrint()
{
}

QgsQuickPrint::~QgsQuickPrint()
{

}
void QgsQuickPrint::setTitle(QString theText)
{
  mTitleText=theText;
}
void QgsQuickPrint::setName(QString theText)
{
  mNameText=theText;
}
void QgsQuickPrint::setCopyright(QString theText)
{
  mCopyrightText=theText;
}
void QgsQuickPrint::setNorthArrow(QString theFileName)
{
  mNorthArrowFile=theFileName;
}
void QgsQuickPrint::setLogo1(QString theFileName)
{
  mLogo1File=theFileName;
  qDebug("Logo1 set to: " + mLogo1File.toLocal8Bit());
}
void QgsQuickPrint::setLogo2(QString theFileName)
{
  mLogo2File=theFileName;
  qDebug("Logo2 set to: " + mLogo2File.toLocal8Bit());
}
void QgsQuickPrint::setOutputPdf(QString theFileName)
{
  mOutputFileName=theFileName;
}
void QgsQuickPrint::setMapCanvas(QgsMapCanvas * thepMapCanvas)
{
  mpMapCanvas=thepMapCanvas;
}

void QgsQuickPrint::printMap()
{
  if ( mOutputFileName.isEmpty() )
  {
    return;
  }
  if ( mpMapCanvas == NULL )
  {
    return;
  }
  //ensure the user never ommitted the extension from the filename
  if ( !mOutputFileName.toUpper().endsWith ( ".PDF" ) )
  {
    mOutputFileName += ".pdf";
  }

  QPrinter myPrinter ( QPrinter::HighResolution ); //1200dpi for ps ( & pdf I think )
  myPrinter.setPageSize ( QPrinter::Letter );
  //myPrinter.setPageSize ( QPrinter::A4 );
  //
  // Try to force the printer resolution to 300dpi
  // to get past platform specific defaults in printer
  // resolution...
  //
  int myPrintResolutionDpi = 300;
  int myScreenResolutionDpi = 72; //try to get programmatically
  myPrinter.setResolution( myPrintResolutionDpi );
  myPrinter.setOutputFormat ( QPrinter::PdfFormat );
  myPrinter.setOutputFileName ( mOutputFileName );
  myPrinter.setOrientation ( QPrinter::Landscape );
  myPrinter.setDocName ( "quickprint Report" );
  QPainter myPrintPainter( &myPrinter );
  myPrintPainter.setPen( Qt::gray );
  myPrintPainter.setBrush( Qt::white );
  // This is what we are aiming for:
  // a 
  // +-(1)------ Acme Maps (2) --------------------------------------+
  // |b         12/01/2007 (3)                                         |
  // |                           Earthquakes (4)                       |
  // | +--(5)--------------------------------------------------------+ |
  // | |c                                                            | |
  // | | +-(6)---------------------------------------+  +~(7)~~~~~~+ | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | |                                           |  |          | | |
  // | | +-------------------------------------------+  +~~~~~~~~~~+ | |
  // | |                                                             | |
  // | +-------------------------------------------------------------+ |
  // |                                                                 |
  // |   +-(8)-----+ +-(9-)----+ +-(10)----+                 /|\       |
  // |   |         | |Copyright| |         |                / | \      |
  // |   |         | |  2008   | |         |                  |(11)    |
  // |   +---------+ +---------+ +---------+                           |
  // |                                                  +~(12)~~~~~~+  |
  // +-----------------------------------------------------------------+
  //
  // 1) PageBorder              8) Logo1
  // 2) PageTitle               9) CopyrightText
  // 3) MapDate                 10) Logo2
  // 4) MapTitle                11) NorthArrow
  // 5) MapFrame                12) ScaleBar
  // 6) MapPixmap               
  // 7) LegendPixmap
  // a OriginXY
  // b HorizontalSpacing
  // c VerticalSpacing

  //
  // Note: Different operating systems will use different
  // page resolutions for QPrinter::HighResolution so I'm
  // working all coordinates out as percentages of page
  // size so that we can hopefully get comarable print 
  // results on all platforms.
  //

  //
  // Note #2: Im defining all measurements here as my plan
  // is to later support templates with different page
  // layouts and paper sizes etc.
  //


  //set the top left origin for the print layout
  int myOriginX = myPrinter.pageRect().left();
  int myOriginY = myPrinter.pageRect().top();
  int myDrawableWidth = myPrinter.pageRect().width()-myOriginX;
  int myDrawableHeight = myPrinter.pageRect().height()-myOriginY;

  //define the spacing between layout elements
  int myHorizontalSpacing = myDrawableWidth / 100; // 1%
  int myVerticalSpacing = myDrawableHeight / 100; // 1%

  //define the proportions for the page layout
  int myMapWidthPercent = 65;
  int myMapHeightPercent = 71;
  int myLegendWidthPercent = 25;
  int myLegendHeightPercent = 65;
  int myLogoWidthPercent = 23;
  int myLogoHeightPercent = 17;
  int mySymbolScalingAmount = myPrintResolutionDpi / myScreenResolutionDpi; 

  //define the font sizes and family
  int myMapTitleFontSize = 24;
  int myMapDateFontSize = 16;
  int myMapNameFontSize = 32;
  int myLegendFontSize = 12;
#ifdef WIN32 //this sucks...
   myMapTitleFontSize /= 2;
   myMapDateFontSize /= 2;
   myMapNameFontSize /= 2;
   myLegendFontSize /= 2;
#endif
  QString myFontFamily = "Arial";

  // Background colour for pixmaps
  QColor myLegendBackgroundColour = Qt::white;
  QColor myMapBackgroundColour = mpMapCanvas->canvasColor();
  //QColor myMapBackgroundColour = "#98dbf9"; // nice blue colour


  //
  // Draw the PageBorder
  // 
  myPrintPainter.drawRect( 
      myOriginX, myOriginY, myDrawableWidth, myDrawableHeight );
  // 
  // Draw the PageTitle
  // 
  QFont myTitleFont(myFontFamily, myMapTitleFontSize);
  myPrintPainter.setFont(myTitleFont);
  QFontMetrics myTitleMetrics(myTitleFont, &myPrinter);
  int myPageTitleHeight = myTitleMetrics.height();
  int myPageTitleWidth = myTitleMetrics.width(mTitleText);
  myOriginX += myHorizontalSpacing;
  myOriginY -= (myPageTitleHeight / 2);
  QRect myPageTitleRect (myOriginX,
      myOriginY,
      myPageTitleWidth,
      myPageTitleHeight);
  // make sure the title goes onto a white background
  myPrintPainter.setPen( Qt::white );
  myPrintPainter.drawRect( myPageTitleRect );
  myPrintPainter.setPen( Qt::black );
  myPrintPainter.drawText( myPageTitleRect, Qt::AlignCenter, mTitleText );

  // 
  // Draw the MapDate
  // 
  QFont myDateFont(myFontFamily, myMapDateFontSize);
  QString myDateText (QDate::currentDate().toString(Qt::LocalDate));
  myPrintPainter.setFont(myDateFont);
  QFontMetrics myDateMetrics(myDateFont, &myPrinter);
  int myDateHeight = myDateMetrics.height();
  //int myDateWidth = myDateMetrics.width(myDateText);
  myOriginX += myHorizontalSpacing;
  myOriginY += myPageTitleHeight  + myVerticalSpacing ;
  QRect myDateRect (myOriginX,
      myOriginY,
      myPageTitleWidth, //use same width as page title for centering
      myDateHeight);
  // make sure the title goes onto a white background
  myPrintPainter.setPen( Qt::white );
  myPrintPainter.drawRect( myDateRect );
  myPrintPainter.setPen( Qt::black );
  myPrintPainter.drawText( myDateRect, Qt::AlignCenter, myDateText );

  // 
  // Draw the MapName
  // 
  QFont myNameFont(myFontFamily, myMapNameFontSize);
  myPrintPainter.setFont(myNameFont);
  QFontMetrics myNameMetrics(myNameFont, &myPrinter);
  int myNameHeight = myNameMetrics.height();
  int myNameWidth = myNameMetrics.width(mNameText);
  myOriginX = myPrinter.pageRect().left() + myDrawableWidth /2; //page center
  myOriginX -= myNameWidth / 2;
  myOriginY = myPrinter.pageRect().top() + (myPageTitleHeight /2)  + myVerticalSpacing ;
  QRect myNameRect (myOriginX,
      myOriginY,
      myNameWidth, 
      myNameHeight);
  // make sure the title goes onto a white background
  myPrintPainter.setPen( Qt::white );
  myPrintPainter.drawRect( myNameRect );
  myPrintPainter.setPen( Qt::black );
  myPrintPainter.drawText( myNameRect, Qt::AlignCenter, mNameText );

  // 
  // Draw the MapFrame (top)
  // 
  int myMapFrameWidth = myDrawableWidth ;
  myOriginX = myPrinter.pageRect().left() + myHorizontalSpacing;
  myOriginY += myNameHeight + myVerticalSpacing;
  QLine myMapFrameTopLine (myOriginX,
      myOriginY,
      myMapFrameWidth, 
      myOriginY);
  myPrintPainter.setPen( Qt::black );
  myPrintPainter.drawLine( myMapFrameTopLine );


  // Draw the map onto a pixmap
  // @TODO: we need to save teh extent of the screen map and
  // then set them again for the print map so that the map scales
  // properly in the print
  int myMapDimensionX = (myDrawableWidth / 100) * myMapHeightPercent;
  int myMapDimensionY = (myDrawableHeight / 100) * myMapWidthPercent;
  QPixmap myMapPixmap ( myMapDimensionX,myMapDimensionY );
  myMapPixmap.fill ( myMapBackgroundColour );
  QPainter myMapPainter;
  myMapPainter.begin( &myMapPixmap );
  mpMapCanvas->mapRender()->setOutputSize( 
      QSize ( myMapDimensionX, myMapDimensionY ), myPrinter.resolution() ); 
  scalePointSymbols(mySymbolScalingAmount, ScaleUp);
  scaleTextLabels(mySymbolScalingAmount, ScaleUp);
  mpMapCanvas->mapRender()->render( &myMapPainter );
  //maprender has no accessor for output size so 
  //we couldnt store the size before starting the print render
  //so that it can be restored properly so what follows here is a
  //best guess approach
  mpMapCanvas->mapRender()->setOutputSize( 
      QSize ( mpMapCanvas->width(), mpMapCanvas->height() ), myScreenResolutionDpi ); 
  myMapPainter.end();
  //draw the map pixmap onto our pdf print device
  myOriginX = myPrinter.pageRect().left() + myHorizontalSpacing; 
  myOriginY += myVerticalSpacing * 2;
  myPrintPainter.drawPixmap( 
      myOriginX, 
      myOriginY, 
      myMapPixmap ); 

  //
  // Draw the legend
  //
  QFont myLegendFont(myFontFamily, myLegendFontSize);
  //myPrintPainter.setFont(myLegendFont);
  int myLegendDimensionX = (myDrawableWidth / 100) * myLegendWidthPercent;
  int myLegendDimensionY = (myDrawableHeight / 100) * myLegendHeightPercent;
  // Create a viewport to make coordinate conversions easier
  // The viewport has the same dimensions as the page(otherwise items
  // drawn into it will appear squashed), but a different origin.
  QRect myOriginalViewport = myPrintPainter.viewport(); //for restoring later
  myOriginX += myMapDimensionX + myHorizontalSpacing;
  myPrintPainter.setViewport(myOriginX,
      myOriginY, 
      myOriginalViewport.width(),
      myOriginalViewport.height());
  QFontMetrics myLegendFontMetrics( myLegendFont, &myPrinter );
  int myLegendFontHeight = myLegendFontMetrics.height();
  int myLegendXPos = 0;
  int myLegendYPos = 0;
  int myLegendSpacer = myLegendFontHeight; //for vertical and horizontal spacing
  int myLegendVerticalSpacer = myLegendFontHeight/3; //for vertical between rows
  int myIconWidth = myLegendFontHeight;
  myPrintPainter.setFont( myLegendFont );
  QStringList myLayerSet = mpMapCanvas->mapRender()->layerSet();
  QStringListIterator myLayerIterator ( myLayerSet );
  while ( myLayerIterator.hasNext() )
  {
    QString myLayerId = myLayerIterator.next();
    QgsMapLayer * mypLayer = 
      QgsMapLayerRegistry::instance()->mapLayer ( myLayerId );
    if ( mypLayer )
    {
      QgsVectorLayer *mypVectorLayer  =
        dynamic_cast<QgsVectorLayer *> ( mypLayer );
      if ( mypVectorLayer )
      {
        QString myLayerName = mypVectorLayer->name();
        int myLayerNameWidth = myLegendFontMetrics.width(myLayerName);
        QIcon myIcon;
        QPixmap myPixmap ( QSize ( myIconWidth, myIconWidth ) ); //square
        //based on code from qgslegendlayer.cpp - see that file for more info
        const QgsRenderer* mypRenderer = mypVectorLayer->renderer();
        const QList<QgsSymbol*> mySymbolList = mypRenderer->symbols();
        //
        // Single symbol
        //
        if ( 1 == mySymbolList.size() )
        {
          QgsSymbol * mypSymbol = mySymbolList.at(0); 
          myPrintPainter.setPen( mypSymbol->pen() );
          myPrintPainter.setBrush( mypSymbol->brush() );
          myLegendXPos = myLegendSpacer;
          if ( mypSymbol->type() == QGis::Point )
          {
            QImage myImage;
            myImage = mypSymbol->getPointSymbolAsImage();
            myPixmap = QPixmap::fromImage ( myImage ); // convert to pixmap
            myPixmap = myPixmap.scaled (myIconWidth,myIconWidth);
            myPrintPainter.drawPixmap ( myLegendXPos, myLegendYPos, myPixmap);
          }
          else if ( mypSymbol->type() == QGis::Line )
          {
            myPrintPainter.drawLine( myLegendXPos, myLegendYPos,
                myLegendXPos + myIconWidth, 
                myLegendYPos + myIconWidth );
          }
          else //polygon
          {
            myPrintPainter.drawRect( myLegendXPos, myLegendYPos, myIconWidth, myIconWidth );
          }
          myLegendXPos += myIconWidth + myLegendSpacer;
          myPrintPainter.setPen( Qt::black );
          // check if the text will overflow the space we have
          int myMaximumLabelWidth = myLegendDimensionX - myLegendXPos;
          if (myLayerNameWidth > myMaximumLabelWidth)
          {
            int myRowCount = (myLayerNameWidth / myMaximumLabelWidth) + 1;
            QRect myLegendItemRect (myLegendXPos,
                myLegendYPos,
                myLayerNameWidth, 
                myLegendFontHeight * myRowCount);
            QTextOption myTextOption( Qt::AlignCenter );
            myTextOption.setWrapMode ( 
                QTextOption::WrapAtWordBoundaryOrAnywhere );
            myPrintPainter.drawText( 
                myLegendItemRect, 
                myLayerName,
                myTextOption 
                );
            myLegendYPos += myLegendVerticalSpacer + (myLegendFontHeight * myRowCount);
          }
          else //fits ok on a single line
          {
            QRect myLegendItemRect (myLegendXPos,
                myLegendYPos,
                myLayerNameWidth, 
                myLegendFontHeight);
            myPrintPainter.drawText( myLegendItemRect, Qt::AlignCenter, myLayerName );
            myLegendYPos += myLegendVerticalSpacer + myLegendFontHeight;
          }
        }
        else  //class breaks
        {
          // draw in the layer name first, after we loop for the class breaks
          myLegendXPos = myIconWidth + myLegendSpacer;
          QRect myLegendItemRect (myLegendXPos,
              myLegendYPos,
              myLayerNameWidth, 
              myLegendFontHeight);
          myPrintPainter.setPen( Qt::black );
          myPrintPainter.drawText( myLegendItemRect, Qt::AlignCenter, myLayerName );
          myLegendYPos += myLegendVerticalSpacer + myLegendFontHeight;
          QListIterator<QgsSymbol *> myIterator ( mySymbolList );
          while ( myIterator.hasNext() )
          {
            QgsSymbol * mypSymbol = myIterator.next();
            myPrintPainter.setPen( mypSymbol->pen() );
            myPrintPainter.setBrush( mypSymbol->brush() );
            myLegendXPos = myLegendSpacer * 3; //extra indent for class breaks
            if ( mypSymbol->type() == QGis::Point )
            {
              QImage myImage;
              myImage = mypSymbol->getPointSymbolAsImage();
              myPixmap = QPixmap::fromImage ( myImage ); // convert to pixmap
              myPixmap = myPixmap.scaled (myIconWidth,myIconWidth);
              myPrintPainter.drawPixmap ( myLegendXPos, myLegendYPos, myPixmap);
            }
            else if ( mypSymbol->type() == QGis::Line )
            {
              myPrintPainter.drawLine( myLegendXPos, myLegendYPos,
                  myLegendXPos + myIconWidth, 
                  myLegendYPos + myIconWidth );
            }
            else //polygon
            {
              myPrintPainter.drawRect( myLegendXPos, myLegendYPos, myIconWidth, myIconWidth );
            }
            //
            // Now work out the class break label
            //
            QString myLabel;
            QString myLower = mypSymbol->lowerValue();
            if(!myLower.isEmpty())
            {
              myLabel = myLower;
            }
            QString myUpper = mypSymbol->upperValue();
            if(!myUpper.isEmpty())
            {
              myLabel += " - ";
              myLabel += myUpper;
            }
            QString myText = mypSymbol->label();
            if(!myText.isEmpty())
            {
              myLabel += " ";
              myLabel += myText;
            }
            myLabel = myLabel.trimmed();
            myLegendXPos += myIconWidth + myLegendSpacer;
            int myLabelWidth = myLegendFontMetrics.width(myLabel);
            myPrintPainter.setPen( Qt::black );
            // check if the text will overflow the space we have
            int myMaximumLabelWidth = myLegendDimensionX - myLegendXPos;
            if (myLabelWidth > myMaximumLabelWidth)
            {
              int myRowCount = (myLabelWidth / myMaximumLabelWidth) + 1;
              QRect myLegendItemRect (myLegendXPos,
                  myLegendYPos,
                  myLabelWidth, 
                  myLegendFontHeight * myRowCount);
              QTextOption myTextOption( Qt::AlignLeft );
              myTextOption.setWrapMode ( 
                  QTextOption::WordWrap);
              //QTextOption::WrapAtWordBoundaryOrAnywhere );
              myPrintPainter.drawText( 
                  myLegendItemRect, 
                  myLabel, 
                  myTextOption
                  );
              /*
                 myPrintPainter.drawText( myLegendItemRect, 
                 Qt::AlignLeft | Qt::TextWordWrap, 
                 myLabel );
                 myPrintPainter.drawRect( myLegendItemRect ), 
                 */
              myLegendYPos += myLegendVerticalSpacer + (myLegendFontHeight * myRowCount);
            }
            else //fits ok on a single line
            {
              QRect myLegendItemRect (myLegendXPos,
                  myLegendYPos,
                  myLabelWidth, 
                  myLegendFontHeight);
              myPrintPainter.drawText( myLegendItemRect, Qt::AlignCenter, myLabel );
              myLegendYPos += myLegendVerticalSpacer + myLegendFontHeight;
            }
          } //symbol loop
        } //class breaks
      } //if vectorlayer
    } //if maplayer
  } //layer iterator

  //reinstate the viewport
  myPrintPainter.setViewport(myOriginalViewport); 


  // 
  // Draw the MapFrame (bottom)
  // 
  myOriginX = myPrinter.pageRect().left() + myHorizontalSpacing;
  myOriginY += myMapDimensionY + (myVerticalSpacing * 2);
  QLine myMapFrameBottomLine (myOriginX,
      myOriginY,
      myMapFrameWidth, 
      myOriginY);
  myPrintPainter.setPen(  Qt::black );
  myPrintPainter.drawLine( myMapFrameBottomLine );


  //
  // Draw logo 1
  //
  int myLogoXDim = ( myDrawableWidth / 100 ) * myLogoWidthPercent;
  int myLogoYDim = ( myDrawableHeight / 100 ) * myLogoHeightPercent;
  QPixmap myLogo1;
  qDebug("Logo1:");
  qDebug(mLogo1File);
  myLogo1.fill ( Qt::white );
  myLogo1.load ( mLogo1File ); 
  myLogo1 = myLogo1.scaled ( myLogoXDim,myLogoYDim, Qt::KeepAspectRatio);
  myOriginX = myPrinter.pageRect().left() + myHorizontalSpacing;
  myOriginY += myVerticalSpacing ;
  myPrintPainter.drawPixmap( myOriginX ,
      myOriginY , 
      myLogo1); 

  //
  // Draw Copyright Text
  //
  myOriginX += myHorizontalSpacing + myLogoXDim;
  QRect myCopyrightRect ( myOriginX, myOriginY, myLogoXDim, myLogoYDim );
  myPrintPainter.setPen(  Qt::black );
  QFont myCopyrightFont(myFontFamily, myMapDateFontSize);
  myPrintPainter.setFont ( myCopyrightFont );
  //myPrintPainter.drawRect( myCopyrightRect ); 
  myPrintPainter.drawText( myCopyrightRect, Qt::AlignCenter | Qt::TextWordWrap, mCopyrightText );

  //
  // Draw logo 2
  //
  QPixmap myLogo2;
  myLogo2.fill ( Qt::white );
  myLogo2.load ( mLogo2File ); 
  myLogo2 = myLogo2.scaled ( myLogoXDim,myLogoYDim, Qt::KeepAspectRatio );
  myOriginX += myHorizontalSpacing + myLogoXDim;
  myPrintPainter.drawPixmap( myOriginX ,
      myOriginY , 
      myLogo2); 


  //
  // Draw the north arrow
  //
  myOriginX += myHorizontalSpacing + myLogoXDim;
  QPixmap myNorthArrow ( myLogoYDim/2, myLogoYDim/2 );
  myNorthArrow.fill ( Qt::white );
  QPainter myNorthPainter ( &myNorthArrow );
  QSvgRenderer mySvgRenderer ( mNorthArrowFile );
  mySvgRenderer.render ( &myNorthPainter );
  myPrintPainter.drawPixmap( myOriginX + ((myLogoXDim/2)-(myLogoYDim/2)) ,
      myOriginY , 
      myNorthArrow); 

  //
  // Draw the scale bar
  //
  renderPrintScaleBar(&myPrintPainter, mpMapCanvas);

  //
  // Finish up
  //

  //reinstate the symbols scaling for screen display
  scalePointSymbols(mySymbolScalingAmount, ScaleDown);
  scaleTextLabels(mySymbolScalingAmount, ScaleDown);


  myPrintPainter.end();
  /*
     mProgressDialog.setValue ( 0 );
     mProgressDialog.setLabelText ( tr ( "Please wait while your report is generated" ) );
     mProgressDialog.show();
     mProgressDialog.setWindowModality ( Qt::WindowModal );
     mProgressDialog.setAutoClose ( true );
     */
} 

void QgsQuickPrint::scaleTextLabels( int theScaleFactor, SymbolScalingType theDirection)
{
  if (0 >= theScaleFactor)
  {
    QgsDebugMsg ("QgsQuickPrintGui::scaleTextLabels invalid scale factor");
    return;
  }
  QStringList myLayerSet = mpMapCanvas->mapRender()->layerSet();
  QStringListIterator myLayerIterator ( myLayerSet );
  while ( myLayerIterator.hasNext() )
  {
    QString myLayerId = myLayerIterator.next();
    QgsDebugMsg ( "Scaling text labels for print for " + myLayerId );
    QgsMapLayer * mypLayer = 
      QgsMapLayerRegistry::instance()->mapLayer ( myLayerId );
    if ( mypLayer )
    {
      QgsVectorLayer *mypVectorLayer  =
        dynamic_cast<QgsVectorLayer *> ( mypLayer );
      if ( mypVectorLayer )
      {
        QgsLabel * mypLabel = mypVectorLayer->label();
        QgsLabelAttributes * mypLabelAttributes = mypLabel->layerAttributes();
        if (theDirection == ScaleUp)
        {
          mypLabelAttributes->setSize(
              mypLabelAttributes->size() * theScaleFactor , 
              mypLabelAttributes->sizeType());
        }
        else //scale down
        {
          mypLabelAttributes->setSize(
              mypLabelAttributes->size() / theScaleFactor , 
              mypLabelAttributes->sizeType());
        }
      } //if vectorlayer
    } //if maplayer
  } //layer iterator
}

void QgsQuickPrint::scalePointSymbols( int theScaleFactor, SymbolScalingType theDirection)
{
  if (0 >= theScaleFactor)
  {
    QgsDebugMsg ("QgsQuickPrintGui::scalePointSymbolsForPrint invalid scale factor");
    return;
  }
  QStringList myLayerSet = mpMapCanvas->mapRender()->layerSet();
  QStringListIterator myLayerIterator ( myLayerSet );
  while ( myLayerIterator.hasNext() )
  {
    QString myLayerId = myLayerIterator.next();
    QgsDebugMsg ( "Scaling point symbols for print for " + myLayerId );
    QgsMapLayer * mypLayer = 
      QgsMapLayerRegistry::instance()->mapLayer ( myLayerId );
    if ( mypLayer )
    {
      QgsVectorLayer *mypVectorLayer  =
        dynamic_cast<QgsVectorLayer *> ( mypLayer );
      if ( mypVectorLayer )
      {
        const QgsRenderer* mypRenderer = mypVectorLayer->renderer();
        const QList<QgsSymbol*> mySymbolList = mypRenderer->symbols();
        //
        // Single symbol
        //
        if ( 1 == mySymbolList.size() )
        {
          QgsSymbol * mypSymbol = mySymbolList.at(0); 
          if ( mypSymbol->type() == QGis::Point )
          {
            if (theDirection == ScaleUp)
            {
              mypSymbol->setPointSize ( mypSymbol->pointSize() * theScaleFactor );
            }
            else //Scale Down
            {
              mypSymbol->setPointSize ( mypSymbol->pointSize() / theScaleFactor );
            }
          }
        }
        else  //class breaks
        {
          QListIterator<QgsSymbol *> myIterator ( mySymbolList );
          while ( myIterator.hasNext() )
          {
            QgsSymbol * mypSymbol = myIterator.next();
            if ( mypSymbol->type() == QGis::Point )
            {
              if (theDirection == ScaleUp)
              {
                mypSymbol->setPointSize ( mypSymbol->pointSize() * theScaleFactor );
              }
              else //Scale Down
              {
                mypSymbol->setPointSize ( mypSymbol->pointSize() / theScaleFactor );
              }
            }
          } //symbol loop
        } //class breaks
      } //if vectorlayer
    } //if maplayer
  } //layer iterator
}



void QgsQuickPrint::renderPrintScaleBar(QPainter * thepPainter, QgsMapCanvas * thepMapCanvas)
{
  //hard coding some options for now
  int myStyleIndex = 1; //tick up
  bool mySnappingFlag = true;
  QColor mColour = Qt::black;
  int myPlacementIndex = 3; //br
  // Hard coded sizes
  int myMajorTickSize=20;
  int myTextOffsetX=30;
  int myTextOffsetY=30;
  int myXMargin=260;
  int myYMargin=180;
  int myCanvasWidth = thepMapCanvas->width();
  int myPreferredSize = myCanvasWidth;
  double myActualSize=myPreferredSize;
  int myBufferSize=1; //softcode this later
  QColor myBackColor = Qt::white; //used for text
  QColor myForeColor = Qt::black; //used for text

  //Get canvas dimensions
  //int myCanvasHeight = thepMapCanvas->height();

  //Get map units per pixel. This can be negative at times (to do with
  //projections) and that just confuses the rest of the code in this
  //function, so force to a positive number.
  double myMuppDouble = std::abs(thepMapCanvas->mupp());

  // Exit if the canvas width is 0 or layercount is 0 or QGIS will freeze
  int myLayerCount=thepMapCanvas->layerCount();
  if (!myLayerCount || !myMuppDouble) return;


  //Calculate size of scale bar for preferred number of map units
  double myScaleBarWidth = myPreferredSize / myMuppDouble;

  //If scale bar is very small reset to 1/4 of the canvas wide
  if (myScaleBarWidth < 30)
  {
    myScaleBarWidth = myCanvasWidth / 4; // pixels
    myActualSize = myScaleBarWidth * myMuppDouble; // map units
  };

  myActualSize = myScaleBarWidth * myMuppDouble;

  // Work out the exponent for the number - e.g, 1234 will give 3,
  // and .001234 will give -3
  double myPowerOf10 = floor(log10(myActualSize));

  // snap to integer < 10 times power of 10
  if (mySnappingFlag) 
  {
    double scaler = pow(10.0, myPowerOf10);
    myActualSize = round(myActualSize / scaler) * scaler;
    myScaleBarWidth = myActualSize / myMuppDouble;
  }

  //Get type of map units and set scale bar unit label text
  QGis::units myMapUnits=thepMapCanvas->mapUnits();
  QString myScaleBarUnitLabel;
  switch (myMapUnits)
  {
    case QGis::METERS: 
      if (myActualSize > 1000.0)
      {
        myScaleBarUnitLabel=tr(" km");
        myActualSize = myActualSize/1000;
      }
      else if (myActualSize < 0.01)
      {
        myScaleBarUnitLabel=tr(" mm");
        myActualSize = myActualSize*1000;
      }
      else if (myActualSize < 0.1)
      {
        myScaleBarUnitLabel=tr(" cm");
        myActualSize = myActualSize*100;
      }
      else
        myScaleBarUnitLabel=tr(" m"); 
      break;
    case QGis::FEET:
      if (myActualSize > 5280.0) //5280 feet to the mile
      {
        myScaleBarUnitLabel=tr(" miles");
        myActualSize = myActualSize/5280;
      }
      else if (myActualSize == 5280.0) //5280 feet to the mile
      {
        myScaleBarUnitLabel=tr(" mile");
        myActualSize = myActualSize/5280;
      }
      else if (myActualSize < 1)
      {
        myScaleBarUnitLabel=tr(" inches");
        myActualSize = myActualSize*12;
      }
      else if (myActualSize == 1.0)
      {
        myScaleBarUnitLabel=tr(" foot"); 
      }
      else
      {
        myScaleBarUnitLabel=tr(" feet"); 
      }
      break;
    case QGis::DEGREES:
      if (myActualSize == 1.0)
        myScaleBarUnitLabel=tr(" degree"); 
      else
        myScaleBarUnitLabel=tr(" degrees"); 
      break;
    case QGis::UNKNOWN:
      myScaleBarUnitLabel=tr(" unknown");
    default: 
      QgsDebugMsg( "Error: not picked up map units - actual value = " 
          + QString::number (myMapUnits) );
  };

  //Set font and calculate width of unit label
  int myFontSize = 10; //we use this later for buffering
  QFont myFont( "helvetica", myFontSize );
  thepPainter->setFont(myFont);
  QFontMetrics myFontMetrics( myFont );
  double myFontWidth = myFontMetrics.width( myScaleBarUnitLabel );
  double myFontHeight = myFontMetrics.height();

  //Set the maximum label
  QString myScaleBarMaxLabel=QString::number(myActualSize);

  //Calculate total width of scale bar and label
  double myTotalScaleBarWidth = myScaleBarWidth + myFontWidth;

  //determine the origin of scale bar depending on placement selected
  int myOriginX=myXMargin;
  int myOriginY=myYMargin;
  switch (myPlacementIndex)
  {
    case 0: // Bottom Left
      myOriginX = myXMargin;
      myOriginY = thepPainter->device()->height() - myYMargin + myTextOffsetY;
      break;
    case 1: // Top Left
      myOriginX = myXMargin;
      myOriginY = myYMargin;
      break;
    case 2: // Top Right
      myOriginX = thepPainter->device()->width() - ((int) myTotalScaleBarWidth) - myXMargin;
      myOriginY = myXMargin;
      break;
    case 3: // Bottom Right
      myOriginX = thepPainter->device()->width() - 
        ((int) myTotalScaleBarWidth) - 
        (myXMargin * 2);
      myOriginY = thepPainter->device()->height() - myYMargin;
      break;
    default:
      QgsDebugMsg( "Unable to determine where to put scale bar so defaulting to top left");
  }

  //Set pen to draw with
  QPen myForegroundPen( mColour, 2 );
  QPen myBackgroundPen( Qt::white, 3 );

  //Cast myScaleBarWidth to int for drawing
  int myScaleBarWidthInt = (int) myScaleBarWidth;

  //Create array of vertices for scale bar depending on style
  switch (myStyleIndex)
  {
    case 0: // Tick Down
      {
        QPolygon myTickDownArray(4);
        //draw a buffer first so bar shows up on dark images
        thepPainter->setPen( myBackgroundPen );
        myTickDownArray.putPoints(0,4,
            myOriginX                    , (myOriginY + myMajorTickSize) ,
            myOriginX                    ,  myOriginY                    ,
            (myScaleBarWidthInt + myOriginX),  myOriginY                    ,
            (myScaleBarWidthInt + myOriginX), (myOriginY + myMajorTickSize)
            );
        thepPainter->drawPolyline(myTickDownArray);
        //now draw the bar itself in user selected color
        thepPainter->setPen( myForegroundPen );
        myTickDownArray.putPoints(0,4,
            myOriginX                    , (myOriginY + myMajorTickSize) ,
            myOriginX                    ,  myOriginY                    ,
            (myScaleBarWidthInt + myOriginX),  myOriginY                    ,
            (myScaleBarWidthInt + myOriginX), (myOriginY + myMajorTickSize)
            );
        thepPainter->drawPolyline(myTickDownArray);
        break;
      }
    case 1: // tick up
      {
        QPolygon myTickUpArray(4);
        //draw a buffer first so bar shows up on dark images
        thepPainter->setPen( myBackgroundPen );
        myTickUpArray.putPoints(0,4,
            myOriginX                    ,  myOriginY                    ,
            myOriginX                    ,  myOriginY + myMajorTickSize  ,
            (myScaleBarWidthInt + myOriginX),  myOriginY + myMajorTickSize  ,
            (myScaleBarWidthInt + myOriginX),  myOriginY
            );
        thepPainter->drawPolyline(myTickUpArray);
        //now draw the bar itself in user selected color
        thepPainter->setPen( myForegroundPen );
        myTickUpArray.putPoints(0,4,
            myOriginX                    ,  myOriginY                    ,
            myOriginX                    ,  myOriginY + myMajorTickSize  ,
            (myScaleBarWidthInt + myOriginX),  myOriginY + myMajorTickSize  ,
            (myScaleBarWidthInt + myOriginX),  myOriginY
            );
        thepPainter->drawPolyline(myTickUpArray);
        break;
      }
    case 2: // Bar
      {
        QPolygon myBarArray(2);
        //draw a buffer first so bar shows up on dark images
        thepPainter->setPen( myBackgroundPen );
        myBarArray.putPoints(0,2,
            myOriginX,  
            (myOriginY + (myMajorTickSize/2)),
            (myScaleBarWidthInt + myOriginX),  (myOriginY + (myMajorTickSize/2))
            );
        thepPainter->drawPolyline(myBarArray);
        //now draw the bar itself in user selected color
        thepPainter->setPen( myForegroundPen );
        myBarArray.putPoints(0,2,
            myOriginX ,  (myOriginY + (myMajorTickSize/2)),
            (myScaleBarWidthInt + myOriginX),  (myOriginY + (myMajorTickSize/2))
            );
        thepPainter->drawPolyline(myBarArray);
        break;
      }
    case 3: // box
      {
        // Want square corners for a box
        myBackgroundPen.setJoinStyle( Qt::MiterJoin );
        myForegroundPen.setJoinStyle( Qt::MiterJoin );
        QPolygon myBoxArray(5);
        //draw a buffer first so bar shows up on dark images
        thepPainter->setPen( myBackgroundPen );
        myBoxArray.putPoints(0,5,
            myOriginX                    ,  myOriginY,
            (myScaleBarWidthInt + myOriginX),  myOriginY,
            (myScaleBarWidthInt + myOriginX), (myOriginY+myMajorTickSize),
            myOriginX                    , (myOriginY+myMajorTickSize),
            myOriginX                    ,  myOriginY
            );
        thepPainter->drawPolyline(myBoxArray);
        //now draw the bar itself in user selected color
        thepPainter->setPen( myForegroundPen );
        thepPainter->setBrush( QBrush( mColour, Qt::SolidPattern) );
        int midPointX = myScaleBarWidthInt/2 + myOriginX; 
        myBoxArray.putPoints(0,5,
            myOriginX                    ,  myOriginY,
            midPointX,  myOriginY,
            midPointX, (myOriginY+myMajorTickSize),
            myOriginX                    , (myOriginY+myMajorTickSize),
            myOriginX                    ,  myOriginY
            );
        thepPainter->drawPolygon(myBoxArray);

        thepPainter->setBrush( Qt::NoBrush );
        myBoxArray.putPoints(0,5,
            midPointX                    ,  myOriginY,
            (myScaleBarWidthInt + myOriginX),  myOriginY,
            (myScaleBarWidthInt + myOriginX), (myOriginY+myMajorTickSize),
            midPointX                    , (myOriginY+myMajorTickSize),
            midPointX                    ,  myOriginY
            );
        thepPainter->drawPolygon(myBoxArray);
        break;
      }
    default:
      std::cerr << "Unknown style\n";
  }

  //Do actual drawing of scale bar

  //
  //Do drawing of scale bar text
  //


  //Draw the minimum label buffer
  thepPainter->setPen( myBackColor );
  myFontWidth = myFontMetrics.width( "0" );
  myFontHeight = myFontMetrics.height();

  for (int i = 0-myBufferSize; i <= myBufferSize; i++)
  {
    for (int j = 0-myBufferSize; j <= myBufferSize; j++)
    {
      thepPainter->drawText( int(i +(myOriginX-(myFontWidth/2))),
          int(j + (myOriginY-(myFontHeight/4))) - myTextOffsetY,
          "0");
    }
  }

  //Draw minimum label
  thepPainter->setPen( myForeColor );

  thepPainter->drawText(
      int(myOriginX-(myFontWidth/2)),
      int(myOriginY-(myFontHeight/4)) - myTextOffsetY,
      "0"
      );

  //
  //Draw maximum label
  //
  thepPainter->setPen( myBackColor );
  myFontWidth = myFontMetrics.width( myScaleBarMaxLabel );
  myFontHeight = myFontMetrics.height();
  //first the buffer
  for (int i = 0-myBufferSize; i <= myBufferSize; i++)
  {
    for (int j = 0-myBufferSize; j <= myBufferSize; j++)
    {
      thepPainter->drawText( int(i + (myOriginX+myScaleBarWidthInt-(myFontWidth/2))),
          int(j + (myOriginY-(myFontHeight/4))) - myTextOffsetY,
          myScaleBarMaxLabel);
    }
  }
  //then the text itself
  thepPainter->setPen( myForeColor );
  thepPainter->drawText(
      int(myOriginX+myScaleBarWidthInt-(myFontWidth/2)),
      int(myOriginY-(myFontHeight/4)) - myTextOffsetY,
      myScaleBarMaxLabel
      );

  //
  //Draw unit label
  //
  thepPainter->setPen( myBackColor );
  myFontWidth = myFontMetrics.width( myScaleBarUnitLabel );
  myFontHeight = myFontMetrics.height();
  //first the buffer
  for (int i = 0-myBufferSize; i <= myBufferSize; i++)
  {
    for (int j = 0-myBufferSize; j <= myBufferSize; j++)
    {
      thepPainter->drawText( i + (myOriginX+myScaleBarWidthInt+myTextOffsetX),
          j + (myOriginY+myMajorTickSize) + myTextOffsetY,
          myScaleBarUnitLabel);
    }
  }
  //then the text itself
  thepPainter->setPen( myForeColor );
  thepPainter->drawText(
      myOriginX + myScaleBarWidthInt + myTextOffsetX,
      myOriginY + myMajorTickSize + myTextOffsetY,
      myScaleBarUnitLabel
      );
}
