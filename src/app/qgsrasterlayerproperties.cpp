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

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsrasterlayerproperties.h"
#include "qgslayerprojectionselector.h"
#include "qgsproject.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpyramid.h"
#include "qgscontexthelp.h"
#include "qgsmaplayerregistry.h"
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include "qgscolorrampshader.h"

#include <QTableWidgetItem>
#include <QHeaderView>

#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QLinearGradient>
#include <QPainterPath>
#include <QPolygonF>
#include <QColorDialog>
#include <QList>
#include <QSettings>

#include <iostream>

const char * const ident = 
"$Id$";

// Constant that signals property not used.
const QString QgsRasterLayerProperties::QSTRING_NOT_SET = "Not Set";

QgsRasterLayerProperties::QgsRasterLayerProperties(QgsMapLayer *lyr, QWidget *parent, Qt::WFlags fl)
  : QDialog(parent, fl), 
mRasterLayer( dynamic_cast<QgsRasterLayer*>(lyr) )
{
  ignoreSpinBoxEvent = false; //Short circuit signal loop between min max field and stdDev spin box

  if (mRasterLayer->getDataProvider() == 0)
  {
    // This is a GDAL-based layer
    mRasterLayerIsGdal = TRUE;
    mRasterLayerIsWms = FALSE;
  }
  else if (mRasterLayer->getDataProvider()->name() == "wms")
  {
    // This is a WMS-based layer
    mRasterLayerIsWms = TRUE;
    mRasterLayerIsGdal = FALSE;
  }

  setupUi(this);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(this, SIGNAL(accepted()), this, SLOT(apply()));
  connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

  connect(sliderTransparency, SIGNAL(valueChanged(int)), this, SLOT(sliderTransparency_valueChanged(int)));

  //clear either stdDev or min max entries depending which is changed
  connect(sboxSingleBandStdDev, SIGNAL(valueChanged(double)), this, SLOT(sboxSingleBandStdDev_valueChanged(double)));
  connect(sboxThreeBandStdDev, SIGNAL(valueChanged(double)), this, SLOT(sboxThreeBandStdDev_valueChanged(double)));
  connect(leGrayMin, SIGNAL(textEdited(QString)), this, SLOT(userDefinedMinMax_textEdited(QString)));
  connect(leGrayMax, SIGNAL(textEdited(QString)), this, SLOT(userDefinedMinMax_textEdited(QString)));
  connect(leRedMin, SIGNAL(textEdited(QString)), this, SLOT(userDefinedMinMax_textEdited(QString)));
  connect(leRedMax, SIGNAL(textEdited(QString)), this, SLOT(userDefinedMinMax_textEdited(QString)));
  connect(leGreenMin, SIGNAL(textEdited(QString)), this, SLOT(userDefinedMinMax_textEdited(QString)));
  connect(leGreenMax, SIGNAL(textEdited(QString)), this, SLOT(userDefinedMinMax_textEdited(QString)));
  connect(leBlueMin, SIGNAL(textEdited(QString)), this, SLOT(userDefinedMinMax_textEdited(QString)));
  connect(leBlueMax, SIGNAL(textEdited(QString)), this, SLOT(userDefinedMinMax_textEdited(QString)));
  connect(mColormapTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(handleColormapTreeWidgetDoubleClick(QTreeWidgetItem*,int)));

  // set up the scale based layer visibility stuff....
  chkUseScaleDependentRendering->setChecked(lyr->scaleBasedVisibility());
  spinMinimumScale->setValue((int)lyr->minScale());
  spinMaximumScale->setValue((int)lyr->maxScale());

  // build GUI components
  cboxColorMap->addItem(tr("Grayscale"));
  cboxColorMap->addItem(tr("Pseudocolor"));
  cboxColorMap->addItem(tr("Freak Out"));
  cboxColorMap->addItem(tr("Custom Colormap"));

  //add items to the color stretch combo box
  cboxContrastEnhancementAlgorithm->addItem(tr("No Stretch"));
  cboxContrastEnhancementAlgorithm->addItem(tr("Stretch To MinMax"));
  cboxContrastEnhancementAlgorithm->addItem(tr("Stretch And Clip To MinMax"));
  cboxContrastEnhancementAlgorithm->addItem(tr("Clip To MinMax"));

  //set initial states of all Min Max and StdDev fields and labels to disabled
  sboxThreeBandStdDev->setEnabled(false);
  sboxSingleBandStdDev->setEnabled(false);
  lblGrayMin->setEnabled(false);
  leGrayMin->setEnabled(false);
  lblGrayMax->setEnabled(false);
  leGrayMax->setEnabled(false);
  lblRedMin->setEnabled(false);
  leRedMin->setEnabled(false);
  lblRedMax->setEnabled(false);
  leRedMax->setEnabled(false);
  lblGreenMin->setEnabled(false);
  leGreenMin->setEnabled(false);
  lblGreenMax->setEnabled(false);
  leGreenMax->setEnabled(false);
  lblBlueMin->setEnabled(false);
  leBlueMin->setEnabled(false);
  lblBlueMax->setEnabled(false);
  leBlueMax->setEnabled(false);

  pbtnLoadMinMax->setEnabled(false);
  
  //setup custom colormap tab
  cboxColorInterpolation->addItem(tr("Discrete"));
  cboxColorInterpolation->addItem(tr("Linearly"));
  cboxClassificationMode->addItem(tr("Equal interval"));
  cboxClassificationMode->addItem(tr("Quantiles"));

  QStringList headerLabels;
  headerLabels << "Value";
  headerLabels << "Color";
  headerLabels << "Label";
  mColormapTreeWidget->setHeaderLabels(headerLabels);

  //disable Custom colormap tab completely until 'Custom Colormap' is selected (and only for type GRAY_OR_UNDEFINED)
  tabBar->setTabEnabled(tabBar->indexOf(tabPageColormap), FALSE);

  //
  // Set up the combo boxes that contain band lists using the qstring list generated above
  //
  
  if (mRasterLayer->getRasterLayerType()
      == QgsRasterLayer::PALETTE) //paletted layers have hard coded color entries
  {
    cboRed->addItem(tr("Red"));
    cboGreen->addItem(tr("Red"));
    cboBlue->addItem(tr("Red"));

    cboRed->addItem(tr("Green"));
    cboGreen->addItem(tr("Green"));
    cboBlue->addItem(tr("Green"));

    cboRed->addItem(tr("Blue"));
    cboGreen->addItem(tr("Blue"));
    cboBlue->addItem(tr("Blue"));

    cboRed->addItem(tr(QSTRING_NOT_SET));
    cboGreen->addItem(tr(QSTRING_NOT_SET));
    cboBlue->addItem(tr(QSTRING_NOT_SET));

    cboGray->addItem(tr("Red"));
    cboGray->addItem(tr("Green"));
    cboGray->addItem(tr("Blue"));
    cboGray->addItem(tr(QSTRING_NOT_SET));

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
    QStringList myBandNameList;

    int myBandCountInt = mRasterLayer->getBandCount();
#ifdef QGISDEBIG
    std::cout << "Looping though " << myBandCountInt << " image layers to get their names " << std::endl;
#endif
    for (int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt)
    {
      //find out the name of this band
      QString myRasterBandNameQString = mRasterLayer->getRasterBandName(myIteratorInt) ;

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
      mGradientHeight = pixHistogram->height() / 2;
      mGradientWidth = pixHistogram->width() /2;
      //keep a list of band names for later use
      //! @note band names should not be translated!
      myBandNameList.append(myRasterBandNameQString);
    }

    //select all histogram layers list items by default
    for (int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt)
    {
      Q3ListBoxItem *myItem = lstHistogramLabels->item( myIteratorInt-1 );
      lstHistogramLabels->setSelected( myItem,true);
    }

    for (QStringList::Iterator myIterator = myBandNameList.begin(); 
        myIterator != myBandNameList.end(); 
        ++myIterator)
    {
      QString myQString = *myIterator;
#ifdef QGISDEBUG

      std::cout << "Inserting : " << myQString.toLocal8Bit().data() << std::endl;
#endif

      cboGray->addItem(myQString);
      cboRed->addItem(myQString);
      cboGreen->addItem(myQString);
      cboBlue->addItem(myQString);
    }

    cboRed->addItem(tr(QSTRING_NOT_SET));
    cboGreen->addItem(tr(QSTRING_NOT_SET));
    cboBlue->addItem(tr(QSTRING_NOT_SET));
    cboGray->addItem(tr(QSTRING_NOT_SET));
  }

  cboxTransparencyBand->addItem(tr(QSTRING_NOT_SET));
  cboxTransparencyLayer->addItem(tr(QSTRING_NOT_SET));
  QMap<QString, QgsMapLayer *> myLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer *>::iterator it;
  for(it = myLayers.begin(); it != myLayers.end(); it++)
  {
    if(QgsMapLayer::RASTER == it.value()->type())
    {
      cboxTransparencyLayer->addItem(it.value()->name());
    }
  }

  QString myThemePath = QgsApplication::themePath();
  QPixmap myPyramidPixmap(myThemePath + "/mIconPyramid.png");
  QPixmap myNoPyramidPixmap(myThemePath + "/mIconNoPyramid.png");

  pbnAddValuesManually->setIcon(QIcon(QPixmap(myThemePath + "/mActionNewAttribute.png")));
  pbnAddValuesFromDisplay->setIcon(QIcon(QPixmap(myThemePath + "/mActionContextHelp.png")));
  pbnRemoveSelectedRow->setIcon(QIcon(QPixmap(myThemePath + "/mActionDeleteAttribute.png")));
  pbnDefaultValues->setIcon(QIcon(QPixmap(myThemePath + "/mActionCopySelected.png")));
  pbnImportTransparentPixelValues->setIcon(QIcon(QPixmap(myThemePath + "/mActionFileOpen.png")));
  pbnExportTransparentPixelValues->setIcon(QIcon(QPixmap(myThemePath + "/mActionFileSave.png")));
  pbtnMakeContrastEnhancementAlgorithmDefault->setIcon(QIcon(QPixmap(myThemePath + "/mActionFileSave.png")));

  // Only do pyramids if dealing directly with GDAL.
  if (mRasterLayerIsGdal)
  {
    QgsRasterLayer::RasterPyramidList myPyramidList = mRasterLayer->buildRasterPyramidList();
    QgsRasterLayer::RasterPyramidList::iterator myRasterPyramidIterator;

    for ( myRasterPyramidIterator=myPyramidList.begin();
        myRasterPyramidIterator != myPyramidList.end();
        ++myRasterPyramidIterator )
    {
      if ((*myRasterPyramidIterator).existsFlag==true)
      {
        lbxPyramidResolutions->insertItem(myPyramidPixmap,
            QString::number((*myRasterPyramidIterator).xDim) + QString(" x ") + 
            QString::number((*myRasterPyramidIterator).yDim)); 
      }
      else
      {
        lbxPyramidResolutions->insertItem(myNoPyramidPixmap,
            QString::number((*myRasterPyramidIterator).xDim) + QString(" x ") + 
            QString::number((*myRasterPyramidIterator).yDim)); 
      }
    }
  }
  else if (mRasterLayerIsWms)
  {
    // disable Pyramids tab completely
    tabBar->setTabEnabled(tabBar->indexOf(tabPagePyramids), FALSE);

    // disable Histogram tab completely
    tabBar->setTabEnabled(tabBar->indexOf(tabPageHistogram), FALSE);
  }

  leSpatialRefSys->setText(mRasterLayer->srs().proj4String());

  // Set text for pyramid info box
  QString pyramidFormat("<h2>%1</h2><p>%2 %3 %4</p><b><font color='red'><p>%5</p><p>%6</p>");
  QString pyramidHeader    = tr("Description");
  QString pyramidSentence1 = tr("Large resolution raster layers can slow navigation in QGIS.");
  QString pyramidSentence2 = tr("By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.");
  QString pyramidSentence3 = tr("You must have write access in the directory where the original data is stored to build pyramids.");
  QString pyramidSentence4 = tr("Please note that building pyramids may alter the original data file and once created they cannot be removed!");
  QString pyramidSentence5 = tr("Please note that building pyramids could corrupt your image - always make a backup of your data first!");

  tePyramidDescription->setHtml(pyramidFormat.arg(pyramidHeader).arg(pyramidSentence1)
				.arg(pyramidSentence2).arg(pyramidSentence3)
				.arg(pyramidSentence4).arg(pyramidSentence5));

  // update based on lyr's current state
  sync();
} // QgsRasterLayerProperties ctor


QgsRasterLayerProperties::~QgsRasterLayerProperties()
{
}

/*
 *
 * PUBLIC METHODS
 *
 */
