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
#ifndef QGSATTRIBUTETYPELOADDIALOG_H
#define QGSATTRIBUTETYPELOADDIALOG_H

#include "ui_qgsattributeloadfrommap.h"

#include <vector>
#include "qgis_gui.h"

class QDialog;
class QLayout;
class QgsField;
class QgsMapCanvas;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsAttributeTypeLoadDialog
 */
class GUI_EXPORT QgsAttributeTypeLoadDialog : public QDialog, private Ui::QgsAttributeLoadValues
{
    Q_OBJECT

  public:
    QgsAttributeTypeLoadDialog( QgsVectorLayer *vl );

    /**
     * Overloaded accept method which will write the feature field
     * values, then delegate to QDialog::accept()
     */
    void accept() override;

    /**
     * Sets predefined vector layer for selection of data
     * \param layer Vector layer which is to be set as predefined one
     */
    void setVectorLayer( QgsVectorLayer *layer );

    /**
     * Returns the value map which is currently active.
     *
     * \returns value map of values selected from layer
     */
    QMap<QString, QVariant> &valueMap();

    /**
     * Returns TRUE if the "Add NULL value" checkbox has been checked.
     *
     * \returns TRUE if the "Add NULL value" checkbox has been checked.
     */
    bool insertNull();

  private slots:

    /**
     * Slot to react to button push or change of selected column for display of preview
     * \param fieldIndex indexOfChangedField
     * \param full flag if all values should be displayed or just preview of first 10
     */
    void createPreview( int fieldIndex, bool full = false );


    /**
     * Slot to react to value Preview button pushed
     */
    void previewButtonPushed();

  private:
    /**
     * Function to transfer data from layer to value map used in editing
     */
    void loadDataToValueMap();

    QgsVectorLayer *mLayer = nullptr;
    QMap<QString, QVariant> mValueMap;
};

#endif
