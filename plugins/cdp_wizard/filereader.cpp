#include "filereader.h"

#include <cassert>  //assert debugging
//qt includes
#include <qmap.h>
#include <qtextstream.h>
//move this somewhere else!
#define QGISDEBUG false

FileReader::FileReader()

{
  taskProgressInt=0;
}

FileReader::FileReader(QString theFileNameString)
{

#ifdef QGISDEBUG
  std::cout << "FileReader::FileReader(QString theFileNameString)" << std::endl;
#endif
  openFile(theFileNameString);
  taskProgressInt=0;

}

FileReader::~FileReader()
{
  delete textStream;
  delete filePointer;
}

/**
  This method will open a given file. The file pointer will be moved to the first matrix element, and any header info will be skipped.
  @param open. The filename (including full path) to open.
  */
bool FileReader::openFile(const QString theFileNameQString)
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
  currentColLong=1;
  currentRowLong=1;
  currentElementLong=0;
  return true;
}
/**
  This method will close the file currently associated with the fileReader object.
  */
bool FileReader::closeFile()
{
    filePointer->close();
    return true;
}

/**
  Move the internal pointer to the first matrix of the current data block element.
  */
bool FileReader::moveFirst()
{
  std::cout << "Error move first is not implemented yet!";
}

/**
  Get the next available element from the file matrix.
  */
