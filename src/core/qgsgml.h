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

#include <expat.h>
#include "qgis.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgspoint.h"
#include "qgsrectangle.h"

#include <QPair>
#include <QByteArray>
#include <QDomElement>
#include <QStringList>
#include <QStack>

class QgsRectangle;

/** This class reads data from a WFS server or alternatively from a GML file. It
 * uses the expat XML parser and an event based model to keep performance high.
 * The parsing starts when the first data arrives, it does not wait until the
 * request is finished */
class CORE_EXPORT QgsGml : public QObject
{
    Q_OBJECT
  public:
    QgsGml(
      const QString& typeName,
      const QString& geometryAttribute,
      const QgsFields & fields );

    ~QgsGml();

    /** Does the Http GET request to the wfs server
     *  Supports only UTF-8, UTF-16, ISO-8859-1, ISO-8859-1 XML encodings.
     *  @param uri GML URL
     *  @param wkbType wkbType to retrieve
     *  @param extent retrieved extents
     *  @param userName username for authentication
     *  @param password password for authentication
     *  @return 0 in case of success
     */
    int getFeatures( const QString& uri, QGis::WkbType* wkbType, QgsRectangle* extent = 0, const QString& userName = QString(), const QString& password = QString() );

    /** Read from GML data. Constructor uri param is ignored
     *  Supports only UTF-8, UTF-16, ISO-8859-1, ISO-8859-1 XML encodings.
     */
    int getFeatures( const QByteArray &data, QGis::WkbType* wkbType, QgsRectangle* extent = 0 );

    /** Get parsed features for given type name */
    QMap<QgsFeatureId, QgsFeature* > featuresMap() const { return mFeatures; }

    /** Get feature ids map */
    QMap<QgsFeatureId, QString > idsMap() const { return mIdMap; }

    /** Returns features spatial reference system
      @note Added in QGIS 2.1 */
    QgsCoordinateReferenceSystem crs() const;

  private slots:

    void setFinished();

    /** Takes progress value and total steps and emit signals 'dataReadProgress' and 'totalStepUpdate'*/
    void handleProgressEvent( qint64 progress, qint64 totalSteps );

  signals:
    void dataReadProgress( int progress );
    void totalStepsUpdate( int totalSteps );
    //also emit signal with progress and totalSteps together (this is better for the status message)
    void dataProgressAndSteps( int progress, int totalSteps );

  private:

    enum ParseMode
    {
      none,
      boundingBox,
      feature,  // feature element containing attrs and geo (inside gml:featureMember)
      attribute,
      geometry,
      coordinate,
      posList,
      multiPoint,
      multiLine,
      multiPolygon
    };

    /** XML handler methods*/
    void startElement( const XML_Char* el, const XML_Char** attr );
    void endElement( const XML_Char* el );
    void characters( const XML_Char* chars, int len );
    static void start( void* data, const XML_Char* el, const XML_Char** attr )
    {
      static_cast<QgsGml*>( data )->startElement( el, attr );
    }
    static void end( void* data, const XML_Char* el )
    {
      static_cast<QgsGml*>( data )->endElement( el );
    }
    static void chars( void* data, const XML_Char* chars, int len )
    {
      static_cast<QgsGml*>( data )->characters( chars, len );
    }

    // Set current feature attribute
    void setAttribute( const QString& name, const QString& value );

    //helper routines

    /** Reads attribute srsName="EpsgCrsId:..."
       @param epsgNr result
       @param attr attribute strings
       @return 0 in case of success
      */
    int readEpsgFromAttribute( int& epsgNr, const XML_Char** attr ) const;
    /** Reads attribute as string
       @param attributeName
       @param attr
       @return attribute value or an empty string if no such attribute
      */
    QString readAttribute( const QString& attributeName, const XML_Char** attr ) const;
    /** Creates a rectangle from a coordinate string.
     @return 0 in case of success*/
    int createBBoxFromCoordinateString( QgsRectangle &bb, const QString& coordString ) const;
    /** Creates a set of points from a coordinate string.
       @param points list that will contain the created points
       @param coordString the text containing the coordinates
       @return 0 in case of success
      */
    int pointsFromCoordinateString( QList<QgsPoint>& points, const QString& coordString ) const;

