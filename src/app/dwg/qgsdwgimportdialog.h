/***************************************************************************
                         qgsdwgimportdialog.h
                         --------------------
    begin                : May 2016
    copyright            : (C) 2016 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDWGIMPORTDIALOG_H
#define QGSDWGIMPORTDIALOG_H

#include "ui_qgsdwgimportbase.h"

class QgsVectorLayer;
class QgsLayerTreeGroup;

class QgsDwgImportDialog : public QDialog, private Ui::QgsDwgImportBase
{
    Q_OBJECT
  public:
    QgsDwgImportDialog( QWidget * parent = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsDwgImportDialog();

  private slots:
    void on_buttonBox_accepted();
    void on_pbBrowseDatabase_clicked();
    void on_pbBrowseDrawing_clicked();
    void on_pbImportDrawing_clicked();
    void on_pbLoadDatabase_clicked();
    void on_pbSelectAll_clicked();
    void on_pbDeselectAll_clicked();
    void on_leDatabase_textChanged( const QString &text );
    void on_leLayerGroup_textChanged( const QString &text );

  private:
    QgsVectorLayer *layer( QgsLayerTreeGroup *layerGroup, QString layer, QString table );
    void createGroup( QgsLayerTreeGroup *group, QString name, QStringList layers, bool visible );
    void updateUI();
    void expandInserts();
    void updateCheckState( Qt::CheckState state );
};

#endif // QGSDWGIMPORTDIALOG_H
