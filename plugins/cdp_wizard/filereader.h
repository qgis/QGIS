
#ifndef _FILEREADER_H_
#define _FILEREADER_H_

#include <qfile.h>
#include <iostream>
#include <qstring.h>
#include <qvaluevector.h>
#include <qmap.h>

//other includes
#include <gdal_priv.h>

/** This class will handle opening a file containing a climate matrix and iterating through the file in a columnwise / rowwise manner.
 **/
class FileReader
{

public:

   //
  // Enumerators
  //
  /**
  * This enum defines the different types of files that can be read in.
  */
  enum FileTypeEnum { 
                      HADLEY_SRES,
                      HADLEY_IS92,
                      IPCC_OBSERVED,
                      VALDES,
                      ECHAM4,
                      CSIRO_MK2,
                      NCAR_CSM_PCM,
                      GFDL_R30,
                      CGCM2,
                      CCSR_AGCM_OGCM,
                      GDAL };


  //
  //   Public methods
  //

  /** Default constructor */
  FileReader();
  /** Constructor taking the name of the file to open and its file type. */
  FileReader(QString theFileNameString,const FileTypeEnum theFileType);
  /** Destructor  */
   ~FileReader();
  /**
  * This method will close the file currently associated with the fileReader object.
  *@return bool - flag indicating success or failure
  */
   bool closeFile();

  /**
  * This  will open a given file. The file pointer will be moved to the first matrix element,
  * and any header info will be skipped. This is an overloaded version of openFile to take a
  * QString argument rather than a QString .
  * @param theFileNameString - QString with the filename (including full path) to open.
  * @param theFileType - 
  * @return bool - flag indicating success or failure
  */
   bool openFile(const QString theFileNameString,const FileTypeEnum theFileType);

  /**
  * Mutator of QFile * filePointer.
  * @param theNewVal - A pointer to a QFile object.
  * @return bool - flag indicating success or failure
  */
   bool setFilePointer( QFile*  theNewVal);

  /**
  * Accessor of QFile *filePointer.
  * @return QFile -  a pointer to a QFile object
  */
   const QFile * getFilePointer();

  /**
  *Get the next available element from the file matrix.
  *The cell index will be advanced by one.
  * @return float - the value at the element at the next cell.
  */
   float getElement();

  /**
  *Move the internal pointer to the first matrix element.
  *@todo This method needs to be implemented still!
  *@return bool - flag indicating success or failure
  */
   bool moveFirst();

  /**
  * Mutator for long currentElementLong (calculated as
  * (currentRowLong * rows) + currentColLong).
  * @note This method is deprecated and likely to be removed!
  * @param theNewVal - a long containing the value to be written
  * @return bool - flag indicating success or failure
  */
   bool setCurrentElement( const long theNewVal);

  /**
  * Accessor for long currentElementLong.
  * Calculated as (currentRowLong * rows) + currentColLong.
  * @return long - the current position in the current block.
  */
   const long getCurrentElement();

  /**
  * Mutator for long currentRowLong.
  * @note This method likely to be removed!
  * @param theNewVal - a long with the index of the desired new row pos.
  * @return bool - flag indicating success or failure
  */
   bool setCurrentRow( const long theNewVal);

  /**
  * Accessor property of long currentRowLong.
  * @return long - the current row position in the current block.
  */
   const long getCurrentRow();

  /**
  * Mutator for long currentColLong.
  * @note This method likely to be removed!
  * @param theNewVal - a long with the index of the desired new col pos.
  * @return bool - flag indicating success or failure
  */
   bool setCurrentCol( const long theNewVal);

  /**
  * Accessor property of long currentColLong.
  * @return long - the current row position in the current block.
  */
   const long getCurrentCol();

  /** Mutator for long columnsPerRowLong.
  * @param theNewVal - a long with the index of the desired new col pos.
  * @return bool - flag indicating success or failure
  */
   bool setColumnsPerRow( const long theNewVal);

  /**
  * Accessor for long columnsPerRowLong.
  * @note The number of columns per row differs from the xDimension of the block.
  * This is because the file may wrap / include line breaks before xdim is reached.
  * @return long - the number of columns in each row (as represented in the file).
  */
   const long getColumnsPerRow();

  /**
  * Mutator for int headerLinesInt.
  * @note The number of columns per row differs from the xDimension of the block.
  * This is because the file may wrap / include line breaks before xdim is reached.
  * @param theNewVal - an integer indicating the number of header lines.
  * @return bool - flag indicating success or failure
  */
   bool setHeaderLines( const int theNewVal);

