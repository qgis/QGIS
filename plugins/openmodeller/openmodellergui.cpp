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
#include <qlineedit.h>
#include <qstring.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qtextedit.h>
#include <qtextbrowser.h>
#include <qregexp.h>
#include <qlabel.h>
#include <qsettings.h>
#include <qcursor.h>
#include <qapplication.h> 

//
//openmodeller includes
#include <openmodeller/om_control.hh>
#include <openmodeller/om.hh>
#include <om_alg_parameter.hh>
#include <request_file.hh>

//standard includes
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

OpenModellerGui::OpenModellerGui()
 : OpenModellerGuiBase()
{
  getAlgorithmList();
}

OpenModellerGui::OpenModellerGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: OpenModellerGuiBase( parent, name, modal, fl )
{
   
  getAlgorithmList();
}  

OpenModellerGui::~OpenModellerGui()
{
}

void OpenModellerGui::getAlgorithmList()
{

  OpenModeller  myOpenModeller;
  // Find out which model algorithm is to be used.
  std::cerr << "-------------- openModeller plugin :  Reading algorithm list..." << std::endl;
  AlgMetadata **myAlgorithmMetadataArray = myOpenModeller.availableAlgorithms();
  AlgMetadata *myAlgorithmMetadata;
  //loop through the algorithm names adding to the algs combo
  while ( myAlgorithmMetadata = *myAlgorithmMetadataArray++ )
  {
    std::cerr << "Found Algorithm: " << myAlgorithmMetadata->id << std::endl;
    cboModelAlgorithm->insertItem(myAlgorithmMetadata->id);
  }     
  delete myAlgorithmMetadataArray;
  delete myAlgorithmMetadata;
  return ;

}

void OpenModellerGui::getParameterList( QString theAlgorithmNameQString )
{
  OpenModeller  myOpenModeller;
  // Find out which model algorithm is to be used.
  AlgMetadata **myAlgorithmsMetadataArray = myOpenModeller.availableAlgorithms();
  AlgMetadata *myAlgorithmMetadata;
  std::cerr << "-------------- openModeller plugin :  Reading algorithm list..." << std::endl;

  //Clear parameters combo pick list in case reloading page
  cbxParameterType->clear();
  
  while ( myAlgorithmMetadata = *myAlgorithmsMetadataArray++ )
  {
    std::cerr << "Found Algorithm: " << myAlgorithmMetadata->id << std::endl;
    QString myAlgorithmNameQString=myAlgorithmMetadata->id;
    if (myAlgorithmNameQString==theAlgorithmNameQString)
    {

      int myParameterCountInt = myAlgorithmMetadata->nparam;
      AlgParamMetadata * myParameter = myAlgorithmMetadata->param;
      //detailed list of parameters and their useage
      //to be placed in a textbox control on the algorithm selection page

      txtAlgorithmParameters->setText("<h3>Algorithm Parameters</h3><p>Use the descriptions below set parameters for this algorithm</p>");
      for ( int i = 0; i < myParameterCountInt; i++, myParameter++ )
      {
        //
        //first we add a new combo item to the parameter picklist
        //
        QString myQString = myParameter->name ;
        cbxParameterType->insertItem(myQString);
        //
        // Now we build up a detailed description of the parameters
        //

        //check if the parameter has min and max constraints
        QString myDescriptionQString=""; 
        QString myHeadingQString=""; 

        myQString="";
        if ( myParameter->has_min && myParameter->has_max )
        {
          myQString.sprintf( "<p><b>%s (&gt;= %f and &lt;= %f) default is %f</b></p>\n", myParameter->name, myParameter->min, myParameter->max, myParameter->typical );
        }
        //or just min constraint
        else if ( myParameter->has_min )
        {
          myQString.sprintf( "<p>%s (&gt;= %f) default is %f</b></p>\n", myParameter->name, myParameter->min, myParameter->typical );
        }
        //or just max contraint
        if ( myParameter->has_max )
        {
          myQString.sprintf( "<p>%s (&lt;= %f) default is %f</b></p>\n", myParameter->name, myParameter->max, myParameter->typical );
        }
        //or neither
        else
        {
          myQString.sprintf( "<p>%s default is %f</b></p>\n", myParameter->name, myParameter->typical);
        }

        myHeadingQString.sprintf( "<p>%s</p>", myParameter->description );
        txtAlgorithmParameters->setText(txtAlgorithmParameters->text()+myQString+myDescriptionQString);
        std::cerr << txtAlgorithmParameters->text() << std::endl;
      }
    }
  }
  
  
  // no matching algorthm was found :-(
  delete myAlgorithmsMetadataArray;
  delete myAlgorithmMetadata;
  return;
  
}

