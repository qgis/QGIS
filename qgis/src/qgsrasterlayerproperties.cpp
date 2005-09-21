/***************************************************************************
  qgsrasterlayerproperties.cpp  -  description
  -------------------
begin                : 1/1/2004
copyright            : (C) 2004 Tim Sutton
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


#include "qgsrasterlayerproperties.h"
#include <qlabel.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qstring.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qtable.h>
#include <qtextedit.h>
#include <qpainter.h>
#include <qfont.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qtextbrowser.h>
#include <qspinbox.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qglobal.h>

const char * const ident = 
"$Id";


    QgsRasterLayerProperties::QgsRasterLayerProperties(QgsMapLayer * lyr)
: rasterLayer( dynamic_cast<QgsRasterLayer*>(lyr) )
{

  // set up the scale based layer visibility stuff....
  chkUseScaleDependentRendering->setChecked(lyr->scaleBasedVisibility());
  spinMinimumScale->setValue((int)lyr->minScale());
  spinMaximumScale->setValue((int)lyr->maxScale());

  // build GUI components

  cboColorMap->insertItem(tr("Grayscale"));
  cboColorMap->insertItem(tr("Pseudocolor"));
  cboColorMap->insertItem(tr("Freak Out"));

  //set the std deviations to be plotted combo box
  cboStdDev->insertItem("0");
  cboStdDev->insertItem("0.5");
  cboStdDev->insertItem("0.75");
  cboStdDev->insertItem("1");
  cboStdDev->insertItem("1.25");
  cboStdDev->insertItem("1.5");
  cboStdDev->insertItem("1.75");
  cboStdDev->insertItem("2");
  cboStdDev->insertItem("2.25");
  cboStdDev->insertItem("2.5");
  cboStdDev->insertItem("2.75");
  cboStdDev->insertItem("3");

  //
  // Set up the combo boxes that contain band lists using the qstring list generated above
  //

  if (rasterLayer->getRasterLayerType()
          == QgsRasterLayer::PALETTE) //paletted layers have hard coded color entries
  {
    cboRed->insertItem("Red");
    cboGreen->insertItem("Red");
    cboBlue->insertItem("Red");

    cboRed->insertItem("Green");
    cboGreen->insertItem("Green");
    cboBlue->insertItem("Green");

    cboRed->insertItem("Blue");
    cboGreen->insertItem("Blue");
    cboBlue->insertItem("Blue");

    cboRed->insertItem("Not Set");
    cboGreen->insertItem("Not Set");
    cboBlue->insertItem("Not Set");

    cboGray->insertItem("Red");
    cboGray->insertItem("Green");
    cboGray->insertItem("Blue");
    cboGray->insertItem("Not Set");
    
    lstHistogramLabels->insertItem(tr("Palette"));
  }
  else                   // all other layer types use band name entries only
  {
#ifdef QGISDEBUG
    std::cout << "Populating combos for non paletted layer" << std::endl;
#endif

    //
    // Get a list of band names
    //
    QStringList myBandNameQStringList;

    int myBandCountInt = rasterLayer->getBandCount();
#ifdef QGISDEBIG
    std::cout << "Looping though " << myBandCountInt << " image layers to get their names " << std::endl;
#endif
    for (int myIteratorInt = 1;
            myIteratorInt <= myBandCountInt;
            ++myIteratorInt)
    {
      //find out the name of this band
      QString myRasterBandNameQString = rasterLayer->getRasterBandName(myIteratorInt) ;

      //
      //add the band to the histogram tab
      //
      QPixmap myPixmap(10,10);
          
      if (myBandCountInt==1) //draw single band images with black
      {
      myPixmap.fill( Qt::black );
      }
      else if (myIteratorInt==1)
      {
      myPixmap.fill( Qt::red );
      }
      else if (myIteratorInt==2)
      {
      myPixmap.fill( Qt::green );
      }
      else if (myIteratorInt==3)
      {
      myPixmap.fill( Qt::blue );
      }
      else if (myIteratorInt==4)
      {
      myPixmap.fill( Qt::magenta );
      }
      else if (myIteratorInt==5)
      {
      myPixmap.fill( Qt::darkRed );
      }
      else if (myIteratorInt==6)
      {
      myPixmap.fill( Qt::darkGreen );
      }
      else if (myIteratorInt==7)
      {
      myPixmap.fill( Qt::darkBlue );
      }
      else
      {
      myPixmap.fill( Qt::gray );
      }
      lstHistogramLabels->insertItem(myPixmap,myRasterBandNameQString);
      //keep a list of band names for later use
      myBandNameQStringList.append(myRasterBandNameQString);
    }


    for (QStringList::Iterator myIterator = myBandNameQStringList.begin(); 
            myIterator != myBandNameQStringList.end(); 
            ++myIterator)
    {
      QString myQString = *myIterator;
#ifdef QGISDEBUG

      std::cout << "Inserting : " << myQString.local8Bit() << std::endl;
#endif

      cboGray->insertItem(myQString);
      cboRed->insertItem(myQString);
      cboGreen->insertItem(myQString);
      cboBlue->insertItem(myQString);
    }
    cboRed->insertItem("Not Set");
    cboGreen->insertItem("Not Set");
    cboBlue->insertItem("Not Set");
    cboGray->insertItem("Not Set");
  }

  //
#if defined(WIN32) || defined(Q_OS_MACX)
  QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
  QPixmap myPyramidPixmap(QString(PKGDATAPATH) + QString("/images/icons/pyramid.png"));
  QPixmap myNoPyramidPixmap(QString(PKGDATAPATH) + QString("/images/icons/no_pyramid.png"));

  RasterPyramidList myPyramidList = rasterLayer->buildRasterPyramidList();
  RasterPyramidList::iterator myRasterPyramidIterator;

  for ( myRasterPyramidIterator=myPyramidList.begin();
          myRasterPyramidIterator != myPyramidList.end();
          ++myRasterPyramidIterator )
  {
    if ((*myRasterPyramidIterator).existsFlag==true)
    {
      lbxPyramidResolutions->insertItem(myPyramidPixmap,
              QString::number((*myRasterPyramidIterator).xDimInt) + QString(" x ") + 
              QString::number((*myRasterPyramidIterator).yDimInt)); 
    }
    else
    {
      lbxPyramidResolutions->insertItem(myNoPyramidPixmap,
              QString::number((*myRasterPyramidIterator).xDimInt) + QString(" x ") + 
              QString::number((*myRasterPyramidIterator).yDimInt)); 
    }
  }
  
  //draw the histogram
  //pbnHistRefresh_clicked();
  
 // update based on lyr's current state
  sync();  
} // QgsRasterLayerProperties ctor


QgsRasterLayerProperties::~QgsRasterLayerProperties()
{
  delete rasterLayer;
}


void QgsRasterLayerProperties::apply()
{
  // set up the scale based layer visibility stuff....
  rasterLayer->setScaleBasedVisibility(chkUseScaleDependentRendering->isChecked());
  rasterLayer->setMinScale(spinMinimumScale->value());
  rasterLayer->setMaxScale(spinMaximumScale->value());

  rasterLayer->setTransparency(static_cast < unsigned int >(255 - sliderTransparency->value()));
  //set the std deviations to be plotted
  rasterLayer->setStdDevsToPlot(cboStdDev->currentText().toDouble());
  //set whether the layer histogram should be inverted
  if (cboxInvertColorMap->isChecked()
     )
  {
    rasterLayer->setInvertHistogramFlag(true);
  }
  else
  {
    rasterLayer->setInvertHistogramFlag(false);
  }
  //now set the color -> band mapping combos to the correct values
  rasterLayer->setRedBandName(cboRed->currentText());
  rasterLayer->setGreenBandName(cboGreen->currentText());
  rasterLayer->setBlueBandName(cboBlue->currentText());
  rasterLayer->setGrayBandName(cboGray->currentText());
  //set the appropriate color ramping type
  if (cboColorMap->currentText() == tr("Pseudocolor"))
  {
    rasterLayer->setColorRampingType(QgsRasterLayer::BLUE_GREEN_RED);  
  }
  else if (cboColorMap->currentText() == tr("Freak Out"))
  {
    rasterLayer->setColorRampingType(QgsRasterLayer::FREAK_OUT);  
  }
  //set the appropriate render style
  if ((grpBoxGrayscale->isEnabled())
          && (rbtnSingleBand->isChecked()))
  {
    //
    // Grayscale
    //
    if (rasterLayer->rasterLayerType == QgsRasterLayer::GRAY_OR_UNDEFINED)
    {

      if (cboColorMap->currentText() != tr("Grayscale"))
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: SINGLE_BAND_PSEUDO_COLOR" << std::endl;
#endif

        rasterLayer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: SINGLE_BAND_GRAY" << std::endl;
#endif

        rasterLayer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_GRAY);
      }
    }
    //
    // Paletted Image
    //
    else if (rasterLayer->rasterLayerType == QgsRasterLayer::PALETTE)
    {
      if (cboColorMap->currentText() != tr("Grayscale"))
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: PALETTED_SINGLE_BAND_PSEUDO_COLOR" << std::endl;
#endif

        rasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR);
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: PALETTED_SINGLE_BAND_GRAY" << std::endl;
#endif
#ifdef QGISDEBUG

        std::cout << "Combo value : " << cboGray->currentText().local8Bit() << " GrayBand Mapping : " << rasterLayer->
            getGrayBandName().local8Bit() << std::endl;
#endif

        rasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY);
      }
    }
    //
    // Mutltiband
    //
    else if (rasterLayer->rasterLayerType == QgsRasterLayer::MULTIBAND)
    {
      if (cboColorMap->currentText() != tr("Grayscale"))
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to ::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR " << std::endl;
#endif

        rasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR);
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: MULTI_BAND_SINGLE_BAND_GRAY" << std::endl;
        std::cout << "Combo value : " << cboGray->currentText().local8Bit() << " GrayBand Mapping : " << rasterLayer->
            getGrayBandName().local8Bit() << std::endl;
#endif

        rasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_SINGLE_BAND_GRAY);

      }
    }
  }                           //end of grayscale box enabled and rbtnsingleband checked
  else                          //assume that rbtnThreeBand is checked and render in rgb color
  {
    //set the grayscale color table type if the groupbox is enabled

    if (rasterLayer->rasterLayerType == QgsRasterLayer::PALETTE)
    {
#ifdef QGISDEBUG
      std::cout << "Setting Raster Drawing Style to :: PALETTED_MULTI_BAND_COLOR" << std::endl;
#endif

      rasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_MULTI_BAND_COLOR);
    }
    else if (rasterLayer->rasterLayerType == QgsRasterLayer::MULTIBAND)
    {

#ifdef QGISDEBUG
      std::cout << "Setting Raster Drawing Style to :: MULTI_BAND_COLOR" << std::endl;
#endif

      rasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_COLOR);
    }

  }
  
  rasterLayer->setLayerName(leDisplayName->text());
  
  //update the legend pixmap
  pixmapLegend->setPixmap(rasterLayer->getLegendQPixmap());
  pixmapLegend->setScaledContents(true);
  pixmapLegend->repaint(false);
  rasterLayer->updateItemPixmap();
  
  //see if the user would like debug overlays
  if (cboxShowDebugInfo->isChecked()
          == true)
  {
    rasterLayer->setShowDebugOverlayFlag(true);
  }
  else
  {
    rasterLayer->setShowDebugOverlayFlag(false);
  }
  //
  // update histogram clipping ranges from the advanced symbology tab
  //
  rasterLayer->setMinRedDouble(static_cast < double >(sliderMinRed->value()));
  rasterLayer->setMaxRedDouble(static_cast < double >(255 - sliderMaxRed->value()));
  rasterLayer->setMinGreenDouble(static_cast < double >(sliderMinGreen->value()));
  rasterLayer->setMaxGreenDouble(static_cast < double >(255 - sliderMaxGreen->value()));
  rasterLayer->setMinBlueDouble(static_cast < double >(sliderMinBlue->value()));
  rasterLayer->setMaxBlueDouble(static_cast < double >(255 - sliderMaxBlue->value()));
  rasterLayer->setMinGrayDouble(static_cast < double >(sliderMinGray->value()));
  rasterLayer->setMaxGrayDouble(static_cast < double >(255 - sliderMaxGray->value()));
  //get the thumbnail for the layer
  QPixmap myQPixmap = QPixmap(pixmapThumbnail->width(),pixmapThumbnail->height());
  rasterLayer->drawThumbnail(&myQPixmap);
  pixmapThumbnail->setPixmap(myQPixmap);
  //make sure the layer is redrawn
  rasterLayer->refreshLegend();
  rasterLayer->triggerRepaint();
}

void QgsRasterLayerProperties::accept()
{
  apply();
  close();
}
void QgsRasterLayerProperties::sliderTransparency_valueChanged(int theValue)
{
  //set the transparency percentage label to a suitable value
  int myInt = static_cast < int >((theValue / 255.0) * 100);  //255.0 to prevent integer division
  lblTransparencyPercent->setText(QString::number(myInt) + "%");
}

void QgsRasterLayerProperties::sliderMaxRed_valueChanged(int)
{
  //the 255- is used because a vertical qslider has its max value at the bottom and
  //we want it to appear to the user that the max value is at the top, so we invert its value
  if ((255 - sliderMaxRed->value()) < sliderMinRed->value())
  {
    sliderMinRed->setValue(255 - sliderMaxRed->value());
  }
  makeScalePreview("red");
}


void QgsRasterLayerProperties::sliderMinRed_valueChanged(int)
{
  //the 255- is used because a vertical qslider has its max value at the bottom and
  //we want it to appear to the user that the max value is at the top, so we invert its value
  if ((255 - sliderMaxRed->value()) < sliderMinRed->value())
  {
    sliderMaxRed->setValue(255 - sliderMinRed->value());
  }
  makeScalePreview("red");
}


void QgsRasterLayerProperties::sliderMaxBlue_valueChanged(int)
{
  //the 255- is used because a vertical qslider has its max value at the bottom and
  //we want it to appear to the user that the max value is at the top, so we invert its value
  if ((255 - sliderMaxBlue->value()) < sliderMinBlue->value())
  {
    sliderMinBlue->setValue(255 - sliderMaxBlue->value());
  }
  makeScalePreview("blue");
}


void QgsRasterLayerProperties::sliderMinBlue_valueChanged(int)
{
  //the 255- is used because a vertical qslider has its max value at the bottom and
  //we want it to appear to the user that the max value is at the top, so we invert its value
  if ((255 - sliderMaxBlue->value()) < sliderMinBlue->value())
  {
    sliderMaxBlue->setValue(255 - sliderMinBlue->value());
  }

  makeScalePreview("blue");
}


void QgsRasterLayerProperties::sliderMaxGreen_valueChanged(int)
{
  //the 255- is used because a vertical qslider has its max value at the bottom and
  //we want it to appear to the user that the max value is at the top, so we invert its value
  if ((255 - sliderMaxGreen->value()) < sliderMinGreen->value())
  {
    sliderMinGreen->setValue(255 - sliderMaxGreen->value());
  }

  makeScalePreview("green");
}


void QgsRasterLayerProperties::sliderMinGreen_valueChanged(int)
{
  //the 255- is used because a vertical qslider has its max value at the bottom and
  //we want it to appear to the user that the max value is at the top, so we invert its value

  if ((255 - sliderMaxGreen->value()) < sliderMinGreen->value())
  {
    sliderMaxGreen->setValue(255 - sliderMinGreen->value());
  }
  makeScalePreview("green");
}


void QgsRasterLayerProperties::sliderMaxGray_valueChanged(int)
{
  //the 255- is used because a vertical qslider has its max value at the bottom and
  //we want it to appear to the user that the max value is at the top, so we invert its value

  if ((255 - sliderMaxGray->value()) < sliderMinGray->value())
  {
    sliderMinGray->setValue(255 - sliderMaxGray->value());
  }
  makeScalePreview("gray");
}


void QgsRasterLayerProperties::sliderMinGray_valueChanged(int)
{
  //the 255- is used because a vertical qslider has its max value at the bottom and
  //we want it to appear to the user that the max value is at the top, so we invert its value
  if ((255 - sliderMaxGray->value()) < sliderMinGray->value())
  {
    sliderMaxGray->setValue(255 - sliderMinGray->value());
  }
  makeScalePreview("gray");
}



void QgsRasterLayerProperties::makeScalePreview(QString theColor)
{
  double myMinDouble = 0;
  double myMaxDouble = 255;
  double myRedDouble = 0;
  double myBlueDouble = 0;
  double myGreenDouble = 0;
  unsigned int myTransparencyInt = sliderTransparency->value();
  //the 255- is used because a vertical qslider has its max value at the bottom and
  //we want it to appear to the user that the max value is at the top, so we invert its value
  if (theColor == "red")
  {
    myMinDouble = sliderMinRed->value();
    myMaxDouble = 255 - sliderMaxRed->value();
    myRedDouble = myMaxDouble;
  }
  else if (theColor == "green")
  {
    myMinDouble = sliderMinGreen->value();
    myMaxDouble = 255 - sliderMaxGreen->value();
  }
  else if (theColor == "blue")
  {
    myMinDouble = sliderMinBlue->value();
    myMaxDouble = 255 - sliderMaxBlue->value();
  }
  else if (theColor == "gray")
  {
    myMinDouble = sliderMinGray->value();
    myMaxDouble = 255 - sliderMaxGray->value();
  }
  QImage myQImage = QImage(100, 100, 32); //32bpp
  double myRangeDouble = myMaxDouble - myMinDouble;
  double myDecrementDouble = myRangeDouble / 100;
  //std::cout << "Decrementing " << theColor << " by : " << myDecrementDouble << std::endl;
  if (myDecrementDouble == 0)
    return;
  for (int myRowInt = 99; myRowInt >= 0; myRowInt = myRowInt - 1)
  {
    //reset the max value that the scale starts at
    if (theColor == "red")
    {
      myRedDouble = myMaxDouble;
    }
    else if (theColor == "green")
    {
      myGreenDouble = myMaxDouble;
    }
    else if (theColor == "blue")
    {
      myBlueDouble = myMaxDouble;
    }
    else if (theColor == "gray")
    {
      myRedDouble = myMaxDouble;
      myGreenDouble = myMaxDouble;
      myBlueDouble = myMaxDouble;
    }
    for (int myColInt = 99; myColInt >= 0; myColInt--)
    {
      if (theColor == "red")
      {
        myRedDouble -= myDecrementDouble;
      }
      else if (theColor == "green")
      {
        myGreenDouble -= myDecrementDouble;
      }
      else if (theColor == "blue")
      {
        myBlueDouble -= myDecrementDouble;
      }
      else if (theColor == "gray")
      {
        myRedDouble -= myDecrementDouble;
        myGreenDouble -= myDecrementDouble;
        myBlueDouble -= myDecrementDouble;
      }
      //std::cout << "R " << myRedDouble << " G " << myGreenDouble << " B " << myBlueDouble << std::endl;
      myQImage.setPixel(myColInt, myRowInt,
              qRgb((unsigned int) myRedDouble, (unsigned int) myGreenDouble, (unsigned int) myBlueDouble));
    }
  }
  // Create a pixmap the same size as the image - to be placed in the pixmalLabel
  QPixmap *myQPixmap = new QPixmap(100, 100);

  //
  // Draw a text alabel onto the pixmap showing the min max value
  //
  QPainter myQPainter(myQPixmap);
  myQPainter.rotate(-45);
#if QT_VERSION < 0x040000
  myQPainter.drawImage(-70, 0, myQImage.scale(140, 140));  // TODO: maybe should add ScaleMin ?
#else
  myQPainter.drawImage(-70, 0, myQImage.scaled(140, 140)); // TODO: maybe should add Qt::KeepAspectRatio ?
#endif
  myQPainter.rotate(45);
  QFont myQFont("arial", 18, QFont::Bold);
  myQPainter.setFont(myQFont);
  myQPainter.setPen(Qt::white);
  myQPainter.drawText(15, 50, QString::number(static_cast < unsigned int >(myMinDouble)) + " - " + QString::number(static_cast
              <
              unsigned int
              >(myMaxDouble)));

  //now draw the image into the relevant pixmap label
  if (theColor == "red")
  {
    pixmapScaleRed->setScaledContents(true);
    pixmapScaleRed->setPixmap(*myQPixmap);
    pixmapScaleRed->repaint(false);
  }
  else if (theColor == "green")
  {
    pixmapScaleGreen->setScaledContents(true);
    pixmapScaleGreen->setPixmap(*myQPixmap);
    pixmapScaleGreen->repaint(false);
  }
  else if (theColor == "blue")
  {
    pixmapScaleBlue->setScaledContents(true);
    pixmapScaleBlue->setPixmap(*myQPixmap);
    pixmapScaleBlue->repaint(false);
  }
  else if (theColor == "gray")
  {
    pixmapScaleGray->setScaledContents(true);
    pixmapScaleGray->setPixmap(*myQPixmap);
    pixmapScaleGray->repaint(false);
  }
}


void QgsRasterLayerProperties::rbtnSingleBand_toggled(bool)
{}


void QgsRasterLayerProperties::rbtnThreeBand_toggled(bool)
{}

void QgsRasterLayerProperties::buttonBuildPyramids_clicked()
{

  //
  // Go through the list marking any files that are selected in the listview
  // as true so that we can generate pyramids for them.
  //
  RasterPyramidList myPyramidList = rasterLayer->buildRasterPyramidList();
  for ( unsigned int myCounterInt = 0; myCounterInt < lbxPyramidResolutions->count(); myCounterInt++ )
  {
    QListBoxItem *myItem = lbxPyramidResolutions->item( myCounterInt );
    if ( myItem->isSelected() )
    {
      //mark to be pyramided
      myPyramidList[myCounterInt].existsFlag=true;
    }
  }
  //
  // Ask raster layer to build the pyramids
  //
  rasterLayer->buildPyramids(myPyramidList,cboResamplingMethod->currentText());  
  //
  // repopulate the pyramids list
  //
  lbxPyramidResolutions->clear();
#if defined(WIN32) || defined(Q_OS_MACX)
  QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
  QPixmap myPyramidPixmap(QString(PKGDATAPATH) + QString("/images/icons/pyramid.png"));
  QPixmap myNoPyramidPixmap(QString(PKGDATAPATH) + QString("/images/icons/no_pyramid.png"));
  RasterPyramidList::iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator=myPyramidList.begin();
          myRasterPyramidIterator != myPyramidList.end();
          ++myRasterPyramidIterator )
  {
    if ((*myRasterPyramidIterator).existsFlag==true)
    {
      lbxPyramidResolutions->insertItem(myPyramidPixmap,
              QString::number((*myRasterPyramidIterator).xDimInt) + QString(" x ") + 
              QString::number((*myRasterPyramidIterator).yDimInt)); 
    }
    else
    {
      lbxPyramidResolutions->insertItem(myNoPyramidPixmap,
              QString::number((*myRasterPyramidIterator).xDimInt) + QString(" x ") + 
              QString::number((*myRasterPyramidIterator).yDimInt)); 
    }
  }
  //update the legend pixmap
  pixmapLegend->setPixmap(rasterLayer->getLegendQPixmap());
  pixmapLegend->setScaledContents(true);
  pixmapLegend->repaint(false);
  rasterLayer->updateItemPixmap();
  //populate the metadata tab's text browser widget with gdal metadata info
  txtbMetadata->setText(rasterLayer->getMetadata());
}


/**
  @note moved from ctor

  Previously this dialog was created anew with each right-click pop-up menu
  invokation.  Changed so that the dialog always exists after first
  invocation, and is just re-synchronized with its layer's state when
  re-shown.

*/
void QgsRasterLayerProperties::sync()
{
  // XXX Remove the advanced symbology widget and debug overlay for 0.1 release

  tabSymbology->removePage(TabPage);
  cboxShowDebugInfo->hide();

  //get the thumbnail for the layer
  QPixmap myQPixmap = QPixmap(pixmapThumbnail->width(),pixmapThumbnail->height());
  rasterLayer->drawThumbnail(&myQPixmap);
  pixmapThumbnail->setPixmap(myQPixmap);

  //populate the metadata tab's text browser widget with gdal metadata info
  txtbMetadata->setText(rasterLayer->getMetadata());
  //tabSymbology->removePage(tabMetadata);
  //display the raster dimensions and no data value
  lblColumns->setText(tr("Columns:") + QString::number(rasterLayer->getRasterXDim()));
  lblRows->setText(tr("Rows:") + QString::number(rasterLayer->getRasterYDim()));
  lblNoData->setText(tr("No Data:") + QString::number(rasterLayer->getNoDataValue()));

  //these properties (layername and label) are provided by the qgsmaplayer superclass
  leLayerSource->setText(rasterLayer->source());
  leDisplayName->setText(rasterLayer->name());

  //update the debug checkbox
  cboxShowDebugInfo->setChecked(rasterLayer->getShowDebugOverlayFlag());

  //update the legend pixmap on this dialog
  pixmapLegend->setPixmap(rasterLayer->getLegendQPixmap());
  pixmapLegend->setScaledContents(true);
  pixmapLegend->repaint(false);

  //set the palette pixmap
  pixmapPalette->setPixmap(rasterLayer->getPaletteAsPixmap());
  pixmapPalette->setScaledContents(true);
  pixmapPalette->repaint(false);

  //set the transparency slider
  sliderTransparency->setValue(255 - rasterLayer->getTransparency());
  //update the transparency percentage label
  sliderTransparency_valueChanged(255 - rasterLayer->getTransparency());
  //decide whether user can change rgb settings

  switch (rasterLayer->getDrawingStyle())
  {
      case QgsRasterLayer::SINGLE_BAND_GRAY:
          rbtnSingleBand->toggle();
          rbtnThreeBand->setEnabled(false);
          rbtnSingleBand->setEnabled(true);
          break;
      case QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR:
          rbtnSingleBand->toggle();
          rbtnThreeBand->setEnabled(false);
          rbtnSingleBand->setEnabled(true);
          break;
      case QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY:
          rbtnSingleBand->toggle();
          rbtnThreeBand->setEnabled(true);
          rbtnSingleBand->setEnabled(true);
          break;
      case QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR:
          rbtnSingleBand->toggle();
          rbtnThreeBand->setEnabled(true);
          rbtnSingleBand->setEnabled(true);
          break;
      case QgsRasterLayer::PALETTED_MULTI_BAND_COLOR:
          rbtnThreeBand->toggle();
          rbtnThreeBand->setEnabled(true);
          rbtnSingleBand->setEnabled(true);
          break;
      case QgsRasterLayer::MULTI_BAND_SINGLE_BAND_GRAY:
          rbtnSingleBand->toggle();
          rbtnThreeBand->setEnabled(true);
          rbtnSingleBand->setEnabled(true);
          break;
      case QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR:
          rbtnSingleBand->toggle();
          rbtnThreeBand->setEnabled(true);
          rbtnSingleBand->setEnabled(true);
          break;
      case QgsRasterLayer::MULTI_BAND_COLOR:
          rbtnThreeBand->toggle();
          rbtnThreeBand->setEnabled(true);
          rbtnSingleBand->setEnabled(true);
          break;
      default:
          break;
  }


  if (rasterLayer->getRasterLayerType() == QgsRasterLayer::MULTIBAND)
  {
    //multiband images can also be rendered as single band (using only one of the bands)
    txtSymologyNotes->
        setText(tr
                ("<h3>Multiband Image Notes</h3><p>This is a multiband image. You can choose to render it as grayscale or color (RGB). For color images, you can associate bands to colors arbitarily. For example, if you have a seven band landsat image, you may choose to render it as:</p><ul><li>Visible Blue (0.45 to 0.52 microns) - not mapped</li><li>Visible Green (0.52 to 0.60 microns) - not mapped</li></li>Visible Red (0.63 to 0.69 microns) - mapped to red in image</li><li>Near Infrared (0.76 to 0.90 microns) - mapped to green in image</li><li>Mid Infrared (1.55 to 1.75 microns) - not mapped</li><li>Thermal Infrared (10.4 to 12.5 microns) - not mapped</li><li>Mid Infrared (2.08 to 2.35 microns) - mapped to blue in image</li></ul>"));
  }
  else if (rasterLayer->getRasterLayerType() == QgsRasterLayer::PALETTE)
  {
    //paletted images (e.g. tif) can only be rendered as three band rgb images
    txtSymologyNotes->
        setText(tr
                ("<h3>Paletted Image Notes</h3> <p>This image uses a fixed color palette. You can remap these colors in different combinations e.g.</p><ul><li>Red - blue in image</li><li>Green - blue in image</li><li>Blue - green in image</li></ul>"));
  }
  else                        //only grayscale settings allowed
  {
    //grayscale images can only be rendered as singleband
    txtSymologyNotes->
        setText(tr
                ("<h3>Grayscale Image Notes</h3> <p>You can remap these grayscale colors to a pseudocolor image using an automatically generated color ramp.</p>"));
  }


  //
  // Populate the various controls on the form
  //
  if (rasterLayer->getDrawingStyle() == QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR ||
          rasterLayer->getDrawingStyle() == QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR ||
          rasterLayer->getDrawingStyle() == QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR)
  {
    if(rasterLayer->getColorRampingType()==QgsRasterLayer::BLUE_GREEN_RED)
    {
      cboColorMap->setCurrentText(tr("Pseudocolor"));
    }
    else
    {
      cboColorMap->setCurrentText(tr("Freak Out"));
    }

  }
  else
  {
    cboColorMap->setCurrentText(tr("Grayscale"));
  }
  //set whether the layer histogram should be inverted
  if (rasterLayer->getInvertHistogramFlag())
  {
    cboxInvertColorMap->setChecked(true);
  }
  else
  {
    cboxInvertColorMap->setChecked(false);
  }

  double myStdDevsDouble = rasterLayer->getStdDevsToPlot();
  cboStdDev->setCurrentText(QString::number(myStdDevsDouble));

  //
  // Set up the sliders on the advanced symbology tab
  //
  sliderMinRed->setValue(static_cast < int >(rasterLayer->getMinRedDouble()));
  sliderMaxRed->setValue(static_cast < int >(255 - rasterLayer->getMaxRedDouble()));
  sliderMinGreen->setValue(static_cast < int >(rasterLayer->getMinGreenDouble()));
  sliderMaxGreen->setValue(static_cast < int >(255 - rasterLayer->getMaxGreenDouble()));
  sliderMinBlue->setValue(static_cast < int >(rasterLayer->getMinBlueDouble()));
  sliderMaxBlue->setValue(static_cast < int >(255 - rasterLayer->getMaxBlueDouble()));
  sliderMinGray->setValue(static_cast < int >(rasterLayer->getMinGrayDouble()));
  sliderMaxGray->setValue(static_cast < int >(255 - rasterLayer->getMaxGrayDouble()));

  //now set the combos to the correct values
  cboRed->setCurrentText(rasterLayer->getRedBandName());
  cboGreen->setCurrentText(rasterLayer->getGreenBandName());
  cboBlue->setCurrentText(rasterLayer->getBlueBandName());
  cboGray->setCurrentText(rasterLayer->getGrayBandName());

  //
  // Set up the colour scaling previews
  //
  makeScalePreview("red");
  makeScalePreview("green");
  makeScalePreview("blue");
  makeScalePreview("gray");


} // QgsRasterLayerProperties::sync()

