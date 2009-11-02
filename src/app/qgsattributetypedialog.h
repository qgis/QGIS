/***************************************************************************
                         qgsattributetypedialog.h  -  description
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
#ifndef QGSATTRIBUTETYPEDIALOG_H
#define QGSATTRIBUTETYPEDIALOG_H

#include "ui_qgsattributetypeedit.h"

#include "qgsvectorlayer.h"

class QDialog;
class QLayout;
class QgsField;

class QgsAttributeTypeDialog: public QDialog, private Ui::QgsAttributeTypeDialog
{
    Q_OBJECT

  public:
    QgsAttributeTypeDialog( QgsVectorLayer *vl );
    ~QgsAttributeTypeDialog();

    /**
     * Overloaded accept method which will write the feature field
     * values, then delegate to QDialog::accept()
     */
    void accept();

    /**
     * Setting index, which page should be selected
     * @param index of page to be selected
     * @param editTypeInt type of edit type which was selected before save
     */
    void setIndex( int index, int editTypeInt = -1 );

    /**
     * Setting page which is to be selected
     * @param index index of page which was selected
     */
    void setPage( int index );

    /**
     * Getter to get selected edit type
     * @return selected edit type
     */
    QgsVectorLayer::EditType editType();

    /**
     * Setter to value map variable to display actual value
     * @param valueMap map which is to be dispayed in this dialog
     */
    void setValueMap( QMap<QString, QVariant> valueMap );

    /**
     * Setter to range for to be displayed and edited in this dialog
     * @param rangeData rande data which is to be displayed
     */
    void setRange( QgsVectorLayer::RangeData rangeData );

    /**
     * Getter for value map after editing
     * @return map which is to be returned
     */
    QMap<QString, QVariant> &valueMap();

    /**
     * Getter for range data
     * @return range data after editing
     */
    QgsVectorLayer::RangeData rangeData();

  private slots:
    /**
     * Slot to handle change of index in combobox to select correct page
     * @param index index of value in combobox
     */
    void setStackPage( int index );

    /**
     * Slot to handle button push to delete selected rows
     */
    void removeSelectedButtonPushed( );

    /**
     * Slot to handle load from layer button pushed to display dialog to load data
     */
    void loadFromLayerButtonPushed( );

    /**
     * Slot to handle load from CSV button pushed to display dialog to load data
     */
    void loadFromCSVButtonPushed( );

    /**
     * Slot to handle change of cell to have always empty row at end
     * @param row index of row which was changed
     * @param column index of column which was changed
     */
    void vCellChanged( int row, int column );

  private:

    QString defaultWindowTitle();

    /**
     * Function to set page index
     * @param index index of page to be changed
     */
    void setPageForIndex( int index );

    /**
     * Function to set page according to edit type
     * @param editType edit type to set page
     */
    void setPageForEditType( QgsVectorLayer::EditType editType );

    /**
     * Function to update the value map
     * @param map new map
     */
    void updateMap( const QMap<QString, QVariant> &map );


    QMap<QString, QVariant> mValueMap;

    QgsVectorLayer *mLayer;
    int mIndex;

    QgsVectorLayer::RangeData mRangeData;
    QgsVectorLayer::EditType mEditType;
};

#endif
