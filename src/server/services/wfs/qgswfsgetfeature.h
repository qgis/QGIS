/***************************************************************************
                              qgswfsgetfeature.h
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
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


namespace QgsWfs
{
  struct getFeatureQuery
  {
    QString typeName;

    QgsFeatureRequest featureRequest;

    QStringList propertyList;
  };

  struct getFeatureRequest
  {
    long maxFeatures;

    long startIndex;

    QString outputFormat;

    QList< getFeatureQuery > queries;

    QString geometryName;
  };

  /** Transform Query element to getFeatureQuery
   */
  getFeatureQuery parseQueryElement( QDomElement &queryElem );

  /** Transform RequestBody root element to getFeatureRequest
   */
  getFeatureRequest parseGetFeatureRequestBody( QDomElement &docElem );

  /** Transform parameters to getFeatureRequest
   */
  getFeatureRequest parseGetFeatureParameters( QgsServerRequest::Parameters parameters );

  /** Output WFS  GetFeature response
   */
  void writeGetFeature( QgsServerInterface *serverIface, const QgsProject *project,
                        const QString &version, const QgsServerRequest &request,
                        QgsServerResponse &response );

} // samespace QgsWfs

#endif

