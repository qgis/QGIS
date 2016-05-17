/***************************************************************************
      qgsclipboard.h  -  QGIS internal clipboard for storage of features
      ------------------------------------------------------------------
    begin                : 20 May, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLIPBOARD_H
#define QGSCLIPBOARD_H

#include <QList>
#include <QMap>
#include <QObject>

#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgscoordinatereferencesystem.h"

/**
  \brief QGIS internal clipboard for features.

  An internal clipboard is required so that features can be retained in
  their original fidelity.

  The internal clipboard makes a copy of features that are presented to it,
  therefore the original objects can safely be destructed independent of
  the lifetime of the internal clipboard.

  As this class matures it should also be able to accept CSV repesentations
  of features in and out of the system clipboard (QClipboard).

*/

class QgsVectorLayer;

/*
 * Constants used to describe copy-paste MIME types
 */
#define QGSCLIPBOARD_STYLE_MIME "application/qgis.style"

class APP_EXPORT QgsClipboard : public QObject
{
    Q_OBJECT
  public:

    //! Available formats for copying features as text
    enum CopyFormat
    {
      AttributesOnly, /*!< Tab delimited text, attributes only */
      AttributesWithWKT, /*!< Tab delimited text, with geometry in WKT format */
      GeoJSON, /*!< GeoJSON FeatureCollection format */
    };

    /**
     * Constructor for the clipboard.
     */
    QgsClipboard();

    //! Destructor
    virtual ~QgsClipboard();

    /**
     *  Place a copy of the selected features from the specified layer on
     *  the internal clipboard, destroying the previous contents.
     */
    void replaceWithCopyOf( QgsVectorLayer *src );

    /**
     *  Place a copy of features on the internal clipboard,
     *  destroying the previous contents.
     */
    void replaceWithCopyOf( QgsFeatureStore & featureStore );

    /**
     *  Returns a copy of features on the internal clipboard.
     */
    QgsFeatureList copyOf( const QgsFields &fields = QgsFields() ) const;

    /**
     *  Clears the internal clipboard.
     */
    void clear();

    /**
     *  Inserts a copy of the feature on the internal clipboard.
     */
    void insert( const QgsFeature& feature );

    /**
     *  Returns true if the internal clipboard is empty, else false.
     */
    bool isEmpty() const;

    /**
     *  Returns a copy of features on the internal clipboard, transformed
     *  from the clipboard CRS to the destCRS.
     */
    QgsFeatureList transformedCopyOf( const QgsCoordinateReferenceSystem& destCRS, const QgsFields &fields = QgsFields() ) const;

    /**
     *  Get the clipboard CRS
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Stores a MimeData together with a text into the system clipboard
     */
    void setData( const QString& mimeType, const QByteArray& data, const QString& text = QString() );

    /**
     * Stores a text into the system clipboard
     */
    void setText( const QString& text );

    /**
     * Proxy to QMimeData::hasFormat
     * Tests whether the system clipboard contains data of a given MIME type
     */
    bool hasFormat( const QString& mimeType ) const;

    /**
     * Retrieve data from the system clipboard.
     * No copy is involved, since the return QByteArray is implicitly shared
     */
    QByteArray data( const QString& mimeType ) const;

    /**
     * Source fields
     */
    QgsFields fields() const { return !mUseSystemClipboard ? mFeatureFields : retrieveFields(); }

  private slots:

    void systemClipboardChanged();

  signals:
    /** Emitted when content changed */
    void changed();

  private:

    /**
     * Set system clipboard from previously set features.
     */
    void setSystemClipboard();

    /** Creates a text representation of the clipboard features.
     * @returns clipboard text, respecting user export format
     */
    QString generateClipboardText() const;

    /** Attempts to convert a string to a list of features, by parsing the string as WKT and GeoJSON
     * @param string string to convert
     * @param fields fields for resultant features
     * @returns list of features if conversion was successful
     */
    QgsFeatureList stringToFeatureList( const QString& string, const QgsFields& fields ) const;

    /** Attempts to parse the clipboard contents and return a QgsFields object representing the fields
     * present in the clipboard.
     * @note Only valid for text based clipboard contents
     */
    QgsFields retrieveFields() const;

    /** QGIS-internal vector feature clipboard.
        Stored as values not pointers as each clipboard operation
        involves a deep copy anyway.
     */
    QgsFeatureList mFeatureClipboard;
    QgsFields mFeatureFields;
    QgsCoordinateReferenceSystem mCRS;

    /** True when the data from the system clipboard should be read */
    bool mUseSystemClipboard;

    friend class TestQgisAppClipboard;

};

#endif