void QgsRasterLayerProperties::populateTransparencyTable()
{
  //Clear existsing color transparency list
  //NOTE: May want to just tableTransparency->clearContents() and fill back in after checking to be sure list and table are the same size
  QString myNumberFormatter;
  if(rbtnThreeBand->isChecked() && QgsRasterLayer::PALETTED_COLOR != mRasterLayer->getDrawingStyle() && 
                                   QgsRasterLayer::PALETTED_MULTI_BAND_COLOR != mRasterLayer->getDrawingStyle())
  {
    for(int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >= 0; myTableRunner--)
    {
      tableTransparency->removeRow(myTableRunner);
    }

    tableTransparency->clear();
    tableTransparency->setColumnCount(4);
    tableTransparency->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Red")));
    tableTransparency->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Green")));
    tableTransparency->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Blue")));
    tableTransparency->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Percent Transparent")));

    //populate three band transparency list
    QList<QgsRasterTransparency::TransparentThreeValuePixel> myTransparentThreeValuePixelList = mRasterLayer->getRasterTransparency()->getTransparentThreeValuePixelList();
    for(int myListRunner = 0; myListRunner < myTransparentThreeValuePixelList.count(); myListRunner++)
    {
      tableTransparency->insertRow(myListRunner);
      QTableWidgetItem* myRedItem = new QTableWidgetItem(myNumberFormatter.sprintf("%.2f",myTransparentThreeValuePixelList[myListRunner].red));
      QTableWidgetItem* myGreenItem = new QTableWidgetItem(myNumberFormatter.sprintf("%.2f",myTransparentThreeValuePixelList[myListRunner].green));
      QTableWidgetItem* myBlueItem = new QTableWidgetItem(myNumberFormatter.sprintf("%.2f",myTransparentThreeValuePixelList[myListRunner].blue));
      QTableWidgetItem* myPercentTransparentItem = new QTableWidgetItem(myNumberFormatter.sprintf("%.2f", myTransparentThreeValuePixelList[myListRunner].percentTransparent));

      tableTransparency->setItem(myListRunner, 0, myRedItem);
      tableTransparency->setItem(myListRunner, 1, myGreenItem);
      tableTransparency->setItem(myListRunner, 2, myBlueItem);
      tableTransparency->setItem(myListRunner, 3, myPercentTransparentItem);
    }
  }
  else
  {
    //Clear existing single band or pallet values gransparency list
    for(int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >=0; myTableRunner--)
    {
      tableTransparency->removeRow(myTableRunner);
    }

    tableTransparency->clear();
    tableTransparency->setColumnCount(2);
    if(QgsRasterLayer::PALETTED_COLOR != mRasterLayer->getDrawingStyle() && 
       QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY != mRasterLayer->getDrawingStyle() && 
       QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR != mRasterLayer->getDrawingStyle() &&
       QgsRasterLayer::PALETTED_MULTI_BAND_COLOR != mRasterLayer->getDrawingStyle())
    {
      tableTransparency->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Gray")));
    }
    else
    {
      tableTransparency->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Indexed Value")));
    }
    tableTransparency->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Percent Transparent")));

    //populate gray transparency list
    QList<QgsRasterTransparency::TransparentSingleValuePixel> myTransparentSingleValuePixelList = mRasterLayer->getRasterTransparency()->getTransparentSingleValuePixelList();
    for(int myListRunner = 0; myListRunner < myTransparentSingleValuePixelList.count(); myListRunner++)
    {
      tableTransparency->insertRow(myListRunner);
      QTableWidgetItem* myGrayItem = new QTableWidgetItem(myNumberFormatter.sprintf("%.2f",myTransparentSingleValuePixelList[myListRunner].pixelValue));
      QTableWidgetItem* myPercentTransparentItem = new QTableWidgetItem(myNumberFormatter.sprintf("%.2f", myTransparentSingleValuePixelList[myListRunner].percentTransparent));

      tableTransparency->setItem(myListRunner, 0, myGrayItem);
      tableTransparency->setItem(myListRunner, 1, myPercentTransparentItem);
    }
  }

  tableTransparency->resizeColumnsToContents();
  tableTransparency->resizeRowsToContents();
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
#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::sync called");
#endif
  cboxShowDebugInfo->hide();

#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::sync populate symbology tab");
#endif
  /*
   * Symbology Tab
   */
  //decide whether user can change rgb settings
#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::sync DrawingStyle = " + QString::number(mRasterLayer->getDrawingStyle()));
#endif
  switch (mRasterLayer->getDrawingStyle())
  {
    case QgsRasterLayer::SINGLE_BAND_GRAY:
      rbtnThreeBand->setEnabled(false);
      rbtnSingleBand->setEnabled(true);
      rbtnSingleBand->setChecked(true);
      pbtnLoadMinMax->setEnabled(true);
      cboxContrastEnhancementAlgorithm->setEnabled(true);
      labelContrastEnhancement->setEnabled(true);
      break;
    case QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR:
      rbtnThreeBand->setEnabled(false);
      rbtnSingleBand->setEnabled(true);
      rbtnSingleBand->setChecked(true);
      pbtnLoadMinMax->setEnabled(false);
      cboxContrastEnhancementAlgorithm->setEnabled(false);
      labelContrastEnhancement->setEnabled(false);
      break;
    case QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY:
      rbtnThreeBand->setEnabled(true);
      rbtnSingleBand->setEnabled(true);
      rbtnSingleBand->setChecked(true);
      rbtnThreeBandMinMax->setEnabled(false);
      rbtnThreeBandStdDev->setEnabled(false);
      pbtnLoadMinMax->setEnabled(false);
      cboxContrastEnhancementAlgorithm->setEnabled(false);
      labelContrastEnhancement->setEnabled(false);
      break;
    case QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR:
      rbtnThreeBand->setEnabled(true);
      rbtnSingleBand->setEnabled(true);
      rbtnSingleBand->setChecked(true);
      rbtnThreeBandMinMax->setEnabled(false);
      rbtnThreeBandStdDev->setEnabled(false);
      pbtnLoadMinMax->setEnabled(false);
      cboxContrastEnhancementAlgorithm->setEnabled(false);
      labelContrastEnhancement->setEnabled(false);
      break;
    case QgsRasterLayer::PALETTED_MULTI_BAND_COLOR:
      rbtnThreeBand->setEnabled(true);
      rbtnSingleBand->setEnabled(true);
      rbtnThreeBand->setChecked(true);
      rbtnThreeBandMinMax->setEnabled(false);
      rbtnThreeBandStdDev->setEnabled(false);
      pbtnLoadMinMax->setEnabled(false);
      cboxContrastEnhancementAlgorithm->setEnabled(false);
      labelContrastEnhancement->setEnabled(false);
      break;
    case QgsRasterLayer::MULTI_BAND_SINGLE_BAND_GRAY:
      rbtnThreeBand->setEnabled(true);
      rbtnSingleBand->setEnabled(true);
      rbtnSingleBand->setChecked(true);
      pbtnLoadMinMax->setEnabled(true);
      cboxContrastEnhancementAlgorithm->setEnabled(true);
      labelContrastEnhancement->setEnabled(true);
      break;
    case QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR:
      rbtnThreeBand->setEnabled(true);
      rbtnSingleBand->setEnabled(true);
      rbtnSingleBand->setChecked(true);
      pbtnLoadMinMax->setEnabled(false);
      cboxContrastEnhancementAlgorithm->setEnabled(false);
      labelContrastEnhancement->setEnabled(false);
      break;
    case QgsRasterLayer::MULTI_BAND_COLOR:
      rbtnThreeBand->setEnabled(true);
      rbtnSingleBand->setEnabled(true);
      rbtnThreeBand->setChecked(true);
      pbtnLoadMinMax->setEnabled(true);
      cboxContrastEnhancementAlgorithm->setEnabled(true);
      labelContrastEnhancement->setEnabled(true);
      break;
    default:
      break;
  }
  
  if(mRasterLayerIsWms)
  {
    tabBar->setCurrentIndex(tabBar->indexOf(tabPageMetadata));
    tabBar->removeTab(tabBar->indexOf(tabPageColormap));
    tabBar->removeTab(tabBar->indexOf(tabPageSymbology));
    tabBar->removeTab(tabBar->indexOf(tabPageTransparency));
    tabBar->removeTab(tabBar->indexOf(tabPageHistogram));
    tabBar->removeTab(tabBar->indexOf(tabPagePyramids));
  }
/*
  if (mRasterLayer->getRasterLayerType() == QgsRasterLayer::MULTIBAND)
  {
    //multiband images can also be rendered as single band (using only one of the bands)
    txtSymologyNotes->
      setText(tr
          ("<h3>Multiband Image Notes</h3><p>This is a multiband image. You can choose to render it as grayscale or color (RGB). For color images, you can associate bands to colors arbitarily. For example, if you have a seven band landsat image, you may choose to render it as:</p><ul><li>Visible Blue (0.45 to 0.52 microns) - not mapped</li><li>Visible Green (0.52 to 0.60 microns) - not mapped</li></li>Visible Red (0.63 to 0.69 microns) - mapped to red in image</li><li>Near Infrared (0.76 to 0.90 microns) - mapped to green in image</li><li>Mid Infrared (1.55 to 1.75 microns) - not mapped</li><li>Thermal Infrared (10.4 to 12.5 microns) - not mapped</li><li>Mid Infrared (2.08 to 2.35 microns) - mapped to blue in image</li></ul>"));
  }
  else if (mRasterLayer->getRasterLayerType() == QgsRasterLayer::PALETTE)
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
*/
  //
  // Populate the various controls on the form
  //
#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::sync colorShadingAlgorithm = " + QString::number(mRasterLayer->getColorShadingAlgorithm()));
#endif
  if (mRasterLayer->getDrawingStyle() == QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR || 
      mRasterLayer->getDrawingStyle() == QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR || 
      mRasterLayer->getDrawingStyle() == QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR) 
  {   
    if(mRasterLayer->getColorShadingAlgorithm()==QgsRasterLayer::PSEUDO_COLOR)
    {
      cboxColorMap->setCurrentText(tr("Pseudocolor"));
    }
    else if(mRasterLayer->getColorShadingAlgorithm()==QgsRasterLayer::FREAK_OUT)
    {
      cboxColorMap->setCurrentText(tr("Freak Out"));
    }
    else if(mRasterLayer->getColorShadingAlgorithm()==QgsRasterLayer::COLOR_RAMP)
    {
      cboxColorMap->setCurrentText(tr("Custom Colormap"));
    }
    else if(mRasterLayer->getColorShadingAlgorithm()==QgsRasterLayer::USER_DEFINED)
    {
      cboxColorMap->setCurrentText(tr("User Defined"));
    }
  }
  else
  {
    cboxColorMap->setCurrentText(tr("Grayscale"));
  }
  
  //set whether the layer histogram should be inverted
  if (mRasterLayer->getInvertHistogramFlag())
  {
    cboxInvertColorMap->setChecked(true);
  }
  else
  {
    cboxInvertColorMap->setChecked(false);
  }

  //set the combos to the correct values
  cboRed->setCurrentText(mRasterLayer->getRedBandName());
  cboGreen->setCurrentText(mRasterLayer->getGreenBandName());
  cboBlue->setCurrentText(mRasterLayer->getBlueBandName());
  cboGray->setCurrentText(mRasterLayer->getGrayBandName());

  //set the stdDevs and min max values
  if(mRasterLayerIsGdal && rbtnThreeBand->isChecked())
  {
    if(mRasterLayer->getUserDefinedRGBMinMax())
    {
      sboxThreeBandStdDev->setValue(0.0);
      rbtnThreeBandStdDev->setChecked(false);
      rbtnThreeBandMinMax->setChecked(true);
    }
    else
    {
      sboxThreeBandStdDev->setValue(mRasterLayer->getStdDevsToPlot());
      rbtnThreeBandStdDev->setChecked(true);
      rbtnThreeBandMinMax->setChecked(false);
    }
    
    if(QgsRasterLayer::PALETTED_COLOR != mRasterLayer->getDrawingStyle() && 
       QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY != mRasterLayer->getDrawingStyle() && 
       QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR != mRasterLayer->getDrawingStyle() &&
       QgsRasterLayer::PALETTED_MULTI_BAND_COLOR != mRasterLayer->getDrawingStyle())
    {
      if(mRasterLayer->getRedBandName() != tr(QSTRING_NOT_SET))
      {
        leRedMin->setText(QString::number(mRasterLayer->getMinimumValue(mRasterLayer->getRedBandName())));
        leRedMax->setText(QString::number(mRasterLayer->getMaximumValue(mRasterLayer->getRedBandName())));
      }
      if(mRasterLayer->getGreenBandName() != tr(QSTRING_NOT_SET))
      {
        leGreenMin->setText(QString::number(mRasterLayer->getMinimumValue(mRasterLayer->getGreenBandName())));
        leGreenMax->setText(QString::number(mRasterLayer->getMaximumValue(mRasterLayer->getGreenBandName())));
      }
      if(mRasterLayer->getBlueBandName() != tr(QSTRING_NOT_SET))
      {
        leBlueMin->setText(QString::number(mRasterLayer->getMinimumValue(mRasterLayer->getBlueBandName())));
        leBlueMax->setText(QString::number(mRasterLayer->getMaximumValue(mRasterLayer->getBlueBandName())));
      }
    }
  }
  else if(mRasterLayerIsGdal)
  {
    if(mRasterLayer->getUserDefinedGrayMinMax())
    {
      sboxSingleBandStdDev->setValue(0.0);
      rbtnSingleBandStdDev->setChecked(false);
      rbtnSingleBandMinMax->setChecked(true);
    }
    else
    {
      sboxSingleBandStdDev->setValue(mRasterLayer->getStdDevsToPlot());
      rbtnSingleBandStdDev->setChecked(true);
      rbtnSingleBandMinMax->setChecked(false);
    }

    if(QgsRasterLayer::PALETTED_COLOR != mRasterLayer->getDrawingStyle() && 
       QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY != mRasterLayer->getDrawingStyle() && 
       QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR != mRasterLayer->getDrawingStyle() &&
       QgsRasterLayer::PALETTED_MULTI_BAND_COLOR != mRasterLayer->getDrawingStyle())
    {
      if(mRasterLayer->getGrayBandName() != tr(QSTRING_NOT_SET))
      {
        leGrayMin->setText(QString::number(mRasterLayer->getMinimumValue(mRasterLayer->getGrayBandName())));
        leGrayMax->setText(QString::number(mRasterLayer->getMaximumValue(mRasterLayer->getGrayBandName())));
      }
    }

  }

  //set color scaling algorithm
  if(QgsContrastEnhancement::STRETCH_TO_MINMAX == mRasterLayer->getContrastEnhancementAlgorithm())
  {
    cboxContrastEnhancementAlgorithm->setCurrentText(tr("Stretch To MinMax"));
  }
  else if(QgsContrastEnhancement::STRETCH_AND_CLIP_TO_MINMAX == mRasterLayer->getContrastEnhancementAlgorithm())
  {
    cboxContrastEnhancementAlgorithm->setCurrentText(tr("Stretch And Clip To MinMax"));
  }
  else if(QgsContrastEnhancement::CLIP_TO_MINMAX == mRasterLayer->getContrastEnhancementAlgorithm())
  {
    cboxContrastEnhancementAlgorithm->setCurrentText(tr("Clip To MinMax"));
  }
  else if(QgsContrastEnhancement::USER_DEFINED == mRasterLayer->getContrastEnhancementAlgorithm())
  {
    cboxContrastEnhancementAlgorithm->setCurrentText(tr("User Defined"));
  }
  else
  {
    cboxContrastEnhancementAlgorithm->setCurrentText(tr("No Scaling"));
  }
  
  //Display the current default contrast enhancement algorithm
  QSettings myQSettings;
  QString myDefaultAlgorithm = myQSettings.value("/Raster/defaultContrastEnhancementAlgorithm", "NO_STRETCH").toString();
  if(myDefaultAlgorithm == "NO_STRETCH")
  {
    labelDefaultContrastEnhancementAlgorithm->setText(tr("No Scaling"));
  }
  if(myDefaultAlgorithm == "STRETCH_TO_MINMAX")
  {
    labelDefaultContrastEnhancementAlgorithm->setText(tr("Stretch To MinMax"));
  }
  else if(myDefaultAlgorithm == "STRETCH_AND_CLIP_TO_MINMAX")
  {
    labelDefaultContrastEnhancementAlgorithm->setText(tr("Stretch And Clip To MinMax"));
  }
  else if(myDefaultAlgorithm == "CLIP_TO_MINMAX")
  {
    labelDefaultContrastEnhancementAlgorithm->setText(tr("Clip To MinMax"));
  }
  else
  {
    labelDefaultContrastEnhancementAlgorithm->setText(tr("No Scaling"));
  }



#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::sync populate transparency tab");
#endif
  /*
   * Transparent Pixel Tab
   */
  
  //set the transparency slider
  sliderTransparency->setValue(255 - mRasterLayer->getTransparency());
  //update the transparency percentage label
  sliderTransparency_valueChanged(255 - mRasterLayer->getTransparency());
   
  int myIndex = cboxTransparencyLayer->findText(mRasterLayer->getTransparentLayerName());
  if(-1 != myIndex)
  {
    cboxTransparencyLayer->setCurrentIndex(myIndex);
  }
  else
  {
    cboxTransparencyLayer->setCurrentIndex(cboxTransparencyLayer->findText(tr(QSTRING_NOT_SET)));
  }

  myIndex = cboxTransparencyBand->findText(mRasterLayer->getTransparentBandName());
  if(-1 != myIndex)
  {
    cboxTransparencyBand->setCurrentIndex(myIndex);
  }
  else
  {
    cboxTransparencyBand->setCurrentIndex(cboxTransparencyBand->findText(tr(QSTRING_NOT_SET)));
  }
  //add current NoDataValue to NoDataValue line edit 
  if(mRasterLayer->isNoDataValueValid())
  {
    leNoDataValue->setText(QString::number(mRasterLayer->getNoDataValue(), 'f'));
  }
  else
  {
    leNoDataValue->setText("");
  }
  
  //restore colormap tab if the layer has custom classification
  if(cboxColorMap->currentText() == tr("Custom Colormap"))
  {
    syncColormapTab();
  }

#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::sync populate general tab");
#endif
  /*
   * General Tab
   */
  cboxShowDebugInfo->hide();

  //these properties (layername and label) are provided by the qgsmaplayer superclass
  leLayerSource->setText(mRasterLayer->source());
  leDisplayName->setText(mRasterLayer->name());

  //update the debug checkbox
  cboxShowDebugInfo->setChecked(mRasterLayer->getShowDebugOverlayFlag());

  //display the raster dimensions and no data value
  if (mRasterLayerIsGdal)
  {
    lblColumns->setText(tr("Columns: ") + QString::number(mRasterLayer->getRasterXDim()));
    lblRows->setText(tr("Rows: ") + QString::number(mRasterLayer->getRasterYDim()));
    if(mRasterLayer->isNoDataValueValid())
    {
      lblNoData->setText(tr("No-Data Value: ") + QString::number(mRasterLayer->getNoDataValue()));
    }
    else
    {
      lblNoData->setText(tr("No-Data Value: Not Set"));
    }
  }
  else if (mRasterLayerIsWms)
  {
    // TODO: Account for fixedWidth and fixedHeight WMS layers
    lblColumns->setText(tr("Columns: ") + tr("n/a"));
    lblRows->setText(tr("Rows: ") + tr("n/a"));
    lblNoData->setText(tr("No-Data Value: ") + tr("n/a"));
  }

  //get the thumbnail for the layer
  QPixmap myQPixmap = QPixmap(pixmapThumbnail->width(),pixmapThumbnail->height());
  mRasterLayer->drawThumbnail(&myQPixmap);
  pixmapThumbnail->setPixmap(myQPixmap);

  //update the legend pixmap on this dialog
  pixmapLegend->setPixmap(mRasterLayer->getLegendQPixmap());
  pixmapLegend->setScaledContents(true);
  pixmapLegend->repaint(false);

  //set the palette pixmap
  pixmapPalette->setPixmap(mRasterLayer->getPaletteAsPixmap());
  pixmapPalette->setScaledContents(true);
  pixmapPalette->repaint(false);

#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::sync populate metadata tab");
#endif
  /*
   * Metadata Tab
   */
  //populate the metadata tab's text browser widget with gdal metadata info
  QString myStyle = QgsApplication::reportStyleSheet();
  txtbMetadata->setHtml(mRasterLayer->getMetadata());
  txtbMetadata->document()->setDefaultStyleSheet(myStyle);

} // QgsRasterLayerProperties::sync()

