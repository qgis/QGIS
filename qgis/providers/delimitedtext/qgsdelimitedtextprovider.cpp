/***************************************************************************
  qgsdelimitedtextprovider.cpp -  Data provider for delimted text
  -------------------
begin                : 2004-02-27
copyright            : (C) 2004 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <iostream>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfeature.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsrect.h"
#include "qgsdelimitedtextprovider.h"
#include <cfloat>

  QgsDelimitedTextProvider::QgsDelimitedTextProvider(QString uri)
:mDataSourceUri(uri), mMinMaxCacheDirty(true)
{
  // Get the file name and mDelimiter out of the uri
  mFileName = uri.left(uri.find("?"));
  // split the string up on & to get the individual parameters
  QStringList parameters = QStringList::split("&", uri.mid(uri.find("?")));
#ifdef QGISDEBUG
  std::cerr << "Parameter count after split on &" << parameters.size() << std::endl; 
#endif
  // get the individual parameters and assign values
  QStringList temp = parameters.grep("delimiter=");
  mDelimiter = temp.size() ?temp[0].mid(temp[0].find("=") +1):"";
  temp = parameters.grep("xField=");
  mXField = temp.size() ?temp[0].mid(temp[0].find("=") +1):"";
  temp = parameters.grep("yField=");
  mYField = temp.size() ?temp[0].mid(temp[0].find("=") +1):"";
#ifdef QGISDEBUG
  std::cerr << "Data source uri is " << uri << std::endl;
  std::cerr << "Delimited text file is: " << mFileName << std::endl;
  std::cerr << "Delimiter is: " << mDelimiter << std::endl;
  std::cerr << "xField is: " << mXField << std::endl; 
  std::cerr << "yField is: " << mYField << std::endl; 
#endif
  // Set the selection rectangle to null
  mSelectionRectangle = 0;
  // assume the layer is invalid until proven otherwise
  mValid = false;
  if(!mFileName.isEmpty() && !mDelimiter.isEmpty() && !mXField.isEmpty() && !mYField.isEmpty()){
    // check to see that the file exists and perform some sanity checks
    if(QFile::exists(mFileName)){
      // Open the file and get number of rows, etc. We assume that the
      // file has a header row and process accordingly. Caller should make
      // sure the the delimited file is properly formed.
      mFile = new QFile(mFileName);
      if ( mFile->open( IO_ReadOnly ) ) {
        QTextStream stream( mFile );
        QString line;
        mNumberFeatures = 0;
        int xyCount = 0;
        int lineNumber = 0;
        // set the initial extent
        mExtent = new QgsRect(9999999999999.0,9999999999999.0,-9999999999999.0,-9999999999999.0);
        while ( !stream.atEnd() ) {
          lineNumber++;
          line = stream.readLine(); // line of text excluding '\n'
          if(mNumberFeatures++ == 0){
            // Get the fields from the header row and store them in the 
            // fields vector
#ifdef QGISDEBUG
            std::cerr << "Attempting to split the input line: " << line <<
              " using delimiter " << mDelimiter << std::endl;
#endif
            QStringList fieldList = QStringList::split(mDelimiter, line);
#ifdef QGISDEBUG
            std::cerr << "Split line into " << fieldList.size() << " parts" << std::endl; 
#endif
            // We don't know anything about a text based field other
            // than its name. All fields are assumed to be text
            int fieldPos = 0;
            for ( QStringList::Iterator it = fieldList.begin(); it != fieldList.end(); ++it ) {
              attributeFields.push_back(QgsField(*it, "Text"));
              fieldPositions[*it] = fieldPos++;
              // check to see if this field matches either the x or y field
              if(mXField == *it)
              {
#ifdef QGISDEBUG
                std::cerr << "Found x field " <<  *it << std::endl; 
#endif
                xyCount++;
              }
              if(mYField == *it)
              {
#ifdef QGISDEBUG
                std::cerr << "Found y field " <<  *it << std::endl; 
#endif
                xyCount++;
              }
#ifdef QGISDEBUG
              std::cerr << "Adding field: " << *it << std::endl; 
#endif

            }           
          }else
          {
            // examine the x,y and update extents
            //  std::cout << line << std::endl; 
            // split the line on the delimiter
            QStringList parts = QStringList::split(mDelimiter, line);
            //if(parts.size() == attributeFields.size())
            //{
            //  // we can populate attributes if required
            //  fieldsMatch = true;
            //}else
            //{
            //  fieldsMatch = false;
            //}
            /*
               std::cout << "Record hit line " << lineNumber << ": " <<
               parts[fieldPositions[mXField]] << ", " <<
               parts[fieldPositions[mYField]] << std::endl;
               */
            // Get the x and y values, first checking to make sure they
            // aren't null.
            QString sX = parts[fieldPositions[mXField]];
            QString sY = parts[fieldPositions[mYField]];
            //std::cout << "x ,y " << sX << ", " << sY << std::endl; 
            bool xOk = true;
            bool yOk = true;
            double x = sX.toDouble(&xOk);
            double y = sY.toDouble(&yOk);
            if(xOk && yOk)
            {
              if(x > mExtent->xMax())
              { 
                mExtent->setXmax(x);
              }
              if(x < mExtent->xMin())
              {
                mExtent->setXmin(x);
              }
              if(y > mExtent->yMax())
              {
                mExtent->setYmax(y);
              }
              if(y < mExtent->yMin())
              {
                mExtent->setYmin(y);
              }
            }
          }
        }
        reset();
        mNumberFeatures--;

        if(xyCount == 2)
        {
#ifdef QGISDEBUG
          std::cerr << "Data store is valid" << std::endl; 
          std::cerr << "Number of features " << mNumberFeatures << std::endl; 
          std::cerr << "Extents " << mExtent->stringRep() << std::endl; 
#endif
          mValid = true;
        }else
        {
          std::cerr << "Data store is invalid. Specified x,y fields do not match\n"
            << "those in the database (xyCount=" << xyCount << ")" << std::endl;
        }
      } 

