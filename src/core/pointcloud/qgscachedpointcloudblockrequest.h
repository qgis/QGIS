/***************************************************************************
                         qgscachedpointcloudblockrequest.h
                         ---------------------------------
    begin                : January 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : stefanos.natsis at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCACHEDPOINTCLOUDBLOCKREQUEST_H
#define QGSCACHEDPOINTCLOUDBLOCKREQUEST_H

#include <QObject>

#include "qgspointcloudblockrequest.h"

#define SIP_NO_FILE

class QgsPointCloudAttributeCollection;
class QgsPointCloudBlock;

/**
 * \ingroup core
 * \brief Class for handling a QgsPointCloudBlockRequest using existing cached QgsPointCloudBlock
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsCachedPointCloudBlockRequest : public QgsPointCloudBlockRequest
{
    Q_OBJECT
  public:

    /**
     * QgsCachedPointCloudBlockRequest constructor using an existing \a block
     * Note: Ownership of \a block is transferred
     */
    QgsCachedPointCloudBlockRequest( QgsPointCloudBlock *block, const QgsPointCloudNodeId &node, const QString &uri,
                                     const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
                                     const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression, const QgsRectangle &filterRect );

    ~QgsCachedPointCloudBlockRequest() = default;
};
#endif // QGSCACHEDPOINTCLOUDBLOCKREQUEST_H
