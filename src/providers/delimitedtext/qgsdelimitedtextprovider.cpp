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

#include "qgsdelimitedtextprovider.h"

#include <cfloat>
#include <iostream>

#include <QtGlobal>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QSettings>
#include <QRegExp>
#include <QUrl>


#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfeatureattribute.h"
#include "qgsfield.h"
#include "qgsmessageoutput.h"
#include "qgsrect.h"
#include "qgsspatialrefsys.h"
#include "qgis.h"
#include "qgslogger.h"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif


static const QString TEXT_PROVIDER_KEY = "delimitedtext";
static const QString TEXT_PROVIDER_DESCRIPTION = "Delimited text data provider";



QgsDelimitedTextProvider::QgsDelimitedTextProvider(QString uri)
    : QgsVectorDataProvider(uri), 
      mShowInvalidLines(true),
      mMinMaxCacheDirty(true)
{
  // Get the file name and mDelimiter out of the uri
  mFileName = uri.left(uri.find("?"));
  // split the string up on & to get the individual parameters
  QStringList parameters = QStringList::split("&", uri.mid(uri.find("?")));
#ifdef QGISDEBUG
  std::cerr << "Parameter count after split on &" << parameters.
    size() << std::endl;
#endif
  // get the individual parameters and assign values
  QStringList temp = parameters.grep("delimiter=");
  mDelimiter = temp.size()? temp[0].mid(temp[0].find("=") + 1) : "";
  temp = parameters.grep("xField=");
  mXField = temp.size()? temp[0].mid(temp[0].find("=") + 1) : "";
  temp = parameters.grep("yField=");
  mYField = temp.size()? temp[0].mid(temp[0].find("=") + 1) : "";
  // Decode the parts of the uri. Good if someone entered '=' as a delimiter, for instance.
  mFileName  = QUrl::fromPercentEncoding(mFileName.toUtf8());
  mDelimiter = QUrl::fromPercentEncoding(mDelimiter.toUtf8());
  mXField    = QUrl::fromPercentEncoding(mXField.toUtf8());
  mYField    = QUrl::fromPercentEncoding(mYField.toUtf8());
#ifdef QGISDEBUG
  std::cerr << "Data source uri is " << (const char *)uri.toLocal8Bit().data() << std::endl;
  std::cerr << "Delimited text file is: " << (const char *)mFileName.toLocal8Bit().data() << std::endl;
  std::cerr << "Delimiter is: " << (const char *)mDelimiter.toLocal8Bit().data() << std::endl;
  std::cerr << "xField is: " << (const char *)mXField.toLocal8Bit().data() << std::endl;
  std::cerr << "yField is: " << (const char *)mYField.toLocal8Bit().data() << std::endl;
#endif
  
  // if delimiter contains some special characters, convert them
  // (we no longer use delimiter as regexp as it introduces problems with special characters)
  mDelimiter.replace("\\t", "\t"); // replace "\t" with a real tabulator
  
  // Set the selection rectangle to null
  mSelectionRectangle = QgsRect();
  // assume the layer is invalid until proven otherwise
  mValid = false;
  if (!mFileName.isEmpty() && !mDelimiter.isEmpty() && !mXField.isEmpty() &&
      !mYField.isEmpty())
  {
    // check to see that the file exists and perform some sanity checks
    if (QFile::exists(mFileName))
    {
      // Open the file and get number of rows, etc. We assume that the
      // file has a header row and process accordingly. Caller should make
      // sure the the delimited file is properly formed.
      mFile = new QFile(mFileName);
      if (mFile->open(QIODevice::ReadOnly))
      {
        mStream = new QTextStream(mFile);
        QString line;
        mNumberFeatures = 0;
        int xyCount = 0;
        int lineNumber = 0;
        // set the initial extent
        mExtent = QgsRect();
        //commented out by Tim for now - setMinimal needs to be merged in from 0.7 branch
        //mExtent->setMinimal(); // This defeats normalization
        bool firstPoint = true;
        while (!mStream->atEnd())
        {
          lineNumber++;
          line = mStream->readLine(); // line of text excluding '\n', default local 8 bit encoding.
          if (mNumberFeatures++ == 0)
          {
            // Get the fields from the header row and store them in the 
            // fields vector
#ifdef QGISDEBUG
            std::
              cerr << "Attempting to split the input line: " << (const char *)line.toLocal8Bit().data() <<
              " using delimiter " << (const char *)mDelimiter.toLocal8Bit().data() << std::endl;
#endif
            QStringList fieldList = QStringList::split(mDelimiter, line, true);
#ifdef QGISDEBUG
            std::cerr << "Split line into " 
                      << fieldList.size() << " parts" << std::endl;
#endif
            // We don't know anything about a text based field other
            // than its name. All fields are assumed to be text
            int fieldPos = 0;
            for (QStringList::Iterator it = fieldList.begin();
                 it != fieldList.end(); ++it)
            {
              QString field = *it;
              if (field.length() > 0)
              {
                attributeFields[fieldPos] = QgsField(*it, "Text");
                fieldPositions[*it] = fieldPos++;
                // check to see if this field matches either the x or y field 
                if (mXField == *it)
                {
#ifdef QGISDEBUG
                  std::cerr << "Found x field " << (const char *)(*it).toLocal8Bit().data() << std::endl;
#endif
                  xyCount++;
                }
                if (mYField == *it)
                {
#ifdef QGISDEBUG
                  std::cerr << "Found y field " << (const char *)(*it).toLocal8Bit().data() << std::endl;
#endif
                  xyCount++;
                }
#ifdef QGISDEBUG
                std::cerr << "Adding field: " << (const char *)(*it).toLocal8Bit().data() << std::endl;
#endif

              }
            }
#ifdef QGISDEBUG
            std::
              cerr << "Field count for the delimited text file is " <<
              attributeFields.size() << std::endl;
#endif
          }
          else
          {
            // examine the x,y and update extents
            //  std::cout << line << std::endl; 
            // split the line on the delimiter
            QStringList parts =
              QStringList::split(mDelimiter, line, true);

	    // Skip malformed lines silently. Report line number with getNextFeature()
	    if ( (parts.size() <= fieldPositions[mXField]) || (parts.size() <= fieldPositions[mYField]) )
	    {
	      continue;
	    }
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

            if (xOk && yOk)
            {
              if (!firstPoint)
              {
                if (x > mExtent.xMax())
                {
                  mExtent.setXmax(x);
                }
                if (x < mExtent.xMin())
                {
                  mExtent.setXmin(x);
                }
                if (y > mExtent.yMax())
                {
                  mExtent.setYmax(y);
                }
                if (y < mExtent.yMin())
                {
                  mExtent.setYmin(y);
                }
              }
              else
              { // Extent for the first point is just the first point
                mExtent.set(x,y,x,y);
                firstPoint = false;
              }
            }
          }
        }
        reset();
        mNumberFeatures--;

        if (xyCount == 2)
        {
#ifdef QGISDEBUG
          std::cerr << "Data store is valid" << std::endl;
          std::cerr << "Number of features " << mNumberFeatures << std::endl;
          std::cerr << "Extents " << (const char *)mExtent.stringRep().toLocal8Bit().data() << std::endl;
#endif
          mValid = true;
        }
        else
        {
          std::
            cerr << "Data store is invalid. Specified x,y fields do not match\n"
            << "those in the database (xyCount=" << xyCount << ")" << std::endl;
        }
      }
#ifdef QGISDEBUG
      std::cerr << "Done checking validity\n";
#endif

      //resize the cache matrix
      mMinMaxCache = new double *[attributeFields.size()];
      for (int i = 0; i < attributeFields.size(); i++)
      {
        mMinMaxCache[i] = new double[2];
      }
    }
    else
      // file does not exist
      std::
        cerr << "Data source " << (const char *)dataSourceUri().toLocal8Bit().data() << " could not be opened" <<
        std::endl;

  }
  else
  {
    // uri is invalid so the layer must be too...
    std::cerr << "Data source is invalid" << std::endl;

  }
}

