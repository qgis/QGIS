#include "filereader.h"
#include <iostream>
#include <cassert>  //assert debugging
#include <qstring.h>
using namespace std;

//this struct is required for stl transform operations
//you used to not need it but now you do as described at:
//http://linux-rep.fnal.gov/software/gcc/onlinedocs/libstdc++/22_locale/howto.html#7
//and
//
struct ToUpper
{
  ToUpper (std::locale const& l) : loc(l) {;}
  char operator() (char c)  { return std::toupper(c,loc); }
    private:
  std::locale const& loc;
};


FileReader::FileReader(std::string theFileNameString, const FileTypeEnum& theFileTypeEnum)
{

  debugModeFlag=false;
  loadBlocksInMemoryFlag=true; //change this to false to make program pass-through data rather than load to an array
  if (debugModeFlag) cout << "FileReader::FileReader(std::string theFileNameString)" << endl;
  //set the class member
  fileNameString=theFileNameString;
  //open the file - gdal files are only opened when their filetype is set!
  if (theFileTypeEnum != GDAL)
  {
    openFile(&theFileNameString);
    loadBlocksInMemoryFlag=true; //force blocks to mem
  }
  //set the file type
  setFileType(theFileTypeEnum);
  //set progress to zero
  taskProgressInt=0;

}  

/** Destructor */
FileReader::~FileReader()
{
  /*clean up all class vars */


  delete filePointer;
  delete headerFPos;
  delete dataStartFPos;


}

