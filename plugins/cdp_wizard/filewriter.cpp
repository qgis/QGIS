/***************************************************************************
  filewriter.cpp  -  description
  -------------------
begin                : Tue May 13 2003
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

#include "filewriter.h"
#include <iostream>
using namespace std;

FileWriter::FileWriter(){
}

/** Alternate constructor that takes a string for the filename rather than a char array. */
FileWriter::FileWriter(QString theFileNameString, FileWriter::FileTypeEnum theFileFormat)
{
  //cout << "FileWriter constructor called..." << endl;
  //replace any spaces in the filename with underscores and suffix the correct filename
  //extension
  QString myFileNameString=theFileNameString;
  try
  {
    bool endOfStringFlag=false;
    while(!endOfStringFlag)
    {
      int myInt =  myFileNameString.find(" ");
      if(myInt != -1)    //-1 means no match found
      {
        myFileNameString.replace(myInt,1,"_");
        //cout << "FileWriter::Filename after space replacement (myInt " << myInt << "):" << myFileNameString << endl;
      }
      else
      {
        endOfStringFlag=true;
      }
    }
  }
  catch (...)
  {
    cout << "Space replacement threw an error! " <<  endl;
  }

  if ((filePointer=fopen(myFileNameString,"wb"))==NULL)  //try to open in binary mode
  {
    //cout << "FileWriter::Cannot open file : " << myFileNameString << endl;
  }
  else
  {
    fileNameString = myFileNameString;
    //cout << "FileWriter::Opened file : " << fileNameString << " successfully." << endl;

  }
}

FileWriter::~FileWriter(){
}

/** Write a float element to the output file. */
bool FileWriter::writeElement(float theElementFloat){
  //cout << "FileWriter::writeElement Writing element to   " << fileNameString << endl;
  int myResultInt =fprintf ( filePointer, "%f ",theElementFloat);
  if (myResultInt < 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

/** Read property of QString fileNameString. */
const QString FileWriter::getFileNameString()
{
  return fileNameString;
}

/*
 * This method sends a line break to the output file.
 */
bool FileWriter::sendLineBreak()
{
  int myResultInt =fprintf ( filePointer, "\n");
  if (myResultInt < 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

/*
 * Close the currently open file.
 */
bool FileWriter::close()
{
  /* Close stream */
  if( fclose( filePointer ) )
  {
    cout << "The file '" << fileNameString << "' was not closed\n" ;
    return false;
  }
  else
  {
    return true;
  }

}

