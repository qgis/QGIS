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

#include "qgis_app.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsmaplayer.h"

#include <QList>
#include <QMap>
#include <QMimeData>
#include <QObject>
#include <QPointer>

class QgsVectorLayer;
class QgsVectorTileLayer;
class QgsFeatureStore;
class QgsMessageBar;

/**
 * A custom QMimeData class which stores original representations of features alongside their CRS and field data.
 *
 * This allows:
 *
 * - Lossless pasting of features, retaining all their original properties (including CRS and field definitions)
 * - Lazy generation of clipboard text, only when needed (via retrieveData()). We don't have to build potentially costly
 *   text and HTML representations of thousands of features that may never be used.
 */
class QgsFeatureListMimeData : public QMimeData
{
    Q_OBJECT

  public:
    QgsFeatureListMimeData( QgsMapLayer *sourceLayer, const QgsCoordinateReferenceSystem &crs, const QgsFields &fields, const QgsFeatureList &features );

    QStringList formats() const override;

    QgsFeatureList mFeatures;
    QgsFields mFields;
    QgsCoordinateReferenceSystem mCRS;
    QPointer<QgsMapLayer> mSourceLayer;

  protected:
    QVariant retrieveData( const QString &mimeType, QMetaType type ) const override;

  private:
    QString toText( bool asHtml ) const;
};

/**
 * \brief Handles application clipboard related logic.
 *
 * As this class matures it should also be able to accept CSV representations
 * of features in and out of the system clipboard (QClipboard).
*/
class APP_EXPORT QgsClipboard : public QObject
{
    Q_OBJECT
  public:
    //! Available formats for copying features as text
    enum CopyFormat
    {
      AttributesOnly,    //!< Tab delimited text, attributes only
      AttributesWithWKT, //!< Tab delimited text, with geometry in WKT format
      AttributesWithWKB, //!< Tab delimited text, with geometry in WKB format
      GeoJSON,           //!< GeoJSON FeatureCollection format
    };
    Q_ENUM( CopyFormat )

    static const QString FEATURES_MIME_TYPE;

    /**
     * Constructor for the clipboard.
     */
    QgsClipboard();

    /**
     * Place a copy of the selected features from the specified layer on
     * the internal clipboard, destroying the previous contents.
     */
    void replaceWithCopyOf( QgsVectorLayer *src );

    /**
     * Place a copy of the selected features from the specified layer on
     * the internal clipboard, destroying the previous contents.
     */
    void replaceWithCopyOf( QgsVectorTileLayer *src );

    /**
     * Place a copy of features on the internal clipboard,
     * destroying the previous contents.
     */
    void replaceWithCopyOf( QgsFeatureStore &featureStore, QgsVectorLayer *src = nullptr );

    QgsFeatureList getFeaturesFromClipboardText( const QgsFields &fields = QgsFields() ) const;

    bool getFeaturesFromClipboard( QgsCoordinateReferenceSystem &crs, QgsFields &fields, QgsFeatureList &features ) const;

    /**
     * Returns TRUE if the internal clipboard is empty, else FALSE.
     */
    bool isEmpty() const;

    /**
     * Returns a copy of features on the internal clipboard, transformed
     * from the clipboard CRS to the destCRS.
     */
    QgsFeatureList transformedCopyOf( QgsVectorLayer *destinationLayer ) const;

    /**
     * Stores a MimeData together with a text into the system clipboard
     */
    void setData( const QString &mimeType, const QByteArray &data, const QString &text = QString() );

    /**
     * Stores a text into the system clipboard
     */
    void setText( const QString &text );

    /**
     * Proxy to QMimeData::hasFormat
     * Tests whether the system clipboard contains data of a given MIME type
     */
    bool hasFormat( const QString &mimeType ) const;

    /**
     * Retrieve data from the system clipboard.
     * No copy is involved, since the return QByteArray is implicitly shared
     */
    QByteArray data( const QString &mimeType ) const;

    QgsMapLayer *layer() const;

    /**
     * Pastes clipboard features to a new memory layer.
     *
     * If no features are in clipboard an empty layer is returned.
     *
     * Returns a new memory layer or NULLPTR if the operation failed.
     */
    std::unique_ptr<QgsVectorLayer> pasteToNewMemoryVector( QgsMessageBar *messageBar );

  private slots:

    void systemClipboardChanged();

  signals:
    //! Emitted when content changed
    void changed();

  private:
    const QgsFeatureListMimeData *featureListDataFromClipboard() const;

    /**
     * Set system clipboard from previously set features.
     */
    void setSystemClipboard( QgsMapLayer *sourceLayer, const QgsCoordinateReferenceSystem &crs, const QgsFields &fields, const QgsFeatureList &features );

    /**
     * Attempts to convert a string to a list of features, by parsing the string as WKT/WKB and GeoJSON
     * \param string string to convert
     * \param fields fields for resultant features
     * \returns list of features if conversion was successful
     */
    QgsFeatureList stringToFeatureList( const QString &string, const QgsFields &fields ) const;

    /**
     * Attempts to parse the clipboard contents and return a QgsFields object representing the fields
     * present in the clipboard.
     * \note Only valid for text based clipboard contents
     */
    QgsFields retrieveFields() const;

    friend class TestQgisAppClipboard;
};

#endif
