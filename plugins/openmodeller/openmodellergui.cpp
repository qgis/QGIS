/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   Gyps - Species Distribution Modelling Toolkit                         *
 *   This toolkit provides data transformation and visualisation           *
 *   tools for use in species distribution modelling tools such as GARP,   *
 *   CSM, Bioclim etc.                                                     *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "openmodellergui.h"

//qt includes
#include "qlineedit.h"
#include "qstring.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include <qtextstream.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qtextedit.h>
#include <qregexp.h>
#include <qlabel.h>
//
//openmodeller includes
#include <om_control.hh>
#include <om.hh>
#include <om_log.hh>
#include "file_parser.hh"

//standard includes
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

OpenModellerGui::OpenModellerGui()
 : OpenModellerGuiBase()
{
  _log.info( "\n%s qgis openModeller plugin starting.\n\n", "" );
}

OpenModellerGui::OpenModellerGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: OpenModellerGuiBase( parent, name, modal, fl )
{
   
  _log.info( "\n%s qgis openModeller plugin starting.\n\n", "" );
}  
OpenModellerGui::~OpenModellerGui()
{
}

/** This is the page selected event which I am reimplementing to do some housekeeping
 *    between wizard pages. 
 *    This routine is a slot that runs each time next is pressed  */
void OpenModellerGui::formSelected(const QString &thePageNameQString)
{
  std::cout << thePageNameQString << " has focus " << std::endl;
  QString myQString;
  if (thePageNameQString==tr("Step 3 of 8")) //we do this after leaving the file selection page
  {
    setNextEnabled(currentPage(),false);
    if (leLocalitiesFileName->text() !="")
    {
      setNextEnabled(currentPage(),true);
    }
  }
  if (thePageNameQString==tr("Step 4 of 8")) 
  {  
    if ( lstLayers->count()==0)
    {
      setNextEnabled(currentPage(),false);
    }  
    else
    {
      setNextEnabled(currentPage(),true);
    }
  }
  if (thePageNameQString==tr("Step 7 of 8")) 
  {  
    if ( leOutputFileName->text()=="")
    {
      setNextEnabled(currentPage(),false);
    }  
    else
    {
      setNextEnabled(currentPage(),true);
    }
  }  
  else if (thePageNameQString==tr("Step 8 of 8"))
  {
    setFinishEnabled(currentPage(),true);
  }
}


/**  This method is used when you already have a valid model paramters text file
     and want to run it without interacting with the user. 
     @see pbnRun method which is run when model inputs have been obtained via the wizard. */
