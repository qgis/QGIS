/***************************************************************************
    qgspointcloudlayereditutils.h
    ---------------------
    begin                : December 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYEREDITUTILS_H
#define QGSPOINTCLOUDLAYEREDITUTILS_H

#include <QVector>
#include <QByteArray>

#define SIP_NO_FILE

class QgsPointCloudLayer;
class QgsPointCloudEditingIndex;
class QgsPointCloudNodeId;
class QgsPointCloudAttribute;
class QgsPointCloudAttributeCollection;
class QgsPointCloudRequest;

class QgsPointCloudLayerEditUtils
{
  public:
    QgsPointCloudLayerEditUtils( QgsPointCloudLayer *layer );

    /**
     * Attempts to modify attribute values for specific points in the editing buffer.
     *
     * \param n The point cloud node containing the points
     * \param points The point ids of the points to be modified
     * \param attribute The attribute whose value will be updated
     * \param value The new value to set to the attribute
     * \return TRUE if the editing buffer was updated successfully, FALSE otherwise
     */
    bool changeAttributeValue( const QgsPointCloudNodeId &n, const QVector<int> &points, const QgsPointCloudAttribute &attribute, double value );

    //! Takes \a data comprising of \a allAttributes and returns a QByteArray with data only for the attributes included in the \a request
    static QByteArray dataForAttributes( const QgsPointCloudAttributeCollection &allAttributes, const QByteArray &data, const QgsPointCloudRequest &request );

    //! Check if \a value is within proper range for the \a attribute
    static bool isAttributeValueValid( const QgsPointCloudAttribute &attribute, double value );

  private:
    QgsPointCloudEditingIndex *mIndex = nullptr;
};

#endif // QGSPOINTCLOUDLAYEREDITUTILS_H
