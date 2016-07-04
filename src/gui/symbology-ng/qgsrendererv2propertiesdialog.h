/***************************************************************************
    qgsrendererv2propertiesdialog.h

    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERERV2PROPERTIESDIALOG_H
#define QGSRENDERERV2PROPERTIESDIALOG_H

#include <QDialog>

#include "ui_qgsrendererv2propsdialogbase.h"

#include "qgsfeaturerequest.h"

class QKeyEvent;

class QgsVectorLayer;
class QgsStyleV2;
class QgsSymbolV2;
class QgsPaintEffect;
class QgsRendererV2Widget;
class QgsMapCanvas;

/** \ingroup gui
 * \class QgsRendererV2PropertiesDialog
 */
class GUI_EXPORT QgsRendererV2PropertiesDialog : public QDialog, private Ui::QgsRendererV2PropsDialogBase
{
    Q_OBJECT

  public:

    /** Constructor for QgsRendererV2PropertiesDialog.
     * @param layer associated layer
     * @param style style collection
     * @param embedded set to true to indicate that the dialog will be embedded in another widget, rather
     * than shown as a dialog by itself
     * @param parent parent widget
     */
    QgsRendererV2PropertiesDialog( QgsVectorLayer* layer, QgsStyleV2* style, bool embedded = false, QWidget* parent = nullptr );
    ~QgsRendererV2PropertiesDialog();

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /**
     * Set the widget in dock mode which tells the widget to emit panel
     * widgets and not open dialogs
     * @param dockMode True to enable dock mode.
     */
    void setDockMode( bool dockMode );

  signals:
    /**
     * Emitted when expression context variables on the associated
     * vector layers have been changed. Will request the parent dialog
     * to re-synchronize with the variables.
     */
    void layerVariablesChanged();

    /**
     * Emitted when something on the widget has changed.
     * All widgets will fire this event to notify of an internal change.
     */
    void widgetChanged();

    /**
     * Emit when you require a panel to be show in the interface.
     * @param panel The panel widget to show.
     * @note If you are connected to this signal you should also connect
     * given panels showPanel signal as they can be nested.
     */
    void showPanel( QgsPanelWidget* panel );

  public slots:
    //! called when user changes renderer type
    void rendererChanged();

    //! Apply the changes from the dialog to the layer.
    void apply();

    //! Apply and accept the changes for the dialog.
    void onOK();

    /**
     * Open a panel or dialog depending on dock mode setting
     * If dock mode is true this method will emit the showPanel signal
     * for connected slots to handle the open event.
     *
     * If dock mode is false this method will open a dialog
     * and block the user.
     *
     * @param panel The panel widget to open.
     */
    void openPanel( QgsPanelWidget* panel );


  private slots:
    void showOrderByDialog();

    void changeOrderBy( const QgsFeatureRequest::OrderBy& orderBy, bool orderByEnabled );

    void updateUIState( bool hidden );

    void syncToLayer();

  protected:
    /**
     * Connect the given slot to the value changed event for the set of widgets
     * Each widget is checked for type and the common type of signal is connected
     * to the slot.
     *
     * @param widgets The list of widgets to check.
     * @param slot The slot to connect to the signals.
     */
    void connectValueChanged( QList<QWidget *> widgets, const char *slot );

    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event ) override;

    QgsVectorLayer* mLayer;
    QgsStyleV2* mStyle;

    QgsRendererV2Widget* mActiveWidget;

    QgsPaintEffect* mPaintEffect;

    QgsMapCanvas* mMapCanvas;

    QgsFeatureRequest::OrderBy mOrderBy;

  private:
    bool mDockMode;
};


#endif