    /** Creates a set of points from a gml:posList or gml:pos coordinate string.
       @param points list that will contain the created points
       @param coordString the text containing the coordinates
       @param dimension number of dimensions
       @return 0 in case of success
      */
    int pointsFromPosListString( QList<QgsPoint>& points, const QString& coordString, int dimension ) const;

    int pointsFromString( QList<QgsPoint>& points, const QString& coordString ) const;
    int getPointWKB( unsigned char** wkb, int* size, const QgsPoint& ) const;
    int getLineWKB( unsigned char** wkb, int* size, const QList<QgsPoint>& lineCoordinates ) const;
    int getRingWKB( unsigned char** wkb, int* size, const QList<QgsPoint>& ringCoordinates ) const;
    /** Creates a multiline from the information in mCurrentWKBFragments and
     * mCurrentWKBFragmentSizes. Assign the result. The multiline is in
     * mCurrentWKB and mCurrentWKBSize. The function deletes the memory in
     * mCurrentWKBFragments. Returns 0 in case of success.
     */
    int createMultiLineFromFragments();
    int createMultiPointFromFragments();
    int createPolygonFromFragments();
    int createMultiPolygonFromFragments();
    /** Adds all the integers contained in mCurrentWKBFragmentSizes*/
    int totalWKBFragmentSize() const;

    /** Returns pointer to main window or 0 if it does not exist*/
    QWidget* findMainWindow() const;
    /** This function evaluates the layer bounding box from the features and
     * sets it to mExtent.  Less efficient compared to reading the bbox from
     * the provider, so it is only done if the wfs server does not provider
     * extent information.
     */
    void calculateExtentFromFeatures();

    /** Get safely (if empty) top from mode stack */
    ParseMode modeStackTop() { return mParseModeStack.isEmpty() ? none : mParseModeStack.top(); }

    /** Safely (if empty) pop from mode stack */
    ParseMode modeStackPop() { return mParseModeStack.isEmpty() ? none : mParseModeStack.pop(); }

    QString mTypeName;
    QString mUri;
    //results are members such that handler routines are able to manipulate them
    /** Bounding box of the layer*/
    QgsRectangle mExtent;
    /** The features of the layer, map of feature maps for each feature type*/
    //QMap<QgsFeatureId, QgsFeature* > &mFeatures;
    QMap<QgsFeatureId, QgsFeature* > mFeatures;
    //QMap<QString, QMap<QgsFeatureId, QgsFeature* > > mFeatures;

    /** Stores the relation between provider ids and WFS server ids*/
    //QMap<QgsFeatureId, QString > &mIdMap;
    QMap<QgsFeatureId, QString > mIdMap;
    //QMap<QString, QMap<QgsFeatureId, QString > > mIdMap;
    /** Name of geometry attribute*/
    QString mGeometryAttribute;
    //const QMap<QString, QPair<int, QgsField> > &mThematicAttributes;
    QMap<QString, QPair<int, QgsField> > mThematicAttributes;
    QGis::WkbType* mWkbType;
    /** True if the request is finished*/
    bool mFinished;
    /** Keep track about the most important nested elements*/
    QStack<ParseMode> mParseModeStack;
    /** This contains the character data if an important element has been encountered*/
    QString mStringCash;
    QgsFeature* mCurrentFeature;
    QVector<QVariant> mCurrentAttributes; //attributes of current feature
    QString mCurrentFeatureId;
    int mFeatureCount;
    /** The total WKB for a feature*/
    unsigned char* mCurrentWKB;
    /** The total WKB size for a feature*/
    int mCurrentWKBSize;
    QgsRectangle mCurrentExtent;
    /** WKB intermediate storage during parsing. For points and lines, no
     * intermediate WKB is stored at all. For multipoints and multilines and
     * polygons, only one nested list is used. For multipolygons, both nested lists
     * are used*/
    QList< QList<unsigned char*> > mCurrentWKBFragments;
    /** Similar to mCurrentWKB, but only the size*/
    QList< QList<int> > mCurrentWKBFragmentSizes;
    QString mAttributeName;
    QgsApplication::endian_t mEndian;
    /** Coordinate separator for coordinate strings. Usually "," */
    QString mCoordinateSeparator;
    /** Tuple separator for coordinate strings. Usually " " */
    QString mTupleSeparator;
    /** Number of dimensions in pos or posList */
    int mDimension;
    /** Coordinates mode, coordinate or posList */
    ParseMode mCoorMode;
    /** EPSG of parsed features geometries */
    int mEpsg;
};

#endif
