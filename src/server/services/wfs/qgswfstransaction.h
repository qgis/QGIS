/***************************************************************************
                              qgswfstransaction.h
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
#ifndef QGSWFSTRANSACTION_H
#define QGSWFSTRANSACTION_H


namespace QgsWfs
{
  struct transactionInsert
  {
    QString typeName;

    QString handle;

    QDomNodeList featureNodeList;

    QStringList insertFeatureIds;

    bool error;

    QString errorMsg;
  };

  struct transactionUpdate
  {
    QString typeName;

    QString handle;

    QMap<QString, QString> propertyMap;

    QDomElement geometryElement;

    QgsFeatureRequest featureRequest;

    int totalUpdated = 0;

    bool error;

    QString errorMsg;
  };

  struct transactionDelete
  {
    QString typeName;

    QString handle;

    QgsFeatureRequest featureRequest;

    int totalDeleted = 0;

    bool error;

    QString errorMsg;
  };

  struct transactionRequest
  {
    QList< transactionInsert > inserts;

    QList< transactionUpdate > updates;

    QList< transactionDelete > deletes;
  };

  /**
   * Transform Insert element to transactionInsert
   */
  transactionInsert parseInsertActionElement( QDomElement &actionElem );

  /**
   * Transform Update element to transactionUpdate
   */
  transactionUpdate parseUpdateActionElement( QDomElement &actionElem );

  /**
   * Transform Delete element to transactionDelete
   */
  transactionDelete parseDeleteActionElement( QDomElement &actionElem );

  /**
   * Transform RequestBody root element to getFeatureRequest
   */
  transactionRequest parseTransactionRequestBody( QDomElement &docElem );

  transactionRequest parseTransactionParameters( QgsServerRequest::Parameters parameters );

  /**
   * Transform GML feature nodes to features
   */
  QgsFeatureList featuresFromGML( QDomNodeList featureNodeList, QgsVectorDataProvider *provider );

  /**
   * Perform the transaction
   */
  void performTransaction( transactionRequest &aRequest, QgsServerInterface *serverIface, const QgsProject *project );

  /**
   * Output WFS  transaction response
   */
  void writeTransaction( QgsServerInterface *serverIface, const QgsProject *project,
                         const QString &version, const QgsServerRequest &request,
                         QgsServerResponse &response );


  /**
   * Create a wfs transaction document
   */
  QDomDocument createTransactionDocument( QgsServerInterface *serverIface, const QgsProject *project,
                                          const QString &version, const QgsServerRequest &request );

} // namespace QgsWfs

#endif

