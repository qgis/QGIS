/***************************************************************************
  climatedataprocessor.cpp  -  description
  -------------------
begin                : Thu May 15 2003
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

#include "climatedataprocessor.h"
#include "filewriter.h"
#include "filereader.h"
#include "filegroup.h"
#include <iostream>
//QT Includes
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qregexp.h>

const float NO_DATA=-9999.0;

ClimateDataProcessor::ClimateDataProcessor() : QObject()
{
    std::cout << "Climate Data Processor constructor called." << std::endl;

    // initialise the fileGroup file names  - not strictly needed but neater
    QString myString=QString("");
    meanTempFileNameString=myString;
    minTempFileNameString=myString;
    maxTempFileNameString=myString;
    diurnalTempFileNameString=myString;
    meanPrecipFileNameString=myString;
    frostDaysFileNameString=myString;
    totalSolarRadFileNameString=myString;
    windSpeedFileNameString=myString;

    outputFilePathString = myString;
    inputFileType=FileReader::GDAL;


}

/** Destructor */
ClimateDataProcessor::~ClimateDataProcessor()
{}


const QString ClimateDataProcessor::getOutputHeaderString()
{
    return outputHeaderString;
}

void ClimateDataProcessor::setOutputHeaderString( const QString& theOutputHeaderString)
{
    outputHeaderString = theOutputHeaderString;
}

bool ClimateDataProcessor::makeInputFileTypeMap()
{
    QString myString;
    FileReader::FileTypeEnum myEnum;
    inputFileTypeMap.clear();
    // Copied from FileReader header file:
    //enum FileTypeEnum { GDAL, HADLEY_SRES , HADLEY_IS92 ,  IPCC_OBSERVED ,
    //                                    VALDES ,  ECHAM4 ,  CSIRO_MK2 ,  NCAR_CSM_PCM , GFDL_R30 , CGCM2 ,
    //                                    CCSR_AGCM_OGCM };


    //declare the key value
    myString=QString("GDAL Supported Raster");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::GDAL;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("Hadley Centre HadCM3 SRES Scenario");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::HADLEY_SRES;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("Hadley Centre HadCM3 IS92a Scenario");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::HADLEY_IS92;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("IPCC Observed Climatology");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::IPCC_OBSERVED;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("University of Reading Palaeoclimate data");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::VALDES;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("Max Planck Institute fur Meteorologie (MPIfM) ECHAM4 data");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::ECHAM4;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("CSIRO-Mk2 Model data");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::CSIRO_MK2;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("National Center for Atmospheric Research (NCAR) NCAR-CSM and NCAR-PCM data");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::NCAR_CSM_PCM;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("Geophysical Fluid Dynamics Laboratory (GFDL) R30 Model data");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::GFDL_R30;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("Canadian Center for Climate Modelling and Analysis (CCCma) CGCM2 Model data");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::CGCM2;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("CCSR/NIES AGCM model data and CCSR OGCM model data");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileReader::CCSR_AGCM_OGCM;
    //add it to the associative array
    inputFileTypeMap[myString]=myEnum;

    return true;
}

bool ClimateDataProcessor::makeOutputFileTypeMap()
{
    QString myString;
    FileWriter::FileTypeEnum myEnum;
    outputFileTypeMap.clear();
    // Copied from FileWriter header file:
    //enum FileFormatEnum { GDAL_TIFF, CSM_MATLAB , CSM_OCTAVE ,  GARP ,  ESRI_ASCII ,  PLAIN };

    //declare the key value
    myString=QString("GDAL Tiff Image");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileWriter::ESRI_ASCII;
    //add it to the associative array
    outputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("Matlab");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileWriter::MATLAB;
    //add it to the associative array
    outputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("ESRI ASCII Grid");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileWriter::ESRI_ASCII;
    //add it to the associative array
    outputFileTypeMap[myString]=myEnum;

    //declare the key value
    myString=QString("Plain matrix with no header");
    //convert it to upper case
    myString=myString.upper();
    //set its associated enum
    myEnum=FileWriter::PLAIN;
    //add it to the associative array
    outputFileTypeMap[myString]=myEnum;
    return true;
}

const QString  ClimateDataProcessor::getMeanTempFileName()
{
    return meanTempFileNameString;
}

void ClimateDataProcessor::setMeanTempFileName( QString theFileNameString)
{
    meanTempFileNameString = theFileNameString;
    std::cout << "meanTempFileNameString set to : " << meanTempFileNameString << std::endl;
}

const QString  ClimateDataProcessor::getMinTempFileName()
{
    return minTempFileNameString;
}

void ClimateDataProcessor::setMinTempFileName( QString theFileNameString)
{
    minTempFileNameString = theFileNameString;
    std::cout << "minTempFileNameString set to : " << minTempFileNameString << std::endl;
}

const QString  ClimateDataProcessor::getMaxTempFileName()
{
    return maxTempFileNameString;
}

void ClimateDataProcessor::setMaxTempFileName( QString theFileNameString)
{
    maxTempFileNameString = theFileNameString;
    std::cout << "maxTempFileNameString set to : " << maxTempFileNameString << std::endl;
}

const QString  ClimateDataProcessor::getDiurnalTempFileName()
{
    return diurnalTempFileNameString;
}

void ClimateDataProcessor::setDiurnalTempFileName( QString theFileNameString)
{
    diurnalTempFileNameString = theFileNameString;
    std::cout << "diurnalTempFileNameString set to : " << diurnalTempFileNameString << std::endl;
}

const QString  ClimateDataProcessor::getMeanPrecipFileName()
{
    return meanPrecipFileNameString;
}

void ClimateDataProcessor::setMeanPrecipFileName( QString theFileNameString)
{
    meanPrecipFileNameString = theFileNameString;
    std::cout << "meanPrecipFileNameString set to : " << meanPrecipFileNameString << std::endl;
}

const QString  ClimateDataProcessor::getFrostDaysFileName()
{

    std::cout << "frostDaysFileNameString set to : " << frostDaysFileNameString << std::endl;
    return frostDaysFileNameString;
}

void ClimateDataProcessor::setFrostDaysFileName( QString theFileNameString)
{
    frostDaysFileNameString = theFileNameString;
    std::cout << "frostDaysFileNameString set to : " << frostDaysFileNameString << std::endl;
}

const QString  ClimateDataProcessor::getTotalSolarRadFileName()
{
    return totalSolarRadFileNameString;
}

