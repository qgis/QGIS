/***************************************************************************
                              qgsserverfeatureid.h
                              -----------------------
  begin                : May 17, 2019
  copyright            : (C) 2019 by René-Luc DHONT
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSERVERFEATUREID_H
#define QGSSERVERFEATUREID_H

#include <QString>
#include <QHash>

#include "qgis_server.h"
#include "qgsfield.h"

class QgsVectorDataProvider;
class QgsFeature;
class QgsFeatureRequest;


#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsserverfeatureid.h"
% End
#endif



/**
 * \ingroup server
 * \brief The QgsServerFeatureId namespace provides a way to use primary keys for
 * feature id.
 * \since QGIS 3.4.9
 */
namespace QgsServerFeatureId
{

  /**
   * Returns the feature id based on primary keys.
   * \param feature the feature
   * \param pkAttributes the primary keys list
   * \returns the feature id based on primary keys
   * \since QGIS 3.4.9
   */
  SERVER_EXPORT QString getServerFid( const QgsFeature &feature, const QgsAttributeList &pkAttributes );

  /**
   * Returns the feature request based on feature ids build with primary keys.
   * \param featureRequest the feature request to update
   * \param serverFids the feature ids build with QgsServerFeatureId::getServerFid
   * \param provider the vector layer provider to provide fields and primary keys list
   * \returns the feature request updated
   * \since QGIS 3.4.9
   */
  SERVER_EXPORT QgsFeatureRequest updateFeatureRequestFromServerFids( QgsFeatureRequest &featureRequest, const QStringList &serverFids, const QgsVectorDataProvider *provider );

  /**
   * Returns the expression feature id based on primary keys.
   * \param serverFid the feature id build with primary keys
   * \param provider the vector layer provider to provide fields and primary keys list
   * \returns the feature id based on primary keys
   * \since QGIS 3.4.9
   */
  SERVER_EXPORT QString getExpressionFromServerFid( const QString &serverFid, const QgsVectorDataProvider *provider );

  /**
   * Returns the primary keys separator
   * \returns @@ the primary keys separator
   * \since QGIS 3.4.9
   */
  SERVER_EXPORT QString pkSeparator();

};

#endif
