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
#include <qmap.h>
#include <qstring.h>
#include <qobject.h>

/**
* This struct is simple container used in the 'run' method.
* @todo Remove this if possible
*/
struct FileWriterStruct
{
    /** A filewriter pointer */
    FileWriter * structFileWriter;
    /** The fill path and file name of the file refenced */
    QString structFullFileName;
};


/**The ClimateDataProcessor calculates specific climate variables using
 *DataProcessor functions.
 *@author Tim Sutton
 */

class ClimateDataProcessor : public QObject {
    Q_OBJECT;
    public:
        /** Default constructor */
        ClimateDataProcessor();

        /*
        ClimateDataProcessor(
                int theFileStartYear,
                int theJobStartYear,
                int theJobEndYear,
                QString theInputFileTypeString,
                QString theOutputFileTypeString
                );
        */
        /** Destructor */
        ~ClimateDataProcessor();

        // Getters and setters

        /** Mutator for filename of the meanTemp calculation inputs.
        * If the files are in series, the name should be of the first file in the series.
        * @param theFileNameString - The new filename for the calculation inputs.
        * @return void - No return.
        */
        void setMeanTempFileName ( QString theFileNameString);
        /** Accessor for filename of the meanTemp calculation inputs.
        * @return a QString containing the filename (which will be the first file
        * in the series if the files are in series).
        */
        const QString  getMeanTempFileName ();

        /** Mutator for filename of the minTemp calculation inputs.
        * If the files are in series, the name should be of the first file in the series.
        * @param theFileNameString - The new filename for the calculation inputs.
        * @return void - No return.
        */
        void setMinTempFileName ( QString theFileNameString);
        /** Accessor for filename of the minTemp calculation inputs.
        * @return a QString containing the filename (which will be the first file
        * in the series if the files are in series).
        */
        const QString  getMinTempFileName ();

        /** Mutator for filename of the maxTemp calculation inputs.
        * If the files are in series, the name should be of the first file in the series.
        * @param theFileNameString - The new filename for the calculation inputs.
        * @return void - No return.
        */
        void setMaxTempFileName ( QString theFileNameString);
        /** Accessor for filename of the maxTemp calculation inputs.
        * @return a QString containing the filename (which will be the first file
        * in the series if the files are in series).
        */
        const QString  getMaxTempFileName ();

        /** Mutator for filename of the diurnalTemp calculation inputs.
        * If the files are in series, the name should be of the first file in the series.
        * @param theFileNameString - The new filename for the calculation inputs.
        * @return void - No return.
        */
        void setDiurnalTempFileName ( QString theFileNameString);
        /** Accessor for filename of the diurnalTemp calculation inputs.
        * @return a QString containing the filename (which will be the first file
        * in the series if the files are in series).
        */
        const QString  getDiurnalTempFileName ();

        /** Mutator for filename of the mean precipitation calculation inputs.
        * If the files are in series, the name should be of the first file in the series.
        * @param theFileNameString - The new filename for the calculation inputs.
        * @return void - No return.
        */
        void setMeanPrecipFileName ( QString theFileNameString);
        /** Accessor for filename of the mean precipitation calculation inputs.
        * @return a QString containing the filename (which will be the first file
        * in the series if the files are in series).
        */
        const QString  getMeanPrecipFileName ();

        /** Mutator for filename of the frost days calculation inputs.
        * If the files are in series, the name should be of the first file in the series.
        * @param theFileNameString - The new filename for the calculation inputs.
        * @return void - No return.
        */
        void setFrostDaysFileName ( QString theFileNameString);
        /** Accessor for filename of the frost days calculation inputs.
        * @return a QString containing the filename (which will be the first file
        * in the series if the files are in series).
        */
        const QString  getFrostDaysFileName ();

        /** Mutator for filename of the solar radiation calculation inputs.
        * If the files are in series, the name should be of the first file in the series.
        * @param theFileNameString - The new filename for the calculation inputs.
        * @return void - No return.
        */
        void setTotalSolarRadFileName ( QString theFileNameString);
        /** Accessor for filename of the solar radiation calculation inputs.
        * @return a QString containing the filename (which will be the first file
        * in the series if the files are in series).
        */
        const QString  getTotalSolarRadFileName ();

        /** Mutator for filename of the wind speed calculation inputs.
        * If the files are in series, the name should be of the first file in the series.
        * @param theFileNameString - The new filename for the calculation inputs.
        * @return void - No return.
        */
        void setWindSpeedFileName ( QString theFileNameString);
        /** Accessor for filename of the wind speed calculation inputs.
        * @return a QString containing the filename (which will be the first file
        * in the series if the files are in series).
        */
        const QString  getWindSpeedFileName ();

