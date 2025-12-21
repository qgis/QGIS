/***************************************************************************
                              qgsxmlschemaanalyzer.h
                              ----------------------
  begin                : December 2025
  copyright            : (C) 2006 by Marco Hugentobler
                         (C) 2016 by Even Rouault
  email                : even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSXMLSCHEMAANALYSER_H
#define QGSXMLSCHEMAANALYSER_H

#include "qgis.h"
#include "qgsfields.h"

#include <QByteArray>
#include <QDomDocument>
#include <QString>

class QgsBackgroundCachedSharedData;

class QgsXmlSchemaAnalyzer
{
  public:
    /**
   * For a given typename, reads the name of the geometry attribute, the
   * thematic attributes and their types from a dom document. Returns true in case of success.
  */
    static bool readAttributesFromSchema(
      const QString &translatedProviderName,
      QgsBackgroundCachedSharedData *sharedData,
      const Qgis::VectorProviderCapabilities &capabilities,
      QDomDocument &schemaDoc,
      const QByteArray &response,
      bool singleLayerContext,
      const QString &prefixedTypename,
      QString &geometryAttribute,
      QgsFields &fields,
      Qgis::WkbType &geomType,
      bool &geometryMaybeMissing,
      QString &errorMsg,
      bool &metadataRetrievalCanceled
    );

  private:
    static bool readAttributesFromSchemaWithGMLAS(
      const QString &translatedProviderName,
      QgsBackgroundCachedSharedData *sharedData,
      const QByteArray &response,
      const QString &prefixedTypename,
      QString &geometryAttribute,
      QgsFields &fields,
      Qgis::WkbType &geomType,
      bool &geometryMaybeMissing,
      QString &errorMsg,
      bool &metadataRetrievalCanceled
    );

    static bool readAttributesFromSchemaWithoutGMLAS(
      const QString &translatedProviderName,
      QgsBackgroundCachedSharedData *sharedData,
      QDomDocument &schemaDoc,
      const QString &prefixedTypename,
      QString &geometryAttribute,
      QgsFields &fields,
      Qgis::WkbType &geomType,
      QString &errorMsg,
      bool &mayTryWithGMLAS
    );
};

#endif