void QgsRasterLayerProperties::pbnHistRefresh_clicked()
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayerProperties::pbnHistRefresh_clicked" << std::endl;
#endif
  int myBandCountInt = rasterLayer->getBandCount();
  //
  // Find out how many bands are selected and short circuit out clearing the image
  // if needed
  int mySelectionCount=0;
  for (int myIteratorInt = 1;
          myIteratorInt <= myBandCountInt;
          ++myIteratorInt)
  {
    RasterBandStats myRasterBandStats = rasterLayer->getRasterBandStats(myIteratorInt);
    QListBoxItem *myItem = lstHistogramLabels->item( myIteratorInt-1 );
    if ( myItem->isSelected() )
    {
      mySelectionCount++;
    }
  }
  if (mySelectionCount==0)
  {
    int myImageWidth = pixHistogram->width();
    int myImageHeight =  pixHistogram->height();
    QPixmap myPixmap(myImageWidth,myImageHeight);
    myPixmap.fill(Qt::white);
    pixHistogram->setPixmap(myPixmap);
  }

  // Explanation:
  // We use the gdal histogram creation routine is called for each selected  
  // layer. Currently the hist is hardcoded
  // to create 256 bins. Each bin stores the total number of cells that 
  // fit into the range defined by that bin.
  //
  // The graph routine below determines the greatest number of pixesl in any given 
  // bin in all selected layers, and the min. It then draws a scaled line between min 
  // and max - scaled to image height. 1 line drawn per selected band
  //
  const int BINCOUNT = spinHistBinCount->value();
  enum GRAPH_TYPE { BAR_CHART, LINE_CHART } myGraphType;
  if (radHistTypeBar->isOn()) myGraphType=BAR_CHART; else myGraphType=LINE_CHART;
  bool myIgnoreOutOfRangeFlag = chkHistIgnoreOutOfRange->isChecked();
  bool myThoroughBandScanFlag = chkHistAllowApproximation->isChecked();

  long myCellCount = rasterLayer->getRasterXDim() * rasterLayer->getRasterYDim();