        /** Mutator for directory name for the calculation output files.
        * @param theFilePathString - The name of an existing directory where the
        * output files will be stored.
        * @return void - No return.
        */
        void setOutputFilePathString( QString theFilePathString);
        /** Accessor for the file output path.
        * @return a QString containing the directory name.
        */
        const QString getOutputFilePathString();

        /**
        * Mutator for int fileStartYearInt.
        * @param theYearInt - The year of the first 12 blocks of data
        * in the file (or the 12 files if the files are in series).
        * @return void - No return.
        * @note If the files are in series, jobStartYearInt should be the same as
        * fileStartYearInt and jobEndYearInt!
        */
        void setFileStartYearInt( const int theYearInt);
        /** Accessor for int fileStartYearInt. */
        const int getFileStartYearInt();

        /**
        * Mutator for int setJobStartYearInt.
        * @param theYearInt - The first year in the input file(s) to be read.
        * @return void - No return.
        * @note If the files are in series, jobStartYearInt should be the same as
        * fileStartYearInt and jobEndYearInt!
        */
        void setJobStartYearInt( const int theYearInt);
        /**
        * Accessor for int jobStartYearInt.
        * @return int - the current value of the first year in the input
        * files to be read.
        */
        const int getJobStartYearInt();

        /**
        * Mutator for int jobEndYearInt.
        * @param theYearInt - The last year in the input file(s) to be read.
        * @return void - No return
        * @note If the files are in series, jobStartYearInt should be the same as
        * fileStartYearInt and jobEndYearInt!
        */
        void setJobEndYearInt( const int theYearInt);
        /**
        * Accessor for int jobEndYearInt.
        * @return int - the current value of the last year in the input
        * files to be read.
        */
        const int getJobEndYearInt();

        /**
        * Mutator for FileReader::FileType inputFileType.
        * @param theInputFileType - The input filetype as specified in FileReader::FileTypeEnum
        * @return void - No return
        */
        void setInputFileType( const FileReader::FileTypeEnum theInputFileType);
        /**
        * Mutator for FileReader::FileType inputFileType.
        * This is an overloaded version of above that takes a string and looks up the enum.
        * @param theInputFileTypeString - The input filetype as a string. Valid options being:
        * CRES African climate data
        * ESRI & ASCII raster
        * Hadley Centre HadCM3 SRES Scenario
        * Hadley Centre HadCM3 IS92a Scenario
        * IPCC Observed Climatology
        * University of Reading Palaeoclimate data
        * Max Planck Institute fur Meteorologie (MPIfM) ECHAM4 data
        * CSIRO-Mk2 Model data
        * National Center for Atmospheric Research (NCAR) NCAR-CSM and NCAR-PCM data
        * Geophysical Fluid Dynamics Laboratory (GFDL) R30 Model data
        * Canadian Center for Climate Modelling and Analysis (CCCma) CGCM2 Model data
        * CCSR/NIES AGCM model data and CCSR OGCM model data
        * @return void - No return
        */
        void setInputFileType( const QString theInputFileTypeString);
        /**
        * Accessor for FileReader::FileType inputFileType.
        * @return a FileReader::FileTypeEnum indicating the current input file type.
        */
        const FileReader::FileTypeEnum getInputFileType();

        /**
        * Mutator for FileWriter::FileType outputFileType.
        * This will determine how outputs from calculations will be written to disk.
        * @param theOutputFileType - a  FileWriter::FileTypeEnum value
        * @return void - No return
        */
        void setOutputFileType( const FileWriter::FileTypeEnum theOutputFileType);
        /**
        * Mutator for FileWriter::FileType outputFileType.
        * Overloaded version of above that takes a string and looks up the enum.
        * @param theOutputFileTypeString - a QString containing the desired output file type.
        * Valid options include:
        * CSM for Matlab
        * CSM for Octave
        * Desktop GARP
        * ESRI ASCII Grid
        * Plain matrix with no header
        * @return void - No return
        */
        void setOutputFileType( const QString theOutputFileTypeString);
        /**
        * Accessor for FileWriter::FileType outputFileType.
        * @return FileWriter::FileTypeEnum - the current output file type
        */
        const FileWriter::FileTypeEnum getOutputFileType();


        /**
        * Set up the filegroups for each filename that has been registered
        * @return bool - A boolean indicating success or failure of the operation
        */
        bool makeFileGroups();