/** This is the page selected event which I am reimplementing to do some housekeeping
 *    between wizard pages. 
 *    This routine is a slot that runs each time next is pressed  */
void OpenModellerGui::formSelected(const QString &thePageNameQString)
{
  std::cout << thePageNameQString << " has focus " << std::endl;
  QString myQString;
  QSettings settings;
  if (thePageNameQString==tr("Step 1 of 8")) //we do this when arriving at the mode selection page
  {
    //select the last model used by getting the name from qsettings
    QString myModelName = settings.readEntry("/openmodeller/modelName");
    if (myModelName=="")
    {
      //do nothing
    }
    else
    {
      cboModelAlgorithm->setCurrentText(tr(myModelName));
    }

  }
  if (thePageNameQString==tr("Step 2 of 8")) //we do this after leaving the file selection page
  {
    settings.writeEntry("/openmodeller/modelName",cboModelAlgorithm->currentText());
    getParameterList(cboModelAlgorithm->currentText());
    
    
    //set the coordinate system to the same as the last time run
    QString myCoordSystem = settings.readEntry("/openModeller/coordSystem");
    if (myCoordSystem=="")
    {
      //do nothing
    }
    else
    {
      cboCoordinateSystem->setCurrentText(tr(myCoordSystem));
    }
    
  }
  if (thePageNameQString==tr("Step 3 of 8")) //we do this after leaving the file selection page
  {
    setNextEnabled(currentPage(),false);
    settings.writeEntry("/openmodeller/coordSystem",cboCoordinateSystem->currentText());        
    
    if (leLocalitiesFileName->text() !="")
    {
      setNextEnabled(currentPage(),true);
    }
    else
    {
      QString myFileName = settings.readEntry("/openmodeller/localitiesFileName");
      if (!myFileName.isEmpty())
      {
        setSpeciesList(myFileName);
      } 
    }
  }
  if (thePageNameQString==tr("Step 4 of 8")) 
  {  

    const QString myFileNameQString =  settings.readEntry("/openmodeller/layerNames");
    //tokenise the setting list (its separated by ^e)
    const QString mySeparatorQString = "^e";
    QStringList myFileNameQStringList =  QStringList::split (mySeparatorQString, myFileNameQString, false ); 
    //only try to restore the list of layers used in the last session if the list is empty
    if (myFileNameQStringList.size() > 0 && lstLayers->count()==0)
    {
      //loop through the layer names
     QStringList::Iterator myIterator= myFileNameQStringList.begin();
     QString myLastFileNameQString="";
     while( myIterator!= myFileNameQStringList.end() ) 
     {
        QString myFileNameQString=*myIterator;
        if (myFileNameQString!=myLastFileNameQString)
        {
          lstLayers->insertItem(myFileNameQString);
          //also add the layer to the mask combo
          cboMaskLayer->insertItem(myFileNameQString);
        }
        myLastFileNameQString=*myIterator;
         ++myIterator;
     }     
      //enable the user to carry on to the next page...
      setNextEnabled(currentPage(),true);
    }
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

  OpenModeller  myOpenModeller;
  RequestFile myRequestFile;
  int myResult = myRequestFile.configure( &myOpenModeller, strdup(theParametersFileNameQString));

  if ( myResult < 0 )
  {
    //notify user of eror here!
    QMessageBox::warning( this,"openModeller Wizard Error","Error reading request file!");
    return;
  }

  if ( ! myOpenModeller.run() )
  {
    QMessageBox::warning( this,
            "openModeller Wizard Error","Error running model!",
            myOpenModeller.error()
            );
    return;
  }

  // Prepare the output map
  myOpenModeller.createMap( myOpenModeller.getEnvironment() );
  //if all went ok, send notification to the parent app that we are finished
  emit drawRasterLayer(outputFileNameQString+QString(".tif"));
}
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
        myQTextStream << tr("Output file = ") << outputFileNameQString << ".tif\n";
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
          QMessageBox::warning( this,QString("openModeller Wizard Error"),QString("The model algorithm name is not specified!"));
          return;
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
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QSettings myQSettings;
  std::cout << "cboModelAlgorithm .. current text : " << cboModelAlgorithm->currentText() << std::endl;
  modelNameQString=cboModelAlgorithm->currentText();

  //
  // set the well known text coordinate string for the coordinate system that the point data are stored in
  //
  
  //make sure that you have no linefeeds and escape carriage returns in WKT string!
  if (cboCoordinateSystem->currentText()==tr("Lat/Long WGS84"))
  {
    coordinateSystemQString =  "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]";
  }
  else if (cboCoordinateSystem->currentText()==tr("Lat/Long 1924 Brazil"))
  {
    coordinateSystemQString =  "GEOGCS[\"1924 ellipsoid\", DATUM[\"Not_specified\", SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG\",\"7022\"]], AUTHORITY[\"EPSG","6022\"]], PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]], UNIT[\"degree\",0.0174532925199433, AUTHORITY[\"EPSG","9108\"]], AUTHORITY[\"EPSG","4022\"]]";
  }
  else if (cboCoordinateSystem->currentText()==tr("UTM Zone 22 - Datum: Corrego Alegre"))
  {
    coordinateSystemQString = "UTM Zone 22 - Datum: Corrego Alegre: PROJCS[\"UTM Zone 22, Southern Hemisphere\", GEOGCS[\"Datum Corrego Alegre\", DATUM[\"Datum Corrego Alegre\", SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG","7022\"]], AUTHORITY[\"EPSG\",\"6022\"]], PRIMEM[\"Greenwich\",0, AUTHORITY[\"EPSG\",\"8901\"]], UNIT[\"degree\",0.0174532925199433, AUTHORITY[\"EPSG\",\"9108\"]], AUTHORITY[\"EPSG\",\"4022\"]], PROJECTION[\"Transverse_Mercator\"], PARAMETER[\"latitude_of_origin\",0], PARAMETER[\"central_meridian\",-51], PARAMETER[\"scale_factor\",0.9996], PARAMETER[\"false_easting\",500000], PARAMETER[\"false_northing\",10000000], UNIT[\"METERS\",1]]";
  }
  else if (cboCoordinateSystem->currentText()==tr("Long/Lat - Datum: Corrego Alegre"))
  {
    coordinateSystemQString = "GEOGCS[\"Datum Corrego Alegre\", DATUM[\"Datum Corrego Alegre\", SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG\",\"7022\"]], AUTHORITY[\"EPSG\",\"6022\"]], PRIMEM[\"Greenwich\",0, AUTHORITY[\"EPSG\",\"8901\"]], UNIT[\"degree\",0.0174532925199433, AUTHORITY[\"EPSG\",\"9108\"]], AUTHORITY[\"EPSG\",\"4022\"]]";
  }
   
  
  
  //pull all the form data into local class vars.
  outputFileNameQString=leOutputFileName->text();
  myQSettings.writeEntry("/openmodeller/outputFileName",outputFileNameQString);
  localitiesFileNameQString=leLocalitiesFileName->text();
  myQSettings.writeEntry("/openmodeller/localitiesFileName",localitiesFileNameQString);
  //build up the map layers qstringlist
  layerNamesQStringList.clear();
  for ( unsigned int myInt = 0; myInt < lstLayers->count(); myInt++ )
    {
      QListBoxItem *myItem = lstLayers->item( myInt );
      layerNamesQStringList.append(myItem->text());      
    }
  myQSettings.writeEntry("/openmodeller/layerNames",layerNamesQStringList);
  // build up the model parameters qstringlist
  extraParametersQStringList.clear();
  for ( unsigned int myInt = 0; myInt < lstParameters->count(); myInt++ )
  {
     QListBoxItem *myItem = lstParameters->item( myInt );
     extraParametersQStringList.append(myItem->text());      
  }   
  
  maskNameQString=cboMaskLayer->currentText();
  taxonNameQString=cboTaxon->currentText();
  makeConfigFile();
  parseAndRun(outputFileNameQString+".cfg");
  
  QApplication::restoreOverrideCursor();
  //close the dialog
  done(1);
}

