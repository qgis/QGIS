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


#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <string.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qsettings.h>
#include <qapplication.h>
#include <qdatetime.h>
#include <meridianswitcher.h>


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
    //
    // Populate the file types combo
    //
    cboFileType->insertItem(tr("CRES African climate data"));
    cboFileType->insertItem(tr("ESRI & ASCII raster"));
    cboFileType->insertItem(tr("Hadley Centre HadCM3 SRES Scenario"));
    cboFileType->insertItem(tr("Hadley Centre HadCM3 IS92a Scenario"));
    cboFileType->insertItem(tr("IPCC Observed Climatology"));
    cboFileType->insertItem(tr("University of Reading Palaeoclimate data"));
    cboFileType->insertItem(tr("Max Planck Institute fur Meteorologie (MPIfM) ECHAM4 data"));
    cboFileType->insertItem(tr("CSIRO-Mk2 Model data"));
    cboFileType->insertItem(tr("National Center for Atmospheric Research (NCAR) NCAR-CSM and NCAR-PCM data"));
    cboFileType->insertItem(tr("Geophysical Fluid Dynamics Laboratory (GFDL) R30 Model data"));
    cboFileType->insertItem(tr("Canadian Center for Climate Modelling and Analysis (CCCma) CGCM2 Model data"));
    cboFileType->insertItem(tr("CCSR/NIES AGCM model data and CCSR OGCM model data"));

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

    cboOutputFormat->insertItem(tr("Matlab"));
    cboOutputFormat->insertItem(tr("ESRI ASCII Grid"));
    cboOutputFormat->insertItem(tr("Plain matrix with no header"));

    //
    // Create a climate data processor instance - this is the thing that does all
    // the hard work!
    //
    climateDataProcessor = new ClimateDataProcessor();

    //hook uo signals and slots
    connect(climateDataProcessor,SIGNAL(numberOfCellsToCalc(int)),
                                 this,SLOT(numberOfCellsToCalc(int)));
    connect(climateDataProcessor,SIGNAL(numberOfVariablesToCalc(int)),
                                 this,SLOT(numberOfVariablesToCalc(int)));
    connect(climateDataProcessor,SIGNAL(numberOfYearsToCalc(int)),
                                 this,SLOT(numberOfYearsToCalc(int)));
    connect(climateDataProcessor,SIGNAL(yearStart(QString)),
                                 this,SLOT(yearStart(QString )));
    connect(climateDataProcessor,SIGNAL(yearDone()),
                                 this,SLOT(yearDone()));
    connect(climateDataProcessor,SIGNAL(variableStart(QString )),
                                 this,SLOT(variableStart(QString )));
    connect(climateDataProcessor,SIGNAL(variableDone(QString)),
                                 this,SLOT(variableDone(QString)));
    connect(climateDataProcessor,SIGNAL(cellDone(float)),
                                 this,SLOT(cellDone(float)));

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
    myQSettings.writeEntry("/qgis/cdpwizard/windSpeed",leWindSpeed->text());
    myQSettings.writeEntry("/qgis/cdpwizard/fileType",cboFileType->currentItem());

    //Page 3
    myQSettings.writeEntry("/qgis/cdpwizard/firstYearInFile",spinFirstYearInFile->value());
    myQSettings.writeEntry("/qgis/cdpwizard/firstYearToCalc",spinFirstYearToCalc->value());
    myQSettings.writeEntry("/qgis/cdpwizard/lastYearToCalc",spinLastYearToCalc->value());

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
    leWindSpeed->setText(myQSettings.readEntry("/qgis/cdpwizard/windSpeed"));
    cboFileType->setCurrentItem(myQSettings.readNumEntry("/qgis/cdpwizard/fileType"));

    spinFirstYearInFile->setValue(myQSettings.readNumEntry("/qgis/cdpwizard/firstYearInFile"));
    spinFirstYearToCalc->setValue(myQSettings.readNumEntry("/qgis/cdpwizard/firstYearToCalc"));
    spinLastYearToCalc->setValue(myQSettings.readNumEntry("/qgis/cdpwizard/lastYearToCalc"));

    leOutputPath->setText(myQSettings.readEntry("/qgis/cdpwizard/outputPath"));
    cboOutputFormat->setCurrentItem(myQSettings.readNumEntry("/qgis/cdpwizard/outputFormat"));

    cboFileType_activated(cboFileType->currentText());

}