        /**  Build a list of which calculations can be performed given the input files
         *    that have been registered. The boolean field indicates whether the user actually
         *    want to perform this calculation
         *    @see addUserCalculation */
        bool  makeAvailableCalculationsMap();
        /**
        * Accessor for the list of available calculations.
        * @return QMap<QString, bool>  - wherethe string is the name of the calculation and
        * bool indicates true if the user has asked for the calculation to be carried out,
        * false if he hasnt.
        */
        QMap <QString, bool > getAvailableCalculationsMap();

        /**
        * Add a calculation to the list of those requested to be carried out by the user.
        * The available availableCalculationsMap will be searched for a string match, and
        * if a match is found that map entry will be tagged as true - 'please calculate'
        * if it is currently false.
        * @param theCalculationNameString - the name of the calculation to be enabled.
        * @return bool - Returns false if no matching calculation name is found.
         */
        bool addUserCalculation(QString theCalculationNameString);

        /**
        * Start the data analysis process.
        * When everything else is set up, this is the method to call!
        * @todo If there ever was a good place to optimise performance,
        * this is it!For starters this method can be refactored so that the
        * vector from each active filegroup is retrieved first, and then
        * passed to any calculation that needs it, looping through all the filegroups
        * simultaneously. This will prevent multiple reads of the same file for
        * different calculations. RTFS for more info.
        * @return bool - A boolean indicating success or failure of the operation.
        */
        bool run();

        /**
        * This is a helper method that will return a Description of the ClimateDataProcessor vars.
        * This will indicate things like whether the files are in series or not, what calculations
        * are available in the available calculations map, and whether they are tagged true for
        * execution or not.
        * @return QString - containing the summary description for this climatedataprocessor.
        */
        QString getDescription();

        /**
        * Mutator for bool filesInSeriesFlag.
        * When files are in series, it means that the file format only contains one months
        * data block per file and that there should be 12 such files to provide monthly
        * data for an entire year. The files should be suffixed with the month numer they
        * represent e.g.
        * meanTemp01.asc
        * meanTemp02.asc
        * meanTemp03.asc
        * meanTemp04.asc
        * meanTemp05.asc
        * meanTemp06.asc
        * meanTemp07.asc
        * meanTemp08.asc
        * meanTemp09.asc
        * meanTemp10.asc
        * meanTemp11.asc
        * meanTemp12.asc
        * @param theFlag - a flag indicating true if files are in series, otherwise false.
        * @return void - No return.
        */
        void setFilesInSeriesFlag( const bool theFlag);
        /**
        * Accessor for bool filesInSeriesFlag.
        * @see setFilesInSeriesFlag
        * @return bool - true if files are in series.
        */
        const bool getFilesInSeriesFlag();

        /**
        * If the output format requires a header (e.g. Arc/Info ASCII grid, you can
        * define one using this method.
        * @param theOutputHeaderString - a QString containing the new header.
        * @return void - No return.
        */
        void setOutputHeaderString( const QString& theOutputHeaderString);
        /**
        * Accessor for QString outputHeaderString.
        * @return QString - the currently set file header.
        */
        const QString getOutputHeaderString();


    signals:
        /**
        * A signal emitted to notify listeners how many variables
        * are going to be calculated for each years data.
        *@param theNumberInt - The total number of variables
        *@return void - No return
        */
        void numberOfVariablesToCalc(int theNumberInt);
        /**
        * A signal emitted to notify listeners how many cells
        * will be passed through in each block.
        *@param theNumberInt - The total number of cells in any block
        *@return void - No return
        */
        void numberOfCellsToCalc(int theNumberInt);
        /**
        * A signal emitted to notify listeners that we are about to
        * start calculating a variable for one years data.
        *@param theNameQString - A String containing the variable name e.g.
        *                        'Precipitation over coolest month'
        *@return void - No return
        */
        void variableStart(QString theNameQString);
        /**
        * A signal emitted to notify listeners that we
        * completed calculating the given variable.
        *@param theFileNameString - the filename that the variable was outputted to
        *@return void - No return
        */
        void variableDone(QString theFileNameString);
        /**
        * A signal emitted to notify listeners that we
        * have completed calculating a given cell.
        *@param theResultFloat - The calculated value for a cell
        *@return void - No return
        */
        void cellDone(float theResultFloat);

    private:

        // Private methods

        /** This method is intended for debugging purposes only */
        void printVectorAndResult(QValueVector<float> theVector, float theResultFloat);

