/***************************************************************************
     qgseditreservedwordsdialog.h
     --------------------------------------
    Date                 : Sun Sep 16 12:13:24 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEDITRESERVEDWORDSDIALOG_H
#define QGSEDITRESERVEDWORDSDIALOG_H

// $Id$

#include "ui_qgseditreservedwordsbase.h"
#include "qgisgui.h"
class QgsEditReservedWordsDialog : public QDialog, private Ui::QgsEditReservedWordsBase
{
    Q_OBJECT
public:
    QgsEditReservedWordsDialog(QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags);
    ~QgsEditReservedWordsDialog();
    void addColumn(QString column, bool isReserved, int index);
    void setReservedWords(const QStringList &);
    QStringList columnNames();
    //! Set the description displayed in the dialog
    void setDescription(const QString &description);

private:
    void checkWord(QTableWidgetItem *);

    static const int context_id = 0;

private slots:
    void on_buttonBox_accepted() { accept(); }
    void on_buttonBox_rejected() { reject(); }
    void on_buttonBox_helpRequested();
    void on_lvColumns_itemChanged(QTableWidgetItem* item) { checkWord(item); }
    void on_lvColumns_itemClicked(QTableWidgetItem* item) 
      { lvColumns->editItem(item); }

};
#endif //QGSEDITRESERVEDWORDSDIALOG_H
