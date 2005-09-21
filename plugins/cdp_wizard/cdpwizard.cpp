/***************************************************************************
  cdpwizard.cpp  -  description
  -------------------
begin                : Wed May 14 2003
copyright            : (C) 2003 by Tim Sutton
email                : t.sutton@reading.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cdpwizard.h"
#include "imagewriter.h"
#include <meridianswitcher.h>

#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <string.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qsettings.h>
#include <qapplication.h>
#include <qdatetime.h>
#include <qfileinfo.h> 
#include <qdir.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qcheckbox.h>
#include <qcursor.h>

CDPWizard::CDPWizard( QWidget* parent , const char* name , bool modal , WFlags fl  )
        : CDPWizardBase( parent, name, modal, fl )
{
    initialise();
}

CDPWizard::CDPWizard()
{
    initialise();
}

CDPWizard::~CDPWizard()
{
    delete climateDataProcessor;
}

bool CDPWizard::initialise()
{
  QSettings myQSettings;
  //
  // Populate the file types combo
  //
  //cboFileType->insertItem(tr("GDAL Supported Raster"));
  //
  // set up the lstVariablesToCalc combo box
  //
#ifdef QGISDEBUG

  std::cout << "Adding items into the lstVariablesToCalc list box" << std::endl;
#endif

  lstVariablesToCalc->insertItem(tr("Annual mean diurnal temperature range"));
  lstVariablesToCalc->insertItem(tr("Annual mean number of frost days"));
  lstVariablesToCalc->insertItem(tr("Annual mean total incident solar radiation"));
  lstVariablesToCalc->insertItem(tr("Annual temperature range"));
  lstVariablesToCalc->insertItem(tr("Highest temperature in warmest month"));
  lstVariablesToCalc->insertItem(tr("Lowest temperature in coolest month"));
  lstVariablesToCalc->insertItem(tr("Mean daily precipitation"));
  lstVariablesToCalc->insertItem(tr("Mean daily precipitation in coolest month"));
  lstVariablesToCalc->insertItem(tr("Mean daily precipitation in coolest quarter"));
  lstVariablesToCalc->insertItem(tr("Mean daily precipitation in driest month"));
  lstVariablesToCalc->insertItem(tr("Mean daily precipitation in driest quarter"));
  lstVariablesToCalc->insertItem(tr("Mean daily precipitation in warmest month"));
  lstVariablesToCalc->insertItem(tr("Mean daily precipitation in warmest quarter"));
  lstVariablesToCalc->insertItem(tr("Mean daily precipitation in wettest month"));
  lstVariablesToCalc->insertItem(tr("Mean daily precipitation in wettest quarter"));
  lstVariablesToCalc->insertItem(tr("Mean diurnal temperature range in coolest month"));
  lstVariablesToCalc->insertItem(tr("Mean diurnal temperature range in warmest month"));
  lstVariablesToCalc->insertItem(tr("Mean precipitation in frost free months"));
  lstVariablesToCalc->insertItem(tr("Mean temperature"));
  lstVariablesToCalc->insertItem(tr("Mean temperature in coolest month"));
  lstVariablesToCalc->insertItem(tr("Mean temperature in coolest quarter"));
  lstVariablesToCalc->insertItem(tr("Mean temperature in frost free months"));
  lstVariablesToCalc->insertItem(tr("Mean temperature in warmest month"));
  lstVariablesToCalc->insertItem(tr("Mean temperature in warmest quarter"));
  lstVariablesToCalc->insertItem(tr("Mean wind speed"));
  lstVariablesToCalc->insertItem(tr("Number of months with minimum temperature above freezing"));
  lstVariablesToCalc->insertItem(tr("Radiation in coolest month"));
  lstVariablesToCalc->insertItem(tr("Radiation in coolest quarter"));
  lstVariablesToCalc->insertItem(tr("Radiation in driest month"));
  lstVariablesToCalc->insertItem(tr("Radiation in driest quarter"));
  lstVariablesToCalc->insertItem(tr("Radiation in warmest month"));
  lstVariablesToCalc->insertItem(tr("Radiation in warmest quarter"));
  lstVariablesToCalc->insertItem(tr("Radiation in wettest month"));
  lstVariablesToCalc->insertItem(tr("Radiation in wettest quarter"));
  lstVariablesToCalc->insertItem(tr("Standard deviation of mean precipitation"));
  lstVariablesToCalc->insertItem(tr("Standard deviation of mean temperature"));

  //
  //set up the output formats combo
  //
#ifdef QGISDEBUG

  std::cout << "Adding items into the output filetype combo box" << endl;
#endif

  cboOutputFormat->insertItem(tr("GDAL Tiff Image"));
  cboOutputFormat->insertItem(tr("Arc/Info Ascii Grid"));
  cboOutputFormat->insertItem(tr("Plain matrix with no header"));
  cboOutputFormat->insertItem(tr("Matlab"));
  //
  // Create a climate data processor instance - this is the thing that does all
  // the hard work!
  //
  climateDataProcessor = new ClimateDataProcessor();

  //hook uo signals and slots
  connect(climateDataProcessor,SIGNAL(variableStart(QString )),
          this,SLOT(variableStart(QString )));
  connect(climateDataProcessor,SIGNAL(variableDone(QString)),
          this,SLOT(variableDone(QString)));

  //Load default settings
  loadDefaults();

  //presume all went ok
  return true;
}


void CDPWizard::saveDefaults()
{
    QSettings myQSettings;

    //Page 2
    myQSettings.writeEntry("/qgis/cdpwizard/meanTemp",leMeanTemp->text());
    myQSettings.writeEntry("/qgis/cdpwizard/minTemp",leMinTemp->text());
    myQSettings.writeEntry("/qgis/cdpwizard/maxTemp",leMaxTemp->text());
    myQSettings.writeEntry("/qgis/cdpwizard/diurnalTemp",leDiurnalTemp->text());
    myQSettings.writeEntry("/qgis/cdpwizard/meanPrecip",leMeanPrecipitation->text());
    myQSettings.writeEntry("/qgis/cdpwizard/frostDays",leFrostDays->text());
    myQSettings.writeEntry("/qgis/cdpwizard/totalSolarRadiation",leTotalSolarRadiation->text());


    //Page 4
    myQSettings.writeEntry("/qgis/cdpwizard/outputPath",leOutputPath->text());
    myQSettings.writeEntry("/qgis/cdpwizard/outputFormat",cboOutputFormat->currentItem());

}

void CDPWizard::loadDefaults()
{
    QSettings myQSettings;
    leMeanTemp->setText(myQSettings.readEntry("/qgis/cdpwizard/meanTemp"));
    leMinTemp->setText(myQSettings.readEntry("/qgis/cdpwizard/minTemp"));
    leMaxTemp->setText(myQSettings.readEntry("/qgis/cdpwizard/maxTemp"));
    leDiurnalTemp->setText(myQSettings.readEntry("/qgis/cdpwizard/diurnalTemp"));
    leMeanPrecipitation->setText(myQSettings.readEntry("/qgis/cdpwizard/meanPrecip"));
    leFrostDays->setText(myQSettings.readEntry("/qgis/cdpwizard/frostDays"));
    leTotalSolarRadiation->setText(myQSettings.readEntry("/qgis/cdpwizard/totalSolarRadiation"));


    QString myOutputDir = myQSettings.readEntry("/qgis/cdpwizard/DefaultDirectories/OutputDir",QDir::homeDirPath());
    leOutputPath->setText(myOutputDir);
    cboOutputFormat->setCurrentItem(myQSettings.readNumEntry("/qgis/cdpwizard/outputFormat"));


}



void CDPWizard::formSelected(const QString  &thePageNameQString)
{
    QString myQString;
    QLineEdit *myLineEdit;

    if (thePageNameQString==tr("Raw data file selection"))
    {
#ifdef QGISDEBUG
        std::cout << "Opening Raw Data File Selection page" << std::endl;
#endif

        checkInputFilenames();
    }

    if (thePageNameQString==tr("File type and variables")) //we do this after leaving the file selection page
    {

        //#ifdef QGISDEBUG
        std::cout << "Leaving file selection page" << std::endl;
        //#endif
        /** @todo Add some checking to make sure each file exists and is readable and valid for its type */
        climateDataProcessor->setMeanTempFileName(leMeanTemp->text());
        climateDataProcessor->setMinTempFileName(leMinTemp->text());
        climateDataProcessor->setMaxTempFileName(leMaxTemp->text());
        climateDataProcessor->setDiurnalTempFileName(leDiurnalTemp->text());
        climateDataProcessor->setMeanPrecipFileName(leMeanPrecipitation->text());
        climateDataProcessor->setFrostDaysFileName(leFrostDays->text());
        climateDataProcessor->setTotalSolarRadFileName(leTotalSolarRadiation->text());

        //turn off next button until one or more variables have been selected
        setNextEnabled(currentPage(), false);


        /////////////////////////////////////////////////////////////////////
        //
        // OK now we need to update the list of available calculations that
        //                       can be carried out.
        //
        /////////////////////////////////////////////////////////////////////
        //
        // get the input file type
        //
        climateDataProcessor->setInputFileType("GDAL Supported Raster");
        //Should not need to have the next line here - it slows everythinf down!
        //climateDataProcessor->makeFileGroups();
        //#ifdef QGISDEBUG

        std::cout << "Getting available calculations list" << std::endl;
        //#endif

        climateDataProcessor->makeAvailableCalculationsMap();
        // and then update the list box

        //List the calculations in  availableCalculationsMap  using an iterator
        QMap<QString, bool> myAvailableCalculationsMap = climateDataProcessor->getAvailableCalculationsMap();
        QMap<QString, bool>::const_iterator myIter;
        //#ifdef QGISDEBUG

        std::cout << myAvailableCalculationsMap.size() << " available calculations in list which are:" << std::endl;
        std::cout << climateDataProcessor->getDescription() << std::endl;
        //#endif
        //clear the current entries from the box
        lstVariablesToCalc->clear();
        for (myIter=myAvailableCalculationsMap.begin(); myIter != myAvailableCalculationsMap.end(); myIter++)
        {
            if (myIter.data())
            {
                //#ifdef QGISDEBUG
                std::cout << myIter.key() << QString(": true\n");
                //#endif

                //need to add some logic here to select the inserted item
                lstVariablesToCalc->insertItem(myIter.key());
            }
            else
            {
#ifdef QGISDEBUG
                std::cout << myIter->first <<  QString(": false\n");
#endif

                //need to add some logic here to select the inserted item
                lstVariablesToCalc->insertItem(myIter.key());
            }
        }
    } //end of test for page 3

    if (thePageNameQString==tr("Output destination")) //we do this after leaving the file type and variables page
    {

    }



    if (thePageNameQString==tr("Summary of processing to be performed")) //we do this when  we arrive at the summary of variables to be calculated
    {
        //update the summary of vars to calculate
        txtVariableSummary->clear();
        for ( unsigned int i = 0; i < lstVariablesToCalc->count(); i++ )
        {
            QListBoxItem *item = lstVariablesToCalc->item( i );
            // if the item is selected...
            if ( item->isSelected() )
            {
                // increment the count of selected items
                txtVariableSummary->append(item->text());
            }
        }

        //update the output file format summary box
        leInputFormatSummary->setText("GDAL Supported Raster");

        //update the file output path box
        leOutputPathSummary->setText(leOutputPath->text());

        txtInputFileSummary->clear();
        txtInputFileSummary->append(leMeanTemp->text());
        txtInputFileSummary->append(leMinTemp->text());
        txtInputFileSummary->append(leMaxTemp->text());
        txtInputFileSummary->append(leDiurnalTemp->text());
        txtInputFileSummary->append(leMeanPrecipitation->text());
        txtInputFileSummary->append(leMeanPrecipitation->text());
        txtInputFileSummary->append(leFrostDays->text());
        txtInputFileSummary->append(leTotalSolarRadiation->text());
    } //end of test for page 5

    if (thePageNameQString==tr("Progress")) //we do this when we start the calculation
    {
        qApp->processEvents();
        run();
    }
} //end of formSelected

