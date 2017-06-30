/***************************************************************************
                            qgslayoutitemregistry.h
                            ------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTITEMREGISTRY_H
#define QGSLAYOUTITEMREGISTRY_H

#include "qgis_core.h"
#include <QGraphicsItem> //for QGraphicsItem::UserType

class QgsLayoutItem;

/**
 * \ingroup core
 * \class QgsLayoutItemRegistry
 * \brief Registry of available layout item types.
 *
 * QgsLayoutItemRegistry is not usually directly created, but rather accessed through
 * QgsApplication::layoutItemRegistry().
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemRegistry : public QObject
{
    Q_OBJECT

  public:

    //! Item types
    enum ItemType
    {
      LayoutItem = QGraphicsItem::UserType + 100, //!< Base class for items

      // known item types
      LayoutPage, //!< Page items

      // item types provided by plugins
      PluginItem, //!< Starting point for plugin item types
    };

    /**
     * Creates a registry and populates it with standard item types.
     *
     * QgsLayoutItemRegistry is not usually directly created, but rather accessed through
     * QgsApplication::layoutItemRegistry().
    */
    QgsLayoutItemRegistry( QObject *parent = nullptr );


  private:

};

#endif //QGSLAYOUTITEMREGISTRY_H



