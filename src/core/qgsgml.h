/***************************************************************************
    qgsgml.h
    ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGML_H
#define QGSGML_H

#include "qgis_core.h"
#include <expat.h>
#include "qgis.h"
#include "qgsfields.h"
#include "qgsrectangle.h"
#include "qgswkbptr.h"
#include "qgsfeature.h"

#include <QPair>
#include <QByteArray>
#include <QDomElement>
#include <QStringList>
#include <QStack>
#include <QVector>

#include <string>

class QgsCoordinateReferenceSystem;

#ifndef SIP_RUN

/**
 * \ingroup core
 * This class builds features from GML data in a streaming way. The caller must call processData()
 * as soon it has new content from the source. At any point, it can call
 * getAndStealReadyFeatures() to collect the features that have been completely
 * parsed.
 * \note not available in Python bindings
 * \since QGIS 2.16
 */
class CORE_EXPORT QgsGmlStreamingParser
{
  public:

    typedef QPair<QgsFeature *, QString> QgsGmlFeaturePtrGmlIdPair;

    /**
     * \ingroup core
     * Layer properties
    */
    class LayerProperties
    {
      public:
        //! Constructor
        LayerProperties() = default;

        //! Layer name
        QString mName;
        //! Geometry attribute name
        QString mGeometryAttribute;
    };

    //! Axis orientation logic.
    typedef enum
    {
      //! Honour EPSG axis order only if srsName is of the form urn:ogc:def:crs:EPSG: *
      Honour_EPSG_if_urn,
      //! Honour EPSG axis order
      Honour_EPSG,
      //! Ignore EPSG axis order
      Ignore_EPSG,
    } AxisOrientationLogic;

    //! Constructor
    QgsGmlStreamingParser( const QString &typeName,
                           const QString &geometryAttribute,
                           const QgsFields &fields,
                           AxisOrientationLogic axisOrientationLogic = Honour_EPSG_if_urn,
                           bool invertAxisOrientation = false );

    //! Constructor for a join layer, or dealing with renamed fields
    QgsGmlStreamingParser( const QList<LayerProperties> &layerProperties,
                           const QgsFields &fields,
                           const QMap< QString, QPair<QString, QString> > &mapFieldNameToSrcLayerNameFieldName,
                           AxisOrientationLogic axisOrientationLogic = Honour_EPSG_if_urn,
                           bool invertAxisOrientation = false );
    ~QgsGmlStreamingParser();

    //! QgsGmlStreamingParser cannot be copied.
    QgsGmlStreamingParser( const QgsGmlStreamingParser &other ) = delete;
    //! QgsGmlStreamingParser cannot be copied.
    QgsGmlStreamingParser &operator=( const QgsGmlStreamingParser &other ) = delete;

    /**
     * Process a new chunk of data. atEnd must be set to true when this is
        the last chunk of data. */
    bool processData( const QByteArray &data, bool atEnd, QString &errorMsg );

    /**
     * Process a new chunk of data. atEnd must be set to true when this is
        the last chunk of data. */
    bool processData( const QByteArray &data, bool atEnd );

    /**
     * Returns the list of features that have been completely parsed. This
        can be called at any point. This will empty the list maintained internally
        by the parser, so that features already returned will no longer be returned
        by later calls. */
    QVector<QgsGmlFeaturePtrGmlIdPair> getAndStealReadyFeatures();

    //! Returns the EPSG code, or 0 if unknown
    int getEPSGCode() const { return mEpsg; }

    //! Returns the value of the srsName attribute
    QString srsName() const { return mSrsName; }

    //! Returns layer bounding box
    const QgsRectangle &layerExtent() const { return mLayerExtent; }

    //! Returns the geometry type
    QgsWkbTypes::Type wkbType() const { return mWkbType; }

    //! Returns WFS 2.0 "numberMatched" attribute, or -1 if invalid/not found
    int numberMatched() const { return mNumberMatched; }