void CDPWizard::cboFileType_activated( const QString &myQString )
{
#ifdef QGISDEBUG
    std::cout << "cboFileType text changed" << std::endl;
#endif

    if (myQString==tr("CRES African climate data") || myQString==tr("University of Reading Palaeoclimate data") || myQString==tr("ESRI & ASCII raster"))
    {
        //show the user some instructions about how the files must be on disk
        lblFileSeriesNote->show();
        //set the label on the summary form
        lblFileSeriesSummary->setText(tr("<p align=\"right\">And <font color=\"red\"><b>are</b></font> in a file series</p>"));
    }
    else
    {
        //show the user some instructions about how the files must be on disk
        lblFileSeriesNote->hide();
        //set the label on the summary form
        lblFileSeriesSummary->setText(tr("<p align=\"right\">And <font color=\"red\"><b>are not</b></font> in a file series</p>"));
    }

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
        climateDataProcessor->setWindSpeedFileName(leWindSpeed->text());

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
        climateDataProcessor->setInputFileType(cboFileType->currentText().latin1());
        //Should not need to have the next line here - it slows everythinf down!
        //climateDataProcessor->makeFileGroups(0);
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

        //update the start and end date summary boxes
        leStartYearSummary->setText(QString::number(spinFirstYearToCalc->value()));
        leEndYearSummary->setText(QString::number(spinLastYearToCalc->value()));

        //update the output file format summary box
        leInputFormatSummary->setText(cboFileType->currentText());

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
    //mark the start time:
    startTime.start();

    int myFirstYearInFileInt, myJobStartYearInt, myJobEndYearInt;
    QString myInputFileTypeString, myOutputFileTypeString, myOutputPathString;
    QString myQString;

    // get the first year in file value
    climateDataProcessor->setFileStartYearInt(spinFirstYearInFile->value());

    // get the first year in file to be processed in this job
    climateDataProcessor->setJobStartYearInt(spinFirstYearToCalc->value());

    // get the last year in file to be processed in this job
    climateDataProcessor->setJobEndYearInt(spinLastYearToCalc->value());

    // get the ouput file path
    climateDataProcessor->setOutputFilePathString(leOutputPath->text());

    // get the input file type
    climateDataProcessor->setInputFileType(cboFileType->currentText());

    // get the ouput file type
    climateDataProcessor->setOutputFileType(cboOutputFormat->currentText());



    //
    // find out if datafiles are in series (discrete files for each month)
    // We are using the isVisible property for the label, but we cant guarantee
    // that the wizard page on which the label occurs will be visible so we need
    // to use the isVisibleTo(parent) property, with a reference to the wzard page as parent
    //
    QWidget *  myPageWidget = page(1);
#ifdef QGISDEBUG

    std::cout << "\nFile in series label visible? "<<   lblFileSeriesNote->isVisible() << std::endl;
#endif

    if (lblFileSeriesNote->isVisibleTo(myPageWidget))
    {
#ifdef QGISDEBUG
        std::cout << "Setting files in series flag to true" << std::endl;
#endif

        climateDataProcessor->setFilesInSeriesFlag(true);
    }
    else
    {
#ifdef QGISDEBUG
        std::cout << "Setting files in series flag to false" << std::endl;
#endif

        climateDataProcessor->setFilesInSeriesFlag(false);
    }


    //setup the climate data processor's filereaders
    /** @todo see what this hardcoding means and remove if possible */
    if (!climateDataProcessor->makeFileGroups (1))    //hardcoding year 1 for now
    {
        std::cerr << "cdpwizards call to make file groups failed!" << std::endl;
        return;
    }
    //add each selected user calculation to the user calculation map
    // Go through all items of the first ListBox
    for ( unsigned int i = 0; i < lstVariablesToCalc->count(); i++ )
    {
        QListBoxItem *myQListBoxItem = lstVariablesToCalc->item( i );
        // if the item is selected...
        if ( myQListBoxItem->isSelected() )
        {
            climateDataProcessor->addUserCalculation(myQListBoxItem->text().latin1() );
        }
    }

    //get a summary of the climate dataprocessor class now
#ifdef QGISDEBUG
    std::cout << climateDataProcessor->getDescription() << endl;
#endif
    //
    //now we let the climatedataprocessor run!
    //
    climateDataProcessor->run();
    setFinishEnabled( step_6, TRUE );
} //end of run()

//
// Next bunch of methods oare slots for signals
// emitted by climatedataprocessor
//