void QgsRasterLayerProperties::syncColormapTab()
{
#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::sync populate color ramp tab");
#endif
  if(!mRasterLayerIsGdal)
  {
    return;
  }
  
  if(!mRasterLayer)
  {
    return;
  }
  
  if(QgsRasterLayer::COLOR_RAMP != mRasterLayer->getColorShadingAlgorithm())
  {
    return;
  }
  
  QgsColorRampShader* myRasterShaderFunction =  (QgsColorRampShader*)mRasterLayer->getRasterShader()->getRasterShaderFunction();
  if(!myRasterShaderFunction)
  {
    return;
  }
  //restore the colormap tab if layer has custom symbology
  QList<QgsColorRampShader::ColorRampItem> myColorRampList = myRasterShaderFunction->getColorRampItemList();
  if(myColorRampList.size() > 0)
  {
    QList<QgsColorRampShader::ColorRampItem>::const_iterator it;
    for(it = myColorRampList.begin(); it != myColorRampList.end(); ++it)
    {
	    //restore state of colormap tab
	    QTreeWidgetItem* newItem = new QTreeWidgetItem(mColormapTreeWidget);
	    newItem->setText(0, QString::number(it->value, 'f'));
	    newItem->setBackground(1, QBrush(it->color));
	    newItem->setText(2, it->label);
	  }
  }

  sboxNumberOfEntries->setValue(myColorRampList.size());

  //restor state of 'color interpolation' combo box
  if(QgsColorRampShader::DISCRETE == myRasterShaderFunction->getColorRampType())
  {
    cboxColorInterpolation->setCurrentIndex(cboxColorInterpolation->findText(tr("Discrete")));
  }
  else
  {
    cboxColorInterpolation->setCurrentIndex(cboxColorInterpolation->findText(tr("Linearly")));
  }

}

bool QgsRasterLayerProperties::validUserDefinedMinMax()
{
  if(rbtnThreeBand->isChecked())
  {
    bool myDoubleOk;
    leRedMin->text().toDouble(&myDoubleOk);
    if(myDoubleOk)
    {
      leRedMax->text().toDouble(&myDoubleOk);
      if(myDoubleOk) 
      {
        leGreenMin->text().toDouble(&myDoubleOk);
        if(myDoubleOk)
        {
          leGreenMax->text().toDouble(&myDoubleOk);
          if(myDoubleOk)
          {
            leBlueMin->text().toDouble(&myDoubleOk);
            if(myDoubleOk)
            {
              leBlueMax->text().toDouble(&myDoubleOk);
              if(myDoubleOk)
              {
                return true;
              }
            }
          }
        }
      }
    }
  }
  else
  {
    bool myDoubleOk;
    leGrayMin->text().toDouble(&myDoubleOk);
    if(myDoubleOk)
    {
      leGrayMax->text().toDouble(&myDoubleOk);
      if(myDoubleOk)
      {
        return true;
      }
    }
  }

  return false;
}


/*
 *
 * PUBLIC AND PRIVATE SLOTS
 *
 */
void QgsRasterLayerProperties::apply()
{
#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::apply processing symbology tab");
#endif
  /*
   * Symbology Tab
   */
  //set the appropriate render style
  if (rbtnSingleBand->isChecked())
  {
    //
    // Grayscale
    //
    if (mRasterLayer->getRasterLayerType() == QgsRasterLayer::GRAY_OR_UNDEFINED)
    {

      if (cboxColorMap->currentText() != tr("Grayscale"))
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: SINGLE_BAND_PSEUDO_COLOR" << std::endl;
#endif

        mRasterLayer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: SINGLE_BAND_GRAY" << std::endl;
#endif

        mRasterLayer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_GRAY);
      }
    }
    //
    // Paletted Image
    //
    else if (mRasterLayer->getRasterLayerType() == QgsRasterLayer::PALETTE)
    {
      if (cboxColorMap->currentText() != tr("Grayscale"))
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: PALETTED_SINGLE_BAND_PSEUDO_COLOR" << std::endl;
#endif

        mRasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR);
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: PALETTED_SINGLE_BAND_GRAY" << std::endl;
#endif
#ifdef QGISDEBUG

        std::cout << "Combo value : " << cboGray->currentText().toLocal8Bit().data() << " GrayBand Mapping : " << mRasterLayer->
          getGrayBandName().toLocal8Bit().data() << std::endl;
