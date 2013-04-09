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


#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"

#include <QStringList>

class QgsFeature;
class QgsField;
class QFile;
class QTextStream;

class QgsDelimitedTextFeatureIterator;
class QgsDelimitedTextFile;


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
    Q_OBJECT

public:

    QgsDelimitedTextProvider( QString uri = QString() );

    virtual ~QgsDelimitedTextProvider();

    /* Implementation of functions from QgsVectorDataProvider */

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const;

    /**
     * Number of features in the layer
     * @return long containing number of features
     */
    virtual long featureCount() const;

    /**
     * Return a map of indexes with field names for this layer
     * @return map of fields
     */
    virtual const QgsFields & fields() const;

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
    virtual QgsRectangle extent();

    /**
     * Returns true if this is a valid delimited file
     */
    bool isValid();

    virtual QgsCoordinateReferenceSystem crs();

    /* new functions */

    /**
     * Check to see if the point is withn the selection
     * rectangle
     * @param x X value of point
     * @param y Y value of point
     * @return True if point is within the rectangle
    */
    bool boundsCheck( double x, double y );


    /**
     * Check to see if a geometry overlaps the selection
     * rectangle
     * @param geom geometry to test against bounds
     * @param y Y value of point
     * @return True if point is within the rectangle
    */
    bool boundsCheck( QgsGeometry *geom );

private:
    void handleInvalidLines();
    void resetStream();

    //! Text file
    QgsDelimitedTextFile *mFile;

    // Fields
    QList<int> attributeColumns;
    QgsFields attributeFields;

    int mFieldCount;  // Note: this includes field count for wkt field
    int mXFieldIndex;
    int mYFieldIndex;
    int mWktFieldIndex;

    // Handling of WKT types with .. Z, .. M, and .. ZM geometries (ie
    // Z values and/or measures).  mWktZMRegexp is used to test for and
    // remove the Z or M fields, and mWktCrdRegexp is used to remove the
    // extra coordinate values. mWktPrefix regexp is used to clean up
    // prefixes sometimes used for WKT (postgis EWKT, informix SRID)

    bool mWktHasZM;
    bool mWktHasPrefix;
    QRegExp mWktZMRegexp;
    QRegExp mWktCrdRegexp;
    QRegExp mWktPrefixRegexp;

    //! Layer extent
    QgsRectangle mExtent;

    bool mValid;

    int mGeomType;

    long mNumberFeatures;
    int mSkipLines;
    QString mDecimalPoint;

    //! Storage for any lines in the file that couldn't be loaded
    QStringList mInvalidLines;
    //! Only want to show the invalid lines once to the user
    bool mShowInvalidLines;

    struct wkbPoint
    {
        unsigned char byteOrder;
        quint32 wkbType;
        double x;
        double y;
    };
    wkbPoint mWKBpt;

    // Coordinate reference sytem
    QgsCoordinateReferenceSystem mCrs;

    QGis::WkbType mWkbType;

    friend class QgsDelimitedTextFeatureIterator;
    QgsDelimitedTextFeatureIterator* mActiveIterator;
};
