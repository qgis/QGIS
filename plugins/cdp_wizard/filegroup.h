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
#include <qvaluevector.h>
#include <qptrvector.h>


/**A file group manages a collection of FileReader objects and is used
  * to steop through several filereaders simultaneously.
  * @author Tim Sutton
  */

class FileGroup
{
public:
    /** Constructor for filegroup. Initialises the filereader vector. */
    FileGroup();
    /** Desctructor - closes each file in the filereader vector and then destroys the vector. */
    ~FileGroup();
    /** Add a new file reader object to the filegroup and position the fpos_t at the start of the data block requested. */
    bool addFileReader(FileReader *theFileReader, int theDataBlockNo) ;
    /** Get the next element from each fileReader and return the result as a vector. */
    QValueVector<float> getElementVector();
    /**
    * Accessor for the elementCountInt property. This property tells us
    * how many cells in any one data block in a filereader exist.
    * @return int - The number of cells in a block of the first filereader.
    */
    int getElementCount();
        /**
    * Accessor for the xDimInt property. This property tells us
    * how many cells in any one data block in the x dimension in a filereader exist.
    * @return int - The number of cells in a block of the first filereader.
    */
    int getXDimInt() {return xDimInt;};
        /**
    * Accessor for the xDimInt property. This property tells us
    * how many cells in any one data block in the y dimension in a filereader exist.
    * @return int - The number of cells in a block of the first filereader.
    */
    int getYDimInt() {return yDimInt;};
    /** Read property of bool endOfMatrixFlag. */
    const bool getEndOfMatrixFlag();
    /** Move to the start of the active data block */
    bool moveToDataStart();
    /** Increment the currently active datablock by theIncrementAmount.
    This allows you to move to a new  datablock in SRES type continuous files.
    The file pointer will be moved to the start of the datablock */
    bool incrementDataBlocks(int theIncrementAmountInt);
private:
    /** Type specification for pointer vector for holding file readers. */
    typedef QPtrVector <FileReader> FileReaderVector;
    /**This is the container for all the file readers in this group. */
    FileReaderVector * fileReaderVector;
    /**
    * The number of cells (xdim * ydim) in the block in the first file in the
    * file group. It is assumed that all files in the group have the same block
    * dimensions.
    */
    int elementCountInt;
   /**
    * The number of cells in the x dimension in a block in the first file in the
    * file group. It is assumed that all files in the group have the same block
    * dimensions.
    */
    int xDimInt;
   /**
    * The number of cells in the y dimension in a block in the first file in the
    * file group. It is assumed that all files in the group have the same block
    * dimensions.
    */
    int yDimInt;
    /**
    * A flag to show whether the end of the matrix has been reached.
    * @note the first fileReader in the fileGroup is used to determine this.
    */
    bool endOfMatrixFlag;
    
    
};

#endif
