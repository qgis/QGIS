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
/* $Id$ */

#include "qgsrasterlayerproperties.h"
#include <qlabel.h>
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
#include <qtextbrowser.h>
QgsRasterLayerProperties::QgsRasterLayerProperties(QgsMapLayer * lyr):QgsRasterLayerPropertiesBase()
{
  //
  //
  //
  // Remove the advanced symbology widget and debug overlay for 0.1 release
  //
  //
  //
  tabSymbology->removePage(TabPage);
  cboxShowDebugInfo->hide();
  //downcast the maplayer to rasterlayer
  rasterLayer = (QgsRasterLayer *) lyr;

#ifdef DEBUG
  //populate the metadata tab's text browser widget with gdal metadata info
  txtbMetadata->setText(rasterLayer->getMetadata());
#else
  tabSymbology->removePage(tabMetadata);
#endif
  //display the raster dimensions and no data value
  lblColumns->setText(tr("<p align=\"center\">Columns:") + QString::number(rasterLayer->getRasterXDim()) + "</p>");
  lblRows->setText(tr("<p align=\"right\">Rows:") + QString::number(rasterLayer->getRasterYDim()) + "</p>");
  lblNoData->setText(tr("No Data:") + QString::number(rasterLayer->getNoDataValue()));
  //these properties (layername and label) are provided by the qgsmaplayer superclass
  leLayerSource->setText(rasterLayer->source());
  leDisplayName->setText(lyr->name());

  //update the debug checkbox
  cboxShowDebugInfo->setChecked(rasterLayer->getShowDebugOverlayFlag());

  //update the legend pixmap on this dialog
  pixmapLegend->setPixmap(rasterLayer->getLegendQPixmap());
  pixmapLegend->setScaledContents(true);
  pixmapLegend->repaint(false);

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
  } else if (rasterLayer->getRasterLayerType() == QgsRasterLayer::PALETTE)
    {
      //paletted images (e.g. tif) can only be rendered as three band rgb images
      txtSymologyNotes->
        setText(tr
                ("<h3>Paletted Image Notes</h3> <p>This image uses a fixed color palette. You can remap these colors in different combinations e.g.</p><ul><li>Red - blue in image</li><li>Green - blue in image</li><li>Blue - green in image</li></ul>"));
  } else                        //only grayscale settings allowed
    {
      //grayscale images can only be rendered as singleband
      txtSymologyNotes->
        setText(tr
                ("<h3>Grayscale Image Notes</h3> <p>You can remap these grayscale colors to a pseudocolor image using an automatically generated color ramp.</p>"));
    }
  //
  // Populate the various controls on the form
  //
  cboColorMap->insertItem(tr("Grayscale"));
  cboColorMap->insertItem(tr("Pseudocolor"));
  if (rasterLayer->getDrawingStyle() == QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR ||
      rasterLayer->getDrawingStyle() == QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR ||
      rasterLayer->getDrawingStyle() == QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR)
    {
      cboColorMap->setCurrentText(tr("Pseudocolor"));
  } else
    {
      cboColorMap->setCurrentText(tr("Grayscale"));
    }
  //set whether the layer histogram should be inverted
  if (rasterLayer->getInvertHistogramFlag())
    {
      cboxInvertColorMap->setChecked(true);
  } else
    {
      cboxInvertColorMap->setChecked(false);
    }
  //set the std deviations to be plotted combo
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
  //
  // Populate the statistics table
  //
  fillStatsTable();
  //
  // Get a list of band names
  //
  QStringList myBandNameQStringList;
  int myBandCountInt = rasterLayer->getBandCount();
  for (int myIteratorInt = 1; myIteratorInt <= myBandCountInt; ++myIteratorInt)
    {
      //find out the name of this band
      QString myRasterBandNameQString = rasterLayer->getRasterBandName(myIteratorInt);
      //keep a list of band names for later use
      myBandNameQStringList.append(myRasterBandNameQString);
    }
  //
  // Set up the combo boxes that contain band lists using the qstring list generated above
  //
  if (rasterLayer->getRasterLayerType() == QgsRasterLayer::PALETTE) //paletted layers have hard coded color entries
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
    }

  else                          //all other layer types use band name entries only
    {
#ifdef DEBUG
      std::cout << "Populating combos for non paletted layer" << std::endl;
#endif
      int myBandCountInt = 1;
      for (QStringList::Iterator myIterator = myBandNameQStringList.begin(); myIterator != myBandNameQStringList.end(); ++myIterator)
        {
          QString myQString = *myIterator;
#ifdef DEBUG
          std::cout << "Inserting : " << myQString << std::endl;
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

}

QgsRasterLayerProperties::~QgsRasterLayerProperties()
{
  delete rasterLayer;
}


void QgsRasterLayerProperties::apply()
{

  rasterLayer->setTransparency(static_cast < unsigned int >(255 - sliderTransparency->value()));
  //set the std deviations to be plotted
  rasterLayer->setStdDevsToPlot(cboStdDev->currentText().toDouble());
  //set whether the layer histogram should be inverted
  if (cboxInvertColorMap->isChecked())
    {
      rasterLayer->setInvertHistogramFlag(true);
  } else
    {
      rasterLayer->setInvertHistogramFlag(false);
    }
  //now set the color -> band mapping combos to the correct values
  rasterLayer->setRedBandName(cboRed->currentText());
  rasterLayer->setGreenBandName(cboGreen->currentText());
  rasterLayer->setBlueBandName(cboBlue->currentText());
  rasterLayer->setGrayBandName(cboGray->currentText());
  //set the appropriate render style
  if ((grpBoxGrayscale->isEnabled()) && (rbtnSingleBand->isChecked()))
    {
      //
      // Grayscale
      //
      if (rasterLayer->rasterLayerType == QgsRasterLayer::GRAY_OR_UNDEFINED)
        {

          if (cboColorMap->currentText() == tr("Pseudocolor"))
            {
#ifdef DEBUG
              std::cout << "Setting Raster Drawing Style to :: SINGLE_BAND_PSEUDO_COLOR" << std::endl;
#endif
              rasterLayer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
          } else
            {
#ifdef DEBUG
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
          if (cboColorMap->currentText() == tr("Pseudocolor"))
            {
#ifdef DEBUG
              std::cout << "Setting Raster Drawing Style to :: PALETTED_SINGLE_BAND_PSEUDO_COLOR" << std::endl;
#endif
              rasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR);
          } else
            {
#ifdef DEBUG
              std::cout << "Setting Raster Drawing Style to :: PALETTED_SINGLE_BAND_GRAY" << std::endl;
#endif
#ifdef DEBUG
              std::cout << "Combo value : " << cboGray->currentText() << " GrayBand Mapping : " << rasterLayer->
                getGrayBandName() << std::endl;
#endif
              rasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY);
            }


        }
      //
      // Mutltiband
      //
      else if (rasterLayer->rasterLayerType == QgsRasterLayer::MULTIBAND)
        {
          if (cboColorMap->currentText() == tr("Pseudocolor"))
            {
#ifdef DEBUG
              std::cout << "Setting Raster Drawing Style to ::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR " << std::endl;
#endif
              rasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR);
          } else
            {
#ifdef DEBUG
              std::cout << "Setting Raster Drawing Style to :: MULTI_BAND_SINGLE_BAND_GRAY" << std::endl;
              std::cout << "Combo value : " << cboGray->currentText() << " GrayBand Mapping : " << rasterLayer->
                getGrayBandName() << std::endl;
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
#ifdef DEBUG
          std::cout << "Setting Raster Drawing Style to :: PALETTED_MULTI_BAND_COLOR" << std::endl;
#endif
          rasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_MULTI_BAND_COLOR);
      } else if (rasterLayer->rasterLayerType == QgsRasterLayer::MULTIBAND)
        {

#ifdef DEBUG
          std::cout << "Setting Raster Drawing Style to :: MULTI_BAND_COLOR" << std::endl;
#endif
          rasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_COLOR);
        }

    }


  //update the legend pixmap
  pixmapLegend->setPixmap(rasterLayer->getLegendQPixmap());
  pixmapLegend->setScaledContents(true);
  pixmapLegend->repaint(false);
  QgsLegendItem *myLegendItem = rasterLayer->legendItem();
  ((QCheckListItem *) myLegendItem)->setPixmap(0, *rasterLayer->legendPixmap());

  rasterLayer->setLayerName(leDisplayName->text());
  //see if the user would like debug overlays
  if (cboxShowDebugInfo->isChecked() == true)
    {
      rasterLayer->setShowDebugOverlayFlag(true);
  } else
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
  //update the stats table
  fillStatsTable();
  //make sure the layer is redrawn
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
  } else if (theColor == "green")
    {
      myMinDouble = sliderMinGreen->value();
      myMaxDouble = 255 - sliderMaxGreen->value();
  } else if (theColor == "blue")
    {
      myMinDouble = sliderMinBlue->value();
      myMaxDouble = 255 - sliderMaxBlue->value();
  } else if (theColor == "gray")
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
      } else if (theColor == "green")
        {
          myGreenDouble = myMaxDouble;
      } else if (theColor == "blue")
        {
          myBlueDouble = myMaxDouble;
      } else if (theColor == "gray")
        {
          myRedDouble = myMaxDouble;
          myGreenDouble = myMaxDouble;
          myBlueDouble = myMaxDouble;
        }
      for (double myColInt = 99; myColInt >= 0; myColInt = myColInt - 1)
        {
          if (theColor == "red")
            {
              myRedDouble -= myDecrementDouble;
          } else if (theColor == "green")
            {
              myGreenDouble -= myDecrementDouble;
          } else if (theColor == "blue")
            {
              myBlueDouble -= myDecrementDouble;
          } else if (theColor == "gray")
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
  myQPainter.drawImage(-70, 0, myQImage.scale(140, 140));
  myQPainter.rotate(45);
  QFont myQFont("time", 18, QFont::Bold);
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
  } else if (theColor == "green")
    {
      pixmapScaleGreen->setScaledContents(true);
      pixmapScaleGreen->setPixmap(*myQPixmap);
      pixmapScaleGreen->repaint(false);
  } else if (theColor == "blue")
    {
      pixmapScaleBlue->setScaledContents(true);
      pixmapScaleBlue->setPixmap(*myQPixmap);
      pixmapScaleBlue->repaint(false);
  } else if (theColor == "gray")
    {
      pixmapScaleGray->setScaledContents(true);
      pixmapScaleGray->setPixmap(*myQPixmap);
      pixmapScaleGray->repaint(false);
    }
}


