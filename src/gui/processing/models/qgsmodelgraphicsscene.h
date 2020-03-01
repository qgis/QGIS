/***************************************************************************
                             qgsmodelgraphicsscene.h
                             -----------------------
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

#ifndef QGSMODELGRAPHICSCENE_H
#define QGSMODELGRAPHICSCENE_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QGraphicsScene>


///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief QGraphicsScene subclass representing the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelGraphicsScene with the specified \a parent object.
     */
    QgsModelGraphicsScene( QObject *parent SIP_TRANSFERTHIS = nullptr );


};

///@endcond

#endif // QGSMODELCOMPONENTGRAPHICITEM_H