void CDPWizard::run()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  qApp->processEvents();
  //mark the start time:
  startTime.start();
  QString myLabelString;
  myLabelString.sprintf("<p align=\"right\">Time elapsed: %d s</p>", startTime.elapsed()/1000);
  lblElapsedTime->setText(myLabelString);

  int myFirstYearInFileInt, myJobStartYearInt, myJobEndYearInt;
  QString myInputFileTypeString, myOutputFileTypeString, myOutputPathString;
  QString myQString;

  // get the ouput file path
  climateDataProcessor->setOutputFilePathString(leOutputPath->text()+QDir::separator ());

  // get the input file type
  climateDataProcessor->setInputFileType("GDAL Supported Raster");

  // get the ouput file type
  climateDataProcessor->setOutputFileType(cboOutputFormat->currentText());


  //! @todo Get rid of the whole files in series paradigm
  climateDataProcessor->setFilesInSeriesFlag(true);


  //setup the climate data processor's filereaders
  /** @todo see what this hardcoding means and remove if possible */
  if (!climateDataProcessor->makeFileGroups ())
  {
    std::cerr << "cdpwizards call to make file groups failed!" << std::endl;
    return;
  }
  //add each selected user calculation to the user calculation map
  // Go through all items of the first ListBox
  unsigned int myVarsCount=0;
  for ( unsigned int i = 0; i < lstVariablesToCalc->count(); i++ )
  {
    QListBoxItem *myQListBoxItem = lstVariablesToCalc->item( i );
    // if the item is selected...
    if ( myQListBoxItem->isSelected() )
    {
      climateDataProcessor->addUserCalculation(myQListBoxItem->text().latin1() );
      myVarsCount++;
    }
  }

  progressTotalJob->setProgress(0,myVarsCount);
  //get a summary of the climate dataprocessor class now
