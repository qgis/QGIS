/***************************************************************************
                             qgsmaplayerserverproperties.h
                              ------------------
  begin                : June 21, 2021
  copyright            : (C) 2021 by Etienne Trimaille
  email                : etrimaille at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERSERVERPROPERTIES_H
#define QGSMAPLAYERSERVERPROPERTIES_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include <QMap>
#include <QString>
#include <QMetaType>
#include <QVariant>

class QgsMapLayer;

class QDomNode;
class QDomDocument;

/**
 * \ingroup core
 * \brief Manages QGIS Server properties for a map layer
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMapLayerServerProperties
{
    Q_GADGET

  public:

    /**
     * Constructor - Creates a Map Layer QGIS Server Properties
     *
     * \param layer The map layer
     */
    QgsMapLayerServerProperties( QgsMapLayer *layer = nullptr );

  private:
    QgsMapLayer *mLayer = nullptr;

};

#endif // QGSMAPLAYERSERVERPROPERTIES_H
