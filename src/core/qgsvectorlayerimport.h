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

#ifndef _QGSVECTORLAYERIMPORT_H_
#define _QGSVECTORLAYERIMPORT_H_

#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

/** \ingroup core
  * A convenience class for writing vector files to disk.
 There are two possibilities how to use this class:
 1. static call to QgsVectorFileWriter::writeAsShapefile(...) which saves the whole vector layer
 2. create an instance of the class and issue calls to addFeature(...)

 Currently supports only writing to shapefiles, but shouldn't be a problem to add capability
 to support other OGR-writable formats.
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
      ErrConnectionFailed
    };


    /** Write contents of vector layer to a different datasource */
    static ImportError importLayer( QgsVectorLayer* layer,
                                    const QString& uri,
                                    const QString& providerKey,
                                    const QgsCoordinateReferenceSystem *destCRS,
                                    bool onlySelected = false,
                                    QString *errorMessage = 0,
                                    bool skipAttributeCreation = false,
                                    QMap<QString, QVariant> *options = 0
                                  );

    /** create a empty layer and add fields to it */
    QgsVectorLayerImport( const QString &uri,
                          const QString &provider,
                          const QgsFieldMap& fields,
                          QGis::WkbType geometryType,
                          const QgsCoordinateReferenceSystem* crs,
                          bool overwrite = false,
                          const QMap<QString, QVariant> *options = 0
                        );

    /** checks whether there were any errors */
    ImportError hasError();

    /** retrieves error message */
    QString errorMessage();

    /** add feature to the new created layer */
    bool addFeature( QgsFeature& feature );

    /** close the new created layer */
    ~QgsVectorLayerImport();

  protected:
    /** contains error value */
    ImportError mError;
    QString mErrorMessage;

    QgsVectorDataProvider *mProvider;

    /** map attribute indexes to new field indexes */
    QMap<int, int> mOldToNewAttrIdx;
};

#endif
