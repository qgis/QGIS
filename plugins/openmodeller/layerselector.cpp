
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
#include "layerselector.h"

//qt includes
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qlistview.h>

//for the is valid gdal file functions
#include "openmodellergui.h"

//standard includes
#include <iostream>



LayerSelector::LayerSelector( QWidget* parent , const char* name , bool modal , WFlags fl  )
  : LayerSelectorBase( parent, name, modal, fl )
{
  QString myStartDirString = "/home/aps02ts/dev/cpp/sample_data/";
  listParent = new QListViewItem(listFileTree,myStartDirString);
  traverseDirectories(myStartDirString,listParent);
}

void LayerSelector::traverseDirectories(const QString& dirname, QListViewItem* theParentListViewItem)
{
  QDir dir(dirname);
  dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks );
  std::cout << "Current directory is: " << dirname.ascii() << std::endl;

  const QFileInfoList* fileinfolist = dir.entryInfoList();
  QFileInfoListIterator it(*fileinfolist);
  QFileInfo* fi;

  bool myInvalidFileFlag = false;
  bool myInvalidFileProjFlag = false;
  QString myInvalidFileList;
  QString myInvalidFileProjList;

  while( (fi = it.current() ) )
  {
    //Ignore directories
    if( fi->fileName() == "." || fi->fileName() == ".." ) 
    {
      ++it;
      continue;
    }

    //check to see if entry is a directory - if so iterate through it (recursive function)
    //a new tree node will be created each time
    if(fi->isDir() && fi->isReadable() )
    {
      traverseDirectories(fi->absFilePath(), new QListViewItem(theParentListViewItem,fi->absFilePath()));
    }

    //check to see if its an adf file type
    //only add the hdr.adf files to ensure multiple adf files from one directory aren't added
    else if (fi->extension(false)=="adf")
    {
      if (fi->fileName()=="hdr.adf")
      {
        std::cout << "Current filename is: " << fi->dirPath(true).ascii() << std::endl;
        QListViewItem myItem = new QListViewItem(theParentListViewItem,fi->dirPath(true));
      }
    }

    //check to see if entry is of the other required file types
    else if ((fi->extension(false)=="tif") ||
            (fi->extension(false)=="asc") ||
            (fi->extension(false)=="bil") ||
            (fi->extension(false)=="jpg")   )
    {      

      //test whether the file is GDAL compatible
      if (OpenModellerGui::isValidGdalFile(fi->absFilePath()) && OpenModellerGui::isValidGdalProj(fi->absFilePath()))
      {
        //GOOD FILE AND GOOD PROJ
	std::cout << fi->absFilePath().ascii() << " is a valid GDAL file and contains projection info" << std::endl;
        QListViewItem myItem = new QListViewItem(theParentListViewItem,fi->dirPath(true));
      }
      else if (OpenModellerGui::isValidGdalFile(fi->absFilePath()) && !OpenModellerGui::isValidGdalProj(fi->absFilePath()))
      {
        //GOOD FILE AND BAD PROJ
	std::cout << fi->absFilePath().ascii() << " is a valid GDAL file but contains no projection info" << std::endl;
        QListViewItem myItem = new QListViewItem(theParentListViewItem,fi->dirPath(true));
	myInvalidFileProjFlag = true;	  
	myInvalidFileProjList += fi->absFilePath()+"\n"; 	
      }
      else 
      {
        //BAD FILE AND/OR BAD PROJ
        myInvalidFileFlag = true;
        myInvalidFileList += fi->absFilePath()+"\n";         
      } 
    }  
  ++it;
  }
  
  if (myInvalidFileFlag)
  {
     //BAD FILE WARNING
     //QMessageBox::critical( this,QString("openModeller Wizard Error"),QString("The following are not valid GDAL files.  Please check and try again:\n\n "+myInvalidFileList));
  }
  else
  {
    if (myInvalidFileProjFlag)
    {
      //BAD PROJ WARNING 
      //QMessageBox::warning( this,QString("openModeller Wizard Error"),QString("Warning!! The following files do not have any projection information associated.\n\n "+myInvalidFileProjList));
    }
   
  }

}   
