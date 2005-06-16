#include "filereader.h"

//qt includes
#include <qmap.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qglobal.h>
#include <qregexp.h>

FileReader::FileReader()
{
  taskProgressInt=0;
}

FileReader::FileReader(QString theFileNameString,const FileTypeEnum theFileType)
{
#ifdef QGISDEBUG
  std::cout << "FileReader::FileReader(QString theFileNameString)" << std::endl;
#endif

  openFile(theFileNameString,theFileType);
  taskProgressInt=0;
}

FileReader::~FileReader()
{
  delete textStream;
  delete filePointer;
}

bool FileReader::openFile(const QString theFileNameQString,const FileTypeEnum theFileType)
{
  filenameString=theFileNameQString;
  //if gdal is being used we have a comlpetely different behaviour
  //as we dont open the file directly but rather use a gda dataset
  if (theFileType==GDAL)
  {
    GDALAllRegister();
    gdalDataset = (GDALDataset *) GDALOpen( theFileNameQString, GA_ReadOnly );
    if( gdalDataset == NULL )
    {
      std::cerr << "Cannot open file : " << theFileNameQString << std::endl;
      return false;
    }

  }
  else //use our own file access mechanism
  {


    filePointer = new QFile ( theFileNameQString );
    if ( !filePointer->open( IO_ReadOnly | IO_Translate) )
    {
      std::cerr << "Cannot open file : " << theFileNameQString << std::endl;
      return false;
    }
#ifdef QGISDEBUG
    std::cout << "Opened file : " << theFileNameQString << " successfully." << std::endl;
#endif
    // now open the text stream on the filereader
    textStream = new QTextStream(filePointer);
  }
  currentColLong=1;
  currentRowLong=1;
  currentElementLong=0;
  setFileType(theFileType);
  return true;
}

bool FileReader::closeFile()
{
  if (fileType==GDAL)
  {
    delete gdalDataset;
  }
  else
  {
    filePointer->close();
  }
  return true;
}

bool FileReader::moveFirst()
{
  std::cout << "Error move first is not implemented yet!";
  return false;
}

float FileReader::getElement()
{
  float myElementFloat=0;
  //see if it is ok to get another element
  if (!endOfMatrixFlag)
  {
    if (fileType==GDAL)
    {
      //get the cell value for current col and row
      GDALRasterBand * myGdalBand = gdalDataset->GetRasterBand(1);
      GDALDataType myType = myGdalBand->GetRasterDataType();
      int mySize = GDALGetDataTypeSize ( myType ) / 8;
      void *myData = CPLMalloc ( mySize );
      //-1 in row is to cater for different offset system used by non gdal readers
      CPLErr err = myGdalBand->RasterIO ( GF_Read, currentColLong, currentRowLong-1, 1, 1, myData, 1, 1, myType, 0, 0 );
      myElementFloat = readValue ( myData, myType, 0 );
      //std::cout << "Gdal Driver retrieved : " << myElementFloat << " at " << currentColLong <<" , " << currentRowLong << " ... from... "<<  filenameString << std::endl;
      free (myData);
    }
    else
    {
      //read a float from the file - this will advance the file pointer
      *textStream >> myElementFloat;
    }
    currentElementLong++;

    //check if we have now run to the end of the matrix
    if (currentElementLong == ((xDimLong*yDimLong)))
    {
      endOfMatrixFlag=true;
    }
    else
    {
      endOfMatrixFlag=false;
    }
  }
  else
  {
    //you should not reach this code because you should stop any reading
    //when end of block has been detected.
    std::cout << " XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX " << std::endl;
    std::cout << " FileReader Notice:                   " << std::endl;
    std::cout << " Error trying to get element beyond   " << std::endl;
    std::cout << " end of block! A vector of zeros will " << std::endl;
    std::cout << " be returned!                         " << std::endl;
    std::cout << " XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX " << std::endl;
  }
  //increment the column and row counter and wrap them if needed
  if ((currentColLong ) == xDimLong)
  {
    currentColLong=1;
    currentRowLong++;
  }
  else
  {
    currentColLong++;
  }
  //std::cout << "Col " << currentColLong << " Row " << currentRowLong << " value : " << myElementFloat << std::endl ;
  return myElementFloat;
}

