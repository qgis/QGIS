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
#include <map>
#include <string>
#include <iostream>
#include <sstream> //used to convert from a string to an int
#include <algorithm>
#include <cctype>

// using namespace std; --- dont use this - stl transforms will break, rather use the namespace like this std::

//this struct is used in the 'run' method
struct FileWriterStruct {FileWriter * structFileWriter; std::string structFullFileName;};



ClimateDataProcessor::ClimateDataProcessor()
{
  std::cout << "Climate Data Processor constructor called." << std::endl;

  // initialise the fileGroup file names  - not strictly needed but neater
  std::string myString=std::string("");
  meanTempFileNameString=myString;
  minTempFileNameString=myString;
  maxTempFileNameString=myString;
  diurnalTempFileNameString=myString;
  meanPrecipFileNameString=myString;
  frostDaysFileNameString=myString;
  totalSolarRadFileNameString=myString;
  windSpeedFileNameString=myString;


}
ClimateDataProcessor::ClimateDataProcessor(
        int theFileStartYear,
        int theJobStartYear,
        int theJobEndYear,
        std::string theInputFileTypeString,
        std::string theOutputFileTypeString)
{

  std::cout << "Climate Data Processor constructoror called." << std::endl;
}
/** Destructor */
ClimateDataProcessor::~ClimateDataProcessor(){
}


/** Read property of std::string outputHeaderString. */
const std::string& ClimateDataProcessor::getOutputHeaderString()
{
  return outputHeaderString;
}
/** Write property of std::string outputHeaderString. */
void ClimateDataProcessor::setOutputHeaderString( const std::string& theOutputHeaderString)
{
  outputHeaderString = theOutputHeaderString;
}

/**This is a private method. It is asimple method to populate the
 * fileTypeMap attribute - this will usually be called by the
 * constructor(s). All keys (file type strings) will be  stored in upper case.*/

bool ClimateDataProcessor::makeInputFileTypeMap()
{
  std::string myString;
  FileReader::FileTypeEnum myEnum;
  inputFileTypeMap.clear();
  // Copied from FileReader header file:
  //enum FileTypeEnum { CRES,  ARCINFO_GRID , HADLEY_SRES , HADLEY_IS92 ,  IPCC_OBSERVED ,
  //                                    VALDES ,  ECHAM4 ,  CSIRO_MK2 ,  NCAR_CSM_PCM , GFDL_R30 , CGCM2 ,
  //                                    CCSR_AGCM_OGCM };

  //declare the key value
  myString=std::string("CRES African climate data");
  //convert it to upper case
  std::transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::CRES;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("ESRI & ASCII raster");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::ARCINFO_GRID;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("Hadley Centre HadCM3 SRES Scenario");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::HADLEY_SRES;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("Hadley Centre HadCM3 IS92a Scenario");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::HADLEY_IS92;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("IPCC Observed Climatology");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::IPCC_OBSERVED;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("University of Reading Palaeoclimate data");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::VALDES;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("Max Planck Institute fur Meteorologie (MPIfM) ECHAM4 data");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::ECHAM4;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("CSIRO-Mk2 Model data");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::CSIRO_MK2;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("National Center for Atmospheric Research (NCAR) NCAR-CSM and NCAR-PCM data");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::NCAR_CSM_PCM;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("Geophysical Fluid Dynamics Laboratory (GFDL) R30 Model data");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::GFDL_R30;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("Canadian Center for Climate Modelling and Analysis (CCCma) CGCM2 Model data");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::CGCM2;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("CCSR/NIES AGCM model data and CCSR OGCM model data");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileReader::CCSR_AGCM_OGCM;
  //add it to the associative array
  inputFileTypeMap[myString]=myEnum;

  return true;
}

/**This is a private method. It is a simple method to populate the
 * outputFileTypeMap attribute - this will usually be called by the
 * constructor(s). All keys (file type strings) will be  stored in upper case.*/

bool ClimateDataProcessor::makeOutputFileTypeMap()
{
  std::string myString;
  FileWriter::FileTypeEnum myEnum;
  outputFileTypeMap.clear();
  // Copied from FileWriter header file:
  //enum FileFormatEnum { CSM_MATLAB , CSM_OCTAVE ,  GARP ,  ESRI_ASCII ,  PLAIN };


  //declare the key value
  myString=std::string("CSM for Matlab");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileWriter::CSM_MATLAB;
  //add it to the associative array
  outputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("CSM for Octave");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileWriter::CSM_OCTAVE;
  //add it to the associative array
  outputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("Desktop GARP");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileWriter::GARP;
  //add it to the associative array
  outputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("ESRI ASCII Grid");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileWriter::ESRI_ASCII;
  //add it to the associative array
  outputFileTypeMap[myString]=myEnum;

  //declare the key value
  myString=std::string("Plain matrix with no header");
  //convert it to upper case
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
  //set its associated enum
  myEnum=FileWriter::PLAIN;
  //add it to the associative array
  outputFileTypeMap[myString]=myEnum;
  return true;
}