    //! Returns WFS 2.0 "numberReturned" or WFS 1.1 "numberOfFeatures" attribute, or -1 if invalid/not found
    int numberReturned() const { return mNumberReturned; }

    //! Returns whether the document parser is a OGC exception
    bool isException() const { return mIsException; }

    //! Returns the exception text.
    QString exceptionText() const { return mExceptionText; }

    //! Returns whether a "truncatedResponse" element is found
    bool isTruncatedResponse() const { return mTruncatedResponse; }

  private:

    enum ParseMode
    {
      None,
      BoundingBox,
      Null,
      Envelope,
      LowerCorner,
      UpperCorner,
      Feature,  // feature element containing attrs and geo (inside gml:featureMember)
      Attribute,
      Tuple, // wfs:Tuple of a join layer
      FeatureTuple,
      AttributeTuple,
      Geometry,
      Coordinate,
      PosList,
      MultiPoint,
      MultiLine,
      MultiPolygon,
      ExceptionReport,
      ExceptionText
    };

    //! XML handler methods
    void startElement( const XML_Char *el, const XML_Char **attr );
    void endElement( const XML_Char *el );
    void characters( const XML_Char *chars, int len );
    static void start( void *data, const XML_Char *el, const XML_Char **attr )
    {
      static_cast<QgsGmlStreamingParser *>( data )->startElement( el, attr );
    }
    static void end( void *data, const XML_Char *el )
    {
      static_cast<QgsGmlStreamingParser *>( data )->endElement( el );
    }
    static void chars( void *data, const XML_Char *chars, int len )
    {
      static_cast<QgsGmlStreamingParser *>( data )->characters( chars, len );
    }

    // Set current feature attribute
    void setAttribute( const QString &name, const QString &value );

    //helper routines

    /**
     * Reads attribute srsName="EpsgCrsId:..."
       \param epsgNr result
       \param attr attribute strings
       \returns 0 in case of success
      */
    int readEpsgFromAttribute( int &epsgNr, const XML_Char **attr );

    /**
     * Reads attribute as string
       \param attributeName
       \param attr
       \returns attribute value or an empty string if no such attribute
      */
    QString readAttribute( const QString &attributeName, const XML_Char **attr ) const;
    //! Creates a rectangle from a coordinate string.
    bool createBBoxFromCoordinateString( QgsRectangle &bb, const QString &coordString ) const;

    /**
     * Creates a set of points from a coordinate string.
       \param points list that will contain the created points
       \param coordString the text containing the coordinates
       \returns 0 in case of success
      */
    int pointsFromCoordinateString( QList<QgsPointXY> &points, const QString &coordString ) const;

    /**
     * Creates a set of points from a gml:posList or gml:pos coordinate string.
       \param points list that will contain the created points
       \param coordString the text containing the coordinates
       \param dimension number of dimensions
       \returns 0 in case of success
      */
    int pointsFromPosListString( QList<QgsPointXY> &points, const QString &coordString, int dimension ) const;

    int pointsFromString( QList<QgsPointXY> &points, const QString &coordString ) const;
    int getPointWKB( QgsWkbPtr &wkbPtr, const QgsPointXY & ) const;
    int getLineWKB( QgsWkbPtr &wkbPtr, const QList<QgsPointXY> &lineCoordinates ) const;
    int getRingWKB( QgsWkbPtr &wkbPtr, const QList<QgsPointXY> &ringCoordinates ) const;

    /**
     * Creates a multiline from the information in mCurrentWKBFragments and
     * mCurrentWKBFragmentSizes. Assign the result. The multiline is in
     * mCurrentWKB. The function deletes the memory in
     * mCurrentWKBFragments. Returns 0 in case of success.
     */
    int createMultiLineFromFragments();
    int createMultiPointFromFragments();
    int createPolygonFromFragments();
    int createMultiPolygonFromFragments();
    //! Adds all the integers contained in mCurrentWKBFragmentSizes
    int totalWKBFragmentSize() const;

