/***************************************************************************
      qgsdelimitedtextprovider.h  -  Data provider for delimited text
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

#ifndef QGSDELIMITEDTEXTPROVIDER_H
#define QGSDELIMITEDTEXTPROVIDER_H

#include <QStringList>
#include <QRegularExpression>

#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdelimitedtextfile.h"
#include "qgsfields.h"

#include "qgsprovidermetadata.h"

class QgsFeature;
class QgsField;
class QgsGeometry;
class QgsPointXY;
class QFile;
class QTextStream;
class QgsFeedback;

class QgsDelimitedTextFeatureIterator;
class QgsExpression;
class QgsSpatialIndex;

/**
 * \class QgsDelimitedTextProvider
 * \brief Data provider for delimited text files.
 *
 * The provider needs to know both the path to the text file and
 * the delimiter to use. Since the means to add a layer is fairly
 * rigid, we must provide this information encoded in a form that
 * the provider can decipher and use.
 *
 * The uri must defines the file path and the parameters used to
 * interpret the contents of the file.
 *
 * Example uri = "/home/foo/delim.txt?delimiter=|"*
 *
 * For detailed information on the uri format see the QGSVectorLayer
 * documentation.  Note that the interpretation of the URI is split
 * between QgsDelimitedTextFile and QgsDelimitedTextProvider.
 *
 */
