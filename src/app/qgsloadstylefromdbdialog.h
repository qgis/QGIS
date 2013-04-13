/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADDATTRDIALOG_H
#define QGSADDATTRDIALOG_H

#include "ui_qgsloadstylefromdbdialog.h"
#include "qgisgui.h"
#include "qgsfield.h"

//class QgsVectorLayer;

class QgsLoadStyleFromDBDialog: public QDialog, private Ui::QgsLoadStyleFromDBDialogLayout
{
    Q_OBJECT
  public:
    explicit QgsLoadStyleFromDBDialog( QWidget *parent = 0 );
//    QgsAddAttrDialog( QgsVectorLayer *vlayer,
//                      QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
//    QgsAddAttrDialog( const std::list<QString>& typelist,
//                      QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );

//    QgsField field() const;

  public slots:
//    void on_mTypeBox_currentIndexChanged( int idx );
//    void on_mLength_editingFinished();
//    void accept();

  private:
//    QString mLayerType;

//    void setPrecisionMinMax();
};

#endif
