#include <iostream>
#include <sstream> //for string concatenation
#include <stdlib.h>
#include "filegroup.h"
#include "filereader.h"
#include "filewriter.h"
#include "dataprocessor.h"

//qt includes
#include <qapplication.h>
#include <cdpwizard.h>
#include <qvaluevector.h>
#include <qstring.h>
#include "testdatamaker.h"
int main(int argc, char *argv[])
{
    std::cout << "+---------------------------------------------------+" << std::endl;
    std::cout << "|             Climate Data Processor                |" << std::endl;
    std::cout << "|                   Tim Sutton                      |" << std::endl;
    std::cout << "|                      2004                         |" << std::endl;
    std::cout << "+---------------------------------------------------+" << std::endl;
    //TestDataMaker myTestDataMaker("/tmp/hadleytest.asc");
    QApplication myApp(argc, argv);
    CDPWizard *myCDPWizard = new CDPWizard(0);
    myApp.setMainWidget(myCDPWizard);
    myCDPWizard->show();
    return myApp.exec();

}

int produceAverages(int argc, char *argv[])
{
    std::cout << "+---------------------------------------------------+" << std::endl;
    std::cout << "|             Climate Data Averager                 |" << std::endl;
    std::cout << "|                   Tim Sutton                      |" << std::endl;
    std::cout << "|                      2003                         |" << std::endl;
    std::cout << "+---------------------------------------------------+" << std::endl;

    if (argc < 3 || argc > 4)
    {
        std::cout << "useage: " << argv[0]  << " <input sres file> <ouput file prefix> [debug]" << endl;
        return 0;
    }
    std::string myInputFileName;
    myInputFileName = argv[1];
    //check we have reasonable arguments

    if (myInputFileName=="" || myInputFileName=="/?" || myInputFileName=="--help")
    {
        std::cout << "useage: " << argv[0]  << " <input sres file> <ouput file prefix> [debug]" << endl;
        return 0;
    }
    std::cout << "Program name: " << argv[0] << endl;
    std::cout << "First arg: " << argv[1] << endl;
    std::cout << "Second arg: " << argv[2] << endl;
    std::string myOutputFileName;
    myOutputFileName = argv[2];
    //decide if we are in debug mode
    bool myDebugModeFlag;
    if (argv[3])
        myDebugModeFlag = 1;
    else
        myDebugModeFlag=0;
    FileReader *myFileReader = new FileReader(myInputFileName,FileReader::HADLEY_SRES);
    QValueVector <QFile::Offset>  myDataBlockMarkersVector =
        myFileReader->getBlockMarkers();
    int myNumberOfBlocksInt = myDataBlockMarkersVector.size();
    if (myDebugModeFlag)
        std::cout << "There are " << myNumberOfBlocksInt <<
        "  blocks in the input file." << std::endl;
    const int myNumberOfYearsInt = myNumberOfBlocksInt / 12; //modulus will be trimmed!
    FileGroup *myFileGroup = new FileGroup();
    for (int myCurrentMonthInt = 1; myCurrentMonthInt<13 ; myCurrentMonthInt++)
    {
        //construct a filename for this months output
        std::ostringstream myOStringStream;
        myOStringStream << myOutputFileName << "_" << myCurrentMonthInt << ".asc";
        std::string myCurrentFileNameString=myOStringStream.str();
        std::cout << "Output filename for month " <<
        myCurrentMonthInt << " set to " << myCurrentFileNameString << endl;
        //now make a filewriter using the filename string
        FileWriter *myFileWriter =
            new FileWriter(myCurrentFileNameString,FileWriter::ESRI_ASCII);
        //construct a filegroup containing each fileblock that corresponds
        //to this month. For jan this would be blocks 1,13,26, etc
        FileGroup *myFileGroup = new FileGroup();
        int myInt;
        for (myInt=myCurrentMonthInt; myInt <= myNumberOfYearsInt; myInt=myInt+12)
        {
            FileReader *myFileReader2 = new FileReader();
            myFileReader2->openFile(myInputFileName.c_str(),FileReader::HADLEY_SRES);
            
            myFileReader2->setBlockMarkers(myDataBlockMarkersVector);
            myFileReader2->setStartMonth(myInt);
            myFileGroup->addFileReader(myFileReader2,myInt);
        }
        long myElementsInBlockInt = myFileReader->getXDim() * myFileReader->getYDim();
        for (myInt=1; myInt < myElementsInBlockInt; myInt++)
        {
            DataProcessor *myDataProcessor = new DataProcessor();
            float myMeanFloat =       myDataProcessor->meanOverYear(myFileGroup->getElementVector());
            /*
               printf("Mean over (%i) datablocks is %f. Element %i of %i for month %i\n",
               myNumberOfYearsInt,
               myMeanFloat,
               myInt,
               myElementsInBlockInt,
               myCurrentMonthInt);
               */
            myFileWriter->writeElement(myMeanFloat);
        }
        delete myFileGroup;
        myFileWriter->close();
        delete myFileWriter;
    }
    /*
       QValueVector<fpos_t>::iterator myFposIterator;
       int myBlockCounterInt=0;
       for (myFposIterator = myDataBlockMarkersVector->begin(); myFposIterator != myDataBlockMarkersVector->end(); myFposIterator++)
       {
       myFileReader->setDataStartFPos(*myFposIterator);
       myFileReader->moveToDataStart();
       printf ("Block %i first Element is %f\n",
       myBlockCounterInt,
       myFileReader->getElement()
       );
       myBlockCounterInt+=12;
       }
       */
    system("PAUSE");
    return 0;
}