void OpenModellerGui::parseAndRun(QString theParametersFileNameQString)
{
  //
  // Create a fileparser to read in the request file
  // and a controlInterface to manage the model process
  //
  
  //strdup is used to convert the qstring to a non const char *
  FileParser myFileParser( strdup(theParametersFileNameQString));
  ControlInterface  myController;
  
  //
  // Parse the model paramter file...
  //
  
  // Get the environmental variables (continuous and categorical)
  char *myMaskFileNameCharArray = myFileParser.get( "Mask" );
  char *myCategoricalMapCharArray = "Categorical map";
  int myNumberOfCategoriesInt = myFileParser.count( myCategoricalMapCharArray );
  //The key name in the paramters file that indicates its value is a map layer
  char *myLayerLabelCharArray = "Map";
  //find out the number of enviroinmental layers
  int myNumberOfLayersInt = myFileParser.count( myLayerLabelCharArray );
  //Find out the total layer count including environmental and mask layers
  int myTotalLayerCountInt = myNumberOfLayersInt + myNumberOfCategoriesInt;
  //create a char array and populate it with all layer names
  char *myLayersCollectionCharArray[myTotalLayerCountInt];
  myFileParser.getAll( myCategoricalMapCharArray, myLayersCollectionCharArray );
  myFileParser.getAll( myLayerLabelCharArray, myLayersCollectionCharArray + myNumberOfCategoriesInt );
  // Get the details for the output Map
  char *myOutputFleNameCharArray       = myFileParser.get( "Output" );
  char *myOutputFormatCharArray        = myFileParser.get( "Output format" );
  //scale is used to scale the model results e.g. from 0-1 to 0-255 - useful for image gen
  char *myScaleCharArray               = myFileParser.get( "Scale" );
  // find out which model algorithm is to be used e,g. bioclim, cartesian etc
  char *myAlgorithmNameCharArray       = myFileParser.get( "Algorithm" );
  //obtain any model parameters that are specified in the request file
  char *myAlgorithmParametersCharArray= myFileParser.get( "Parameters" );
  // Obtain the Well Known Text string for the localities coordinate system
  char *myLocalitiesCoordinateSystem   = myFileParser.get( "WKT coord system" );
  // Get the name of the file containing localities
  char *myLocalitiesFileCharArray      = myFileParser.get( "Species file" );
  // Get the name of the taxon being modelled!
  char *myTaxonNameCharArray           = myFileParser.get( "Species" );
  
  //
  // Make sure the basic variables have been defined in the parameter file...
  //  
  if ( ! myOutputFleNameCharArray )
  {
      QMessageBox::warning( this,QString("Acme Wizard Error"),QString("The 'Output file name' was not specified!"));
      return;
  }
  if ( ! myOutputFormatCharArray )
  {
      QMessageBox::warning( this,"Acme Wizard Error","The 'Output format' was not specified!");
      return;
  }
  if ( ! myScaleCharArray )
  {
    myScaleCharArray = "255.0";
  }
  //
  // Set up the output map builder
  //
  //RasterFile map( myOutputFormatCharArray );
  //
  // Set up the model controller
  //
  myController.setEnvironment(myNumberOfCategoriesInt, myTotalLayerCountInt, myLayersCollectionCharArray, myMaskFileNameCharArray );  
  // Prepare the output map
  
  myController.setOutputMap( myOutputFleNameCharArray,myOutputFormatCharArray, atof(myScaleCharArray) );
  // Set the model algorithm to be used by the controller
  myController.setAlgorithm( myAlgorithmNameCharArray, myAlgorithmParametersCharArray );
  // Populate the occurences list from the localities file
  myController.setOccurrences( myLocalitiesFileCharArray, myLocalitiesCoordinateSystem, myTaxonNameCharArray );

  //
  // Run the model
  //
  if ( ! myController.run() )
  {
    QString myErrorQString;
    QMessageBox::warning( this,"Acme Wizard Error",myErrorQString.sprintf("Error: %s\nModel aborted.", myController.error()));
  }
  else
  {
    //if all went ok, send notification to the parent app that we are finished
    emit drawRasterLayer(QString( myOutputFleNameCharArray ));
  }

} //end of parseAndRun