void ClimateDataProcessor::setTotalSolarRadFileName( QString theFileNameString)
{
    totalSolarRadFileNameString = theFileNameString;

    std::cout << "totalSolarRadFileNameString set to : " << totalSolarRadFileNameString << std::endl;
}

const QString  ClimateDataProcessor::getWindSpeedFileName()
{
    return windSpeedFileNameString;
}

void ClimateDataProcessor::setWindSpeedFileName( QString theFileNameString)
{
    windSpeedFileNameString = theFileNameString;
    std::cout << "windSpeedFileNameString set to : " << windSpeedFileNameString << std::endl;
}

const QString  ClimateDataProcessor::getOutputFilePathString()
{
    return outputFilePathString;
}

void ClimateDataProcessor::setOutputFilePathString( QString theFilePathString)
{
    outputFilePathString = theFilePathString;
    std::cout << "outputFilePathString set to : " << outputFilePathString << std::endl;
}

bool ClimateDataProcessor::makeFileGroups()
{

    /*   These are the possible filegroups available:
         -----------------------------------------------------------------
         meanTempFileGroup;
         minTempFileGroup;
         maxTempFileGroup;
         diurnalTempFileGroup;
         meanPrecipFileGroup;
         frostDaysFileGroup;
         totalSolarRadFileGroup;
         windSpeedFileGroup;
         -----------------------------------------------------------------
         */
    const int START_YEAR=1; //this will be deprecated soon!
    if (meanTempFileNameString==QString(""))
    {
        std::cout <<     "makeFileGroups - meanTempFileNameString is NOT initialised! *****************************" << std::endl;
    }
    else
    {
        meanTempFileGroup = initialiseFileGroup(meanTempFileNameString,START_YEAR);
    }

    if (minTempFileNameString==QString(""))
    {
        std::cout <<     "makeFileGroups - minTempFileNameString is NOT initialised! *****************************" << std::endl;
    }
    else
    {
        minTempFileGroup = initialiseFileGroup(minTempFileNameString,START_YEAR);
    }

    if (maxTempFileNameString==QString(""))
    {
        std::cout <<     "makeFileGroups - maxTempFileNameString is NOT initialised! *****************************" << std::endl;
    }
    else
    {
        maxTempFileGroup = initialiseFileGroup(maxTempFileNameString,START_YEAR);
    }

    if (diurnalTempFileNameString==QString(""))
    {
        std::cout <<     "makeFileGroups - diurnalTempFileNameString is NOT initialised! *****************************" << std::endl;
    }
    else
    {
        diurnalTempFileGroup = initialiseFileGroup(diurnalTempFileNameString,START_YEAR);
    }

    if (meanPrecipFileNameString==QString(""))
    {
        std::cout <<     "makeFileGroups - meanPrecipFileNameString is NOT initialised! *****************************" << std::endl;
    }
    else
    {
        meanPrecipFileGroup = initialiseFileGroup(meanPrecipFileNameString,START_YEAR);
    }

    if (meanPrecipFileNameString==QString(""))
    {
        std::cout <<     "makeFileGroups - meanPrecipFileNameString is NOT initialised! *****************************" << std::endl;
    }
    else
    {
        meanPrecipFileGroup = initialiseFileGroup(meanPrecipFileNameString,START_YEAR);
    }


    if (frostDaysFileNameString==QString(""))
    {
        std::cout <<     "makeFileGroups - frostDaysFileNameString is NOT initialised! *****************************" << std::endl;
    }
    else
    {
        frostDaysFileGroup = initialiseFileGroup(frostDaysFileNameString,START_YEAR);
    }


    if (totalSolarRadFileNameString==QString(""))
    {
        std::cout << "makeFileGroups - totalSolarRadFileNameString is NOT initialised! *****************************" << std::endl;
    }
    else
    {
        totalSolarRadFileGroup = initialiseFileGroup(totalSolarRadFileNameString,START_YEAR);
    }


    if (windSpeedFileNameString==QString(""))
    {
        std::cout << "makeFileGroups - totalSolarRadFileNameString is NOT initialised! *****************************" << std::endl;
    }
    else
    {
        windSpeedFileGroup = initialiseFileGroup(windSpeedFileNameString,START_YEAR);
    }
    return true;
}
/*   Construct a filegroup given a base file name.
 *   @param theStartYearInt Used when processing data NOT in series to shift the
 *           internal file pointer to the correct year. Note this is a base 1 number.
 *           Default is 1.
 */
