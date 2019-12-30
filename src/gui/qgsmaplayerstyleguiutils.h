/***************************************************************************
  qgsmaplayerstyleguiutils.h
  --------------------------------------
  Date                 : January 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERSTYLEGUIUTILS_H
#define QGSMAPLAYERSTYLEGUIUTILS_H

#include <QObject>
#include "qgis_gui.h"

class QgsMapLayer;

class QAction;
class QMenu;

/**
 * \ingroup gui
 * \class QgsMapLayerStyleGuiUtils
 * Various GUI utility functions for dealing with map layer's style manager
 * \since QGIS 3.10 (in the GUI API)
 */

class GUI_EXPORT QgsMapLayerStyleGuiUtils : public QObject
{
    Q_OBJECT

  public:

    /**
     * \brief returns a singleton instance of this class
     */
    static QgsMapLayerStyleGuiUtils *instance();

    /**
     * \brief adds actions to the menu in accordance to the layer
     */
    void addStyleManagerActions( QMenu *m, QgsMapLayer *layer );

  private :
    QAction *actionAddStyle( QgsMapLayer *layer, QObject *parent = nullptr );
    QAction *actionRemoveStyle( QgsMapLayer *layer, QObject *parent = nullptr );
    QAction *actionRenameStyle( QgsMapLayer *layer, QObject *parent = nullptr );
    QList<QAction *> actionsUseStyle( QgsMapLayer *layer, QObject *parent = nullptr );

  private slots:
    void addStyle();
    void useStyle();
    void removeStyle();
    void renameStyle();
};

#endif // QGSMAPLAYERSTYLEGUIUTILS_H