void OpenModellerGui::makeConfigFile()
{
    QFile myQFile( outputFileNameQString+".cfg");
    if ( myQFile.open( IO_WriteOnly ) ) {
        QTextStream myQTextStream( &myQFile );
        //write the header to the file
        myQTextStream << tr("#################################################################\n");
        myQTextStream << tr("##\n");
        myQTextStream << tr("##            openModeller Configuration file\n");
        myQTextStream << tr("##\n");
        myQTextStream << tr("##Autogenerated using openModeller Plugin for QGIS (c) T.Sutton 2003\n");
        myQTextStream << tr("##\n");
        myQTextStream << tr("#################################################################\n");
        myQTextStream << tr("\n\n##\n");                 
        myQTextStream << tr("## Coordinate system and projection in WKT format\n");
        myQTextStream << tr("##\n\n");        
       myQTextStream << tr("WKT Coord System = ") << coordinateSystemQString << "\n";
        
        myQTextStream << tr("\n\n##\n");                 
        myQTextStream << tr("## Localities Data Configuration Options...\n");
        myQTextStream << tr("##\n\n");
        myQTextStream << tr("# Full path and file name of the file containing localities data.\n");        
        myQTextStream << tr("Species file = ") << localitiesFileNameQString << "\n";
        myQTextStream << tr("# The taxon in the localities file to be modelled.\n");
        myQTextStream << tr("# (Defaults to the first taxon found.)\n");
        myQTextStream << tr("Species = ") << taxonNameQString << "\n";
        myQTextStream << tr("\n\n##\n");                 
        myQTextStream << tr("## Independent Variable Layers (map layers)\n");
        myQTextStream << tr("##\n\n");      
        // Iterate through the items in the layers list
        for ( QStringList::Iterator myIterator = layerNamesQStringList.begin(); myIterator != layerNamesQStringList.end(); ++myIterator)
        {          
          myQTextStream << tr("Map = ") << *myIterator << "\n";
        }
        myQTextStream << tr("# A layer that species the region of interest\n");      
        myQTextStream << tr("Mask = ") << maskNameQString << "\n";

        myQTextStream << tr("\n\n##\n");                 
        myQTextStream << tr("## Model Output Settings\n");
        myQTextStream << tr("##\n\n");   
        // NOTE by Tim: not too sure what this next option does - will have to sak Mauro
        // I think it just sets the ouput file extents etc
        // I am hardcoding it to match the first layer in the collection for now
        myQTextStream << tr("# File to be used as the output format.\n") ;
        myQTextStream << tr("Output format = ") << layerNamesQStringList.front() << "\n";
        myQTextStream << tr("# Output file name (should end in .tif)\n");
        myQTextStream << tr("Output = ") << outputFileNameQString << ".tif\n";
        myQTextStream << tr("# Scale algorithm output (originally between 0 and 1) by this factor.\n");
        //NOTE I am hard coding the output scaling variable for now!
        myQTextStream << tr("Scale = 240.0\n");                
        myQTextStream << tr("\n\n##\n");                 
        myQTextStream << tr("## Model Type and Extra Model Parameters\n");
        myQTextStream << tr("##\n\n");         
        myQTextStream << tr("# Name of the algorithm used to construct the model.\n");
        myQTextStream << tr("Algorithm  = ");
        if (modelNameQString=="")
        {
          // Default to bioclim if modelname has not been set
          modelNameQString="Bioclim";
        }
        myQTextStream << modelNameQString << "\n";
        // Iterate through the items in the extra parameters list
        for ( QStringList::Iterator myIterator = extraParametersQStringList.begin(); myIterator != extraParametersQStringList.end(); ++myIterator)
        {          
          myQTextStream << tr("Parameters = ") << *myIterator << "\n";
        }        
        
        myQTextStream << tr("\n\n###########################################\n");
        myQTextStream << tr("## End of autogenerated configuration file.\n");
        myQTextStream << tr("###########################################\n");
        //for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it )
        //    myQTextStream << *it << "\n";
        myQFile.close();
    }

}

  
  //
  // What follow are the overridden methods from the base form...
  //
/** The accept method overrides the qtwizard method of the same name and is run when the finish button is pressed */
void OpenModellerGui::accept()
{
  std::cout << "cboModelAlgorithm .. current text : " << cboModelAlgorithm->currentText() << std::endl;
  if (cboModelAlgorithm->currentText()==tr("Bioclimatic Envelope Model"))
  {
    modelNameQString="Bioclim";
  }
  else if (cboModelAlgorithm->currentText()==tr("Climate Space Model"))
  {
    modelNameQString="Csm";
  }
  else if (cboModelAlgorithm->currentText()==tr("Euclidian Distance"))
  {
    modelNameQString="Distance";
  }
  else if (cboModelAlgorithm->currentText()==tr("Min Distance"))
  {
    modelNameQString="MinDistance";
  }
  //pull all the form data into local class vars.
  outputFileNameQString=leOutputFileName->text();
  localitiesFileNameQString=leLocalitiesFileName->text();
  //build up the map layers qstringlist
  layerNamesQStringList.clear();
  for ( unsigned int myInt = 0; myInt < lstLayers->count(); myInt++ )
    {
      QListBoxItem *myItem = lstLayers->item( myInt );
      layerNamesQStringList.append(myItem->text());      
    }
  // build up the model parameters qstringlist
  extraParametersQStringList.clear();
  for ( unsigned int myInt = 0; myInt < lstParameters->count(); myInt++ )
  {
     QListBoxItem *myItem = lstParameters->item( myInt );
     extraParametersQStringList.append(myItem->text());      
  }   
  
  maskNameQString=cboMaskLayer->currentText();
  taxonNameQString=cboTaxon->currentText();
  coordinateSystemQString=txtWKT->text();
  //strip linefeeds and escape carriage returns in WKT string
  coordinateSystemQString=coordinateSystemQString.replace( QRegExp("\""), "\\\"" );
  coordinateSystemQString=coordinateSystemQString.replace( QRegExp("\n"), "" );
  makeConfigFile();
  parseAndRun(outputFileNameQString+".cfg");
  //close the dialog
  done(1);
}