#ifdef QGISDEBUG
  std::cout << climateDataProcessor->getDescription() << endl;
#endif
  //
  //now we let the climatedataprocessor run!
  //
  climateDataProcessor->run();
  QApplication::restoreOverrideCursor();
  setFinishEnabled( step_6, TRUE );
} //end of run()

//
// Next bunch of methods oare slots for signals
// emitted by climatedataprocessor
//

void CDPWizard::variableStart(QString theNameQString)
{
  lblCurrentTask->setText("<p align=\"right\">" + theNameQString + "</p>");
}
void CDPWizard::variableDone(QString theFileNameString)
{
  std::cout << " ---------------- Variable " << theFileNameString << " written! " << std::endl;
  QFileInfo myQFileInfo(theFileNameString);
  //convert the completed variable layer to an image file
  ImageWriter myImageWriter;
  if (cbxPseudoColour->isChecked())
  {
    QString myImageFileNameString = myQFileInfo.dirPath()+"/"+myQFileInfo.baseName()+".png";
    myImageWriter.writeImage(theFileNameString,myImageFileNameString);
    //set the image label on the calculating variables screen to show the last
    //variable calculated
    QPixmap myPixmap(myImageFileNameString);
    pixmapLabel2->setScaledContents(true);
    pixmapLabel2->setPixmap(myPixmap);
  }
  if (cbxMeridianSwitch->isChecked())
  {
    //spit the filename up so we can rename it for the meridian shift
    QString myMSFileNameString = myQFileInfo.dirPath()+"/"+myQFileInfo.baseName()+"MS."+myQFileInfo.extension();
    //perform the meridian shift (hard coding for now but we should have a class member 
    //boolean that stores whether this is needed
    MeridianSwitcher mySwitcher;
    mySwitcher.doSwitch(theFileNameString,myMSFileNameString);
    if (cbxPseudoColour->isChecked())
    {
      //make an image for the shifted file too
      QFileInfo myQFileInfo2(myMSFileNameString);
      QString myImageFileNameString = myQFileInfo2.dirPath()+"/"+myQFileInfo2.baseName()+".png";
      myImageWriter.writeImage(myMSFileNameString,myImageFileNameString);
      //set the image label on the calculating variables screen to show the last
      //variable calculated
      QPixmap myPixmap(myImageFileNameString);
      pixmapLabel2->setScaledContents(true);
      pixmapLabel2->setPixmap(myPixmap);
    }
  }

  progressTotalJob->setProgress(progressTotalJob->progress()+1);
  //update the elapsed time
  QString myLabelString;
  myLabelString.sprintf("<p align=\"right\">Time elapsed: %d s</p>", startTime.elapsed()/1000);
  lblElapsedTime->setText(myLabelString);
  qApp->processEvents();
}