QgsDelimitedTextProvider::~QgsDelimitedTextProvider()
{
  mFile->close();
  delete mFile;
  delete mStream;
  for (uint i = 0; i < fieldCount(); i++)
  {
    delete mMinMaxCache[i];
  }
  delete[]mMinMaxCache;
}


QString QgsDelimitedTextProvider::storageType() const
{
  return "Delimited text file";
}


/**

  insure double value is properly translated into locate endian-ness

*/
/*
static
double
translateDouble_( double d )
{
    union
    {
        double fpval;
        char   char_val[8];
    } from, to;

    // break double into byte sized chunks
    from.fpval = d;

    to.char_val[7] = from.char_val[0];
    to.char_val[6] = from.char_val[1];
    to.char_val[5] = from.char_val[2];
    to.char_val[4] = from.char_val[3];
    to.char_val[3] = from.char_val[4];
    to.char_val[2] = from.char_val[5];
    to.char_val[1] = from.char_val[6];
    to.char_val[0] = from.char_val[7];

    return to.fpval;

} // translateDouble_
*/

bool
QgsDelimitedTextProvider::getNextFeature_( QgsFeature & feature, 
                                           QgsAttributeList desiredAttributes )
{
    // before we do anything else, assume that there's something wrong with
    // the feature
    feature.setValid( false );
    while ( ! mStream->atEnd() )
    {
      double x = 0.0;
      double y = 0.0;
      QString line = mStream->readLine(); // Default local 8 bit encoding
        // lex the tokens from the current data line
        QStringList tokens = QStringList::split(mDelimiter, line, true);

        bool xOk = false;
        bool yOk = false;

	// Skip indexing malformed lines.
	if ( ! ((tokens.size() <= fieldPositions[mXField]) || (tokens.size() <= fieldPositions[mXField])) )
	{

	  int xFieldPos = fieldPositions[mXField];
	  int yFieldPos = fieldPositions[mYField];

	  x = tokens[xFieldPos].toDouble( &xOk );
	  y = tokens[yFieldPos].toDouble( &yOk );

	}
        if (! (xOk && yOk))
        {
          // Accumulate any lines that weren't ok, to report on them
          // later, and look at the next line in the file, but only if
          // we need to.
	  QgsDebugMsg("Malformed line : " + line);
          if (mShowInvalidLines)
            mInvalidLines << line;

          continue;
        }

        // Give every valid line in the file an id, even if it's not
        // in the current extent or bounds.
        ++mFid;             // increment to next feature ID

        if (! boundsCheck(x,y))
          continue;

        // at this point, one way or another, the current feature values
        // are valid
           feature.setValid( true );

           feature.setFeatureId( mFid );

           QByteArray  buffer;
           QDataStream s( &buffer, QIODevice::WriteOnly ); // open on buffers's data

           switch ( QgsApplication::endian() )
           {
               case QgsApplication::NDR :
                   // we're on a little-endian platform, so tell the data
                   // stream to use that
                   s.setByteOrder( QDataStream::LittleEndian );
                   s << (Q_UINT8)1; // 1 is for little-endian
                   break;
               case QgsApplication::XDR :
                   // don't change byte order since QDataStream is big endian by default
                   s << (Q_UINT8)0; // 0 is for big-endian
                   break;
               default :
                   qDebug( "%s:%d unknown endian", __FILE__, __LINE__ );
                   //delete [] geometry;
                   return false;
           }

           s << (Q_UINT32)QGis::WKBPoint;
           s << x;
           s << y;

           unsigned char* geometry = new unsigned char[buffer.size()];
           memcpy(geometry, buffer.data(), buffer.size());

           feature.setGeometryAndOwnership( geometry, sizeof(wkbPoint) );

           if ( desiredAttributes.size() > 0 )
           {
               for ( QgsAttributeList::const_iterator i = desiredAttributes.begin();
                     i != desiredAttributes.end();
                     ++i )
               {
                   feature.addAttribute(*i, QgsFeatureAttribute(attributeFields[*i].name(), tokens[*i]));
               }
           }
           
           // We have a good line, so return
           return true;

    } // ! textStream EOF

    // End of the file. If there are any lines that couldn't be
    // loaded, display them now.

    if (mShowInvalidLines && !mInvalidLines.isEmpty())
    {
      mShowInvalidLines = false;
      QgsMessageOutput* output = QgsMessageOutput::createMessageOutput();
      output->setTitle(tr("Error"));
      output->setMessage(tr("Note: the following lines were not loaded because Qgis was "
                            "unable to determine values for the x and y coordinates:\n"),
                         QgsMessageOutput::MessageText);
      
      for (int i = 0; i < mInvalidLines.size(); ++i)
        output->appendMessage(mInvalidLines.at(i));
      
      output->showMessage();

      // We no longer need these lines.
      mInvalidLines.empty();
    }

    return false;

} // getNextFeature_( QgsFeature & feature )


