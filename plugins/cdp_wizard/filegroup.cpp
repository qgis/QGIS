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
}
/** Destructor for file group - will delete any contained file readers */
FileGroup::~FileGroup(){

  /* crashes !    
     for ( vector<FileReader>::iterator myIterator = fileReaderVector.begin();
     myIterator != fileReaderVector.end();
     myIterator++)
     {
     delete &myIterator;
     }
     */
}
/** Add a new file reader object to the filegroup and position the
 * fpos_t at the start of the data block requested. */
bool FileGroup::addFileReader(FileReader* theFileReader) 
{

  //if the program crashes at this point it is most likely because
  //of an unintialised pointer! (Thanks Paul!)
  //so check for uninitialised FileReader param:
  if (!theFileReader)
  {
    if (debugModeFlag) cout << "FileGroup::addFileReader(FileReader* theFileReader, int theDataBlockNo) error - theFileReader is uninitialised!" << endl;
    return false;
  }
  fileReaderVector.push_back(*theFileReader);
  if (debugModeFlag) cout << "File group size: " << fileReaderVector.size() << endl;
  return true;


}
/** Return the number of filereaders associated with this filegroup. */
int FileGroup::getFileReaderCount()
{
  return fileReaderVector.size();
}
/** Get the next element from each fileReader and return the result as a vector. */
std::vector<float> FileGroup::getElementVector()
{

  std::vector<float> myFloatVector;
  //test that there are some files in our filereader group
  if ( fileReaderVector.size()==0)
  {
    return myFloatVector;
  }
  //test we are not at the end of the matrix
  if (endOfMatrixFlag)
  {
    return myFloatVector;
  }
  //this bit works
  /*
     FileReader *myFileReader;
     myFileReader= &fileReaderVector[0];
     if (debugModeFlag) cout << "Colums per row in first filereader: " << myFileReader->getColumnsPerRow() << endl;
     */

  //lets hope this does too  
  for ( vector<FileReader>::iterator myIterator = fileReaderVector.begin();
          myIterator != fileReaderVector.end();
          myIterator++)
  {
    myFloatVector.push_back(myIterator->getElement());
  }
  //test if we are at the end of the matrix
  if ( fileReaderVector[0].getEndOfMatrixFlag())
  {
    endOfMatrixFlag=true;
  }


  return myFloatVector;
}
/** Read property of bool endOfMatrixFlag. */
const bool FileGroup::getEndOfMatrixFlag(){
  return endOfMatrixFlag;
}

/** Move to the start of the active data block */
bool FileGroup::moveToDataStart()
{
  for ( vector<FileReader>::iterator myIterator = fileReaderVector.begin();
          myIterator != fileReaderVector.end();
          myIterator++)
  {
    myIterator->moveToDataStart();
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
  for ( vector<FileReader>::iterator myIterator = fileReaderVector.begin();
          myIterator != fileReaderVector.end();
          myIterator++)
  {

    myIterator->setCurrentBlock(myIterator->getCurrentBlock()+theIncrementAmountInt);
    myIterator->moveToDataStart();
  }
  //

  endOfMatrixFlag=false;
  //add better error checking
  return true;
}
