/***************************************************************************
                          qgsvirtuallayertask.h  -  description
                             -------------------
    begin                : Jan 19, 2018
    copyright            : (C) 2017 by Paul Blottiere
    email                : blottiere.paul@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUALLAYERTASK_H
#define QGSVIRTUALLAYERTASK_H

#include "qgsvectorlayer.h"
#include "qgsvirtuallayerdefinition.h"
#include "qgstaskmanager.h"

/**
 * \ingroup core
 *
 * Initializes a virtual layer in a separated task.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsVirtualLayerTask : public QgsTask
{
    Q_OBJECT

  public:

    QgsVirtualLayerTask( const QgsVirtualLayerDefinition &definition );

    QgsVectorLayer *layer();

    QgsVirtualLayerDefinition definition() const;

    bool run() override;

    void cancel() override;

  private:
    QgsVirtualLayerDefinition mDefinition;
    std::unique_ptr<QgsVectorLayer> mLayer;
};

#endif // QGSVECTORLAYERTASK_H