bool QgsDelimitedTextProvider::getNextFeature(QgsFeature& feature,
                              bool fetchGeometry,
                              QgsAttributeList fetchAttributes,
                              uint featureQueueSize)
{
  return getNextFeature_(feature, fetchAttributes);
}




/**
 * Select features based on a bounding rectangle. Features can be retrieved
 * with calls to getFirstFeature and getNextFeature.
 * @param mbr QgsRect containing the extent to use in selecting features
 */
void QgsDelimitedTextProvider::select(QgsRect rect, bool useIntersect)
{

  // Setting a spatial filter doesn't make much sense since we have to
  // compare each point against the rectangle.
  // We store the rect and use it in getNextFeature to determine if the
  // feature falls in the selection area
  reset();
  mSelectionRectangle = rect;
}




// Return the extent of the layer
QgsRect QgsDelimitedTextProvider::extent()
{
  return QgsRect(mExtent.xMin(), mExtent.yMin(), mExtent.xMax(),
                     mExtent.yMax());
}

/** 
 * Return the feature type
 */
QGis::WKBTYPE QgsDelimitedTextProvider::geometryType() const
{
  return QGis::WKBPoint;
}

/** 
 * Return the feature type
 */
long QgsDelimitedTextProvider::featureCount() const
{
  return mNumberFeatures;
}

