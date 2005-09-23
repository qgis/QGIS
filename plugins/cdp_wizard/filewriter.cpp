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

  filePointer=new QFile(myFileNameString);
  seperatorString=QString(" ");
  if (!filePointer->open(IO_WriteOnly))
  {
    std::cout << "FileWriter::Cannot open file : " << myFileNameString << std::endl;
    isWriteableFlag=false;
  }
  else
  {
    textStream = new QTextStream(filePointer);
    fileNameString = myFileNameString;
    std::cout << "FileWriter::Opened file ... " << fileNameString << " successfully." << std::endl;
    isWriteableFlag=true;
  }
}

FileWriter::~FileWriter()
{
  delete filePointer;
}

bool FileWriter::writeElement(float theElementFloat){
  //cout << "FileWriter::writeElement Writing element to   " << fileNameString << endl;
  if (theElementFloat==-9999.0) { theElementFloat=-9999.9; }
  if (filePointer==0)
  {
    return false;
  }
  if (textStream==0)
  {
    return false;
  }
  //write the number to the file
  *textStream << theElementFloat << seperatorString;
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
  if (filePointer==0)
  {
    return false;
  }
  if (textStream==0)
  {
    return false;
  }
  //write the number to the file
  *textStream << QString("\n");
}

void FileWriter::close()
{
  filePointer->close();
}

bool FileWriter::writeString(QString theQString)
{
  if (filePointer==0)
  {
    return false;
  }
  if (textStream==0)
  {
    return false;
  }
  //write the string to the file
  *textStream << theQString;
}

