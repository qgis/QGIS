
#ifndef _FILEREADER_H_
#define _FILEREADER_H_

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

/**
 * This class will handle opening a file containing a climate matrix and iterating through the file in a columnwise / rowwise manner.
 * Tim Sutton
 **/
class FileReader
{

public:

   //
  // Enumerators
  //
  enum FileTypeEnum { CRES,  ARCINFO_GRID , HADLEY_SRES , HADLEY_IS92 ,  IPCC_OBSERVED ,
                                      VALDES ,  ECHAM4 ,  CSIRO_MK2 ,  NCAR_CSM_PCM , GFDL_R30 , CGCM2 ,
                                      CCSR_AGCM_OGCM };

  enum FileFormatEnum { CSM_MATLAB , CSM_OCTAVE ,  GARP ,  ESRI_ASCII ,  PLAIN };

  //
  //   Public methods
  //

  /* Default constructor */
  FileReader();
  /* constructor to open a file */
  FileReader(std::string theFileNameString);
  /*Does nothing */
  virtual ~FileReader();
  /**
  *Get the next available element from the file matrix.
  */
  virtual float getElement();
  /**
  Move the internal pointer to the first matrix element.
  */
  virtual bool moveFirst();
  /**
  This method will close the file currently associated with the fileReader object.
  */
  virtual bool closeFile();
  /**
  * This  will open a given file. The file pointer will be moved to the first matrix element,
  * and any header info will be skipped.
  * @param open. The filename (including full path) to open.
  */
  virtual bool openFile(const char *theFileName);
  /**
  * This  will open a given file. The file pointer will be moved to the first matrix element,
  * and any header info will be skipped. This is an overloaded version of openFile to take a
  * std::string argument rather than a char*.
  * @param open. The filename (including full path) to open.
  */
  virtual bool openFile(const std::string *theFileNameString);  
  /** Write property of FILE *filePointer. */
  virtual bool setFilePointer( FILE*  theNewVal);
  /** Read property of FILE *filePointer. */
  virtual const FILE* getFilePointer();
  /** Write property of long currentElementLong. This method likely to be removed!*/
  virtual bool setCurrentElement( const long& theNewVal);
  /** Read property of long currentElementLong. */
  virtual const long& getCurrentElement();
  /** Write property of long currentRowLong. This method likely to be removed!*/
  virtual bool setCurrentRow( const long& theNewVal);
  /** Read property of long currentRowLong. */
  virtual const long& getCurrentRow();
  /** Write property of long currentColLong. This method likely to be removed!*/
  virtual bool setCurrentCol( const long& theNewVal);
  /** Read property of long currentColLong. */
  virtual const long& getCurrentCol();
  /** Write property of long columnsPerRowLong. */
  virtual bool setColumnsPerRow( const long& theNewVal);
  /** Read property of long columnsPerRowLong. */
  virtual const long& getColumnsPerRow();
  /** Write property of int headerLinesInt. */
  virtual bool setHeaderLines( const int& theNewVal);
  /** Read property of int headerLinesInt. */
  virtual const int& getHeaderLines();
  /** Write property of int startMonthInt. */
  virtual bool setStartMonth( const int& theNewVal);
  /** Read property of int startMonthInt. */
  virtual const int& getStartMonth();
  /** Write property of bool endOfMatrixFlag. */
  virtual bool setEndOfMatrixFlag( const bool& theNewVal);
  /** Read property of bool endOfMatrixFlag. */
  virtual const bool& getEndOfMatrixFlag();
  /** Write property of char *Filename. */
  virtual bool setFilename( std::string *theNewVal);
  /** Read property of char *Filename. */
  virtual const std::string* getFilename();
  /** Write property of FileFormatEnum fileFormat. */
  virtual bool setFileFormat( const FileFormatEnum& theNewVal);
  /** Read property of FileFormatEnum fileFormat. Note that
  * return type is FileReader::FileFormatEnum& because the calling
  * class does not have the enum in its name space so we need to
  * explicitly specifiy the namespace*/
  virtual const FileReader::FileFormatEnum& getFileFormat();
  /** Write property of FileTypeEnum fileType. */
  virtual bool setFileType( const FileTypeEnum& theNewVal);
  /** Read property of FileTypeEnum fileType. */
  virtual const FileReader::FileTypeEnum& getFileType();
  /** Write property of long yDimLong. */
  virtual bool setYDim( const long& theNewVal);
  /** Read property of long yDimLong. */
  virtual const long& getYDim();
  /** Write property of long xDimLong. */
  virtual bool setXDim( const long& theNewVal);
  /** Read property of long xDimLong. */
  virtual const long& getXDim();
  /** Write property of int monthHeaderLinesInt. */
  virtual bool setMonthHeaderLinesInt( const int& theNewVal);
  /** Read property of int monthHeaderLinesInt. */
  virtual const int& getMonthHeaderLinesInt();
  /** Return the various metadata stored for the open file. */
  virtual std::string getFileReaderInfo();
  /** Write property of fpos_t headerPos. */
  virtual bool setHeaderFPos( fpos_t &theNewVal);
  /** Read property of fpos_t headerPos. */
  virtual const fpos_t* getHeaderFPos();
   /** Write property of fpos_t dataStartFPos. */ 
  virtual bool setDataStartFPos( std::fpos_t &theNewVal);
  /** Read property of fpos_t dataStartFPos. */
  virtual const fpos_t* getDataStartFPos();
  /** Move the internal file pointer to the start of the file header. */
  virtual bool moveToHeader();
  /** Move the internal file pointer to the start of the file header. */
  virtual bool moveToDataStart();  
  /** Use the header info for a given file type to determine the begining of the data block and position the
  *    dataStartFPos there. This method will need to be called explicitly by the client app so that when multiple
  *   copies of the same file are being opened, we dont need to do the same thing each time.*/
  virtual std::vector <fpos_t>* getBlockMarkers();
  
