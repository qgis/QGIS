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
#include <qdir.h> 
#include <qfileinfo.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qtooltip.h> 
#include <qprogressbar.h>
//
//openmodeller includes
#ifdef WIN32
  #include <om_control.hh>
  #include <om.hh>
  #include <om_alg_parameter.hh>
  #include <request_file.hh>
#else
  #include <openmodeller/om_control.hh>
  #include <openmodeller/om.hh>
  //gdal includes
  #include "gdal_priv.h"
#endif
  #include <om_alg_parameter.hh>
  #include <request_file.hh>


//standard includes
#include <stdlib.h>
#include <stdio.h>
#include <iostream>



    OpenModellerGui::OpenModellerGui()
: OpenModellerGuiBase()
{
  mOpenModeller = new OpenModeller();
  getAlgorithmList();
  mParametersScrollView = new QScrollView(frameParameters);
  mParametersVBox = new QVBox (mParametersScrollView->viewport());
  mParametersScrollView->addChild(mParametersVBox);
  mParametersFrame = new QFrame(mParametersVBox);
  //temporarily make a layout
  //mLayout = new QGridLayout(mParametersFrame,1,2);
  mLayout = new QGridLayout(frameParameters,1,2);
}

    OpenModellerGui::OpenModellerGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: OpenModellerGuiBase( parent, name, modal, fl )
{
  mOpenModeller = new OpenModeller();
  getAlgorithmList();
  mParametersScrollView = new QScrollView(frameParameters);
  mParametersVBox = new QVBox (mParametersScrollView->viewport());
  mParametersScrollView->addChild(mParametersVBox);
  mParametersFrame = new QFrame(mParametersVBox);
  //temporarily make a layout
  //mLayout = new QGridLayout(mParametersFrame,1,2);
  mLayout = new QGridLayout(frameParameters,1,2);
}

OpenModellerGui::~OpenModellerGui()
{
  delete mLayout;
  // DO ME!!
  //if (mMap!=NULL) delete mMap;
}

void OpenModellerGui::getAlgorithmList()
{

  // Find out which model algorithm is to be used.
  std::cerr << "-------------- openModeller plugin :  Reading algorithm list..." << std::endl;
  AlgMetadata **myAlgorithmMetadataArray = mOpenModeller->availableAlgorithms();
  AlgMetadata *myAlgorithmMetadata;
  //loop through the algorithm names adding to the algs combo
  while ( myAlgorithmMetadata = *myAlgorithmMetadataArray++ )
  {
    std::cerr << "Found Algorithm: " << myAlgorithmMetadata->id << std::endl;
    cboModelAlgorithm->insertItem(myAlgorithmMetadata->id);
  }     
  return ;

}