void QgsRasterLayerProperties::rbtnSingleBand_toggled(bool)
{
}


void QgsRasterLayerProperties::rbtnThreeBand_toggled(bool)
{
}


void QgsRasterLayerProperties::fillStatsTable()
{
  //
  // Populate the statistics table
  //
  int myRowInt = 0;
  int myBandCountInt = rasterLayer->getBandCount();
  //allocate 1 row per struct element (11)
  tblStats->setNumRows(11 * myBandCountInt);
  tblStats->setNumCols(2);
  QHeader *myQHeader = tblStats->horizontalHeader();
  myQHeader->setLabel(0, "Property");
  myQHeader->setLabel(1, "Value");
  for (int myIteratorInt = 1; myIteratorInt <= myBandCountInt; ++myIteratorInt)
    {
#ifdef DEBUG
      std::cout << "Raster properties : checking if band " << myIteratorInt << " has stats? ";
#endif
      //check if full stats for this layer have already been collected
      if (!rasterLayer->hasStats(myIteratorInt))  //not collected
        {
#ifdef DEBUG
          std::cout << ".....no" << std::endl;
#endif
          tblStats->setText(myRowInt, 0, "Band");
          tblStats->setText(myRowInt, 1, rasterLayer->getRasterBandName(myIteratorInt));
          ++myRowInt;

          tblStats->setText(myRowInt, 0, "Band No");
          tblStats->setText(myRowInt, 1, QString::number(myIteratorInt));
          ++myRowInt;
          tblStats->setText(myRowInt, 0, "Stats");
          tblStats->setText(myRowInt, 1, "Not collected yet");
          ++myRowInt;
      } else                    // collected - show full detail
        {
#ifdef DEBUG
          std::cout << ".....yes" << std::endl;
#endif
          RasterBandStats myRasterBandStats = rasterLayer->getRasterBandStats(myIteratorInt);

          tblStats->setText(myRowInt, 0, "Band");
          tblStats->setText(myRowInt, 1, myRasterBandStats.bandName);
          ++myRowInt;

          tblStats->setText(myRowInt, 0, "Band No");
          tblStats->setText(myRowInt, 1, QString::number(myRasterBandStats.bandNoInt));
          ++myRowInt;
          tblStats->setText(myRowInt, 0, "minValDouble");
          tblStats->setText(myRowInt, 1, QString::number(myRasterBandStats.minValDouble));
          ++myRowInt;
          tblStats->setText(myRowInt, 0, "maxValDouble");
          tblStats->setText(myRowInt, 1, QString::number(myRasterBandStats.maxValDouble));
          ++myRowInt;
          tblStats->setText(myRowInt, 0, "rangeDouble");
          tblStats->setText(myRowInt, 1, QString::number(myRasterBandStats.rangeDouble));
          ++myRowInt;
          tblStats->setText(myRowInt, 0, "meanDouble");
          tblStats->setText(myRowInt, 1, QString::number(myRasterBandStats.meanDouble));
          ++myRowInt;
          tblStats->setText(myRowInt, 0, "sumSqrDevDouble");  //used to calculate stddev
          tblStats->setText(myRowInt, 1, QString::number(myRasterBandStats.sumSqrDevDouble)); //used to calculate stddev
          ++myRowInt;
          tblStats->setText(myRowInt, 0, "stdDevDouble");
          tblStats->setText(myRowInt, 1, QString::number(myRasterBandStats.stdDevDouble));
          ++myRowInt;
          tblStats->setText(myRowInt, 0, "sumDouble");
          tblStats->setText(myRowInt, 1, QString::number(myRasterBandStats.sumDouble));
          ++myRowInt;
          tblStats->setText(myRowInt, 0, "elementCountInt");
          tblStats->setText(myRowInt, 1, QString::number(myRasterBandStats.elementCountInt));
          ++myRowInt;
        }
      ++myRowInt;
    }
}