#ifdef QGISDEBUG
      std::cerr << "Done checking validity\n";
#endif

      //resize the cache matrix
      mMinMaxCache=new double*[attributeFields.size()];
      for(int i=0;i<attributeFields.size();i++)
      {
        mMinMaxCache[i]=new double[2];
      }
    }else
      // file does not exist
      std::cerr << "Data source " << mDataSourceUri << " could not be opened" << std::endl; 

  }else
  {
    // uri is invalid so the layer must be too...
    std::cerr << "Data source is invalid" << std::endl;

  }
}

QgsDelimitedTextProvider::~QgsDelimitedTextProvider()
{
  mFile->close();
  delete mFile;
  for(int i=0;i<fieldCount();i++)
  {
    delete mMinMaxCache[i];
  }
  delete[] mMinMaxCache;
}

/**
 * Get the first feature resutling from a select operation
 * @return QgsFeature
 */
QgsFeature *QgsDelimitedTextProvider::getFirstFeature( bool fetchAttributes)
{
  QgsFeature *f = 0;
  if(mValid){
    /*
#ifdef QGISDEBUG
std::cerr << "getting first feature\n";
#endif
ogrLayer->ResetReading();
OGRFeature *feat = ogrLayer->GetNextFeature();
if(feat){
#ifdef QGISDEBUG
std::cerr << "First feature is not null\n";
#endif
}else{
#ifdef QGISDEBUG
std::cerr << "First feature is null\n";
#endif
}
f = new QgsFeature(feat->GetFID());
f->setGeometry(getGeometryPointer(feat));
if(fetchAttributes){
getFeatureAttributes(feat, f);
}
}
*/
  }
return f;
}

/**
 * Get the next feature resulting from a select operation
 * Return 0 if there are no features in the selection set
 * @return QgsFeature
 */