#endif

        mRasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY);
      }
    }
    //
    // Mutltiband
    //
    else if (mRasterLayer->getRasterLayerType() == QgsRasterLayer::MULTIBAND)
    {
      if (cboxColorMap->currentText() != tr("Grayscale"))
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to ::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR " << std::endl;
#endif

        mRasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR);
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "Setting Raster Drawing Style to :: MULTI_BAND_SINGLE_BAND_GRAY" << std::endl;
        std::cout << "Combo value : " << cboGray->currentText().toLocal8Bit().data() << " GrayBand Mapping : " << mRasterLayer->
          getGrayBandName().toLocal8Bit().data() << std::endl;
#endif

        mRasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_SINGLE_BAND_GRAY);

      }
    }
  }                           //end of grayscale box enabled and rbtnsingleband checked
  else                          //assume that rbtnThreeBand is checked and render in rgb color
  {
    //set the grayscale color table type if the groupbox is enabled

    if (mRasterLayer->getRasterLayerType() == QgsRasterLayer::PALETTE)
    {
#ifdef QGISDEBUG
      std::cout << "Setting Raster Drawing Style to :: PALETTED_MULTI_BAND_COLOR" << std::endl;
#endif

      mRasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_MULTI_BAND_COLOR);
    }
    else if (mRasterLayer->getRasterLayerType() == QgsRasterLayer::MULTIBAND)
    {

#ifdef QGISDEBUG
      std::cout << "Setting Raster Drawing Style to :: MULTI_BAND_COLOR" << std::endl;
#endif

      mRasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_COLOR);
    }

  }

  //set whether the layer histogram should be inverted
  if (cboxInvertColorMap->isChecked())
  {
    mRasterLayer->setInvertHistogramFlag(true);
  }
  else
  {
    mRasterLayer->setInvertHistogramFlag(false);
  }

  //set transparency
  mRasterLayer->setTransparency(static_cast < unsigned int >(255 - sliderTransparency->value()));

  //now set the color -> band mapping combos to the correct values
  mRasterLayer->setRedBandName(cboRed->currentText());
  mRasterLayer->setGreenBandName(cboGreen->currentText());
  mRasterLayer->setBlueBandName(cboBlue->currentText());
  mRasterLayer->setGrayBandName(cboGray->currentText());
  mRasterLayer->setTransparentBandName(cboxTransparencyBand->currentText());
  mRasterLayer->setTransparentLayerName(cboxTransparencyLayer->currentText());

  //set the appropriate color shading type
  //If USER_DEFINED do nothing, user defined can only be set programatically
  if (cboxColorMap->currentText() == tr("Pseudocolor"))
  {
    mRasterLayer->setColorShadingAlgorithm(QgsRasterLayer::PSEUDO_COLOR);  
  }
  else if (cboxColorMap->currentText() == tr("Freak Out"))
  {
    mRasterLayer->setColorShadingAlgorithm(QgsRasterLayer::FREAK_OUT);  
  }
  else if (cboxColorMap->currentText() == tr("Custom Colormap"))
  {
    mRasterLayer->setColorShadingAlgorithm(QgsRasterLayer::COLOR_RAMP);
  }

  //set the color scaling algorithm
  //Since the maximum, minimum values are going to be set anyway, pass in false for the second parameter of setContrastEnahancementAlgorithm
  //so the the look up tables are not generated for each band, since their parameters are about to change anyway.This will also generate the 
  //lookup tables for the three or one band(s) that are immediately needed
  if(cboxContrastEnhancementAlgorithm->currentText() == tr("Stretch To MinMax"))
  {
    mRasterLayer->setContrastEnhancementAlgorithm(QgsContrastEnhancement::STRETCH_TO_MINMAX, false);
  }
  else if(cboxContrastEnhancementAlgorithm->currentText() == tr("Stretch And Clip To MinMax"))
  {
    mRasterLayer->setContrastEnhancementAlgorithm(QgsContrastEnhancement::STRETCH_AND_CLIP_TO_MINMAX, false);
  }
  else if(cboxContrastEnhancementAlgorithm->currentText() == tr("Clip To MinMax"))
  {
    mRasterLayer->setContrastEnhancementAlgorithm(QgsContrastEnhancement::CLIP_TO_MINMAX, false);
  }
  else if(QgsContrastEnhancement::USER_DEFINED == mRasterLayer->getContrastEnhancementAlgorithm())
  {
    //do nothing
  }
  else
  {
    mRasterLayer->setContrastEnhancementAlgorithm(QgsContrastEnhancement::NO_STRETCH, false);
  }
  
  //set the std deviations to be plotted and check for user defined Min Max values
  if(mRasterLayerIsGdal && rbtnThreeBand->isChecked())
  {
    //Set min max based on user defined values if all are set and stdDev is 0.0
    if(rbtnThreeBandMinMax->isEnabled() && rbtnThreeBandMinMax->isChecked() && validUserDefinedMinMax())
    {
      if(mRasterLayer->getRedBandName() != tr(QSTRING_NOT_SET))
      {
        mRasterLayer->setMinimumValue(cboRed->currentText(), leRedMin->text().toDouble(), false);
        mRasterLayer->setMaximumValue(cboRed->currentText(), leRedMax->text().toDouble());
      }
      if(mRasterLayer->getGreenBandName() != tr(QSTRING_NOT_SET))
      {
        mRasterLayer->setMinimumValue(cboGreen->currentText(), leGreenMin->text().toDouble(), false);
        mRasterLayer->setMaximumValue(cboGreen->currentText(), leGreenMax->text().toDouble());
      }
      if(mRasterLayer->getBlueBandName() != tr(QSTRING_NOT_SET))
      {
        mRasterLayer->setMinimumValue(cboBlue->currentText(), leBlueMin->text().toDouble(), false);
        mRasterLayer->setMaximumValue(cboBlue->currentText(), leBlueMax->text().toDouble());
      }
      mRasterLayer->setStdDevsToPlot(0.0);
      mRasterLayer->setUserDefinedRGBMinMax(true);
    }
    else if(rbtnThreeBandStdDev->isEnabled() && rbtnThreeBandStdDev->isChecked())
    {
      mRasterLayer->setStdDevsToPlot(sboxThreeBandStdDev->value());
      mRasterLayer->setUserDefinedRGBMinMax(false);
    }
    else
    {
      mRasterLayer->setStdDevsToPlot(0.0);
      mRasterLayer->setUserDefinedRGBMinMax(false);
    }
  }
  else if(mRasterLayerIsGdal)
  {
    //Set min max based on user defined values if all are set and stdDev is 0.0
    if(rbtnSingleBandMinMax->isEnabled() && rbtnSingleBandMinMax->isChecked() && validUserDefinedMinMax())
    {
      if(mRasterLayer->getGrayBandName() != tr(QSTRING_NOT_SET))
      {
        mRasterLayer->setMinimumValue(cboGray->currentText(), leGrayMin->text().toDouble(), false);
        mRasterLayer->setMaximumValue(cboGray->currentText(), leGrayMax->text().toDouble());
      }
      mRasterLayer->setStdDevsToPlot(0.0);
      mRasterLayer->setUserDefinedGrayMinMax(true);
    }
    else if(rbtnSingleBandStdDev->isEnabled() && rbtnSingleBandStdDev->isChecked())
    {
      mRasterLayer->setStdDevsToPlot(sboxSingleBandStdDev->value());
      mRasterLayer->setUserDefinedGrayMinMax(false);
    }
    else
    {
      mRasterLayer->setStdDevsToPlot(0.0);
      mRasterLayer->setUserDefinedGrayMinMax(false);
    }
  }

#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::apply processing transparency tab");
#endif
  /*
   * Transparent Pixel Tab
   */
  //If reset NoDataValue is checked do this first, will ignore what ever is in the LineEdit
  if(mRasterLayerIsGdal && chkboxResetNoDataValue->isChecked())
  {
    mRasterLayer->resetNoDataValue();
    if(mRasterLayer->isNoDataValueValid())
    {
      leNoDataValue->setText(QString::number(mRasterLayer->getNoDataValue(), 'f'));
    }
    else
    {
      leNoDataValue->setText("");
    }
    chkboxResetNoDataValue->setChecked(false);
  } 
   
  //set NoDataValue
  bool myDoubleOk = false;
  if("" != leNoDataValue->text())
  {
    double myNoDataValue = leNoDataValue->text().toDouble(&myDoubleOk);
    if(myDoubleOk)
    {
      mRasterLayer->setNoDataValue(myNoDataValue);
    }
  }

  //Walk through each row in table and test value. If not valid set to 0.0 and continue building transparency list
  if(rbtnThreeBand->isChecked() && QgsRasterLayer::MULTI_BAND_COLOR == mRasterLayer->getDrawingStyle())
  {
    double myTransparentValue;
    double myPercentTransparent;
    QgsRasterTransparency::TransparentThreeValuePixel myTransparentPixel;
    QList<QgsRasterTransparency::TransparentThreeValuePixel> myTransparentThreeValuePixelList;
    for(int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++)
    {
      if(!tableTransparency->item(myListRunner, 0))
      {
        myTransparentPixel.red = 0.0;
        QTableWidgetItem* newItem = new QTableWidgetItem("0.0");
        tableTransparency->setItem(myListRunner, 0, newItem);
      }
      else 
      {
        myTransparentValue = tableTransparency->item(myListRunner, 0)->text().toDouble(&myDoubleOk);
        if(myDoubleOk)
        {
          myTransparentPixel.red = myTransparentValue;
        }
        else 
        {
          myTransparentPixel.red = 0.0;
          tableTransparency->item(myListRunner, 0)->setText("0.0");
        }
      }

      if(!tableTransparency->item(myListRunner, 1))
      {
        myTransparentPixel.green = 0.0;
        QTableWidgetItem* newItem = new QTableWidgetItem("0.0");
        tableTransparency->setItem(myListRunner, 1, newItem);
      }
      else 
      {
        myTransparentValue = tableTransparency->item(myListRunner, 1)->text().toDouble(&myDoubleOk);
        if(myDoubleOk)
        {
          myTransparentPixel.green = myTransparentValue;
        }
        else 
        {
          myTransparentPixel.green = 0.0;
          tableTransparency->item(myListRunner, 1)->setText("0.0");
        }
      }

      if(!tableTransparency->item(myListRunner, 2))
      {
        myTransparentPixel.blue = 0.0;
        QTableWidgetItem* newItem = new QTableWidgetItem("0.0");
        tableTransparency->setItem(myListRunner, 2, newItem);
      }
      else 
      {
        myTransparentValue = tableTransparency->item(myListRunner, 2)->text().toDouble(&myDoubleOk);
        if(myDoubleOk)
        {
          myTransparentPixel.blue = myTransparentValue;
        }
        else 
        {
          myTransparentPixel.blue = 0.0;
          tableTransparency->item(myListRunner, 2)->setText("0.0");
        }
      }

      if(!tableTransparency->item(myListRunner, 3))
      {
        myTransparentPixel.percentTransparent = 100.0;
        QTableWidgetItem* newItem = new QTableWidgetItem("100.0");
        tableTransparency->setItem(myListRunner, 3, newItem);
      }
      else 
      {
        QString myNumberFormatter;
        myPercentTransparent = tableTransparency->item(myListRunner, 3)->text().toDouble(&myDoubleOk);
        if(myDoubleOk)
        {
          if(myPercentTransparent > 100.0)
            myTransparentPixel.percentTransparent = 100.0;
          else if(myPercentTransparent < 0.0)
            myTransparentPixel.percentTransparent = 0.0;
          else
            myTransparentPixel.percentTransparent = myPercentTransparent;

          tableTransparency->item(myListRunner, 3)->setText(myNumberFormatter.sprintf("%.2f",myTransparentPixel.percentTransparent));
        }
        else 
        {
          myTransparentPixel.percentTransparent = 100.0;
          tableTransparency->item(myListRunner, 3)->setText("100.0");
        }
      }

      myTransparentThreeValuePixelList.append(myTransparentPixel);
    }

    mRasterLayer->getRasterTransparency()->setTransparentThreeValuePixelList(myTransparentThreeValuePixelList);
  }
  else
  {
    double myTransparentValue;
    double myPercentTransparent;
    QgsRasterTransparency::TransparentSingleValuePixel myTransparentPixel;
    QList<QgsRasterTransparency::TransparentSingleValuePixel> myTransparentSingleValuePixelList;
    for(int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++)
    {
      if(!tableTransparency->item(myListRunner, 0))
      {
        myTransparentPixel.pixelValue = 0.0;
        QTableWidgetItem* newItem = new QTableWidgetItem("0.0");
        tableTransparency->setItem(myListRunner, 0, newItem);
      }
      else 
      {
        myTransparentValue = tableTransparency->item(myListRunner, 0)->text().toDouble(&myDoubleOk);
        if(myDoubleOk)
        {
          myTransparentPixel.pixelValue = myTransparentValue;
        }
        else 
        {
          myTransparentPixel.pixelValue = 0.0;
          tableTransparency->item(myListRunner, 0)->setText("0.0");
        }
      }

      if(!tableTransparency->item(myListRunner, 1))
      {
        myTransparentPixel.percentTransparent = 100.0;
        QTableWidgetItem* newItem = new QTableWidgetItem("100.0");
        tableTransparency->setItem(myListRunner, 1, newItem);
      }
      else 
      {
        QString myNumberFormatter;
        myPercentTransparent = tableTransparency->item(myListRunner, 1)->text().toDouble(&myDoubleOk);
        if(myDoubleOk)
        {
          if(myPercentTransparent > 100.0)
            myTransparentPixel.percentTransparent = 100.0;
          else if(myPercentTransparent < 0.0)
            myTransparentPixel.percentTransparent = 0.0;
          else
            myTransparentPixel.percentTransparent = myPercentTransparent;

          tableTransparency->item(myListRunner, 1)->setText(myNumberFormatter.sprintf("%.2f",myTransparentPixel.percentTransparent));
        }
        else 
        {
          myTransparentPixel.percentTransparent = 100.0;
          tableTransparency->item(myListRunner, 1)->setText("100.0");
        }
      }

      myTransparentSingleValuePixelList.append(myTransparentPixel);
    }

    mRasterLayer->getRasterTransparency()->setTransparentSingleValuePixelList(myTransparentSingleValuePixelList);
  }
  
