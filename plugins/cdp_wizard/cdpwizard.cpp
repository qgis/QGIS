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
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <string.h>
#include <qlabel.h>
#include <qlabel.h>


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
  std::cout << "Adding items into the lstVariablesToCalc list box" << std::endl;
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
  cout << "Adding items into the output filetype combo box" << endl;
  cboOutputFormat->insertItem(tr("CSM for Matlab"));
  cboOutputFormat->insertItem(tr("CSM for Octave"));
  cboOutputFormat->insertItem(tr("Desktop GARP"));
  cboOutputFormat->insertItem(tr("ESRI ASCII Grid"));
  cboOutputFormat->insertItem(tr("Plain matrix with no header"));

  //
  // Create a climate data processor instance - this is the thing that does all 
  // the hard work!
  //
  climateDataProcessor = new ClimateDataProcessor();

  //presume all went ok
  return true;
}


/** This method overrides the virtual CDPWizardBase method of the same name. */
void CDPWizard::cboFileType_activated( const QString &myQString )
{
  std::cout << "cboFileType text changed" << std::endl;
  if (myQString==tr("CRES African climate data") || myQString==tr("University of Reading Palaeoclimate data"))
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

/** This is the page selected event which I am reimplementing to do some housekeeping
 *    between wizard pages. 
 *    This routine is a slot that runs each time next is pressed to update the summary page */
void CDPWizard::formSelected(const QString  &thePageNameQString)
{
  QString myQString;
  QLineEdit *myLineEdit;
  if (thePageNameQString==tr("File type and variables")) //we do this after leaving the file selection page
  {
    climateDataProcessor->setMeanTempFileName(leMeanTemp->text());
    climateDataProcessor->setMinTempFileName(leMinTemp->text());
    climateDataProcessor->setMaxTempFileName(leMaxTemp->text());
    climateDataProcessor->setDiurnalTempFileName(leDiurnalTemp->text());
    climateDataProcessor->setMeanPrecipFileName(leMeanPrecipitation->text());
    climateDataProcessor->setFrostDaysFileName(leFrostDays->text());
    climateDataProcessor->setTotalSolarRadFileName(leTotalSolarRadiation->text());
    climateDataProcessor->setWindSpeedFileName(leWindSpeed->text());

    /////////////////////////////////////////////////////////////////////
    //
    // OK now we need to update the list of available calculations that
    //                       can be carried out.
    //
    /////////////////////////////////////////////////////////////////////
    //
    // get the input file type
    //
      std::string myInputFileTypeString =  cboFileType->currentText().latin1();
      climateDataProcessor->setInputFileType(myInputFileTypeString);
    //Should not need to have the next line here - it slows everythinf down!
    //climateDataProcessor->makeFileGroups(0);
    climateDataProcessor->makeAvailableCalculationsMap();
    // and then update the list box

    //List the calculations in  availableCalculationsMap  using an iterator
    map<std::string, bool> myAvailableCalculationsMap = climateDataProcessor->getAvailableCalculationsMap();
    map<std::string, bool>::const_iterator myIter;
    cout << "******** updated available calculations list **********" << endl;
    //clear the current entries from the box
    lstVariablesToCalc->clear();
    for (myIter=myAvailableCalculationsMap.begin(); myIter != myAvailableCalculationsMap.end(); myIter++)
    {
      if (myIter->second)
      {
        cout << myIter->first << std::string(": true\n");
        QString * myQString = new  QString(    myIter->first.c_str() );
        //need to add some logic here to select the inserted item
        lstVariablesToCalc->insertItem(*myQString);
      }
      else
      {
        cout << myIter->first <<  std::string(": false\n");
        QString * myQString = new  QString(    myIter->first.c_str() );
        //need to add some logic here to select the inserted item
        lstVariablesToCalc->insertItem(*myQString);           
      }
    }


  } //end of test for page 3

  if (thePageNameQString=="Summary of processing to be performed") //we do this when  we arrive at the summary of variables to be calculated
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

  if (thePageNameQString=="Progress") //we do this when we start the calculation
  {
    int myFirstYearInFileInt, myJobStartYearInt, myJobEndYearInt;
    std::string myInputFileTypeString, myOutputFileTypeString, myOutputPathString;
    QString myQString;
    /////////////////////////////////////////////////////////////
    //                                                         //
    // First we test the basic form of climate data processor  //
    // where no parameters are passed with the constructor and //
    // each property is set individually.                      //
    //                                                         //
    /////////////////////////////////////////////////////////////

    //
    // get the first year in file value
    //
    QSpinBox *myQSpinBox = (QSpinBox *) this->child("spinFirstYearInFile");
    if (myQSpinBox)
    {
      myFirstYearInFileInt = myQSpinBox->value();
      climateDataProcessor->setFileStartYearInt(myFirstYearInFileInt);
    }
    else
    {
      std::cout << "Could not locate spinFirstYearInFile!" << std::endl;
    }

    //
    // get the first year in file to be processed in this job
    //
    myQSpinBox = (QSpinBox *) this->child("spinFirstYearToCalc");
    if (myQSpinBox)
    {
      myJobStartYearInt = myQSpinBox->value();
      climateDataProcessor->setJobStartYearInt(myJobStartYearInt);
    }
    else
    {
      std::cout << "Could not locate spinFirstYearToCalc!" << std::endl;
    }

    //
    // get the last year in file to be processed in this job
    //
    myQSpinBox = (QSpinBox *) this->child("spinLastYearToCalc");
    if (myQSpinBox)
    {
      myJobEndYearInt = myQSpinBox->value();
      climateDataProcessor->setJobEndYearInt(myJobEndYearInt);
    }
    else
    {
      std::cout << "Could not locate spinLastYearToCalc!" << std::endl;
    }

    //
    // get the input file type
    //
    QComboBox *myQComboBox = (QComboBox *) this->child("cboFileType");
    if (myQComboBox)
    {
      myQString = myQComboBox->currentText();
      myInputFileTypeString =  myQString.latin1();
      climateDataProcessor->setInputFileType(myInputFileTypeString);
    }
    else
    {
      std::cout << "Could not locate cboFileType!" << std::endl;
    }

    //
    // get the ouput file type
    //
    myQComboBox = (QComboBox *) this->child("cboOutputFormat");
    if (myQComboBox)
    {
      myQString = myQComboBox->currentText();
      myInputFileTypeString =  myQString.latin1();
      climateDataProcessor->setOutputFileType(myInputFileTypeString);
    }
    else
    {
      std::cout << "Could not locate cboOutputFormat!" << std::endl;
    }

    //
    // get the ouput file path
    //
    QLineEdit * myQLineEdit = (QLineEdit *) this->child("leOutputPath");
    if (myQLineEdit)
    {
      myQString = myQLineEdit->text();
      myOutputPathString =  myQString.latin1();
      climateDataProcessor->setOutputFilePathString(myOutputPathString);
    }
    else
    {
      std::cout << "Could not locate cboOutputFormat!" << std::endl;
    }

    //
    // find out if datafiles are in series (discrete files for each month)
    // We are using the isVisible property for the label, but we cant guarantee
    // that the wizard page on which the label occurs will be visible so we need
    //' to use the isVisibleTo(parent) property, with a reference to the wzard page as parent
    //
    QWidget *  myPageWidget = page(1);
    QLabel *myLabel = (QLabel *) this->child("lblFileSeriesNote");
    if (myLabel)
    {
      cout << "\nMyLabel visible? "<<   myLabel->isVisible() << endl;
      if (myLabel->isVisibleTo(myPageWidget))
      {
        cout << "Setting files in series flag to true" << endl;
        climateDataProcessor->setFilesInSeriesFlag(true);
      }
      else
      {
        cout << "Setting files in series flag to false" << endl;
        climateDataProcessor->setFilesInSeriesFlag(false);
      }
    }
    else
    {
      std::cout << "Could not locate lblFileSeriesNote!" << std::endl;
    }

    //setup the climate data processor's filereaders
    climateDataProcessor->makeFileGroups (1);    //hardcoding year 1 for now

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
    cout << climateDataProcessor->getDescription() << endl;
    //
    //now we let the climatedataprocessor run!
    //
    climateDataProcessor->run();


  }
}               //end of formSelected

