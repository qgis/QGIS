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

FileWriter::FileWriter(const char *theFileNameChar, FileWriter::FileTypeEnum theFileFormat)
{
  //cout << "FileWriter constructor called..." << endl;
  //replace any spaces in the filename with underscores and suffix the correct filename
  //extension
  std::string myFileNameString(theFileNameChar);
  try
  {
    bool endOfStringFlag=false;
    while(!endOfStringFlag)
    {
      int myInt =  myFileNameString.find(" ");
      if(myInt != std::string::npos)    //string::npos means no match found
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

  if ((filePointer=fopen(myFileNameString.c_str(),"wb"))==NULL)  //try to open in binary mode
  {
    cout << "FileWriter::Cannot open file : " << myFileNameString << endl;
  }
  else
  {
    fileNameString = myFileNameString;
    //cout << "FileWriter::Opened file : " << fileNameString << " successfully." << endl;

  }
}

FileWriter::FileWriter(const char *theFileNameChar, FileWriter::FileTypeEnum theFileFormat, bool theFlag)
{
  appendModeFlag = theFlag;


  //cout << "FileWriter constructor called..." << endl;
  //replace any spaces in the filename with underscores and suffix the correct filename
  //extension
  std::string myFileNameString(theFileNameChar);
  try
  {
    bool endOfStringFlag=false;
    while(!endOfStringFlag)
    {
      int myInt =  myFileNameString.find(" ");
      if(myInt != std::string::npos)    //string::npos means no match found
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

  if (appendModeFlag)
  {
    if ((filePointer=fopen(myFileNameString.c_str(),"wb"))==NULL)  //try to open in binary mode
    {
      cout << "FileWriter::Cannot open file : " << myFileNameString << endl;
    }
    else
    {
      fileNameString = myFileNameString;
      //cout << "FileWriter::Opened file : " << fileNameString << " successfully." << endl;

    }
  }
  else
  {
    if ((filePointer=fopen(myFileNameString.c_str(),"ab"))==NULL)  //try to open in binary mode
    {
      cout << "FileWriter::Cannot open file : " << myFileNameString << endl;
    }
    else
    {
      fileNameString = myFileNameString;
      //cout << "FileWriter::Opened file : " << fileNameString << " successfully." << endl;

    }
  }
}

/** Alternate constructor that takes a string for the filename rather than a char array. */
FileWriter::FileWriter(std::string theFileNameString, FileWriter::FileTypeEnum theFileFormat)
{
  //cout << "FileWriter constructor called..." << endl;
  //replace any spaces in the filename with underscores and suffix the correct filename
  //extension
  std::string myFileNameString=theFileNameString;
  try
  {
    bool endOfStringFlag=false;
    while(!endOfStringFlag)
    {
      int myInt =  myFileNameString.find(" ");
      if(myInt != std::string::npos)    //string::npos means no match found
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

  if ((filePointer=fopen(myFileNameString.c_str(),"wb"))==NULL)  //try to open in binary mode
  {
    //cout << "FileWriter::Cannot open file : " << myFileNameString << endl;
  }
  else
  {
    fileNameString = myFileNameString;
    //cout << "FileWriter::Opened file : " << fileNameString << " successfully." << endl;


  }
}

/** Alternate constructor that takes a string for the filename rather than a char array. */
FileWriter::FileWriter(std::string theFileNameString, FileWriter::FileTypeEnum theFileFormat, bool theAppendModeFlag)
{

  appendModeFlag = theAppendModeFlag;

  //cout << "FileWriter constructor called..." << endl;
  //replace any spaces in the filename with underscores and suffix the correct filename
  //extension
  std::string myFileNameString=theFileNameString;
  try
  {
    bool endOfStringFlag=false;
    while(!endOfStringFlag)
    {
      int myInt =  myFileNameString.find(" ");
      if(myInt != std::string::npos)    //string::npos means no match found
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

  if (appendModeFlag)
  { 
    if ((filePointer=fopen(myFileNameString.c_str(),"ab"))==NULL)  //try to open in binary mode
    {
      //cout << "FileWriter::Cannot open file : " << myFileNameString << endl;
    }
    else
    {
      fileNameString = myFileNameString;
      //cout << "FileWriter::Opened file : " << fileNameString << " successfully." << endl;

    }
  }
  else
  { 
    if ((filePointer=fopen(myFileNameString.c_str(),"wb"))==NULL)  //try to open in binary mode
    {
      //cout << "FileWriter::Cannot open file : " << myFileNameString << endl;
    }
    else
    {
      fileNameString = myFileNameString;
      //cout << "FileWriter::Opened file : " << fileNameString << " successfully." << endl;

    }
  }	
}


FileWriter::~FileWriter(){
}

bool FileWriter::getAppendModeFlag(){
  return appendModeFlag;
}

void FileWriter::setAppendModeFlag(bool theFlag){
  appendModeFlag = theFlag;
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

/**
 * Write an arbitary header at start of the file. 
 * You must call this before writeElement 
 */
bool FileWriter::writeHeader(string theHeaderString)
{
  int myResultInt =fprintf ( filePointer, "%s ",theHeaderString.c_str());
  if (myResultInt < 0)
  {
    return false;
  }
  else
  {
    return true;
  } 



}

/** Read property of std::string fileNameString. */
const std::string FileWriter::getFileNameString()
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

