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

class GUI_EXPORT QgsRendererV2PropertiesDialog : public QDialog, private Ui::QgsRendererV2PropsDialogBase
{
    Q_OBJECT

  public:
    QgsRendererV2PropertiesDialog( QgsVectorLayer* layer, QgsStyleV2* style, bool embedded = false );
    ~QgsRendererV2PropertiesDialog();

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

  signals:
    /**
     * Emitted when expression context variables on the associated
     * vector layers have been changed. Will request the parent dialog
     * to re-synchronize with the variables.
     */
    void layerVariablesChanged();

    /**
     * Emmited when something on the widget has changed.
     * All widgets will fire this event to notify of an internal change.
     */
    void widgetChanged();

  public slots:
    //! called when user changes renderer type
    void rendererChanged();

    //! Apply the changes from the dialog to the layer.
    void apply();

    //! Apply and accept the changes for the dialog.
    void onOK();

  private slots:
    void showOrderByDialog();

    void changeOrderBy( const QgsFeatureRequest::OrderBy& orderBy, bool orderByEnabled );

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
};


#endif
