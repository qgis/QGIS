/***************************************************************************
                          qgsvectorlayerimport.cpp
                          vector layer importer
                             -------------------
    begin                : Thu Aug 25 2011
    copyright            : (C) 2011 by Giuseppe Sucameli
    email                : brush.tyler at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERIMPORT_H
#define QGSVECTORLAYERIMPORT_H

#include "qgsfeature.h"

class QProgressDialog;
class QgsVectorDataProvider;
class QgsVectorLayer;
class QgsFields;

/** \ingroup core
  * A convenience class for writing vector files to disk.
 There are two possibilities how to use this class:
 1. static call to QgsVectorFileWriter::writeAsShapefile(...) which saves the whole vector layer
 2. create an instance of the class and issue calls to addFeature(...)
 */
class CORE_EXPORT QgsVectorLayerImport
{
  public:

    enum ImportError
    {
      NoError = 0,
      ErrDriverNotFound,
      ErrCreateDataSource,
      ErrCreateLayer,
      ErrAttributeTypeUnsupported,
      ErrAttributeCreationFailed,
      ErrProjection,
      ErrFeatureWriteFailed,
      ErrInvalidLayer,
      ErrInvalidProvider,
      ErrProviderUnsupportedFeature,
      ErrConnectionFailed,
      ErrUserCancelled, /*!< User cancelled the import*/
    };

    /**
     * Writes the contents of vector layer to a different datasource.
     * @param layer source layer
     * @param uri URI for destination data source
     * @param providerKey string key for destination data provider
     * @param destCRS destination CRS, or an invalid (default constructed) CRS if
     * not available
     * @param onlySelected set to true to export only selected features
     * @param errorMessage if non-null, will be set to any error messages
     * @param skipAttributeCreation set to true to skip exporting feature attributes
     * @param options optional provider dataset options
     * @param progress optional progress dialog to show progress of export
     * @returns NoError for a successful export, or encountered error
     */
    static ImportError importLayer( QgsVectorLayer* layer,
                                    const QString& uri,
                                    const QString& providerKey,
                                    const QgsCoordinateReferenceSystem& destCRS,
                                    bool onlySelected = false,
                                    QString *errorMessage = nullptr,
                                    bool skipAttributeCreation = false,
                                    QMap<QString, QVariant> *options = nullptr,
                                    QProgressDialog *progress = nullptr
                                  );

    /** Constructor for QgsVectorLayerImport.
     * @param uri URI for destination data source
     * @param provider string key for destination data provider
     * @param fields fields to include in created layer
     * @param geometryType destination geometry type
     * @param crs desired CRS, or an invalid (default constructed) CRS if
     * not available
     * @param overwrite set to true to overwrite any existing data source
     * @param options optional provider dataset options
     * @param progress optional progress dialog to show progress of export
     */
    QgsVectorLayerImport( const QString &uri,
                          const QString &provider,
                          const QgsFields &fields,
                          Qgis::WkbType geometryType,
                          const QgsCoordinateReferenceSystem& crs,
                          bool overwrite = false,
                          const QMap<QString, QVariant> *options = nullptr,
                          QProgressDialog *progress = nullptr
                        );

    /** Checks whether there were any errors */
    ImportError hasError();

    /** Retrieves error message */
    QString errorMessage();

    int errorCount() const { return mErrorCount; }

    /** Add feature to the new created layer */
    bool addFeature( QgsFeature& feature );

    /** Close the new created layer */
    ~QgsVectorLayerImport();

  protected:
    /** Flush the buffer writing the features to the new layer */
    bool flushBuffer();

    /** Create index */
    bool createSpatialIndex();

    /** Contains error value */
    ImportError mError;
    QString mErrorMessage;

    int mErrorCount;

    QgsVectorDataProvider *mProvider;

    /** Map attribute indexes to new field indexes */
    QMap<int, int> mOldToNewAttrIdx;
    int mAttributeCount;

    QgsFeatureList mFeatureBuffer;
    QProgressDialog *mProgress;

  private:

    QgsVectorLayerImport( const QgsVectorLayerImport& rh );
    QgsVectorLayerImport& operator=( const QgsVectorLayerImport& rh );
};

#endif
