/***************************************************************************
  qgslayerpropertiesdialog.h
  --------------------------------------
  Date                 : June 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERPROPERTIESDIALOG_H
#define QGSLAYERPROPERTIESDIALOG_H

#include "qgsgui.h"
#include "qgsoptionsdialogbase.h"
#include "qgsmaplayerstyle.h"

#include <QObject>
#include <QPointer>

class QgsMapLayer;
class QgsMetadataWidget;

/**
 * \ingroup gui
 * \class QgsLayerPropertiesDialog
 * \brief Base class for "layer properties" dialogs, containing common utilities for handling functionality in these dialog.
 * \warning This is exposed to SIP bindings for legacy reasons only, and is NOT to be considered as stable API.
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsLayerPropertiesDialog : public QgsOptionsDialogBase SIP_ABSTRACT
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayerPropertiesDialog.
     *
     * \param layer associated map layer
     * \param settingsKey QgsSettings subgroup key for saving/restore ui states, e.g. "VectorLayerProperties".
     * \param parent parent object (owner)
     * \param fl widget flags
     * \param settings custom QgsSettings pointer
     */
    QgsLayerPropertiesDialog( QgsMapLayer *layer, const QString &settingsKey, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = Qt::WindowFlags(), QgsSettings *settings = nullptr );

    /**
     * Sets the metadata \a widget and \a page associated with the dialog.
     *
     * This must be called in order for the standard metadata loading/saving functionality to be avialable.
     */
    void setMetadataWidget( QgsMetadataWidget *widget, QWidget *page );

  public slots:

    /**
     * Allows the user to load layer metadata from a file.
     *
     * \see saveMetadataToFile()
     */
    void loadMetadataFromFile();

    /**
     * Allows the user to save the layer's metadata as a file.
     *
     * \see loadMetadataFromFile()
     */
    void saveMetadataToFile();

    /**
     * Saves the current layer metadata as the default for the layer.
     *
     * loadDefaultMetadata()
     */
    void saveMetadataAsDefault();

    /**
     * Reloads the default layer metadata for the layer.
     *
     * \see saveMetadataAsDefault()
     */
    void loadDefaultMetadata();

    /**
     * Allows the user to load layer style from a file.
     *
     * \see saveStyleToFile()
     */
    void loadStyleFromFile();

    /**
     * Allows the user to save the layer's style to a file.
     *
     * \see loadStyleFromFile()
     */
    void saveStyleToFile();

    /**
     * Saves the current layer style as the default for the layer.
     *
     * loadDefaultStyle()
     */
    void saveStyleAsDefault();

    /**
     * Reloads the default style for the layer.
     */
    void loadDefaultStyle();

  protected:

    /**
     * Ensures the dialog is focused and activated.
     */
    void refocusDialog();

    /**
     * Stores the current layer style so that undo operations can be performed.
     */
    void storeCurrentStyleForUndo();

    /**
    * Previous layer style. Used to reset style to previous state if new style
    * was loaded but dialog is canceled.
    */
    QgsMapLayerStyle mOldStyle;

    //! Style button
    QPushButton *mBtnStyle = nullptr;

    //! Metadata button
    QPushButton *mBtnMetadata = nullptr;

  protected slots:

    /**
     * Resets the dialog to the current layer state.
      */
    virtual void syncToLayer() SIP_SKIP = 0;

    /**
     * Applies the dialog settings to the layer.
     */
    virtual void apply() SIP_SKIP = 0;

    void optionsStackedWidget_CurrentChanged( int index ) override;

  private:

    QPointer< QgsMapLayer> mLayer;

    QgsMetadataWidget *mMetadataWidget = nullptr;
    QWidget *mMetadataPage = nullptr;

};

#endif // QGSLAYERPROPERTIESDIALOG_H
