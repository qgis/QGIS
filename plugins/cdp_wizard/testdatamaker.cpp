//
// C++ Implementation: testdatamaker
//
// Description:
//
//
// Author: Tim Sutton <tim@linfiniti.com>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "testdatamaker.h"
#include <iostream>

TestDataMaker::TestDataMaker(QString outputFileName, int baseValue)
{
    bool endOfStringFlag=false;
    while(!endOfStringFlag)
    {
        int myInt =  outputFileName.find(" ");
        if(myInt != -1)    //-1 means no match found
        {
            outputFileName.replace(myInt,1,"_");
        }
        else
        {
            endOfStringFlag=true;
        }
    }
    filePointer=new QFile(outputFileName);
    seperatorString=QString(" ");
    if (!filePointer->open(IO_WriteOnly))
    {
        std::cout << "FileWriter::Cannot open file : " << outputFileName << std::endl;
        isWriteableFlag=false;
    }
    else
    {
        textStream = new QTextStream(filePointer);
        std::cout << "FileWriter::Opened file ... " << outputFileName << " successfully." << std::endl;
        isWriteableFlag=true;
    }

    QString myHeaderLine("");
    for (int myBlockIteratorInt = 0; myBlockIteratorInt < 600; myBlockIteratorInt++)
    {
        //write the header line
        //     +-- this no changes with each block
        //     |
        //     v
        //20010101  11     2   7008
        //20010201  11     2   7008
        // ...
        //20020101  11     2   7008
        //   ^
        //   |
        //   +-- year changes

        *textStream << QString("2002") << myBlockIteratorInt << QString("01  11     2   7008\n");
        int myColumnCountInt=0; //format data in 6 cols
        for (int myIteratorInt = 0; myIteratorInt < 7008; myIteratorInt++)
        {
            myColumnCountInt++;
            //write the number to the file
            *textStream << myIteratorInt + (myBlockIteratorInt*10000) << seperatorString;
            if (myColumnCountInt==6)
            {
                *textStream << QString("\n");
                myColumnCountInt=0;
            }
        }
    }
}

TestDataMaker::~TestDataMaker()
{}


