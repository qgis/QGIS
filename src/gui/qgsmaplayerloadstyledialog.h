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
#include "qgsguiutils.h"
#include "qgis_gui.h"
#include "qgsvectorlayerproperties.h"
#include "qgsmaplayer.h"

class QgsMapLayerStyleCategoriesModel;

class GUI_EXPORT QgsMapLayerLoadStyleDialog : public QDialog, private Ui::QgsVectorLayerLoadStyleDialog
{
    Q_OBJECT
  public:
    explicit QgsMapLayerLoadStyleDialog( QgsMapLayer *layer, QWidget *parent = nullptr );

    QgsMapLayer::StyleCategories styleCategories() const;

    QgsVectorLayerProperties::StyleType currentStyleType() const;

    QString fileExtension() const;

    QString filePath() const;

    void initializeLists( const QStringList &ids, const QStringList &names, const QStringList &descriptions, int sectionLimit );
    QString selectedStyleId();
    void selectionChanged( QTableWidget *styleTable );

  public slots:
    void accept() override;

  private slots:
    void updateLoadButtonState();
    void onRelatedTableSelectionChanged();
    void onOthersTableSelectionChanged();
    void deleteStyleFromDB();
    void showHelp();

  private:
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