void CDPWizard::numberOfCellsToCalc(int theNumberInt)
{
   progressCurrentTask->setTotalSteps(theNumberInt);
}
void CDPWizard::numberOfVariablesToCalc(int theNumberInt)
{
  //progressCurrentYear->setTotalSteps(theNumberInt);
  progressCurrentYear->setTotalSteps(progressCurrentTask->totalSteps()* theNumberInt);
}
void CDPWizard::numberOfYearsToCalc(int theNumberInt)
{
  //progressTotalJob->setTotalSteps(theNumberInt);
  progressTotalJob->setTotalSteps(progressCurrentYear->totalSteps()* theNumberInt);
}
void CDPWizard::yearStart(QString theNameQString)
{
  progressCurrentYear->setProgress(0);
  lblCurrentYear->setText("<p align=\"right\">Calculating variables for " + theNameQString + "</p>");
  qApp->processEvents();
}
void CDPWizard::yearDone()
{
   //dont set progress to 0 - 0 has a special qt meaning of 'busy'
  //progressCurrentTask->setProgress(0);
  //progressCurrentYear->setProgress(0);
  //progressTotalJob->setProgress(progressTotalJob->progress()+1);
  qApp->processEvents();
}
void CDPWizard::variableStart(QString theNameQString)
{
  lblCurrentTask->setText("<p align=\"right\">" + theNameQString + "</p>");
  progressCurrentTask->setProgress(0);
}
void CDPWizard::variableDone(QString theFileNameString)
{
  std::cout << " ---------------- Variable " << theFileNameString << " written! " << std::endl;
  //convert the completed variable layer to an image file
  ImageWriter myImageWriter;
  myImageWriter.writeImage(theFileNameString,theFileNameString+QString(".png"));
  //perform the meridian shift (hard coding for now but we should have a class member 
  //boolean that stores whether this is needed
  MeridianSwitcher mySwitcher;
  mySwitcher.doSwitch(theFileNameString,QString("MS")+theFileNameString);
  //make an image for the shifted file too
  myImageWriter.writeImage(theFileNameString,QString("MS")+theFileNameString+QString(".png"));
  //set the image label on the calculating variables screen to show the last
  //variable calculated
  QPixmap myPixmap(QString("MS")+theFileNameString+QString(".png"));
  pixmapLabel2->setScaledContents(true);
  pixmapLabel2->setPixmap(myPixmap);
  //dont set progress to 0 - 0 has a special qt meaning of 'busy'
  //progressCurrentYear->setProgress(progressCurrentYear->progress()+1);
  //progressTotalJob->setProgress(progressTotalJob->progress()+1);
  qApp->processEvents();
}
void CDPWizard::cellDone(float theResultFloat)
{
  progressCurrentTask->setProgress(progressCurrentTask->progress()+1);
  progressCurrentYear->setProgress(progressCurrentYear->progress()+1);
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

}

void CDPWizard::checkInputFilenames()
{
    if (leMeanTemp->text() =="" && leMinTemp->text() =="" && leMaxTemp->text() ==""  && leDiurnalTemp->text()=="" && leMeanPrecipitation->text()=="" && leFrostDays->text()=="" && leFrostDays->text()=="" && leTotalSolarRadiation->text()=="" && leWindSpeed->text() =="" )
    {
        setNextEnabled(currentPage(),false);
    }
    else
    {
        setNextEnabled(currentPage(),true);
    }

}

void CDPWizard::pbtnMeanTemp_clicked()
{
    QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Mean Temperature","Select Mean Temperature");
    leMeanTemp->setText(myFileNameQString);
    checkInputFilenames();
}


void CDPWizard::pbtnMinTemp_clicked()
{
    QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Minimum Temperature","Select Minumum Temperature");
    leMinTemp->setText(myFileNameQString);
    checkInputFilenames();
}


void CDPWizard::pbtnMaxTemp_clicked()
{
    QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Max Temperature","Select Max Temperature");
    leMaxTemp->setText(myFileNameQString);
    checkInputFilenames();
}


void CDPWizard::pbtnDiurnalTemp_clicked()
{
    QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Diurnal Temperature","Select Diurnal Temperature");
    leDiurnalTemp->setText(myFileNameQString);
    checkInputFilenames();
}


void CDPWizard::pbtnMeanPrecipitation_clicked()
{
    QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Mean Precipitation","Select Mean Precipitation");
    leMeanPrecipitation->setText(myFileNameQString);
    checkInputFilenames();
}


void CDPWizard::pbtnFrostDays_clicked()
{
    QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Frost Days","Select Frost Days");
    leFrostDays->setText(myFileNameQString);
    checkInputFilenames();
}


void CDPWizard::pbtnTotalSolarRad_clicked()
{
    QString myFileNameQString = QFileDialog::getOpenFileName ("sample_data/","*.asc;*.grd",0,"Select Total Solar Radiation","Select Total Solar Radiation");
    leTotalSolarRadiation->setText(myFileNameQString);
    checkInputFilenames();
}

void CDPWizard::pbtnOutputPath_clicked()
{
    QString myFileNameQString = QFileDialog::getExistingDirectory(QString::null,0, QString("select dir"), QString("select dir"), true, true);
    leOutputPath->setText(myFileNameQString);
}