void OpenModellerGui::cboModelAlgorithm_activated(  const QString &theAlgorithmQString)
{
  if (theAlgorithmQString==tr("Bioclimatic Envelope Model"))
  {
    modelNameQString=="Bioclim";
  }
  else if (theAlgorithmQString==tr("Cartesian Distance"))
  {
    modelNameQString=="MinDistance";
  }
}


void OpenModellerGui::pbnSelectOutputFile_clicked()
{
  std::cout << " OpenModellerGui::pbnSelectOutputFile_clicked() " << std::endl;
    QString myOutputFileNameQString = QFileDialog::getSaveFileName(
                    ".",
                    "All Files (*.)",
                    this,
                    "save file dialog"
                    "Choose a filename to save under" );
    leOutputFileName->setText(myOutputFileNameQString);
    if ( leOutputFileName->text()=="")
    {
      setNextEnabled(currentPage(),false);
    }  
    else
    {
      setNextEnabled(currentPage(),true);
    }
}

void OpenModellerGui::pbnRemoveParameter_clicked()
{
    for ( unsigned int myInt = 0; myInt< lstParameters->count(); myInt++ )
    {
        QListBoxItem *myItem = lstParameters->item( myInt );
        // if the item is selected...
        if ( myItem->isSelected() )
        {
            //remove the item if it is selected
            lstParameters->removeItem(myInt);
        }
    }
}


void OpenModellerGui::pbnAddParameter_clicked()
{
  lstParameters->insertItem(leNewParameter->text());
  leNewParameter->setText("");
}


void OpenModellerGui::pbnRemoveLayerFile_clicked()
{
    for ( unsigned int myInt = 0; myInt< lstLayers->count(); myInt++ )
    {
        QListBoxItem *myItem = lstLayers->item( myInt );
        // if the item is selected...
        if ( myItem->isSelected() )
        {
            //remove the item if it is selected
            lstLayers->removeItem(myInt);
            //also remove the item from the mask layer combo
           cboMaskLayer->removeItem(myInt);
        }
    }
    //if user has removed last list entry, disable next button
    if ( lstLayers->count()==0)
    {
      setNextEnabled(currentPage(),false);
    }
    
}


void OpenModellerGui::pbnSelectLayerFile_clicked()
{
  std::cout << " OpenModellerGui::pbnSelectLayerFile_clicked() " << std::endl;
  QString myFileTypeQString;
  QString myGDALFilterString="GDAL (*.tif; *.asc; *.bil;*.jpg;*.adf)";
  QString myFileNameQString = QFileDialog::getOpenFileName(
   "" , //initial dir
  myGDALFilterString,  //filters to select
  this , //parent dialog
  "OpenFileDialog" , //QFileDialog qt object name
  "Select localities text file" , //caption
  &myFileTypeQString //the pointer to store selected filter
 );  
  std::cout << "Selected filetype filter is : " << myFileTypeQString << std::endl;
  if (myFileNameQString==NULL || myFileNameQString=="") return;
  //check if the file is an arc/info binary grid in which case we should only use the
  //directory name in which the adf file occurs
  if (myFileNameQString.endsWith(".adf"))
  {
    //try to find  unix path separater first (search backwards from end of line)
    if (myFileNameQString.findRev('/') != -1)
    {
      myFileNameQString=myFileNameQString.mid(0,myFileNameQString.findRev('/')+1);
    }
    else //no forward slash found so assume dos and look for backslash
    {
      //try looking for dos separaters
      myFileNameQString=myFileNameQString.mid(0,myFileNameQString.findRev('\\')+1);
    }
  }
  lstLayers->insertItem(myFileNameQString);
  //also add the layer to the mask combo
  cboMaskLayer->insertItem(myFileNameQString);
  //enable the user to carry on to the next page...
  setNextEnabled(currentPage(),true);
}