    //! Gets safely (if empty) top from mode stack
    ParseMode modeStackTop() { return mParseModeStack.isEmpty() ? None : mParseModeStack.top(); }

    //! Safely (if empty) pop from mode stack
    ParseMode modeStackPop() { return mParseModeStack.isEmpty() ? None : mParseModeStack.pop(); }

    //! Expat parser
    XML_Parser mParser;

    //! List of (feature, gml_id) pairs
    QVector<QgsGmlFeaturePtrGmlIdPair> mFeatureList;

    //! Describe the various feature types of a join layer
    QList<LayerProperties> mLayerProperties;
    QMap< QString, LayerProperties > mMapTypeNameToProperties;

    //! Typename without namespace prefix
    QString mTypeName;
    QByteArray mTypeNameBA;
    const char *mTypeNamePtr = nullptr;
    size_t mTypeNameUTF8Len;

    QgsWkbTypes::Type mWkbType;

    //results are members such that handler routines are able to manipulate them

    //! Name of geometry attribute
    QString mGeometryAttribute;
    QByteArray mGeometryAttributeBA;
    const char *mGeometryAttributePtr = nullptr;
    size_t mGeometryAttributeUTF8Len;

    QgsFields mFields;
    QMap<QString, QPair<int, QgsField> > mThematicAttributes;

    bool mIsException;
    QString mExceptionText;
    bool mTruncatedResponse;
    //! Parsing depth
    int mParseDepth;
    int mFeatureTupleDepth;
    QString mCurrentTypename; //! Used to track the current (unprefixed) typename for wfs:Member in join layer
    //! Keep track about the most important nested elements
    QStack<ParseMode> mParseModeStack;
    //! This contains the character data if an important element has been encountered
    QString mStringCash;
    QgsFeature *mCurrentFeature = nullptr;
    QVector<QVariant> mCurrentAttributes; //attributes of current feature
    QString mCurrentFeatureId;
    int mFeatureCount;
    //! The total WKB for a feature
    QgsWkbPtr mCurrentWKB;
    QgsRectangle mCurrentExtent;
    bool mBoundedByNullFound;

    /**
     * WKB intermediate storage during parsing. For points and lines, no
     * intermediate WKB is stored at all. For multipoints and multilines and
     * polygons, only one nested list is used. For multipolygons, both nested lists
     * are used*/
    QList< QList<QgsWkbPtr> > mCurrentWKBFragments;
    QString mAttributeName;
    char mEndian;
    //! Coordinate separator for coordinate strings. Usually ","
    QString mCoordinateSeparator;
    //! Tuple separator for coordinate strings. Usually " "
    QString mTupleSeparator;
    //! Keep track about number of dimensions in pos or posList
    QStack<int> mDimensionStack;
    //! Number of dimensions in pos or posList for the current geometry
    int mDimension;
    //! Coordinates mode, coordinate or posList
    ParseMode mCoorMode;
    //! EPSG of parsed features geometries
    int mEpsg;
    //! Literal srsName attribute
    QString mSrsName;
    //! Layer bounding box
    QgsRectangle mLayerExtent;
    //! GML namespace URI
    QString mGMLNameSpaceURI;
    const char *mGMLNameSpaceURIPtr = nullptr;
    //! Axis orientation logic
    AxisOrientationLogic mAxisOrientationLogic;
    //! Whether to invert axis orientation. This value is immutable, but combined with what is inferred from data and mAxisOrientationLogic, is used to compute mInvertAxisOrientation
    bool mInvertAxisOrientationRequest;
    //! Whether to invert axis orientation: result of mAxisOrientationLogic, mInvertAxisOrientationRequest and what is inferred from data and mAxisOrientationLogic
    bool mInvertAxisOrientation;
    //! WFS 2.0 "numberReturned" or WFS 1.1 "numberOfFeatures" attribute, or -1 if invalid/not found
    int mNumberReturned;
    //! WFS 2.0 "numberMatched" attribute, or -1 if invalid/not found
    int mNumberMatched;
    //! XML blob containing geometry
    std::string mGeometryString;
    //! Whether we found a unhandled geometry element
    bool mFoundUnhandledGeometryElement;
};