double FileReader::readValue ( void *theData, GDALDataType theType, int theIndex )
{
  double myVal;

  switch ( theType )
  {
  case GDT_Byte:
    return (double) ((GByte *)theData)[theIndex];
    break;
  case GDT_UInt16:
    return (double) ((GUInt16 *)theData)[theIndex];
    break;
  case GDT_Int16:
    return (double) ((GInt16 *)theData)[theIndex];
    break;
  case GDT_UInt32:
    return (double) ((GUInt32 *)theData)[theIndex];
    break;
  case GDT_Int32:
    return (double) ((GInt32 *)theData)[theIndex];
    break;
  case GDT_Float32:
    return (double) ((float *)theData)[theIndex];
    break;
  case GDT_Float64:
    myVal = ((double *)theData)[theIndex];
    return (double) ((double *)theData)[theIndex];
    break;
  default:
    qWarning("Data type %d is not supported", theType);
  }
  return 0.0;
}


const long FileReader::getXDim()
{
  return xDimLong;
}

bool FileReader::setXDim( const long theNewVal)
{
  try
  {
    xDimLong = theNewVal;
    return true;
  }
  catch (...)
  {
    return false;
  }
}

/** Read property of long yDimLong. */

const long FileReader::getYDim()
{

  return yDimLong;

}

/** Write property of long yDimLong. */

bool FileReader::setYDim( const long theNewVal)
{

  try

  {

    yDimLong = theNewVal;

    return true;

  }

  catch (...)

  {

    return false;

  }

}

/** Read property of FileTypeEnum fileType. */

const FileReader::FileTypeEnum FileReader::getFileType()
{

  return fileType;

}

/** Write property of FileTypeEnum fileType. */

bool FileReader::setFileType( const FileTypeEnum theNewVal)
{

  try
  {
    fileType = theNewVal;
    std::cout << "FileReader::setFileType() -  fileType set to : " << fileType << std::endl;
    //Set Hadley member variables
    if ((fileType == HADLEY_SRES) || (fileType == HADLEY_IS92))
    {
      xDimLong = 96;
      yDimLong = 73;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
    }
    //Set IPCC observed member variables
    else if (fileType == IPCC_OBSERVED)
    {
      xDimLong = 720;
      yDimLong = 360;
      columnsPerRowLong = 720;
      headerLinesInt = 2;
      monthHeaderLinesInt = 0;
    }
    //Set ECHAM4 member variables
    else if (fileType == ECHAM4)
    {
      xDimLong = 128;
      yDimLong = 64;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
    }
    //Set CCCma member variables
    else if (fileType == CGCM2)
    {
      xDimLong = 96;
      yDimLong = 48;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
    }
    //Set CSIRO_Mk2 member variables
    else if (fileType == CSIRO_MK2)
    {
      xDimLong = 64;
      yDimLong = 56;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
    }
    //Set NCAR member variables
    else if (fileType == NCAR_CSM_PCM)
    {
      xDimLong = 128;
      yDimLong = 64;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
    }
    //Set GFDL member variables
    else if (fileType == GFDL_R30)
    {
      xDimLong = 96;
      yDimLong = 80;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
    }
    //Set CCSRC member variables
    else if (fileType == CCSR_AGCM_OGCM)
    {
      xDimLong = 64;
      yDimLong = 32;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
    }
    //use gdal to determine header info
    else if (fileType == GDAL)
    {
      if (!gdalDataset)
      {
        std::cout << "Error : Cant set file type before the file has been opened! " << std::endl;
      }
      xDimLong = gdalDataset->GetRasterXSize();
      yDimLong = gdalDataset->GetRasterYSize();
      columnsPerRowLong = 0; //not used
      headerLinesInt = 0;//not used
      monthHeaderLinesInt = 0;//not  used
    }
    //Set Valdes member variables
    else if (fileType == VALDES)
    {
      /* class user will need to specify rows and cols */
      //xDimLong = frmCDPWizard.txtSpecifyNumCol;
      //yDimLong = frmCDPWizard.txtSpecifyNumRow;
      //columnsPerRowLong = frmCDPWizard.txtSpecifyNumDataCols;
      headerLinesInt = 0;
      monthHeaderLinesInt = 0;
    }//end of filetype handling
    return true;
  }

  catch (...)
  {
    return false;
  }
}