        /** Set up an individual file group (called by makeFileGroups for
         *   each filegroup that needs to be initialised) */
        FileGroup * initialiseFileGroup(QString theFileNameString,int theStartYearInt);

        /**This is a private method. It is a simple method to populate the
         * inputFileTypeMap attribute - this will usually be called by the
         * constructor(s). All keys (file type strings) will be  stored in upper case.*/
        bool makeInputFileTypeMap();

        /**This is a private method. It is a simple method to populate the
         * outputFileTypeMap attribute - this will usually be called by the
         * constructor(s). All keys (file type strings) will be  stored in upper case.*/
        bool makeOutputFileTypeMap();

        /** Little utility method to convert from int to string */
        QString intToString(int theInt);

        // Private attributes
        /** The directory where the processed results will be stored. */
        QString outputFilePathString;
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
        QMap <QString, FileReader::FileTypeEnum > inputFileTypeMap;


        /** This is a map (associative array) that stores the key/value pairs
         * for the OUTPUT filetype. The key is the verbose name for the file type
         * (as will typically appear in the user interface, and the value
         * is the FileWriter::FileTypeEnum equivalent.
         * @see makeInputFileTypeMap()
         * @see makeOutputFileTypeMap()
         */
        QMap <QString, FileWriter::FileTypeEnum > outputFileTypeMap;

        /** This is a map (associative array) that stores which calculations can be performed
         *   given the input files that have been registered with this climatedataprocessor.
         *   The boolean flag will be used to indicate whether the user actually wants to
         *   perform the calculation on the input dataset(s).
         *   @see makeAvailableCalculationsMap
         *   @see addUserCalculation
         */

        QMap <QString, bool > availableCalculationsMap;


        /** A filegroup containing files with mean temperature data. */
        FileGroup *  meanTempFileGroup;
        /** The file name that contains mean temp data. If the file type is
        * one where the data is stored in series (a single file per month),
        * this member will store the name of the first file in the series. */
        QString meanTempFileNameString;

        /** A filegroup containing files with minimum temperature data. */
        FileGroup * minTempFileGroup;
        /** The file name that contains min temp data. If the file type is
        * one where the data is stored in series (a single file per month),
        * this member will store the name of the first file in the series. */
        QString minTempFileNameString;

        /** A filegroup containing files with maximum temperature data. */
        FileGroup *  maxTempFileGroup;
        /** The file name that contains max temp data. If the file type is
        * one where the data is stored in series (a single file per month),
        * this member will store the name of the first file in the series. */
        QString maxTempFileNameString;

        /** A filegroup containing files with diurnal temperature data. */
        FileGroup *  diurnalTempFileGroup;
        /** The file name that contains diurnal temp data. If the file type is
        * one where the data is stored in series (a single file per month),
        * this member will store the name of the first file in the series. */
        QString diurnalTempFileNameString;

        /** A filegroup containing files with mean precipitation data. */
        FileGroup *  meanPrecipFileGroup;
        /** The file name that contains mean precipitation data. If the file type is
        * one where the data is stored in series (a single file per month),
        * this member will store the name of the first file in the series. */
        QString meanPrecipFileNameString;

        /** A filegroup containing files with number of frost days data. */
        FileGroup *  frostDaysFileGroup;
        /** The file name that contains mean frost data. If the file type is
        * one where the data is stored in series (a single file per month),
        * this member will store the name of the first file in the series. */
        QString frostDaysFileNameString;

        /** A filegroup containing files with solar radiation data. */
        FileGroup *  totalSolarRadFileGroup;
        /** The file name that contains mean solar radiation data. If the file type is
        * one where the data is stored in series (a single file per month),
        * this member will store the name of the first file in the series. */
        QString totalSolarRadFileNameString;

        /** A filegroup containing files with wind speed data. */
        FileGroup *  windSpeedFileGroup;
        /** The file name that contains wind speed data. If the file type is
        * one where the data is stored in series (a single file per month),
        * this member will store the name of the first file in the series. */
        QString windSpeedFileNameString;

        /** For certain input types (notably cres, arcinfo and Reading paleoclimate),
         * each months data is stored in a discrete file. Files should be numbered
         * e.g. meantemp01.asc, meantemp2.asc...meantemp12.asc for each month.
         * This flag lets us know whether data is in a series of seperate files for each month
         * or can all be found in the same file. */
        bool filesInSeriesFlag;

        /** This is a standard header (e.g. arc/info header) that will be appended to any output grids. */
        QString outputHeaderString;
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
