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
#include "qgshelp.h"

class QgsVectorLayer;
class QgsLayerTreeGroup;

class QgsDwgImportDialog : public QDialog, private Ui::QgsDwgImportBase
{
    Q_OBJECT
  public:
    QgsDwgImportDialog( QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsDwgImportDialog();

  private slots:
    void buttonBox_accepted();
    void pbBrowseDatabase_clicked();
    void pbBrowseDrawing_clicked();
    void pbImportDrawing_clicked();
    void pbLoadDatabase_clicked();
    void pbSelectAll_clicked();
    void pbDeselectAll_clicked();
    void leDatabase_textChanged( const QString &text );
    void leLayerGroup_textChanged( const QString &text );
    void showHelp();

  private:
    QgsVectorLayer *layer( QgsLayerTreeGroup *layerGroup, const QString &layer, const QString &table );
    void createGroup( QgsLayerTreeGroup *group, const QString &name, const QStringList &layers, bool visible );
    void updateUI();
    void expandInserts();
    void updateCheckState( Qt::CheckState state );
};

#endif // QGSDWGIMPORTDIALOG_H
