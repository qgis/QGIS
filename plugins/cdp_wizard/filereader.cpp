#include "filereader.h"
#include <cassert>  //assert debugging
#define PLATFORM_IS_WIN 1 //0 = linux
using namespace std;

FileReader::FileReader()

{
  debugModeFlag=false;
  taskProgressInt=0;
}

FileReader::FileReader(std::string theFileNameString)
{

  debugModeFlag=false;
  if (debugModeFlag) cout << "FileReader::FileReader(std::string theFileNameString)" << endl;
  openFile(&theFileNameString);
  taskProgressInt=0;

}  

FileReader::~FileReader()
{
}

/**
  This method will open a given file. The file pointer will be moved to the first matrix element, and any header info will be skipped.
  @param open. The filename (including full path) to open.
  */
bool FileReader::openFile(const char *theFileNameChar)
{
  if ((filePointer=fopen(theFileNameChar,"rb"))==NULL)  //open in binary mode
  {
    if (debugModeFlag) cout << "Cannot open file : " << theFileNameChar << endl;
    return false;
  }
  if (debugModeFlag) cout << "Opened file : " << theFileNameChar << " successfully." << endl;
  currentColLong=1;
  currentRowLong=1;
  currentElementLong=0;    
  return true;
}
/** Overloaded version of openFile */
bool FileReader::openFile(const std::string *theFileNameString)
{
  openFile( theFileNameString->c_str() );
  //presume all went ok - need to add better error checking later
  return true;  
}  
/**
  This method will close the file currently associated with the fileReader object.
  */
bool FileReader::closeFile()
{
  try {
    fclose (filePointer);
    return true;
  }
  catch (...)
  {
    return false;
  }
}

/**
  Move the internal pointer to the first matrix of the current data block element.
  */
