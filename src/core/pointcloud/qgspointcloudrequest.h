/***************************************************************************
                         qgspointcloudrequest.h
                         ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDREQUEST_H
#define QGSPOINTCLOUDREQUEST_H

#include "qgis.h"
#include "qgis_core.h"
#include <QPair>
#include <QString>
#include <QVector>
#include <QByteArray>

#define SIP_NO_FILE

#include "qgspointcloudattribute.h"

/**
 * \ingroup core
 *
 * \brief Point cloud data request
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRequest
{
  public:
    //! Ctor
    QgsPointCloudRequest();

    //! Returns attributes
    QgsPointCloudAttributeCollection attributes() const;

    //! Set attributes filter in the request
    void setAttributes( const QgsPointCloudAttributeCollection &attributes );

  private:
    QgsPointCloudAttributeCollection mAttributes;
};

#endif // QGSPOINTCLOUDREQUEST_H
