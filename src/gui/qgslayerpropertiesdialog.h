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
class QgsMapLayerConfigWidget;
class QgsMapLayerConfigWidgetFactory;
class QgsMapCanvas;

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

#ifndef SIP_RUN

    /**
     * Style storage type.
     */
    enum StyleType
    {
      QML,
      SLD,
      DatasourceDatabase,
      UserDatabase,
    };
    Q_ENUM( StyleType )
#endif

    /**
     * Constructor for QgsLayerPropertiesDialog.
     *
     * \param layer associated map layer
     * \param canvas associated map canvas
     * \param settingsKey QgsSettings subgroup key for saving/restore ui states, e.g. "VectorLayerProperties".
     * \param parent parent object (owner)
     * \param fl widget flags
     * \param settings custom QgsSettings pointer
     */
    QgsLayerPropertiesDialog( QgsMapLayer *layer, QgsMapCanvas *canvas, const QString &settingsKey, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = Qt::WindowFlags(), QgsSettings *settings = nullptr );

    /**
     * Sets the metadata \a widget and \a page associated with the dialog.
     *
     * This must be called in order for the standard metadata loading/saving functionality to be available.
     */
    void setMetadataWidget( QgsMetadataWidget *widget, QWidget *page );

    /**
     * Adds properties page from a \a factory.
     */
    virtual void addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory );

    /**
     * Saves the default style when appropriate button is pressed
     *
     * \since QGIS 3.30
     */
    void saveDefaultStyle();

    /**
     * Triggers a dialog to load a saved style
     *
     * \since QGIS 3.30
     */
    void loadStyle();

    /**
     * Saves a style when appriate button is pressed
     *
     * \since QGIS 3.30
     */
    void saveStyleAs();

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
     * Initialize the dialog. Must be called in the subclass constructor
     * as the final call.
     */
    void initialize();

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

    //! Associated map canvas
    QgsMapCanvas *mCanvas = nullptr;

    //! Layer config widgets
    QList<QgsMapLayerConfigWidget *> mConfigWidgets;

  protected slots:

    /**
     * Resets the dialog to the current layer state.
      */
    virtual void syncToLayer() SIP_SKIP = 0;

    /**
     * Applies the dialog settings to the layer.
     */
    virtual void apply() SIP_SKIP = 0;

    /**
     * Rolls back changes made to the layer.
     */
    virtual void rollback();

    void optionsStackedWidget_CurrentChanged( int index ) override;

    /**
     * Handles opening a \a url from the dialog.
     *
     * If the \a url refers to a local file then a file explorer will be opened pointing to the file.
     * If it refers to a remote link then a web browser will be opened instead.
     */
    void openUrl( const QUrl &url );

  private:

    /**
     * Generates the window title for the dialog.
     */
    QString generateDialogTitle() const;

    QPointer< QgsMapLayer> mLayer;

    QgsMetadataWidget *mMetadataWidget = nullptr;
    QWidget *mMetadataPage = nullptr;

};

#endif // QGSLAYERPROPERTIESDIALOG_H