/** Read property of QString Filename. */

const QString  FileReader::getFilename()
{

  return filenameString;

}

/** Write property of QString Filename. */

bool FileReader::setFilename( QString theNewVal)
{

  try

  {

    filenameString = theNewVal;

    return true;

  }

  catch (...)

  {

    return false;

  }

}

/** Read property of bool endOfMatrixFlag. */

const bool FileReader::getEndOfMatrixFlag()
{

  return endOfMatrixFlag;

}

/** Write property of bool endOfMatrixFlag. */

bool FileReader::setEndOfMatrixFlag( const bool theNewVal)
{

  try

  {

    endOfMatrixFlag = theNewVal;

    return true;

  }

  catch (...)

  {

    return false;

  }



}

/** Read property of int startMonthInt. */

const int FileReader::getStartMonth()
{

  return startMonthInt;

}

/** Write property of int startMonthInt. */

bool FileReader::setStartMonth( const int theNewVal)
{

  try

  {

    startMonthInt = theNewVal;

    //get the QFile::Offset of the month

    QFile::Offset myOffset = dataBlockMarkersVector[theNewVal-1];
    setDataStartOffset(myOffset);
    moveToDataStart();
    return true;
  }
  catch (...)
  {
    return false;
  }

}

const int FileReader::getHeaderLines()
{
  return headerLinesInt;
}

bool FileReader::setHeaderLines( const int theNewVal)
{
  try
  {
    headerLinesInt = theNewVal;
    return true;
  }

  catch (...)
  {
    return false;
  }
}
/** @note - I'd like to get of 'columns per row' type functionality if possible */
const long FileReader::getColumnsPerRow()
{
  return columnsPerRowLong;
}

bool FileReader::setColumnsPerRow( const long theNewVal)
{
  try
  {
    columnsPerRowLong = theNewVal;
    return true;
  }
  catch (...)
  {
    return false;
  }
}

const long FileReader::getCurrentCol()
{
  return currentColLong;
}

bool FileReader::setCurrentCol( const long theNewVal)
{
  try
  {
    currentColLong = theNewVal;
    return true;
  }
  catch (...)
  {
    return false;
  }
}

const long FileReader::getCurrentRow()
{
  return currentRowLong;
}

bool FileReader::setCurrentRow( const long theNewVal)
{
  try

  {
    currentRowLong = theNewVal;
    return true;
  }
  catch (...)
  {
    return false;
  }
}

const long FileReader::getCurrentElement()
{
  return currentElementLong;
}
/** @todo remove this method!
  @note this method is deprecated - use file writer! */
bool FileReader::setCurrentElement( const long theNewVal)
{
  try
  {
    currentElementLong = theNewVal;
    return true;
  }
  catch (...)
  {
    return false;
  }
}

const QFile* FileReader::getFilePointer()
{
  return filePointer;
}

bool FileReader::setFilePointer( QFile* theNewVal)
{
  try
  {
    //close any currently opened filepointer
    //if (filePointer) fclose(filePointer);
    filePointer = theNewVal;
    currentColLong=0;
    currentRowLong=1;
    currentElementLong=0;
    return true;
  }
  catch (...)
  {
    return false;
  }
}

const int FileReader::getMonthHeaderLinesInt()
{
  return monthHeaderLinesInt;
}

