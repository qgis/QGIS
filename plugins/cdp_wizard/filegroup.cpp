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
using namespace std;

FileGroup::FileGroup()
{
    debugModeFlag=false;
    endOfMatrixFlag=false;
    fileReaderVector = new FileReaderVector();
    //automatically delete any filereader as it is removed from the collection
    fileReaderVector->setAutoDelete(true);
}
FileGroup::~FileGroup()
{}
/** Add a new file reader object to the filegroup and position the
*   offset at the start of the data block requested. */
bool FileGroup::addFileReader(FileReader* theFileReader, int theDataBlockNo)
{

    //if the program crashes at this point it is most likely because
    //of an unintialised pointer! (Thanks Paul!)
    //so check for uninitialised FileReader param:
    if (!theFileReader)
    {
        if (debugModeFlag)
            cout << "FileGroup::addFileReader(FileReader* theFileReader, int theDataBlockNo) error - theFileReader is uninitialised!" << endl;
        return false;
    }
    int mySizeInt = fileReaderVector->size();
    fileReaderVector->resize(mySizeInt+1);
    fileReaderVector->insert(mySizeInt,theFileReader);
    if (debugModeFlag)
        cout << "File group size: " << fileReaderVector->size() << endl;
    return true;


}

/** Get the next element from each fileReader and return the result as a vector. */
QValueVector<float> FileGroup::getElementVector()
{

    QValueVector<float> myFloatVector;
    //test that there are some files in our filereader group
    if ( fileReaderVector->size()==0)
    {
        return myFloatVector;
    }
    //test we are not at the end of the matrix
    if (endOfMatrixFlag)
    {
        return myFloatVector;
    }
    //retrieve the each FileReader from the colelction and get its current element
    for (int myIteratorInt = 0; myIteratorInt < fileReaderVector->size();myIteratorInt++)
    {
        FileReader * myCurrentFileReader = fileReaderVector->at(myIteratorInt);
        //test if we are at the end of the matrix
        if ( myIteratorInt==0)
        {
            endOfMatrixFlag=myCurrentFileReader->getEndOfMatrixFlag();
        }

        float myFloat = myCurrentFileReader->getElement();
        myFloatVector.push_back(myFloat);
        myCurrentFileReader = fileReaderVector->at(myIteratorInt);
    }

    return myFloatVector;
}

/** Read property of bool endOfMatrixFlag. */
const bool FileGroup::getEndOfMatrixFlag()
{
    return endOfMatrixFlag;
}

/** Move to the start of the active data block */
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
    //

    endOfMatrixFlag=false;
    //add better error checking
    return true;
}
/** Increment the currently active datablock by theIncrementAmount.
This allows you to move to a new  datablock in SRES type continuous files.
The file pointer will be moved to the start of the datablock */
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
    //

    endOfMatrixFlag=false;
    //add better error checking
    return true;
}
