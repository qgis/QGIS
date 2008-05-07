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
#include <QLinearGradient>

//other includes
#include <cmath>

#ifdef _MSC_VER
#define round(x)  ((x) >= 0 ? floor((x)+0.5) : floor((x)-0.5))
#endif

QgsQuickPrint::QgsQuickPrint()
{
  mPageSize = QPrinter::A4;
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
  mpMapRender=thepMapCanvas->mapRender();
  mMapBackgroundColour = thepMapCanvas->canvasColor();
}
void QgsQuickPrint::setMapRender(QgsMapRender * thepMapRender)
{
  mpMapRender=thepMapRender;
}
void QgsQuickPrint::setMapBackgroundColor(QColor theColor)
{
  mMapBackgroundColour = theColor;
}
void QgsQuickPrint::setPageSize (QPrinter::PageSize theSize)
{
  mPageSize = theSize;
}

void QgsQuickPrint::printMap()
{
  if ( mOutputFileName.isEmpty() )
  {
    return;
  }
  if ( mpMapRender == NULL )
  {
    return;
  }
  //ensure the user never ommitted the extension from the filename
  if ( !mOutputFileName.toUpper().endsWith ( ".PDF" ) )
  {
    mOutputFileName += ".pdf";
  }

  // Initialising the printer this way lets us find out what
  // the screen resolution is which we store and then 
  // reset the resolution of the printer after that...
  QPrinter myPrinter ( QPrinter::ScreenResolution ); 
  int myScreenResolutionDpi = myPrinter.resolution(); //try to get programmatically
  //
  // Try to force the printer resolution to 300dpi
  // to get past platform specific defaults in printer
  // resolution...
  //
  int myPrintResolutionDpi = 300;
  myPrinter.setResolution( myPrintResolutionDpi );
  myPrinter.setOutputFormat ( QPrinter::PdfFormat );
  qDebug( "Printing to page size"  +  pageSizeToString(mPageSize).toLocal8Bit());
  myPrinter.setPageSize ( mPageSize );
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
  //
  // Remember the size and dpi of the maprender
  // so we can restore it properly
  //
  int myOriginalDpi = mpMapRender->outputDpi();
  QSize myOriginalSize = mpMapRender->outputSize();
  int mySymbolScalingAmount = myPrintResolutionDpi / myOriginalDpi; 

  //define the font sizes and family
  int myMapTitleFontSize = 24;
  int myMapDateFontSize = 16;
  int myMapNameFontSize = 32;
  int myLegendFontSize = 12;
#ifdef Q_OS_LINUX//this sucks...
  myLegendFontSize -= 2;
#endif

#ifdef WIN32 //this sucks too...
   myMapTitleFontSize /= 2;
   myMapDateFontSize /= 2;
   myMapNameFontSize /= 2;
   myLegendFontSize /= 2;
#endif
  QString myFontFamily = "Arial";

  // Background colour for pixmaps
  QColor myLegendBackgroundColour = Qt::white;
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
  myMapPixmap.fill ( mMapBackgroundColour );
  QPainter myMapPainter;
  myMapPainter.begin( &myMapPixmap );
  // Now resize for print
  mpMapRender->setOutputSize( 
      QSize ( myMapDimensionX, myMapDimensionY ), myPrinter.resolution() ); 
  scalePointSymbols(mySymbolScalingAmount, ScaleUp);
  scaleTextLabels(mySymbolScalingAmount, ScaleUp);
  mpMapRender->render( &myMapPainter );

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
  //draw a rectangale around the legend frame 
  //@TODO make this user settable
  if (0==1) //put some real logic here
  {
    myPrintPainter.drawRect( 0, 0, myLegendDimensionX, myLegendDimensionY );
  }
  //get font metric and other vars needed
  QFontMetrics myLegendFontMetrics( myLegendFont, &myPrinter );
  int myLegendFontHeight = myLegendFontMetrics.height();
  int myLegendXPos = 0;
  int myLegendYPos = 0;
  int myLegendSpacer = myLegendFontHeight/2; //for vertical and horizontal spacing
  int myLegendVerticalSpacer = myLegendFontHeight/3; //for vertical between rows
  int myIconWidth = myLegendFontHeight;
  myPrintPainter.setFont( myLegendFont );
  QStringList myLayerSet = mpMapRender->layerSet();
  QStringListIterator myLayerIterator ( myLayerSet );
  //second clause below is to prevent legend spilling out the bottom
  while ( myLayerIterator.hasNext() && 
      myLegendYPos < myLegendDimensionY )
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
          myLegendXPos = 0 ;
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
          int myMaximumLabelWidth = myLegendDimensionX - myLegendXPos;
          QStringList myWrappedLayerNameList = wordWrap(myLayerName, 
              myLegendFontMetrics, 
              myLegendDimensionX - myIconWidth);
          //
          // Loop through wrapped legend label lines
          //
          QStringListIterator myLineWrapIterator(myWrappedLayerNameList);
          while (myLineWrapIterator.hasNext())
          {
            QString myLine = myLineWrapIterator.next();
            QRect myLegendItemRect (myLegendXPos,
                myLegendYPos,
                myLegendDimensionX - myIconWidth, 
                myLegendFontHeight);
            myPrintPainter.drawText( myLegendItemRect, Qt::AlignLeft, myLine );
            myLegendYPos += myLegendVerticalSpacer + myLegendFontHeight;
          }
        }
        else  //class breaks
        {
          // draw in the layer name first, after we loop for the class breaks
          QStringList myWrappedLayerNameList = wordWrap(myLayerName, 
                                    myLegendFontMetrics, 
                                    myLegendDimensionX - myIconWidth);
          // Check the wrapped layer name wont overrun the space we have
          // for the legend ...
          int myLabelHeight = myLegendFontHeight * 
                  myWrappedLayerNameList.count(); 
          if (myLegendYPos + myLabelHeight > myLegendDimensionY )
          {
            continue;
          }

          //
          // Loop through wrapped legend label lines
          //
          QStringListIterator myLineWrapIterator(myWrappedLayerNameList);
          while (myLineWrapIterator.hasNext()) 
          {
            QString myLine = myLineWrapIterator.next();
            myLegendXPos = myIconWidth;
            QRect myLegendItemRect (myLegendXPos,
                myLegendYPos,
                myLegendFontMetrics.width(myLine), 
                myLegendFontHeight);
            myPrintPainter.setPen( Qt::black );
            myPrintPainter.drawText( myLegendItemRect, Qt::AlignLeft, myLine );
            myLegendYPos += myLegendVerticalSpacer + myLegendFontHeight;
          }
          //
          // Loop through the class breaks
          //
          QListIterator<QgsSymbol *> myIterator ( mySymbolList );
          while ( myIterator.hasNext() && myLegendYPos < myLegendDimensionY)
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
              myPrintPainter.drawRect( 
                  myLegendXPos, myLegendYPos, myIconWidth, myIconWidth );
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
            //

            QStringList myWrappedLayerNameList = wordWrap(myLabel, 
                myLegendFontMetrics, 
                myLegendDimensionX - myLegendXPos);
            //
            // Loop through wrapped legend label lines
            //
            QStringListIterator myLineWrapIterator(myWrappedLayerNameList);
            while (myLineWrapIterator.hasNext())
            {
              QString myLine = myLineWrapIterator.next();
              // check if the text will overflow the space we have
              QRect myLegendItemRect (myLegendXPos,
                  myLegendYPos,
                  myLegendDimensionX - myIconWidth, 
                  myLegendFontHeight);
              myPrintPainter.drawText( myLegendItemRect, Qt::AlignLeft, myLine );
              myLegendYPos += myLegendVerticalSpacer + myLegendFontHeight;
            } //wordwrap loop
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
  qDebug(mLogo1File.toLocal8Bit());
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
  // use half the available space for the n.arrow
  // and the rest for the scale bar (see below)
  QPixmap myNorthArrow ( myLogoYDim/2, myLogoYDim/2 );
  myNorthArrow.fill ( Qt::white );
  QPainter myNorthPainter ( &myNorthArrow );
  QSvgRenderer mySvgRenderer ( mNorthArrowFile );
  mySvgRenderer.render ( &myNorthPainter );
  myPrintPainter.drawPixmap( myOriginX + ((myLogoXDim/2)) ,
      myOriginY , 
      myNorthArrow); 

  //
  // Draw the scale bar
  //
  myOriginY += myLogoYDim/2 + myVerticalSpacing;
  myPrintPainter.setViewport(myOriginX,
      myOriginY, 
      myOriginalViewport.width(),
      myOriginalViewport.height());
  renderPrintScaleBar(&myPrintPainter, mpMapRender, myLogoXDim);
  myPrintPainter.setViewport(myOriginalViewport); 

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
  //
  // Restore the map render to its former glory
  //
  mpMapRender->setOutputSize( myOriginalSize, myOriginalDpi); 
} 

