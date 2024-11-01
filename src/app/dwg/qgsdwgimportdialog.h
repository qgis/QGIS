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
#include "qgis_app.h"

class QgsVectorLayer;
class QgsLayerTreeGroup;
class QgsMapToolPan;

class APP_EXPORT QgsDwgImportDialog : public QDialog, private Ui::QgsDwgImportBase
{
    Q_OBJECT
  public:
    QgsDwgImportDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
    ~QgsDwgImportDialog() override;

  private slots:
    void buttonBox_accepted();
    void pbImportDrawing_clicked();
    void pbLoadDatabase_clicked();
    void pbSelectAll_clicked();
    void pbDeselectAll_clicked();
    void mDatabaseFileWidget_textChanged( const QString &filename );
    void drawingFileWidgetFileChanged( const QString &filename );
    void leLayerGroup_textChanged( const QString &text );
    void showHelp();
    void layersClicked( QTableWidgetItem *item );
    void blockModeCurrentIndexChanged();
    void useCurvesClicked();

  private:
    enum class ColumnIndex : int
    {
      Name = 0,
      Visibility = 1
    };

    enum BlockImportFlag
    {
      BlockImportExpandGeometry = 1 << 0,
      BlockImportAddInsertPoints = 1 << 1
    };
    Q_DECLARE_FLAGS( BlockImportFlags, BlockImportFlag )

    QgsVectorLayer *createLayer( const QString &layer, const QString &table );
    QList<QgsVectorLayer *> createLayers( const QStringList &layerNames );
    void createGroup( QgsLayerTreeGroup *group, const QString &name, const QStringList &layers, bool visible );
    void updateUI();
    void expandInserts();
    void updateCheckState( Qt::CheckState state );

    QgsMapToolPan *mPanTool = nullptr;
    QList<QgsVectorLayer *> mPreviewLayers;

    friend class TestQgsDwgImportDialog;
};

#endif // QGSDWGIMPORTDIALOG_H