//
// End of slots linked to climateDataProcessor
//



void CDPWizard::accept()
{

    //LOGIC REQUIRED TO CHECK THE USER IS ON THE LAST PAGE
    //Save default settings
    saveDefaults();
    done(1);

}

void CDPWizard::checkInputFilenames()
{
    if (leMeanTemp->text() =="" && leMinTemp->text() =="" && leMaxTemp->text() ==""  && leDiurnalTemp->text()=="" && leMeanPrecipitation->text()=="" && leFrostDays->text()=="" && leFrostDays->text()=="" && leTotalSolarRadiation->text()=="" )
    {
        setNextEnabled(currentPage(),false);
    }
    else
    {
        setNextEnabled(currentPage(),true);
    }

}

void CDPWizard::promptForFileName(QLineEdit * theLineEdit, QString theShortName, QString theLongName)
{
    QSettings myQSettings;
    QString myFilterList;
    FileReader::getGdalDriverMap(myFilterList);
    QString myWorkDirString = myQSettings.readEntry("/qgis/cdpwizard/DefaultDirectories/" + theShortName + "Dir",QDir::homeDirPath());

    std::cout << "Filter List: " << myFilterList << std::endl;
    QString myFileNameQString;
    QFileDialog myFileDialog (myWorkDirString,myFilterList,0,"Select " + theLongName ,"Select " + theLongName);
    QString myLastFilter = myQSettings.readEntry("/qgis/cdpwizard/DefaultDirectories/" + theShortName + "Filter","");
    if (!myLastFilter.isEmpty())
    {
      myFileDialog.setSelectedFilter(myLastFilter);
    }
    if ( myFileDialog.exec() == QDialog::Accepted )
    {
      myFileNameQString = myFileDialog.selectedFile();
      theLineEdit->setText(myFileNameQString);
      QFileInfo myFileInfo(myFileNameQString);
      myQSettings.writeEntry("/qgis/cdpwizard/DefaultDirectories/" + theShortName + "Dir",myFileInfo.dirPath());
      myQSettings.writeEntry("/qgis/cdpwizard/DefaultDirectories/" + theShortName + "Filter",myFileDialog.selectedFilter());
      checkInputFilenames();
    }
}

