/***************************************************************************
  qgsmaplayersavestyledialog.h
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

#ifndef QGSMAPLAYERSAVESTYLEDIALOG_H
#define QGSMAPLAYERSAVESTYLEDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QDialog>
#include "ui_qgsmaplayersavestyledialog.h"
#include "qgsmaplayer.h"
#include "qgslayerpropertiesdialog.h"
#include "qgis_gui.h"

class QgsMapLayerStyleCategoriesModel;

/**
 * \ingroup gui
 * \class QgsMapLayerSaveStyleDialog
 *
 * \brief The QgsMapLayerSaveStyleDialog class provides the UI to save the current style
 * or multiple styles into different storage containers (QML, SLD and DB).
 * The user can select what categories must be saved.
 *
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsMapLayerSaveStyleDialog : public QDialog, private Ui::QgsMapLayerSaveStyleDialog
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

    /**
     * Constructor
     */
    explicit QgsMapLayerSaveStyleDialog( QgsMapLayer *layer, QWidget *parent = nullptr );

    /**
     * Returns the database settings for saving the style in the DB.
     */
    SaveToDbSettings saveToDbSettings() const;

    /**
     * Returns the selected file output path.
     */
    QString outputFilePath() const;

    /**
     * Returns the available style categories.
     */
    QgsMapLayer::StyleCategories styleCategories() const;

    /**
     * Returns the selected style storage type.
     */
    QgsLayerPropertiesDialog::StyleType currentStyleType() const;

    /**
     * Returns whether the user only allowed to save the current style.
     *
     * \see setSaveOnlyCurrentStyle()
     */
    bool saveOnlyCurrentStyle() const;

    /**
     * Sets whether the user only allowed to save the current style.
     *
     * \see saveOnlyCurrentStyle()
     */
    void setSaveOnlyCurrentStyle( bool saveCurrentStyle );

    /**
     * Returns the styles list widget.
     */
    const QListWidget *stylesWidget();

    /**
     * Returns the SLD export options.
     * \since QGIS 3.30
     */
    Qgis::SldExportOptions sldExportOptions() const;

  public slots:
    void accept() override;

  private slots:
    void updateSaveButtonState();
    void showHelp();
    void readUiFileContent( const QString &filePath );
    void selectAll();
    void deselectAll();
    void invertSelection();

  private:
    void setupMultipleStyles();
    void populateStyleComboBox();
    QgsMapLayer *mLayer = nullptr;
    QgsMapLayerStyleCategoriesModel *mModel;
    QString mUiFileContent;
    bool mSaveOnlyCurrentStyle = true;
};

#endif // QGSMAPLAYERSAVESTYLEDIALOG_H
