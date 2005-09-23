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

#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>

/**A FileWriter is used to store the results of the climate data processor in a persistant manner.
  *@author Tim Sutton
  */
class FileWriter
{

public:

    /** An enumerator defining the different types of output files that can be written. */
    enum FileTypeEnum { MATLAB,
                        ESRI_ASCII ,
                        PLAIN };
    /** Constructor */
    FileWriter();
    /** Constructor
    * @param theFileName - a QString containing the name of the output file.
    * @param theFileFormat - the output format as defined in FileTypeEnum.
    * @see FileTypeEnum
    */
    FileWriter(const QString theFileName, FileTypeEnum theFileFormat);
    /** Destructor */
    ~FileWriter();
    /** Write a float element to the output file.
    * @param theElementFloat - the variable that is to be written off to file.
    * @return bool - indicating successor failure of the write operation.
    */
    bool writeElement(float theElementFloat);
    /** Write a header to the output file.
    * @param theString - the variable that is to be written off to file.
    * @return bool - indicating successor failure of the write operation.
    */    
    bool writeString(QString theQString); 
    /** Read property of QString fileNameString.
    * @return QString - containing the name of the file.
    */
    const QString getFileNameString();

    /** Close the currently open file.
    * @return bool - indicating successor failure of the close operation.
    */
    void close();

    /** This method sends a line break to the output file.
    * @return bool - indicating successor failure of the write operation.
    */
    bool sendLineBreak();
    /** Used to find out if this filewriter is writeable (it wont be if there
    * was an error opening the file for writing.
    * @return bool - flag indicating whether the file is writeable or not.
    */
    bool isWriteable() {return isWriteableFlag;};
    /** Accessor for the seperator between numbers written to the file.
    * @return QString - the separated currently in use.
    */

    QString seperator() {return seperatorString;};
    /** Mutator for the seperator between numbers in a file.
    * @param theQString - the new seperator to be used.
    */
    void setSeperator(QString theQString) {seperatorString=theQString;};

private:

    /**  The file handle containing our output data matrix. */
    QFile mFile;
    /** A text stream associated with the output file that
    * will be used when writing data to the file. */
    QTextStream mTextStream;
    /* The separater that will be used between each
    *  value as its written to file */
    QString seperatorString;

    /** The filename that is being written to */
    QString fileNameString;

    /** State of the filewriter - file open etc errors will
    mke the writeable state false. */
    bool isWriteableFlag;



};



#endif

