/***************************************************************************
                         qgsattributetypeloaddialog.h
                             -------------------
    begin                : June 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf.kostej@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSATTRIBUTETYPELOADDIALOG_H
#define QGSATTRIBUTETYPELOADDIALOG_H

#include "ui_qgsattributeloadfrommap.h"

#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include <vector>

class QDialog;
class QLayout;
class QgsField;
class QgsMapCanvas;

class QgsAttributeTypeLoadDialog: public QDialog, private Ui::QgsAttributeLoadValues
{
    Q_OBJECT

  public:
    QgsAttributeTypeLoadDialog( QgsVectorLayer *vl );
    ~QgsAttributeTypeLoadDialog();

    /**
     * Overloaded accept method which will write the feature field
     * values, then delegate to QDialog::accept()
     */
    void accept();

    /**
     * Sets predefined vector layer for selection of data
     * @param layer Vector layer which is to be set as predefined one
     */
    void setVectorLayer( QgsVectorLayer *layer );

    /**
     * Getter to value map which is currently active
     * @return value map of vlues selected from layer
     */
    QMap<QString, QVariant> &valueMap();

  private slots:
    /**
     * Slot which reacts to change of selected layer to fill other two comboboxes with correct data
     * @param layerIndex index of layer which was selected
     */
    void fillComboBoxes( int layerIndex );

    /**
     * Slot to react to button push or change of selected column for display of preview
     * @param fieldIndex indexOfChangedField
     * @param full flag if all values should be displayed or just preview of first 10
     */
    void createPreview( int fieldIndex, bool full = false);


    /**
     * Slot to react to value Preview button pushed
     */
    void previewButtonPushed( );

  private:

    /**
     * Internal function to fill the list of layers
     */
    void fillLayerList();

    /**
     * Function to transfer data from layer to value map used in editing
     */
    void loadDataToValueMap();

    QgsVectorLayer *mLayer;
    int mIndex;


    QMap<QString, QVariant> mValueMap;
    QgsVectorLayer::EditType mEditType;
};

#endif
