/***************************************************************************
                              qgsaddjoindialog.h
                              ------------------
  begin                : July 10, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADDJOINDIALOG_H
#define QGSADDJOINDIALOG_H

#include "ui_qgsaddjoindialogbase.h"
class QgsVectorLayer;

class APP_EXPORT QgsAddJoinDialog: public QDialog, private Ui::QgsAddJoinDialogBase
{
    Q_OBJECT
  public:
    QgsAddJoinDialog( QgsVectorLayer* layer, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsAddJoinDialog();

    //retrieve results

    /**Get the id of the layer to join*/
    QString joinedLayerId() const;
    /**Returns the name of the join field*/
    QString joinFieldName() const;
    /**Returns the name of the target field (join-to field)*/
    QString targetFieldName() const;
    /**True if joined layer should be cached in virtual memory*/
    bool cacheInMemory() const;
    /**Returns true if user wants to create an attribute index on the join field*/
    bool createAttributeIndex() const;
    /**True if onle a subset of fields of joined layer should be used*/
    bool hasJoinFieldsSubset() const;
    /**Return list of checked fields from joined layer to be used in join*/
    QStringList joinFieldsSubset() const;

  private slots:
    void on_mJoinLayerComboBox_currentIndexChanged( int index );
    void on_mUseJoinFieldsSubset_clicked();

  private:
    /**Target layer*/
    QgsVectorLayer* mLayer;
};


#endif // QGSADDJOINDIALOG_H