void CDPWizard::pbtnMeanTemp_clicked()
{
    QString myShortName = "MeanTemp";
    QString myLongName = tr("Mean Temperature");
    promptForFileName (leMeanTemp,myShortName,myLongName);
}


void CDPWizard::pbtnMinTemp_clicked()
{
    QString myShortName = "MinTemp";
    QString myLongName = tr("Min Temperature");
    promptForFileName (leMinTemp,myShortName,myLongName);
}


void CDPWizard::pbtnMaxTemp_clicked()
{
    QString myShortName = "MaxTemp";
    QString myLongName = tr("Max Temperature");
    promptForFileName (leMaxTemp,myShortName,myLongName);
}


void CDPWizard::pbtnDiurnalTemp_clicked()
{
    QString myShortName = "DiurnalTemp";
    QString myLongName = tr("Diurnal Temperature");
    promptForFileName (leDiurnalTemp,myShortName,myLongName);
}


void CDPWizard::pbtnMeanPrecipitation_clicked()
{
    QString myShortName = "MeanPrecip";
    QString myLongName = tr("Mean Precipitation");
    promptForFileName (leMeanPrecipitation,myShortName,myLongName);
}


void CDPWizard::pbtnFrostDays_clicked()
{
    QString myShortName = "FrostDays";
    QString myLongName = tr("Frost Days");
    promptForFileName (leFrostDays,myShortName,myLongName);
}


