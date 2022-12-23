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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QDialog>
#include "ui_qgsvectorlayersavestyledialog.h"
#include "qgsvectorlayerproperties.h"
#include "qgis_gui.h"

class QgsVectorLayer;
class QgsMapLayerStyleCategoriesModel;

/**
 * \ingroup gui
 * \class QgsVectorLayerSaveStyleDialog
 *
 * \brief The QgsVectorLayerSaveStyleDialog class provides the UI to save the current style
 * or multiple styles into different storage containers (QML, SLD and DB).
 * The user can select what categories must be saved.
 */
class GUI_EXPORT QgsVectorLayerSaveStyleDialog : public QDialog, private Ui::QgsVectorLayerSaveStyleDialog
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

    SaveToDbSettings saveToDbSettings() const;
    QString outputFilePath() const;
    QgsMapLayer::StyleCategories styleCategories() const;

    QgsVectorLayerProperties::StyleType currentStyleType() const;

    bool saveOnlyCurrentStyle() const;
    void setSaveOnlyCurrentStyle( bool saveCurrentStyle );

    const QListWidget *stylesWidget( );

  public slots:
    void accept() override;

  private slots:
    void updateSaveButtonState();
    void showHelp();
    void readUiFileContent( const QString &filePath );

  private:
    void setupMultipleStyles();
    void populateStyleComboBox();
    QgsVectorLayer *mLayer = nullptr;
    QgsMapLayerStyleCategoriesModel *mModel;
    QString mUiFileContent;
    bool mSaveOnlyCurrentStyle = true;
};

#endif // QGSVECTORLAYERSAVESTYLE_H
