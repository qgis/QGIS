/***************************************************************************
  qgsthememanagerwidget.h
  --------------------------------------
  Date                 : April 2021
  Copyright            : (C) 2021 by Alex RL
  Email                : ping me on github
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTHEMEMANAGERWIDGET_H
#define QGSTHEMEMANAGERWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QComboBox>
#include "qgsdockwidget.h"
#include "qgslayertreeview.h"
#include "ui_qgsthememanagerwidgetbase.h"


class QgsMapThemeCollection;
class QMimeData;
class QgsThemeViewer;
#define SIP_NO_FILE


/**
 * QgsThemeManagerWidget class: Used to display layers of selected themes in order to easily modify existing themes.
 * \since QGIS 3.20
 */

class QgsThemeManagerWidget : public QgsDockWidget, private Ui::QgsThemeManagerWidgetBase
{
    Q_OBJECT
  public:

    //! Base constructor for the widget
    QgsThemeManagerWidget( QWidget *parent = nullptr );

  signals:

    /**
     * Used to call viewCurrentTheme
     */
    void themeChanged();

    /**
     * Used to add layers from the layertree to the theme
     */
    void addLayerTreeLayers();

    /**
     * Used to remove layers from the theme
     */
    void droppedLayers();

  private slots:

    /**
     * Reset members to match current project.
     */
    void projectLoaded();

    /**
     * Used to catche the ComboBox signal
     */
    void setTheme( const int index );

    /**
     * Triggered by the previous theme button
     */
    void previousTheme();

    /**
     * Triggered by the next theme button
     */
    void nextTheme();

    /**
     * Populate the ComboBox to keep the list synched
     */
    void populateCombo();

    /**
     * Populate the ThemeViewer
     */
    void viewCurrentTheme() const;

    /**
     * Triggered by the all layer button
     */
    void addSelectedLayers();

    /**
     * Triggered by the remove layer button
     */
    void removeSelectedLayers();

    /**
     * Update the comboBox
     */
    void updateComboBox();

    //! Show the widget
    void showWidget();

    //! Remove selected theme from the project
    void removeTheme();

    //! Create a new theme based on visible layers/nodes.
    void createTheme();

    //! right click menu
    void showContextMenu( const QPoint &pos );

    //! show or hide all non-spatial layers & empty groups, used by context menu.
    void changeVisibility();

  private:

    /**
     * Used by the tool or drag & drop from the main layertree
     */
    void appendLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Used by the tool or drag & drop
     */
    void removeThemeLayers( const QList<QgsMapLayer *> &layers );

    //! Intercept drags to prevent loss of information.
    void startDrag( Qt::DropActions );

    bool mShowAllLayers = false;
    QString mCurrentTheme;
    QgsMapThemeCollection *mThemeCollection = nullptr;

};


#endif
