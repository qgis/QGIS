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

#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

class QProgressDialog;

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

    /** Write contents of vector layer to a different datasource */
    static ImportError importLayer( QgsVectorLayer* layer,
                                    const QString& uri,
                                    const QString& providerKey,
                                    const QgsCoordinateReferenceSystem *destCRS,
                                    bool onlySelected = false,
                                    QString *errorMessage = nullptr,
                                    bool skipAttributeCreation = false,
                                    QMap<QString, QVariant> *options = nullptr,
                                    QProgressDialog *progress = nullptr
                                  );

    /** Create a empty layer and add fields to it */
    QgsVectorLayerImport( const QString &uri,
                          const QString &provider,
                          const QgsFields &fields,
                          QGis::WkbType geometryType,
                          const QgsCoordinateReferenceSystem* crs,
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