  bool setBlockMarkers(std::vector <fpos_t>* theBlockMarkers);
  /** Read property of int taskProgressInt. */
  const int& gettaskProgressInt();
  /** Find out how many blocks (useful in multiblock formats such as SRES) are in this file. */
  int getNumberOfBlocks();
private: 
  //
  //   Private attributes
  //

  /** This is the xDim (columns) of a matrix for one month (files may contain more than one matrix). */
  long xDimLong;
  /** Number of rows in matrix for one month / year */
  long yDimLong;
  /** Type of file we are reading. */
  FileTypeEnum fileType;
  /** Format for this file e.g. arc/info grid */
  FileFormatEnum fileFormat;
  /** The name of the file, including full path if neccessary. */
  std::string *filenameString;
  /** Whether the file pointer has reached the end of the matrix */
  bool endOfMatrixFlag;
  /** Month in file that matrix extraction should start at. */
  int startMonthInt;
  /** Number of columns per row in raw data file - maybe dont need this? */
  long columnsPerRowLong;
  /** Number of header lines specifically at start of file. */
  int headerLinesInt;
  /** Current column in the matrix */
  long currentColLong;
  /** Current row in matrix */
  long currentRowLong;
  /** Position in the matrix expressed as (current row * cols) + current col */
  long currentElementLong;
  /**  The FILE handle containing our data matrix. */
  FILE *filePointer;

  /** Number of header lines per month data block (applicable to files containing multiple months in a single file only. */
  int monthHeaderLinesInt;
  /** The fpos (STL) of the start of the header. This may not be the start of the file in instances where files contain more the one dataset. */
  std::fpos_t *headerFPos;
  /** The fpos (STL) of the start of the header. This may not be after the header in instances where files contain more the one dataset. */
  std::fpos_t *dataStartFPos;
  /* This is a vector  that stores the filepos for each  datablock in the file*/
  std::vector <fpos_t> dataBlockMarkersVector;

  /** The progress (as a percentage) of any task currently being executed. */
  int taskProgressInt;

  /** flag to see whether we are at the end of the current line or not - probably wont need this
  //eolFlag As Boolean
  */
  bool debugModeFlag;
  //
  //   Private methods
  //
  
};

#endif