float FileReader::getElement()
{
  float myElementFloat=0;
  //see if it is ok to get another element
  if (!endOfMatrixFlag)
  {

    //read a float from the file - this will advance the file pointer
    *textStream >> myElementFloat;
    currentElementLong++;
#ifdef QGISDEBUG
    //print out the last entries for debuggging
    if (currentElementLong > 0)
    {
     // if (debugModeFlag) std::cout << "FileReader::getElement() retrieved value : " << myElementFloat << " for element no " << currentElementLong << std::endl;
    }
#endif
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
  return myElementFloat;
}
/** Read property of long xDimLong. */
const long FileReader::getXDim(){
  return xDimLong;
}
/** Write property of long xDimLong. */
bool FileReader::setXDim( const long theNewVal){
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
const long FileReader::getYDim(){
  return yDimLong;
}
/** Write property of long yDimLong. */
bool FileReader::setYDim( const long theNewVal){
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
const FileReader::FileTypeEnum FileReader::getFileType(){
  return fileType;
}
/** Write property of FileTypeEnum fileType. */
bool FileReader::setFileType( const FileTypeEnum theNewVal){

#ifdef QGISDEBUG
  if (debugModeFlag) std::cout << "FileReader::setFileType() -  called with fileType: " << theNewVal << std::endl;
#endif
  try
  {
    fileType = theNewVal;
#ifdef QGISDEBUG
    if (debugModeFlag) std::cout << "FileReader::setFileType() -  fileType set to : " << fileType << std::endl;
#endif
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
    //Set ArcInfo ASCII grid and CRES member variables
    else if ((fileType == ARCINFO_GRID) || (fileType == CRES))
    {
      /* Typical Header:
         ncols         241
         nrows         207
         xllcorner     -795282.26306056
         yllcorner     -3846910.6343577
         cellsize      7128.3696735486
         NODATA_value  -9999
         */

#ifdef QGISDEBUG
      std::cout << "FileReader::setFileType() - setting file type to ARCINFO_GRID / CRES" << std::endl;
#endif
      headerLinesInt = 6;
      monthHeaderLinesInt = 0;
      if (filePointer==0)
      {
        return false;
      }
      //Just testing remove this later! vvvvvvvvv
      //fseek(filePointer,0,SEEK_END);
      //long myFileSizeLong = ftell(filePointer);
      //rewind(filePointer);
      //Just testing remove this later!  ^^^^^^^^^^
      float myFloat;
      QString myString;

      //bit of hoop jumping here
      //fgetpos (filePointer, headerOffset);

#ifdef QGISDEBUG
      std::cout << "FileReader::setFileType()- creating properties QMap" << std::endl;
#endif
      /*Create the QMap (associative array) to store the header key value pairs.
       * Doing it this way means that we dont need to worry about the order of the header
       * fields in the file */
      QMap <QString, float > myHeaderMap;
      for (int i=0; i <6;i++)
      {

          //read a float from the file - this will advance the file pointer
         *textStream >> myString;

        //fscanf (filePointer, "%s", myString);
        myString=myString.upper();   //make sure all keys are in upper case!
        *textStream >> myFloat;
        //fscanf (filePointer, "%f", &myFloat);

        myHeaderMap[myString]=myFloat;
      }
      //print the QMap contents to stdout using an iterator
      QMap<QString, float>::const_iterator myIterator;
      for (myIterator=myHeaderMap.begin(); myIterator != myHeaderMap.end(); myIterator++)
      {
        std::cout << "FileReader::setFileType() retrieved values : " << myIterator.key().latin1() << " --- " << myIterator.data() << std::endl;
      }

      //good, now we can assign the member vars their value
      //next four are not currently implemented used:
      //cellSizeFloat = myHeaderMap["CELLSIZE"];
      //lowerLeftXFloat = myHeaderMap["XLLCORNER"];
      //lowerLeftYFloat = myHeaderMap["YLLCORNER"];
      // noDataValueFloat = myHeaderMap["NODATA_VALUE"];
      //this may cause problems if the arcinfo data structure does not match the file structure!
      xDimLong  = static_cast<long>(myHeaderMap["NCOLS"]);            //note implicit cast from float to long int
      yDimLong = static_cast<long>(myHeaderMap["NROWS"]);            //note implicit cast from float to long int

      //set the start of data block pointer
      //fgetpos (filePointer,dataStartOffset); //erk this causes a crash
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
/** Read property of FileFormatEnum fileFormat. */
const FileReader::FileFormatEnum FileReader::getFileFormat(){
  return fileFormat;
}
/** Write property of FileFormatEnum fileFormat. */
bool FileReader::setFileFormat( const FileFormatEnum theNewVal){
  try
  {
    fileFormat = theNewVal;
    return true;
  }
  catch (...)
  {
    return false;
  }
}
/** Read property of QString Filename. */
const QString  FileReader::getFilename(){
  return filenameString;
}
/** Write property of QString Filename. */
bool FileReader::setFilename( QString theNewVal){
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
const bool FileReader::getEndOfMatrixFlag(){
  return endOfMatrixFlag;
}
/** Write property of bool endOfMatrixFlag. */
bool FileReader::setEndOfMatrixFlag( const bool theNewVal){
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
const int FileReader::getStartMonth(){
  return startMonthInt;
}
/** Write property of int startMonthInt. */
bool FileReader::setStartMonth( const int theNewVal){
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
/** Read property of int headerLinesInt. */
const int FileReader::getHeaderLines(){
  return headerLinesInt;
}
/** Write property of int headerLinesInt. */
bool FileReader::setHeaderLines( const int theNewVal){
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
/** Read property of long columnsPerRowLong. */
const long FileReader::getColumnsPerRow(){
  return columnsPerRowLong;
}
/** Write property of long columnsPerRowLong. */
bool FileReader::setColumnsPerRow( const long theNewVal){
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
/** Read property of long currentColLong. */
const long FileReader::getCurrentCol(){
  return currentColLong;
}
/** Write property of long currentColLong. */
bool FileReader::setCurrentCol( const long theNewVal){
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
/** Read property of long currentRowLong. */
const long FileReader::getCurrentRow(){
  return currentRowLong;
}
/** Write property of long currentRowLong. */
bool FileReader::setCurrentRow( const long theNewVal){
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
/** Read property of long currentElementLong. This is the position, not value of the the current element*/
const long FileReader::getCurrentElement(){
  return currentElementLong;
}
/** Write property of long currentElementLong. */
bool FileReader::setCurrentElement( const long theNewVal){
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


/** Read property of FILE *filePointer. */
const QFile* FileReader::getFilePointer(){
  return filePointer;
}
/** Write property of FILE *filePointer. Make sure the file was opened in binary mode*/
bool FileReader::setFilePointer( QFile* theNewVal){
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
/** Read property of int monthHeaderLinesInt. */
const int FileReader::getMonthHeaderLinesInt(){
  return monthHeaderLinesInt;
}
/** Write property of int monthHeaderLinesInt. */
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

/** Return the various metadata stored for the open file. */
QString FileReader::getFileReaderInfo()
{
  QString myMetadataString = "";



  return myMetadataString;
}
/** Read property of QFile::Offset headerPos. */
const QFile::Offset FileReader::getHeaderOffset(){
  return headerOffset;
}
/** Write property of QFile::Offset headerPos. */
bool FileReader::setHeaderOffset( QFile::Offset theNewVal){
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
/** Read property of QFile::Offset dataStartOffset. */
const QFile::Offset FileReader::getDataStartOffset(){
  return dataStartOffset;

}

/** Write property of QFile::Offset dataStartOffset. */
bool FileReader::setDataStartOffset( QFile::Offset theNewVal){
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
/** Move the internal file pointer to the start of the file header. */
bool FileReader::moveToHeader(){
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

/** Move the internal file pointer to the start of the current file data block. */
bool FileReader::moveToDataStart(){
  try
  {

    headerOffset=filePointer->at();
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

/** Use the header info for a given file type to determine the begining of the data block and position the
 *    dataStartOffset there. This method will need to be called explicitly by the client app so that when multiple
 *   copies of the same file are being opened, we dont need to do the same thing each time.*/
QValueVector <QFile::Offset> FileReader::getBlockMarkers()
{
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

  //if the datafile is a an arc/info grid file, there is only one data block
  if (fileType==ARCINFO_GRID || fileType==CRES)
  {

    for (int i=1; i <= headerLinesInt; i++)
    {

    //read a line from the file - this will advance the file pointer
    *textStream->readLine();

    }
    myFileOffset=filePointer->at();
    dataStartOffset=myFileOffset;
    dataBlockMarkersVector.push_back(myFileOffset);       //was *dataStartOffset
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
  for (int i=1; i <= headerLinesInt; i++)
  {
    //read an impossibly long line - fgets will stop if it hits a newline
    *textStream->readLine();
  }

  //Calculate number of rows in a month (depends on FileType)
  myMatrixRowsLong = ((xDimLong * yDimLong) / columnsPerRowLong);
#ifdef QGISDEBUG
  std::cout << "FileReader::getBlockMarkers() - looping through rest of file with month header size of " << monthHeaderLinesInt << " lines and data block size of " << myMatrixRowsLong << " lines." << std::endl;
#endif
  //loop through the rest of the file getting the start pos for each datablock
  do
  {
#ifdef QGISDEBUG
    //std::cout << "FileReader::getBlockMarkers()  getting header block " << std::endl;
#endif
    //skip the datablock headers
    if (monthHeaderLinesInt > 0)
    {
      for (int i=1; i <= monthHeaderLinesInt; i++)
      {
        //read an impossibly long line - fgets will stop if it hits a newline
        *textStream->readLine();
#ifdef QGISDEBUG
        //std::cout << myLineQString;
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


  }    while (!filePointer->atEnd()) ;


#ifdef QGISDEBUG
  std::cout << "FileReader::getBlockMarkers() - read markers for " << dataBlockMarkersVector.size() << " data block(s)" << std::endl;
  std::cout << "FileReader::getBlockMarkers() - moving back to the start of the file" << std::endl;
#endif
  filePointer->at(0);
#ifdef QGISDEBUG
  std::cout << "FileReader::getBlockMarkers() - finished - returning vector of datablock start positions" << std::endl;
#endif

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

/** Read property of int taskProgressInt. */
const int FileReader::gettaskProgressInt(){
  return taskProgressInt;
}
/** Find out how many blocks (useful in multiblock formats such as SRES) are in this file. */
int FileReader::getNumberOfBlocks(){
  return dataBlockMarkersVector.size();
}
