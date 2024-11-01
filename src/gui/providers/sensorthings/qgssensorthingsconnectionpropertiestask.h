/***************************************************************************
    qgssensorthingsconnectionpropertiestask.h
    ---------------------
    Date                 : February 2024
    Copyright            : (C) 2024 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENSORTHINGSCONNECTIONPROPERTIESTASK_H
#define QGSSENSORTHINGSCONNECTIONPROPERTIESTASK_H

///@cond PRIVATE
#define SIP_NO_FILE

#include "qgstaskmanager.h"
#include "qgis.h"

class QgsSensorThingsConnectionPropertiesTask : public QgsTask
{
    Q_OBJECT
  public:
    QgsSensorThingsConnectionPropertiesTask( const QString &uri, Qgis::SensorThingsEntity entity );
    void cancel() final;

    /**
     * Returns the retrieved available geometry types.
     */
    QList<Qgis::GeometryType> geometryTypes() const { return mGeometryTypes; };

  protected:
    bool run() final;

  private:
    QString mUri;
    Qgis::SensorThingsEntity mEntity = Qgis::SensorThingsEntity::Invalid;
    std::unique_ptr<QgsFeedback> mFeedback;
    QList<Qgis::GeometryType> mGeometryTypes;
};

///@endcond PRIVATE
#endif // QGSSENSORTHINGSCONNECTIONPROPERTIESTASK_H