#ifdef QGISDEBUG
  std::cout << "Computing histogram minima and maxima" << std::endl;
#endif
  //somtimes there are more bins than needed
  //we find out the last on that actully has data in it
  //so we can discard the rest adn the x-axis scales correctly
  int myLastBinWithData=0;
  //
  // First scan through to get max and min cell counts from among selected layers' histograms
  //
  double myYAxisMax=0;
  double myYAxisMin=0;
  int myXAxisMin=0;
  int myXAxisMax=0;
  for (int myIteratorInt = 1;
          myIteratorInt <= myBandCountInt;
          ++myIteratorInt)
  {
    RasterBandStats myRasterBandStats = rasterLayer->getRasterBandStats(myIteratorInt);
    //calculate the x axis min max
    if (myRasterBandStats.minValDouble < myXAxisMin || myIteratorInt==1)
    {
      myXAxisMin=static_cast < unsigned int >(myRasterBandStats.minValDouble);
    }
    if (myRasterBandStats.maxValDouble < myXAxisMax || myIteratorInt==1)
    {
      myXAxisMax=static_cast < unsigned int >(myRasterBandStats.maxValDouble);
    }
    QListBoxItem *myItem = lstHistogramLabels->item( myIteratorInt-1 );
    if ( myItem->isSelected() )
    {
#ifdef QGISDEBUG
      std::cout << "Ensuring hist is populated for this layer" << std::endl;
#endif
      rasterLayer->populateHistogram(myIteratorInt,BINCOUNT,myIgnoreOutOfRangeFlag,myThoroughBandScanFlag); 

#ifdef QGISDEBUG
      std::cout << "...done..." << myRasterBandStats.histogramVector->size() << " bins filled" << std::endl;
#endif
      for (int myBin = 0; myBin <BINCOUNT; myBin++)
      {
        int myBinValue = myRasterBandStats.histogramVector->at(myBin);
        if (myBinValue > 0 && myBin > myLastBinWithData)
        {
          myLastBinWithData = myBin;
        }
#ifdef QGISDEBUG
        std::cout << "Testing if " << myBinValue << " is less than " << myYAxisMin  << "or greater then " <<myYAxisMax  <<  std::endl;
#endif
        if ( myBin==0)
        {
          myYAxisMin = myBinValue;
          myYAxisMax = myBinValue;
        }

        if (myBinValue  > myYAxisMax)
        {
          myYAxisMax = myBinValue;
        }
        if ( myBinValue < myYAxisMin)
        {
          myYAxisMin = myBinValue;
        }
      }
    }
  }
