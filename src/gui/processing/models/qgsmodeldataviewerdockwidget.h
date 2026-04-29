/***************************************************************************
    qgsmodeldataviewerdockwidget.h
    ---------------------------------
    begin                : April 2026
    copyright            : (C) 2026 by Valentin Buira
    email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMODELDATAVIEWERDOCKWIDGET_H
#define QGSMODELDATAVIEWERDOCKWIDGET_H

#include "ui_qgsmodeldataviewerdockwidgetbase.h"

#include "qgsdockwidget.h"

#define SIP_NO_FILE

class QgsSpinBox;

/**
 * A dock widget which displays the output layer of a model child algorithm in a map canvas and an attribute table.
 *
 * \ingroup gui
 * \note Not available in Python bindings.
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsModelDataViewerDockWidget : public QgsDockWidget, private Ui::QgsModelDataViewerDockWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelDataViewerDockWidget()
     */
    QgsModelDataViewerDockWidget( QWidget *parent = nullptr, QgsMapLayer *layer = nullptr, QString childId = QString(), QString outputName = QString() );
    ~QgsModelDataViewerDockWidget() override;

    /**
     * Returns the child algorithm's unique ID string, used to identify
     * this child algorithm within its parent model.
     */
    QString childId() const { return mChildId; }

    /**
     * Returns the output name from the child algorithm with the unique id mChildId
     */
    QString outputName() const { return mOutputName; }

    /**
     * Sets the layer to be displayed in the data viewer.
     *
     * The layer is cloned, so ownership is not transferrer to the caller.
     */
    void setLayer( QgsMapLayer *layer );

  private:
    void loadAttributeTable();
    void attributeTableToggled( bool checked );
    void toggleProjectLayer( bool checked );

    void selectByExpression();
    void selectAll();
    void deselectAll();
    void invertSelection();
    void addLayerToMap();

    QgsMapLayer *mLayer = nullptr;

    QString mChildId;
    QString mOutputName;

    QgsMapTool *mToolPan = nullptr;
    QgsMapTool *mToolSelect = nullptr;

    QgsSpinBox *mMaxFeaturesSpinBox = nullptr;
};

#endif // QGSMODELDATAVIEWERDOCKWIDGET_H