void CDPWizard::pbtnTotalSolarRad_clicked()
{
    QString myShortName = "TotalSolarRadiation";
    QString myLongName = tr("TotalSolarRadiation");
    promptForFileName (leTotalSolarRadiation,myShortName,myLongName);
}

void CDPWizard::pbtnOutputPath_clicked()
{
  QSettings myQSettings;
  QString myLastDir = myQSettings.readEntry("/qgis/cdpwizard/DefaultDirectories/OutputDir",QDir::homeDirPath());
  QString myFileNameQString;
  QFileDialog myFileDialog (myLastDir,"",this,"Select Output Directory");
  myFileDialog.setMode(QFileDialog::DirectoryOnly);
  if ( myFileDialog.exec() == QDialog::Accepted )
  {
    myFileNameQString = myFileDialog.selectedFile();
    leOutputPath->setText(myFileNameQString);
    myQSettings.writeEntry("/qgis/cdpwizard/DefaultDirectories/OutputDir",myFileNameQString);
  }
}



void CDPWizard::lstVariablesToCalc_selectionChanged()
{
    QString myQString;
    int selectionSizeInt=0;
    for ( unsigned int i = 0; i < lstVariablesToCalc->count(); i++ )
    {
        QListBoxItem *item = lstVariablesToCalc->item( i );
        // if the item is selected...
        if ( item->isSelected() )
        {
            // increment the count of selected items
            selectionSizeInt++;
        }
    }
    myQString.sprintf("<p align=\"right\">(%i) Variables selected </p>",selectionSizeInt);
    lblVariableCount->setText(myQString);

    if (selectionSizeInt==0)
    {
        setNextEnabled(currentPage(), false);
    }
    else
    {
        setNextEnabled(currentPage(), true);
    }

}




void CDPWizard::leMeanTemp_textChanged( const QString & )
{
    checkInputFilenames();
}


void CDPWizard::leMinTemp_textChanged( const QString & )
{
    checkInputFilenames();
}


void CDPWizard::leMaxTemp_textChanged( const QString & )
{
    checkInputFilenames();
}


void CDPWizard::leDiurnalTemp_textChanged( const QString & )
{
    checkInputFilenames();
}


void CDPWizard::leMeanPrecipitation_textChanged( const QString & )
{
    checkInputFilenames();
}


void CDPWizard::leFrostDays_textChanged( const QString & )
{
    checkInputFilenames();
}


void CDPWizard::leTotalSolarRadiation_textChanged( const QString & )
{
    checkInputFilenames();
}



void CDPWizard::leOutputPath_textChanged( const QString & theOutputPath)
{

    //Check the output path exists and is readable
    QDir dirCheck(theOutputPath);
    if ( (!dirCheck.exists()) & (!dirCheck.isReadable()) )
    {
        setNextEnabled(currentPage(), false);
    }
    else
    {
        setNextEnabled(currentPage(), true);
    }

}