#ifdef QGISDEBUG
  std::cout << "max " << myYAxisMax << std::endl;
  std::cout << "min " << myYAxisMin << std::endl;
#endif


  //create the image onto which graph and axes will be drawn
  int myImageWidth = pixHistogram->width();
  int myImageHeight =  pixHistogram->height();
  QPixmap myPixmap(myImageWidth,myImageHeight);
  myPixmap.fill(Qt::white);

  // TODO: Confirm that removing the "const QWidget * copyAttributes" 2nd parameter,
  // in order to make things work in Qt4, doesn't break things in Qt3.
  //QPainter myPainter(&myPixmap, this);
  QPainter myPainter(&myPixmap);

  //determine labels sizes and draw them
  QFont myQFont("arial", 8, QFont::Normal);
  QFontMetrics myFontMetrics( myQFont );
  myPainter.setFont(myQFont);
  myPainter.setPen(Qt::black);
  QString myYMaxLabel= QString::number(static_cast < unsigned int >(myYAxisMax));
  QString myXMinLabel= QString::number(myXAxisMin);
  QString myXMaxLabel= QString::number(myXAxisMax);
  //calculate the gutters
  int myYGutterWidth=0;
  if (myFontMetrics.width(myXMinLabel) < myFontMetrics.width(myYMaxLabel))
  {
    myYGutterWidth = myFontMetrics.width(myYMaxLabel )+2; //add 2 so we can have 1 pix whitespace either side of label
  }
  else
  {
    myYGutterWidth = myFontMetrics.width(myXMinLabel )+2; //add 2 so we can have 1 pix whitespace either side of label
  }
  int myXGutterHeight = myFontMetrics.height()+2;
  int myXGutterWidth = myFontMetrics.width(myXMaxLabel)+1;//1 pix whtispace from right edge of image

  //
  // Now calculate the graph drawable area after the axis labels have been taken
  // into account
  //
  int myGraphImageWidth =myImageWidth-myYGutterWidth; 
  int myGraphImageHeight = myImageHeight-myXGutterHeight; 

  //find out how wide to draw bars when in bar chart mode
  int myBarWidth = static_cast<int>((((double)myGraphImageWidth)/((double)BINCOUNT)));


  //
  //now draw actual graphs
  //
  if (rasterLayer->getRasterLayerType()
          == QgsRasterLayer::PALETTE) //paletted layers have hard coded color entries
  {
    QPointArray myPointArray(myLastBinWithData);
    QgsColorTable *myColorTable=rasterLayer->colorTable(1);
#ifdef QGISDEBUG
    std::cout << "Making paletted image histogram....computing band stats" << std::endl;
    std::cout << "myLastBinWithData = " << myLastBinWithData << std::endl;
#endif

    RasterBandStats myRasterBandStats = rasterLayer->getRasterBandStats(1);
    for (int myBin = 0; myBin < myLastBinWithData; myBin++)
    {
      double myBinValue = myRasterBandStats.histogramVector->at(myBin);
#ifdef QGISDEBUG
      std::cout << "myBin = " << myBin << " myBinValue = " << myBinValue << std::endl;
#endif
      //NOTE: Int division is 0 if the numerator is smaller than the denominator.
      //hence the casts
      int myX = static_cast<int>((((double)myGraphImageWidth)/((double)myLastBinWithData))*myBin);
      //height varies according to freq. and scaled to greatet value in all layers
      int myY=0;
      if (myYAxisMax!=0)
      {  
        myY=static_cast<int>(((double)myBinValue/(double)myYAxisMax)*myGraphImageHeight);
      }
      
      //see wehter to draw something each loop or to save up drawing for after iteration
      if (myGraphType==BAR_CHART)
      {
	//determin which color to draw the bar
	int c1, c2, c3;
	// Take middle of the interval for color
	// TODO: this is not precise
	double myInterval = (myXAxisMax - myXAxisMin) / myLastBinWithData;
	double myMiddle = myXAxisMin + myBin * myInterval + myInterval/2;

#ifdef QGISDEBUG
	std::cout << "myMiddle = " << myMiddle << std::endl;
#endif
	
	bool found = myColorTable->color ( myMiddle, &c1, &c2, &c3 );
	if ( !found ) {
	    std::cout << "Color not found" << std::endl;
	    c1 = c2 = c3 = 180; // grey
	}
	  
#ifdef QGISDEBUG
	std::cout << "c1 = " << c1 << " c2 = " << c2 << " c3 = " << c3 << std::endl;
#endif
	  
        //draw the bar
        //QBrush myBrush(QColor(c1,c2,c3));
	myPainter.setBrush(QColor(c1,c2,c3));
        myPainter.setPen(QColor(c1,c2,c3));
#ifdef QGISDEBUG
        std::cout << "myX = " << myX << " myY = " << myY << std::endl;
        std::cout << "rect: " << myX+myYGutterWidth << ", " << myImageHeight-(myY+myXGutterHeight)
	          << ", " << myBarWidth << ", " << myY << std::endl;
#endif
        myPainter.drawRect(myX+myYGutterWidth,myImageHeight-(myY+myXGutterHeight),myBarWidth,myY);
      }
      //store this point in our line too
      myY = myGraphImageHeight - myY;
      myPointArray.setPoint(myBin, myX+myYGutterWidth, myY-myXGutterHeight);
    }
    //draw a line on the graph along the bar peaks; 
    if (myGraphType==LINE_CHART)
    {
      myPainter.setPen( Qt::black );
      myPainter.drawPolyline(myPointArray);
    }
  }
  else
  {

    for (int myIteratorInt = 1;
            myIteratorInt <= myBandCountInt;
            ++myIteratorInt)
    {
      RasterBandStats myRasterBandStats = rasterLayer->getRasterBandStats(myIteratorInt);
      QListBoxItem *myItem = lstHistogramLabels->item( myIteratorInt-1 );
      if ( myItem->isSelected() )
      {

        QPointArray myPointArray(myLastBinWithData);
        for (int myBin = 0; myBin <myLastBinWithData; myBin++)
        {
          double myBinValue = myRasterBandStats.histogramVector->at(myBin);
          //NOTE: Int division is 0 if the numerator is smaller than the denominator.
          //hence the casts
          int myX = static_cast<int>((((double)myGraphImageWidth)/((double)myLastBinWithData))*myBin);
          //height varies according to freq. and scaled to greatet value in all layers
          int myY = static_cast<int>(((double)myBinValue/(double)myYAxisMax)*myGraphImageHeight);
          //adjust for image origin being top left
#ifdef QGISDEBUG
          std::cout << "-------------" << std::endl;
          std::cout << "int myY = (myBinValue/myCellCount)*myGraphImageHeight" << std::endl;
          std::cout << "int myY = (" << myBinValue << "/" << myCellCount << ")*" << myGraphImageHeight << std::endl;
          std::cout << "Band " << myIteratorInt << ", bin " << myBin << ", Hist Value : " << myBinValue << ", Scaled Value : " << myY << std::endl;
          std::cout << "myY = myGraphImageHeight - myY" << std::endl;
          std::cout << "myY = " << myGraphImageHeight << "-" << myY << std::endl;
#endif
          if (myGraphType==BAR_CHART)
          {
            //draw the bar
            if (myBandCountInt==1) //draw single band images with black
            {
              myPainter.setPen( Qt::black );
            }
            else if (myIteratorInt==1)
            {
              myPainter.setPen( Qt::red );
            }
            else if (myIteratorInt==2)
            {
              myPainter.setPen( Qt::green );
            }
            else if (myIteratorInt==3)
            {
              myPainter.setPen( Qt::blue );
            }
            else if (myIteratorInt==4)
            {
              myPainter.setPen( Qt::magenta );
            }
            else if (myIteratorInt==5)
            {
              myPainter.setPen( Qt::darkRed );
            }
            else if (myIteratorInt==6)
            {
              myPainter.setPen( Qt::darkGreen );
            }
            else if (myIteratorInt==7)
            {
              myPainter.setPen( Qt::darkBlue );
            }
            else
            {
              myPainter.setPen( Qt::gray );
            }
#ifdef QGISDEBUG
            //  std::cout << "myPainter.fillRect(QRect(" << myX << "," << myY << "," << myBarWidth << "," <<myY << ") , myBrush );" << std::endl;
#endif
            myPainter.drawRect(myX+myYGutterWidth,myImageHeight-(myY+myXGutterHeight),myBarWidth,myY);
          }
          else //line graph
          {
            myY = myGraphImageHeight - myY;
            myPointArray.setPoint(myBin, myX+myYGutterWidth, myY);
          }
        }
        if (myGraphType==LINE_CHART)
        {
          if (myBandCountInt==1) //draw single band images with black
          {
            myPainter.setPen( Qt::black );
          }
          else if (myIteratorInt==1)
          {
            myPainter.setPen( Qt::red );
          }
          else if (myIteratorInt==2)
          {
            myPainter.setPen( Qt::green );
          }
          else if (myIteratorInt==3)
          {
            myPainter.setPen( Qt::blue );
          }
          else if (myIteratorInt==4)
          {
            myPainter.setPen( Qt::magenta );
          }
          else if (myIteratorInt==5)
          {
            myPainter.setPen( Qt::darkRed );
          }
          else if (myIteratorInt==6)
          {
            myPainter.setPen( Qt::darkGreen );
          }
          else if (myIteratorInt==7)
          {
            myPainter.setPen( Qt::darkBlue );
          }
          else
          {
            myPainter.setPen( Qt::gray );
          }
          myPainter.drawPolyline(myPointArray);
        }
      }
    }
  }

  //
  // Now draw interval markers on the x axis
  //
  int myXDivisions = myGraphImageWidth/10;
  myPainter.setPen( Qt::gray );
  for (int i=0;i<myXDivisions;++i)
  {
    QPointArray myPointArray(4);
    myPointArray.setPoint(0,(i*myXDivisions)+myYGutterWidth , myImageHeight-myXGutterHeight);
    myPointArray.setPoint(1,(i*myXDivisions)+myYGutterWidth , myImageHeight-(myXGutterHeight-5));
    myPointArray.setPoint(2,(i*myXDivisions)+myYGutterWidth , myImageHeight-myXGutterHeight);
    myPointArray.setPoint(3,((i+1)*myXDivisions)+myYGutterWidth , myImageHeight-myXGutterHeight);
    myPainter.drawPolyline(myPointArray);
  }
  //
  // Now draw interval markers on the y axis
  //
  int myYDivisions = myGraphImageHeight/10;
  myPainter.setPen( Qt::gray );
  for (int i=myYDivisions;i>0;--i)
  {

    QPointArray myPointArray(4);
    int myYOrigin = myImageHeight-myXGutterHeight;
    myPointArray.setPoint(0,myYGutterWidth,myYOrigin-(i*myYDivisions ));
    myPointArray.setPoint(1,myYGutterWidth-5,myYOrigin-(i*myYDivisions ));
    myPointArray.setPoint(2,myYGutterWidth,myYOrigin-(i*myYDivisions ));
    myPointArray.setPoint(3,myYGutterWidth,myYOrigin-((i-1)*myYDivisions ));
    myPainter.drawPolyline(myPointArray);
  }

  //now draw the axis labels onto the graph
  myPainter.drawText(1, 12, myYMaxLabel);
  myPainter.drawText(1, myImageHeight-myXGutterHeight, QString::number(static_cast < unsigned int >(myYAxisMin)));
  myPainter.drawText(myYGutterWidth,myImageHeight-1 , myXMinLabel);
  myPainter.drawText( myImageWidth-myXGutterWidth,myImageHeight-1, myXMaxLabel );

  //
  // Finish up
  //
  myPainter.end();
  pixHistogram->setPixmap(myPixmap);
}

