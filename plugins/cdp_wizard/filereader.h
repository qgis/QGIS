
#ifndef _FILEREADER_H_
#define _FILEREADER_H_

#include <qfile.h>
#include <iostream>
#include <qstring.h>
#include <qvaluevector.h>
#include <qmap.h>

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
  FileReader(QString theFileNameString);
  /*Does nothing */
   ~FileReader();
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
  * and any header info will be skipped. This is an overloaded version of openFile to take a
  * QString argument rather than a QString .
  * @param open. The filename (including full path) to open.
  */
   bool openFile(const QString theFileNameString);
  /** Write property of FILE *filePointer. */
   bool setFilePointer( QFile*  theNewVal);
  /** Read property of FILE *filePointer. */
   const QFile * getFilePointer();
  /** Write property of long currentElementLong. This method likely to be removed!*/
   bool setCurrentElement( const long theNewVal);
  /** Read property of long currentElementLong. */
   const long getCurrentElement();
  /** Write property of long currentRowLong. This method likely to be removed!*/
   bool setCurrentRow( const long theNewVal);
  /** Read property of long currentRowLong. */
   const long getCurrentRow();
  /** Write property of long currentColLong. This method likely to be removed!*/
   bool setCurrentCol( const long theNewVal);
  /** Read property of long currentColLong. */
   const long getCurrentCol();
  /** Write property of long columnsPerRowLong. */
   bool setColumnsPerRow( const long theNewVal);
  /** Read property of long columnsPerRowLong. */
   const long getColumnsPerRow();
  /** Write property of int headerLinesInt. */
   bool setHeaderLines( const int theNewVal);
  /** Read property of int headerLinesInt. */
   const int getHeaderLines();
  /** Write property of int startMonthInt. */
   bool setStartMonth( const int theNewVal);
  /** Read property of int startMonthInt. */
   const int getStartMonth();
  /** Write property of bool endOfMatrixFlag. */
   bool setEndOfMatrixFlag( const bool theNewVal);
  /** Read property of bool endOfMatrixFlag. */
   const bool getEndOfMatrixFlag();
  /** Write property of QString Filename. */
   bool setFilename( QString theNewVal);
  /** Read property of QString Filename. */
   const QString  getFilename();
  /** Write property of FileFormatEnum fileFormat. */
   bool setFileFormat( const FileFormatEnum theNewVal);
  /** Read property of FileFormatEnum fileFormat. Note that
  * return type is FileReader::FileFormatEnum because the calling
  * class does not have the enum in its name space so we need to
  * explicitly specifiy the namespace*/
   const FileReader::FileFormatEnum getFileFormat();
  /** Write property of FileTypeEnum fileType. */
   bool setFileType( const FileTypeEnum theNewVal);
  /** Read property of FileTypeEnum fileType. */
   const FileReader::FileTypeEnum getFileType();
  /** Write property of long yDimLong. */
   bool setYDim( const long theNewVal);
  /** Read property of long yDimLong. */
   const long getYDim();
  /** Write property of long xDimLong. */
   bool setXDim( const long theNewVal);
  /** Read property of long xDimLong. */
   const long getXDim();
  /** Write property of int monthHeaderLinesInt. */
   bool setMonthHeaderLinesInt( const int theNewVal);
  /** Read property of int monthHeaderLinesInt. */
   const int getMonthHeaderLinesInt();
  /** Return the various metadata stored for the open file. */
   QString getFileReaderInfo();
  /** Write property of QFile::Offset headerPos. */
   bool setHeaderOffset( QFile::Offset theNewVal);
  /** Read property of QFile::Offset headerPos. */
   const QFile::Offset getHeaderOffset();
   /** Write property of QFile::Offset dataStartOffset. */
   bool setDataStartOffset( QFile::Offset theNewVal);
  /** Read property of QFile::Offset dataStartOffset. */
   const QFile::Offset getDataStartOffset();
  /** Move the internal file pointer to the start of the file header. */
   bool moveToHeader();
  /** Move the internal file pointer to the start of the file header. */
   bool moveToDataStart();
  /** Use the header info for a given file type to determine the begining of the data block and position the
  *    dataStartOffset there. This method will need to be called explicitly by the client app so that when multiple
  *   copies of the same file are being opened, we dont need to do the same thing each time.*/
   QValueVector <QFile::Offset> getBlockMarkers();

  bool setBlockMarkers(QValueVector <QFile::Offset> theBlockMarkers);
  /** Read property of int taskProgressInt. */
  const int gettaskProgressInt();
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
  QString filenameString;
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
  QFile *filePointer;


  /** Number of header lines per month data block (applicable to files containing multiple months in a single file only. */
  int monthHeaderLinesInt;
  /** The offset of the start of the header. This may not be the start of the file in instances where files contain more the one dataset. */
  QFile::Offset headerOffset;
  /** The offset of the start of the header. This may not be after the header in instances where files contain more the one dataset. */
  QFile::Offset dataStartOffset;
  /* This is a vector  that stores the filepos for each  datablock in the file*/
  QValueVector <QFile::Offset> dataBlockMarkersVector;

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
