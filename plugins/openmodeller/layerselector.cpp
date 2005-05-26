
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
#include <qlabel.h>
#include <qdir.h>
//for the is valid gdal file functions
#include "openmodellergui.h"

//standard includes
#include <iostream>

#include "gdal_priv.h"


LayerSelector::LayerSelector( QString theBaseDir, QWidget* parent , const char* name , bool modal , WFlags fl  )
  : LayerSelectorBase( parent, name, modal, fl )
{
  if (!theBaseDir)
  {
    baseDirString ="";
  }
  else
  {
    baseDirString = theBaseDir;
  }
    
  listFileTree->setRootIsDecorated(true);
  listFileTree->setColumnWidthMode(0,QListView::Maximum);
  listFileTree->setColumnWidth(0,300);
  listFileTree->setColumnWidth(1,30);
  listFileTree->setColumnWidth(2,30);
  //listFileTree->setResizeMode(QListView::AllColumns);
  QFileInfo myFileInfo(baseDirString);
  if ( myFileInfo.exists() && !baseDirString.isEmpty())
  {
    std::cout << "Provided directory found - traversing subdirectories" << std::endl;
    lblBaseDir->setText(tr("Base Dir: ") + baseDirString);
    listFileTree->clear();
    listParent = new QListViewItem(listFileTree,baseDirString);
    traverseDirectories(baseDirString,listParent);
    listParent->setOpen(true);
    listFileTree->triggerUpdate();
  }
  else
  {
    std::cout << "Provided directory does not exist - prompting for directory" << std::endl;
    pbnDirectorySelector_clicked();
  }
}

void LayerSelector::pbnDirectorySelector_clicked()
{

  baseDirString = QFileDialog::getExistingDirectory(
          baseDirString, //initial dir
          this,
          "get existing directory",
          "Choose a directory",
          TRUE );
  lblBaseDir->setText(tr("Base Dir: ") + baseDirString);
  listFileTree->clear();
  listParent = new QListViewItem(listFileTree,baseDirString);
  traverseDirectories(baseDirString,listParent);
  listParent->setOpen(true);
  listFileTree->setResizeMode(QListView::AllColumns);
  listFileTree->triggerUpdate();
  
}




void LayerSelector::pbnOK_clicked()
{

  selectedLayersList.clear();
  QListViewItemIterator myIterator (listParent);
  while (myIterator.current())
  {
    //Check whether the item is selected and that it is not a directory or the tree root
    if ((myIterator.current()->isSelected()) && !((myIterator.current()->text(1)=="DIR") || (myIterator.current()->text(1)=="")))
    {
      if (myIterator.current()->text(1)=="AIG")
      {
        selectedLayersList+=myIterator.current()->text(0);
      }
      else
      {        
        selectedLayersList+=(myIterator.current()->parent()->text(0) + "/" +myIterator.current()->text(0));
      }
    }
    ++myIterator;
  }
  accept();

}


void LayerSelector::pbnCancel_clicked()
{
  selectedLayersList.clear();
  accept();
}



