/***************************************************************************
  qgslateralpanelwidget.h
  --------------------------------------
  Date                 : July 2026
  Copyright            : (C) 2026 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLATERALPANELWIDGET_H
#define QGSLATERALPANELWIDGET_H

#include "qgslateralpanelwidget.h"

#include "qgis.h"
#include "qgis_3d.h"

#include <QTabWidget>

#define SIP_NO_FILE

class Qgs3DEditingToolBar;
class Qgs3DMapCanvas;


/**
 * \ingroup qgis_3d
 * Provide abstraction to lateral panel.
 * \since QGIS 4.4
 */
class _3D_EXPORT QgsLateralPanelWidget : public QTabWidget
{
    Q_OBJECT
  public:
    //! Default constructor
    QgsLateralPanelWidget( QAction *toggleAction, QWidget *parent = nullptr );

    /**
     * Adds a widget to the panel by its name.
     * \param widget the widget to add
     * \param name widget display name
     * \return the widget index inside the panel
     */
    int addWidget( QWidget *widget, const QString &name );

    /**
     * Activates the panel and focus on a specific widget.
     * \param index widget index
     */
    void showWidget( int index );

    /**
     * Activates the panel and focus on a specific widget.
     * \param name widget display name
     */
    void showWidget( const QString &name );

    /**
     * Hides a specific widget.
     * \param index widget index
     */
    void hideWidget( int index );

    /**
     * Hides a specific widget.
     * \param name widget display name
     */
    void hideWidget( const QString &name );

    /**
     * Removes a specific widget.
     * \param index widget index
     */
    void removeWidget( int index );

    /**
     * Removes a specific widget.
     * \param name widget display name
     */
    void removeWidget( const QString &name );

    /**
     * Hides the panel
     */
    void hide();

    /**
     * Returns the action to show/hide the panel
     */
    QAction *toggleAction();

  private:
    QAction *mToggleAction = nullptr;
};

#endif //QGSLATERALPANELWIDGET_H
