/******************************************************
                          climatedataprocessor.h  -  description
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



#ifndef CLIMATEDATAPROCESSOR_H
#define CLIMATEDATAPROCESSOR_H

#include "dataprocessor.h"
#include "filewriter.h"
#include "filereader.h"
#include "filegroup.h"
#include <map>
#include <string>



/**The ClimateDataProcessor calculates specific climate variables using
  *DataProcessor functions.
  *@author Tim Sutton
  */

class ClimateDataProcessor {
public: 
	ClimateDataProcessor();

  ClimateDataProcessor(
                    int theFileStartYear,
                    int theJobStartYear,
                    int theJobEndYear,
                    std::string theInputFileTypeString,
                    std::string theOutputFileTypeString
                    );
  
	~ClimateDataProcessor();


  
  // Getters and setters
  
  /** Write property of std::string meanTempFileGroup. */
   void setMeanTempFileName ( std::string theFileNameString);
  /** Read property of std::string meanTempFileGroup. */
   const std::string  getMeanTempFileName ();

  /** Write property of std::string minTempFileGroup. */
   void setMinTempFileName ( std::string theFileNameString);
  /** Read property of std::string minTempFileGroup. */
   const std::string  getMinTempFileName ();

  /** Write property of std::string maxTempFileGroup. */
   void setMaxTempFileName ( std::string theFileNameString);
  /** Read property of std::string maxTempFileGroup. */
   const std::string  getMaxTempFileName ();

  /** Write property of std::string diurnalTempFileGroup. */
   void setDiurnalTempFileName ( std::string theFileNameString);
  /** Read property of std::string diurnalTempFileGroup. */
  const std::string  getDiurnalTempFileName ();

  /** Write property of std::string meanPrecipFileGroup. */
   void setMeanPrecipFileName ( std::string theFileNameString);
  /** Read property of std::string meanPrecipFileGroup. */
   const std::string  getMeanPrecipFileName ();

  /** Write property of std::string frostDaysFileGroup. */
   void setFrostDaysFileName ( std::string theFileNameString);
  /** Read property of std::string frostDaysFileGroup. */
   const std::string  getFrostDaysFileName ();

  /** Write property of std::string totalSolarRadFileGroup. */
   void setTotalSolarRadFileName ( std::string theFileNameString);
  /** Read property of std::string totalSolarRadFileGroup. */
   const std::string  getTotalSolarRadFileName ();

  /** Write property of std::string windSpeedFileGroup. */
   void setWindSpeedFileName ( std::string theFileNameString);
  /** Read property of std::string windSpeedFileGroup. */
   const std::string  getWindSpeedFileName ();

  /** Write property of std::string * outputFilePathString. */
  void setOutputFilePathString( std::string theFilePathString);
  /** Read property of std::string * outputFilePathString. */
  const std::string getOutputFilePathString();

  /** Write property of int fileStartYearInt. */
   void setFileStartYearInt( const int theYearInt);
  /** Read property of int fileStartYearInt. */
   const int getFileStartYearInt();

  /** Write property of int jobStartYearInt. */
   void setJobStartYearInt( const int theYearInt);
  /** Read property of int jobStartYearInt. */
   const int getJobStartYearInt();

  /** Write property of int jobEndYearInt. */
   void setJobEndYearInt( const int theYearInt);
  /** Read property of int jobEndYearInt. */
   const int getJobEndYearInt();

  /** Write property of FileReader::FileType inputFileType. */
  void setInputFileType( const FileReader::FileTypeEnum theInputFileType);
  /** Overloaded version of above that taks a string and looks up the enum */
  void setInputFileType( const std::string theInputFileTypeString);  
  /** Read property of FileReader::FileType inputFileType. */
  const FileReader::FileTypeEnum getInputFileType();

  /** Write property of FileWriter::FileType outputFileType. */
  void setOutputFileType( const FileWriter::FileTypeEnum theOutputFileType);
  /** Overloaded version of above that takes a string and looks up the enum */
  void setOutputFileType( const std::string theOutputFileTypeString);
  /** Read property of FileWriter::FileType outputFileType. */
  const FileWriter::FileTypeEnum getOutputFileType();


  /**  Set up the filegroups for each filename that has been registered */
  bool makeFileGroups(int theStartYearInt);
  /** Set up an individual file group (called by makeFileGroups for
  *   each filegroup that needs to be initialised) */
  FileGroup * initialiseFileGroup(std::string theFileNameString,int theStartYearInt);
  /**  Build a list of which calculations can be performed given the input files
   *    that have been registered. The boolean field indicates whether the user actually
   *    want to perform this calculation
   *    @see addUserCalculation */
  bool  makeAvailableCalculationsMap();
  /** Get the list of available calculations */
  std::map <std::string, bool > getAvailableCalculationsMap();
  
  /**  Add a calculation to the list of those requested to be carried out by the user */
  bool addUserCalculation(std::string theCalculationNameString);

  /** Start the data analysis process. When everything else is set up, this is the method to call! */
  bool run();

    /** get a Description of the ClimateDataProcessor vars. */
   std::string getDescription();
   
  /** Write property of bool filesInSeriesFlag. */
  void setFilesInSeriesFlag( const bool theFlagl);
  /** Read property of bool filesInSeriesFlag. */
  const bool getFilesInSeriesFlag();
  
  /** Write property of std::string outputHeaderString. */
  void setOutputHeaderString( const std::string& theOutputHeaderString);
  /** Read property of std::string outputHeaderString. */
  const std::string& getOutputHeaderString();
  
private:

// Private methods
  bool meanTempOverCoolestQ();
  
