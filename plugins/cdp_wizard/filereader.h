

#ifndef _FILEREADER_H_
#define _FILEREADER_H_

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <gsl/gsl_matrix.h>
#include "gdal_priv.h"
struct MatrixStats {
  float minValFloat;
  float maxValFloat;
  //the distance between min & max
  float rangeFloat;
  float meanFloat;
  float sumSqrDevFloat; //used to calculate stddev
  float stdDevFloat;
  float sumFloat;
  int elementCountInt;
  float noDataFloat;
};
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
                                      CCSR_AGCM_OGCM, GDAL };

  enum FileFormatEnum { CSM_MATLAB , CSM_OCTAVE ,  GARP ,  ESRI_ASCII ,  PLAIN };

  //
  //   Public methods
  //


  /* constructor to open a file */
  FileReader(std::string theFileNameString, const FileTypeEnum& theFileTypeEnum);
  /*Does nothing */
   ~FileReader();
  /** Find out whether the matrix is to be loaded in memory, or read element
   * by element from the file. */
   bool getLoadInMemoryFlag();
  /**
  *Get the next available element from the file matrix.
  */
   float getElement();
  /**
  Move the internal pointer to the first matrix element.
  */
   bool moveFirst();
  /**
  This method will close the file currently associated with the fileReader object.
  */
   bool closeFile();
  /**
  * This  will open a given file. The file pointer will be moved to the first matrix element,
  * and any header info will be skipped.
  * @param open. The FileName (including full path) to open.
  */
   bool openFile(const char *theFileName);
  /**
  * This  will open a given file. The file pointer will be moved to the first matrix element,
  * and any header info will be skipped. This is an overloaded version of openFile to take a
  * std::string argument rather than a char*.
  * @param open. The FileName (including full path) to open.
  */
   bool openFile(const std::string *theFileNameString);
  /** Write property of FILE *filePointer. */
   bool setFilePointer( FILE*  theNewVal);
  /** Read property of FILE *filePointer. */
   const FILE* getFilePointer();
  /** Write property of long currentElementLong. This method likely to be removed!*/
   bool setCurrentElement( const long& theNewVal);
  /** Read property of long currentElementLong. */
   const long& getCurrentElement();
  /** Write property of long currentRowLong. This method likely to be removed!*/
   bool setCurrentRow( const long& theNewVal);
  /** Read property of long currentRowLong. */
   const long& getCurrentRow();
  /** Write property of long currentColLong. This method likely to be removed!*/
   bool setCurrentCol( const long& theNewVal);
  /** Read property of long currentColLong. */
   const long& getCurrentCol();
  /** Write property of long columnsPerRowLong. */
   bool setColumnsPerRow( const long& theNewVal);
  /** Read property of long columnsPerRowLong. */
   const long& getColumnsPerRow();
  /** Write property of int headerLinesInt. */
   bool setHeaderLines( const int& theNewVal);
  /** Read property of int headerLinesInt. */
   const int& getHeaderLines();
  /** Write property of int currentBlockInt. */
   bool setCurrentBlock( const int& theNewVal);
  /** Read property of int currentBlockInt. */
   const int& getCurrentBlock();
  /** Write property of bool endOfMatrixFlag. */
   bool setEndOfMatrixFlag( const bool& theNewVal);
  /** Read property of bool endOfMatrixFlag. */
   const bool& getEndOfMatrixFlag();
  /** Write property of char *FileName. */
   bool setFileName( std::string theNewVal);
  /** Read property of char *FileName. */
   const std::string getFileName();
   const std::string getFileExtension();
   const std::string getFilePath();
   const std::string getFileNamePart();
  /** Write property of FileFormatEnum fileFormat. */
   bool setFileFormat( const FileFormatEnum& theNewVal);
  /** Read property of FileFormatEnum fileFormat. Note that
  * return type is FileReader::FileFormatEnum& because the calling
  * class does not have the enum in its name space so we need to
  * explicitly specifiy the namespace*/
   const FileReader::FileFormatEnum& getFileFormat();
  /** Read property of FileTypeEnum fileType. */
   const FileReader::FileTypeEnum& getFileType();
  /** Write property of long yDimLong. */
   bool setYDim( const long& theNewVal);
  /** Read property of long yDimLong. */
   const long& getYDim();
  /** Write property of long xDimLong. */
   bool setXDim( const long& theNewVal);
  /** Read property of long xDimLong. */
   const long& getXDim();
  /** Write property of int monthHeaderLinesInt. */
   bool setBlockHeaderLinesInt( const int& theNewVal);
  /** Read property of int monthHeaderLinesInt. */
   const int& getBlockHeaderLinesInt();
  /** Return the various metadata stored for the open file. */
   std::string getFileReaderInfo();
  /** Write property of fpos_t headerPos. */
   bool setHeaderFPos( fpos_t &theNewVal);
  /** Read property of fpos_t headerPos. */
   const fpos_t* getHeaderFPos();
   /** Write property of fpos_t dataStartFPos. */
   bool setDataStartFPos( std::fpos_t &theNewVal);
   /** get the noData value used to represent null elements */
   float getNoDataValue();
   /**set the noData value used to represent null elements */
   bool setNoDataValue(float);
  /** Read property of fpos_t dataStartFPos. */
   const fpos_t* getDataStartFPos();
  /** Move the internal file pointer to the start of the file header. */
   bool moveToHeader();
  /** Move the internal file pointer to the start of the file header. */
   bool moveToDataStart();
  /** Use the header info for a given file type to determine the begining of the data block and position the
  *    dataStartFPos there. This method will need to be called explicitly by the client app so that when multiple
  *   copies of the same file are being opened, we dont need to do the same thing each time.*/
   std::vector <fpos_t>* getBlockMarkers(bool theForceParseFlag = false);
  
  bool setBlockMarkers(std::vector <fpos_t>* theBlockMarkers);
  /** Read property of int taskProgressInt. */
  const int& gettaskProgressInt();
  /** Find out how many blocks (useful in multiblock formats such as SRES) are in this file. */
  int getNumberOfBlocks();
  /** Generate some useful stats about the active matrix block */
  MatrixStats calulateMatrixStats();
  /** Return the active data block as a gnu scientific library matrix */
  gsl_matrix * FileReader::getGslMatrix();
  /** Populate the active data block as a gnu scientific library matrix */
  bool populateGslMatrix();
  /** Load the a gdal band into the (in memory) matrix */
  bool loadGdalLayerAsMatrix(const int theLayerNo);
  /** This method will perfrma a smothing function on the matrix */
  bool smooth()  ;
  /** This will perform a local function on the grid, reclassifying each cell
 within the range 0-1 with .25 increments */
  bool localFunction();
  /** This function will generate zones - cells with
  values of > 0 will become 1, all other cells will remain 0 */
  bool zonalFunction();
  
  
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
  std::string fileNameString;
  /** Whether the file pointer has reached the end of the matrix */
  bool endOfMatrixFlag;
  /** Month in file that matrix extraction should start at. */
  int currentBlockInt;
  /** Number of columns per row in raw data file - maybe dont need this? */
  long columnsPerRowLong;
  /** Number of header lines specifically at start of file. */
  int headerLinesInt;
  /** Values which represent null or no data in the file */
  float noDataFloat;
  /** Current column in the matrix */
  long currentColLong;
  /** Current row in matrix */
  long currentRowLong;
  /** Position in the matrix expressed as (current row * cols) + current col */
  long currentElementLong;
  /**  The FILE handle containing our data matrix. */
  FILE *filePointer;
  /** Write property of FileTypeEnum fileType. This is private! Use the constructor to set this. */
   bool setFileType( const FileTypeEnum& theNewVal);
  /** Number of header lines per month data block (applicable to files containing multiple months in a single file only. */
  int monthHeaderLinesInt;
  /** The fpos (STL) of the start of the header. This may not be the start of the file in instances where files contain more the one dataset. */
  std::fpos_t *headerFPos;
  /** The fpos (STL) of the start of the header. This may not be after the header in instances where files contain more the one dataset. */
  std::fpos_t *dataStartFPos;
  /* This is a vector  that stores the filepos for each  datablock in the file*/
  std::vector <fpos_t> dataBlockMarkersVector;
  /* This is a representation of the current data block as a gnu scientific library matrix */
  gsl_matrix * currentGslMatrix;
  /* This is a GDAL dataset that can be used to read froma variety of formats */
  GDALDataset  *gdalDataset;
  /* This flag determines whether data blocks should be loaded into memory, or read element by element fromm disk.
     The former approach is faster and preferable, but the latter option should allow very large files to be read reagardless of the available
     ammount of memory */
  bool loadBlocksInMemoryFlag;
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