void QgsQuickPrint::scaleTextLabels( int theScaleFactor, SymbolScalingType theDirection)
{
  if (0 >= theScaleFactor)
  {
    QgsDebugMsg ("QgsQuickPrintGui::scaleTextLabels invalid scale factor");
    return;
  }
  QStringList myLayerSet = mpMapRender->layerSet();
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
  QStringList myLayerSet = mpMapRender->layerSet();
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



void QgsQuickPrint::renderPrintScaleBar(QPainter * thepPainter, 
    QgsMapRender * thepMapRender,
    int theMaximumWidth)
{
  //hard coding some options for now
  bool mySnappingFlag = true;
  QColor mColour = Qt::black;
  // Hard coded sizes
  int myTextOffsetX=0;
  int myTextOffsetY=5;
  int myXMargin=20;
  int myYMargin=20;
  int myPreferredSize = theMaximumWidth - (myXMargin *2);
  double myActualSize = 0;
  int myBufferSize=1; //softcode this later
  QColor myBackColor = Qt::white; //used for text
  QColor myForeColor = Qt::black; //used for text

  //Get canvas dimensions
  //int myCanvasHeight = thepMapCanvas->height();

  //Get map units per pixel. This can be negative at times (to do with
  //projections) and that just confuses the rest of the code in this
  //function, so force to a positive number.
  double myMuppDouble = std::abs(thepMapRender->mupp());
  // 
  // Exit if the canvas width is 0 or layercount is 0 or QGIS will freeze
  int myLayerCount=thepMapRender->layerSet().count();
  if (!myLayerCount || !myMuppDouble) return;

  //Calculate size of scale bar for preferred number of map units
  double myScaleBarWidth = myPreferredSize;
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
  QGis::units myMapUnits=thepMapRender->mapUnits();
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
  //we divide by 2 because the max scale label
  //will be centered over the endpoint of the scale bar
  double myTotalScaleBarWidth = myScaleBarWidth + (myFontWidth/2);

  //determine the origin of scale bar (bottom right)
  //for x origin set things up so the scalebar is centered
  int myOriginX=(theMaximumWidth - myTotalScaleBarWidth)/2;
  int myOriginY=myYMargin;

  //Set pen to draw with
  QPen myForegroundPen( mColour, 2 );
  QPen myBackgroundPen( Qt::white, 3 );

  //Cast myScaleBarWidth to int for drawing
  int myScaleBarWidthInt = (int) myScaleBarWidth;

  //now draw the bar itself in user selected color
  thepPainter->setPen( myForegroundPen );
  //make a glossygradient for the background
  QGradientStops myStops;
  myStops << QGradientStop(0.0,QColor("#616161"));
  myStops << QGradientStop(0.5,QColor("#505050"));
  myStops << QGradientStop(0.6,QColor("#434343"));
  myStops << QGradientStop(1.0,QColor("#656565"));
  //draw again with the brush in the revers direction to complete teh glossiness
  QLinearGradient myReverseGlossyBrush(
                  QPointF(myOriginX,myOriginY +  myFontHeight*3), 
                  QPointF(myOriginX,myOriginY));
  thepPainter->setBrush(myReverseGlossyBrush);
  thepPainter->drawRect( 
      myOriginX, 
      myOriginY, 
      myOriginX + myScaleBarWidthInt,  
      myOriginY + myFontHeight
      );

  //
  //Do drawing of scale bar text
  //


  //Draw the minimum label buffer
  thepPainter->setPen( myBackColor );
  myFontWidth = myFontMetrics.width( "0" );

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
  //first the buffer
  for (int i = 0-myBufferSize; i <= myBufferSize; i++)
  {
    for (int j = 0-myBufferSize; j <= myBufferSize; j++)
    {
      thepPainter->drawText( i + (myOriginX+myScaleBarWidthInt+myTextOffsetX),
          j + myOriginY + myFontHeight + (myFontHeight*2.5) + myTextOffsetY,
          myScaleBarUnitLabel);
    }
  }
  //then the text itself
  thepPainter->setPen( myForeColor );
  thepPainter->drawText(
      myOriginX + myScaleBarWidthInt + myTextOffsetX,
      myOriginY + myFontHeight + (myFontHeight*2.5) +  myTextOffsetY,
      myScaleBarUnitLabel
      );
}

QStringList QgsQuickPrint::wordWrap(QString theString, 
                                    QFontMetrics theMetrics, 
                                    int theWidth)
{
  //iterate the string 
  QStringList myList;
  QString myCumulativeLine="";
  QString myStringToPreviousSpace="";
  int myPreviousSpacePos=0;
  for (int i=0; i < theString.count(); ++i)
  {
    QChar myChar = theString.at(i);
    if (myChar == QChar(' '))
    {
      myStringToPreviousSpace = myCumulativeLine;
      myPreviousSpacePos=i;
    }
    myCumulativeLine += myChar;
    if (theMetrics.width(myCumulativeLine) >= theWidth)
    {
      //time to wrap
      //@todo deal with long strings that have no spaces
      //forcing a break at current pos...
      myList << myStringToPreviousSpace.trimmed();
      i = myPreviousSpacePos;
      myStringToPreviousSpace = "";
      myCumulativeLine = "";
    }
  }//end of i loop
  //add whatever is left in the string to the list
  if (!myCumulativeLine.trimmed().isEmpty())
  {
      myList << myCumulativeLine.trimmed();
  }

  //qDebug("Wrapped legend entry:");
  //qDebug(theString);
  //qDebug(myList.join("\n").toLocal8Bit());
  return myList;

}
QString QgsQuickPrint::pageSizeToString(QPrinter::PageSize theSize)
{
  if (theSize==QPrinter::A0) return "QPrinter::A0";
  if (theSize==QPrinter::A1) return "QPrinter::A1";
  if (theSize==QPrinter::A2) return "QPrinter::A2";
  if (theSize==QPrinter::A3) return "QPrinter::A3";
  if (theSize==QPrinter::A4) return "QPrinter::A4";
  if (theSize==QPrinter::A5) return "QPrinter::A5";
  if (theSize==QPrinter::A6) return "QPrinter::A6";
  if (theSize==QPrinter::A7) return "QPrinter::A7";
  if (theSize==QPrinter::A8) return "QPrinter::A8";
  if (theSize==QPrinter::A9) return "QPrinter::A9";
  if (theSize==QPrinter::B0) return "QPrinter::B0";
  if (theSize==QPrinter::B1) return "QPrinter::B1";
  if (theSize==QPrinter::B10) return "QPrinter::B10";
  if (theSize==QPrinter::B2) return "QPrinter::B2";
  if (theSize==QPrinter::B3) return "QPrinter::B3";
  if (theSize==QPrinter::B4) return "QPrinter::B4";
  if (theSize==QPrinter::B5) return "QPrinter::B5";
  if (theSize==QPrinter::B6) return "QPrinter::B6";
  if (theSize==QPrinter::B7) return "QPrinter::B7";
  if (theSize==QPrinter::B8) return "QPrinter::B8";
  if (theSize==QPrinter::B9) return "QPrinter::B9";
  if (theSize==QPrinter::C5E) return "QPrinter::C5E";
  if (theSize==QPrinter::Comm10E) return "QPrinter::Comm10E";
  if (theSize==QPrinter::DLE) return "QPrinter::DLE";
  if (theSize==QPrinter::Executive) return "QPrinter::Executive";
  if (theSize==QPrinter::Folio) return "QPrinter::Folio";
  if (theSize==QPrinter::Ledger) return "QPrinter::Ledger";
  if (theSize==QPrinter::Legal) return "QPrinter::Legal";
  if (theSize==QPrinter::Letter) return "QPrinter::Letter";
  //falback
  return "QPrinter::A4";

}

QPrinter::PageSize QgsQuickPrint::stringToPageSize(QString theSize)
{
  if (theSize=="QPrinter::A0") return QPrinter::A0;
  if (theSize=="QPrinter::A1") return QPrinter::A1;
  if (theSize=="QPrinter::A2") return QPrinter::A2;
  if (theSize=="QPrinter::A3") return QPrinter::A3;
  if (theSize=="QPrinter::A4") return QPrinter::A4;
  if (theSize=="QPrinter::A5") return QPrinter::A5;
  if (theSize=="QPrinter::A6") return QPrinter::A6;
  if (theSize=="QPrinter::A7") return QPrinter::A7;
  if (theSize=="QPrinter::A8") return QPrinter::A8;
  if (theSize=="QPrinter::A9") return QPrinter::A9;
  if (theSize=="QPrinter::B0") return QPrinter::B0;
  if (theSize=="QPrinter::B1") return QPrinter::B1;
  if (theSize=="QPrinter::B10") return QPrinter::B10;
  if (theSize=="QPrinter::B2") return QPrinter::B2;
  if (theSize=="QPrinter::B3") return QPrinter::B3;
  if (theSize=="QPrinter::B4") return QPrinter::B4;
  if (theSize=="QPrinter::B5") return QPrinter::B5;
  if (theSize=="QPrinter::B6") return QPrinter::B6;
  if (theSize=="QPrinter::B7") return QPrinter::B7;
  if (theSize=="QPrinter::B8") return QPrinter::B8;
  if (theSize=="QPrinter::B9") return QPrinter::B9;
  if (theSize=="QPrinter::C5E") return QPrinter::C5E;
  if (theSize=="QPrinter::Comm10E") return QPrinter::Comm10E;
  if (theSize=="QPrinter::DLE") return QPrinter::DLE;
  if (theSize=="QPrinter::Executive") return QPrinter::Executive;
  if (theSize=="QPrinter::Folio") return QPrinter::Folio;
  if (theSize=="QPrinter::Ledger") return QPrinter::Ledger;
  if (theSize=="QPrinter::Legal") return QPrinter::Legal;
  if (theSize=="QPrinter::Letter") return QPrinter::Letter;
  //falback
  return QPrinter::A4;

}