void OpenModellerGui::pbnSelectLocalitiesFile_clicked()
{
  std::cout << " OpenModellerGui::pbnSelectLocalitiesFile_clicked() " << std::endl;
  QString myFileTypeQString;
  QString myTextFileFilterString="Text File (*.txt)";
  QString myFileNameQString = QFileDialog::getOpenFileName(
   "" , //initial dir
  myTextFileFilterString,  //filters to select
  this , //parent dialog
  "OpenFileDialog" , //QFileDialog qt object name
  "Select localities text file" , //caption
  &myFileTypeQString //the pointer to store selected filter
 );  
  std::cout << "Selected filetype filter is : " << myFileTypeQString << std::endl;
  if (myFileNameQString==NULL || myFileNameQString=="") return;
  
  
  //
  // Now that we have the localities text file, we need to parse it and find
  // all unique instances of taxon names and populate the taxon combo...
  //
  //first build a regex to match text at the beginning of the line
 QRegExp myQRegExp( "^[^#][ a-zA-Z]*" ); //seconf caret means 'not'
 QStringList myTaxonQStringList;;
 QFile myQFile( myFileNameQString );
 if ( myQFile.open( IO_ReadOnly ) ) 
 {
     //clear the existing entries in the taxon combo first
     cboTaxon->clear();     
     //now we parse the loc file, checking each line for its taxon
     QTextStream myQTextStream( &myQFile );
     QString myCurrentLineQString;
     while ( !myQTextStream.atEnd() ) 
     {
        myCurrentLineQString = myQTextStream.readLine(); // line of text excluding '\n'
        myQRegExp.search(myCurrentLineQString);
        QStringList myMatchesQStringList = myQRegExp.capturedTexts();
        QStringList::Iterator myIterator = myMatchesQStringList.begin();
        QString myTaxonQString=*myIterator;
        myTaxonQString=myTaxonQString.stripWhiteSpace();
        if (myTaxonQString != "")
        {
          //make sure there are only single spaces separating words.
          myTaxonQString=myTaxonQString.replace( QRegExp(" {2,}"), " " );
          myTaxonQStringList.append(myTaxonQString);
        }
     }
     myQFile.close();
     //sort the taxon list alpabetically
     myTaxonQStringList.sort();
     //now find the uniqe entries in the qstringlist and
     //add each entry to the taxon combo
     QString myLastTaxon="";
     QStringList::Iterator myIterator= myTaxonQStringList.begin();
     while( myIterator!= myTaxonQStringList.end() ) 
     {
        QString myCurrentTaxon=*myIterator;
        if (myCurrentTaxon!=myLastTaxon)
        {
          cboTaxon->insertItem(myCurrentTaxon);
        }
        myLastTaxon=*myIterator;
         ++myIterator;
     }     
 }
 else
 {
      QMessageBox::warning( this,QString("Acme Wizard Error"),QString("The localities file is not readable. Check you have the neccessary file permissions and try again."));      
      return; 
 }   
  // if all that went ok, update the form field and the class var  
  leLocalitiesFileName->setText(myFileNameQString);
  localitiesFileNameQString = myFileNameQString;
  //enable the user to carry on to the next page...
  setNextEnabled(currentPage(),true);
} //end of pbnSelectLocalitiesFile_clicked

void OpenModellerGui::leLocalitiesFileName_textChanged( const QString &theFileNameQString )
{
  // this should never run - I am changing the line  edit control to read only so that
  // file parsing for taxon names does not happen with every text change
  localitiesFileNameQString = theFileNameQString;
   //enable the user to carry on to the next page...
  setNextEnabled(currentPage(),true);
}

void OpenModellerGui::leLocalitiesFileName_returnPressed()
{
  // this should never run - I am changing the line  edit control to read only so that
  // file parsing for taxon names does not happen with every text change
  localitiesFileNameQString = leLocalitiesFileName->text();
    //enable the user to carry on to the next page...
  setNextEnabled(currentPage(),true);
}