#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::apply processing Colormap tab");
#endif
  /*
   * ColorMap Tab
   */
  if(cboxColorMap->currentText() == tr("Custom Colormap"))
  {
    QgsColorRampShader* myRasterShaderFunction =  (QgsColorRampShader*)mRasterLayer->getRasterShader()->getRasterShaderFunction();
    if(myRasterShaderFunction)
    {
      //iterate through mColormapTreeWidget and set colormap info of layer
      QList<QgsColorRampShader::ColorRampItem> mColorRampItems;
  
      int myTopLevelItemCount = mColormapTreeWidget->topLevelItemCount();
      QTreeWidgetItem* myCurrentItem;
  
      for(int i = 0; i < myTopLevelItemCount; ++i)
      {
        myCurrentItem = mColormapTreeWidget->topLevelItem(i);
        if(!myCurrentItem)
        {
          continue;
        }
        QgsColorRampShader::ColorRampItem myNewColorRampItem;
        myNewColorRampItem.value = myCurrentItem->text(0).toDouble();
        myNewColorRampItem.color = myCurrentItem->background(1).color();
        myNewColorRampItem.label = myCurrentItem->text(2);
        mColorRampItems.push_back(myNewColorRampItem);
      }
      myRasterShaderFunction->setColorRampItemList(mColorRampItems);
  
      if(cboxColorInterpolation->currentText() == tr("Discrete"))
      {
        myRasterShaderFunction->setColorRampType(QgsColorRampShader::DISCRETE);
      }
      else
      {
        myRasterShaderFunction->setColorRampType(QgsColorRampShader::INTERPOLATED);
      }
    }
    else
    {
      #ifdef QGISDEBUG
        QgsDebugMsg("QgsRasterLayerProperties::apply color ramp was NOT set because RasterShaderFunction was NULL");
      #endif
    }
  }
  
#ifdef QGISDEBUG
      QgsDebugMsg("QgsRasterLayerProperties::apply processing general tab");
#endif
  /*
   * General Tab
   */
  mRasterLayer->setLayerName(leDisplayName->text());

  //see if the user would like debug overlays
  if (cboxShowDebugInfo->isChecked() == true)
  {
    mRasterLayer->setShowDebugOverlayFlag(true);
  }
  else
  {
    mRasterLayer->setShowDebugOverlayFlag(false);
  }

  // set up the scale based layer visibility stuff....
  mRasterLayer->setScaleBasedVisibility(chkUseScaleDependentRendering->isChecked());
  mRasterLayer->setMinScale(spinMinimumScale->value());
  mRasterLayer->setMaxScale(spinMaximumScale->value());

  //update the legend pixmap
  pixmapLegend->setPixmap(mRasterLayer->getLegendQPixmap());
  pixmapLegend->setScaledContents(true);
  pixmapLegend->repaint(false);

  //get the thumbnail for the layer
  QPixmap myQPixmap = QPixmap(pixmapThumbnail->width(),pixmapThumbnail->height());
  mRasterLayer->drawThumbnail(&myQPixmap);
  pixmapThumbnail->setPixmap(myQPixmap);

  // update symbology
  emit refreshLegend(mRasterLayer->getLayerID(), false);

  //make sure the layer is redrawn
  mRasterLayer->triggerRepaint();

  //Becuase Min Max values can be set during the redraw if a strech is requested we need to resync after apply
  if(mRasterLayerIsGdal && QgsContrastEnhancement::NO_STRETCH != mRasterLayer->getContrastEnhancementAlgorithm())
  {

    //set the stdDevs and min max values
    if(rbtnThreeBand->isChecked())
   {
      if(rbtnThreeBandStdDev->isEnabled())
      {
        sboxThreeBandStdDev->setValue(mRasterLayer->getStdDevsToPlot());
      }

      if(rbtnThreeBandMinMax->isEnabled())
      {
        if(mRasterLayer->getRedBandName() != tr(QSTRING_NOT_SET))
        {
          leRedMin->setText(QString::number(mRasterLayer->getMinimumValue(mRasterLayer->getRedBandName())));
          leRedMax->setText(QString::number(mRasterLayer->getMaximumValue(mRasterLayer->getRedBandName())));
        } 
        if(mRasterLayer->getGreenBandName() != tr(QSTRING_NOT_SET))
        {
          leGreenMin->setText(QString::number(mRasterLayer->getMinimumValue(mRasterLayer->getGreenBandName())));
          leGreenMax->setText(QString::number(mRasterLayer->getMaximumValue(mRasterLayer->getGreenBandName())));
        }
        if(mRasterLayer->getBlueBandName() != tr(QSTRING_NOT_SET))
        {
          leBlueMin->setText(QString::number(mRasterLayer->getMinimumValue(mRasterLayer->getBlueBandName())));
          leBlueMax->setText(QString::number(mRasterLayer->getMaximumValue(mRasterLayer->getBlueBandName())));
        }
      }
    }
    else
    {
      if(rbtnSingleBandStdDev->isEnabled())
      {
        sboxSingleBandStdDev->setValue(mRasterLayer->getStdDevsToPlot());
      }

      if(rbtnSingleBandMinMax->isEnabled())
      {
        if(mRasterLayer->getGrayBandName() != tr(QSTRING_NOT_SET))
        {
          leGrayMin->setText(QString::number(mRasterLayer->getMinimumValue(mRasterLayer->getGrayBandName())));
          leGrayMax->setText(QString::number(mRasterLayer->getMaximumValue(mRasterLayer->getGrayBandName())));
        }
      }
    }
  }

  //GUI Cleanup
  //Once the user has applied the changes, user defined function will not longer be a valid option so it should be
  //removed from the list
  if(-1 != cboxColorMap->findText(tr("User Defined")) && tr("User Defined") != cboxColorMap->currentText())
  {
    cboxColorMap->removeItem(cboxColorMap->findText(tr("User Defined")));
  }
  
  if(-1 != cboxContrastEnhancementAlgorithm->findText(tr("User Defined")) && tr("User Defined") != cboxContrastEnhancementAlgorithm->currentText())
  {
    cboxContrastEnhancementAlgorithm->removeItem(cboxContrastEnhancementAlgorithm->findText(tr("User Defined")));
  }

}//apply

void QgsRasterLayerProperties::on_buttonBox_helpRequested()
{
  QgsContextHelp::run(context_id);
}

void QgsRasterLayerProperties::on_buttonBuildPyramids_clicked()
{

  //
  // Go through the list marking any files that are selected in the listview
  // as true so that we can generate pyramids for them.
  //
  QgsRasterLayer::RasterPyramidList myPyramidList = mRasterLayer->buildRasterPyramidList();
  for ( unsigned int myCounterInt = 0; myCounterInt < lbxPyramidResolutions->count(); myCounterInt++ )
  {
    Q3ListBoxItem *myItem = lbxPyramidResolutions->item( myCounterInt );
    if ( myItem->isSelected() )
    {
      //mark to be pyramided
      myPyramidList[myCounterInt].existsFlag=true;
    }
  }
  //
  // Ask raster layer to build the pyramids
  //

  // let the user know we're going to possibly be taking a while
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QString res = mRasterLayer->buildPyramids(myPyramidList,cboResamplingMethod->currentText());
  QApplication::restoreOverrideCursor();
  if (!res.isNull())
  {
    if (res == "ERROR_WRITE_ACCESS")
    {
      QMessageBox::warning(this, tr("Write access denied"),
          tr("Write access denied. Adjust the file permissions and try again.\n\n") );
    }
    else if (res == "ERROR_WRITE_FORMAT")
    {
      QMessageBox::warning(this, tr("Building pyramids failed."),
          tr("The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.") );
    }
    else if (res == "FAILED_NOT_SUPPORTED")
    {
      QMessageBox::warning(this, tr("Building pyramids failed."),
          tr("Building pyramid overviews is not supported on this type of raster.") );
    }
  }


  //
  // repopulate the pyramids list
  //
  lbxPyramidResolutions->clear();
  QString myThemePath = QgsApplication::themePath();
  QPixmap myPyramidPixmap(myThemePath + "/mIconPyramid.png");
  QPixmap myNoPyramidPixmap(myThemePath + "/mIconNoPyramid.png");

  QgsRasterLayer::RasterPyramidList::iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator=myPyramidList.begin();
      myRasterPyramidIterator != myPyramidList.end();
      ++myRasterPyramidIterator )
  {
    if ((*myRasterPyramidIterator).existsFlag==true)
    {
      lbxPyramidResolutions->insertItem(myPyramidPixmap,
          QString::number((*myRasterPyramidIterator).xDim) + QString(" x ") + 
          QString::number((*myRasterPyramidIterator).yDim)); 
    }
    else
    {
      lbxPyramidResolutions->insertItem(myNoPyramidPixmap,
          QString::number((*myRasterPyramidIterator).xDim) + QString(" x ") + 
          QString::number((*myRasterPyramidIterator).yDim)); 
    }
  }
  //update the legend pixmap
  pixmapLegend->setPixmap(mRasterLayer->getLegendQPixmap());
  pixmapLegend->setScaledContents(true);
  pixmapLegend->repaint(false);
  //populate the metadata tab's text browser widget with gdal metadata info
  QString myStyle = QgsApplication::reportStyleSheet();
  txtbMetadata->setHtml(mRasterLayer->getMetadata());
  txtbMetadata->document()->setDefaultStyleSheet(myStyle);
}

void QgsRasterLayerProperties::on_cboBlue_currentIndexChanged(const QString& theText)
{
  if(mRasterLayerIsGdal && tr(QSTRING_NOT_SET) != theText)
  {
    leBlueMin->setText(QString::number(mRasterLayer->getMinimumValue(theText)));
    leBlueMax->setText(QString::number(mRasterLayer->getMaximumValue(theText)));
  }
}

void QgsRasterLayerProperties::on_cboGray_currentIndexChanged(const QString& theText)
{
  if(mRasterLayerIsGdal && tr(QSTRING_NOT_SET) != theText)
  {
    leGrayMin->setText(QString::number(mRasterLayer->getMinimumValue(theText)));
    leGrayMax->setText(QString::number(mRasterLayer->getMaximumValue(theText)));
  }
}

void QgsRasterLayerProperties::on_cboGreen_currentIndexChanged(const QString& theText)
{
  if(mRasterLayerIsGdal && tr(QSTRING_NOT_SET) != theText)
  {
    leGreenMin->setText(QString::number(mRasterLayer->getMinimumValue(theText)));
    leGreenMax->setText(QString::number(mRasterLayer->getMaximumValue(theText)));
  }
}

void QgsRasterLayerProperties::on_cboRed_currentIndexChanged(const QString& theText)
{
  if(mRasterLayerIsGdal && tr(QSTRING_NOT_SET) != theText)
  {
    leRedMin->setText(QString::number(mRasterLayer->getMinimumValue(theText)));
    leRedMax->setText(QString::number(mRasterLayer->getMaximumValue(theText)));
  }
}

void QgsRasterLayerProperties::on_pbnAddValuesFromDisplay_clicked()
{
  QMessageBox::warning(this, "Function Not Available", "This functionality will be added soon");
}