/**
 * Return the number of fields
 */
uint QgsDelimitedTextProvider::fieldCount() const
{
  return attributeFields.size();
}


const QgsFieldMap & QgsDelimitedTextProvider::fields() const
{
  return attributeFields;
}

void QgsDelimitedTextProvider::reset()
{
  // Reset feature id to 0
  mFid = 0;
  // Skip ahead one line since first record is always assumed to be
  // the header record
  mStream->seek(0);
  mStream->readLine();
  //reset any spatial filters
  mSelectionRectangle = mExtent;
}

QString QgsDelimitedTextProvider::minValue(uint position)
{
  if (position >= fieldCount())
  {
    std::
      cerr << "Warning: access requested to invalid position " <<
      "in QgsDelimitedTextProvider::minValue(..)" << std::endl;
  }
  if (mMinMaxCacheDirty)
  {
    fillMinMaxCash();
  }
  return QString::number(mMinMaxCache[position][0], 'f', 2);
}


QString QgsDelimitedTextProvider::maxValue(uint position)
{
  if (position >= fieldCount())
  {
    std::
      cerr << "Warning: access requested to invalid position " <<
      "in QgsDelimitedTextProvider::maxValue(..)" << std::endl;
  }
  if (mMinMaxCacheDirty)
  {
    fillMinMaxCash();
  }
  return QString::number(mMinMaxCache[position][1], 'f', 2);
}

