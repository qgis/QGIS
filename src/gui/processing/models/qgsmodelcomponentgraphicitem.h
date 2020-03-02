/***************************************************************************
                             qgsmodelcomponentgraphicitem.h
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELCOMPONENTGRAPHICITEM_H
#define QGSMODELCOMPONENTGRAPHICITEM_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QGraphicsObject>

class QgsProcessingModelComponent;


///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Base class for graphic items representing model components in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelComponentGraphicItem : public QGraphicsObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelComponentGraphicItem for the specified \a component, with the specified \a parent item.
     *
     * Ownership of \a component is transferred to the item.
     */
    QgsModelComponentGraphicItem( QgsProcessingModelComponent *component SIP_TRANSFER, QGraphicsItem *parent SIP_TRANSFERTHIS );

    /**
     * Returns the model component associated with this item.
     */
    QgsProcessingModelComponent *component();

  private:

    std::unique_ptr< QgsProcessingModelComponent > mComponent;


};

///@endcond

#endif // QGSMODELCOMPONENTGRAPHICITEM_H
