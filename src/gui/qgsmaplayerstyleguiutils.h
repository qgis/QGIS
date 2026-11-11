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

#include "qgis_gui.h"

#include <QObject>

#define SIP_NO_FILE

class QgsMapLayer;

class QAction;
class QMenu;


/**
 * \ingroup gui
 * \class QgsMapLayerStyleGuiUtils
 * \brief Various GUI utility functions for dealing with map layer's style manager
 * \note not available in Python bindings
 * \since QGIS 3.12 (in the GUI API)
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

    /**
     * \brief removes extra separators from the menu
     *
     * \since QGIS 3.14
     */
    void removesExtraMenuSeparators( QMenu *m );

    /**
     * Saves the current style of the \a layer as its default style.
     *
     * \returns TRUE if the style was saved
     *
     * \see loadDefaultStyle()
     * \since QGIS 4.6
     */
    static bool saveStyleAsDefault( QgsMapLayer *layer );

    /**
     * Restores the default style for the \a layer.
     *
     * \returns TRUE if the default style was loaded
     *
     * \see saveStyleAsDefault()
     * \since QGIS 4.6
     */
    static bool loadDefaultStyle( QgsMapLayer *layer );

  public slots:

  private:
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
