/***************************************************************************
                          filewriter.h  -  description
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

#ifndef FILEWRITER_H
#define FILEWRITER_H

//#include <stdio.h>   //Paul reckons we dont need to explicitly include this
#include <string>
/**A FileWriter is used to store the results of the climate data processor in a persistant manner.
  *@author Tim Sutton
  */

class FileWriter {
public:
    enum FileTypeEnum { CSM_MATLAB , CSM_OCTAVE ,  GARP ,  ESRI_ASCII ,  PLAIN };
  /** constructor */
	FileWriter();
  /** constructor */
	FileWriter(const char *theFileName, FileWriter::FileTypeEnum theFileFormat);
	FileWriter(const char *theFileName, FileWriter::FileTypeEnum theFileFormat, bool theFlag);
  /** Alternate constructor that takes a string for the filename rather than a char array. */
   FileWriter(std::string theFileNameString, FileWriter::FileTypeEnum theFileFormat);
   FileWriter(std::string theFileNameString, FileWriter::FileTypeEnum theFileFormat, bool theFlag);
  /** destrctor */
  ~FileWriter();
  /** Write a float element to the output file. */
  bool writeElement(float theElementFloat);
  /** Read property of std::string fileNameString. */
   const std::string getFileNameString();
  /**
  * Write an arbitary header at start of the file. 
  * You must call this before writeElement 
  */
  bool writeHeader (std::string theHeaderString);
  /**
  * Close the currently open file.
  */
  bool close();
  /**
  * This method sends a line break to the output file.
  */
  bool sendLineBreak();

  bool getAppendModeFlag();
  
  void setAppendModeFlag(bool theFlag);

private:
  /**  The FILE handle containing our output data matrix. */
  FILE *filePointer;
  /** The filename that is being written to */
  std::string fileNameString;
  /**  To determine whether new data will overwrite an existing file*/
  bool appendModeFlag;
  
};

#endif
