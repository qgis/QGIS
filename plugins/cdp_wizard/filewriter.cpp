/***************************************************************************
  filewriter.cpp  -  description
  -------------------
begin                : Tue May 13 2003
copyright            : (C) 2003 by Tim Sutton
email                : t.sutton@reading.ac.uk

 ***************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filewriter.h"
#include <iostream>


FileWriter::FileWriter()

{


}


FileWriter::FileWriter(QString theFileNameString, FileWriter::FileTypeEnum theFileFormat)
{
  //replace any spaces in the filename with underscores and
  //suffix the correct filename extension
  QString myFileNameString=theFileNameString;
    bool endOfStringFlag=false;
    while(!endOfStringFlag)
    {
      int myInt =  myFileNameString.find(" ");
      if(myInt != -1)    //-1 means no match found
      {
        myFileNameString.replace(myInt,1,"_");
      }
      else
      {
        endOfStringFlag=true;
      }
    }
  mFile.setName(theFileNameString);
  seperatorString=QString(" ");
  if (!mFile.open(IO_WriteOnly))
  {
    std::cout << "FileWriter::Cannot open file : " << myFileNameString << std::endl;
    isWriteableFlag=false;
  }
  else
  {
    mTextStream.setDevice(&mFile);
    fileNameString = myFileNameString;
    std::cout << "FileWriter::Opened file ... " << fileNameString << " successfully." << std::endl;
    isWriteableFlag=true;
  }
}

FileWriter::~FileWriter()
{

}

bool FileWriter::writeElement(float theElementFloat){
  //cout << "FileWriter::writeElement Writing element to   " << fileNameString << endl;
  //
  // Right there is a bit of kludging going on here:
  // 1) if you send -9999.0 out the output stream, it truncates it to -9999
  // 2) if gdal reads -9999 as the first cell, it assumes dataset is int 16, causeing all values
  //    thereafter to be read as int16, losing any data after the decimal place
  // 3) using a decimal place wich isnt well represented by float32 will cause problems.
  //    we initially used -9999.99 as no data, but when gdal reads this from the asc file again
  //    it incorrectly receivese the value of -9999.8998433943 or similar. THis causes all the
  //    stats for the file to be incorrect.
  // 4) in dataprocessor, comparisons of no data are made between the input file data (which is -9999 usually)
  //    and the data processords idea of what no data shoud be. Consequently we need to rewrite nodata now.
  //
  if (theElementFloat==-9999.0) { theElementFloat=-9999.5; }
  //if (mFile==0)
  //{
  //  return false;
  //}
  //if (mTextStream==0)
  //{
   // return false;
  //}
  //write the number to the file
  mTextStream << theElementFloat << seperatorString;
}

const QString FileWriter::getFileNameString()
{
  return fileNameString;
}

/*
 * This method sends a line break to the output file.
 */
bool FileWriter::sendLineBreak()
{
  //cout << "FileWriter::writeElement Writing element to   " << fileNameString << endl;
  mTextStream << QString("\n");
}

void FileWriter::close()
{
  mFile.close();
}

bool FileWriter::writeString(QString theQString)
{
  //write the string to the file
  mTextStream << theQString;
}

