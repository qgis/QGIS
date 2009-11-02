/***************************************************************************
                         qgsnewvectorlayerdialog.h  -  description
                             -------------------
    begin                : October 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
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
#ifndef qgsnewvectorlayerdialog_H
#define qgsnewvectorlayerdialog_H

#include "ui_qgsnewvectorlayerdialogbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

#include "qgis.h"

class QgsNewVectorLayerDialog: public QDialog, private Ui::QgsNewVectorLayerDialogBase
{
    Q_OBJECT
  public:
    QgsNewVectorLayerDialog( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsNewVectorLayerDialog();
    /**Returns the selected geometry type*/
    QGis::WkbType selectedType() const;
    /**Appends the chosen attribute names and types to at*/
    void attributes( std::list<std::pair<QString, QString> >& at ) const;
    /**Returns the file format for storage*/
    QString selectedFileFormat() const;

  protected slots:
    void on_mAddAttributeButton_clicked();
    void on_mRemoveAttributeButton_clicked();
    void on_buttonBox_helpRequested();
    void on_mTypeBox_currentIndexChanged( int index );

  private:
    QPushButton * mOkButton;
    static const int context_id = 165149618;

};

#endif //qgsnewvectorlayerdialog_H
