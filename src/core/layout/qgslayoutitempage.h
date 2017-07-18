/***************************************************************************
                              qgslayoutitempage.h
                             --------------------
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

#ifndef QGSLAYOUTITEMPAGE_H
#define QGSLAYOUTITEMPAGE_H

#include "qgis_core.h"
#include "qgslayoutitem.h"
#include "qgslayoutitemregistry.h"

/**
 * \ingroup core
 * \class QgsLayoutItemPage
 * \brief Item representing the paper in a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemPage : public QgsLayoutItem
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemPage, with the specified parent \a layout.
     */
    explicit QgsLayoutItemPage( QgsLayout *layout SIP_TRANSFERTHIS );
    int type() const override { return QgsLayoutItemRegistry::LayoutPage; }
    QString stringType() const override { return QStringLiteral( "ItemPaper" ); }

  protected:

    void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;
};

#endif //QGSLAYOUTITEMPAGE_H