void QgsRasterLayerProperties::on_pbnAddValuesManually_clicked()
{
  tableTransparency->insertRow(tableTransparency->rowCount());
  tableTransparency->setItem(tableTransparency->rowCount() - 1, tableTransparency->columnCount() - 1, new QTableWidgetItem("100.0"));
}

void QgsRasterLayerProperties::on_pbnChangeSpatialRefSys_clicked()
{

  QgsLayerProjectionSelector * mySelector = new QgsLayerProjectionSelector(this);
  mySelector->setSelectedSRSID(mRasterLayer->srs().srsid());
  if(mySelector->exec())
  {
    QgsSpatialRefSys srs(mySelector->getCurrentSRSID(), QgsSpatialRefSys::QGIS_SRSID);
    mRasterLayer->setSrs(srs);
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }
  delete mySelector;

  leSpatialRefSys->setText(mRasterLayer->srs().proj4String());
}

void QgsRasterLayerProperties::on_cboxColorMap_currentIndexChanged(const QString& theText)
{
  if(mRasterLayerIsGdal && theText == tr("Pseudocolor") || theText == tr("Freak Out"))
  {
    tabBar->setTabEnabled(tabBar->indexOf(tabPageColormap), FALSE);
    rbtnSingleBandMinMax->setEnabled(false);
    rbtnSingleBandStdDev->setEnabled(true);
    sboxSingleBandStdDev->setEnabled(true);
    pbtnLoadMinMax->setEnabled(false);
    cboxContrastEnhancementAlgorithm->setEnabled(false);
    labelContrastEnhancement->setEnabled(false);
  }
  else if(mRasterLayerIsGdal && theText == tr("Custom Colormap"))
  {
    tabBar->setTabEnabled(tabBar->indexOf(tabPageColormap), TRUE);
    rbtnSingleBandMinMax->setEnabled(false);
    rbtnSingleBandStdDev->setEnabled(false);
    sboxSingleBandStdDev->setEnabled(false);
    pbtnLoadMinMax->setEnabled(false);
    cboxContrastEnhancementAlgorithm->setEnabled(false);
    labelContrastEnhancement->setEnabled(false);
  }
  else if(mRasterLayerIsGdal && theText == tr("User Defined"))
  {
    tabBar->setTabEnabled(tabBar->indexOf(tabPageColormap), FALSE);
    rbtnSingleBandMinMax->setEnabled(true);
    rbtnSingleBandStdDev->setEnabled(true);
    sboxSingleBandStdDev->setEnabled(true);
    pbtnLoadMinMax->setEnabled(true);
    cboxContrastEnhancementAlgorithm->setEnabled(false);
    labelContrastEnhancement->setEnabled(false);
  }
  else if(mRasterLayerIsGdal)
  {
    tabBar->setTabEnabled(tabBar->indexOf(tabPageColormap), FALSE);
    rbtnSingleBandMinMax->setEnabled(true);
    rbtnSingleBandStdDev->setEnabled(true);
    sboxSingleBandStdDev->setEnabled(true);
    if(mRasterLayer->getRasterLayerType() != QgsRasterLayer::PALETTE)
    {
      pbtnLoadMinMax->setEnabled(true);
      cboxContrastEnhancementAlgorithm->setEnabled(true);
      labelContrastEnhancement->setEnabled(true);
    }
    else
    {
      pbtnLoadMinMax->setEnabled(false);
      cboxContrastEnhancementAlgorithm->setEnabled(false);
      labelContrastEnhancement->setEnabled(false);
    }

  }
}

void QgsRasterLayerProperties::on_cboxTransparencyLayer_currentIndexChanged(const QString& theText)
{
  if(theText == tr(QSTRING_NOT_SET))
  {
    cboxTransparencyBand->clear();
    cboxTransparencyBand->addItem(tr(QSTRING_NOT_SET));
  }
  else
  {
    QMap<QString, QgsMapLayer *> myLayers = QgsMapLayerRegistry::instance()->mapLayers();
    QMap<QString, QgsMapLayer *>::iterator it;
    for(it = myLayers.begin(); it != myLayers.end(); it++)
    {
      if(theText == it.value()->name() && QgsMapLayer::RASTER == it.value()->type())
      {
        QgsRasterLayer* myRasterLayer = (QgsRasterLayer*)it.value();
        int myBandCount = myRasterLayer->getBandCount();
        cboxTransparencyBand->clear();
        cboxTransparencyBand->addItem(tr(QSTRING_NOT_SET));
        for(int bandRunner = 1; bandRunner <= myBandCount; bandRunner++)
        {
          cboxTransparencyBand->addItem(myRasterLayer->getRasterBandName(bandRunner));
        }
        break;
      }
    }
  }
}

void QgsRasterLayerProperties::on_pbnDefaultValues_clicked()
{

  if(rbtnThreeBand->isChecked() && QgsRasterLayer::PALETTED_COLOR != mRasterLayer->getDrawingStyle() && 
                                   QgsRasterLayer::PALETTED_MULTI_BAND_COLOR != mRasterLayer->getDrawingStyle())
  {
    tableTransparency->clear();
    tableTransparency->setColumnCount(4);
    tableTransparency->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Red")));
    tableTransparency->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Green")));
    tableTransparency->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Blue")));
    tableTransparency->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Percent Transparent")));
    if(mRasterLayer->isNoDataValueValid())
    {
      tableTransparency->insertRow(tableTransparency->rowCount());
      tableTransparency->setItem(0, 0, new QTableWidgetItem(QString::number(mRasterLayer->getNoDataValue(), 'f')));
      tableTransparency->setItem(0, 1, new QTableWidgetItem(QString::number(mRasterLayer->getNoDataValue(), 'f')));
      tableTransparency->setItem(0, 2, new QTableWidgetItem(QString::number(mRasterLayer->getNoDataValue(), 'f')));
      tableTransparency->setItem(0, 3, new QTableWidgetItem("100.0"));
    }
  }
  else
  {
    tableTransparency->clear();
    tableTransparency->setColumnCount(2);
    if(QgsRasterLayer::PALETTED_COLOR != mRasterLayer->getDrawingStyle() && 
       QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY != mRasterLayer->getDrawingStyle() && 
       QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR != mRasterLayer->getDrawingStyle() &&
       QgsRasterLayer::PALETTED_MULTI_BAND_COLOR != mRasterLayer->getDrawingStyle())
    {
      tableTransparency->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Gray")));
    }
    else
    {
      tableTransparency->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Indexed Value")));
    }
    tableTransparency->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Percent Transparent")));

    if(mRasterLayer->isNoDataValueValid())
    {
      tableTransparency->insertRow(tableTransparency->rowCount());
      tableTransparency->setItem(0, 0, new QTableWidgetItem(QString::number(mRasterLayer->getNoDataValue(), 'f')));
      tableTransparency->setItem(0, 1, new QTableWidgetItem("100.0"));
    }

  }
}

void QgsRasterLayerProperties::on_pbnExportTransparentPixelValues_clicked()
{
  QString myFilename = QFileDialog::getSaveFileName(this, tr("Save file"), "/", tr("Textfile (*.txt)"));
  if(!myFilename.isEmpty())
  {
    if(!myFilename.endsWith(".txt", Qt::CaseInsensitive))
    {
      myFilename = myFilename + ".txt";
    }

    QFile myOutputFile(myFilename);
    if (myOutputFile.open(QFile::WriteOnly))
    {
      QTextStream myOutputStream(&myOutputFile);
      myOutputStream << "# " << tr("QGIS Generated Transparent Pixel Value Export File") << "\n";
      if(rbtnThreeBand->isChecked() && QgsRasterLayer::MULTI_BAND_COLOR == mRasterLayer->getDrawingStyle())
      {
        myOutputStream << "#\n#\n# " << tr("Red") << "\t" << tr("Green") << "\t" << tr("Blue") << "\t" << tr("Percent Transparent");
        for(int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++)
        {
          myOutputStream << "\n" << tableTransparency->item(myTableRunner, 0)->text() << "\t" << tableTransparency->item(myTableRunner, 1)->text() << "\t" << tableTransparency->item(myTableRunner, 2)->text() << "\t" << tableTransparency->item(myTableRunner, 3)->text();
        }
      }
      else
      {
        if(QgsRasterLayer::PALETTED_COLOR != mRasterLayer->getDrawingStyle() && 
           QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY != mRasterLayer->getDrawingStyle() && 
           QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR != mRasterLayer->getDrawingStyle() &&
           QgsRasterLayer::PALETTED_MULTI_BAND_COLOR != mRasterLayer->getDrawingStyle())
        {
          myOutputStream << "#\n#\n# " << tr("Gray") << "\t" << tr("Percent Transparent");
        }
        else
        {
          myOutputStream << "#\n#\n# " << tr("Indexed Value") << "\t" << tr("Percent Transparent");
        }

        for(int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++)
        {
          myOutputStream << "\n" << tableTransparency->item(myTableRunner, 0)->text() << "\t" << tableTransparency->item(myTableRunner, 1)->text();
        }
      }
    }
    else
    {
      QMessageBox::warning(this, tr("Write access denied"), tr("Write access denied. Adjust the file permissions and try again.\n\n") );
    }
  }
}

