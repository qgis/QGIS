/***************************************************************************
    qgsmaplayerserverpropertieswidget.h
    ---------------------
    begin                : 2025/02/20
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERSERVERPROPERTIESWIDGET
#define QGSMAPLAYERSERVERPROPERTIESWIDGET

#include "ui_qgsmaplayerserverpropertieswidgetbase.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

class QgsMapLayerServerProperties;
class QStandardItemModel;

/**
 * \ingroup gui
 * \class QgsMapLayerServerPropertiesWidget
 * Provides widget to edit server properties
 *
 * \note Not available in Python bindings
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsMapLayerServerPropertiesWidget : public QWidget, private Ui::QgsMapLayerServerPropertiesWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param parent parent widget
     */
    QgsMapLayerServerPropertiesWidget( QWidget *parent = nullptr );

    /**
     * Sets the server properties \a serverProperties associated with the widget.
     */
    void setServerProperties( QgsMapLayerServerProperties *serverProperties );

    /**
     * Set whether or not server properties widget has to display a WFS title widget
     * \param hasWfsTitle TRUE if server properties has to display a WFS title widget
     * \see hasWfsTitle()
     */
    void setHasWfsTitle( bool hasWfsTitle );

    /**
     * Returns TRUE if widget display a WFS title widget
     * \see setHasWfsTitle()
     */
    bool hasWfsTitle() const;

    /**
     * Saves the settings to the server properties.
     *
     * Returns TRUE if server properties have been modified
     */
    bool save();

    /**
     * Updates the widget state to match the current server properties state.
     */
    void sync();

  private slots:

    void addMetadataUrl();
    void removeSelectedMetadataUrl();

  private:
    QgsMapLayerServerProperties *mServerProperties = nullptr;
    QStandardItemModel *mMetadataUrlModel = nullptr;
    bool mHasWfsTitle = true;
};


#endif