/**
  This method will open a given file. The file pointer will be moved to the first matrix element, and any header info will be skipped.
  @param open. The FileName (including full path) to open.
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
    cout <<  " *********     FileReader::moveFirst()  ---- this function is not implemented !!!!!!!!!!! *********! " << endl;
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
    if (!loadBlocksInMemoryFlag)
    {
      fscanf (filePointer, "%f", &myElementFloat);
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
  //we do this after incrementing positio markers to make sure we get the correct element
  if (loadBlocksInMemoryFlag)
  {
    myElementFloat=static_cast<float>(gsl_matrix_get (currentGslMatrix, currentColLong-1,currentRowLong-1 ));

  }
  //print out the last entries for debuggging
  if (currentElementLong > 0)
  {
    if (debugModeFlag) cout << "FileReader::getElement() retrieved value : " << myElementFloat << " for element no " << currentElementLong << endl;
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
      noDataFloat=-99999;
    }
    //Set IPCC observed member variables
    else if (fileType == IPCC_OBSERVED)
    {
      xDimLong = 720;
      yDimLong = 360;
      columnsPerRowLong = 720;
      headerLinesInt = 2;
      monthHeaderLinesInt = 0;
      noDataFloat=-99999;
    }
    //Set ECHAM4 member variables
    else if (fileType == ECHAM4)
    {
      xDimLong = 128;
      yDimLong = 64;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
      noDataFloat=-99999;
    }
    //Set CCCma member variables
    else if (fileType == CGCM2)
    {
      xDimLong = 96;
      yDimLong = 48;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
      noDataFloat=-99999;
    }

    //Set CSIRO_Mk2 member variables
    else if (fileType == CSIRO_MK2)
    {
      xDimLong = 64;
      yDimLong = 56;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
      noDataFloat=-99999;
    }
    //Set NCAR member variables
    else if (fileType == NCAR_CSM_PCM)
    {
      xDimLong = 128;
      yDimLong = 64;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
      noDataFloat=-99999;
    }
    //Set GFDL member variables
    else if (fileType == GFDL_R30)
    {
      xDimLong = 96;
      yDimLong = 80;
      columnsPerRowLong = 6;

      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
      noDataFloat=-99999;
    }
    //Set CCSRC member variables
    else if (fileType == CCSR_AGCM_OGCM)
    {
      xDimLong = 64;
      yDimLong = 32;
      columnsPerRowLong = 6;
      headerLinesInt = 0;
      monthHeaderLinesInt = 1;
      noDataFloat=-99999;
    }

    else if (fileType == VALDES)
    {
      xDimLong = 360;
      yDimLong = 180;
      columnsPerRowLong = 360;
      headerLinesInt = 0;
      monthHeaderLinesInt = 0;		
      noDataFloat=-99999;
    }
    else if (fileType == GDAL)  //this will open the file too
    {
      //we need to interrogate the file to find out its metadata
      // xDimLong = 360;
      // yDimLong = 180;
      // columnsPerRowLong = 360;
      // headerLinesInt = 0;
      //  monthHeaderLinesInt = 0;		
      //  noDataFloat=-99999;
      currentColLong=1;
      currentRowLong=1;
      currentElementLong=0;    
      GDALAllRegister();
      gdalDataset = (GDALDataset *) GDALOpen( fileNameString.c_str(), GA_ReadOnly );
      //look at the first band to see how big the bands are
      GDALRasterBand  *gdalBand = gdalDataset->GetRasterBand( 1 );
      xDimLong = (long)gdalBand->GetXSize();
      yDimLong = (long)gdalBand->GetYSize();	
      //do not use delete on the gdal band - gdalDataset takes care of this when it is deleted!

      

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
         NODATA_value  -999

*/

      if (debugModeFlag) cout << "FileReader::setFileType() - setting file type to ARCINFO_GRID / CRES" << endl;
      headerLinesInt = 6;
      monthHeaderLinesInt = 0;

      //
      //
      //we really need some decent checking here to make sure the file is open
      // the next line does not sem to do the job properly!
      //
      //

      assert(filePointer);
      //Just testing remove this later! vvvvvvvvv
      //fseek(filePointer,0,SEEK_END);
      //long myFileSizeLong = ftell(filePointer);
      //rewind(filePointer);
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

        //create an instance of the upped case struct as described at
        //http://linux-rep.fnal.gov/software/gcc/onlinedocs/libstdc++/22_locale/howto.html#7
        ToUpper      myUpperCaseStruct   ( std::locale("C") );

        transform (myString.begin(),myString.end(), myString.begin(), myUpperCaseStruct);   //make sure all keys are in upper case!

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
      noDataFloat = static_cast<long>(myHeaderMap["NODATA_VALUE"]);     //note implicit cast from float to long int
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
/** Read property of char *FileName. */
const std::string FileReader::getFileName(){
  return fileNameString;
}
/** Write property of char *FileName. */
bool FileReader::setFileName( std::string theNewVal){
  try
  {
    fileNameString = theNewVal;
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
/** Read property of int currentBlockInt. */
const int& FileReader::getCurrentBlock(){
  return currentBlockInt;
}
/** Write property of int currentBlockInt. */
bool FileReader::setCurrentBlock( const int& theNewVal){
  //special behaviour for GDAL datasets - forced loading into mem and a separate subroutine to populate the gsl_matrix
  if (fileType==GDAL)
  {
    currentBlockInt = theNewVal;
    return loadGdalLayerAsMatrix(theNewVal);    
  }
  //check if we are loading matrices into memory...
  else if (loadBlocksInMemoryFlag)
  {
    currentBlockInt = theNewVal;
    return populateGslMatrix();
  }
  else //otherwise we are reading element by element from disk so no need to set up atrray
  {
    if (static_cast<int>(dataBlockMarkersVector.size()) < theNewVal)
    {
      cout << "FileReader::setCurrentBlock - error block out of range!" << endl ;
      return false;
    }
    try
    {
      currentBlockInt = theNewVal;
      moveToDataStart();
      return true;
    }
    catch (...)
    {
      return false;
    }
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
const int& FileReader::getBlockHeaderLinesInt(){
  return monthHeaderLinesInt;
}
/** Write property of int monthHeaderLinesInt. */
bool FileReader::setBlockHeaderLinesInt( const int& theNewVal)
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
    //only manipulate the fileointer directly if this is a non gdal dataset
    if (fileType!=GDAL)
    {
      //get the fpos_t of the month
      fpos_t myFPos = dataBlockMarkersVector[currentBlockInt-1];
      setDataStartFPos(myFPos);
      fsetpos (filePointer,  dataStartFPos);
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

/** Use the header info for a given file type to determine the begining of the data block and position the
 *    dataStartFPos there. This method will need to be called explicitly by the client app so that when multiple
 *   copies of the same file are being opened, we dont need to do the same thing each time.
 @param forceParseFlag Default to false. If possible, block markers will be read from a file. If you wish to force rescanning of the data file for block markers, set this parameter to true. Note that the blockmarker scanning process is cpu and disk intensive, so you should only rescan if you suspect the file contents have changed. If the .bmr blockmarker file does not exist, a full scan will be carried out and the .bmr file generated.*/
std::vector <fpos_t>* FileReader::getBlockMarkers(bool theForceParseFlag)
{
  //the block markers vector does not apply to GDAL bands - we use gdal to access individual bands
  if (fileType==GDAL)
  {
    //clear the vector
    dataBlockMarkersVector.clear();
    //and return an empty vector
    return &dataBlockMarkersVector;
  } 
  else
  {
    //
    // Set up some vars
    //

    char myLineChar[65535];  //temporary holder for fgetted lines
    fpos_t myFilePos; //store the current position in the file
    long myMatrixSizeLong; //number of elements in the matrix
    long myFileSizeLong;
    long myFilePosLong;  //where we are in the currently open file
    myFilePosLong=0;
    //clear the vector
    dataBlockMarkersVector.clear();

    //
    //calculate the name of the file we expect to find the block markers in
    //
    //construct a FileName for this months output. In order to concatenate
    //a string and an integer, we have to use an output string stream...
    ostringstream myOStringStream;
    myOStringStream << getFilePath() << getFileNamePart() << ".bmr";
    //now convert the concatenated stringstream back to a string...
    string myBlockMarkersFileNameString=myOStringStream.str();
    cout << "Block markers FileName: " << myBlockMarkersFileNameString << endl;

    //set up some vars governing caching of block markers
    bool myWriteToFileFlag=false;
    FILE * myBlockMarkersFilePointer;
    //see if the caller actually want to force a parse and not read cached markers
    if (!theForceParseFlag)
    {
      //see if a block markers file exists and is readable
      if ((myBlockMarkersFilePointer=fopen(myBlockMarkersFileNameString.c_str(),"rb"))==NULL) 
      {
        //file is not readable
        cout << "FileReader::getBlockMarkers cannot open the markers cache fle - a new one will be created and a complete file parse carried out! " << endl;      
      }
      else  //we managed to open the blockmarkers file so just read them in from there
      {  
        cout << "FileReader::getBlockMarkers reading markers from cache file " << endl; 
        long myCurrentMarkerLong=0;
        bool myFirstMarkerFlag = true;
        do 
        {
          fscanf (myBlockMarkersFilePointer, "%i", &myCurrentMarkerLong);
          //fpos is a struct so we need to set the appropriate member...
          fpos_t myFpos;
          myFpos.__pos= myCurrentMarkerLong;
          dataBlockMarkersVector.push_back(myFpos);
          if (myFirstMarkerFlag)
          {
            myFirstMarkerFlag=false;
            dataStartFPos=&myFpos ;
          }
        }    while (!feof(myBlockMarkersFilePointer)) ;

        fclose(myBlockMarkersFilePointer);
        return &dataBlockMarkersVector;
      }
    }


    //otherwisecreate a file to store the block markers in
    //try to open in write binary mode
    if ((myBlockMarkersFilePointer=fopen(myBlockMarkersFileNameString.c_str(),"wb"))==NULL) 
    {
      cout << "FileReader::getBlockMarkers cannot open output file the markers will be parsed but not cached! " << endl;
    }
    else
    {
      myWriteToFileFlag=true;
    }
    //if the datafile is a an arc/info grid file, there is only one data block
    if (fileType==ARCINFO_GRID || fileType==CRES)
    {
      rewind(filePointer);
      for (int i=1; i <= headerLinesInt; i++)
      {
        //read an impossibly long line - fgets will stop if it hits a newline
        fgets (myLineChar, 65535, filePointer);
        if (debugModeFlag) cout << myLineChar ;
      }
      fgetpos(filePointer, &myFilePos);
      dataBlockMarkersVector.push_back(myFilePos);
      dataStartFPos=  &myFilePos       ;
      //and write it to the block markers cache file
      if (myWriteToFileFlag)
      {
        int myResultInt =fprintf ( myBlockMarkersFilePointer, "%i  ",myFilePos);  
        fclose(myBlockMarkersFilePointer);
      }


      if (debugModeFlag) cout << "FileReader::getBlockMarkers() - read markers for " << dataBlockMarkersVector.size() << " data block(s)" << endl;
      if (debugModeFlag) cout << "FileReader::getBlockMarkers() - moving back to the start of the file" << endl;

      rewind(filePointer);   
      return &dataBlockMarkersVector;
    }

    //
    //end of single block filetype parsers - now we have the code for multiblock filetype parsers
    //


    //find out how long the file is
    fseek(filePointer,0,SEEK_END);
    myFileSizeLong = ftell(filePointer);
    if (debugModeFlag) printf ("FileReader::getBlockMarkers - file size is %f .\n",static_cast<double>(myFileSizeLong));
    if (debugModeFlag) printf ("FileReader::getBlockMarkers block xDimLong is %f block yDimLong is %f.\n",
            static_cast<double>(xDimLong), static_cast<double>(yDimLong));
    rewind(filePointer);

    //
    // Start parsing the file
    //

    if (debugModeFlag) cout << "FileReader::getBlockMarkers() - moving to the start of the file" << endl;
    fseek( filePointer,0, SEEK_SET);
    if (debugModeFlag) cout << "FileReader::getBlockMarkers() - skipping " << headerLinesInt << " file header line(s)" << endl;
    for (int i=1; i <= headerLinesInt; i++)
    {
      //read an (hopefully) impossibly long line - 
      // fgets will stop if it hits a newline
      fgets (myLineChar, 65535, filePointer);
      if (debugModeFlag)
      {
        cout << "File header: " << myLineChar << endl ;

      }
    }

    //
    //loop through the rest of the file getting the start pos for each datablock
    //

    do 
    {
      //if (debugModeFlag) cout << "FileReader::getBlockMarkers() " <<
      // "getting header block " << endl;
      //skip the datablock headers
      if (monthHeaderLinesInt > 0)
      {
        for (int i=1; i <= monthHeaderLinesInt; i++)
        {
          //read an impossibly long line - fgets will stop if it hits a newline
          fgets (myLineChar, 65535, filePointer);
          //quick hack here
          if (debugModeFlag)
          {
            cout << "\nFound datablock header:: " 
                << myLineChar << "\n" << endl;
          }
        }
      }
      else
      {
        if (debugModeFlag) cout << "no datablock header" << endl;
      }

      //if (debugModeFlag) cout << "FileReader::getBlockMarkers()  " << 
      //	" getting data block " << endl;
      //so the file pointer is now at the start of the datablock - add it to the vector
      fgetpos(filePointer, &myFilePos);
      dataBlockMarkersVector.push_back(myFilePos);
      //and write it to the block markers cache file
      if (myWriteToFileFlag)
      {
        int myResultInt =fprintf ( myBlockMarkersFilePointer, "%i  ",myFilePos);  
      }


      //now skip the data objects for this datablock
      //I used to do this line by line, but this is unreliable - especially
      //if the row/column arrangements changes. So now I am going to read in element
      //by element, which is slower, but hopefully more reliable. I have kept the 
      //deprecated code in a comment block below because I may in the future
      //provide a settable flag which allows both options to be implemented.

      myMatrixSizeLong = (xDimLong * yDimLong);
      float myElementFloat;
      for (int i=0; i <= myMatrixSizeLong; i++)
      {
        //read in the next matrix element as a float
        fscanf (filePointer, "%f", &myElementFloat);

        //just bodge testing - show the first 10 elements from the datablock 
        if (i < 10)
        {
          if (debugModeFlag)
          {
            cout << myElementFloat << ", ";
          }
        }

      }       //end of for i loop


      /* //deprecated version of above code
         myMatrixRowsLong = ((xDimLong * yDimLong) / columnsPerRowLong);

         if (debugModeFlag) cout << "FileReader::getBlockMarkers() - " <<
         "looping through rest of file with month header size of " << 
         monthHeaderLinesInt << " lines and data block size of " << 
         myMatrixRowsLong << " lines." << endl;    
         for (int i=1; i <= myMatrixRowsLong; i++)
         {

      //read an impossibly long line - fgets will stop if it hits a newline

      fgets (myLineChar, 65535, filePointer);
      //if (i==1) if (debugModeFlag) 
      //{
      //	//print out the first line of each datablock
      //	cout << myLineChar; 
      //}

      }
      */

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

    //close the block markers cache file if we are using it
    if (myWriteToFileFlag)
    {
      fclose (myBlockMarkersFilePointer);  
    }

    if (debugModeFlag) cout << "FileReader::getBlockMarkers() - read markers for " << dataBlockMarkersVector.size() << " data block(s)" << endl;
    if (debugModeFlag) cout << "FileReader::getBlockMarkers() - moving back to the start of the file" << endl;

    fseek( filePointer,0, SEEK_SET);
    if (debugModeFlag) cout << "FileReader::getBlockMarkers() - finished - returning vector of datablock start positions" << endl;

    return  &dataBlockMarkersVector;
  }
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
      if (fileType==GDAL)
      {
        //see how many blocks int this dataset
        return  gdalDataset->GetRasterCount();
      }
      else if (  fileType==ARCINFO_GRID || fileType==CRES)
      {
        return 1;
      }
      else
      {
        return dataBlockMarkersVector.size();
      }
    }

/*!
  \fn FileUtil::getFileExtension(string theFileNameString)
  */
const std::string FileReader::getFileExtension()
{
  std::string myExtensionString;
  myExtensionString=fileNameString.substr(fileNameString.rfind('.')+1);
  cout << "File Name: " << fileNameString ;
  cout << " File Extension: " + myExtensionString << endl;
  return myExtensionString;
}


/*!
  \fn FileUtil::getFilePath(string theFileNameString)
  */
const std::string FileReader::getFilePath()
{
  std::string myFilePathString;
  //try to find  unix path separater first (search backwards from end of line)
  myFilePathString=fileNameString.substr(0,fileNameString.rfind('/')+1);
  if (myFilePathString.size()==0)
  {
    //try looking for dos separaters
    myFilePathString=fileNameString.substr(0,fileNameString.rfind('\\')+1);
  }
  cout << "FilePath: " << myFilePathString << endl;
  return myFilePathString;
}


/*!
  \fn FileUtil::getFileNameString()
 * This gets the filename without its basepath or extension
 */
const std::string FileReader::getFileNamePart()
{
  std::string myExtensionString = getFileExtension();
  std::string myBasePathString = getFilePath();
  std::string myFileNamePart = fileNameString.substr(myBasePathString.size(), fileNameString.size()-(myExtensionString.size()+1+myBasePathString.size()));
  cout << "FileNamePart: " << myFileNamePart << endl;
  return myFileNamePart;
}

/** get the noData value used to represent null elements */
float FileReader::getNoDataValue()
{
  return noDataFloat;
}
/**set the noData value used to represent null elements */
bool FileReader::setNoDataValue(float theNoDataFloat)
{
  noDataFloat = theNoDataFloat;
  return true;
}

/*!
  \fn FileReader::calulateMatrixStats
 *  find the largest value in the array - this is inefficient
 *  note this will rewind the matrix position to first element
 * TODO: move this into FileReader class rather, as well as the MatrixStats struct
 */
MatrixStats FileReader::calulateMatrixStats()
{
  /// @todo implement me
  MatrixStats myMatrixStats;

  myMatrixStats.noDataFloat=noDataFloat;
  //go to start of matrix
  //this works when not loading to memory...
  //setCurrentBlock(currentBlockInt);
  moveToDataStart();

  bool myFirstIterationFlag = true;
  myMatrixStats.elementCountInt=0;
  while (!endOfMatrixFlag)
  {
    float myFloat = getElement();
    //only use this element if we have a non null element
    if (myFloat != noDataFloat)
    {                  
      if (myFirstIterationFlag)
      {
        //this is the first iteration so initialise vars
        myFirstIterationFlag=false;
        myMatrixStats.minValFloat=myFloat;
        myMatrixStats.maxValFloat=myFloat;

      }
      else
      {
        //this is done for all subsequent iterations
        if (myFloat < myMatrixStats.minValFloat) 
        {
          myMatrixStats.minValFloat=myFloat;
        }
        if (myFloat > myMatrixStats.maxValFloat)
        {
          myMatrixStats.maxValFloat=myFloat;
        }
        //only increment the running total if it is not a nodata value
        if (myFloat != noDataFloat)
        {
          myMatrixStats.sumFloat += myFloat;
          ++myMatrixStats.elementCountInt;  
        }
      }
    }
  }
  //calculate the range
  myMatrixStats.rangeFloat = myMatrixStats.maxValFloat-myMatrixStats.minValFloat;
  //calculate the mean
  myMatrixStats.meanFloat = myMatrixStats.sumFloat / myMatrixStats.elementCountInt;

  //go back to start of matrix again - needed to calculate stddev
  //this works when not loading to memory...
  //setCurrentBlock(currentBlockInt);
  moveToDataStart();
  //
  myMatrixStats.sumSqrDevFloat=0;
  //Calculate the sum of the squared deviations
  while (!endOfMatrixFlag)
  {
    float myFloat = getElement();
    if (myFloat != noDataFloat)
    {
      myMatrixStats.sumSqrDevFloat += static_cast<float>(pow(myFloat - myMatrixStats.meanFloat,2));
    }
  }

  //divide result by sample size - 1 and get square root to get stdev
  myMatrixStats.stdDevFloat = static_cast<float>(sqrt(myMatrixStats.sumSqrDevFloat /
              (myMatrixStats.elementCountInt - 1)));

  //go back to start of matrix
  //this works when not loading to memory...
  //setCurrentBlock(currentBlockInt);
  moveToDataStart();
  return myMatrixStats;
}

gsl_matrix * FileReader::getGslMatrix()
{
  return currentGslMatrix;
}

bool FileReader::populateGslMatrix()
{
  //go back to start of matrix
  int myBlockMarkersVectorSizeInt = static_cast<int>(dataBlockMarkersVector.size()) ;

  if (myBlockMarkersVectorSizeInt < currentBlockInt)
  {
    cout << "FileReader::getGslMatrix - error block out of range!" << endl ;
    return false;
  }
  try
  {
    //disable load in memory flag so that getElement will read from disk while we populate matrix
    loadBlocksInMemoryFlag=false; 

    moveToDataStart();

    int myColsInt = static_cast<int>(xDimLong);
    int myRowsInt = static_cast<int>(yDimLong);
    //clear any existing memory allocation for the matrix
    //gsl_matrix_free (currentGslMatrix); //cases segfault
    //reallocate memory for the matrix
    currentGslMatrix = gsl_matrix_alloc (myColsInt, myRowsInt);
    for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
    {
      for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
      {
        float myElementFloat=getElement();
        gsl_matrix_set (currentGslMatrix,myCurrentColInt, myCurrentRowInt,  myElementFloat);
      }
    }

    //print out the matrix
    if (debugModeFlag) 
    {
      for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
      {
        for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
        {
          cout << gsl_matrix_get (currentGslMatrix,myCurrentColInt, myCurrentRowInt) << "-";
        }
        cout << endl;
      }    
    } 
    //go back to start of matrix
    moveToDataStart();
    //re-enable read from memory flag
    loadBlocksInMemoryFlag=true; 
    return true;
  }
  catch (...)
  {
    return false;
  }
}

/* This function will load a GDAL band into the GSL matrix as if it were a block */
bool FileReader::loadGdalLayerAsMatrix(const int theLayerNo)
{

  // a band can have color values that correspond to colors in a palette
  // or it can contain the red, green or blue value for an rgb image
  cout << "loadGdalLayerAsMatrix" << endl;
  GDALRasterBand  *gdalBand = gdalDataset->GetRasterBand( theLayerNo );  
  QString myColorInterpQString = GDALGetColorInterpretationName(gdalBand->GetColorInterpretation());
  //next lines are for testing
  //GDALAllRegister();
  //gdalDataset = (GDALDataset *) GDALOpen( fileNameString.c_str(), GA_ReadOnly );
  //int rXSize = gdalBand->GetXSize();
  //int rYSize = gdalBand->GetYSize();
  //uint *scandata = (uint*) CPLMalloc(sizeof(uint)*rXSize*rYSize);	
  //end of testing lines

  //get the raster dimensions
  cout << "Reading GDAL Band (" << xDimLong << " x " << yDimLong << " - " << myColorInterpQString <<  ") into gsl_matrix" << endl;
  int myColsInt = static_cast<int>(xDimLong);
  int myRowsInt = static_cast<int>(yDimLong);
  //clear any existing memory allocation for the matrix
  //gsl_matrix_free (currentGslMatrix); //causes segfault
  //reallocate memory for the matrix
  
  currentGslMatrix = gsl_matrix_alloc (myColsInt, myRowsInt);
  //allocate a buffer to hold one row of ints
  int myAllocationSizeInt = sizeof(uint)*myColsInt; 
  cout << "Allocating " << myAllocationSizeInt << " bytes for gdal line (" << sizeof(uint) << " x "  << xDimLong  << ")" << endl;
  //allocate enough memory to hold one scanline (row)
  uint * myScanlineAllocInt = (uint*) CPLMalloc(myAllocationSizeInt); 
  for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
  {  
    /*
       eRWFlag - Either GF_Read to read a region of data, or GT_Write to write a region of data.
       nXOff - The pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.
       nYOff - The line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.
       nXSize - The width of the region of the band to be accessed in pixels.
       nYSize - The height of the region of the band to be accessed in lines.
       pData - The buffer into which the data should be read, or from which it should be written. This buffer must contain at least nBufXSize * nBufYSize words of type eBufType. It is organized in left to right, top to bottom pixel order. Spacing is controlled by the nPixelSpace, and nLineSpace parameters.
       nBufXSize - the width of the buffer image into which the desired region is to be read, or from which it is to be written.
       nBufYSize - the height of the buffer image into which the desired region is to be read, or from which it is to be written.
       eBufType - the type of the pixel values in the pData data buffer. The pixel values will automatically be translated to/from the GDALRasterBand data type as needed.
       nPixelSpace - The byte offset from the start of one pixel value in pData to the start of the next pixel value within a scanline. If defaulted (0) the size of the datatype eBufType is used.
       nLineSpace - The byte offset from the start of one scanline in pData to the start of the next. If defaulted the size of the datatype eBufType * nBufXSize is used. 
       */
              
    CPLErr myResult = gdalBand->RasterIO( 
            GF_Read, 0, myCurrentRowInt, xDimLong, 1, myScanlineAllocInt, xDimLong, 1, GDT_UInt32, 0, 0 );
    for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
    {
      //get the nth element from the current row
      float myElementFloat=myScanlineAllocInt[myCurrentColInt];
      //and save it into the gsl matrix
      gsl_matrix_set (currentGslMatrix,myCurrentColInt, myCurrentRowInt,  myElementFloat);
    }
    
  }  
  //free up the memory reserved for storing row data
  CPLFree(myScanlineAllocInt);              
  std::cout << "Band scan completed..." << std::endl;

  //according to the gdal covs you should not call delete on a gdal band, rather GDALClose() then

}

//the next couple of methods carry out neighbourhood stats on gsl matrix
// I am not really happy for them to be in filereader, but am keeping them
//here until I find a better home for them
/** This method will perfrma a smothing function on the matrix */
bool FileReader::smooth(){
  cout << "++++++++++++++++++++++++++++++++" << endl;
  cout << "Performing smoothing function on grid" << endl;
  cout << "++++++++++++++++++++++++++++++++" << endl;
  int myColsInt = static_cast<int>(xDimLong);
  int myRowsInt = static_cast<int>(yDimLong);
  //create a temporary working matrix

  //allocate memory for the matrix
  gsl_matrix * myGslMatrix = gsl_matrix_alloc (myColsInt, myRowsInt);
  for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
  {
    for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
    {
      /*
      //check if we are on the edge of the grid somewhere
      int divisor = 0;
      double tLeft =0;
      double Top=0;
      double tRight=0;
      double Left=0;
      double Kernel=0;
      double Right=0;
      double bLeft=0;
      double Bottom=0;
      double bRight=0;

      if (myRow-1 > 0 && myCol-1 >0)
      {
      tLeft = getCell(myRow-1,myCol-1);
      divisor = divisor +1;
      }
      if (myRow-1 > 0)
      {
      Top = getCell(myRow-1,myCol);
      divisor = divisor +1;
      }
      if (myRow-1 > 0 && myCol < Cols-1)
      {
      tRight = getCell(myRow-1,myCol+1);
      divisor = divisor +1;
      }
      if (myCol>0 )
      {
      Left = getCell(myRow,myCol-1);
      divisor = divisor +1;
      }

      Kernel = getCell(myRow,myCol);
      divisor = divisor +1;
      if (myCol < Cols-1)
      {
      Right = getCell(myRow,myCol+1);
      divisor = divisor +1;
      }
      if (myRow < Rows-1 && myCol > 0)
      {
      bLeft = getCell(myRow+1,myCol-1);
      divisor = divisor +1;
      }
      if (myRow < Rows-1)
      {
      Bottom = getCell(myRow+1,myCol);
      divisor = divisor +1;
      }
      if (myRow < Rows-1 && myCol < Cols-1)
      {
      bRight = getCell(myRow+1,myCol+1);
      divisor = divisor +1;
      }
      double mySum =  (tLeft + Top + tRight + Left + Kernel + Right + bLeft +Bottom + bRight);
      double myMean = ( mySum / divisor)  ;
      myArray[myRow][myCol]=myMean;
      */
    }

  }			
  //now update the grid with the contents of myArray
  /*
     for (int myRow=0;myRow<Rows;myRow++)
     {
     for (int myCol=0;myCol<Cols;myCol++)
     {
     setCell(myRow,myCol,myArray[myRow][myCol]);
     }
     }
     */
   //clear any existing memory allocation for the matrix
  gsl_matrix_free (myGslMatrix);
  delete myGslMatrix;
  return(true);
};

// This will perform a local function on the grid, reclassifying each cell
// within the range 0-1 with .25 increments

bool FileReader::localFunction(){
  cout << "++++++++++++++++++++++++++++++++" << endl;
  cout << "Performing local function on grid" << endl;
  cout << "++++++++++++++++++++++++++++++++" << endl;
  /*
     int myColsInt = static_cast<int>(xDimLong);
     int myRowsInt = static_cast<int>(yDimLong);

     currentGslMatrix; = gsl_matrix_alloc (myColsInt, myRowsInt);
     for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
     {
     for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
     {
     float myFloat = getElement(myCurrentRowInt,myCurrentColInt) ;
     switch(int(myVal))
     {
     case 5 :
     setCell(myRow,myCol,1.0);
     break;
     case 4 :
     setCell(myRow,myCol,0.75);							
     break;
     case 3 :
     setCell(myRow,myCol,0.5);
     break;
     case 2 :
     setCell(myRow,myCol,0.25);
     break;
     case 1 :
     setCell(myRow,myCol,0);
     break;
     }
     }
     }
     */			
  return (1);
} ;

/** This function will generate zones - cells with
  values of > 0 will become 1, all other cells will remain 0 */
bool FileReader::zonalFunction(){

  cout << "++++++++++++++++++++++++++++++++" << endl;
  cout << "Performing zonal function on grid" << endl;
  cout << "++++++++++++++++++++++++++++++++" << endl;
  int myColsInt = static_cast<int>(xDimLong);
  int myRowsInt = static_cast<int>(yDimLong);
  //clear any existing memory allocation for the matrix
  gsl_matrix_free (currentGslMatrix);
  //reallocate memory for the matrix
  currentGslMatrix = gsl_matrix_alloc (myColsInt, myRowsInt);
  for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
  {
    for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
    {
      if ( gsl_matrix_get (currentGslMatrix,myCurrentColInt, myCurrentRowInt)!=0)
      {
        gsl_matrix_set (currentGslMatrix,myCurrentColInt, myCurrentRowInt,1);
      }
    }
  }

}