void QgsDelimitedTextProvider::fillMinMaxCash()
{
  for (uint i = 0; i < fieldCount(); i++)
  {
    mMinMaxCache[i][0] = DBL_MAX;
    mMinMaxCache[i][1] = -DBL_MAX;
  }

  QgsFeature f;
  reset();

  getNextFeature(f, true);
  do
  {
    for (uint i = 0; i < fieldCount(); i++)
    {
      double value = (f.attributeMap())[i].fieldValue().toDouble();
      if (value < mMinMaxCache[i][0])
      {
        mMinMaxCache[i][0] = value;
      }
      if (value > mMinMaxCache[i][1])
      {
        mMinMaxCache[i][1] = value;
      }
    }
  }
  while (getNextFeature(f, true));

  mMinMaxCacheDirty = false;
}

bool QgsDelimitedTextProvider::isValid()
{
  return mValid;
}

/** 
 * Check to see if the point is within the selection rectangle
 */
bool QgsDelimitedTextProvider::boundsCheck(double x, double y)
{
  bool inBounds(true);
  if (!mSelectionRectangle.isEmpty())
    inBounds = (((x <= mSelectionRectangle.xMax()) &&
                 (x >= mSelectionRectangle.xMin())) &&
                ((y <= mSelectionRectangle.yMax()) &&
                 (y >= mSelectionRectangle.yMin())));
  // QString hit = inBounds?"true":"false";

  // std::cerr << "Checking if " << x << ", " << y << " is in " << 
  //mSelectionRectangle->stringRep().ascii() << ": " << hit.ascii() << std::endl; 
  return inBounds;
}

int QgsDelimitedTextProvider::capabilities() const
{
    return 0;
}


int *QgsDelimitedTextProvider::getFieldLengths()
{
  // this function parses the entire data file and calculates the
  // max for each

  // Only do this if we haven't done it already (ie. the vector is
  // empty)
  int *lengths = new int[attributeFields.size()];
  // init the lengths to zero
  for (int il = 0; il < attributeFields.size(); il++)
  {
    lengths[il] = 0;
  }
  if (mValid)
  {
    reset();
    // read the line
    QString line;
    while (!mStream->atEnd())
    {
      line = mStream->readLine(); // line of text excluding '\n'
      // split the line
      QStringList parts = QStringList::split(mDelimiter, line, true);
      // iterate over the parts and update the max value
      for (int i = 0; i < parts.size(); i++)
      {
        if (parts[i] != QString::null)
        {
          // std::cerr << "comparing length for " << parts[i] << " against max len of " << lengths[i] << std::endl; 
          if (parts[i].length() > lengths[i])
          {
            lengths[i] = parts[i].length();
          }
        }

      }
    }
  }
  return lengths;
}

void QgsDelimitedTextProvider::setSRS(const QgsSpatialRefSys& theSRS)
{
  // TODO: make provider projection-aware
}
  
QgsSpatialRefSys QgsDelimitedTextProvider::getSRS()
{
  // TODO: make provider projection-aware
  return QgsSpatialRefSys(); // return default SRS
}




QString  QgsDelimitedTextProvider::name() const
{
    return TEXT_PROVIDER_KEY;
} // ::name()



QString  QgsDelimitedTextProvider::description() const
{
    return TEXT_PROVIDER_DESCRIPTION;
} //  QgsDelimitedTextProvider::name()


/**
 * Class factory to return a pointer to a newly created 
 * QgsDelimitedTextProvider object
 */
QGISEXTERN QgsDelimitedTextProvider *classFactory(const QString *uri)
{
  return new QgsDelimitedTextProvider(*uri);
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
    return TEXT_PROVIDER_KEY;
}

/**
 * Required description function 
 */
QGISEXTERN QString description()
{
    return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}
