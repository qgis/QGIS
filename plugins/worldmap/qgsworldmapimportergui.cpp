/***************************************************************************
                          qgsworldmapimportergui.cpp  -  description
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

#include "qgsworldmapimportergui.h"
#include <qstring.h>
#include <qlineedit.h>
#include <qsettings.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qcombobox.h>

//non qt headers
#include "vector.h"
#include "string.h"
#include <fstream.h>
#include <iostream>
#include <cstdlib>
#include <cassert>  //assert debugging
#include <stdio.h>


QgsWorldMapImporterGui::QgsWorldMapImporterGui():QgsWorldMapImporterGuiBase()
{
  //disable the ok button until both input and output filenames have been chosed
    pbnOK->setEnabled(false);
}

QgsWorldMapImporterGui::~QgsWorldMapImporterGui()
{
  
}






void QgsWorldMapImporterGui::pbnInputFile_clicked()
{
  QSettings settings;          // where we keep last used filter in  persistant state
  //an auto_ptr means we dont have to delete the object later
  std::auto_ptr<QFileDialog> myFileDialogPointer( new QFileDialog( ".",
              "All Files (*.*)",
              0,
              QFileDialog::tr("File to be imported")));

  // allow for selection of only one file
  myFileDialogPointer->setMode( QFileDialog::ExistingFiles );


  if ( myFileDialogPointer->exec() == QDialog::Accepted )
  {
    leInputFileName->setText(myFileDialogPointer->selectedFile());
  }
  //check if we can enable the ok button
  if ((leInputFileName->text()!="")&&(leOutputFileName->text()!=""))
  {
    pbnOK->setEnabled(true);
  }
}


void QgsWorldMapImporterGui::pbnOutputFile_clicked()
{
  QSettings settings;          // where we keep last used filter in  persistant state
  //an auto_ptr means we dont have to delete the object later
  QString myOutputFileNameQString = QFileDialog::getSaveFileName(
          ".",
          "Arc/Info ASCII Grid (*.asc)",
          this,
          "save file dialog"
          "Choose a filename to save under" );


  if (myOutputFileNameQString!=NULL)
  {
   leOutputFileName->setText(myOutputFileNameQString);
  }
  //check if we can enable the ok button
  if ((leInputFileName->text()!="")&&(leOutputFileName->text()!=""))
  {
    pbnOK->setEnabled(true);
  }
}

void QgsWorldMapImporterGui::pbnOK_clicked()
{



  enum FileType {MINIMUM_AREA_SET, SPECIES_RICHNESS} myFileType;
  /** Values which represent null or no data in the file */
  float myNoDataFloat;
  /** Current column in the matrix */
  long myCurrentColLong;
  /** Current row in matrix */
  long myCurrentRowLong;
  /** Position in the matrix expressed as (current row * cols) + current col */
  long myCurrentElementLong;
  /**  The FILE handle containing our input data matrix. */
  FILE *myInputFilePointer;

  /** Flag indicating whether we should output debug info */
  bool myDebugModeFlag=true;
  /** Char array to hold each line of text */
  char myCharArray [1024];
  /** Flag to show whether file opened ok */
  bool myFlag;
  /** number of cols in output matrix - must correspond to worldmap dims*/
  int myColCountInt = 360;
  /** number of rows in output matrix - must correspond to worldmap dims */
  int myRowCountInt = 180;
  /** vector representing the complete matrix */
  vector <float> myMatrixVector;

  //
  // Determine file type to be processed
  //
  if (cboInputFileType->currentText()==tr("Minimum Area Selection"))
  {
    myFileType=MINIMUM_AREA_SET;
  }
  else if (cboInputFileType->currentText()==tr("Species Richness"))
  {
    myFileType=SPECIES_RICHNESS;
  }
  else
  {
    /*
       cout << argv[1] << " is an invalid file type - filetype must be ma or sr." << endl;
       cout << argv[0] << " useage: " << argv[0] << " ma|sr infile outfile" << endl;
       cout << "Where ma - minimum area set file and sr = species richness file" << endl;
       return 1;
       */

  }

  //
  // Open the input file....
  //

  if ((myInputFilePointer=fopen(leInputFileName->text(),"r"))==NULL)  //open in read only binary mode
  {
    cout << "Cannot open file : " << leInputFileName->text() << " for input...aborting" << endl;
    //cout << "Cannot open file : " << argv[2] << " for input...aborting" << endl;
    //return false;  //give up
    return;
  }
  else
  {
    cout << "File : " << leInputFileName->text() << " opened for input" << endl;
    //cout << "File : " << argv[2] << " opened for input" << endl;
  }


  //
  // Open the output file....
  //
  ofstream myOuputFileStream(leOutputFileName->text(), ios::out);
  //ofstream myOuputFileStream(argv[3], ios::out);
  if (! myOuputFileStream.is_open())
  {

    cout << "Cannot open file : " << leOutputFileName->text() << " for output...aborting" << endl;
    //cout << "Cannot open file : " << argv[3] << " for output...aborting" << endl;
    //return false;  //give up
    return ;  //give up
  }
  else
  {
    cout << "File : " << leOutputFileName->text() << " opened for output" << endl;
    //cout << "File : " << argv[3] << " opened for output" << endl;
  }


  //populate the vector with -9999 to initialise it
  myNoDataFloat=-9999;
  int myCounterInt = 0;
  for (myCounterInt; myCounterInt < (myColCountInt*myRowCountInt); ++myCounterInt)
  {
    myMatrixVector.push_back(myNoDataFloat);
  }


  //skip 12 header lines if the file type is min area
  if (myFileType==MINIMUM_AREA_SET)
  {
    for (int myCounterInt = 0; myCounterInt < 13; ++myCounterInt)
    {
      fgets(myCharArray, 1024, myInputFilePointer);
    }
  }
  //skip 5 header lines if the file type is species richness
  else if (myFileType==SPECIES_RICHNESS)
  {
    for (int myCounterInt = 0; myCounterInt < 5; ++myCounterInt)
    {
      fgets(myCharArray, 1024, myInputFilePointer);
    }
  }
  //
  // Parse the datafile
  //
  unsigned int mySpeciesCountInt=0;
  unsigned int myRecordCountInt=0;
  unsigned int myAreaInt=0;
  unsigned int myElementCountInt=0;
  // Read the data. Bail out when no fields filled (0 means no fields were filled)
  do {
    fgets(myCharArray, 1024, myInputFilePointer);
    //
    //        // Get the record line number
    //
    int myResultInt = fscanf(myInputFilePointer,"%i",&myRecordCountInt);
    cout << myResultInt << ": Record No : " << myRecordCountInt ;
    //set the vector value matching this area with
    //the number of taxa
    if (myResultInt < 1)
    {
      cout << endl << "Record Number could not be read from "
          << myElementCountInt+1 << " of input matrix. " << endl;
      continue;
    }
    //                                                                                                                    //
    // Get the  area number
    //
    myResultInt = fscanf(myInputFilePointer,"%i",&myAreaInt);
    cout <<  " Area : " << myAreaInt ;
    //set the vector value matching this area with
    //the number of taxa
    if (myResultInt < 1)
    {
      cout << endl << "Area could not be read from "
          << myElementCountInt+1 << " of input matrix. " << endl;
      continue;
    }
    //
    // Get the  species count
    //
    myResultInt = fscanf(myInputFilePointer,"%i",&mySpeciesCountInt);
    cout << " Species count : " << mySpeciesCountInt << endl;
    //set the vector value matching this area with
    //the number of taxa
    if (myResultInt < 1)
    {
      cout << endl << "Species count could not be read from "
          << myElementCountInt+1 << " of input matrix. " << endl;
      continue;
    }
    //see if we have an unusually high species count for this cell
    if (mySpeciesCountInt > 200)
    {
      cout << endl << "An unusually large  Species Count was found on row "
          << myElementCountInt+1 << " of input matrix ("<< mySpeciesCountInt << ")" << endl;
    }
    myMatrixVector[myAreaInt]=mySpeciesCountInt;
    ++myElementCountInt;

  }while (!feof(myInputFilePointer));


    //printf ("%i Elements read\n" , myElementCount);


    myOuputFileStream << "ncols " <<  myColCountInt << endl;
    myOuputFileStream << "nrows " << myRowCountInt << endl;
    myOuputFileStream << "xllcorner " << -180 << endl;
    myOuputFileStream << "yllcorner " << -90 << endl;
    myOuputFileStream << "cellsize " << 1 << endl;
    myOuputFileStream << "nodata_value " << -9999 << endl;

    //show the user what we got in the final vector
    myCounterInt = 0;
    int myMatrixSize=myColCountInt*myRowCountInt;
    //cout << "Matrix is " << myMatrixSize << " element long" << endl;
    for (myCounterInt; myCounterInt < myMatrixSize; ++myCounterInt)
    {
        int myCellValueInt = myMatrixVector[myCounterInt];
        if (myCellValueInt > 200)
        {
            cout << myCounterInt << " has an unusually large value (" << myCellValueInt << ")" << endl;
        }
        //decide whether to start a new line
        if ((myCounterInt % myColCountInt)==0)
        {
            myOuputFileStream << myCellValueInt << endl ; //note endl!
        }
        else
        {
            myOuputFileStream << myCellValueInt << " " ; //note no endl!
        }
    }

    //
    // Close the input file again....
    //
    try
    {
        fclose (myInputFilePointer);
    }
    catch (...)
    {
        //return false;
        return ;
    }
    //
    // Close the output file again....
    //
    try
    {
        myOuputFileStream.close();
    }
    catch (...)
    {
        //return false;
        return ;
    }

  emit drawLayer(leOutputFileName->text());
    //close the dialog
  close();
  
}