#endif

/**
 * \ingroup core
 * This class reads data from a WFS server or alternatively from a GML file. It
 * uses the expat XML parser and an event based model to keep performance high.
 * The parsing starts when the first data arrives, it does not wait until the
 * request is finished */
class CORE_EXPORT QgsGml : public QObject
{
    Q_OBJECT
  public:
    QgsGml(
      const QString &typeName,
      const QString &geometryAttribute,
      const QgsFields &fields );

    /**
     * Does the Http GET request to the wfs server
     *  Supports only UTF-8, UTF-16, ISO-8859-1, ISO-8859-1 XML encodings.
     *  \param uri GML URL
     *  \param wkbType wkbType to retrieve
     *  \param extent retrieved extents
     *  \param userName username for authentication
     *  \param password password for authentication
     *  \param authcfg authentication configuration id
     *  \returns 0 in case of success
     *  \note available in Python as getFeaturesUri
     */
    int getFeatures( const QString &uri,
                     QgsWkbTypes::Type *wkbType,
                     QgsRectangle *extent = nullptr,
                     const QString &userName = QString(),
                     const QString &password = QString(),
                     const QString &authcfg = QString() ) SIP_PYNAME( getFeaturesUri );

    /**
     * Read from GML data. Constructor uri param is ignored
     *  Supports only UTF-8, UTF-16, ISO-8859-1, ISO-8859-1 XML encodings.
     */
    int getFeatures( const QByteArray &data, QgsWkbTypes::Type *wkbType, QgsRectangle *extent = nullptr );

    //! Gets parsed features for given type name
    QMap<QgsFeatureId, QgsFeature * > featuresMap() const { return mFeatures; }

    //! Gets feature ids map
    QMap<QgsFeatureId, QString > idsMap() const { return mIdMap; }

    /**
     * Returns features spatial reference system
      \since QGIS 2.1 */
    QgsCoordinateReferenceSystem crs() const;

  signals:
    void dataReadProgress( int progress );
    void totalStepsUpdate( int totalSteps );
    //! Also emit signal with progress and totalSteps together (this is better for the status message)
    void dataProgressAndSteps( int progress, int totalSteps );

  private slots:

    void setFinished();

    //! Takes progress value and total steps and emit signals 'dataReadProgress' and 'totalStepUpdate'
    void handleProgressEvent( qint64 progress, qint64 totalSteps );

  private:

    /**
     * This function evaluates the layer bounding box from the features and
     * sets it to mExtent.  Less efficient compared to reading the bbox from
     * the provider, so it is only done if the wfs server does not provider
     * extent information.
     */
    void calculateExtentFromFeatures();

    void fillMapsFromParser();

    QgsGmlStreamingParser mParser;

    //! Typename without namespace prefix
    QString mTypeName;

    //! True if the request is finished
    bool mFinished;

    //! The features of the layer, map of feature maps for each feature type
    //QMap<QgsFeatureId, QgsFeature* > &mFeatures;
    QMap<QgsFeatureId, QgsFeature * > mFeatures;
    //QMap<QString, QMap<QgsFeatureId, QgsFeature* > > mFeatures;

    //! Stores the relation between provider ids and WFS server ids
    //QMap<QgsFeatureId, QString > &mIdMap;
    QMap<QgsFeatureId, QString > mIdMap;
    //QMap<QString, QMap<QgsFeatureId, QString > > mIdMap;

    //! Bounding box of the layer
    QgsRectangle mExtent;
};

#endif