void OpenModellerGui::getParameterList( QString theAlgorithmNameQString )
{  
  std::cout <<"getParameterList called" << std::endl;

  // Find out which model algorithm is to be used.
  AlgMetadata **myAlgorithmsMetadataArray = mOpenModeller->availableAlgorithms();
  AlgMetadata *myAlgorithmMetadata;
  std::cerr << "-------------- openModeller plugin :  Reading algorithm list..." << std::endl;

  //find out how many params and clear maps
  int myRowCountInt = 0;
  while ( myAlgorithmMetadata = *myAlgorithmsMetadataArray++ )
  { 
    myRowCountInt++;
    //delete current parameter map contents
    ParametersMap::Iterator myIterator;

    for ( myIterator = mMap.begin(); myIterator != mMap.end(); ++myIterator ) 
    {
      delete myIterator.data();
      //mMap.remove(myIterator);
    }
    mMap.clear();

    ParameterLabels::Iterator myLabelIterator;
    for ( myLabelIterator = mLabelsMap.begin(); myLabelIterator != mLabelsMap.end(); ++myLabelIterator ) 
    {
      delete myLabelIterator.data();
      //mLabelsMap.remove(myIterator);
    }
    mLabelsMap.clear();

  }
  if (mLayout) 
  {
    std::cout << "mLayout exists so deleting" << std::endl;
    delete mLayout;
  }
  //mLayout = new QGridLayout(mParametersFrame,myRowCountInt+1,3); 
  mLayout = new QGridLayout(frameParameters,myRowCountInt+1,3); 
  //mLayout->setColSpacing(0,200);
  mLayout->setColSpacing(1,10);
  //mLayout->setColSpacing(2,50);
  //mLayout->setColSpacing(3,0);

  //reinitialise the metadataarray 
  myAlgorithmsMetadataArray = mOpenModeller->availableAlgorithms();

  //Buidling maps for widgets
  while ( myAlgorithmMetadata = *myAlgorithmsMetadataArray++ )
  {
    //Set fonts for parameter labels
    QString myFontName = "Arial [Monotype]";
    QFont myLabelFont(myFontName, 11, 75);

    // Scan openModeller algorithm list for the selected algorithm
    QString myAlgorithmNameQString=myAlgorithmMetadata->id;

    if (myAlgorithmNameQString==theAlgorithmNameQString)
    {
      int myParameterCountInt = myAlgorithmMetadata->nparam;
      AlgParamMetadata * myParameter = myAlgorithmMetadata->param;

      if (myParameterCountInt==0)
      {
        //Set label and button for algorithms with no parameters
        lblParameters->setText("No user definable parameters available");
        //pbnDefaultParameters->setEnabled(false);
      }
      else
      {
        //Set label and button for algorithms with parameters
        lblParameters->setText("Algorithm specific parameters");
        //pbnDefaultParameters->setEnabled(true);

        QSettings settings;

        //interate through parameters adding the correct control type
        for ( int i = 0; i < myParameterCountInt; i++, myParameter++ )
        {
          QString myParameterType(myParameter->type);
          std::cout << "Parameter " << QString(myParameter->name).ascii() << " is a " << myParameterType.ascii() << std::endl;

          if (myParameterType=="Integer")
          {
            //Create a spinbox for integer values
            std::cout << QString(myParameter->id).ascii() << " parameter is integer type" << std::endl;

            //Create components
            QSpinBox * mySpinBox = new QSpinBox (frameParameters, ("spin"+QString(myParameter->id)));
            QLabel * myLabel = new QLabel (frameParameters, ("lbl"+QString(myParameter->id)));

            //set spinbox details and write to map
            if (!myParameter->has_min==0) mySpinBox->setMinValue(myParameter->min);
            if (!myParameter->has_max==0) mySpinBox->setMaxValue(myParameter->max);

            //Set value to previous otherwise to default
            QString myPreviousValue = settings.readEntry("/openmodeller/"+cboModelAlgorithm->currentText()+"/"+myParameter->id);
            if (myPreviousValue)
            {
              mySpinBox->setValue(atoi(myPreviousValue));
            }
            else
            {
              mySpinBox->setValue(atoi(myParameter->typical));
            }

            QToolTip::add(mySpinBox, myParameter->description);

            //Set label details and write to map
            myLabel->setText(myParameter->name);	   
            myLabel->setFont(myLabelFont);

            //add label and control to form
            mLayout->addWidget(myLabel, i, 0);
            mLayout->addWidget(mySpinBox, i, 2);
            mLayout->setRowSpacing(i,30);

            //
            // Add the widget to the map
            //
            mMap[myParameter->id] = mySpinBox;
            mLabelsMap[myParameter->name] = myLabel;

          }
          else if (myParameterType.compare("Real") || myParameterType.compare("Double"))
          {
            std::cout << QString (myParameter->id).ascii() << " parameter is " << myParameterType.ascii() 
                      << " type" << std::endl;

            //Create components
            QLineEdit * myLineEdit = new QLineEdit (frameParameters, ("le"+QString(myParameter->id)));
            QLabel * myLabel = new QLabel (frameParameters, ("lbl"+QString(myParameter->id)));	

            //Set value to previous otherwise to default
            QString myPreviousValue = settings.readEntry("/openmodeller/"+cboModelAlgorithm->currentText()+"/"+myParameter->id);
            if (myPreviousValue)
            {
              myLineEdit->setText(myPreviousValue);
            }
            else
            {
              myLineEdit->setText(myParameter->typical);
            }

            QToolTip::add(myLineEdit, myParameter->description);

            //Set label details and write to map
            myLabel->setText(myParameter->name);	
            myLabel->setFont(myLabelFont);   

            //add label and control to form
            mLayout->addWidget(myLabel, i,0);
            mLayout->addWidget(myLineEdit,i,2);
            mLayout->setRowSpacing(i,30);
            myLineEdit->show();

            //
            // Add the widget to the map
            //
            mMap[myParameter->id] = myLineEdit;
            mLabelsMap[myParameter->name] = myLabel;
          }
        }	
      }     
      //Exit loop because we have found the correct algorithm
      break;      
    }
  }
  return; 
}