FileGroup * ClimateDataProcessor::initialiseFileGroup(QString theFileNameString,int theStartYearInt=1)
{
    FileGroup *  myFileGroup = new FileGroup();
    std::cout << "initialiseFileGroup - theFileNameString is initialised to : " << theFileNameString << std::endl;
    if (filesInSeriesFlag)
    {
        //we have a separate file for each months data - the
        //assumption we make then is that the user selected the first file
        //e.g. file_00.asc and that the remaining files are numbered sequentially
        std::cout << "initialiseFileGroup - files in series!" << std::endl;
        //create a file group from this file
        std::cout << "initialiseFileGroup - theFileNameString = " << theFileNameString << std::endl;
        //need to add some error handling here...

       QFileInfo myFileInfo(theFileNameString);
       QString myPath = myFileInfo.dirPath();
       QString myExtension = myFileInfo.extension(TRUE); //include all dotted extensions e.g. tar.gz
       QString myFileName = myFileInfo.baseName(FALSE);//exclude all dotted extentsions e.g. will return 'file' from 'file.tar.gz'
       //! @note the assumption is implicit here that the first file in a file series ends in 00 !!!! 
       QString myFileNameBase = myFileName.left(myFileName.length()-2);//e.g. 'somefile00' becomes 'somefile'

        for (int myInt=1; myInt < 13; myInt++)
        {
            QString myCurrentFileName = myPath+QDir::separator()+myFileNameBase;

            if (myInt < 10)
            {
                myCurrentFileName+=QString("0")+QString::number(myInt);
            }
            else
            {
                myCurrentFileName+=QString::number(myInt);
            }
            myCurrentFileName+="."+myExtension;
            std::cout << "initialiseFileGroup - opening file : " << myCurrentFileName << std::endl;
            FileReader *myFileReader = new FileReader();
            myFileReader->openFile(myCurrentFileName,inputFileType);            
            //myFileReader->setFileName( myCurrentFileNameString.c_str() );
            myFileReader->getBlockMarkers();
            myFileReader->moveToDataStart();
            std::cout << "initialiseFileGroup - *** Adding " << myCurrentFileName
            << " to file group *********************" << std::endl;
            myFileGroup->addFileReader(myFileReader,myInt);

        }

        return myFileGroup;

    }//end of first part of file in series flag test
    else
    {
        //the file is not in series this consists of a single file with
        //multiple datablocks - one for each period (day / month / year etc)
        std::cout << "initialiseFileGroup - files NOT in series!" << std::endl;
        std::cout << "initialiseFileGroup - theFileNameString = " << theFileNameString << std::endl;
        //need to add some error handling here...
        //create a separate filereader object so we can get the blockmarkers
        FileReader *myFileReader = new FileReader();
        if ( !myFileReader->openFile(theFileNameString,inputFileType))
        {
            QMessageBox::critical( 0, "CDP Wizard",
                                   QString("The file you have selected could not be opened:\n\n" + theFileNameString +
                                           "\n\n Halting climate data proceesing." ));
            return false;
        }

        //when we open the first filereader in the group, we find
        //the block markers which will then be  assicgned to all other
        //filereaders we open as the filereaders all use the same file
        QValueVector <QFile::Offset> myDataBlockMarkersVector = myFileReader->getBlockMarkers();
        if (myDataBlockMarkersVector==0)  //chesk if null returned
        {
            QMessageBox::critical( 0, "CDP Wizard",
                                   QString("Error - failed to get block markers for :\n\n" + theFileNameString +
                                           "\n\n Halting climate data proceesing." ));
            return false;

        }
        //calculate the actual blocks numbers represented by the start & end years
        int myCurrentBlockInt = ((theStartYearInt-1)*12)+1;
        int myEndBlockInt = (12*theStartYearInt)+1;

        printf ("ClimatteDataProcessor::initialiseFileGroup Calculated start block : %i and end block is %i\n",
                myCurrentBlockInt,
                myEndBlockInt
               );

        //now we loop through 12 blocks to create our filegroup
        //this is the loop that sets up file readers - one per data
        //block for the required year, for the current month.
        for (int myInt=myCurrentBlockInt; myInt < myEndBlockInt; ++myInt)
        {
            FileReader *myFileReader2 = new FileReader();
            myFileReader2->openFile(theFileNameString,inputFileType);            
            myFileReader2->setBlockMarkers(myDataBlockMarkersVector);
            myFileReader2->setStartMonth(myInt);
            myFileGroup->addFileReader(myFileReader2,myInt);
            std::cout << "Adding block " << myInt << " to the block list to be processed" << std::endl;
        }


        //clean up
        delete myFileReader;

        return myFileGroup;
    } //end of second part of files in series flag test

}


/** Read property of FileReader::FileType FileType. */
const FileReader::FileTypeEnum ClimateDataProcessor::getInputFileType()
{
    return inputFileType;
}
/** Write property of FileReader::FileType FileType. */
void ClimateDataProcessor::setInputFileType( const FileReader::FileTypeEnum theInputFileType)
{
    inputFileType = theInputFileType;
}

/** Overloaded version of above that taks a string and looks up the enum */
void ClimateDataProcessor::setInputFileType( const QString theInputFileTypeString)
{


    QString myString = theInputFileTypeString;
    FileReader::FileTypeEnum myInputFileTypeEnum;
    //make sure the fileTypeMap exists
    makeInputFileTypeMap();
    //convert the input string to ucase
    myString=myString.upper();
    //look up the filetype enum given the string
    myInputFileTypeEnum = inputFileTypeMap[myString];
    //set the filetype given the enum
    setInputFileType (myInputFileTypeEnum);

}


/** Read property of FileWriter::FileType outputFileType. */
const FileWriter::FileTypeEnum ClimateDataProcessor::getOutputFileType()
{
    return outputFileType;
}
/** Write property of FileWriter::FileType FileType. */
void ClimateDataProcessor::setOutputFileType( const FileWriter::FileTypeEnum theOutputFileType)
{
    outputFileType = theOutputFileType;
}

/** Overloaded version of above that taks a string and looks up the enum */
void ClimateDataProcessor::setOutputFileType( const QString theOutputFileTypeString)
{


    QString myString = theOutputFileTypeString;
    FileWriter::FileTypeEnum myOutputFileTypeEnum;
    //make sure the fileTypeMap exists
    makeOutputFileTypeMap();
    //convert the Output string to ucase
    myString=myString.upper();
    //look up the filetype enum given the string
    myOutputFileTypeEnum = outputFileTypeMap[myString];
    //set the filetype given the enum
    setOutputFileType (myOutputFileTypeEnum);

}

/**  Build a list of which calculations can be performed given the input files
 *    that have been registered.
 *   The boolean flag will be used to indicate whether the user actually wants to
 *   perform the calculation on the input dataset(s) defaults to false.
 */
