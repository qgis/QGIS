/***************************************************************************
    qgsmaplayerloadstyledialog.h
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

#ifndef QGSMAPLAYERLOADSTYLEDIALOG_H
#define QGSMAPLAYERLOADSTYLEDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsvectorlayerloadstyledialog.h"
#include "qgis_gui.h"
#include "qgsvectorlayerproperties.h"
#include "qgsmaplayer.h"

class QgsMapLayerStyleCategoriesModel;

/**
 * \ingroup gui
 * \brief A reusable dialog which allows users to select stored layer styles and categories to load
 * for a map layer.
 *
 * Currently supports
 *
 * - vector layers
 * - vector tile layers
 *
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsMapLayerLoadStyleDialog : public QDialog, private Ui::QgsVectorLayerLoadStyleDialog
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsMapLayerLoadStyleDialog, associated with the specified map \a layer.
     */
    explicit QgsMapLayerLoadStyleDialog( QgsMapLayer *layer, QWidget *parent = nullptr );

    /**
     * Returns the list of selected style categories the user has opted to load.
     */
    QgsMapLayer::StyleCategories styleCategories() const;

    /**
     * Returns the selected style type.
     */
    QgsLayerPropertiesDialog::StyleType currentStyleType() const;

    /**
     * Returns the file extension for the selected layer style source file.
     *
     * \see filePath()
     */
    QString fileExtension() const;

    /**
     * Returns the full path to the selected layer style source file.
     *
     * \see fileExtension()
     */
    QString filePath() const;

    /**
     * Initialize list of database stored styles.
     */
    void initializeLists( const QStringList &ids, const QStringList &names, const QStringList &descriptions, int sectionLimit );

    /**
     * Returns the ID of the selected database stored style.
     */
    QString selectedStyleId();

  public slots:
    void accept() override;

  private slots:
    void updateLoadButtonState();
    void onRelatedTableSelectionChanged();
    void onOthersTableSelectionChanged();
    void deleteStyleFromDB();
    void showHelp();
    void selectAll();
    void deselectAll();
    void invertSelection();

  private:
    void selectionChanged( QTableWidget *styleTable );

    QgsMapLayer *mLayer = nullptr;
    QgsMapLayerStyleCategoriesModel *mModel;
    QString mSelectedStyleId;
    QString mSelectedStyleName;
    int mSectionLimit = 0;
    QPushButton *mDeleteButton = nullptr;
    QPushButton *mLoadButton = nullptr;
    QPushButton *mCancelButton = nullptr;
};

#endif //QGSMAPLAYERLOADSTYLEDIALOG_H