  /**This is a private method. It is a simple method to populate the
  * inputFileTypeMap attribute - this will usually be called by the
  * constructor(s). All keys (file type strings) will be  stored in upper case.*/
  bool makeInputFileTypeMap();

  /**This is a private method. It is a simple method to populate the
  * outputFileTypeMap attribute - this will usually be called by the
  * constructor(s). All keys (file type strings) will be  stored in upper case.*/
  bool makeOutputFileTypeMap();

  /** Little utility method to convert from int to string */
  std::string intToString(int theInt);
  
// Private attributes
  /** The directory where the processed results will be stored. */
  std::string outputFilePathString;
  /** This is the FILE START year (must be common to all files used!)
  *   in the files provided to the climate data processor. */
  int fileStartYearInt;
  /** This is the START year that should actually be processed
  *   (must be common to all files used!) in the files provided
  *   to the climate data proccessor. */
  int jobStartYearInt;
  /** This is the END year that should actually be processed
  *   (must be common to all files used!) in the files provided
  *   to the climate data proccessor. */

  int jobEndYearInt;  
   /** The type of input files to be processed by the climate date processor. */
  FileReader::FileTypeEnum inputFileType;

   /** The type of output files to be produced by the climate date processor. */
  FileWriter::FileTypeEnum outputFileType;
      
  /** This is a map (associative array) that stores the key/value pairs
   * for the INPUT filetype. The key is the verbose name for the file type
   * (as will typically appear in the user interface, and the value
   * is the FileReader::FileTypeEnum equivalent.
   * @see makeInputFileTypeMap()
   * @see makeOutputFileTypeMap()
   */
  std::map <std::string, FileReader::FileTypeEnum > inputFileTypeMap;


  /** This is a map (associative array) that stores the key/value pairs
   * for the OUTPUT filetype. The key is the verbose name for the file type
   * (as will typically appear in the user interface, and the value
   * is the FileWriter::FileTypeEnum equivalent.
   * @see makeInputFileTypeMap()
   * @see makeOutputFileTypeMap()
   */
  std::map <std::string, FileWriter::FileTypeEnum > outputFileTypeMap;

  /** This is a map (associative array) that stores which calculations can be performed
  *   given the input files that have been registered with this climatedataprocessor.
  *   The boolean flag will be used to indicate whether the user actually wants to
  *   perform the calculation on the input dataset(s).
  *   @see makeAvailableCalculationsMap
  *   @see addUserCalculation
  */
  
  std::map <std::string, bool > availableCalculationsMap;

      
  /** A filegroup containing files with mean temperature data. */
  FileGroup *  meanTempFileGroup;
  std::string meanTempFileNameString;
  /** A filegroup containing files with minimum temperature data. */
  FileGroup * minTempFileGroup;
  std::string minTempFileNameString;
  /** A filegroup containing files with maximum temperature data. */
  FileGroup *  maxTempFileGroup;
  std::string maxTempFileNameString;  
  /** A filegroup containing files with diurnal temperature data. */
  FileGroup *  diurnalTempFileGroup;
  std::string diurnalTempFileNameString;  
  /** A filegroup containing files with mean precipitation data. */
  FileGroup *  meanPrecipFileGroup;
  std::string meanPrecipFileNameString;  
  /** A filegroup containing files with number of frost days data. */
  FileGroup *  frostDaysFileGroup;
  std::string frostDaysFileNameString;
  /** A filegroup containing files with solar radiation data. */
  FileGroup *  totalSolarRadFileGroup;
  std::string totalSolarRadFileNameString;  
  /** A filegroup containing files with wind speed data. */
  FileGroup *  windSpeedFileGroup;
  std::string windSpeedFileNameString;
  /** For certain input types (notably cres, arcinfo and Reading paleoclimate),
  * each months data is stored in a discrete file. Files should be numbered
  * e.g. meantemp01.asc, meantemp2.asc...meantemp12.asc for each month.
  * This flag lets us know whether data is in a series of seperate files for each month
  * or can all be found in the same file. */
  bool filesInSeriesFlag;
 

  bool debugModeFlag;
  /** This is a standard header (e.g. arc/info header) that will be appended to any output grids. */
  std::string outputHeaderString;
};

#endif
         /*
  bool meanPrecipOverDriestQ
  bool meanTempOverWarmestQ
  bool meanPrecipOverWettestQ
  bool meanTempOverCoolestM
  bool lowestTempOverCoolestM
  bool meanPrecipOverDriestM
  bool meanTempOverWarmestM
  bool highestTempOverWarmestM
  bool meanPrecipOverWettestM
  bool meanTemp
  bool meanPrecip
  bool meanDiurnal
  bool meanFrostDays
  bool meanRadiation
  bool meanWindSpeed
  bool stdevMeanTemp
  bool stdevMeanPrecip
  bool meanPrecipOverCoolestM
  bool meanDiurnalOverCoolestM
  bool meanRadiationOverCoolestM
  bool meanRadiationOverDriestM
  bool meanPrecipOverWarmestM
  bool meanDiurnalOverWarmestM
  bool meanRadiationOverWarmestM
  bool meanRadiationOverWettestM
  bool meanPrecipOverCoolestQ
  bool meanRadiationOverCoolestQ
  bool meanRadiationOverDriestQ
  bool meanPrecipOverWarmestQ
  bool meanRadiationOverWarmestQ
  bool meanRadiationOverWettestQ
  bool annualTempRange
  bool meanTempOverFrostFreeM
  bool meanPrecipOverFrostFreeM
  bool monthCountAboveFreezing
  */