/** This is the page selected event which I am reimplementing to do some housekeeping
 *    between wizard pages. 
 *    This routine is a slot that runs each time next is pressed  */
void OpenModellerGui::formSelected(const QString &thePageNameQString)
{
  std::cout << thePageNameQString.ascii() << " has focus " << std::endl;
  QString myQString;
  QSettings settings;
  if (thePageNameQString==tr("Step 1 of 9")) //we do this when arriving at the mode selection page
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
  if (thePageNameQString==tr("Step 2 of 9")) //we do this after leaving the file selection page
  {
    settings.writeEntry("/openmodeller/modelName",cboModelAlgorithm->currentText());
    getParameterList(cboModelAlgorithm->currentText());

    getProjList();
    std::cout <<"Proj list built"<<std::endl;
    //set the coordinate system to the same as the last time run
    QString myCoordSystem = settings.readEntry("/openModeller/coordSystem");
    if (myCoordSystem=="")
    {
      //default to lat ling
      cboCoordinateSystem->setCurrentText("Lat/Long WGS84");
    }
    else
    {
      cboCoordinateSystem->setCurrentText(tr(myCoordSystem));
    }
    std::cout << "step 2 ready" << std::endl;

  }
  if (thePageNameQString==tr("Step 3 of 9")) //we do this after leaving the file selection page
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
  if (thePageNameQString==tr("Step 4 of 9")) 
  {  
  //MODEL CREATION LAYERSET
  
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
  if (thePageNameQString==tr("Step 5 of 9"))
  {
  //MODEL PROJECTION LAYERSET
  
    const QString myProjFileNameQString =  settings.readEntry("/openmodeller/projectionLayerNames");
    //tokenise the setting list (its separated by ^e)
    const QString myProjSeparatorQString = "^e";
    QStringList myProjFileNameQStringList =  QStringList::split (myProjSeparatorQString, myProjFileNameQString, false ); 
    //only try to restore the list of layers used in the last session if the list is empty
    if (myProjFileNameQStringList.size() > 0 && lstProjLayers->count()==0)
    {
      //loop through the layer names
      QStringList::Iterator myProjIterator= myProjFileNameQStringList.begin();
      QString myProjLastFileNameQString="";
      while( myProjIterator!= myProjFileNameQStringList.end() ) 
      {
        QString myProjFileNameQString=*myProjIterator;
        if (myProjFileNameQString!=myProjLastFileNameQString)
        {
          lstProjLayers->insertItem(myProjFileNameQString);
	  //also add the layer to the mask combo
          cboOutputMaskLayer->insertItem(myProjFileNameQString);
        }
        myProjLastFileNameQString=*myProjIterator;
        ++myProjIterator;
      }     
      //enable the user to carry on to the next page...
      setNextEnabled(currentPage(),true);
    }
    if ( lstProjLayers->count()==0)
    {
      setNextEnabled(currentPage(),false);
    }  
    else
    {
      setNextEnabled(currentPage(),true);
    }
  
  
  }


  if (thePageNameQString==tr("Step 8 of 9")) 
  {  
    //
    // Extract parameters and their values from map and add to QStringList ready for reading 
    //

    //Clear any previous parameters from list
    extraParametersQStringList.clear();

    //Get the algorithm parameters and store in QStringList
    ParametersMap::Iterator myIterator;
    for ( myIterator = mMap.begin(); myIterator != mMap.end(); ++myIterator ) 
    {
      std::cout << "Widget  " <<myIterator.key().ascii() << " : ";
      QString myWidgetName = myIterator.data()->name();
      std::cout << myWidgetName << std::endl;
      QString myValueString = "";
      if (myWidgetName.left(2)=="le")
      {
        QLineEdit * myLineEdit = (QLineEdit*) myIterator.data();
        myValueString = myLineEdit->text();
        settings.writeEntry(
                "/openmodeller/"+cboModelAlgorithm->currentText()+"/"+myIterator.key(),
                myLineEdit->text());
      }
      else if (myWidgetName.left(4)=="spin")
      {
        QSpinBox * mySpinBox = (QSpinBox*) myIterator.data();
        myValueString = QString::number(mySpinBox->value());
        settings.writeEntry(
                "/openmodeller/"+cboModelAlgorithm->currentText()+"/"+myIterator.key(),
                mySpinBox->value());
      }

      extraParametersQStringList.append(myIterator.key() + " " + myValueString);
    }

    //
    // Set up page widgets
    //

    leOutputDirectory->setText(settings.readEntry("/openmodeller/outputDirectory"));

    if ( leOutputFileName->text()=="")
    {
      setNextEnabled(currentPage(),false);
    }  
    else
    {
      setNextEnabled(currentPage(),true);
    }
  } 

  else if (thePageNameQString==tr("Step 9 of 9"))
  {

    QSettings myQSettings;

    //pull all the form data into local class vars.
    outputFileNameQString=leOutputDirectory->text()+"/"+leOutputFileName->text();
    myQSettings.writeEntry("/openmodeller/fullOutputFileName",outputFileNameQString);
    localitiesFileNameQString=leLocalitiesFileName->text();
    myQSettings.writeEntry("/openmodeller/localitiesFileName",localitiesFileNameQString);

    //
    // Check the specified output filename and directory are valid before proceeding
    //
    QFile myQFile( outputFileNameQString+".cfg");
    if ( myQFile.open( IO_WriteOnly ) ) 
    {
      //Filename is valid
      std::cout << "Filename '" << outputFileNameQString.ascii() << "' is valid" << std::endl;
      settings.writeEntry("/openmodeller/outputDirectory",leOutputDirectory->text());
      setFinishEnabled(currentPage(),true);
    }
    else
    {
      //Filename is invalid so switch back to previous page and warn user
      showPage(QWizard::page(7));
      std::cout << "Filename '" << outputFileNameQString.ascii() << "' is invalid" << std::endl;
      QMessageBox::warning(this,"Error opening output file!","The output filename and/or directory you specified is invalid.\n Please check and try again.");

    }

  }
}


/**  This method is used when you already have a valid model paramters text file
  and want to run it without interacting with the user. 
  @see pbnRun method which is run when model inputs have been obtained via the wizard. */
void OpenModellerGui::parseAndRun(QString theParametersFileNameQString)
{

  RequestFile myRequestFile;
  int myResult = myRequestFile.configure( mOpenModeller, strdup(theParametersFileNameQString));

  if ( myResult < 0 )
  {
    //notify user of eror here!
    QMessageBox::warning( this,"openModeller Wizard Error","Error reading request file!");
    return;
  }

  //tell oM to use our locally made callback fn
  mOpenModeller->setModelCallback( mapCallback, creationProgressBar);
  mOpenModeller->setMapCallback( mapCallback, projectionProgressBar);

  if ( ! mOpenModeller->run() )
  {
    QMessageBox::warning( this,
            "openModeller Wizard Error","Error running model!",
            mOpenModeller->error()
            );
    return;
  }

  // Prepare the output map
  //if ( ! mOpenModeller->createMap( mOpenModeller->getEnvironment() ) )
  //when projecting model into a different dataset there should be no parameter passed
  if ( ! mOpenModeller->createMap( ) )
  {
    QMessageBox::warning( this,
            "openModeller Wizard Error","Error projecting model!",
            mOpenModeller->error()
            );
    return;
  }
  
  //if all went ok, send notification to the parent app that we are finished
  emit drawRasterLayer(outputFileNameQString+QString(".tif"));
}
void OpenModellerGui::makeConfigFile()
{
  QFile myQFile( outputFileNameQString+".cfg");
  std::cout << "Config file name: " << outputFileNameQString.ascii() << std::endl;
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
    myQTextStream << tr("# A layer that specifies the region of interest for model building\n");      
    myQTextStream << tr("Mask = ") << maskNameQString << "\n";
    myQTextStream << tr("\n\n##\n");
    myQTextStream << tr("# A layer that specifies the region of interest for model projection\n");      
    myQTextStream << tr("Output Mask = ") << outputMaskNameQString << "\n";
    myQTextStream << tr("\n\n##\n");                             
    myQTextStream << tr("## Model Output Settings\n");
    myQTextStream << tr("##\n\n");   
     
    myQTextStream << tr("## Map projection layers)\n");     
    // Iterate through the items in the projection layers list
    for ( QStringList::Iterator myIterator = projLayerNamesQStringList.begin(); myIterator != projLayerNamesQStringList.end(); ++myIterator)
    {          
      myQTextStream << tr("Output map = ") << *myIterator << "\n";
    }
    
    
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
  std::cout << "cboModelAlgorithm .. current text : " << cboModelAlgorithm->currentText().ascii() << std::endl;
  modelNameQString=cboModelAlgorithm->currentText();

  //
  // set the well known text coordinate string for the coordinate system that the point data are stored in
  //

  coordinateSystemQString = mProjectionsMap[cboCoordinateSystem->currentText()];

  //build up the map layers qstringlist
  layerNamesQStringList.clear();
  for ( unsigned int myInt = 0; myInt < lstLayers->count(); myInt++ )
  {
    QListBoxItem *myItem = lstLayers->item( myInt );
    layerNamesQStringList.append(myItem->text());      
  }
  myQSettings.writeEntry("/openmodeller/layerNames",layerNamesQStringList);

  //build up the projection layers qstringlist
  projLayerNamesQStringList.clear();
  for ( unsigned int myInt = 0; myInt < lstProjLayers->count(); myInt++ )
  {
    QListBoxItem *myItem = lstProjLayers->item( myInt );
    projLayerNamesQStringList.append(myItem->text());      
  }
  myQSettings.writeEntry("/openmodeller/projectionLayerNames",layerNamesQStringList);  
  
  
  maskNameQString=cboMaskLayer->currentText();
  outputMaskNameQString=cboOutputMaskLayer->currentText();
  taxonNameQString=cboTaxon->currentText();
  makeConfigFile();
  parseAndRun(outputFileNameQString+".cfg");

  QApplication::restoreOverrideCursor();
  //close the dialog
  //done(1);
  
  
}

void OpenModellerGui::cboModelAlgorithm_activated(  const QString &theAlgorithmQString)
{
}


void OpenModellerGui::pbnSelectOutputDirectory_clicked()
{
  QSettings settings;
  std::cout << " OpenModellerGui::pbnSelectOutputFile_clicked() " << std::endl;

  QString myOutputDirectoryQString = QFileDialog::getExistingDirectory(
          settings.readEntry("/openmodeller/outputDirectory"), //initial dir
          this,
          "get existing directory",
          "Choose a directory",
          TRUE );
  leOutputDirectory->setText(myOutputDirectoryQString);



}


void OpenModellerGui::pbnRemoveLayerFile_clicked()
{  
  int myLayersCount = lstLayers->count();

  for ( unsigned int myInt = 0; myInt < myLayersCount; myInt++ )
  {
    QListBoxItem *myItem = lstLayers->item( myInt );
    // if the item is selected...
    if ( myItem->isSelected() )
    {
      //remove the item if it is selected
      lstLayers->removeItem(myInt);
      //also remove the item from the mask layer combo
      cboMaskLayer->removeItem(myInt);
      myInt--;
      myLayersCount--;
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
  std::cout << "Selected filetype filter is : " << myFileTypeQString.ascii() << std::endl;
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
  std::cout << "Selected filetype filter is : " << myFileTypeQString.ascii() << std::endl;
  if (myFileNameQString==NULL || myFileNameQString=="") return;
  setSpeciesList(myFileNameQString);

  //store directory where localities file is for next time
  settings.writeEntry("/openmodeller/localitiesFileDirectory", myFileNameQString );

} //end of pbnSelectLocalitiesFile_clicked

void OpenModellerGui::getProjList()
{
  //first some hard coded options 
  mProjectionsMap["Lat/Long WGS84"] = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]";
  mProjectionsMap["Lat/Long 1924 Brazil"] =  "GEOGCS[\"1924 ellipsoid\", DATUM[\"Not_specified\", SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG\",\"7022\"]], AUTHORITY[\"EPSG","6022\"]], PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]], UNIT[\"degree\",0.0174532925199433, AUTHORITY[\"EPSG","9108\"]], AUTHORITY[\"EPSG","4022\"]]";
  mProjectionsMap["UTM Zone 22 - Datum: Corrego Alegre"] = "UTM Zone 22 - Datum: Corrego Alegre: PROJCS[\"UTM Zone 22, Southern Hemisphere\", GEOGCS[\"Datum Corrego Alegre\", DATUM[\"Datum Corrego Alegre\", SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG","7022\"]], AUTHORITY[\"EPSG\",\"6022\"]], PRIMEM[\"Greenwich\",0, AUTHORITY[\"EPSG\",\"8901\"]], UNIT[\"degree\",0.0174532925199433, AUTHORITY[\"EPSG\",\"9108\"]], AUTHORITY[\"EPSG\",\"4022\"]], PROJECTION[\"Transverse_Mercator\"], PARAMETER[\"latitude_of_origin\",0], PARAMETER[\"central_meridian\",-51], PARAMETER[\"scale_factor\",0.9996], PARAMETER[\"false_easting\",500000], PARAMETER[\"false_northing\",10000000], UNIT[\"METERS\",1]]";
  mProjectionsMap["Long/Lat - Datum: Corrego Alegre"] = "GEOGCS[\"Datum Corrego Alegre\", DATUM[\"Datum Corrego Alegre\", SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG\",\"7022\"]], AUTHORITY[\"EPSG\",\"6022\"]], PRIMEM[\"Greenwich\",0, AUTHORITY[\"EPSG\",\"8901\"]], UNIT[\"degree\",0.0174532925199433, AUTHORITY[\"EPSG\",\"9108\"]], AUTHORITY[\"EPSG\",\"4022\"]]";

  std::cout << "Getting proj list " << std::endl;
#ifdef WIN32
  QString theFileNameQString = "wkt_defs.txt";
#else
  QString theFileNameQString = QGISDATAPATH;
  theFileNameQString += "/wkt_defs.txt";
#endif
  
  QFile myQFile( theFileNameQString );
  if ( myQFile.open( IO_ReadOnly ) ) 
  {
    //clear the existing entries in the taxon combo first
    //cboCoordinateSystem->clear();     
    //now we parse the loc file, checking each line for its taxon
    QTextStream myQTextStream( &myQFile );
    QString myCurrentLineQString;
    QStringList myQStringList;
    while ( !myQTextStream.atEnd() ) 
    {
      myCurrentLineQString = myQTextStream.readLine(); // line of text excluding '\n'
      if (myCurrentLineQString.left(4)!="PROJ")
      {
        QString myNextLineQString = myQTextStream.readLine(); // lthis is the actual wkt string
        if (myNextLineQString.left(4)!="PROJ") //the line shoue start with PROJ
        {
          continue;
        }
#ifdef QGISDEBUG
        std::cout << " Match found:" << myCurrentLineQString.ascii() << std::endl;
#endif
        mProjectionsMap[myCurrentLineQString]=myNextLineQString;
      }
    }
    myQFile.close();
    //no add each key to our combo
    ProjectionWKTMap::Iterator myIterator;
    for ( myIterator = mProjectionsMap.begin(); myIterator != mProjectionsMap.end(); ++myIterator ) 
    {
      //std::cout << "Widget map has: " <<myIterator.key().ascii() << std::endl;
      cboCoordinateSystem->insertItem(myIterator.key());
    }
  }
  else
  {
    QMessageBox::warning( this,QString("openModeller Wizard Error"),QString("The projections file is not readable. Check you have the neccessary file permissions and try again. Only a small list of projectsion is now availiable."));      
    ProjectionWKTMap::Iterator myIterator;
    for ( myIterator = mProjectionsMap.begin(); myIterator != mProjectionsMap.end(); ++myIterator ) 
    {
      //std::cout << "Widget map has: " <<myIterator.key().ascii() << std::endl;
      cboCoordinateSystem->insertItem(myIterator.key());
    }
  }   

}

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

void OpenModellerGui::cboModelAlgorithm_highlighted( const QString &theModelAlgorithm )
{
  AlgMetadata **myAlgorithmsMetadataArray = mOpenModeller->availableAlgorithms();
  AlgMetadata *myAlgorithmMetadata;

  while ( myAlgorithmMetadata = *myAlgorithmsMetadataArray++ )
  {
    QString myAlgorithmNameQString=myAlgorithmMetadata->id;
    if (myAlgorithmNameQString==theModelAlgorithm)
    {
      txtAlgorithm->setText(myAlgorithmMetadata->name);
      txtVersion->setText(myAlgorithmMetadata->version);
      txtAuthor->setText(myAlgorithmMetadata->author);
      txtAlgorithmDescription->setText(myAlgorithmMetadata->description);
      txtBibliography->setText(myAlgorithmMetadata->biblio);
    }
  }
}

void OpenModellerGui::leOutputFileName_textChanged( const QString &theOutputFileName)
{
  QSettings settings;

  //store output filename for use next time
  settings.writeEntry("/openmodeller/outputFileName",theOutputFileName);    

  //check whether next button should be enabled or disabled
  if ( !leOutputFileName->text().isEmpty() && !leOutputDirectory->text().isEmpty() )
  {
    setNextEnabled(currentPage(),true);
  }  
  else
  {
    setNextEnabled(currentPage(),false);  
  }

}

void OpenModellerGui::leOutputDirectory_textChanged( const QString &theOutputDirectory)
{
  QSettings settings;

  //store output directory for use next time
  settings.writeEntry("/openmodeller/outputDirectory",theOutputDirectory);    

  //check whether next button should be enabled or disabled
  if ( !leOutputFileName->text().isEmpty() && !leOutputDirectory->text().isEmpty() )
  {
    setNextEnabled(currentPage(),true);
  }  
  else
  {
    setNextEnabled(currentPage(),false);  
  }
}

void OpenModellerGui::pbnSelectLayerFolder_clicked()
{
  QSettings settings;

  QString myLayersDirectoryQString = QFileDialog::getExistingDirectory(
          settings.readEntry("/openmodeller/layersDirectory"), //initial dir
          this,
          "get existing directory",
          "Choose a directory",
          TRUE );

  if (myLayersDirectoryQString==NULL || myLayersDirectoryQString=="") return;

  settings.writeEntry("/openmodeller/layersDirectory",myLayersDirectoryQString);
  traverseDirectories(myLayersDirectoryQString, lstLayers, cboMaskLayer);



  //lstLayers->insertItem(myFileNameQString);
  //also add the layer to the mask combo
  //cboMaskLayer->insertItem(myFileNameQString);
  //enable the user to carry on to the next page...
  setNextEnabled(currentPage(),true);	    
}

void OpenModellerGui::traverseDirectories(const QString& dirname, QListBox* theListBox, QComboBox* theComboBox)
{
  QDir dir(dirname);
  dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks );
  std::cout << "Current directory is: " << dirname.ascii() << std::endl;

  const QFileInfoList* fileinfolist = dir.entryInfoList();
  QFileInfoListIterator it(*fileinfolist);
  QFileInfo* fi;

  bool myInvalidFileFlag = false;
  bool myInvalidProjectionFlag = false;
  QString myInvalidFileList;

  while( (fi = it.current() ) )
  {
    if( fi->fileName() == "." || fi->fileName() == ".." ) 
    {
      ++it;
      continue;
    }

    //check to see if entry is a directory - if so iterate through it
    if(fi->isDir() && fi->isReadable() )
    {
      traverseDirectories(fi->absFilePath(), theListBox, theComboBox);
    }

    //check to see if its an adf file type
    //only add the hdr.adf files to ensure multiple adf files from one directory aren't added
    else if (fi->extension(false)=="adf")
    {
      if (fi->fileName()=="hdr.adf")
      {
        std::cout << "Current filename is: " << fi->dirPath(true).ascii() << std::endl;
        theListBox->insertItem(fi->dirPath(true));
        theComboBox->insertItem(fi->dirPath(true)); 
      }
    }

    //check to see if entry is of the other required file types
    else if ((fi->extension(false)=="tif") ||
            (fi->extension(false)=="asc") ||
            (fi->extension(false)=="bil") ||
            (fi->extension(false)=="jpg")   )
    {      
#ifdef WIN32
 //dont test layers with gdal
#else
      //test whether the file is GDAL compatible

      GDALAllRegister();
      GDALDataset * myTestFile = (GDALDataset *)GDALOpen( fi->absFilePath(), GA_ReadOnly );

      if( myTestFile == NULL )
      {
        //not GDAL compatible
        myInvalidFileFlag = true;
      }
      else
      {
        std::cout << "GDAL opened " << fi->absFilePath().ascii() << " successfully" << std::endl;

        //is GDAL compatible
        //check whether it has projection information
        if (myTestFile->GetProjectionRef()!=NULL)
        {
          //does not have projection info
          myInvalidProjectionFlag = true;
          myInvalidFileList += fi->absFilePath()+"\n";
        }
        else
        {
          //does have projection info
          std::cout << "Current filename is: " << fi->absFilePath().ascii() << std::endl;
          theListBox->insertItem(fi->absFilePath());
          theComboBox->insertItem(fi->absFilePath());
          theComboBox->setCurrentItem(0);	  
        }

        GDALClose(myTestFile);
      }  
#endif
    }
    ++it;
  }
  /*
  //FOLLOWING WARNING MESSAGES REMOVED AS THEY BECOME ANNOYING!  
  if (myInvalidFileFlag)
  {
  QMessageBox::warning( this,QString("openModeller Wizard Error"),QString("One or more of the files you selected are not valid GDAL raster layers.  Please check and try again."));
  }
  */
  if (myInvalidProjectionFlag)
  {
    QMessageBox::warning( this,QString("openModeller Wizard Error"),QString("One or more of the files you selected do not have valid GDAL projection information.  Please check and try again:\n\n "+myInvalidFileList));
  }

}   


bool OpenModellerGui::checkLocalitiesFileFormat(const QString)
{
 //@todo DO ME!!
 return false;
}

void OpenModellerGui::mapCallback( float progress, void *extra_param )
{
  QProgressBar *myProgressBar = (QProgressBar *) extra_param;
  //std::cout << "OMGUI : Map creation progress : " << ( 100 * progress ) << std::endl;
  myProgressBar->setProgress(100 * progress);
  //process events so gui doesnt block...
  qApp->processEvents();
}


void OpenModellerGui::pbnDefaultParameters_clicked()
{

  std::cout << "Setting defaults" << std::endl;

  ParametersMap::Iterator myIterator;
  for ( myIterator = mMap.begin(); myIterator != mMap.end(); ++myIterator ) 
  {
    QString myWidgetName = myIterator.data()->name();
    std::cout << myWidgetName.ascii() << std::endl;
    if (myWidgetName.left(2)=="le")
    {
      //QLineEdit * myLineEdit = (QLineEdit*) myIterator.data();
      //myLineEdit->setText(myParameter->typical());

    }


    else if (myWidgetName.left(4)=="spin")
    {
      //QSpinBox * mySpinBox = (QSpinBox*) myIterator.data();
      //mySpinBox->setValue(atoi(myIterator.data()->typical()));
      //mySpinBox->setValue(5);
    }

  }

}

void OpenModellerGui::pbnSelectLayerFileProj_clicked()
{
  std::cout << " OpenModellerGui::pbnSelectLayerFileProj_clicked() " << std::endl;
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
  std::cout << "Selected filetype filter is : " << myFileTypeQString.ascii() << std::endl;
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
  lstProjLayers->insertItem(myFileNameQString);
  //also add the layer to the mask combo
  cboOutputMaskLayer->insertItem(myFileNameQString);
  //enable the user to carry on to the next page...
  setNextEnabled(currentPage(),true);
}

void OpenModellerGui::pbnSelectLayerFolderProj_clicked()
{
  QSettings settings;

  QString myLayersDirectoryQString = QFileDialog::getExistingDirectory(
          settings.readEntry("/openmodeller/projectionLayersDirectory"), //initial dir
          this,
          "get existing directory",
          "Choose a directory",
          TRUE );

  if (myLayersDirectoryQString==NULL || myLayersDirectoryQString=="") return;

  settings.writeEntry("/openmodeller/projectionLayersDirectory",myLayersDirectoryQString);
  traverseDirectories(myLayersDirectoryQString, lstProjLayers, cboOutputMaskLayer);



  //lstLayers->insertItem(myFileNameQString);
  //also add the layer to the mask combo
  //cboMaskLayer->insertItem(myFileNameQString);
  //enable the user to carry on to the next page...
  setNextEnabled(currentPage(),true);	    
}

void OpenModellerGui::pbnRemoveLayerFileProj_clicked()
{  
  int myLayersCount = lstProjLayers->count();

  for ( unsigned int myInt = 0; myInt < myLayersCount; myInt++ )
  {
    QListBoxItem *myItem = lstProjLayers->item( myInt );
    // if the item is selected...
    if ( myItem->isSelected() )
    {
      //remove the item if it is selected
      lstProjLayers->removeItem(myInt);
      //also remove the item from the mask layer combo
      cboOutputMaskLayer->removeItem(myInt);
      myInt--;
      myLayersCount--;
    }
  }
  //if user has removed last list entry, disable next button
  if ( lstProjLayers->count()==0)
  {
    setNextEnabled(currentPage(),false);
  }

}