bool FileReader::setMonthHeaderLinesInt( const int theNewVal)
{
  try
  {
    monthHeaderLinesInt = theNewVal;
    return true;
  }
  catch (...)
  {
    return false;
  }
}

QString FileReader::getFileReaderInfo()
{
  QString myMetadataString = "";
  return myMetadataString;
}

const QFile::Offset FileReader::getHeaderOffset()
{
  return headerOffset;
}

bool FileReader::setHeaderOffset( QFile::Offset theNewVal)
{
  try
  {
    headerOffset = theNewVal;
    return true;
  }
  catch (...)
  {
    return false;
  }
}

const QFile::Offset FileReader::getDataStartOffset()
{
  return dataStartOffset;
}

bool FileReader::setDataStartOffset( QFile::Offset theNewVal)
{
  try
  {
    dataStartOffset = theNewVal;

    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool FileReader::moveToHeader()
{
  try
  {
    headerOffset=filePointer->at();
    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool FileReader::moveToDataStart()
{

  try
  {
    if (!fileType==GDAL)
    {
      filePointer->at(dataStartOffset);
      headerOffset=filePointer->at();
    }
    currentElementLong=0;
    currentColLong = 0;
    currentRowLong=1;
    endOfMatrixFlag=false;
    return true;
  }
  catch (...)
  {
    return false;
  }
}


QValueVector <QFile::Offset> FileReader::getBlockMarkers(bool forceFlag)
{
  if (fileType==GDAL)
  {
    dataBlockMarkersVector.clear();
    return dataBlockMarkersVector;
  }
  //
  // Set up some vars
  //
  QFile::Offset myFileOffset; //store the current position in the file
  long myMatrixRowsLong;
  long myFileSizeLong;
  long myFileOffsetLong;  //where we are in the currently open file
  myFileOffsetLong=0;
  //clear the vector
  dataBlockMarkersVector.clear();

  //
  //see if we can retrieve the block markers from the .bmr file (if one exists)
  //

  QString myBmrFileName = filenameString;
  std::cout << "Setting block markers file to : " << filenameString << std::endl;
  bool myBmrExistsFlag=false;
  //replace the extension with .bmr - we assum the extension is last 3 chars
  myBmrFileName =  myBmrFileName.left(myBmrFileName.length()-3) + QString("bmr");
  std::cout << "Looking for block markers file : " << myBmrFileName << std::endl;
  if (QFile::exists(myBmrFileName))
  {
    QFile myBmrFile( myBmrFileName );
    if ( !myBmrFile.open( IO_ReadOnly | IO_Translate) )
    {
      std::cerr << "Cannot open file : " << myBmrFileName << " will reparse..." << std::endl;
    }
    else if (forceFlag)
    {
      std::cerr << "Bmr file ignored - forceFlag is true " << std::endl;
    }
    else
    {
#ifdef QGISDEBUG
      std::cout << "Opened block marker file : " << myBmrFileName << " successfully." << std::endl;
#endif
      // now open the text stream on the filereader
      QTextStream myTextStream(&myBmrFile);
      while (!myTextStream.eof())
      {
        myTextStream >> myFileOffset ;
        dataBlockMarkersVector.push_back(myFileOffset);
      }
      myBmrExistsFlag=true;
      myBmrFile.close();
    }
  }
  if (myBmrExistsFlag)
  {
    //for debugging purposes only!
#ifdef QGISDEBUG
    printFirstCellInEachBlock();
#endif
    //no need to do any further parsing because we were able to
    // get the cached bmrs from file

    return dataBlockMarkersVector;
  }


  //
  // Open the bmr file for writing seeing that it doesnt exists
  // or the calling fn has asked for forced parsing
  //
  std::cout << "Opening block markers file to persist bmr's : " << myBmrFileName << std::endl;
  QFile myBmrFile( myBmrFileName );
  QTextStream myOuputTextStream;
  if ( !myBmrFile.open( IO_WriteOnly ) )
  {
    std::cerr << "Cannot open file : " << myBmrFileName << std::endl;
  }
  else
  {
#ifdef QGISDEBUG
    std::cout << "Opened block marker file : " << myBmrFileName << " successfully." << std::endl;
#endif
    // now open the text stream on the filereader
    myOuputTextStream.setDevice(&myBmrFile);
  }

  //if the datafile is a an arc/info grid file, there is only one data block
  if (fileType==GDAL)
  {
    for (int i=1; i <= headerLinesInt; i++)
    {
      //read a line from the file - this will advance the file pointer
      *textStream->readLine();
    }
    myFileOffset=filePointer->at();
    dataStartOffset=myFileOffset;
    dataBlockMarkersVector.push_back(myFileOffset);
    myOuputTextStream << myFileOffset;
    myBmrFile.close();
    return dataBlockMarkersVector;
  }
#ifdef QGISDEBUG
  printf ("FileReader::getBlockMarkers block xDimLong is %i, block yDimLong is %i.\n",
          xDimLong, yDimLong);
#endif

  //
  // Start parsing the file
  //

#ifdef QGISDEBUG

  std::cout << "FileReader::getBlockMarkers() - moving to the start of the file" << filePointer->name() << std::endl;
#endif

  if (!filePointer->exists())
  {
    return false;
  }
  //make sure were at the start of the file -> retruns false if we fail to move there
  if (!filePointer->at(0))
  {
    return 0;
  }
#ifdef QGISDEBUG
  std::cout << "FileReader::getBlockMarkers() - skipping " << headerLinesInt << " file header line(s)" << std::endl;
#endif
  //skip header lines at the top of the file
  for (int i=0; i < headerLinesInt; i++)
  {
    *textStream->readLine();
  }
  //Calculate number of rows in a month (depends on FileType)
  /** @todo Its going to better to avoid using colsPerRow
  /*  if possible and rather just read in in x * y elements */
  myMatrixRowsLong = ((xDimLong * yDimLong) / columnsPerRowLong);
#ifdef QGISDEBUG

  std::cout << "FileReader::getBlockMarkers() - looping through rest of file with month header size of " << monthHeaderLinesInt << " lines and data block size of " << myMatrixRowsLong << " lines." << std::endl;
#endif
  //loop through the rest of the file getting the start pos for each datablock
  do
  {
#ifdef QGISDEBUG
    std::cout << "FileReader::getBlockMarkers()  getting header block " << std::endl;
#endif
    //skip the datablock headers
    if (monthHeaderLinesInt > 0)
    {
      for (int i=1; i <= monthHeaderLinesInt; i++)
      {
        //read an impossibly long line - fgets will stop if it hits a newline
        *textStream->readLine();
#ifdef QGISDEBUG

        std::cout << myLineQString;
#endif

      }
    }
#ifdef QGISDEBUG
    else
    {
      std::cout << "no datablock header" << std::endl;
    }
    //std::cout << "FileReader::getBlockMarkers()  getting data block " << std::endl;
#endif

    //so the file pointer is now offset() the start of the datablock - add it to the vector
    myFileOffset=filePointer->at();
    dataBlockMarkersVector.push_back(myFileOffset);
    //write this marker to our bmr file
    myOuputTextStream << myFileOffset << "\n";
    //now skip the data objects for this datablock
    for (int i=1; i <= myMatrixRowsLong; i++)
    {
      *textStream->readLine();
#ifdef QGISDEBUG
      //std::cout << myLineQString; //print out the first line of each datablock
#endif

    }
    //calculate where we are in the file and update the progress member var
    myFileOffsetLong = filePointer->at();
    /* Note we cannot simply divide two longs and expect to get a float!
     * as the following excerpt from Spencer Collyer shows:
     *
     * The problem is that the expression is being evaluated using integer
     * arithmetic. This is the way C++ (and C) do their arithmetic if there are
     * no floating point values involved. The division 10388820/108262440 in
     * integer maths gives you 0, and so the whole expression will return 0.
     *
     * As it looks like you want a floating point result, you need to force one
     * of the numbers to be floating point. If they are actual constants, just
     * make one of the numbers in the division sub-expression floating point
     * (e.g. 10388820/108262440.0 would work) and the whole expression will be
     * evaluated using fp maths.
     */
    taskProgressInt = static_cast<int>(( static_cast<float>(myFileOffsetLong) / myFileSizeLong) * 100 );
#ifdef QGISDEBUGNONONO

    std::cout << "Task Progress: " << ( static_cast<float>(myFileOffsetLong) / myFileSizeLong) * 100 << std::endl;
    std::cout << "Position " << myFileOffsetLong << "/" << myFileSizeLong << " ("
    << taskProgressInt << ") : ";
    for (int i=1;i<(taskProgressInt/10);i++)
    {
      std::cout << "*";
    }
    std::cout << std::endl;
#endif

  }
  while (!filePointer->atEnd()) ;
#ifdef QGISDEBUG

  std::cout << "FileReader::getBlockMarkers() - read markers for " << dataBlockMarkersVector.size() << " data block(s)" << std::endl;
  std::cout << "FileReader::getBlockMarkers() - moving back to the start of the file" << std::endl;
#endif

  filePointer->at(0);
#ifdef QGISDEBUG

  std::cout << "FileReader::getBlockMarkers() - finished - returning vector of datablock start positions" << std::endl;
#endif

  myBmrFile.close();

  return  dataBlockMarkersVector;
}



bool FileReader::setBlockMarkers(QValueVector <QFile::Offset> theBlockMarkersVector)

{
  if (theBlockMarkersVector.size()==0)
  {
#ifdef QGISDEBUG
    std::cout << "setBlockMarkers() received empty vector!" << std::endl;
#endif

    return false;
  }
  else
  {
    dataBlockMarkersVector.clear();
    QValueVector<QFile::Offset>::iterator myIterator = theBlockMarkersVector.begin();
    while (myIterator != theBlockMarkersVector.end())
    {
      dataBlockMarkersVector.push_back(*myIterator);
      myIterator++;
    }
  }
  //presume all went ok - need to add better error checking later
  return true;
}

const int FileReader::gettaskProgressInt()
{
  return taskProgressInt;
}

int FileReader::getNumberOfBlocks()
{
  return dataBlockMarkersVector.size();
}

void FileReader::printFirstCellInEachBlock()
{
  double myElementDouble=0;
  QValueVector<QFile::Offset>::iterator myIterator = dataBlockMarkersVector.begin();
  while (myIterator != dataBlockMarkersVector.end())
  {
    QFile::Offset myOffset = *myIterator;
    filePointer->at(myOffset);
    *textStream >> myElementDouble;
    std::cout << "Offset " <<  myOffset << " -> Element Value " << myElementDouble << std::endl;
    myIterator++;
  }
}


FileReader::GdalDriverMap FileReader::getGdalDriverMap()
{
  GdalDriverMap myGdalDriverMap;
  // This code was taken largely from qgis qgsrasterlayer.cpp
  GDALAllRegister();
  GDALDriverManager *myGdalDriverManager = GetGDALDriverManager();

  if (!myGdalDriverManager)
  {
    std::cerr << "unable to get GDALDriverManager\n" << std::endl;
    return myGdalDriverMap;                   // XXX good place to throw exception if we
  }                           // XXX decide to do exceptions

  // then iterate through all of the supported drivers, adding the
  // corresponding file filter

  GDALDriver *myGdalDriver;           // current driver

  char **myGdalDriverMetadata;        // driver metadata strings

  QString myGdalDriverLongName("");   // long name for the given driver
  QString myGdalDriverExtension("");  // file name extension for given driver
  QString myGdalDriverDescription;    // QString wrapper of GDAL driver description
  QStringList metadataTokens;   // essentially the metadata string delimited by '='
  QString catchallFilter;       // for Any file(*.*), but also for those
  // drivers with no specific file
  // filter

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, welll, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.
  // Note that file name extension strings are of the form
  // "DMD_EXTENSION=.*".  We'll also store the long name of the
  // driver, which will be found in DMD_LONGNAME, which will have the
  // same form.
  for (int i = 0; i < myGdalDriverManager->GetDriverCount(); ++i)
  {
    myGdalDriver = myGdalDriverManager->GetDriver(i);
    Q_CHECK_PTR(myGdalDriver);
    if (!myGdalDriver)
    {
      qWarning("unable to get driver %d", i);
      continue;
    }


    //add this driver to the map
    // std::cerr << "got driver string " << myGdalDriver->GetDescription() << "\n";

    myGdalDriverMetadata = myGdalDriver->GetMetadata();

    // presumably we know we've run out of metadta if either the
    // address is 0, or the first character is null
    while (myGdalDriverMetadata && '\0' != myGdalDriverMetadata[0])
    {
      metadataTokens = QStringList::split("=", *myGdalDriverMetadata);
      // std::cerr << "\t" << *myGdalDriverMetadata << "\n";

      // XXX add check for malformed metadataTokens

      // Note that it's oddly possible for there to be a
      // DMD_EXTENSION with no corresponding defined extension
      // string; so we check that there're more than two tokens.

      if (metadataTokens.count() > 1)
      {
        std::cout << "Listing GDAL Metadata tokens: "  << std::endl;
        for (int i = 0; i < metadataTokens.count() ; i++)
        {
          std::cout << "GDAL Metadata token: " << metadataTokens[i] << std::endl;
        }
        if ("DMD_EXTENSION" == metadataTokens[0])
        {
          myGdalDriverExtension = metadataTokens[1];
          std::cout << "Extension found: " << myGdalDriverExtension << std::endl;

        }
        else if ("DMD_LONGNAME" == metadataTokens[0])
        {
          myGdalDriverLongName = metadataTokens[1];

          // remove any superfluous (.*) strings at the end as
          // they'll confuse QFileDialog::getOpenFileNames()

          myGdalDriverLongName.remove(QRegExp("\\(.*\\)$"));
        }
      }
      // if we have both the file name extension and the long name,
      // then we've all the information we need for the current
      // driver; therefore emit a file filter string and move to
      // the next driver
      if (!(myGdalDriverExtension.isEmpty() || myGdalDriverLongName.isEmpty()))
      {
        myGdalDriverMap[myGdalDriverLongName]=myGdalDriverExtension;
        break;            // ... to next driver, if any.
      }

      ++myGdalDriverMetadata;

    }                       // each metadata item
    if (myGdalDriverExtension.isEmpty() && !myGdalDriverLongName.isEmpty())
    {
      // Then what we have here is a driver with no corresponding
      // file extension; e.g., GRASS.  In which case we append the
      // string to the "catch-all" which will match all file types.
      // (I.e., "*.*") We use the driver description intead of the
      // long time to prevent the catch-all line from getting too
      // large.

      // ... OTOH, there are some drivers with missing
      // DMD_EXTENSION; so let's check for them here and handle
      // them appropriately

      // USGS DEMs use "*.dem"
      if (myGdalDriverDescription.startsWith("USGSDEM"))
      {
        myGdalDriverMap[myGdalDriverLongName]="dem";
      }
      else if (myGdalDriverDescription.startsWith("DTED"))
      {
        // DTED use "*.dt0"
        myGdalDriverMap[myGdalDriverLongName]="dt0";
      }
      else if (myGdalDriverDescription.startsWith("MrSID"))
      {
        // MrSID use "*.sid"
        myGdalDriverMap[myGdalDriverLongName]="sid";
      }
      else
      {
        myGdalDriverMap["All other types"]="*";
      }
    }
  }
  return myGdalDriverMap;
}
