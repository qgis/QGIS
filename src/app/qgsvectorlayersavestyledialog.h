/***************************************************************************
  qgsvectorlayersavestyledialog.h
  --------------------------------------
  Date                 : September 2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERSAVESTYLEDIALOG_H
#define QGSVECTORLAYERSAVESTYLEDIALOG_H

#include <QDialog>
#include "ui_qgsvectorlayersavestyledialog.h"
#include "qgsvectorlayerproperties.h"

class QgsVectorLayer;
class QgsMapLayerStyleCategoriesModel;


class QgsVectorLayerSaveStyleDialog : public QDialog, private Ui::QgsVectorLayerSaveStyleDialog
{
    Q_OBJECT

  public:

    struct SaveToDbSettings
    {
      public:
        QString uiFileContent;
        QString name;
        QString description;
        bool isDefault;
    };

    explicit QgsVectorLayerSaveStyleDialog( QgsVectorLayer *layer, QWidget *parent = nullptr );
    ~QgsVectorLayerSaveStyleDialog() override;

    SaveToDbSettings saveToDbSettings() const;
    QString outputFilePath() const;
    QgsMapLayer::StyleCategories styleCategories() const;

    QgsVectorLayerProperties::StyleType currentStyleType() const;

  public slots:
    void accept() override;

  private slots:
    void updateSaveButtonState();
    void showHelp();
    void readUiFileContent( const QString &filePath );

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsMapLayerStyleCategoriesModel *mModel;
    QString mUiFileContent;
};

#endif // QGSVECTORLAYERSAVESTYLE_H
