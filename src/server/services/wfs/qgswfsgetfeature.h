/***************************************************************************
                              qgswfsgetfeature.h
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2012 by René-Luc D'Hont    (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2017 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSGETFEATURE_H
#define QGSWFSGETFEATURE_H

#include "qgswfsparameters.h"

namespace QgsWfs
{
  struct getFeatureQuery
  {
      QString typeName;

      QString srsName;

      QgsFeatureRequest featureRequest;

      QStringList serverFids;

      QStringList propertyList;
  };

  struct getFeatureRequest
  {
      long maxFeatures;

      long startIndex;

      QgsWfsParameters::Format outputFormat;

      QList<getFeatureQuery> queries;

      QString geometryName;
  };

  /**
   * Add SortBy element to featureRequest
   */
  void parseSortByElement( QDomElement &sortByElem, QgsFeatureRequest &featureRequest, const QString &typeName );

  /**
   * Transform Query element to getFeatureQuery
   */
  getFeatureQuery parseQueryElement( QDomElement &queryElem, const QgsProject *project = nullptr );

  /**
   * Transform RequestBody root element to getFeatureRequest
   */
  getFeatureRequest parseGetFeatureRequestBody( QDomElement &docElem, const QgsProject *project = nullptr );

  /**
   * Transform parameters to getFeatureRequest
   */
  getFeatureRequest parseGetFeatureParameters( const QgsProject *project = nullptr );

  /**
   * Output WFS  GetFeature response
   */
  void writeGetFeature( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response );

} // namespace QgsWfs

#endif