  /**
  * Accessor for the int headerLinesInt.
  * @return int - the current column position in the current block.
  */
   const int getHeaderLines();

  /**
  * Mutator for int startMonthInt.
  * It will move the file pointer too the start of the data block indicated
  * by the start month.
  * This is really only applicable for file formats that include
  * muliple months / years data in a single file such as Hadley SRES data.
  * @param theNewVal - an int representing the new start month.
  * @return bool - flag indicating success or failure
  */
   bool setStartMonth( const int theNewVal);

  /**
  * Accessor of int startMonthInt.
  * This is really only applicable for file formats that include
  * muliple months / years data in a single file such as Hadley SRES data.
  * @return bool - flag indicating success or failure
  */
   const int getStartMonth();

  /**
  * Mutator for bool endOfMatrixFlag.
  * @note Usually you will not want to do this yourself - setting this flag is handled
  * internally by this class.
  * @param theNewVal - bool indicating the desired state of the end of matrix flag.
  * @return bool - flag indicating success or failure
  */
   bool setEndOfMatrixFlag( const bool theNewVal);

  /**
  * Accessor for bool endOfMatrixFlag.
  * @return bool - Current state of endOfMatrixFlag
  */
   const bool getEndOfMatrixFlag();

  /**
  * Mutator for QString Filename.
  * @note The filenam property is changed, BUT the file is NOT reopened and
  * file offsets etc are not altered in any way.
  * @param theNewVal - the new filename
  * @return bool - flag indicating success or failure
  */
   bool setFilename( QString theNewVal);

  /**
  * Accessor for QString Filename.
  * @return QString - the current filename
  */
   const QString  getFilename();

  /**
  * Mutator of FileTypeEnum fileType.
  * @note You should specify the file type BEFORE opening the file.
  * @param theNewVal - a FileTypeEnum specifying the input file type.
  * @return bool - flag indicating success or failure
  */
      bool setFileType( const FileTypeEnum theNewVal);

  /**
  * Read property of FileTypeEnum fileType.
  * @note The return type is FileReader::FileTypeEnum because the calling
  * class does not have the enum in its name space so we need to.
  * explicitly specifiy the namespace.
  * @return FileReader::FileTypeEnum - the file format of the current file.
  */
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

  /**
  * Use the header info for a given file type to determine the
  * begining of the data block(s) and position the
  * dataStartOffset there. This method will need to be called
  * explicitly by the client app so that when multiple
  * copies of the same file are being opened, we dont need to
  * do the same thing each time.
  * @param forceFlag - Force parsing file for block markers. By default this
  * is set to false and the file will only be parsed if an accompanying
  * .bmr file is found.
  * @return QValueVector <QFile::Offset> - a qvalue vector contining a series of
  * file offsets (ulongs) which mark the start of each data block.
  */
   QValueVector <QFile::Offset> getBlockMarkers(bool forceFlag=false);

  /**
  *
  */
  bool setBlockMarkers(QValueVector <QFile::Offset> theBlockMarkers);
  /** Read property of int taskProgressInt. */
  const int gettaskProgressInt();
  /** Find out how many blocks (useful in multiblock formats such as SRES) are in this file. */
  int getNumberOfBlocks();
  /**
  * A helper function to see if the block markers are correct.
  * The value of each block marker plus its value at first element
  * will be printed to console on std out.
  * @return void - No return.
  */
  void printFirstCellInEachBlock();

 /** Typedef to hold a list of supported gdal drivers 
 *   @see getGdalDriverMap()
 */
 typedef QMap<QString, QString> GdalDriverMap;

  /** A helper function that returns a map of the supported gdal raster files
  *   that can be read (see filewriter for a similar function that provides a list 
  *   of types that can be written to).
  * @return GdalDriverMap An associative array where the key is the text description
  *               of the driver, and the value is the extension pattern e.g. .tif
  */
  static void getGdalDriverMap(QString & theFileFiltersString);

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
  /** The text stream that will be used to pull data from the file */
  QTextStream * textStream;
  /** \brief Pointer to the gdaldataset.  */
  GDALDataset * gdalDataset;
  /** \brief The gdal transform object (descibes origin, pixel size etc)  */
  double adfGeoTransform[6];
  /** A helper method to retrieve a value from a gdal scanline */
  double readValue ( void *theData, GDALDataType theType, int theIndex );

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


};

#endif
