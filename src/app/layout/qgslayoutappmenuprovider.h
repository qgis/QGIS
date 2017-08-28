/***************************************************************************
                             qgslayoutappmenuprovider.h
                             -------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTAPPMENUPROVIDER_H
#define QGSLAYOUTAPPMENUPROVIDER_H

#include "qgis.h"
#include "qgslayoutview.h"
#include <QObject>

class QgsLayoutDesignerDialog;

/**
 * A menu provider for QgsLayoutView
 */
class QgsLayoutAppMenuProvider : public QObject, public QgsLayoutViewMenuProvider
{
    Q_OBJECT

  public:

    QgsLayoutAppMenuProvider( QgsLayoutDesignerDialog *designer );

    QMenu *createContextMenu( QWidget *parent, QgsLayout *layout, QPointF layoutPoint ) const override;

  private:

    QgsLayoutDesignerDialog *mDesigner = nullptr;

};

#endif // QGSLAYOUTAPPMENUPROVIDER_H