/** Read property of FileNameString  meanTempFileNameString. */
const std::string  ClimateDataProcessor::getMeanTempFileName(){
  return meanTempFileNameString;
}
/** Write property of FileNameString  meanTempFileNameString. */
void ClimateDataProcessor::setMeanTempFileName( std::string theFileNameString)
{
  meanTempFileNameString = theFileNameString;
  std::cout << "meanTempFileNameString set to : " << meanTempFileNameString << std::endl;
}
/** Read property of FileNameString  minTempFileNameString. */
const std::string  ClimateDataProcessor::getMinTempFileName()
{
  return minTempFileNameString;
}
/** Write property of FileNameString  minTempFileNameString. */
void ClimateDataProcessor::setMinTempFileName( std::string theFileNameString)
{
  minTempFileNameString = theFileNameString;
  std::cout << "minTempFileNameString set to : " << minTempFileNameString << std::endl;
}
/** Read property of FileNameString  maxTempFileNameString. */
const std::string  ClimateDataProcessor::getMaxTempFileName()
{
  return maxTempFileNameString;
}
/** Write property of FileNameString  maxTempFileNameString. */
void ClimateDataProcessor::setMaxTempFileName( std::string theFileNameString)
{
  maxTempFileNameString = theFileNameString;
  std::cout << "maxTempFileNameString set to : " << maxTempFileNameString << std::endl;
}
/** Read property of FileNameString  diurnalTempFileNameString. */
const std::string  ClimateDataProcessor::getDiurnalTempFileName()
{
  return diurnalTempFileNameString;
}
/** Write property of FileNameString  diurnalTempFileNameString. */
void ClimateDataProcessor::setDiurnalTempFileName( std::string theFileNameString)
{
  diurnalTempFileNameString = theFileNameString;
  std::cout << "diurnalTempFileNameString set to : " << diurnalTempFileNameString << std::endl;
}
/** Read property of FileNameString  meanPrecipFileNameString. */
const std::string  ClimateDataProcessor::getMeanPrecipFileName()
{
  return meanPrecipFileNameString;
}
/** Write property of FileNameString  meanPrecipFileNameString. */
void ClimateDataProcessor::setMeanPrecipFileName( std::string theFileNameString)
{
  meanPrecipFileNameString = theFileNameString;
  std::cout << "meanPrecipFileNameString set to : " << meanPrecipFileNameString << std::endl;
}
/** Read property of FileNameString  frostDaysFileNameString. */
const std::string  ClimateDataProcessor::getFrostDaysFileName()
{
  return frostDaysFileNameString;
  std::cout << "frostDaysFileNameString set to : " << frostDaysFileNameString << std::endl;
}
/** Write property of FileNameString  frostDaysFileNameString. */
void ClimateDataProcessor::setFrostDaysFileName( std::string theFileNameString)
{
  frostDaysFileNameString = theFileNameString;
  std::cout << "frostDaysFileNameString set to : " << frostDaysFileNameString << std::endl;
}
/** Read property of FileNameString  totalSolarRadFileNameString. */
const std::string  ClimateDataProcessor::getTotalSolarRadFileName()
{
  return totalSolarRadFileNameString;
}
/** Write property of FileNameString  totalSolarRadFileNameString. */
void ClimateDataProcessor::setTotalSolarRadFileName( std::string theFileNameString)
{
  totalSolarRadFileNameString = theFileNameString;

  std::cout << "totalSolarRadFileNameString set to : " << totalSolarRadFileNameString << std::endl;
}
/** Read property of FileNameString  windSpeedFileNameString. */
const std::string  ClimateDataProcessor::getWindSpeedFileName()
{
  return windSpeedFileNameString;
}
/** Write property of FileNameString  windSpeedFileNameString. */
void ClimateDataProcessor::setWindSpeedFileName( std::string theFileNameString)
{
  windSpeedFileNameString = theFileNameString;
  std::cout << "windSpeedFileNameString set to : " << windSpeedFileNameString << std::endl;
}
/** Read property of std::string  outputFilePathString. */
const std::string  ClimateDataProcessor::getOutputFilePathString()
{
  return outputFilePathString;
}
/** Write property of std::string  outputFilePathString. */
void ClimateDataProcessor::setOutputFilePathString( std::string theFilePathString)
{
  outputFilePathString = theFilePathString;
  std::cout << "outputFilePathString set to : " << outputFilePathString << std::endl;
}

/**  Set up the filegroups for each FileName that has been registered
 *    @param theStartYearInt Base 1 value used when processing data in series to shift the
 *           internal file pointer to the correct year. Default is 1.
 */
bool ClimateDataProcessor::makeFileGroups(int theStartYearInt=1)
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

  if (meanTempFileNameString==std::string(""))
  {
    std::cout <<     "makeFileGroups - meanTempFileNameString is NOT initialised! *****************************" << std::endl;
  }
  else
  {
    meanTempFileGroup = initialiseFileGroup(meanTempFileNameString,theStartYearInt);
  }

  if (minTempFileNameString==std::string(""))
  {
    std::cout <<     "makeFileGroups - minTempFileNameString is NOT initialised! *****************************" << std::endl;
  }
  else
  {
    minTempFileGroup = initialiseFileGroup(minTempFileNameString,theStartYearInt);
  }

  if (maxTempFileNameString==std::string(""))
  {
    std::cout <<     "makeFileGroups - maxTempFileNameString is NOT initialised! *****************************" << std::endl;
  }
  else
  {
    maxTempFileGroup = initialiseFileGroup(maxTempFileNameString,theStartYearInt);
  }

  if (diurnalTempFileNameString==std::string(""))
  {
    std::cout <<     "makeFileGroups - diurnalTempFileNameString is NOT initialised! *****************************" << std::endl;
  }
  else
  {
    diurnalTempFileGroup = initialiseFileGroup(diurnalTempFileNameString,theStartYearInt);
  }

  if (meanPrecipFileNameString==std::string(""))
  {
    std::cout <<     "makeFileGroups - meanPrecipFileNameString is NOT initialised! *****************************" << std::endl;
  }
  else
  {
    meanPrecipFileGroup = initialiseFileGroup(meanPrecipFileNameString,theStartYearInt);
  }

  if (meanPrecipFileNameString==std::string(""))
  {
    std::cout <<     "makeFileGroups - meanPrecipFileNameString is NOT initialised! *****************************" << std::endl;
  }
  else
  {
    meanPrecipFileGroup = initialiseFileGroup(meanPrecipFileNameString,theStartYearInt);
  }


  if (frostDaysFileNameString==std::string(""))
  {
    std::cout <<     "makeFileGroups - frostDaysFileNameString is NOT initialised! *****************************" << std::endl;
  }
  else
  {
    frostDaysFileGroup = initialiseFileGroup(frostDaysFileNameString,theStartYearInt);
  }


  if (totalSolarRadFileNameString==std::string(""))
  {
    std::cout << "makeFileGroups - totalSolarRadFileNameString is NOT initialised! *****************************" << std::endl;
  }
  else
  {
    totalSolarRadFileGroup = initialiseFileGroup(totalSolarRadFileNameString,theStartYearInt);
  }


  if (windSpeedFileNameString==std::string(""))
  {
    std::cout << "makeFileGroups - totalSolarRadFileNameString is NOT initialised! *****************************" << std::endl;
  }
  else
  {
    windSpeedFileGroup = initialiseFileGroup(windSpeedFileNameString,theStartYearInt);
  }



  /* // this is just for debugging purposes
     std::vector <float> myVector = meanTempFileGroup->getElementVector();
     for (int i=0;i < myVector.size(); i++)
     {
     std::cout << myVector[i] << ",";
     }
     std::cout << std::endl;

*/
  //presume all went ok - need to add better error checking later

  return true;
}
/*   Construct a filegroup given a base file name.
 *   @param theStartYearInt Used when processing data NOT in series to shift the
 *           internal file pointer to the correct year. Note this is a base 1 number.
 *           Default is 1.
 */