bool QgsDelimitedTextProvider::getNextFeature(QgsFeature &feature, bool fetchAttributes)
{
  // We must manually check each point to see if it is within the
  // selection rectangle
  bool returnValue;

  bool processPoint;
  if(mValid){
    // read the line
    QTextStream stream( mFile );
    QString line;
    if ( !stream.atEnd() ) {
#ifdef QGISDEBUG
      std::cerr << "Stream read" << std::endl; 
#endif
      line = stream.readLine(); // line of text excluding '\n'
      // create the geometry from the x, y fields
      QStringList parts = QStringList::split(mDelimiter, line);
      // Get the x and y values, first checking to make sure they
      // aren't null.
      QString sX = parts[fieldPositions[mXField]];
      QString sY = parts[fieldPositions[mYField]];
      std::cerr << "x ,y " << sX << ", " << sY << std::endl; 
      bool xOk = true;
      bool yOk = true;
      double x = sX.toDouble(&xOk);
      double y = sY.toDouble(&yOk);
      if(xOk && yOk)
      {
        if(mSelectionRectangle == 0)
        {
          // no selection in place
          processPoint = true;
        }else
        {
          // check to see if point is in bounds
          processPoint = boundsCheck(x, y);
        }
        if(processPoint)
        {
          std::cerr << "Processing " << x << ", " << y << std::endl; 
          // create WKBPoint
          wkbPoint *geometry = new wkbPoint;
          geometry->byteOrder = endian();
          geometry->wkbType = 1;
          geometry->x = x;
          geometry->y = y;
          feature.setGeometry((unsigned char *)geometry);
          feature.setValid(true);
          // get the attributes if requested
          if(fetchAttributes){
            for(int fi =0; fi < attributeFields.size(); fi++)
            {
              feature.addAttribute(attributeFields.at(fi).name(), parts[fi]);

            }

            QString sX = parts[fieldPositions[mXField]];
          }
        }else
        {
          feature.setValid(false);
        }

      }
      // Return true since the read was successful. The feature itself
      // may be invalid for various reasons
      returnValue = true;
    }else
    {
#ifdef QGISDEBUG
      std::cerr << "Stream is at end" << std::endl; 
#endif
      // Return false since read of next feature failed
      returnValue = false;
      // Set the feature to invalid
      feature.setValid(false);
    }

  }
#ifdef QGISDEBUG
  QString sReturn = returnValue?"true":"false" ;
  std::cerr << "Returning " << sReturn << " from getNextFeature" << std::endl;
#endif
  return returnValue;
}

/**
 * Get the next feature resulting from a select operation
 * Return 0 if there are no features in the selection set
 * @return QgsFeature
 */
QgsFeature *QgsDelimitedTextProvider::getNextFeature(bool fetchAttributes)
{
  // We must manually check each point to see if it is within the
  // selection rectangle
  QgsFeature *f = 0;
  bool processPoint;
  if(mValid){
    // read the line
    QTextStream stream( mFile );
    QString line;
    if ( !stream.atEnd() ) {
      line = stream.readLine(); // line of text excluding '\n'
      // create the geometry from the x, y fields
      QStringList parts = QStringList::split(mDelimiter, line);
      // Get the x and y values, first checking to make sure they
      // aren't null.
      QString sX = parts[fieldPositions[mXField]];
      QString sY = parts[fieldPositions[mYField]];
      //std::cout << "x ,y " << sX << ", " << sY << std::endl; 
      bool xOk = true;
      bool yOk = true;
      double x = sX.toDouble(&xOk);
      double y = sY.toDouble(&yOk);
      if(xOk && yOk)
      {
        if(mSelectionRectangle == 0)
        {
          // no selection in place
          processPoint = true;
        }else
        {
          // check to see if point is in bounds
          processPoint = boundsCheck(x, y);
          if(!processPoint)
          {
            // we need to continue to read until we get a hit in the
            // selection rectangle or the EOF is reached
            while(!stream.atEnd() && !processPoint)
            {
              line = stream.readLine();

              // create the geometry from the x, y fields
              parts = QStringList::split(mDelimiter, line);
              // Get the x and y values, first checking to make sure they
              // aren't null.
              sX = parts[fieldPositions[mXField]];
              sY = parts[fieldPositions[mYField]];
              //std::cout << "x ,y " << sX << ", " << sY << std::endl; 
              xOk = true;
              yOk = true;
              x = sX.toDouble(&xOk);
              y = sY.toDouble(&yOk);
              if(xOk && yOk)
              {
                processPoint = boundsCheck(x, y);
              }

            }
          }
        }
        if(processPoint)
        {
          //std::cout << "Processing " << x << ", " << y << std::endl; 
          // create WKBPoint
          wkbPoint wkbPt;
          unsigned char * geometry = new unsigned char[sizeof(wkbPt)];
          geometry[0] = endian();
          int type = 1;
          void *ptr = geometry+1;
          memcpy((void*)(geometry +1), &type, 4);
          memcpy((void*)(geometry +5), &x, sizeof(x));
          memcpy((void*)(geometry +13), &y, sizeof(y));
          /*
             geometry->byteOrder = endian();
             geometry->wkbType = 1;
             geometry->x = x;
             geometry->y = y;
             */
          f = new QgsFeature();
          f->setGeometry(geometry);
          //std::cerr << "Setting feature id to " << mFid << std::endl; 
          f->setFeatureId(mFid++);
          // get the attributes if requested
          if(fetchAttributes){
            // add the attributes to the attribute map
            for(int fi =0; fi < attributeFields.size(); fi++)
            {
              f->addAttribute(attributeFields.at(fi).name(), parts[fi]);

            }

          }
        }
      }
    }

  }
  return f;
}