void QgsRasterLayerProperties::on_pbnHistRefresh_clicked()
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayerProperties::on_pbnHistRefresh_clicked" << std::endl;
#endif
  int myBandCountInt = mRasterLayer->getBandCount();
  //
  // Find out how many bands are selected and short circuit out clearing the image
  // if needed
  int mySelectionCount=0;
  for (int myIteratorInt = 1;
      myIteratorInt <= myBandCountInt;
      ++myIteratorInt)
  {
    QgsRasterBandStats myRasterBandStats = mRasterLayer->getRasterBandStats(myIteratorInt);
    Q3ListBoxItem *myItem = lstHistogramLabels->item( myIteratorInt-1 );
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

#ifdef QGISDEBUG
  long myCellCount = mRasterLayer->getRasterXDim() * mRasterLayer->getRasterYDim();
#endif

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
  bool myFirstItemFlag=true;
  for (int myIteratorInt = 1;
      myIteratorInt <= myBandCountInt;
      ++myIteratorInt)
  {
    QgsRasterBandStats myRasterBandStats = mRasterLayer->getRasterBandStats(myIteratorInt);
    //calculate the x axis min max
    if (myRasterBandStats.minVal < myXAxisMin || myIteratorInt==1)
    {
      myXAxisMin=static_cast < unsigned int >(myRasterBandStats.minVal);
    }
    if (myRasterBandStats.maxVal < myXAxisMax || myIteratorInt==1)
    {
      myXAxisMax=static_cast < unsigned int >(myRasterBandStats.maxVal);
    }
    Q3ListBoxItem *myItem = lstHistogramLabels->item( myIteratorInt-1 );
    if ( myItem->isSelected() )
    {
#ifdef QGISDEBUG
      std::cout << "Ensuring hist is populated for this layer" << std::endl;
#endif
      mRasterLayer->populateHistogram(myIteratorInt,BINCOUNT,myIgnoreOutOfRangeFlag,myThoroughBandScanFlag); 

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
        if ( myBin==0 && myFirstItemFlag)
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
      myFirstItemFlag=false;
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
  //anti alias lines in the graph
  myPainter.setRenderHint(QPainter::Antialiasing);
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
  int myGraphImageWidth = myImageWidth-myYGutterWidth; 
  int myGraphImageHeight = myImageHeight-myXGutterHeight; 

  //find out how wide to draw bars when in bar chart mode
  int myBarWidth = static_cast<int>((((double)myGraphImageWidth)/((double)BINCOUNT)));


  //
  //now draw actual graphs
  //
  if (mRasterLayer->getRasterLayerType()
      == QgsRasterLayer::PALETTE) //paletted layers have hard coded color entries
  {
    QPolygonF myPolygon;
    QgsColorTable *myColorTable=mRasterLayer->colorTable(1);
#ifdef QGISDEBUG
    std::cout << "Making paletted image histogram....computing band stats" << std::endl;
    std::cout << "myLastBinWithData = " << myLastBinWithData << std::endl;
#endif

    QgsRasterBandStats myRasterBandStats = mRasterLayer->getRasterBandStats(1);
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
      myPolygon << QPointF( myX+myYGutterWidth, myY-myXGutterHeight);
    }
    //draw a line on the graph along the bar peaks; 
    if (myGraphType==LINE_CHART)
    {
      //close of the point array so it makes a nice polygon
      //bottom right point
      myPolygon << QPointF( 
          myImageWidth, 
          myImageHeight-myXGutterHeight);
      //bottom left point
      myPolygon << QPointF( 
          myYGutterWidth, 
          myImageHeight-myXGutterHeight);
      myPainter.setPen( Qt::black );
      //set a gradient fill for the path
      QLinearGradient myGradient = greenGradient();
      myPainter.setBrush(myGradient);
      QPainterPath myPath;
      myPath.addPolygon(myPolygon);
      myPainter.drawPath(myPath);
    }
  }
  else
  {

    for (int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt)
    {
      QgsRasterBandStats myRasterBandStats = mRasterLayer->getRasterBandStats(myIteratorInt);
      Q3ListBoxItem *myItem = lstHistogramLabels->item( myIteratorInt-1 );
      if ( myItem->isSelected() )
      {

        QPolygonF myPolygon;
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
            myPolygon << QPointF(myX+myYGutterWidth, myY);
          }
        }
        if (myGraphType==LINE_CHART)
        {
          QLinearGradient myGradient;
          if (myBandCountInt==1) //draw single band images with black
          {
            myPainter.setPen( Qt::black );
            myGradient = grayGradient();
          }
          else if (myIteratorInt==1)
          {
            myPainter.setPen( Qt::red );
            myGradient = redGradient();
          }
          else if (myIteratorInt==2)
          {
            myPainter.setPen( Qt::green );
            myGradient = greenGradient();
          }
          else if (myIteratorInt==3)
          {
            myPainter.setPen( Qt::blue );
            myGradient = blueGradient();
          }
          else if (myIteratorInt==4)
          {
            myPainter.setPen( Qt::magenta );
            myGradient = grayGradient();
          }
          else if (myIteratorInt==5)
          {
            myPainter.setPen( Qt::darkRed );
            myGradient = grayGradient();
          }
          else if (myIteratorInt==6)
          {
            myPainter.setPen( Qt::darkGreen );
            myGradient = grayGradient();
          }
          else if (myIteratorInt==7)
          {
            myPainter.setPen( Qt::darkBlue );
            myGradient = grayGradient();
          }
          else
          {
            myPainter.setPen( Qt::gray );
            myGradient = grayGradient();
          }
          //close of the point array so it makes a nice polygon
          //bottom right point
          myPolygon << QPointF( 
              myImageWidth, 
              myImageHeight-myXGutterHeight);
          //bottom left point
          myPolygon << QPointF( 
              myYGutterWidth, 
              myImageHeight-myXGutterHeight);
          myPainter.setPen( Qt::black );
          myPainter.setBrush(myGradient);
          QPainterPath myPath;
          myPath.addPolygon(myPolygon);
          myPainter.drawPath(myPath);
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
    QPolygon myPolygon;
    myPolygon << QPoint((i*myXDivisions)+myYGutterWidth , myImageHeight-myXGutterHeight);
    myPolygon << QPoint((i*myXDivisions)+myYGutterWidth , myImageHeight-(myXGutterHeight-5));
    myPolygon << QPoint((i*myXDivisions)+myYGutterWidth , myImageHeight-myXGutterHeight);
    myPolygon << QPoint(((i+1)*myXDivisions)+myYGutterWidth , myImageHeight-myXGutterHeight);
    myPainter.drawPolyline(myPolygon);
  }
  //
  // Now draw interval markers on the y axis
  //
  int myYDivisions = myGraphImageHeight/10;
  myPainter.setPen( Qt::gray );
  for (int i=myYDivisions;i>0;--i)
  {

    QPolygon myPolygon;
    int myYOrigin = myImageHeight-myXGutterHeight;
    myPolygon << QPoint(myYGutterWidth,myYOrigin-(i*myYDivisions ));
    myPolygon << QPoint(myYGutterWidth-5,myYOrigin-(i*myYDivisions ));
    myPolygon << QPoint(myYGutterWidth,myYOrigin-(i*myYDivisions ));
    myPolygon << QPoint(myYGutterWidth,myYOrigin-((i-1)*myYDivisions ));
    myPainter.drawPolyline(myPolygon);
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

void QgsRasterLayerProperties::on_pbnImportTransparentPixelValues_clicked()
{
  int myLineCounter = 0;
  bool myImportError = false;
  QString myBadLines;
  QString myFilename = QFileDialog::getOpenFileName(this, tr("Open file"), "/", tr("Textfile (*.txt)"));
  QFile myInputFile(myFilename);
  if (myInputFile.open(QFile::ReadOnly))
  {
    QTextStream myInputStream(&myInputFile);
    QString myInputLine;
    if(rbtnThreeBand->isChecked() && QgsRasterLayer::MULTI_BAND_COLOR == mRasterLayer->getDrawingStyle())
    {
      for(int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >= 0; myTableRunner--)
      {
        tableTransparency->removeRow(myTableRunner);
      }

      while(!myInputStream.atEnd())
      {
        myLineCounter++;
        myInputLine = myInputStream.readLine();
        if(!myInputLine.isEmpty())
        {
          if(!myInputLine.simplified().startsWith("#"))
          {
            QStringList myTokens = myInputLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if(myTokens.count() != 4)
            {
              myImportError = true;
              myBadLines = myBadLines + QString::number(myLineCounter) + ":\t[" + myInputLine + "]\n";
            }
            else
            {
              tableTransparency->insertRow(tableTransparency->rowCount());
              tableTransparency->setItem(tableTransparency->rowCount() - 1, 0, new QTableWidgetItem(myTokens[0]));
              tableTransparency->setItem(tableTransparency->rowCount() - 1, 1, new QTableWidgetItem(myTokens[1]));
              tableTransparency->setItem(tableTransparency->rowCount() - 1, 2, new QTableWidgetItem(myTokens[2]));
              tableTransparency->setItem(tableTransparency->rowCount() - 1, 3, new QTableWidgetItem(myTokens[3]));
            }
          }
        }
      }
    }
    else
    {
      for(int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >= 0; myTableRunner--)
      {
        tableTransparency->removeRow(myTableRunner);
      }

      while(!myInputStream.atEnd())
      {
        myLineCounter++;
        myInputLine = myInputStream.readLine();
        if(!myInputLine.isEmpty())
        {
          if(!myInputLine.simplified().startsWith("#"))
          {
            QStringList myTokens = myInputLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if(myTokens.count() != 2)
            {
              myImportError = true;
              myBadLines = myBadLines + QString::number(myLineCounter) + ":\t[" + myInputLine + "]\n";
            }
            else
            {
              tableTransparency->insertRow(tableTransparency->rowCount());
              tableTransparency->setItem(tableTransparency->rowCount() - 1, 0, new QTableWidgetItem(myTokens[0]));
              tableTransparency->setItem(tableTransparency->rowCount() - 1, 1, new QTableWidgetItem(myTokens[1]));
            }
          }
        }
      }
    }

    if(myImportError)
    {
      QMessageBox::warning(this, tr("Import Error"), tr("The following lines contained errors\n\n") + myBadLines );
    }
  }
  else if(!myFilename.isEmpty())
  {
    QMessageBox::warning(this, tr("Read access denied"), tr("Read access denied. Adjust the file permissions and try again.\n\n") );
  }
}

void QgsRasterLayerProperties::on_pbnRemoveSelectedRow_clicked() 
{
  if(0 < tableTransparency->rowCount())
  { 
    tableTransparency->removeRow(tableTransparency->currentRow());
  }
}

void QgsRasterLayerProperties::on_rbtnSingleBand_toggled(bool theState)
{
  if(theState)
  {
    //--- enable and disable appropriate controls
    rbtnThreeBand->setChecked(false); 
    cboxColorMap->setEnabled(true);

    if(cboxColorMap->currentText() == tr("Pseudocolor"))
    {
      tabBar->setTabEnabled(tabBar->indexOf(tabPageColormap), true);
    }
    
    if(cboxColorMap->currentText() == tr("Pseudocolor") ||cboxColorMap->currentText() == tr("Color Ramp") || cboxColorMap->currentText() == tr("Freak Out") || mRasterLayer->getRasterLayerType() == QgsRasterLayer::PALETTE)
    {
      pbtnLoadMinMax->setEnabled(false);
      labelContrastEnhancement->setEnabled(false);
      cboxContrastEnhancementAlgorithm->setEnabled(false);
    }
    else
    {
      pbtnLoadMinMax->setEnabled(true);
      labelContrastEnhancement->setEnabled(true);
      cboxContrastEnhancementAlgorithm->setEnabled(true);
    }

    grpRgbBands->setEnabled(false);
    grpRgbScaling->setEnabled(false);
    grpGrayBand->setEnabled(true);
    grpGrayScaling->setEnabled(true);

    if(mRasterLayer->getUserDefinedGrayMinMax())
    {
      sboxSingleBandStdDev->setValue(0.0);
      rbtnSingleBandMinMax->setChecked(true);
      leGrayMin->setText(QString::number(mRasterLayer->getMinimumValue(cboGray->currentText())));
      leGrayMax->setText(QString::number(mRasterLayer->getMaximumValue(cboGray->currentText())));
    }
    else
    {
      sboxSingleBandStdDev->setValue(mRasterLayer->getStdDevsToPlot());
      rbtnSingleBandStdDev->setChecked(true);
    }

    // Populate transparency table with single value transparency pixels
    populateTransparencyTable();
    // If no band is selected but there are multiple bands, selcet the first as the default
    if(cboGray->currentText() == tr(QSTRING_NOT_SET) && 1 < cboGray->count())
    {
      cboGray->setCurrentIndex(0);
    }
  }
  else if(!rbtnThreeBand->isEnabled())
  {
    rbtnSingleBand->setChecked(true);
  }
  else
  {
    rbtnThreeBand->setChecked(true);
  }
}

void QgsRasterLayerProperties::on_rbtnSingleBandMinMax_toggled(bool theState)
{
  lblGrayMin->setEnabled(theState);
  leGrayMin->setEnabled(theState);
  lblGrayMax->setEnabled(theState);
  leGrayMax->setEnabled(theState);
}

void QgsRasterLayerProperties::on_rbtnSingleBandStdDev_toggled(bool theState)
{
  sboxSingleBandStdDev->setEnabled(theState);
}

void QgsRasterLayerProperties::on_rbtnThreeBand_toggled(bool theState)
{
  if(theState)
  {
    //--- enable and disable appropriate controls
    rbtnSingleBand->setChecked(false);
    cboxColorMap->setEnabled(false);
    tabBar->setTabEnabled(tabBar->indexOf(tabPageColormap), false);

    grpRgbBands->setEnabled(true);
    grpRgbScaling->setEnabled(true);
    grpGrayBand->setEnabled(false);
    grpGrayScaling->setEnabled(false);
    
    pbtnLoadMinMax->setEnabled(true);
    labelContrastEnhancement->setEnabled(true);
    cboxContrastEnhancementAlgorithm->setEnabled(true);

    /*
     *This may seem strange at first, but the single bands need to be include here for switching 
     *from gray back to color with a multi-band palleted image
     */
    if(QgsRasterLayer::PALETTED_COLOR == mRasterLayer->getDrawingStyle() ||
       QgsRasterLayer::PALETTED_SINGLE_BAND_GRAY == mRasterLayer->getDrawingStyle() ||
       QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR == mRasterLayer->getDrawingStyle() ||
       QgsRasterLayer::PALETTED_MULTI_BAND_COLOR == mRasterLayer->getDrawingStyle() ||
       mRasterLayer->getRasterLayerType() == QgsRasterLayer::PALETTE)
    {
      pbtnLoadMinMax->setEnabled(false);
      cboxContrastEnhancementAlgorithm->setCurrentText(tr("No Stretch"));
      cboxContrastEnhancementAlgorithm->setEnabled(false);
      labelContrastEnhancement->setEnabled(false);
      sboxThreeBandStdDev->setEnabled(false);
    }

    if(mRasterLayer->getUserDefinedRGBMinMax())
    {
      sboxThreeBandStdDev->setValue(0.0);
      rbtnThreeBandMinMax->setChecked(true);
      leRedMin->setText(QString::number(mRasterLayer->getMinimumValue(cboRed->currentText())));
      leRedMax->setText(QString::number(mRasterLayer->getMaximumValue(cboRed->currentText())));
      leGreenMin->setText(QString::number(mRasterLayer->getMinimumValue(cboGreen->currentText())));
      leGreenMax->setText(QString::number(mRasterLayer->getMaximumValue(cboGreen->currentText())));
      leBlueMin->setText(QString::number(mRasterLayer->getMinimumValue(cboBlue->currentText())));
      leBlueMax->setText(QString::number(mRasterLayer->getMaximumValue(cboBlue->currentText())));
    }
    else
    {
      sboxThreeBandStdDev->setValue(mRasterLayer->getStdDevsToPlot());
      rbtnThreeBandStdDev->setChecked(true);
    }

    // Populate transparency table with single value transparency pixels
    populateTransparencyTable();
  }
  else if(!rbtnSingleBand->isEnabled())
  {
    rbtnThreeBand->setChecked(true);
  }
  else
  {
    rbtnSingleBand->setChecked(true);
  }
}

void QgsRasterLayerProperties::on_rbtnThreeBandMinMax_toggled(bool theState)
{
  lblRedMin->setEnabled(theState);
  leRedMin->setEnabled(theState);
  lblRedMax->setEnabled(theState);
  leRedMax->setEnabled(theState);
  lblGreenMin->setEnabled(theState);
  leGreenMin->setEnabled(theState);
  lblGreenMax->setEnabled(theState);
  leGreenMax->setEnabled(theState);
  lblBlueMin->setEnabled(theState);
  leBlueMin->setEnabled(theState);
  lblBlueMax->setEnabled(theState);
  leBlueMax->setEnabled(theState);
}

void QgsRasterLayerProperties::on_rbtnThreeBandStdDev_toggled(bool theState)
{
  sboxThreeBandStdDev->setEnabled(theState);
}

void QgsRasterLayerProperties::sboxSingleBandStdDev_valueChanged(double theValue) 
{
  if(!ignoreSpinBoxEvent)
  {
    leGrayMin->setText("");
    leGrayMax->setText("");
  }
  else 
    ignoreSpinBoxEvent = false;
}

void QgsRasterLayerProperties::sboxThreeBandStdDev_valueChanged(double theValue) 
{
  if(!ignoreSpinBoxEvent)
  {
    leRedMin->setText("");
    leRedMax->setText("");
    leGreenMin->setText("");
    leGreenMax->setText("");
    leBlueMin->setText("");
    leBlueMax->setText("");
  }
  else
    ignoreSpinBoxEvent = false;
}

void QgsRasterLayerProperties::sliderTransparency_valueChanged(int theValue)
{
  //set the transparency percentage label to a suitable value
  int myInt = static_cast < int >((theValue / 255.0) * 100);  //255.0 to prevent integer division
  lblTransparencyPercent->setText(QString::number(myInt) + "%");
}//sliderTransparency_valueChanged

void QgsRasterLayerProperties::userDefinedMinMax_textEdited(QString theString)
{
  /*
   * If all min max values are set and valid, then reset stdDev to 0.0
   */
  if(rbtnThreeBand->isChecked())
  {
    if(validUserDefinedMinMax() && sboxThreeBandStdDev->value() != 0.0)
    {
      ignoreSpinBoxEvent = true;
      sboxThreeBandStdDev->setValue(0.0);
    }
  }
  else
  {
    if(validUserDefinedMinMax() && sboxSingleBandStdDev->value() != 0.0)
    {
      ignoreSpinBoxEvent = true;
      sboxSingleBandStdDev->setValue(0.0);
    }
  }
}

void QgsRasterLayerProperties::on_mClassifyButton_clicked()
{
  QgsRasterBandStats myRasterBandStats = mRasterLayer->getRasterBandStats(1);
  int numberOfEntries = sboxNumberOfEntries->value();

  std::list<double> entryValues;
  std::list<QColor> entryColors;

  if(cboxClassificationMode->currentText() == tr("Equal interval"))
    {
      double currentValue = myRasterBandStats.minVal;
      double intervalDiff;
      if(numberOfEntries > 1)
	{
	  //because the highest value is also an entry, there are (numberOfEntries - 1)
	  //intervals
	  intervalDiff = (myRasterBandStats.maxVal - myRasterBandStats.minVal) / \
	    (numberOfEntries - 1);
	}
      else
	{
	  intervalDiff = myRasterBandStats.maxVal - myRasterBandStats.minVal;
	}
      
      for(int i = 0; i < numberOfEntries; ++i)
	{
	  entryValues.push_back(currentValue);
	  currentValue += intervalDiff;
	}
    }
  else if(cboxClassificationMode->currentText() == tr("Quantiles"))
    {
      //todo
    }

  //hard code color range from blue -> red for now. Allow choice of ramps in future
  int colorDiff = 0;
  if(numberOfEntries != 0)
    {
      colorDiff = (int)(255/numberOfEntries);
    }
  
  for(int i = 0; i < numberOfEntries; ++i)
    {
      QColor currentColor;
      currentColor.setRgb(colorDiff*i ,0, 255 - colorDiff * i);
      entryColors.push_back(currentColor);
    }
  
  mColormapTreeWidget->clear();

  std::list<double>::const_iterator value_it = entryValues.begin();
  std::list<QColor>::const_iterator color_it = entryColors.begin();

  for(; value_it != entryValues.end(); ++value_it, ++color_it)
    {
      QTreeWidgetItem* newItem = new QTreeWidgetItem(mColormapTreeWidget);
      newItem->setText(0, QString::number(*value_it, 'f'));
      newItem->setBackground(1, QBrush(*color_it));
      newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    }
}

void QgsRasterLayerProperties::on_mDeleteEntryButton_clicked()
{
  QTreeWidgetItem* currentItem = mColormapTreeWidget->currentItem();
  if(currentItem)
    {
      delete currentItem;
    }
}

void QgsRasterLayerProperties::handleColormapTreeWidgetDoubleClick(QTreeWidgetItem* item, int column)
{
  if(item)
  {
    if(column == 1)
	  {
	    //show color dialog
	    QColor newColor = QColorDialog::getColor();
	    if(newColor.isValid())
	    {
	      item->setBackground(1, QBrush(newColor));
	    }
    }
    else
    {
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    } 
	}
}

void QgsRasterLayerProperties::on_pbtnLoadMinMax_clicked()
{
  if(mRasterLayerIsGdal && (mRasterLayer->getDrawingStyle() == QgsRasterLayer::SINGLE_BAND_GRAY || mRasterLayer->getDrawingStyle() == QgsRasterLayer::MULTI_BAND_SINGLE_BAND_GRAY || mRasterLayer->getDrawingStyle() == QgsRasterLayer::MULTI_BAND_COLOR))
  {
    QgsRasterBandStats myRasterBandStats;
    if(rbtnThreeBand->isChecked())
    {
      rbtnThreeBandMinMax->setChecked(true);
      
      if(rbtnActualMinMax->isChecked())
      {
        myRasterBandStats = mRasterLayer->getRasterBandStats(mRasterLayer->getRasterBandNumber(cboRed->currentText()));
        leRedMin->setText(QString::number(myRasterBandStats.minVal));
        leRedMax->setText(QString::number(myRasterBandStats.maxVal));
        myRasterBandStats = mRasterLayer->getRasterBandStats(mRasterLayer->getRasterBandNumber(cboGreen->currentText()));
        leGreenMin->setText(QString::number(myRasterBandStats.minVal));
        leGreenMax->setText(QString::number(myRasterBandStats.maxVal));
        myRasterBandStats = mRasterLayer->getRasterBandStats(mRasterLayer->getRasterBandNumber(cboBlue->currentText()));
        leBlueMin->setText(QString::number(myRasterBandStats.minVal));
        leBlueMax->setText(QString::number(myRasterBandStats.maxVal));
      }
      else
      {
        rbtnEstimateMinMax->setChecked(true);
        double myMinimumMaximum[2];
        mRasterLayer->computeMinimumMaximumEstimates(mRasterLayer->getRasterBandNumber(cboRed->currentText()), myMinimumMaximum);
        leRedMin->setText(QString::number(myMinimumMaximum[0]));
        leRedMax->setText(QString::number(myMinimumMaximum[1]));
        mRasterLayer->computeMinimumMaximumEstimates(mRasterLayer->getRasterBandNumber(cboGreen->currentText()), myMinimumMaximum);
        leGreenMin->setText(QString::number(myMinimumMaximum[0]));
        leGreenMax->setText(QString::number(myMinimumMaximum[1]));
        mRasterLayer->computeMinimumMaximumEstimates(mRasterLayer->getRasterBandNumber(cboBlue->currentText()), myMinimumMaximum);
        leBlueMin->setText(QString::number(myMinimumMaximum[0]));
        leBlueMax->setText(QString::number(myMinimumMaximum[1]));
      }

    }
    else
    {
      rbtnSingleBandMinMax->setChecked(true);
      if(rbtnActualMinMax->isChecked())
      {
        myRasterBandStats = mRasterLayer->getRasterBandStats(mRasterLayer->getRasterBandNumber(cboGray->currentText()));
        leGrayMin->setText(QString::number(myRasterBandStats.minVal));
        leGrayMax->setText(QString::number(myRasterBandStats.maxVal));
      }
      else
      {
        rbtnEstimateMinMax->setChecked(true);
        double myMinimumMaximum[2];
        mRasterLayer->computeMinimumMaximumEstimates(mRasterLayer->getRasterBandNumber(cboGray->currentText()), myMinimumMaximum);
        leGrayMin->setText(QString::number(myMinimumMaximum[0]));
        leGrayMax->setText(QString::number(myMinimumMaximum[1]));
      }
    }
  }
}

void QgsRasterLayerProperties::on_pbtnMakeContrastEnhancementAlgorithmDefault_clicked()
{
  //Like some of the other functionality in the raster properties GUI this deviated a little from the 
  //best practice of GUI design as this pressing cancel will not undo setting the default 
  //contrast enhancement algorithm
  if(cboxContrastEnhancementAlgorithm->currentText() != tr("User Defined"))
  {
    QSettings myQSettings;
    if(cboxContrastEnhancementAlgorithm->currentText() == tr("No Stretch"))
    {
      myQSettings.setValue("/Raster/defaultContrastEnhancementAlgorithm", "NO_STRETCH");
      labelDefaultContrastEnhancementAlgorithm->setText(cboxContrastEnhancementAlgorithm->currentText());
    }
    else if(cboxContrastEnhancementAlgorithm->currentText() == tr("Stretch To MinMax"))
    {
      myQSettings.setValue("/Raster/defaultContrastEnhancementAlgorithm", "STRETCH_TO_MINMAX");
      labelDefaultContrastEnhancementAlgorithm->setText(cboxContrastEnhancementAlgorithm->currentText());
    }
    else if(cboxContrastEnhancementAlgorithm->currentText() == tr("Stretch And Clip To MinMax"))
    {
      myQSettings.setValue("/Raster/defaultContrastEnhancementAlgorithm", "STRETCH_AND_CLIP_TO_MINMAX");
      labelDefaultContrastEnhancementAlgorithm->setText(cboxContrastEnhancementAlgorithm->currentText());
    }
    else if(cboxContrastEnhancementAlgorithm->currentText() == tr("Clip To MinMax"))
    {
      myQSettings.setValue("/Raster/defaultContrastEnhancementAlgorithm", "CLIP_TO_MINMAX");
      labelDefaultContrastEnhancementAlgorithm->setText(cboxContrastEnhancementAlgorithm->currentText());
    }
    else
    {
      //do nothing
    }
  }
}

QLinearGradient QgsRasterLayerProperties::redGradient()
{
  //define a gradient
  ///@TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient(mGradientWidth,0,mGradientWidth,mGradientHeight);
  myGradient.setColorAt(0.0,QColor(242, 14, 25, 190));
  myGradient.setColorAt(0.5,QColor(175, 29, 37, 190));
  myGradient.setColorAt(1.0,QColor(114, 17, 22, 190));
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::greenGradient()
{
  //define a gradient 
  ///@TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient(mGradientWidth,0,mGradientWidth,mGradientHeight);
  myGradient.setColorAt(0.0,QColor(48, 168, 5, 190));
  myGradient.setColorAt(0.8,QColor(36, 122, 4, 190));
  myGradient.setColorAt(1.0,QColor(21, 71, 2, 190));
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::blueGradient()
{
  //define a gradient 
  ///@TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient(mGradientWidth,0,mGradientWidth,mGradientHeight);
  myGradient.setColorAt(0.0,QColor(30, 0, 106, 190));
  myGradient.setColorAt(0.2,QColor(30, 72, 128, 190));
  myGradient.setColorAt(1.0,QColor(30, 223, 196, 190));
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::grayGradient()
{
  //define a gradient 
  ///@TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient(mGradientWidth,0,mGradientWidth,mGradientHeight);
  myGradient.setColorAt(0.0,QColor(5, 5, 5, 190));
  myGradient.setColorAt(0.8,QColor(122, 122, 122, 190));
  myGradient.setColorAt(1.0,QColor(220, 220, 220, 190));
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::highlightGradient()
{
  //define another gradient for the highlight
  ///@TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient(mGradientWidth,0,mGradientWidth,mGradientHeight);
  myGradient.setColorAt(1.0,QColor(255, 255, 255, 50));
  myGradient.setColorAt(0.5,QColor(255, 255, 255, 100));
  myGradient.setColorAt(0.0,QColor(255, 255, 255, 150));
  return myGradient;
}
