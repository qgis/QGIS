/***************************************************************************
                          filegroup.cpp  -  description
                             -------------------
    begin                : Sat May 10 2003
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

#include "filegroup.h"

FileGroup::FileGroup()
{
    elementCountInt=0;
    xDimInt=0;
    yDimInt=0;
    endOfMatrixFlag=false;
    fileReaderVector = new FileReaderVector();
    //automatically delete any filereader as it is
    //removed from the collection
    fileReaderVector->setAutoDelete(true);
}

FileGroup::~FileGroup()
{
  //clean up - close each filereader in the group
  for (int myIteratorInt=0;myIteratorInt<fileReaderVector->size();myIteratorInt++)
  {
    FileReader * myFileReader = fileReaderVector->at(myIteratorInt);
    myFileReader->closeFile();
  }
  //delete thee vector which will in turn autodelete its filereaders
  delete fileReaderVector;

}

bool FileGroup::addFileReader(FileReader* theFileReader,
                              int theDataBlockInt)
{
    if (!theFileReader)
    {
#ifdef QGISDEBUG
        std::cout << "FileGroup::addFileReader() error - theFileReader is uninitialised!" << std::endl;
#endif

        return false;
    }

    //expand the filereader vector by one and insert the new filereader
    //onto the end of the list
    int mySizeInt = fileReaderVector->size();
    fileReaderVector->resize(mySizeInt+1);
    fileReaderVector->insert(mySizeInt,theFileReader);
#ifdef QGISDEBUG
    cout << "File group size: " << fileReaderVector->size() << endl;
#endif
    //see if this was the first filereader being added and if so set the
    //elementCountInt property.
    if (mySizeInt==0)
    {
      elementCountInt=theFileReader->getXDim() * theFileReader->getYDim();
      xDimInt=theFileReader->getXDim();
      yDimInt=theFileReader->getYDim();
    }
    return true;
}

int FileGroup::getElementCount()
{
  return elementCountInt;
}


QValueVector<float> FileGroup::getElementVector()
{
    QValueVector<float> myFloatVector;
    //test that there are some files in our filereader group
    if ( fileReaderVector->isNull())
    {
        return 0;
    }
    //test we are not at the end of the matrix
    if (endOfMatrixFlag)
    {
        return 0;
    }
    //retrieve the each FileReader from the colelction and get its current element
    for (int myIteratorInt = 0; myIteratorInt < fileReaderVector->size();myIteratorInt++)
    {
        FileReader * myCurrentFileReader = fileReaderVector->at(myIteratorInt);
        float myFloat = myCurrentFileReader->getElement();
        //test if we are at the end of the matrix
        if ( myIteratorInt==0)
        {
            endOfMatrixFlag=myCurrentFileReader->getEndOfMatrixFlag();
        }

        myFloatVector.push_back(myFloat);
        myCurrentFileReader = fileReaderVector->at(myIteratorInt);
    }

    return myFloatVector;
}

const bool FileGroup::getEndOfMatrixFlag()
{
    return endOfMatrixFlag;
}

bool FileGroup::moveToDataStart()
{
    if (fileReaderVector->isNull())
    {
        return false;
    }
    for (int myIteratorInt = 0; myIteratorInt < fileReaderVector->size();myIteratorInt++)
    {
        FileReader * myCurrentFileReader = fileReaderVector->at(myIteratorInt);
        myCurrentFileReader->moveToDataStart();
    }
    endOfMatrixFlag=false;
    //add better error checking
    return true;
}

bool FileGroup::incrementDataBlocks(int theIncrementAmountInt)
{
    if (fileReaderVector->isNull())
    {
        return false;
    }
    for (int myIteratorInt = 0; myIteratorInt < fileReaderVector->size();myIteratorInt++)
    {
        FileReader * myCurrentFileReader = fileReaderVector->at(myIteratorInt);
        myCurrentFileReader->setStartMonth(myCurrentFileReader->getStartMonth()+theIncrementAmountInt);
        myCurrentFileReader->moveToDataStart();
    }
    endOfMatrixFlag=false;
    //add better error checking
    return true;
}