bool  ClimateDataProcessor::makeAvailableCalculationsMap()
{
#ifdef QGISDEBUG
    std::cout << "ClimateDataProcessor::makeAvailableCalculationsMap() called!" << std::endl;
#endif

    QString myString;
    bool myBool=false;  //default is not to perform any eligible  calculation
    availableCalculationsMap.clear();
    /*   These are the possible filegroups available:
         -----------------------------------------------------------------
         meanTempFileNameString;
         minTempFileNameString;
         maxTempFileNameString;
         diurnalTempFileNameString;
         meanPrecipFileNameString;
         frostDaysFileNameString;
         totalSolarRadFileNameString;
         windSpeedFileNameString;
         -----------------------------------------------------------------
         */
    std::cout << "Mean Temp FileName : " <<  getMeanTempFileName() << std::endl;
    std::cout << "Max Temp FileName : " << getMaxTempFileName() << std::endl;
    std::cout << "Min Temp FileName : " << getMinTempFileName() << std::endl;
    std::cout << "Diurnal Temp FileName : " << getDiurnalTempFileName() << std::endl;
    std::cout << "Mean Precipitation FileName : " << getMeanPrecipFileName() << std::endl;
    std::cout << "Frost Days FileName : " << getFrostDaysFileName() << std::endl;
    std::cout << "Total Solar Radiation FileName : " << getTotalSolarRadFileName() << std::endl;
    std::cout << "Wind Speed FileName : " << getWindSpeedFileName() << std::endl;

    if (diurnalTempFileNameString != "")
    {
        //declare the key value
        myString=QString("Annual mean diurnal temperature range");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }

    if (frostDaysFileNameString != "")
    {
        //declare the key value
        myString=QString("Annual mean number of frost days");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }

    if (totalSolarRadFileNameString != "")
    {
        std::cout << "Solar incident radiation : " <<    totalSolarRadFileNameString << std::endl;
        //declare the key value
        myString=QString("Annual mean total incident solar radiation");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }

    if (maxTempFileGroup && minTempFileNameString  != "" && maxTempFileNameString != "" )
    {
        //declare the key value
        myString=QString("Annual temperature range");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }

    if (maxTempFileNameString != "")
    {
        //declare the key value
        myString=QString("Highest temperature in warmest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }

    if (minTempFileNameString != "")
    {
        //declare the key value
        myString=QString("Lowest temperature in coolest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanPrecipFileNameString != "")
    {
        //declare the key value
        myString=QString("Mean daily precipitation");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanPrecipFileNameString != "" && meanTempFileNameString != "" )
    {
        //declare the key value
        myString=QString("Mean daily precipitation in coolest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanPrecipFileNameString != "" && meanTempFileNameString != "" )
    {
        //declare the key value
        myString=QString("Mean daily precipitation in coolest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanPrecipFileNameString != "" )
    {
        //declare the key value
        myString=QString("Mean daily precipitation in driest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanPrecipFileNameString != "" )
    {
        //declare the key value
        myString=QString("Mean daily precipitation in driest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanPrecipFileNameString != ""  &&  meanTempFileNameString != ""  )
    {
        //declare the key value
        myString=QString("Mean daily precipitation in warmest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanPrecipFileNameString != ""  && meanTempFileNameString != ""  )
    {
        //declare the key value
        myString=QString("Mean daily precipitation in warmest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanPrecipFileNameString != ""  )
    {
        //declare the key value
        myString=QString("Mean daily precipitation in wettest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanPrecipFileNameString != ""  )
    {
        //declare the key value
        myString=QString("Mean daily precipitation in wettest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (diurnalTempFileNameString != "" && meanTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Mean diurnal temperature range in coolest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( diurnalTempFileNameString != "" && meanTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Mean diurnal temperature range in warmest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (  meanPrecipFileNameString != ""  &&  frostDaysFileNameString != "")
    {
        //declare the key value
        myString=QString("Mean precipitation in frost free months");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( meanTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Mean temperature");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Mean temperature in coolest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( meanTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Mean temperature in coolest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( meanTempFileNameString !="" &&  frostDaysFileNameString != "")
    {
        //declare the key value
        myString=QString("Mean temperature in frost free months");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( meanTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Mean temperature in warmest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( meanTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Mean temperature in warmest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( windSpeedFileNameString != "")
    {
        //declare the key value
        myString=QString("Mean wind speed");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( minTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Number of months with minimum temperature above freezing");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( totalSolarRadFileNameString != "" && meanTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Radiation in coolest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( totalSolarRadFileNameString != ""  && meanTempFileNameString !="")
    {
        //declare the key value
        myString=QString("Radiation in coolest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( totalSolarRadFileNameString != "" && meanPrecipFileNameString != "")
    {
        //declare the key value
        myString=QString("Radiation in driest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (totalSolarRadFileNameString != "" && meanPrecipFileNameString != "")
    {
        //declare the key value
        myString=QString("Radiation in driest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( totalSolarRadFileNameString != ""  && meanTempFileNameString != "")
    {
        //declare the key value
        myString=QString("Radiation in warmest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( totalSolarRadFileNameString != ""  && meanTempFileNameString != "")
    {
        //declare the key value
        myString=QString("Radiation in warmest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( totalSolarRadFileNameString != "" && meanPrecipFileNameString != "")
    {
        //declare the key value
        myString=QString("Radiation in wettest month");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( totalSolarRadFileNameString != ""  && meanPrecipFileNameString != "")
    {
        //declare the key value
        myString=QString("Radiation in wettest quarter");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if ( meanPrecipFileNameString != "")
    {
        //declare the key value
        myString=QString("Standard deviation of mean precipitation");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }
    if (meanTempFileNameString != "")
    {
        //declare the key value
        myString=QString("Standard deviation of mean temperature");
        //add it to the associative array
        availableCalculationsMap[myString]=myBool;
    }

    //presume all went ok - need to add better error checking later
    return true;
} //end of makeAvailableCalculationsMap

/** Return the map of available calculations */
QMap <QString, bool > ClimateDataProcessor::getAvailableCalculationsMap()
{

    return  availableCalculationsMap;

}

bool ClimateDataProcessor::addUserCalculation(QString theCalculationNameString)
{
    QMap<QString, bool>::iterator myMapIterator = availableCalculationsMap.begin();
    myMapIterator = availableCalculationsMap.find(theCalculationNameString);

    if (myMapIterator != availableCalculationsMap.end())
    {
        myMapIterator.data()=true;
        //presume all went ok - need to add better error checking later
        return true;
    }
    else
    {
        return false;
    }
}





/** Read property of bool filesInSeriesFlag. */
const bool ClimateDataProcessor::getFilesInSeriesFlag()
{
    return filesInSeriesFlag;
}
/** Write property of bool filesInSeriesFlag. */
void ClimateDataProcessor::setFilesInSeriesFlag( const bool theFlag)
{
    filesInSeriesFlag = theFlag;
}

/** Start the data analysis process. When everything else is set up,
  this is the method to call! */
bool ClimateDataProcessor::run()
{
  std::cout << "ClimateDataProcessor run() method called.\n " << std::endl;
  std::cout << "Go and have a cup of tea cause this may take a while!" << std::endl;
  std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;

  //create a data processor to do the low level processing
  DataProcessor myDataProcessor;
  //work out how many variables we are going to calculate
  int myNumberOfVariablesInt = 0;
  QMap<QString, bool>::const_iterator myIter;
  for (myIter=availableCalculationsMap.begin(); myIter != availableCalculationsMap.end(); myIter++)
  {
    if (myIter.data()) //true
    {
      myNumberOfVariablesInt++;
    }
  }
  //
  //work out how many cells need to be processed for each calculations
  //
  int myNumberOfCells =0;
  int myXDimInt= 0;
  int myYDimInt=0;
  if (meanTempFileGroup)
  {
    myNumberOfCells = meanTempFileGroup->getElementCount();
    myXDimInt=meanTempFileGroup->getXDimInt();
    myYDimInt=meanTempFileGroup->getYDimInt();
  }
  else if (minTempFileGroup)
  {
    myNumberOfCells = minTempFileGroup->getElementCount();
    myXDimInt=minTempFileGroup->getXDimInt();
    myYDimInt=minTempFileGroup->getYDimInt();
  }
  else if (maxTempFileGroup)
  {
    myNumberOfCells = maxTempFileGroup->getElementCount();
    myXDimInt=maxTempFileGroup->getXDimInt();
    myYDimInt=maxTempFileGroup->getYDimInt();
  }
  else if (diurnalTempFileGroup)
  {
    myNumberOfCells = diurnalTempFileGroup->getElementCount();
    myXDimInt=diurnalTempFileGroup->getXDimInt();
    myYDimInt=diurnalTempFileGroup->getYDimInt();
  }
  else if (meanPrecipFileGroup)
  {
    myNumberOfCells = meanPrecipFileGroup->getElementCount();
    myXDimInt=meanPrecipFileGroup->getXDimInt();
    myYDimInt=meanPrecipFileGroup->getYDimInt();
  }
  else if (frostDaysFileGroup)
  {
    myNumberOfCells = frostDaysFileGroup->getElementCount();
    myXDimInt=frostDaysFileGroup->getXDimInt();
    myYDimInt=frostDaysFileGroup->getYDimInt();
  }
  else if (totalSolarRadFileGroup)
  {
    myNumberOfCells = totalSolarRadFileGroup->getElementCount();
    myXDimInt=totalSolarRadFileGroup->getXDimInt();
    myYDimInt=totalSolarRadFileGroup->getYDimInt();
  }
  else if (windSpeedFileGroup)
  {
    myNumberOfCells = windSpeedFileGroup->getElementCount();
    myXDimInt=windSpeedFileGroup->getXDimInt();
    myYDimInt=windSpeedFileGroup->getYDimInt();
  }
  //check nothing fishy is going on
  if (myNumberOfCells ==  0)
  {
    return false;
  }


  //send singals so progress monitors can set themselves up
  emit numberOfCellsToCalc(myNumberOfCells);
  emit numberOfVariablesToCalc(myNumberOfVariablesInt);


  //create a filewriter map for storing the OUTPUTS of each selected user calculation
  //this is not very element - I put the filewriter pointer into a struct and then  put the struct in to the
  //map, because I cant seem to be able to put the pointer directly into the map itself :-(
  QMap<QString, FileWriterStruct> myFileWriterMap;

  for (myIter=availableCalculationsMap.begin(); myIter != availableCalculationsMap.end(); myIter++)
  {
    if (myIter.data()) //true
    {
      std::cout << "Adding " <<  myIter.key() << " to myFileWriterMap\n";
      //create the fileWriter object
      //note I am not using pointer & the 'new' keyword here
      //because map doesnt let me add a pointer to the list

      QString myFileNameString = myIter.key();

      //replace any spaces with underscores in the name
      myFileNameString = myFileNameString.replace( QRegExp(" "), "_");
      //set the extension
      myFileNameString =  outputFilePathString + myFileNameString + ".asc";
      FileWriter * myFileWriter = new FileWriter(myFileNameString,outputFileType);

      //Use externally defined header if its been set
      if (!outputHeaderString.isEmpty())
      {
        myFileWriter->writeString(outputHeaderString);
      }
      //Otherwise calculate one dynamically
      else
      {
        // Use the matrix dimensions to create the ascii file
        // Warning: this assumes a GLOBAL dataset
        // Warning: this screws up cellsizes that are not square
        // Warning: this only works for integers at present
        QString myHeaderString=
            QString ("ncols         ") +
            QString::number (myXDimInt) + 
            QString ("\n")+
            QString ("nrows         ") + 
            QString::number (myYDimInt) + 
            QString ("\n")+
            QString ("xllcorner     -180\n")+
            QString ("yllcorner     -90\n")+
            QString ("cellsize      ") + 
            QString::number (360/static_cast<float>(myXDimInt)) +
            QString ("\n")+
            QString ("nodata_value  -9999.5\n");
        myFileWriter->writeString(myHeaderString);
        // Formerly this was fixed to the following
        //QString myHeaderString=
        //QString ("ncols         720\n")+
        //QString ("nrows         360\n")+
        //QString ("xllcorner     -180\n")+
        //QString ("yllcorner     -90\n")+
        //QString ("cellsize      0.5\n")+
        //QString ("nodata_value  -9999\n");                    myFileWriter->writeString(myHeaderString);

      }

      std::cout << "Added " << myFileWriter->getFileNameString() << std::endl;
      FileWriterStruct myFileWriterStruct;
      myFileWriterStruct.structFileWriter=myFileWriter;
      myFileWriterStruct.structFullFileName=myFileNameString;
      //add it to the map
      myFileWriterMap[myIter.key()]=myFileWriterStruct;
      std::cout << "Added " << myFileWriterStruct.structFullFileName << std::endl;
    }
  }


  // cycle through each FileGroup, fetching the element array from it and running any user
  // selected calculations, then writing the outputs to its associated filewriter in
  // myFileWriterMap


  if (meanTempFileGroup && meanTempFileNameString !="" &&
          availableCalculationsMap["Mean temperature"])
  {
    emit variableStart("Mean temperature");
    std::cout << "ClimateDataProcessor::run - Mean temperature requested" << std::endl;
    //move to start of the current data matrix
    meanTempFileGroup->moveToDataStart();

    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;

    int myXCountInt=0;
    bool myFirstIterationFlag=true;
    while (!meanTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverYear(myFloatVector );
      if (myFirstIterationFlag || meanTempFileGroup->getEndOfMatrixFlag())
      {
        //this next bit is just for debugging purposes"
        printVectorAndResult(myFloatVector,myFloat);
        myFirstIterationFlag=false;
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      ////emit cellDone(myFloat);
    }
    std::cout << " ++++++ Emitting variableDone signal! " << std::endl;
    emit variableDone(myFileWriterStruct.structFullFileName);

  }
  if (diurnalTempFileGroup && diurnalTempFileNameString != "" &&
          availableCalculationsMap["Annual mean diurnal temperature range"])
  {
    emit variableStart("Annual mean diurnal temperature range");
    std::cout << "ClimateDataProcessor::run Annual mean diurnal temperature range requested" << std::endl;
    //move to start of the current data matrix
    diurnalTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Annual mean diurnal temperature range"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!diurnalTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = diurnalTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverYear(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      ////emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }

  if (frostDaysFileGroup && frostDaysFileNameString != "" &&
          availableCalculationsMap["Annual mean number of frost days"])
  {
    emit variableStart("Annual mean number of frost days");
    std::cout << "ClimateDataProcessor::run Annual mean number of frost days requested" << std::endl;
    //move to start of the current data matrix
    frostDaysFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Annual mean number of frost days"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!frostDaysFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = frostDaysFileGroup->getElementVector();

      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverYear(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      ////emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }

  if (totalSolarRadFileGroup && totalSolarRadFileNameString != "" &&
          availableCalculationsMap["Annual mean total incident solar radiation"])
  {
    emit variableStart("Annual mean total incident solar radiation");
    std::cout << "ClimateDataProcessor::run Annual mean total incident solar radiation requested" << std::endl;
    //move to start of the current data matrix
    totalSolarRadFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Annual mean total incident solar radiation"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = totalSolarRadFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverYear(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }

  if ( minTempFileGroup
          && maxTempFileGroup
          && minTempFileNameString != ""
          && maxTempFileNameString != ""
          && availableCalculationsMap["Annual temperature range"])
  {

    emit variableStart("Annual temperature range");
    std::cout << "ClimateDataProcessor::run - Annual temperature range requested" << std::endl;
    //move to start of the current data matrix
    minTempFileGroup->moveToDataStart();
    maxTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Annual temperature range"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!minTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector, myFloatVector2;
      //get the next element from the file group
      myFloatVector = minTempFileGroup->getElementVector();
      myFloatVector2 = maxTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.greatestTotalRange(myFloatVector,myFloatVector2);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }

  if (maxTempFileGroup
          && maxTempFileNameString != ""
          && availableCalculationsMap["Highest temperature in warmest month"])
  {
    emit variableStart("Highest temperature in warmest month");
    std::cout << "ClimateDataProcessor::run - Highest temperature in warmest month requested" << std::endl;
    maxTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Highest temperature in warmest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!maxTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = maxTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.highestValue(myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }

  if (minTempFileGroup
          && minTempFileNameString != ""
          && availableCalculationsMap["Lowest temperature in coolest month"])
  {
    emit variableStart("Lowest temperature in coolest month");
    std::cout << "ClimateDataProcessor::run - Lowest temperature in coolest month requested" << std::endl;
    minTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Lowest temperature in coolest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!minTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = minTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.lowestValue(myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);

  }
  if (meanPrecipFileGroup
          && meanPrecipFileNameString != ""
          && availableCalculationsMap["Mean daily precipitation"])
  {
    emit variableStart("Mean daily precipitation");
    std::cout << "ClimateDataProcessor::run Mean daily precipitation" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverYear(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if ( meanPrecipFileGroup &&  minTempFileGroup
          && meanPrecipFileNameString != "" && meanTempFileNameString != ""
          && availableCalculationsMap["Mean daily precipitation in coolest month"])
  {
    emit variableStart("Mean daily precipitation in coolest month");
    std::cout << "ClimateDataProcessor::run Mean daily precipitation in coolest month" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in coolest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myBlockInt = myDataProcessor.monthWithLowestValue(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myBlockInt) != NO_DATA)
      {
          myFloat  = myDataProcessor.valueGivenMonth(myFloatVector,myBlockInt);
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanPrecipFileGroup &&  minTempFileGroup && meanPrecipFileNameString != ""
          && meanTempFileNameString != ""
          && availableCalculationsMap["Mean daily precipitation in coolest quarter"])
  {
    emit variableStart("Mean daily precipitation in coolest quarter");
    std::cout << "ClimateDataProcessor::run Mean daily precipitation in coolest month" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in coolest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myBlockInt = myDataProcessor.firstMonthOfLowestQ(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myBlockInt) != NO_DATA)
      {
         myFloat  = myDataProcessor.meanOverQuarter(myFloatVector,myBlockInt);
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  &&
          availableCalculationsMap["Mean daily precipitation in driest month"])
  {
    emit variableStart("Mean daily precipitation in driest month");
    std::cout << "ClimateDataProcessor::run Mean daily precipitation in driest month" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in driest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.lowestValue(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  &&
          availableCalculationsMap["Mean daily precipitation in driest quarter"])
  {
    emit variableStart("Mean daily precipitation in driest quarter");
    std::cout << "ClimateDataProcessor::run Mean daily precipitation in driest quarter" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in driest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverLowestQ(myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""
          && meanTempFileGroup
          && meanTempFileNameString != ""
          && availableCalculationsMap["Mean daily precipitation in warmest month"])
  {
    emit variableStart("Mean daily precipitation in warmest month");
    std::cout << "ClimateDataProcessor::run Mean daily precipitation in warmest month" << std::endl;
    //move to the start of data blocks
    meanPrecipFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in warmest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myBlockInt = myDataProcessor.monthWithHighestValue(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myBlockInt) != NO_DATA)
      {
        myFloat  = myDataProcessor.valueGivenMonth(myFloatVector,myBlockInt);
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""
          && meanTempFileGroup &&  meanTempFileNameString != ""
          &&availableCalculationsMap["Mean daily precipitation in warmest quarter"])
  {
    emit variableStart("Mean daily precipitation in warmest quarter");
    std::cout << "ClimateDataProcessor::run Mean daily precipitation in warmest quarter" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in warmest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myCurrentBlockOfWarmestQuarterInt = myDataProcessor.firstMonthOfHighestQ(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myCurrentBlockOfWarmestQuarterInt) != NO_DATA)
      {
        myFloat = myDataProcessor.meanOverQuarter(myFloatVector,myCurrentBlockOfWarmestQuarterInt);
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""   &&
          availableCalculationsMap["Mean daily precipitation in wettest month"])
  {
    emit variableStart("Mean daily precipitation in wettest month");
    std::cout << "ClimateDataProcessor::run Mean daily precipitation in wettest month" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in wettest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.highestValue(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""   &&
          availableCalculationsMap["Mean daily precipitation in wettest quarter"])
  {
    emit variableStart("Mean daily precipitation in wettest quarter");
    std::cout << "ClimateDataProcessor::run Mean daily precipitation in wettest quarter" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in wettest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float > myFloatVector;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverHighestQ(myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (diurnalTempFileGroup && diurnalTempFileNameString != ""
          && meanTempFileGroup
          && meanTempFileNameString !=""
          && availableCalculationsMap["Mean diurnal temperature range in coolest month"])
  {
    emit variableStart("Mean diurnal temperature range in coolest month");
    std::cout << "ClimateDataProcessor::run Mean diurnal temperature range in coolest month" << std::endl;
    diurnalTempFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean diurnal temperature range in coolest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!diurnalTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = diurnalTempFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myCoolestBlockInt = myDataProcessor.monthWithLowestValue(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myCoolestBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.valueGivenMonth(myFloatVector,myCoolestBlockInt);
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (diurnalTempFileGroup && diurnalTempFileNameString != "" && meanTempFileGroup
          && meanTempFileNameString !=""
          && availableCalculationsMap["Mean diurnal temperature range in warmest month"])
  {
    emit variableStart("Mean diurnal temperature range in warmest month");
    std::cout << "ClimateDataProcessor::run Mean diurnal temperature range in warmest month" << std::endl;
    diurnalTempFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean diurnal temperature range in warmest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!diurnalTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = diurnalTempFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myCoolestBlockInt = myDataProcessor.monthWithHighestValue(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myCoolestBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.valueGivenMonth(myFloatVector,myCoolestBlockInt);
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  && frostDaysFileGroup
          && frostDaysFileNameString != ""
          && availableCalculationsMap["Mean precipitation in frost free months"])
  {
    emit variableStart("Mean precipitation in frost free months");
    std::cout << "ClimateDataProcessor::run Mean precipitation in frost free months" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    frostDaysFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean precipitation in frost free months"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      myFloatVector2 = frostDaysFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanValueOverFrostFreeMonths(myFloatVector2, myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }

  if (meanTempFileGroup && meanTempFileNameString !="" &&
          availableCalculationsMap["Mean temperature in coolest month"])
  {
    emit variableStart("Mean temperature in coolest month");
    std::cout << "ClimateDataProcessor::run Mean temperature in coolest month" << std::endl;

    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in coolest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.lowestValue(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanTempFileGroup && meanTempFileNameString !="" &&
          availableCalculationsMap["Mean temperature in coolest quarter"])
  {
    emit variableStart("Mean temperature in coolest quarter");
    std::cout << "ClimateDataProcessor::run Mean temperature in coolest quarter" << std::endl;
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in coolest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverLowestQ(myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanTempFileGroup && meanTempFileNameString !="" && frostDaysFileGroup
          && frostDaysFileNameString != ""
          && availableCalculationsMap["Mean temperature in frost free months"])
  {
    emit variableStart("Mean temperature in frost free months");
    std::cout << "ClimateDataProcessor::run Mean temperature in frost free months" << std::endl;
    meanTempFileGroup->moveToDataStart();
    frostDaysFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in frost free months"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanTempFileGroup->getElementVector();
      myFloatVector2 = frostDaysFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanValueOverFrostFreeMonths(myFloatVector2, myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanTempFileGroup && meanTempFileNameString !="" &&
          availableCalculationsMap["Mean temperature in warmest month"])
  {
    emit variableStart("Mean temperature in warmest month");
    std::cout << "ClimateDataProcessor::run Mean temperature in warmest month" << std::endl;
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in warmest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.highestValue(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanTempFileGroup && meanTempFileNameString !="" &&
          availableCalculationsMap["Mean temperature in warmest quarter"])
  {
    emit variableStart("Mean temperature in warmest quarter");
    std::cout << "ClimateDataProcessor::run Mean temperature in warmest quarter" << std::endl;
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in warmest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverHighestQ(myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (windSpeedFileGroup && windSpeedFileNameString != "" &&
          availableCalculationsMap["Mean wind speed"])
  {
    emit variableStart("Mean wind speed");
    std::cout << "ClimateDataProcessor::run Mean wind speed" << std::endl;
    windSpeedFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean wind speed"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!windSpeedFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = windSpeedFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.meanOverYear(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (minTempFileGroup && minTempFileNameString !="" &&
          availableCalculationsMap["Number of months with minimum temperature above freezing"])
  {
    emit variableStart("Number of months with minimum temperature above freezing");
    std::cout << "ClimateDataProcessor::run Number of months with minimum temperature above freezing" << std::endl;
    minTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Number of months with minimum temperature above freezing"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!minTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = minTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.numberOfMonthsAboveZero(myFloatVector );
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
          && meanTempFileGroup && meanTempFileNameString !=""
          && availableCalculationsMap["Radiation in coolest month"])
  {
    emit variableStart("Radiation in coolest quarter");
    std::cout << "ClimateDataProcessor::run Radiation in coolest month" << std::endl;
    totalSolarRadFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in coolest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = totalSolarRadFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myCoolestBlockInt = myDataProcessor.monthWithLowestValue(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myCoolestBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.valueGivenMonth(myFloatVector,myCoolestBlockInt );
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
          && meanTempFileGroup && meanTempFileNameString !=""
          && availableCalculationsMap["Radiation in coolest quarter"])
  {
    emit variableStart("Radiation in coolest quarter");
    std::cout << "ClimateDataProcessor::run Radiation in coolest quarter" << std::endl;
    totalSolarRadFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in coolest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = totalSolarRadFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myFirstBlockInt = myDataProcessor.firstMonthOfLowestQ(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myFirstBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.meanOverQuarter(myFloatVector,myFirstBlockInt );
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
          && meanTempFileGroup && meanTempFileNameString != ""
          && availableCalculationsMap["Radiation in warmest month"])
  {
    emit variableStart("Radiation in warmest month");
    std::cout << "ClimateDataProcessor::run Radiation in warmest month" << std::endl;
    totalSolarRadFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in warmest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = totalSolarRadFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myHighestBlockInt = myDataProcessor.monthWithHighestValue(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myHighestBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.valueGivenMonth(myFloatVector,myHighestBlockInt );
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
          && meanTempFileGroup && meanTempFileNameString != ""
          && availableCalculationsMap["Radiation in warmest quarter"])
  {
    emit variableStart("Radiation in warmest quarter");
    std::cout << "ClimateDataProcessor::run Radiation in warmest quarter" << std::endl;
    totalSolarRadFileGroup->moveToDataStart();
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in warmest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
    {

      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = totalSolarRadFileGroup->getElementVector();
      myFloatVector2 = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      int myFirstBlockInt = myDataProcessor.firstMonthOfHighestQ(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myFirstBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.meanOverQuarter(myFloatVector,myFirstBlockInt );
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
          && meanPrecipFileGroup && meanPrecipFileNameString != ""
          && availableCalculationsMap["Radiation in driest month"])
  {
    emit variableStart("Radiation in driest month");
    std::cout << "ClimateDataProcessor::run Radiation in driest month" << std::endl;
    totalSolarRadFileGroup->moveToDataStart();
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in driest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = totalSolarRadFileGroup->getElementVector();
      myFloatVector2 = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      int myDriestBlockInt = myDataProcessor.monthWithLowestValue(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myDriestBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.valueGivenMonth(myFloatVector,myDriestBlockInt );
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
          && meanPrecipFileGroup && meanPrecipFileNameString != ""
          && availableCalculationsMap["Radiation in driest quarter"])
  {
    emit variableStart("Radiation in driest quarter");
    std::cout << "ClimateDataProcessor::run Radiation in driest quarter" << std::endl;
    totalSolarRadFileGroup->moveToDataStart();
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in driest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = totalSolarRadFileGroup->getElementVector();
      myFloatVector2 = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      int myFirstBlockInt = myDataProcessor.firstMonthOfLowestQ(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myFirstBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.meanOverQuarter(myFloatVector,myFirstBlockInt );
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }

  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
          && meanPrecipFileGroup && meanPrecipFileNameString != ""
          && availableCalculationsMap["Radiation in wettest month"])
  {
    emit variableStart("Radiation in wettest month");
    std::cout << "ClimateDataProcessor::run Radiation in wettest month" << std::endl;
    totalSolarRadFileGroup->moveToDataStart();
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in wettest month"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = totalSolarRadFileGroup->getElementVector();
      myFloatVector2 = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      int myWettestBlockInt = myDataProcessor.monthWithHighestValue(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myWettestBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.valueGivenMonth(myFloatVector,myWettestBlockInt );
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
          && meanPrecipFileGroup && meanPrecipFileNameString != ""
          && availableCalculationsMap["Radiation in wettest quarter"])
  {
    emit variableStart("Radiation in wettest quarter");
    std::cout << "ClimateDataProcessor::run Radiation in wettest quarter" << std::endl;
    totalSolarRadFileGroup->moveToDataStart();
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in wettest quarter"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector,myFloatVector2;
      //get the next element from the file group
      myFloatVector = totalSolarRadFileGroup->getElementVector();
      myFloatVector2 = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      int myFirstBlockInt = myDataProcessor.firstMonthOfHighestQ(myFloatVector2);
      float myFloat=NO_DATA;
      if (static_cast<int>(myFirstBlockInt) != NO_DATA)
      {
        myFloat = myDataProcessor.meanOverQuarter(myFloatVector,myFirstBlockInt );
      }
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanPrecipFileGroup && meanPrecipFileNameString != "" &&
          availableCalculationsMap["Standard deviation of mean precipitation"])
  {
    emit variableStart("Standard deviation of mean precipitation");
    std::cout << "ClimateDataProcessor::run Standard deviation of mean precipitation" << std::endl;
    meanPrecipFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Standard deviation of mean precipitation"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanPrecipFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = meanPrecipFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.stddevOverYear(myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  if (meanTempFileGroup && meanTempFileNameString != ""
          && availableCalculationsMap["Standard deviation of mean temperature"])
  {
    emit variableStart("Standard deviation of mean temperature");
    std::cout << "ClimateDataProcessor::run Standard deviation of mean temperature" << std::endl;
    meanTempFileGroup->moveToDataStart();
    //get the struct containing the filewriter pointer and full file name from the writer map
    FileWriterStruct myFileWriterStruct = myFileWriterMap["Standard deviation of mean temperature"];
    //get the filewriter from out of the struct
    FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
    int myXCountInt=0;
    while (!meanTempFileGroup->getEndOfMatrixFlag())
    {
      QValueVector<float> myFloatVector;
      //get the next element from the file group
      myFloatVector = meanTempFileGroup->getElementVector();
      //we are using mean over year summary
      float myFloat = myDataProcessor.stddevOverYear(myFloatVector);
      //write the result to our output file
      bool myResultFlag = myFileWriter->writeElement(myFloat);
      if (!myResultFlag)
      {
        std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
        
        return false;
      }
      myXCountInt++;
      if (myXCountInt%myXDimInt==0)
      {
        myFileWriter->writeString("\n");
      }
      //emit cellDone(myFloat);
    }
    emit variableDone(myFileWriterStruct.structFullFileName);
  }
  return true;
}

void ClimateDataProcessor::printVectorAndResult(QValueVector<float> theVector, float theResultFloat)
{
    int myVectorLengthInt=theVector.size();
    for (int myInt = 0; myInt <= myVectorLengthInt-1; myInt++)
    {

        std::cout << theVector[myInt] << ",";
    }
    std::cout << "\t : " << theResultFloat << std::endl;
}

/** Return a nice summary about this ClimateDataProcessor object */
QString ClimateDataProcessor::getDescription()
{
    //must have an include for sstream.h!

    QString myString, myNumberString;
    myString += "\n Climate Data Processor Description \n";
    myString += " ---------------------------------- \n";


    myNumberString = QString::number(getInputFileType());
    myString += QString("Input File Type Enum : ") + myNumberString+ QString("\n");

    myNumberString = QString::number(getOutputFileType());
    myString += QString("Output File Type Enum : ") + myNumberString + QString("\n");


    //these properties are just plain strings and dont need conversion
    myString += QString("Mean Temp FileName : ") + getMeanTempFileName() + QString("\n");
    myString += QString("Max Temp FileName : ") + getMaxTempFileName() + QString("\n");
    myString += QString("Min Temp FileName : ") + getMinTempFileName() + QString("\n");
    myString += QString("Diurnal Temp FileName : ") + getDiurnalTempFileName() + QString("\n");
    myString += QString("Mean Precipitation FileName : ") + getMeanPrecipFileName() + QString("\n");
    myString += QString("Frost Days FileName : ") + getFrostDaysFileName() + QString("\n");
    myString += QString("Total Solar Radiation FileName : ") + getTotalSolarRadFileName() + QString("\n");
    myString += QString("Wind Speed FileName : ") + getWindSpeedFileName() + QString("\n");
    if (filesInSeriesFlag)
    {
        myString += QString("Datafiles are a series of numbered files for each month \n");
    }
    else
    {
        myString += QString("Datafiles contain all monthly data in a single file \n");
    }

    //List the calculations in  availableCalculationsMap  using an iterator
    myString += QString("Listing items in availableCalculationsMap \n");
    myString += QString("Boolean value suffix indicates whether the user want to use this calculation \n");
    myString += QString("---------------------------------------------------------------------------------------------------- \n");
    QMap<QString, bool>::const_iterator myIter;
    for (myIter=availableCalculationsMap.begin(); myIter != availableCalculationsMap.end(); myIter++)
    {
        if (myIter.data())
        {
            myString += myIter.key() + QString(": true\n");
        }
        else
        {
            myString += myIter.key() + QString(": false\n");
        }
    }
    myString += QString("---------------------------------------------------------------------------------------------------- \n");

    return myString;

}




