/***************************************************************************
                          filegroup.h  -  description
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

#ifndef FILEGROUP_H
#define FILEGROUP_H
#include "filereader.h"

/**A file group manages a collection of FileReader objects and is used
  * to steop through several filereaders simultaneously.
  * @author Tim Sutton
  */

class FileGroup {
public: 
	FileGroup();
	virtual ~FileGroup();
  /** Add a new file reader object to the filegroup and position the fpos_t at the start of the data block requested. */
  virtual bool addFileReader(FileReader *theFileReader) ;
  /** Return the number of filereaders associated with this filegroup. */
  int getFileReaderCount();
  /** Get the next element from each fileReader and return the result as a vector. */
  std::vector<float> getElementVector();
  /** Read property of bool endOfMatrixFlag. */
  virtual const bool getEndOfMatrixFlag();
  /** Move to the start of the active data block */
  virtual bool moveToDataStart();
  /** Increment the currently active datablock by theIncrementAmount.
  This allows you to move to a new  datablock in SRES type continuous files.
  The file pointer will be moved to the start of the datablock */
  bool incrementDataBlocks(int theIncrementAmountInt);
private:
  std::vector <FileReader> fileReaderVector;
  /** A flag to show whether the end of the matrix has been reached.
  * Note the first fileReader in the fileGroup is used to determine this. */
  bool endOfMatrixFlag;
  bool debugModeFlag;
};

#endif
