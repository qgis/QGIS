//
// C++ Interface: testdatamaker
//
// Description:
//
//
// Author: Tim Sutton <tim@linfiniti.com>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TESTDATAMAKER_H
#define TESTDATAMAKER_H
#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>
/**
TestDataMaker is a class to build Hadley (and in future other file types) datasets for testing the climate data processor calculations in a controlled way.

@author Tim Sutton
*/
class TestDataMaker{
public:
    /**
    * Constructor.
    * @param outputFileName - A QString with the output filename for the test data.
    * @param baseValue - an (optional) integer that will be added to each cell in each output
    * block. This can be used to differentiate between two files when debugging with
    * test data. Defaults to 0!
    */
    TestDataMaker(QString outputFileName, int baseValue=0);
    /**
    * Destructor
    */
    ~TestDataMaker();
private:
    /**  The file handle containing our output data matrix. */
    QFile * filePointer;
    /** A text stream associated with the output file that
    * will be used when writing data to the file. */
    QTextStream * textStream;
    /* The separater that will be used between each
    *  value as its written to file */
    QString seperatorString;
    bool isWriteableFlag;
};

#endif
