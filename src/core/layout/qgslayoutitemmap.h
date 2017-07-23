/***************************************************************************
                              qgslayoutitemmap.h
                             -------------------
    begin                : July 2017
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

#ifndef QGSLAYOUTITEMMAP_H
#define QGSLAYOUTITEMMAP_H

#include "qgis_core.h"
#include "qgslayoutitem.h"

/**
 * \ingroup core
 * \class QgsLayoutItemMap
 * \brief Layout graphical items for displaying a map.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemMap : public QgsLayoutItem
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemMap, with the specified parent \a layout.
     */
    explicit QgsLayoutItemMap( QgsLayout *layout );

  protected:

    void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;
};

#endif //QGSLAYOUTITEMMAP_H