FileGroup * ClimateDataProcessor::initialiseFileGroup(std::string theFileNameString,int theStartYearInt=1)
{
  FileGroup *  myFileGroup = new FileGroup();
  std::cout << "initialiseFileGroup - theFileNameString is initialised to : " << theFileNameString << std::endl;
  if (filesInSeriesFlag)
  {
    //we have a separate file for each months data - the
    //assumption we make then is that the user selected the first file
    //e.g. file_01.asc and that the remaining files are numbered sequentially
    std::cout << "initialiseFileGroup - files in series!" << std::endl;
    //create a file group from this file
    std::cout << "initialiseFileGroup - theFileNameString = " << theFileNameString << std::endl;
    //need to add some error handling here...


    for (int myInt=2; myInt <= 13; myInt++)
    {

      char myNumberPartChar[10];
      //get a string showing the FileName part we want to replace
      //I know this is really crappy code, but it works. it late
      //and I am too lazy to write it better :-)
      if (myInt < 11)
      {
        int myFileNoInt = myInt;
        sprintf(myNumberPartChar, "0%i.",myFileNoInt-1);
      }
      else if(myInt<100)
      {
        int myFileNoInt = myInt;
        sprintf(myNumberPartChar, "%i.",myFileNoInt-1);
      }
      std::string myNumberPartString(myNumberPartChar);

      std::string myCurrentFileNameString = theFileNameString;
      int myPosInt = myCurrentFileNameString.find_last_of(myNumberPartString);
      myPosInt = myPosInt - 2; //big time kludge here :-)
      myCurrentFileNameString.replace(myPosInt,sizeof(myNumberPartString)-1,myNumberPartString);
      std::cout << "initialiseFileGroup - opening file : " << myCurrentFileNameString << std::endl;
      FileReader *myFileReader = new FileReader(myCurrentFileNameString,inputFileType);
      myFileReader->setFileName( myCurrentFileNameString.c_str() );
      myFileReader->getBlockMarkers();

      myFileReader->moveToDataStart();        
      std::cout << "initialiseFileGroup - *** Adding " << myCurrentFileNameString
          << " to file group *********************" << std::endl;
      myFileGroup->addFileReader(myFileReader);                        

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
    FileReader *myFileReader = new FileReader(theFileNameString,inputFileType);
    //when we open the first filereader in the, we find
    //the block markers which will then be  assicgned to all other
    //filereaders we open as the filereaders all use the same file
    std::vector <fpos_t>* myDataBlockMarkersVector = myFileReader->getBlockMarkers();
    //calculate the actual blocks numbers represented by the start & end years
    int myCurrentBlockInt = ((theStartYearInt-1)*12);
    int myEndBlockInt = (12*theStartYearInt);

    printf ("ClimatteDataProcessor::initialiseFileGroup Calculated start block : %i and end block is %i\n",
            myCurrentBlockInt,
            myEndBlockInt
           );

    //now we loop through 12 blocks to create our filegroup
    //this is the loop that sets up file readers - one per data
    //block for the required year, for the current month.
    for (int myInt=myCurrentBlockInt; myInt <= myEndBlockInt; ++myInt)
    {
      FileReader *myFileReader2 = new FileReader(theFileNameString.c_str(),FileReader::HADLEY_SRES);
      myFileReader2->setBlockMarkers(myDataBlockMarkersVector);
      myFileReader2->setCurrentBlock(myInt);
      myFileGroup->addFileReader(myFileReader2);
      std::cout << "Adding block " << myInt << " to the block list to be processed" << std::endl;
    }


    //clean up
    delete myFileReader;

    return myFileGroup;
  } //end of second part of files in series flag test

}
/** Read property of int fileStartYearInt. */
const int ClimateDataProcessor::getFileStartYearInt()
{
  return fileStartYearInt;
}
/** Write property of int fileStartYearInt. */
void ClimateDataProcessor::setFileStartYearInt( const int theYearInt)
{
  fileStartYearInt = theYearInt;
  std::cout << "fileStartYearInt set to : " << fileStartYearInt << std::endl;
}


/** Read property of int jobStartYearInt. */
const int ClimateDataProcessor::getJobStartYearInt()
{
  return jobStartYearInt;
}
/** Write property of int jobStartYearInt. */
void ClimateDataProcessor::setJobStartYearInt( const int theYearInt)
{
  jobStartYearInt = theYearInt;
  std::cout << "jobStartYearInt set to : " << jobStartYearInt << std::endl;
}


/** Read property of int jobEndYearInt. */
const int ClimateDataProcessor::getJobEndYearInt()
{
  return jobEndYearInt;
}
/** Write property of int jobEndYearInt. */
void ClimateDataProcessor::setJobEndYearInt( const int theYearInt)
{
  jobEndYearInt = theYearInt;
}

/** Read property of FileReader::FileType FileType. */
const FileReader::FileTypeEnum ClimateDataProcessor::getInputFileType(){
  return inputFileType;
}
/** Write property of FileReader::FileType FileType. */
void ClimateDataProcessor::setInputFileType( const FileReader::FileTypeEnum theInputFileType){
  inputFileType = theInputFileType;
}

/** Overloaded version of above that taks a string and looks up the enum */
void ClimateDataProcessor::setInputFileType( const std::string theInputFileTypeString)
{


  std::string myString = theInputFileTypeString;
  FileReader::FileTypeEnum myInputFileTypeEnum;
  //make sure the fileTypeMap exists
  makeInputFileTypeMap();
  //convert the input string to ucase
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
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
void ClimateDataProcessor::setOutputFileType( const FileWriter::FileTypeEnum theOutputFileType){
  outputFileType = theOutputFileType;
}

/** Overloaded version of above that taks a string and looks up the enum */
void ClimateDataProcessor::setOutputFileType( const std::string theOutputFileTypeString)
{


  std::string myString = theOutputFileTypeString;
  FileWriter::FileTypeEnum myOutputFileTypeEnum;
  //make sure the fileTypeMap exists
  makeOutputFileTypeMap();
  //convert the Output string to ucase
  transform (myString.begin(),myString.end(), myString.begin(), toupper);
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
  std::string myString;
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

  if (diurnalTempFileGroup && diurnalTempFileNameString != "")
  {
    //declare the key value
    myString=std::string("Annual mean diurnal temperature range");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }

  if (frostDaysFileGroup && frostDaysFileNameString != "")
  {
    //declare the key value
    myString=std::string("Annual mean number of frost days");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }

  if (totalSolarRadFileGroup && totalSolarRadFileNameString != "")
  {
    std::cout << "Solar incident radiation : " <<    totalSolarRadFileNameString << std::endl;
    //declare the key value
    myString=std::string("Annual mean total incident solar radiation");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }

  if ( minTempFileGroup &&  maxTempFileGroup && minTempFileNameString  != "" && maxTempFileNameString != "" )
  {
    //declare the key value
    myString=std::string("Annual temperature range");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }

  if (maxTempFileGroup && maxTempFileNameString != "")
  {
    //declare the key value
    myString=std::string("Highest temperature in warmest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }

  if (minTempFileGroup && minTempFileNameString != "")
  {
    //declare the key value
    myString=std::string("Lowest temperature in coolest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup && meanPrecipFileNameString != "")
  {
    //declare the key value
    myString=std::string("Mean daily precipitation");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if ( meanPrecipFileGroup &&  minTempFileGroup && meanPrecipFileNameString != "" && minTempFileNameString != "" )
  {
    //declare the key value
    myString=std::string("Mean daily precipitation in coolest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup &&  minTempFileGroup && meanPrecipFileNameString != "" && minTempFileNameString != "" )
  {
    //declare the key value
    myString=std::string("Mean daily precipitation in coolest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != "" )
  {
    //declare the key value
    myString=std::string("Mean daily precipitation in driest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != "" )
  {
    //declare the key value
    myString=std::string("Mean daily precipitation in driest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  && maxTempFileGroup &&  maxTempFileNameString != ""  )
  {
    //declare the key value
    myString=std::string("Mean daily precipitation in warmest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  && maxTempFileGroup &&  maxTempFileNameString != ""  )
  {
    //declare the key value
    myString=std::string("Mean daily precipitation in warmest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  )
  {
    //declare the key value
    myString=std::string("Mean daily precipitation in wettest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  )
  {
    //declare the key value
    myString=std::string("Mean daily precipitation in wettest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (diurnalTempFileGroup && diurnalTempFileNameString != "" && meanTempFileGroup && meanTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Mean diurnal temperature range in coolest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (diurnalTempFileGroup && diurnalTempFileNameString != "" && meanTempFileGroup && meanTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Mean diurnal temperature range in warmest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  && frostDaysFileGroup && frostDaysFileNameString != "")
  {
    //declare the key value
    myString=std::string("Mean precipitation in frost free months");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanTempFileGroup && meanTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Mean temperature");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanTempFileGroup && meanTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Mean temperature in coolest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanTempFileGroup && meanTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Mean temperature in coolest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanTempFileGroup && meanTempFileNameString !="" && frostDaysFileGroup && frostDaysFileNameString != "")
  {
    //declare the key value
    myString=std::string("Mean temperature in frost free months");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanTempFileGroup && meanTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Mean temperature in warmest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanTempFileGroup && meanTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Mean temperature in warmest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (windSpeedFileGroup && windSpeedFileNameString != "")
  {
    //declare the key value
    myString=std::string("Mean wind speed");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (minTempFileGroup && minTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Number of months with minimum temperature above freezing");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != "" && meanTempFileGroup && meanTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Radiation in coolest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != "" && meanTempFileGroup && meanTempFileNameString !="")
  {
    //declare the key value
    myString=std::string("Radiation in coolest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""  && meanPrecipFileGroup && meanPrecipFileNameString != "")
  {
    //declare the key value
    myString=std::string("Radiation in driest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""  && meanPrecipFileGroup && meanPrecipFileNameString != "")
  {
    //declare the key value
    myString=std::string("Radiation in driest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""  && meanTempFileGroup && meanTempFileNameString != "")
  {
    //declare the key value
    myString=std::string("Radiation in warmest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""  && meanTempFileGroup && meanTempFileNameString != "")
  {
    //declare the key value
    myString=std::string("Radiation in warmest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""  && meanPrecipFileGroup && meanPrecipFileNameString != "")
  {
    //declare the key value
    myString=std::string("Radiation in wettest month");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""  && meanPrecipFileGroup && meanPrecipFileNameString != "")
  {
    //declare the key value
    myString=std::string("Radiation in wettest quarter");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanPrecipFileGroup && meanPrecipFileNameString != "")
  {
    //declare the key value
    myString=std::string("Standard deviation of mean precipitation");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }
  if (meanTempFileGroup && meanTempFileNameString != "")
  {
    //declare the key value
    myString=std::string("Standard deviation of mean temperature");
    //add it to the associative array
    availableCalculationsMap[myString]=myBool;
  }

  //presume all went ok - need to add better error checking later
  return true;
} //end of makeAvailableCalculationsMap

/** Return the map of available calculations */
std::map <std::string, bool > ClimateDataProcessor::getAvailableCalculationsMap()
{

  return  availableCalculationsMap;

}

/**  Add a calculation to the list of those requested to be carried out by the user */
bool ClimateDataProcessor::addUserCalculation(std::string theCalculationNameString)
{
  std::map<std::string, bool>::iterator myMapIterator = availableCalculationsMap.begin();
  myMapIterator = availableCalculationsMap.find(theCalculationNameString.c_str());

  if (myMapIterator != availableCalculationsMap.end())
  {
    myMapIterator->second=true;
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

  //This is the MAIN OUTER LOOP:
  //cycle through each year block, doing all the requested calculations for that year
  std::cout << "Year offsets. Note that this code does not currently implement" << std::endl;
  std::cout << "support for paleaoclimate data or any data before 0AD" << std::endl;
  std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
  int myOffsetInt = jobStartYearInt - fileStartYearInt;
  int myNumberOfIterationsInt = (jobEndYearInt - jobStartYearInt) +1;
  int myCurrentYearInt = jobStartYearInt; //this will be changed on each iteration
  bool myFirstLoopFlag = true;
  for (int myCDPMainLoopInt = jobStartYearInt;
          myCDPMainLoopInt < (jobEndYearInt +1);
          myCDPMainLoopInt ++)
  {

    if (myFirstLoopFlag)
    {
      myFirstLoopFlag=false;
    }
    else
    {
      //shift all the filegroups' filereaders' startmonths on a year

      if (meanTempFileGroup) meanTempFileGroup->incrementDataBlocks(12);
      if (minTempFileGroup)  minTempFileGroup->incrementDataBlocks(12);
      if (maxTempFileGroup)  maxTempFileGroup->incrementDataBlocks(12);
      if (diurnalTempFileGroup)  diurnalTempFileGroup->incrementDataBlocks(12);
      if (meanPrecipFileGroup)      meanPrecipFileGroup->incrementDataBlocks(12);
      if (frostDaysFileGroup)     frostDaysFileGroup->incrementDataBlocks(12);
      if (totalSolarRadFileGroup)  totalSolarRadFileGroup->incrementDataBlocks(12);
      if (windSpeedFileGroup)  windSpeedFileGroup->incrementDataBlocks(12);

    }
    //create a filewriter map for storing the OUTPUTS of each selected user calculation
    //this is not very element - I put the filewriter pointer into a struct and then  put the struct in to the
    //map, because I cant seem to be able to put the pointer directly into the map itself :-(
    std::map<std::string, FileWriterStruct> myFileWriterMap;

    std::map<std::string, bool>::const_iterator myIter;
    for (myIter=availableCalculationsMap.begin(); myIter != availableCalculationsMap.end(); myIter++)
    {
      if (myIter->second) //true
      {
        std::cout << "Adding " <<  myIter->first << " to myFileWriterMap\n";
        //create the fileWriter object
        //note I am not using pointer & the 'new' keyword here
        //because map doesnt let me add a pointer to the list

        std::string myFileNameString = myIter->first;
        std::string myYearString;
        //convert the active year to a number -
        //the same caveats as mentioned above applyin that we presume all data
        //is AD at this stage - need to imlement bc support still
        char myYearChar[7];
        if ( myCDPMainLoopInt >=0)
        {
          sprintf(myYearChar,"%iAD",myCDPMainLoopInt);
        }
        else
        {
          //loop logic still needs to be implemented properly for this to work!
          sprintf(myYearChar,"%iBC",myCDPMainLoopInt);
        }
        myFileNameString =  outputFilePathString + myFileNameString + "_" + myYearChar +".asc";
        FileWriter * myFileWriter = new FileWriter(myFileNameString,outputFileType);
        //write a standard header if one is available

        myFileWriter->writeHeader(outputHeaderString);


        std::cout << "Added " << myFileWriter->getFileNameString() << std::endl;
        FileWriterStruct myFileWriterStruct;
        myFileWriterStruct.structFileWriter=myFileWriter;
        myFileWriterStruct.structFullFileName=myFileNameString;
        //add it to the map
        myFileWriterMap[myIter->first]=myFileWriterStruct;
        std::cout << "Added " << myFileWriterStruct.structFullFileName << std::endl;
      }
    }




    // cycle through each FileGroup, fetching the element array from it and running any user
    // selected calculations, then writing the outputs to its associated filewriter in
    // myFileWriterMap

    if (meanTempFileGroup && meanTempFileNameString !="" &&
            availableCalculationsMap["Mean temperature"])
    {
      std::cout << "ClimateDataProcessor::run - Mean temperature requested" << std::endl;
      //move to start of the current data matrix
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;


      while (!meanTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;



        //get the next element from the file group
        myFloatVector = meanTempFileGroup->getElementVector();

        //this next bit is just for debugging purposes:
        /*                  
                            std::cout << "Printing out myFloatVector ("<< myFloatVector.size() << " elements): "  << std::endl;
                            for (int i=0;i < myFloatVector.size(); i++)
                            {
                            std::cout << myFloatVector[i] << ",";
                            }

        //end of debugging
        */

        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverYear(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }

    }
    if (diurnalTempFileGroup && diurnalTempFileNameString != "" &&
            availableCalculationsMap["Annual mean diurnal temperature range"])
    {
      std::cout << "ClimateDataProcessor::run Annual mean diurnal temperature range requested" << std::endl;
      //move to start of the current data matrix
      diurnalTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Annual mean diurnal temperature range"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!diurnalTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = diurnalTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverYear(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }

    if (frostDaysFileGroup && frostDaysFileNameString != "" &&
            availableCalculationsMap["Annual mean number of frost days"])
    {
      std::cout << "ClimateDataProcessor::run Annual mean number of frost days requested" << std::endl;
      //move to start of the current data matrix
      frostDaysFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Annual mean number of frost days"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!frostDaysFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = frostDaysFileGroup->getElementVector();

        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverYear(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }

    if (totalSolarRadFileGroup && totalSolarRadFileNameString != "" &&
            availableCalculationsMap["Annual mean total incident solar radiation"])
    {
      std::cout << "ClimateDataProcessor::run Annual mean total incident solar radiation requested" << std::endl;
      //move to start of the current data matrix
      totalSolarRadFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Annual mean total incident solar radiation"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = totalSolarRadFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverYear(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }

    if ( minTempFileGroup
            && maxTempFileGroup
            && minTempFileNameString != ""
            && maxTempFileNameString != ""
            && availableCalculationsMap["Annual temperature range"])
    {


      std::cout << "ClimateDataProcessor::run - Annual temperature range requested" << std::endl;
      //move to start of the current data matrix
      minTempFileGroup->moveToDataStart();
      maxTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Annual temperature range"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!minTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector, myFloatVector2;
        //get the next element from the file group
        myFloatVector = minTempFileGroup->getElementVector();
        myFloatVector2 = maxTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->greatestTotalRange(myFloatVector,myFloatVector2);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }

    if (maxTempFileGroup
            && maxTempFileNameString != ""
            && availableCalculationsMap["Highest temperature in warmest month"])
    {
      std::cout << "ClimateDataProcessor::run - Highest temperature in warmest month requested" << std::endl;
      maxTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Highest temperature in warmest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!maxTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = maxTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->highestValue(myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }


    }

    if (minTempFileGroup
            && minTempFileNameString != ""
            && availableCalculationsMap["Lowest temperature in coolest month"])
    {

      std::cout << "ClimateDataProcessor::run - Lowest temperature in coolest month requested" << std::endl;
      minTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Lowest temperature in coolest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!minTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = minTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->lowestValue(myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }

    }
    if (meanPrecipFileGroup
            && meanPrecipFileNameString != ""
            && availableCalculationsMap["Mean daily precipitation"])
    {

      std::cout << "ClimateDataProcessor::run Mean daily precipitation" << std::endl;
      meanPrecipFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverYear(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if ( meanPrecipFileGroup &&  minTempFileGroup
            && meanPrecipFileNameString != "" && minTempFileNameString != ""
            && availableCalculationsMap["Mean daily precipitation in coolest month"])
    {

      std::cout << "ClimateDataProcessor::run Mean daily precipitation in coolest month" << std::endl;
      meanPrecipFileGroup->moveToDataStart();
      minTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in coolest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        myFloatVector2 = minTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myBlockInt = myDataProcessor->monthWithLowestValue(myFloatVector2);
        float myFloat  = myDataProcessor->valueGivenMonth(myFloatVector,myBlockInt);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }

    }
    if (meanPrecipFileGroup &&  minTempFileGroup && meanPrecipFileNameString != ""
            && minTempFileNameString != ""
            && availableCalculationsMap["Mean daily precipitation in coolest quarter"])
    {

      std::cout << "ClimateDataProcessor::run Mean daily precipitation in coolest month" << std::endl;
      meanPrecipFileGroup->moveToDataStart();
      minTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in coolest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        myFloatVector2 = minTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myBlockInt = myDataProcessor->firstMonthOfLowestQ(myFloatVector2);
        float myFloat  = myDataProcessor->meanOverQuarter(myFloatVector,myBlockInt);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  &&
            availableCalculationsMap["Mean daily precipitation in driest month"])
    {

      std::cout << "ClimateDataProcessor::run Mean daily precipitation in driest month" << std::endl;
      meanPrecipFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in driest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->lowestValue(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  &&
            availableCalculationsMap["Mean daily precipitation in driest quarter"])
    {
      std::cout << "ClimateDataProcessor::run Mean daily precipitation in driest quarter" << std::endl;
      meanPrecipFileGroup->moveToDataStart();

      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in driest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverLowestQ(myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""
            && maxTempFileGroup
            && maxTempFileNameString != ""
            && availableCalculationsMap["Mean daily precipitation in warmest month"])
    {
      std::cout << "ClimateDataProcessor::run Mean daily precipitation in warmest month" << std::endl;
      //move to the start of data blocks
      meanPrecipFileGroup->moveToDataStart();
      maxTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in warmest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        myFloatVector2 = minTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myBlockInt = myDataProcessor->monthWithHighestValue(myFloatVector2);
        float myFloat  = myDataProcessor->valueGivenMonth(myFloatVector,myBlockInt);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }

    }
    if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""
            && maxTempFileGroup &&  maxTempFileNameString != ""
            &&availableCalculationsMap["Mean daily precipitation in warmest quarter"])
    {
      std::cout << "ClimateDataProcessor::run Mean daily precipitation in warmest quarter" << std::endl;
      meanPrecipFileGroup->moveToDataStart();
      maxTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in warmest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        myFloatVector2 = maxTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myCurrentBlockOfWarmestQuarterInt = myDataProcessor->firstMonthOfHighestQ(myFloatVector2);
        float myFloat = myDataProcessor->meanOverQuarter(myFloatVector,myCurrentBlockOfWarmestQuarterInt);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }

    }
    if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""   &&
            availableCalculationsMap["Mean daily precipitation in wettest month"])
    {
      std::cout << "ClimateDataProcessor::run Mean daily precipitation in wettest month" << std::endl;
      meanPrecipFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in wettest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->highestValue(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }

    }
    if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""   &&
            availableCalculationsMap["Mean daily precipitation in wettest quarter"])
    {

      std::cout << "ClimateDataProcessor::run Mean daily precipitation in wettest quarter" << std::endl;
      meanPrecipFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean daily precipitation in wettest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverHighestQ(myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (diurnalTempFileGroup && diurnalTempFileNameString != ""
            && meanTempFileGroup
            && meanTempFileNameString !=""
            && availableCalculationsMap["Mean diurnal temperature range in coolest month"])
    {
      std::cout << "ClimateDataProcessor::run Mean diurnal temperature range in coolest month" << std::endl;
      diurnalTempFileGroup->moveToDataStart();
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean diurnal temperature range in coolest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!diurnalTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = diurnalTempFileGroup->getElementVector();
        myFloatVector2 = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myCoolestBlockInt = myDataProcessor->monthWithLowestValue(myFloatVector2);
        float myFloat = myDataProcessor->valueGivenMonth(myFloatVector,myCoolestBlockInt);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }

    }
    if (diurnalTempFileGroup && diurnalTempFileNameString != "" && meanTempFileGroup
            && meanTempFileNameString !=""
            && availableCalculationsMap["Mean diurnal temperature range in warmest month"])
    {
      std::cout << "ClimateDataProcessor::run Mean diurnal temperature range in warmest month" << std::endl;
      diurnalTempFileGroup->moveToDataStart();
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean diurnal temperature range in warmest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!diurnalTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = diurnalTempFileGroup->getElementVector();
        myFloatVector2 = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myCoolestBlockInt = myDataProcessor->monthWithHighestValue(myFloatVector2);
        float myFloat = myDataProcessor->valueGivenMonth(myFloatVector,myCoolestBlockInt);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (meanPrecipFileGroup &&  meanPrecipFileNameString != ""  && frostDaysFileGroup
            && frostDaysFileNameString != ""
            && availableCalculationsMap["Mean precipitation in frost free months"])
    {
      std::cout << "ClimateDataProcessor::run Mean precipitation in frost free months" << std::endl;
      meanPrecipFileGroup->moveToDataStart();
      frostDaysFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean precipitation in frost free months"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        myFloatVector2 = frostDaysFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanValueOverFrostFreeMonths(myFloatVector2, myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }

    if (meanTempFileGroup && meanTempFileNameString !="" &&
            availableCalculationsMap["Mean temperature in coolest month"])
    {
      std::cout << "ClimateDataProcessor::run Mean temperature in coolest month" << std::endl;

      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in coolest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->lowestValue(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (meanTempFileGroup && meanTempFileNameString !="" &&
            availableCalculationsMap["Mean temperature in coolest quarter"])
    {
      std::cout << "ClimateDataProcessor::run Mean temperature in coolest quarter" << std::endl;
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in coolest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverLowestQ(myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (meanTempFileGroup && meanTempFileNameString !="" && frostDaysFileGroup
            && frostDaysFileNameString != ""
            && availableCalculationsMap["Mean temperature in frost free months"])
    {
      std::cout << "ClimateDataProcessor::run Mean temperature in frost free months" << std::endl;
      meanTempFileGroup->moveToDataStart();
      frostDaysFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in frost free months"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanTempFileGroup->getElementVector();
        myFloatVector2 = frostDaysFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanValueOverFrostFreeMonths(myFloatVector2, myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (meanTempFileGroup && meanTempFileNameString !="" &&
            availableCalculationsMap["Mean temperature in warmest month"])
    {
      std::cout << "ClimateDataProcessor::run Mean temperature in warmest month" << std::endl;


      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in warmest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->highestValue(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (meanTempFileGroup && meanTempFileNameString !="" &&
            availableCalculationsMap["Mean temperature in warmest quarter"])
    {
      std::cout << "ClimateDataProcessor::run Mean temperature in warmest quarter" << std::endl;
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean temperature in warmest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;

      while (!meanTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverHighestQ(myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (windSpeedFileGroup && windSpeedFileNameString != "" &&
            availableCalculationsMap["Mean wind speed"])
    {
      std::cout << "ClimateDataProcessor::run Mean wind speed" << std::endl;
      windSpeedFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Mean wind speed"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!windSpeedFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = windSpeedFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->meanOverYear(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (minTempFileGroup && minTempFileNameString !="" &&
            availableCalculationsMap["Number of months with minimum temperature above freezing"])
    {
      std::cout << "ClimateDataProcessor::run Number of months with minimum temperature above freezing" << std::endl;
      minTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Number of months with minimum temperature above freezing"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!minTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = minTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->numberOfMonthsAboveZero(myFloatVector );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
            && meanTempFileGroup && meanTempFileNameString !=""
            && availableCalculationsMap["Radiation in coolest month"])
    {
      std::cout << "ClimateDataProcessor::run Radiation in coolest month" << std::endl;
      totalSolarRadFileGroup->moveToDataStart();
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in coolest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = totalSolarRadFileGroup->getElementVector();
        myFloatVector2 = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myCoolestBlockInt = myDataProcessor->monthWithLowestValue(myFloatVector2);
        float myFloat = myDataProcessor->valueGivenMonth(myFloatVector,myCoolestBlockInt );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
            && meanTempFileGroup && meanTempFileNameString !=""
            && availableCalculationsMap["Radiation in coolest quarter"])
    {
      std::cout << "ClimateDataProcessor::run Radiation in coolest quarter" << std::endl;
      totalSolarRadFileGroup->moveToDataStart();
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in coolest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = totalSolarRadFileGroup->getElementVector();
        myFloatVector2 = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myFirstBlockInt = myDataProcessor->firstMonthOfLowestQ(myFloatVector2);
        float myFloat = myDataProcessor->meanOverQuarter(myFloatVector,myFirstBlockInt );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
            && meanTempFileGroup && meanTempFileNameString != ""
            && availableCalculationsMap["Radiation in warmest month"])
    {
      std::cout << "ClimateDataProcessor::run Radiation in warmest month" << std::endl;
      totalSolarRadFileGroup->moveToDataStart();
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in warmest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = totalSolarRadFileGroup->getElementVector();
        myFloatVector2 = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myHighestBlockInt = myDataProcessor->monthWithHighestValue(myFloatVector2);
        float myFloat = myDataProcessor->valueGivenMonth(myFloatVector,myHighestBlockInt );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
            && meanTempFileGroup && meanTempFileNameString != ""
            && availableCalculationsMap["Radiation in warmest quarter"])
    {
      std::cout << "ClimateDataProcessor::run Radiation in warmest quarter" << std::endl;
      totalSolarRadFileGroup->moveToDataStart();
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in warmest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
      {

        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = totalSolarRadFileGroup->getElementVector();
        myFloatVector2 = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        int myFirstBlockInt = myDataProcessor->firstMonthOfHighestQ(myFloatVector2);
        float myFloat = myDataProcessor->meanOverQuarter(myFloatVector,myFirstBlockInt );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }

      }

    }
    if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
            && meanPrecipFileGroup && meanPrecipFileNameString != ""
            && availableCalculationsMap["Radiation in driest month"])
    {
      std::cout << "ClimateDataProcessor::run Radiation in driest month" << std::endl;
      totalSolarRadFileGroup->moveToDataStart();
      meanPrecipFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in driest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = totalSolarRadFileGroup->getElementVector();
        myFloatVector2 = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        int myDriestBlockInt = myDataProcessor->monthWithLowestValue(myFloatVector2);
        float myFloat = myDataProcessor->valueGivenMonth(myFloatVector,myDriestBlockInt );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }

      }
    }
    if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
            && meanPrecipFileGroup && meanPrecipFileNameString != ""
            && availableCalculationsMap["Radiation in driest quarter"])
    {
      std::cout << "ClimateDataProcessor::run Radiation in driest quarter" << std::endl;
      totalSolarRadFileGroup->moveToDataStart();
      meanPrecipFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in driest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = totalSolarRadFileGroup->getElementVector();
        myFloatVector2 = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        int myFirstBlockInt = myDataProcessor->firstMonthOfLowestQ(myFloatVector2);
        float myFloat = myDataProcessor->meanOverQuarter(myFloatVector,myFirstBlockInt );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }

    if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
            && meanPrecipFileGroup && meanPrecipFileNameString != ""
            && availableCalculationsMap["Radiation in wettest month"])
    {
      std::cout << "ClimateDataProcessor::run Radiation in wettest month" << std::endl;
      totalSolarRadFileGroup->moveToDataStart();
      meanPrecipFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in wettest month"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = totalSolarRadFileGroup->getElementVector();
        myFloatVector2 = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        int myWettestBlockInt = myDataProcessor->monthWithHighestValue(myFloatVector2);
        float myFloat = myDataProcessor->valueGivenMonth(myFloatVector,myWettestBlockInt );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }


      }
    }
    if (totalSolarRadFileGroup && totalSolarRadFileNameString != ""
            && meanPrecipFileGroup && meanPrecipFileNameString != ""
            && availableCalculationsMap["Radiation in wettest quarter"])
    {
      std::cout << "ClimateDataProcessor::run Radiation in wettest quarter" << std::endl;
      totalSolarRadFileGroup->moveToDataStart();
      meanPrecipFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Radiation in wettest quarter"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!totalSolarRadFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector,myFloatVector2;
        //get the next element from the file group
        myFloatVector = totalSolarRadFileGroup->getElementVector();
        myFloatVector2 = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        int myFirstBlockInt = myDataProcessor->firstMonthOfHighestQ(myFloatVector2);
        float myFloat = myDataProcessor->meanOverQuarter(myFloatVector,myFirstBlockInt );
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }

      }
    }
    if (meanPrecipFileGroup && meanPrecipFileNameString != "" &&
            availableCalculationsMap["Standard deviation of mean precipitation"])
    {
      std::cout << "ClimateDataProcessor::run Standard deviation of mean precipitation" << std::endl;
      meanPrecipFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Standard deviation of mean precipitation"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanPrecipFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = meanPrecipFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->stddevOverYear(myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }
      }
    }
    if (meanTempFileGroup && meanTempFileNameString != ""
            && availableCalculationsMap["Standard deviation of mean temperature"])
    {
      std::cout << "ClimateDataProcessor::run Standard deviation of mean temperature" << std::endl;
      meanTempFileGroup->moveToDataStart();
      //create a data processor to do the low level processing
      DataProcessor *myDataProcessor = new DataProcessor();
      //get the struct containing the filewriter pointer and full file name from the writer map
      FileWriterStruct myFileWriterStruct = myFileWriterMap["Standard deviation of mean temperature"];
      //get the filewriter from out of the struct
      FileWriter *myFileWriter = myFileWriterStruct.structFileWriter;
      while (!meanTempFileGroup->getEndOfMatrixFlag())
      {
        std::vector<float> myFloatVector;
        //get the next element from the file group
        myFloatVector = meanTempFileGroup->getElementVector();
        //we are using mean over year summary
        float myFloat = myDataProcessor->stddevOverYear(myFloatVector);
        //write the result to our output file
        bool myResultFlag = myFileWriter->writeElement(myFloat);
        if (!myResultFlag)
        {
          std::cout << "Error! Writing an element to " <<  myFileWriterStruct.structFullFileName << " failed " << std::endl;
          return false;
        }  

      }
    }
  }//This is the END OF MAIN OUTER LOOP:
  //presume all went ok - need to add better error checking later
  return true;
}



/** Return a nice summary about this ClimateDataProcessor object */
std::string ClimateDataProcessor::getDescription()
{
  //must have an include for sstream.h!

  std::string myString, myNumberString;
  myString += "\n Climate Data Processor Description \n";
  myString += " ---------------------------------- \n";
  myNumberString = intToString( getFileStartYearInt());   //convert an int to a string!
  myString += std::string("File Start Year : ") + myNumberString + std::string("\n") ;

  myNumberString = intToString( getJobStartYearInt());
  myString += std::string("Job Start Year : ") + myNumberString+ std::string("\n") ;
  myNumberString = intToString( getJobEndYearInt());
  myString += std::string("Job End Year : ") + myNumberString + std::string("\n") ;

  myNumberString = intToString(getInputFileType());
  myString += std::string("Input File Type Enum : ") + myNumberString+ std::string("\n");

  myNumberString = intToString(getOutputFileType());
  myString += std::string("Output File Type Enum : ") + myNumberString + std::string("\n");


  //these properties are just plain strings and dont need conversion
  myString += std::string("Mean Temp FileName : ") + getMeanTempFileName() + std::string("\n");
  myString += std::string("Max Temp FileName : ") + getMaxTempFileName() + std::string("\n");
  myString += std::string("Min Temp FileName : ") + getMinTempFileName() + std::string("\n");
  myString += std::string("Diurnal Temp FileName : ") + getDiurnalTempFileName() + std::string("\n");
  myString += std::string("Mean Precipitation FileName : ") + getMeanPrecipFileName() + std::string("\n");
  myString += std::string("Frost Days FileName : ") + getFrostDaysFileName() + std::string("\n");
  myString += std::string("Total Solar Radiation FileName : ") + getTotalSolarRadFileName() + std::string("\n");
  myString += std::string("Wind Speed FileName : ") + getWindSpeedFileName() + std::string("\n");
  if (filesInSeriesFlag)
  {
    myString += std::string("Datafiles are a series of numbered files for each month \n");
  }
  else
  {
    myString += std::string("Datafiles contain all monthly data in a single file \n");
  }

  //List the calculations in  availableCalculationsMap  using an iterator
  myString += std::string("Listing items in availableCalculationsMap \n");
  myString += std::string("Boolean value suffix indicates whether the user want to use this calculation \n");
  myString += std::string("---------------------------------------------------------------------------------------------------- \n");
  std::map<std::string, bool>::const_iterator myIter;
  for (myIter=availableCalculationsMap.begin(); myIter != availableCalculationsMap.end(); myIter++)
  {
    if (myIter->second)
    {
      myString += myIter->first + std::string(": true\n");
    }
    else
    {
      myString += myIter->first + std::string(": false\n");
    }
  }
  myString += std::string("---------------------------------------------------------------------------------------------------- \n");

  return myString;

}

/** little helper to convert an int to a string */
std::string ClimateDataProcessor::intToString(int theInt)
{
  std::stringstream myStringStream;
  myStringStream << theInt;
  return myStringStream.str();
}