bool FileReader::moveFirst()
{
  try
  {

    return true;
  }
  catch (...)
  {
    return false;
  }
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
    fscanf (filePointer, "%f", &myElementFloat);
    currentElementLong++;
    //print out the last entries for debuggging
    if (currentElementLong > 0)
    {
      if (debugModeFlag) cout << "FileReader::getElement() retrieved value : " << myElementFloat << " for element no " << currentElementLong << endl;
    }
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
const long& FileReader::getXDim(){
  return xDimLong;
}
/** Write property of long xDimLong. */
bool FileReader::setXDim( const long& theNewVal){
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
const long& FileReader::getYDim(){
  return yDimLong;
}
/** Write property of long yDimLong. */
bool FileReader::setYDim( const long& theNewVal){
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
const FileReader::FileTypeEnum& FileReader::getFileType(){
  return fileType;
}
/** Write property of FileTypeEnum fileType. */
bool FileReader::setFileType( const FileTypeEnum& theNewVal){

  if (debugModeFlag) cout << "FileReader::setFileType() -  called with fileType: " << theNewVal << endl;
  try
  {
    fileType = theNewVal;
    if (debugModeFlag) cout << "FileReader::setFileType() -  fileType set to : " << fileType << endl;
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

      if (debugModeFlag) cout << "FileReader::setFileType() - setting file type to ARCINFO_GRID / CRES" << endl;
      headerLinesInt = 6;
      monthHeaderLinesInt = 0;
      assert(filePointer);
      //Just testing remove this later! vvvvvvvvv
      fseek(filePointer,0,SEEK_END);
      long myFileSizeLong = ftell(filePointer);
      rewind(filePointer);
      //Just testing remove this later!  ^^^^^^^^^^
      float myFloat;
      char myChar[30];

      //bit of hoop jumping here
      //fgetpos (filePointer, headerFPos);

      if (debugModeFlag) cout << "FileReader::setFileType()- creating properties map" << endl;
      /*Create the map (associative array) to store the header key value pairs.
       * Doing it this way means that we dont need to worry about the order of the header
       * fields in the file */
      std::map <std::string, float > myHeaderMap;
      for (int i=0; i <6;i++)
      {
        fscanf (filePointer, "%s", myChar);
        std::string myString;
        myString = myChar;     //assign the character array to the string object!
#if PLATFORM_IS_WIN
        //transform (myString.begin(),myString.end(), myString.begin(), toupper);   //make sure all keys are in upper case!
#else
        transform (myString.begin(),myString.end(), myString.begin(), toupper);   //make sure all keys are in upper case!
#endif
        fscanf (filePointer, "%f", &myFloat);

        myHeaderMap[myString]=myFloat;
      }
      //print the map contents to stdout using an stl iterator
      map<std::string, float>::const_iterator iter;
      for (iter=myHeaderMap.begin(); iter != myHeaderMap.end(); iter++)
      {
        cout << "FileReader::setFileType() retrieved values : " << iter->first << " --- " << iter->second << endl;
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
      //fgetpos (filePointer,dataStartFPos); //erk this causes a crash
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
const FileReader::FileFormatEnum& FileReader::getFileFormat(){
  return fileFormat;
}
/** Write property of FileFormatEnum fileFormat. */
bool FileReader::setFileFormat( const FileFormatEnum& theNewVal){
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
/** Read property of char *Filename. */
const std::string* FileReader::getFilename(){
  return filenameString;
}
/** Write property of char *Filename. */
bool FileReader::setFilename( std::string *theNewVal){
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
const bool& FileReader::getEndOfMatrixFlag(){
  return endOfMatrixFlag;
}
/** Write property of bool endOfMatrixFlag. */
bool FileReader::setEndOfMatrixFlag( const bool& theNewVal){
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
const int& FileReader::getStartMonth(){
  return startMonthInt;
}
/** Write property of int startMonthInt. */
bool FileReader::setStartMonth( const int& theNewVal){
  try
  {
    startMonthInt = theNewVal;
    //get the fpos_t of the month
    fpos_t myFPos = dataBlockMarkersVector[theNewVal-1];
    setDataStartFPos(myFPos);
    moveToDataStart();
    return true;
  }
  catch (...)
  {
    return false;
  }
}
/** Read property of int headerLinesInt. */
const int& FileReader::getHeaderLines(){
  return headerLinesInt;
}
/** Write property of int headerLinesInt. */
bool FileReader::setHeaderLines( const int& theNewVal){
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
const long& FileReader::getColumnsPerRow(){
  return columnsPerRowLong;
}
/** Write property of long columnsPerRowLong. */
bool FileReader::setColumnsPerRow( const long& theNewVal){
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
const long& FileReader::getCurrentCol(){
  return currentColLong;
}
/** Write property of long currentColLong. */
bool FileReader::setCurrentCol( const long& theNewVal){
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
const long& FileReader::getCurrentRow(){
  return currentRowLong;
}
/** Write property of long currentRowLong. */
bool FileReader::setCurrentRow( const long& theNewVal){
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
const long& FileReader::getCurrentElement(){
  return currentElementLong;
}
/** Write property of long currentElementLong. */
bool FileReader::setCurrentElement( const long& theNewVal){
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
const FILE* FileReader::getFilePointer(){
  return filePointer;
}
/** Write property of FILE *filePointer. Make sure the file was opened in binary mode*/
bool FileReader::setFilePointer( FILE* theNewVal){
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
const int& FileReader::getMonthHeaderLinesInt(){
  return monthHeaderLinesInt;
}
/** Write property of int monthHeaderLinesInt. */
bool FileReader::setMonthHeaderLinesInt( const int& theNewVal)
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
std::string FileReader::getFileReaderInfo()
{
  std::string myMetadataString = "";



  return myMetadataString;
}
/** Read property of fpos_t headerPos. */
const std::fpos_t* FileReader::getHeaderFPos(){
  return headerFPos;
}
/** Write property of fpos_t headerPos. */
bool FileReader::setHeaderFPos( std::fpos_t& theNewVal){
  try
  {
    headerFPos = &theNewVal;
    return true;
  }
  catch (...)
  {
    return false;
  }

}
/** Read property of fpos_t dataStartFPos. */
const std::fpos_t* FileReader::getDataStartFPos(){
  return dataStartFPos;

}

/** Write property of fpos_t dataStartFPos. */
bool FileReader::setDataStartFPos( std::fpos_t& theNewVal){
  try
  {
    dataStartFPos = &theNewVal;

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
    fsetpos (filePointer, headerFPos);
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

    fsetpos (filePointer,  dataStartFPos);
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
 *    dataStartFPos there. This method will need to be called explicitly by the client app so that when multiple
 *   copies of the same file are being opened, we dont need to do the same thing each time.*/
std::vector <fpos_t>* FileReader::getBlockMarkers()
{
  //
  // Set up some vars
  //

  char myLineChar[65535];  //temporary holder for fgetted lines
  fpos_t myFilePos; //store the current position in the file
  long myMatrixRowsLong;
  long myFileSizeLong;
  long myFilePosLong;  //where we are in the currently open file
  myFilePosLong=0;
  //clear the vector
  dataBlockMarkersVector.clear();
  //if the datafile is a an arc/info grid file, there is only one data block
  if (fileType==ARCINFO_GRID || fileType==CRES)
  {
    for (int i=1; i <= headerLinesInt; i++)
    {
      //read an impossibly long line - fgets will stop if it hits a newline
      fgets (myLineChar, 65535, filePointer);
      if (debugModeFlag) cout << myLineChar ;
    }
    fgetpos(filePointer, &myFilePos);
    dataStartFPos=&myFilePos;   
    dataBlockMarkersVector.push_back(myFilePos);       //was *dataStartFPos
    return &dataBlockMarkersVector;
  }
  //find out how long the file is
  fseek(filePointer,0,SEEK_END);
  myFileSizeLong = ftell(filePointer);
  if (debugModeFlag) printf ("FileReader::getBlockMarkers - file size is %i .\n",myFileSizeLong);
  if (debugModeFlag) printf ("FileReader::getBlockMarkers block xDimLong is %i, block yDimLong is %i.\n", 
          xDimLong, yDimLong);
  rewind(filePointer);
  //
  // Start parsing the file
  //

  if (debugModeFlag) cout << "FileReader::getBlockMarkers() - moving to the start of the file" << endl;
  fseek( filePointer,0, SEEK_SET);
  if (debugModeFlag) cout << "FileReader::getBlockMarkers() - skipping " << headerLinesInt << " file header line(s)" << endl;
  for (int i=1; i <= headerLinesInt; i++)
  {
    //read an impossibly long line - fgets will stop if it hits a newline
    fgets (myLineChar, 65535, filePointer);
    if (debugModeFlag) cout << myLineChar ;
  }

  //Calculate number of rows in a month (depends on FileType)
  myMatrixRowsLong = ((xDimLong * yDimLong) / columnsPerRowLong);
  if (debugModeFlag) cout << "FileReader::getBlockMarkers() - looping through rest of file with month header size of " << monthHeaderLinesInt << " lines and data block size of " << myMatrixRowsLong << " lines." << endl;    
  //loop through the rest of the file getting the start pos for each datablock
  do 
  {
    //if (debugModeFlag) cout << "FileReader::getBlockMarkers()  getting header block " << endl;
    //skip the datablock headers
    if (monthHeaderLinesInt > 0)
    {
      for (int i=1; i <= monthHeaderLinesInt; i++)
      {
        //read an impossibly long line - fgets will stop if it hits a newline
        fgets (myLineChar, 65535, filePointer);
        //if (debugModeFlag) cout << myLineChar;
      }
    }
    else
    {
      if (debugModeFlag) cout << "no datablock header" << endl;
    }
    //if (debugModeFlag) cout << "FileReader::getBlockMarkers()  getting data block " << endl;
    //so the file pointer is now at the start of the datablock - add it to the vector
    fgetpos(filePointer, &myFilePos);
    dataBlockMarkersVector.push_back(myFilePos);
    //now skip the data objects for this datablock
    for (int i=1; i <= myMatrixRowsLong; i++)
    {
      //read an impossibly long line - fgets will stop if it hits a newline
      fgets (myLineChar, 65535, filePointer);
      //if (i==1) if (debugModeFlag) cout << myLineChar; //print out the first line of each datablock

    }
    //calculate where we are in the file and update the progress member var

    myFilePosLong = ftell(filePointer);

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

    taskProgressInt = static_cast<int>(( static_cast<float>(myFilePosLong) / myFileSizeLong) * 100 );
    /*
       if (debugModeFlag) cout << "Task Progress: " << ( static_cast<float>(myFilePosLong) / myFileSizeLong) * 100 << endl;
       if (debugModeFlag) cout << "Position " << myFilePosLong << "/" << myFileSizeLong << " ("
       << taskProgressInt << ") : ";

       for (int i=1;i<(taskProgressInt/10);i++)
       {
       if (debugModeFlag) cout << "*";
       }
       if (debugModeFlag) cout << endl;
       */


  }    while (!feof(filePointer)) ;
  /* sorry this is a bit kludgy but the above eof detection overruns by one so we need to
   *  ditch the last vector element. */
  //I have fixed this now
  //if (debugModeFlag) cout << "FileReader::findDataStart() - kludge removing last overrun marker" << endl;
  //dataBlockMarkersVector.pop_back();


  if (debugModeFlag) cout << "FileReader::getBlockMarkers() - read markers for " << dataBlockMarkersVector.size() << " data block(s)" << endl;
  if (debugModeFlag) cout << "FileReader::getBlockMarkers() - moving back to the start of the file" << endl;
  fseek( filePointer,0, SEEK_SET);
  if (debugModeFlag) cout << "FileReader::getBlockMarkers() - finished - returning vector of datablock start positions" << endl;

  return  &dataBlockMarkersVector;
}

bool FileReader::setBlockMarkers(std::vector <fpos_t>* theBlockMarkersVector)
{
  if (!theBlockMarkersVector)
  {
    if (debugModeFlag) cout << "setBlockMarkers(std::vector <fpos_t>* theBlockMarkersVector) received null pointer!" << endl;
    return false;
  }  
  else
  {
    dataBlockMarkersVector.clear();
    vector<std::fpos_t>::iterator myIterator = theBlockMarkersVector->begin();
    while (myIterator != theBlockMarkersVector->end())
    {
      dataBlockMarkersVector.push_back(*myIterator);
      myIterator++;
    }

  }
  //presume all went ok - need to add better error checking later
  return true;
}

/** Read property of int taskProgressInt. */
const int& FileReader::gettaskProgressInt(){
  return taskProgressInt;
}
/** Find out how many blocks (useful in multiblock formats such as SRES) are in this file. */
int FileReader::getNumberOfBlocks(){
  return dataBlockMarkersVector.size();
}
