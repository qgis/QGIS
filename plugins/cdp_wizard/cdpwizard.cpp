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

using namespace std;

CDPWizard::CDPWizard( QWidget* parent , const char* name , bool modal , WFlags fl  )
: CDPWizardBase( parent, name, modal, fl )
{
  initialise();
}  

CDPWizard::CDPWizard()
{
  initialise();
}

bool CDPWizard::initialise()
{
  int myPageCountInt = pageCount();
  cout << "CDPWizard has " << myPageCountInt << " pages." << endl;
  //set up the climate data processor object
  climateDataProcessor = new ClimateDataProcessor();
  //set up widgets om input files page
  QWidget *  myPageWidget = page(1);
  //get the handle of the filetype combo box
  QComboBox *myQComboBox = (QComboBox *)  myPageWidget->child("cboFileType");
  if ( myQComboBox )
  {
    myQComboBox->insertItem("CRES African climate data");
    myQComboBox->insertItem("ESRI & ASCII raster");
    myQComboBox->insertItem("Hadley Centre HadCM3 SRES Scenario");
    myQComboBox->insertItem("Hadley Centre HadCM3 IS92a Scenario");
    myQComboBox->insertItem("IPCC Observed Climatology");
    myQComboBox->insertItem("University of Reading Palaeoclimate data");
    myQComboBox->insertItem("Max Planck Institute fur Meteorologie (MPIfM) ECHAM4 data");
    myQComboBox->insertItem("CSIRO-Mk2 Model data");
    myQComboBox->insertItem("National Center for Atmospheric Research (NCAR) NCAR-CSM and NCAR-PCM data");
    myQComboBox->insertItem("Geophysical Fluid Dynamics Laboratory (GFDL) R30 Model data");
    myQComboBox->insertItem("Canadian Center for Climate Modelling and Analysis (CCCma) CGCM2 Model data");
    myQComboBox->insertItem("CCSR/NIES AGCM model data and CCSR OGCM model data");
  }

  //set up widgets on input parameters page
  myPageWidget = page(2);

  //get the handle of the lstVariablesToCalc combo box
  QListBox *myQListBox = (QListBox *)  myPageWidget->child("lstVariablesToCalc");
  if ( myQListBox )
  {
    cout << "Adding items into the lstVariablesToCalc list box" << endl;
    myQListBox->insertItem("Annual mean diurnal temperature range");
    myQListBox->insertItem("Annual mean number of frost days");
    myQListBox->insertItem("Annual mean total incident solar radiation");
    myQListBox->insertItem("Annual temperature range");
    myQListBox->insertItem("Highest temperature in warmest month");
    myQListBox->insertItem("Lowest temperature in coolest month");
    myQListBox->insertItem("Mean daily precipitation");
    myQListBox->insertItem("Mean daily precipitation in coolest month");
    myQListBox->insertItem("Mean daily precipitation in coolest quarter");
    myQListBox->insertItem("Mean daily precipitation in driest month");
    myQListBox->insertItem("Mean daily precipitation in driest quarter");
    myQListBox->insertItem("Mean daily precipitation in warmest month");
    myQListBox->insertItem("Mean daily precipitation in warmest quarter");
    myQListBox->insertItem("Mean daily precipitation in wettest month");
    myQListBox->insertItem("Mean daily precipitation in wettest quarter");
    myQListBox->insertItem("Mean diurnal temperature range in coolest month");
    myQListBox->insertItem("Mean diurnal temperature range in warmest month");
    myQListBox->insertItem("Mean precipitation in frost free months");
    myQListBox->insertItem("Mean temperature");
    myQListBox->insertItem("Mean temperature in coolest month");
    myQListBox->insertItem("Mean temperature in coolest quarter");
    myQListBox->insertItem("Mean temperature in frost free months");
    myQListBox->insertItem("Mean temperature in warmest month");
    myQListBox->insertItem("Mean temperature in warmest quarter");
    myQListBox->insertItem("Mean wind speed");
    myQListBox->insertItem("Number of months with minimum temperature above freezing");
    myQListBox->insertItem("Radiation in coolest month");
    myQListBox->insertItem("Radiation in coolest quarter");
    myQListBox->insertItem("Radiation in driest month");
    myQListBox->insertItem("Radiation in driest quarter");
    myQListBox->insertItem("Radiation in warmest month");
    myQListBox->insertItem("Radiation in warmest quarter");
    myQListBox->insertItem("Radiation in wettest month");
    myQListBox->insertItem("Radiation in wettest quarter");
    myQListBox->insertItem("Standard deviation of mean precipitation");
    myQListBox->insertItem("Standard deviation of mean temperature");

  }

  //set up the widgets on the file selection page
  myPageWidget = page(3);
  myQComboBox = (QComboBox *)  myPageWidget->child("cboOutputFormat");
  if ( myQComboBox )
  {
    cout << "Adding items into the output filetype combo box" << endl;
    myQComboBox->insertItem("CSM for Matlab");
    myQComboBox->insertItem("CSM for Octave");
    myQComboBox->insertItem("Desktop GARP");
    myQComboBox->insertItem("ESRI ASCII Grid");
    myQComboBox->insertItem("Plain matrix with no header");
  }   


  //presume all went ok
  return true;
}
CDPWizard::~CDPWizard(){
}