void CDPWizard::spinFirstYearInFile_valueChanged( int theInt)
{
    if (cbxYearType->currentText()=="AD")
    {
        //
        //Year type is AD
        //

        //check firstyeartocalc is not lower then firstyearinfile
        if (theInt > spinFirstYearToCalc->value())
        {
            spinFirstYearToCalc->setValue(spinFirstYearInFile->value());
        }

        //check lastyeartocalc is not lower then firstyearinfile
        if (theInt > spinLastYearToCalc->value())
        {
            spinLastYearToCalc->setValue(spinFirstYearInFile->value());
        }

        //set lower bounds of first and last year spin boxes to first year in file
        spinFirstYearToCalc->setMinValue(spinFirstYearInFile->value());
        spinLastYearToCalc->setMinValue(spinFirstYearInFile->value());
        spinFirstYearToCalc->setMaxValue(3000);
        spinLastYearToCalc->setMaxValue(3000);
    }

    else
    {
        //
        //Year type is BP
        //

        //check firstyeartocalc is not higher then firstyearinfile
        if (theInt < spinFirstYearToCalc->value())
        {
            spinFirstYearToCalc->setValue(spinFirstYearInFile->value());
        }

        //check lastyeartocalc is not lower then firstyearinfile
        if (theInt < spinLastYearToCalc->value())
        {
            spinLastYearToCalc->setValue(spinFirstYearInFile->value());
        }

        //set lower and upper bounds
        spinFirstYearToCalc->setMaxValue(spinFirstYearInFile->value());
        spinLastYearToCalc->setMaxValue(spinFirstYearInFile->value());
        spinFirstYearToCalc->setMinValue(0);
        spinLastYearToCalc->setMinValue(0);
    }
}

void CDPWizard::spinFirstYearToCalc_valueChanged( int theInt)
{
    if (cbxYearType->currentText()=="AD")
    {
        //
        //Year type is AD
        //

        if (theInt > spinLastYearToCalc->value())
        {
            spinLastYearToCalc->setValue(spinFirstYearToCalc->value());
        }
    }

    else
    {
        //
        //Year type is BP
        //

        if (theInt < spinLastYearToCalc->value())
        {
            spinLastYearToCalc->setValue(spinFirstYearToCalc->value());
        }

    }

}

void CDPWizard::spinLastYearToCalc_valueChanged( int theInt)
{
    if (cbxYearType->currentText()=="AD")
    {
        //
        //Year type is AD
        //

        if (theInt < spinFirstYearToCalc->value())
        {
            spinFirstYearToCalc->setValue(spinLastYearToCalc->value());
        }
    }

    else
    {
        //
        //Year type is BP
        //

        if (theInt > spinFirstYearToCalc->value())
        {
            spinFirstYearToCalc->setValue(spinLastYearToCalc->value());
        }

    }
}

void CDPWizard::cbxYearType_highlighted( const QString & theYearType )
{
    std::cout << "Setting year type to " << theYearType << std::endl;
    spinFirstYearToCalc->setSuffix(theYearType);
    spinLastYearToCalc->setSuffix(theYearType);

    if (theYearType=="AD")
    {
        //
        //Year type is AD
        //

        if (spinFirstYearInFile->value()>3000)
        {
            spinFirstYearInFile->setValue(2000);
        }

        if (spinFirstYearToCalc->value() < spinFirstYearInFile->value())
        {
            spinFirstYearToCalc->setValue(spinFirstYearInFile->value());
        }

        if (spinLastYearToCalc->value() < spinFirstYearInFile->value())
        {
            spinLastYearToCalc->setValue(spinFirstYearInFile->value());
        }

        spinFirstYearToCalc->setMinValue(spinFirstYearInFile->value());
        spinLastYearToCalc->setMinValue(spinFirstYearInFile->value());
        spinFirstYearToCalc->setMaxValue(3000);
        spinLastYearToCalc->setMaxValue(3000);


    }

    else
    {

        //
        //Year type is BP
        //

        if (spinFirstYearToCalc->value() > spinFirstYearInFile->value())
        {
            spinFirstYearToCalc->setValue(spinFirstYearInFile->value());
        }

        if (spinLastYearToCalc->value() > spinFirstYearInFile->value())
        {
            spinLastYearToCalc->setValue(spinFirstYearInFile->value());
        }

        spinFirstYearToCalc->setMaxValue(spinFirstYearInFile->value());
        spinLastYearToCalc->setMaxValue(spinFirstYearInFile->value());
        spinFirstYearToCalc->setMinValue(0);
        spinLastYearToCalc->setMinValue(0);


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


void CDPWizard::cboFileType_textChanged( const QString & )
{
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


void CDPWizard::leWindSpeed_textChanged( const QString & )
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