/**
 * Select features based on a bounding rectangle. Features can be retrieved
 * with calls to getFirstFeature and getNextFeature.
 * @param mbr QgsRect containing the extent to use in selecting features
 */
void QgsDelimitedTextProvider::select(QgsRect *rect, bool useIntersect)
{

  // Setting a spatial filter doesn't make much sense since we have to
  // compare each point against the rectangle.
  // We store the rect and use it in getNextFeature to determine if the
  // feature falls in the selection area
  mSelectionRectangle = new QgsRect((*rect));
  // Select implies an upcoming feature read so we reset the data source
  reset();
  // Reset the feature id to 0
  mFid = 0;

}

/**
 * Set the data source specification. This may be a path or database
 * connection string
 * @uri data source specification
 */
void QgsDelimitedTextProvider::setDataSourceUri(QString uri)
{
  mDataSourceUri = uri;
}

/**
 * Get the data source specification. This may be a path or database
 * connection string
 * @return data source specification
 */
QString QgsDelimitedTextProvider::getDataSourceUri()
{
  return mDataSourceUri;
}

/**
 * Identify features within the search radius specified by rect
 * @param rect Bounding rectangle of search radius
 * @return std::vector containing QgsFeature objects that intersect rect
 */
std::vector<QgsFeature>& QgsDelimitedTextProvider::identify(QgsRect * rect)
{
  // reset the data source since we need to be able to read through
  // all features
  reset();
  std::cerr << "Attempting to identify features falling within "
    << rect->stringRep() << std::endl; 
  // select the features
  select(rect);
}

/*
   unsigned char * QgsDelimitedTextProvider::getGeometryPointer(OGRFeature *fet){
   unsigned char *gPtr=0;
// get the wkb representation

//geom->exportToWkb((OGRwkbByteOrder) endian(), gPtr);
return gPtr;

}
*/
int QgsDelimitedTextProvider::endian()
{
  char *chkEndian = new char[4];
  memset(chkEndian, '\0', 4);
  chkEndian[0] = 0xE8;

  int *ce = (int *) chkEndian;
  int retVal;
  if (232 == *ce)
    retVal = NDR;
  else
    retVal = XDR;
  delete[]chkEndian;
  return retVal;
}

// Return the extent of the layer
QgsRect *QgsDelimitedTextProvider::extent()
{
  return new QgsRect(mExtent->xMin(), mExtent->yMin(), mExtent->xMax(), mExtent->yMax());
}

/** 
 * Return the feature type
 */
int QgsDelimitedTextProvider::geometryType(){
  return 1; // WKBPoint
}
/** 
 * Return the feature type
 */
long QgsDelimitedTextProvider::featureCount(){
  return mNumberFeatures;
}

