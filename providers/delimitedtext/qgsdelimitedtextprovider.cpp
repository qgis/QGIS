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

QgsDelimitedTextProvider::QgsDelimitedTextProvider(QString uri):mDataSourceUri(uri), mMinMaxCacheDirty(true)
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
        // set the initial extent
        mExtent = new QgsRect(9999999999999.0,9999999999999.0,-9999999999999.0,-9999999999999.0);
        while ( !stream.atEnd() ) {
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
#ifdef QGISDEBUG
              std::cerr << "Comparing " << mXField << " to " << *it << std::endl; 
#endif
              if(mXField == *it)
              {
                xyCount++;
              }
#ifdef QGISDEBUG
              std::cerr << "Comparing " << mYField << " to " << *it << std::endl; 
#endif
              if(mYField == *it)
              {
                xyCount++;
              }
#ifdef QGISDEBUG
              std::cerr << "Adding field: " << *it << std::endl; 
#endif

            }           
          }else
          {
            // examine the x,y and update extents
            std::cout << line << std::endl; 
            // split the line on the delimiter
            QStringList parts = QStringList::split(mDelimiter, line);
            if(parts.size() == attributeFields.size())
            {
              std::cout << parts[fieldPositions[mXField]] << std::endl;
              std::cout << parts[fieldPositions[mYField]] << std::endl;
              // get the x value
              double x = parts[fieldPositions[mXField]].toDouble();
              double y = parts[fieldPositions[mYField]].toDouble();
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
QgsFeature *QgsDelimitedTextProvider::getFirstFeature(bool fetchAttributes)
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
 * Get the next feature resutling from a select operation
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
      if(parts.size() == attributeFields.size()){
        double x = parts[fieldPositions[mXField]].toDouble();
        double y = parts[fieldPositions[mYField]].toDouble();
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

          // create WKBPoint
          wkbPoint *geometry = new wkbPoint;
          geometry->byteOrder = endian();
          geometry->wkbType = 1;
          geometry->x = x;
          geometry->y = y;
          f->setGeometry((unsigned char *)geometry);
          // get the attributes if requested
          if(fetchAttributes){
          }
        }
      }
    }

    /*

    // get the wkb representation
    unsigned char *feature = new unsigned char[geom->WkbSize()];
    geom->exportToWkb((OGRwkbByteOrder) endian(), feature);
    f = new QgsFeature(fet->GetFID());
    f->setGeometry(feature);
    if(fetchAttributes){
    getFeatureAttributes(fet, f);
    }
    delete fet;
    }else{
#ifdef QGISDEBUG
std::cerr << "Feature is null\n";
#endif
    // probably should reset reading here
    ogrLayer->ResetReading();
    }


    }else{
#ifdef QGISDEBUG    
std::cerr << "Read attempt on an invalid shapefile data source\n";
#endif
}
*/
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
  return mGeomType;
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
  mFile->reset();
}

QString QgsDelimitedTextProvider::minValue(int position)
{
  if(position>=fieldCount())
  {
    std::cerr << "Warning: access requested to invalid position in QgsDelimitedTextProvider::minValue(..)" << std::endl;
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
    std::cerr << "Warning: access requested to invalid position in QgsDelimitedTextProvider::maxValue(..)" << std::endl;
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

  QgsFeature* f=getFirstFeature(true);
  do
  {
    for(int i=0;i<fieldCount();i++)
    {
      double value=(f->attributeMap())[i].fieldValue().toDouble();
      if(value<mMinMaxCache[i][0])
      {
        mMinMaxCache[i][0]=value;  
      }  
      if(value>mMinMaxCache[i][1])
      {
        mMinMaxCache[i][1]=value;  
      }
    }
  }while(f=getNextFeature(true));

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
  return ((x < mSelectionRectangle->xMax()) &&
      (x > mSelectionRectangle->xMin()) &&
      (y < mSelectionRectangle->yMax()) &&
      (y > mSelectionRectangle->yMin()));

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

