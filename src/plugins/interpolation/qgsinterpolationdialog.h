/***************************************************************************
                              qgsinterpolationdialog.h
                              ------------------------
  begin                : March 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSINTERPOLATIONDIALOG_H
#define QGSINTERPOLATIONDIALOG_H

#include "ui_qgsinterpolationdialogbase.h"
#include "qgsrectangle.h"
#include "qgisinterface.h"
#include <QFileInfo>

class QgsInterpolatorDialog;
class QgsVectorLayer;

class QgsInterpolationDialog: public QDialog, private Ui::QgsInterpolationDialogBase
{
    Q_OBJECT
  public:
    QgsInterpolationDialog( QWidget* parent, QgisInterface* iface );
    ~QgsInterpolationDialog();

  private slots:

    void on_buttonBox_accepted();
    void on_mInputLayerComboBox_currentIndexChanged( const QString& text );
    void on_mOutputFileButton_clicked();
    void on_mConfigureInterpolationButton_clicked();
    void on_mInterpolationMethodComboBox_currentIndexChanged( const QString &text );
    void on_mAddPushButton_clicked();
    void on_mRemovePushButton_clicked();

    void on_mNumberOfColumnsSpinBox_valueChanged( int value );
    void on_mNumberOfRowsSpinBox_valueChanged( int value );
    void on_mCellsizeXSpinBox_valueChanged( double value );
    void on_mCellSizeYSpinBox_valueChanged( double value );
    void on_mBBoxToCurrentExtent_clicked();

    void on_mXMinLineEdit_textEdited( const QString& text );
    void on_mXMaxLineEdit_textEdited( const QString& text );
    void on_mYMinLineEdit_textEdited( const QString& text );
    void on_mYMaxLineEdit_textEdited( const QString& text );



  private:
    QgisInterface* mIface;
    /**Dialog to get input for the current interpolation method*/
    QgsInterpolatorDialog* mInterpolatorDialog;

    /**Returns the vector layer object with the given name
     Returns a pointer to the vector layer or 0 in case of error.*/
    QgsVectorLayer* vectorLayerFromName( const QString& name );
    /**Enables or disables the Ok button depending on the availability of input layers and the output file*/
    void enableOrDisableOkButton();
    /**Get the current output bounding box (might be different to the compound layers bounding box because of user edits)
      @return the bounding box or an empty bounding box in case of error*/
    QgsRectangle currentBoundingBox();
    /**Returns the compound bounding box of the inserted layers*/
    QgsRectangle boundingBoxOfLayers();
    /**Inserts the compound bounding box of the input layers into the line edits for the output bounding box*/
    void setLayersBoundingBox();
    /**Set cellsizes according to nex bounding box and number of columns / rows */
    void setNewCellsizeOnBoundingBoxChange();
    void setNewCellsizeXOnNColumnsChange();
    void setNewCellsizeYOnNRowschange();
    void setNColsOnCellsizeXChange();
    void setNRowsOnCellsizeYChange();
};

#endif