/**
 * Return the number of fields
 */
int QgsDelimitedTextProvider::fieldCount(){
  return attributeFields.size();
}
/**
 * Fetch attributes for a selected feature
 */
void QgsDelimitedTextProvider::getFeatureAttributes(int key, QgsFeature *f){
  //for (int i = 0; i < ogrFet->GetFieldCount(); i++) {

  //  // add the feature attributes to the tree
  //  OGRFieldDefn *fldDef = ogrFet->GetFieldDefnRef(i);
  //  QString fld = fldDef->GetNameRef();
  //  //    OGRFieldType fldType = fldDef->GetType();
  //  QString val;

  //  val = ogrFet->GetFieldAsString(i);
  //  f->addAttribute(fld, val);
  //}
}

std::vector<QgsField>& QgsDelimitedTextProvider::fields(){
  return attributeFields;
}

void QgsDelimitedTextProvider::reset(){
  // Reset the file pointer to BOF
  mFile->reset();
  // Reset feature id to 0
  mFid = 0;
  // Skip ahead one line since first record is always assumed to be
  // the header record
  QTextStream stream( mFile );
  stream.readLine();
}

QString QgsDelimitedTextProvider::minValue(int position)
{
  if(position>=fieldCount())
  {
    std::cerr << "Warning: access requested to invalid position "
      << "in QgsDelimitedTextProvider::minValue(..)" << std::endl;
  }
  if(mMinMaxCacheDirty)
  {
    fillMinMaxCash();
  }
  return QString::number(mMinMaxCache[position][0],'f',2);
}


QString QgsDelimitedTextProvider::maxValue(int position)
{
  if(position>=fieldCount())
  {
    std::cerr << "Warning: access requested to invalid position "
      << "in QgsDelimitedTextProvider::maxValue(..)" << std::endl;
  }
  if(mMinMaxCacheDirty)
  {
    fillMinMaxCash();
  }
  return QString::number(mMinMaxCache[position][1],'f',2);
}

void QgsDelimitedTextProvider::fillMinMaxCash()
{
  for(int i=0;i<fieldCount();i++)
  {
    mMinMaxCache[i][0]=DBL_MAX;
    mMinMaxCache[i][1]=-DBL_MAX;
  }

  QgsFeature f;
  reset();

  getNextFeature(f, true);
  do
  {
    for(int i=0;i<fieldCount();i++)
    {
      double value=(f.attributeMap())[i].fieldValue().toDouble();
      if(value<mMinMaxCache[i][0])
      {
        mMinMaxCache[i][0]=value;  
      }  
      if(value>mMinMaxCache[i][1])
      {
        mMinMaxCache[i][1]=value;  
      }
    }
  }while(getNextFeature(f, true));

  mMinMaxCacheDirty=false;
}
//TODO - add sanity check for shape file layers, to include cheking to
//       see if the .shp, .dbf, .shx files are all present and the layer
//       actually has features
bool QgsDelimitedTextProvider::isValid(){
  return mValid;
}
/** 
 * Check to see if the point is within the selection rectangle
 */
bool QgsDelimitedTextProvider::boundsCheck(double x, double y)
{
  bool inBounds = (((x < mSelectionRectangle->xMax()) &&
        (x > mSelectionRectangle->xMin())) &&
      ((y < mSelectionRectangle->yMax()) &&
       (y > mSelectionRectangle->yMin())));
  QString hit = inBounds?"true":"false";
  //  std::cerr << "Checking if " << x << ", " << y << " is in " << 
  //mSelectionRectangle->stringRep() << ": " << hit << std::endl; 
  return inBounds;
}
/**
 * Class factory to return a pointer to a newly created 
 * QgsDelimitedTextProvider object
 */
extern "C" QgsDelimitedTextProvider * classFactory(const char *uri)
{
  return new QgsDelimitedTextProvider(uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
extern "C" QString providerKey(){
  return QString("delimitedtext");
}
/**
 * Required description function 
 */
extern "C" QString description(){
  return QString("Delimited text data provider");
} 
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
extern "C" bool isProvider(){
  return true;
}

