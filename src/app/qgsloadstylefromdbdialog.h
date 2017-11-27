/***************************************************************************
    qgsloadstylefromdbdialog.h
    ---------------------
    begin                : April 2013
    copyright            : (C) 2013 by Emilio Loi
    email                : loi at faunalia dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOADFILEFROMDBDIALOG_H
#define QGSLOADFILEFROMDBDIALOG_H

#include "ui_qgsloadstylefromdbdialog.h"
#include "qgsguiutils.h"
#include "qgis_app.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

class APP_EXPORT QgsLoadStyleFromDBDialog : public QDialog, private Ui::QgsLoadStyleFromDBDialogLayout
{
    QString mSelectedStyleId;
    QString mSelectedStyleName;
    int mSectionLimit = 0;
    Q_OBJECT
  public:
    explicit QgsLoadStyleFromDBDialog( QWidget *parent = nullptr );

    ~QgsLoadStyleFromDBDialog();

    void initializeLists( const QStringList &ids, const QStringList &names, const QStringList &descriptions, int sectionLimit );
    QString getSelectedStyleId();
    void selectionChanged( QTableWidget *styleTable );
    void setLayer( QgsVectorLayer *l );

  public slots:
    void onRelatedTableSelectionChanged();
    void onOthersTableSelectionChanged();
    void deleteStyleFromDB();

  private:
    QgsVectorLayer *mLayer = nullptr;

};

#endif //QGSLOADFILEFROMDBDIALOG_H