class QgsDelimitedTextProvider final: public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    static const QString TEXT_PROVIDER_KEY;
    static const QString TEXT_PROVIDER_DESCRIPTION;

    /**
     * Regular expression defining possible prefixes to WKT string,
     * (EWKT srid, Informix SRID)
     */
    static QRegularExpression sWktPrefixRegexp;
    static QRegularExpression sCrdDmsRegexp;

    enum GeomRepresentationType
    {
      GeomNone,
      GeomAsXy,
      GeomAsWkt
    };

    explicit QgsDelimitedTextProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );
    ~QgsDelimitedTextProvider() override;

    /* Implementation of functions from QgsVectorDataProvider */

    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    QgsWkbTypes::Type wkbType() const override;
    long long featureCount() const override;
    QgsFields fields() const override;
    QgsVectorDataProvider::Capabilities capabilities() const override;
    bool createSpatialIndex() override;
    QgsFeatureSource::SpatialIndexPresence hasSpatialIndex() const override;
    QString name() const override;
    QString description() const override;
    QgsRectangle extent() const override;
    bool isValid() const override;
    QgsCoordinateReferenceSystem crs() const override;
    bool setSubsetString( const QString &subset, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override { return true; }
    QString subsetString() const override
    {
      return mSubsetString;
    }
    /* new functions */

    /**
     * Check to see if the point is withn the selection
     * rectangle
     * \param x X value of point
     * \param y Y value of point
     * \returns True if point is within the rectangle
     */
    bool boundsCheck( double x, double y );


    /**
     * Check to see if a geometry overlaps the selection
     * rectangle
     * \param geom geometry to test against bounds
     * \param y Y value of point
     * \returns True if point is within the rectangle
     */
    bool boundsCheck( QgsGeometry *geom );

    /**
     * Try to read field types from CSVT (or equivalent xxxT) file.
     * \param filename The name of the file from which to read the field types
     * \param message  Pointer to a string to receive a status message
     * \returns A list of field type strings, empty if not found or not valid
     */
    static QStringList readCsvtFieldTypes( const QString &filename, QString *message = nullptr );

    static QString providerKey();

    /**
     * \brief scanFile scans the file to determine field types and other information about the data
     * \param buildIndexes build spatial indexes
     * \param forceFullScan force a full scan even if the  read flag SkipFullScan is set (when this flag is set the scan will exit after the third record to avoid false boolean detection).
     * \param feedback optional feedback to report scan progress and cancel.
     */
    void scanFile( bool buildIndexes, bool forceFullScan = false, QgsFeedback *feedback = nullptr );

  private slots:

    void onFileUpdated();

  private:


    //some of these methods const, as they need to be called from const methods such as extent()
    void rescanFile() const;
    void resetCachedSubset() const;
    void resetIndexes() const;
    void clearInvalidLines() const;
    void recordInvalidLine( const QString &message );
    void reportErrors( const QStringList &messages = QStringList(), bool showDialog = false ) const;
    static bool recordIsEmpty( QStringList &record );
    void setUriParameter( const QString &parameter, const QString &value );


    static QgsGeometry geomFromWkt( QString &sWkt, bool wktHasPrefixRegexp );
    static bool pointFromXY( QString &sX, QString &sY, QgsPoint &point, const QString &decimalPoint, bool xyDms );
    static void appendZM( QString &sZ, QString &sM, QgsPoint &point, const QString &decimalPoint );

    QList<QPair<QString, QString>> booleanLiterals() const;

    // mLayerValid defines whether the layer has been loaded as a valid layer
    bool mLayerValid = false;
    // mValid defines whether the layer is currently valid (may differ from
    // mLayerValid if the file has been rewritten)
    mutable bool mValid = false;

    //! Text file
    std::unique_ptr< QgsDelimitedTextFile > mFile;

    // Fields
    GeomRepresentationType mGeomRep = GeomNone;
    mutable QList<int> attributeColumns;
    QgsFields attributeFields;

    int mFieldCount = 0;  // Note: this includes field count for wkt field
    QString mWktFieldName;
    QString mXFieldName;
    QString mYFieldName;
    QString mZFieldName;
    QString mMFieldName;
    bool mDetectTypes = true;

    mutable int mXFieldIndex = -1;
    mutable int mYFieldIndex = -1;
    mutable int mZFieldIndex = -1;
    mutable int mMFieldIndex = -1;
    mutable int mWktFieldIndex = -1;

    // mWktPrefix regexp is used to clean up
    // prefixes sometimes used for WKT (PostGIS EWKT, informix SRID)
    bool mWktHasPrefix = false;

    //! Layer extent
    mutable QgsRectangle mExtent;

    int mGeomType;

    mutable long long mNumberFeatures;
    int mSkipLines;
    QString mDecimalPoint;
    bool mXyDms = false;

    QString mSubsetString;
    mutable QString mCachedSubsetString;
    std::unique_ptr< QgsExpression > mSubsetExpression;
    bool mBuildSubsetIndex = true;
    mutable QList<quintptr> mSubsetIndex;
    mutable bool mUseSubsetIndex = false;
    mutable bool mCachedUseSubsetIndex;

    //! Storage for any lines in the file that couldn't be loaded
    int mMaxInvalidLines = 50;
    mutable int mNExtraInvalidLines;
    mutable QStringList mInvalidLines;
    //! Only want to show the invalid lines once to the user
    bool mShowInvalidLines = true;

    //! Record file updates, flags rescan required
    mutable bool mRescanRequired = false;

    // Coordinate reference system
    QgsCoordinateReferenceSystem mCrs;

    QgsWkbTypes::Type mWkbType = QgsWkbTypes::NoGeometry;
    QgsWkbTypes::GeometryType mGeometryType = QgsWkbTypes::UnknownGeometry;

    // Spatial index
    bool mBuildSpatialIndex = false;
    mutable bool mUseSpatialIndex;
    mutable bool mCachedUseSpatialIndex;
    mutable std::unique_ptr< QgsSpatialIndex > mSpatialIndex;

    // Store user-defined column types (i.e. types that are not automatically determined)
    QgsStringMap mUserDefinedFieldTypes;

    QPair<QString, QString> mUserDefinedBooleanLiterals;
    QMap<int, QPair<QString, QString>> mFieldBooleanLiterals;

    friend class QgsDelimitedTextFeatureIterator;
    friend class QgsDelimitedTextFeatureSource;
};

class QgsDelimitedTextProviderMetadata final: public QgsProviderMetadata
{
  public:
    QgsDelimitedTextProviderMetadata();
    QgsDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    ProviderCapabilities providerCapabilities() const override;
};

#endif