/** This method overrides the virtual CDPWizardBase method of the same name. */
void CDPWizard::cboFileType_activated( const QString &myQString )
{
  cout << "bcboFileType text changed" << endl;
  if (myQString=="CRES African climate data" || myQString=="University of Reading Palaeoclimate data")
  {
    lblFileSeriesNote->show( );
    //set the label on the summary form
    lblFileSeriesSummary->setText("<p align=\"right\">And <font color=\"red\"><b>are</b></font> in a file series</p>");
  }
  else
  {
    lblFileSeriesNote->hide( );
    //set the label on the summary form
    lblFileSeriesSummary->setText("<p align=\"right\">And <font color=\"red\"><b>are not</b></font> in a file series</p>");
  }

}

/** This is the page selected event which I am reimplementing to do some housekeeping
 *    between wizard pages. 
 *    This routine is a slot that runs each time next is pressed to update the summary page */
void CDPWizard::formSelected(const QString  &thePageNameQString)
{
  QString myQString;
  QLineEdit *myLineEdit;
  if (thePageNameQString=="File type and variables") //we do this after leaving the file selection page
  {

    //
    // get the mean temp file name
    //
    myLineEdit = (QLineEdit *) this->child("leMeanTemp");
    if (myLineEdit)
    {
      myQString = myLineEdit->text();
      std::cout << myQString << std::endl;
      std::string myFileNameString = myQString.latin1();
      climateDataProcessor->setMeanTempFileName(myFileNameString);
    }
    else
    {
      std::cout << "Could not locate leMeanTemp!" << std::endl;
      climateDataProcessor->setMeanTempFileName("");
    }
    //
    // get the min temp file name
    //
    myLineEdit = (QLineEdit *) this->child("leMinTemp");
    if (myLineEdit)
    {
      QString myQString = myLineEdit->text();
      std::cout << myQString << std::endl;
      std::string myFileNameString = myQString.latin1();
      climateDataProcessor->setMinTempFileName(myFileNameString);
    }
    else
    {
      std::cout << "Could not locate leMinTemp!" << std::endl;
      climateDataProcessor->setMinTempFileName("");      
    }

    //
    // get the max temp file name
    //
    myLineEdit = (QLineEdit *) this->child("leMaxTemp");
    if (myLineEdit)
    {
      QString myQString = myLineEdit->text();
      std::cout << myQString << std::endl;
      std::string myFileNameString = myQString.latin1();
      climateDataProcessor->setMaxTempFileName(myFileNameString);
    }
    else
    {
      std::cout << "Could not locate leMaxTemp!" << std::endl;
      climateDataProcessor->setMaxTempFileName("");      
    }

    //
    // get the diurnal temp file name
    //
    myLineEdit = (QLineEdit *) this->child("leDiurnalTemp");
    if (myLineEdit)
    {
      QString myQString = myLineEdit->text();
      std::cout << myQString << std::endl;
      std::string myFileNameString = myQString.latin1();
      climateDataProcessor->setDiurnalTempFileName(myFileNameString);

    }
    else
    {
      std::cout << "Could not locate leDiurnalTemp!" << std::endl;
      climateDataProcessor->setDiurnalTempFileName("");      
    }
    //
    // get the Mean Precipitation  file name
    //
    myLineEdit = (QLineEdit *) this->child("leMeanPrecipitation");
    if (myLineEdit)
    {
      QString myQString = myLineEdit->text();
      std::cout << myQString << std::endl;
      std::string myFileNameString = myQString.latin1();
      climateDataProcessor->setMeanPrecipFileName(myFileNameString);

    }
    else
    {
      std::cout << "Could not locate leMeanPrecipitation!" << std::endl;
      climateDataProcessor->setMeanPrecipFileName("");      
    }

    //
    // get the Frost Days   file name
    //
    myLineEdit = (QLineEdit *) this->child("leFrostDays");
    if (myLineEdit)
    {
      QString myQString = myLineEdit->text();
      std::cout << myQString << std::endl;
      std::string myFileNameString = myQString.latin1();
      climateDataProcessor->setFrostDaysFileName(myFileNameString);

    }
    else
    {
      std::cout << "Could not locate leFrostDays!" << std::endl;
      climateDataProcessor->setFrostDaysFileName("");      
    }

    //
    // get the Total Solar Radiation   file name
    //
    myLineEdit = (QLineEdit *) this->child("leTotalSolarRadiation");
    if (myLineEdit)
    {
      QString myQString = myLineEdit->text();
      std::cout << myQString << std::endl;
      std::string myFileNameString = myQString.latin1();
      climateDataProcessor->setTotalSolarRadFileName(myFileNameString);

    }
    else
    {
      std::cout << "Could not locate leTotalSolarRadiation!" << std::endl;
      climateDataProcessor->setTotalSolarRadFileName("");      
    }

    //
    // get the Wind Speed file name
    //
    myLineEdit = (QLineEdit *) this->child("leWindSpeed");
    if (myLineEdit)
    {
      QString myQString = myLineEdit->text();
      std::cout << myQString << std::endl;
      std::string myFileNameString = myQString.latin1();
      climateDataProcessor->setWindSpeedFileName(myFileNameString);

    }
    else
    {
      std::cout << "Could not locate leWindSpeed!" << std::endl;
      climateDataProcessor->setWindSpeedFileName("");      
    }
    /////////////////////////////////////////////////////////////////////
    //
    // OK now we need to update the list of available calculations that
    //                       can be carried out.
    //
    /////////////////////////////////////////////////////////////////////
    //
    // get the input file type
    //
    QComboBox *myQComboBox = (QComboBox *) this->child("cboFileType");
    if (myQComboBox)
    {
      QString myQString = myQComboBox->currentText();
      std::string myInputFileTypeString =  myQString.latin1();
      climateDataProcessor->setInputFileType(myInputFileTypeString);
    }
    else
    {
      std::cout << "Could not locate cboFileType!" << std::endl;
    }
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