void LayerSelector::traverseDirectories(const QString& theDirName, QListViewItem* theParentListViewItem)
{
  if (!theDirName) return;
  if (!theParentListViewItem) return;
  if (theDirName.isEmpty()) return;
  std::cout << "Recursing into : " << theDirName << std::endl;
  QDir myDirectory(theDirName);
  myDirectory.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks );
  std::cout << "Current directory is: " << theDirName.ascii() << std::endl;

  const QFileInfoList* myFileInfoList = myDirectory.entryInfoList();
  QFileInfoListIterator myIterator(*myFileInfoList);
  QFileInfo*myFileInfo;

  bool myInvalidFileFlag = false;
  bool myInvalidFileProjFlag = false;
  QString myInvalidFileList;
  QString myInvalidFileProjList;

  while( (myFileInfo = myIterator.current() ) )
  {
    //Ignore directories
    if(myFileInfo->fileName() == "." ||myFileInfo->fileName() == ".." ) 
    {
      ++myIterator;
      continue;
    }

    //check to see if entry is a directory - if so iterate through it (recursive function)
    //a new tree node will be created each time
    if(myFileInfo->isDir() && myFileInfo->isReadable() )
    {
      QListViewItem * myItem = new QListViewItem(theParentListViewItem,myFileInfo->absFilePath());
      myItem->setText(1,"DIR");
      traverseDirectories(myFileInfo->absFilePath(),myItem );
      //if it turned out the dir contained an arc info coverage, the 'DIR' label above will have been changed
      //to AIG in the above travers call. Now we can test if this item has no child nodes and is not named AIG
      //we can prune it out of the tree
      if ((myItem->childCount()<1) && (myItem->text(1) !="AIG"))
      {
        delete myItem;
      }
      else
      {
        myItem->setOpen(true);
      }
    }

    //check to see if its an adf file type
    //only add the hdr.adf files to ensure multiple adf files from one directory aren't added
    else if (myFileInfo->extension(false)=="adf")
    {
      if (myFileInfo->fileName()=="hdr.adf")
      {
        std::cout << "Current filename is: " <<myFileInfo->filePath().ascii() << std::endl;
        //set the parent item as the layer because this is a coverage
        theParentListViewItem->setText(1,"AIG");

        if (isValidGdalProj(myFileInfo->absFilePath()))
        {
          theParentListViewItem->setText(2,"Valid");
        }
        else
        {
          theParentListViewItem->setText(2,"Invalid");
        }
        //dont scan this dir any further (we assume there are not coverages nested inside coverages)!
        return;
      }
    }

    //check to see if entry is of the other required file types
    else if ((myFileInfo->extension(false)=="tif") ||
            (myFileInfo->extension(false)=="asc") ||
            (myFileInfo->extension(false)=="bil") ||
            (myFileInfo->extension(false)=="jpg")   )
    {      

      //test whether the file is GDAL compatible
      if (isValidGdalFile(myFileInfo->absFilePath()) && isValidGdalProj(myFileInfo->absFilePath()))
      {
        //GOOD FILE AND GOOD PROJ
        std::cout <<myFileInfo->absFilePath().ascii() << " is a valid GDAL file and contains projection info" << std::endl;
        QListViewItem * myItem = new QListViewItem(theParentListViewItem,myFileInfo->fileName());
        myItem->setText(1,myFileInfo->extension(false));
        myItem->setText(2,"Valid");
      }
      else if (isValidGdalFile(myFileInfo->absFilePath()) && !isValidGdalProj(myFileInfo->absFilePath()))
      {
        //GOOD FILE AND BAD PROJ
        std::cout <<myFileInfo->absFilePath().ascii() << " is a valid GDAL file but contains no projection info" << std::endl;
        QListViewItem * myItem = new QListViewItem(theParentListViewItem,myFileInfo->fileName());
        myItem->setText(1,myFileInfo->extension(false));
        myItem->setText(2,"Invalid");
        myInvalidFileProjFlag = true;	  
        myInvalidFileProjList +=myFileInfo->absFilePath()+"\n"; 	
      }
      else 
      {
        //BAD FILE AND/OR BAD PROJ
        myInvalidFileFlag = true;
        myInvalidFileList +=myFileInfo->absFilePath()+"\n";         
      } 
    }  
    ++myIterator;
  }


}   


bool LayerSelector::isValidGdalFile(const QString theFilename)
{
      //test whether the file is GDAL compatible
      GDALAllRegister();
      GDALDataset * myTestFile = (GDALDataset *)GDALOpen( theFilename, GA_ReadOnly );

      if( myTestFile == NULL )
      {
        //not GDAL compatible
        GDALClose(myTestFile);
	return false;
      }
      else
      {
        //is GDAL compatible
	GDALClose(myTestFile);
	return true;  
      }
}

bool LayerSelector::isValidGdalProj(const QString theFilename)
{
      //test whether the file has GDAL projection info
      GDALAllRegister();
      GDALDataset * myTestFile = (GDALDataset *)GDALOpen( theFilename, GA_ReadOnly );
      
      QString myProjectionString = myTestFile->GetProjectionRef();
      
      if(myProjectionString.isEmpty())
      {
        //does not have projection info
        GDALClose(myTestFile);
	return false;
      }
      else
      {
        //does have projection info
	GDALClose(myTestFile);
	return true;
      }
}