void OpenModellerGui::cboModelAlgorithm_activated(  const QString &theAlgorithmQString)
{
}


void OpenModellerGui::pbnSelectOutputFile_clicked()
{
  QSettings settings;
  std::cout << " OpenModellerGui::pbnSelectOutputFile_clicked() " << std::endl;
    QString myOutputFileNameQString = QFileDialog::getSaveFileName(
                    settings.readEntry("/openmodeller/outputFileName"), //initial dir
                    "All Files (*.)",
                    this,
                    "save file dialog"
                    "Choose a filename to save under" );
    leOutputFileName->setText(myOutputFileNameQString);
    
    //store output filename for use next time
    settings.writeEntry("/openmodeller/outputFileName",myOutputFileNameQString);
    
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
            //add the deleted item back to combo pick list
	    QStringList deletedItemList( QStringList::split( " ", lstParameters->currentText()));
	    cbxParameterType->insertItem(deletedItemList[0]);
	    cbxParameterType->setCurrentItem(0);
            //remove the item if it is selected
	    lstParameters->removeItem(myInt);
        }
    }
}


void OpenModellerGui::pbnAddParameter_clicked()
{
  QString myParameterString = "";
  
  //create parameter string 
  myParameterString = cbxParameterType->currentText() + " " + txtParameterValue->text() ;
  //remove parameter type from combo pick list
  cbxParameterType->removeItem(cbxParameterType->currentItem());
  //add parameter to selected list
  lstParameters->insertItem(myParameterString);
  //clear value box
  txtParameterValue->setText("");
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
  QSettings settings;
  std::cout << " OpenModellerGui::pbnSelectLocalitiesFile_clicked() " << std::endl;
  QString myFileTypeQString;
  QString myTextFileFilterString="Text File (*.txt)";
  QString myFileNameQString = QFileDialog::getOpenFileName(
  settings.readEntry("/openmodeller/localitiesFileDirectory"), //initial dir
  myTextFileFilterString,  //filters to select
  this , //parent dialog
  "OpenFileDialog" , //QFileDialog qt object name
  "Select localities text file" , //caption
  &myFileTypeQString //the pointer to store selected filter
 );  
  std::cout << "Selected filetype filter is : " << myFileTypeQString << std::endl;
  if (myFileNameQString==NULL || myFileNameQString=="") return;
  setSpeciesList(myFileNameQString);
  
  //store directory where localities file is for next time
  settings.writeEntry("/openmodeller/localitiesFileDirectory", myFileNameQString );
  
} //end of pbnSelectLocalitiesFile_clicked

void OpenModellerGui::setSpeciesList(QString theFileNameQString)
{
  //
  // Now that we have the localities text file, we need to parse it and find
  // all unique instances of taxon names and populate the taxon combo...
  //
  //first build a regex to match text at the beginning of the line
 QRegExp myQRegExp( "^[^#][ a-zA-Z]*" ); //seconf caret means 'not'
 QStringList myTaxonQStringList;;
 QFile myQFile( theFileNameQString );
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
      QMessageBox::warning( this,QString("openModeller Wizard Error"),QString("The localities file is not readable. Check you have the neccessary file permissions and try again."));      
      return; 
 }   
  // if all that went ok, update the form field and the class var  
  leLocalitiesFileName->setText(theFileNameQString);
  localitiesFileNameQString = theFileNameQString;
  //enable the user to carry on to the next page...
  setNextEnabled(currentPage(),true);
} //end of setSpeciesList

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


