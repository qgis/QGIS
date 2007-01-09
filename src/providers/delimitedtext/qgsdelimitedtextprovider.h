/***************************************************************************
      qgsdelimitedtextprovider.h  -  Data provider for delimted text
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

#include "qgsvectordataprovider.h"


#include <map>

#include <QStringList>

class QgsFeature;
class QgsField;
class QFile;
class QTextStream;


/**
\class QgsDelimitedTextProvider
\brief Data provider for delimited text files.
*
* The provider needs to know both the path to the text file and
* the delimiter to use. Since the means to add a layer is farily
* rigid, we must provide this information encoded in a form that
* the provider can decipher and use.
* The uri must contain the path and delimiter in this format:
* /full/path/too/delimited.txt?delimiter=<delimiter>
*
* Example uri = "/home/foo/delim.txt?delimiter=|"
*/
class QgsDelimitedTextProvider : public QgsVectorDataProvider
{
public:

  QgsDelimitedTextProvider(QString uri = QString());

  virtual ~QgsDelimitedTextProvider();

  /* Implementation of functions from QgsVectorDataProvider */
  
  /**
   * Returns the permanent storage type for this layer as a friendly name.
   */
  virtual QString storageType() const;

  /**
   * Select features based on a bounding rectangle. Features can be retrieved 
   * with calls to getFirstFeature and getNextFeature.
   * @param mbr QgsRect containing the extent to use in selecting features
   */
  virtual void select(QgsRect mbr, bool useIntersect = false);

  /**
   * Get the next feature resulting from a select operation.
   * @param feature feature which will receive data from the provider
   * @param fetchGeoemtry if true, geometry will be fetched from the provider
   * @param fetchAttributes a list containing the indexes of the attribute fields to copy
   * @param featureQueueSize  a hint to the provider as to how many features are likely to be retrieved in a batch
   * @return true when there was a feature to fetch, false when end was hit
   */
  virtual bool getNextFeature(QgsFeature& feature,
                              bool fetchGeometry = true,
                              QgsAttributeList fetchAttributes = QgsAttributeList(),
                              uint featureQueueSize = 1);


  /**
   * Get feature type.
   * @return int representing the feature type
   */
  virtual QGis::WKBTYPE geometryType() const;

  /**
   * Number of features in the layer
   * @return long containing number of features
   */
  virtual long featureCount() const;

  /**
   * Number of attribute fields for a feature in the layer
   */
  virtual uint fieldCount() const;
    
  /**
   * Return a map of indexes with field names for this layer
   * @return map of fields
   */
  virtual const QgsFieldMap & fields() const;

  /** Reset the layer (ie move the file pointer to the head
   *  of the file.
   */
  virtual void reset();

  /**
   * Returns the minimum value of an attribute
   * @param position the number of the attribute
   */
  virtual QString minValue(uint position);

  /**
   * Returns the maximum value of an attribute
   * @param position the number of the attribute
   */
  virtual QString maxValue(uint position);

  /** Returns a bitmask containing the supported capabilities
      Note, some capabilities may change depending on whether
      a spatial filter is active on this provider, so it may
      be prudent to check this value per intended operation.
   */
  virtual int capabilities() const;

  
  /* Implementation of functions from QgsDataProvider */
  
  /** return a provider name

      Essentially just returns the provider key.  Should be used to build file
      dialogs so that providers can be shown with their supported types. Thus
      if more than one provider supports a given format, the user is able to
      select a specific provider to open that file.
    
      @note
    
      Instead of being pure virtual, might be better to generalize this
      behavior and presume that none of the sub-classes are going to do
      anything strange with regards to their name or description?
   */
  QString name() const;

  /** return description

      Return a terse string describing what the provider is.
    
      @note
    
      Instead of being pure virtual, might be better to generalize this
      behavior and presume that none of the sub-classes are going to do
      anything strange with regards to their name or description?
   */
  QString description() const;

  /**
   * Return the extent for this data layer
   */
  virtual QgsRect extent();

  /**
   * Returns true if this is a valid delimited file
   */
  bool isValid();

  virtual void setSRS(const QgsSpatialRefSys& theSRS);
  
  virtual QgsSpatialRefSys getSRS();

  /* new functions */
 
  /**
   * Check to see if the point is withn the selection
   * rectangle
   * @param x X value of point
   * @param y Y value of point
   * @return True if point is within the rectangle
  */
  bool boundsCheck(double x, double y);

 



private:

  /** get the next feature, if any

    mFile should be open with the file pointer at the record of the next
    feature, or EOF.  The feature found on the current line is parsed.

    @param feature the feature object to be populated with next feature
    
    @param desiredAttributes attributes fields to be collected for the feature
                             as denoted by their position

    @return false if unable to get the next feature
  */
  bool getNextFeature_( QgsFeature & feature, QgsAttributeList desiredAttributes);

  void fillMinMaxCash();

  int *getFieldLengths();

  //! Fields
  QgsFieldMap attributeFields;

  //! Map to store field position by name
  std::map < QString, int >fieldPositions;

  QString mFileName;
  QString mDelimiter;
  QString mXField;
  QString mYField;

  //! Layer extent
  QgsRect mExtent;

  //! Current selection rectangle

  QgsRect mSelectionRectangle;

  //! Text file
  QFile *mFile;

  QTextStream *mStream;

  bool mValid;

  int mGeomType;

  long mNumberFeatures;

  //! Storage for any lines in the file that couldn't be loaded
  QStringList mInvalidLines;
  //! Only want to show the invalid lines once to the user
  bool mShowInvalidLines;

  //! Feature id
  long mFid;

  /**Flag indicating, if the minmaxcache should be renewed (true) or not (false)*/
  bool mMinMaxCacheDirty;

  /**Matrix storing the minimum and maximum values*/
  double **mMinMaxCache;

  /**Fills the cash and sets minmaxcachedirty to false*/
  void mFillMinMaxCash();

  struct wkbPoint
  {
    unsigned char byteOrder;
    Q_UINT32 wkbType;
    double x;
    double y;
  };
  wkbPoint mWKBpt;

};
