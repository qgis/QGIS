/***************************************************************************
    qgsvectorlayerloadstyle.h
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

#include "ui_qgsvectorlayerloadstyledialog.h"
#include "qgsguiutils.h"
#include "qgis_app.h"
#include "qgsvectorlayerproperties.h"
#include "qgsmaplayer.h"

class QgsMapLayerStyleCategoriesModel;

class APP_EXPORT QgsVectorLayerLoadStyleDialog : public QDialog, private Ui::QgsVectorLayerLoadStyleDialog
{
    Q_OBJECT
  public:
    explicit QgsVectorLayerLoadStyleDialog( QgsVectorLayer *layer, QWidget *parent = nullptr );

    ~QgsVectorLayerLoadStyleDialog() override;

    QgsMapLayer::StyleCategories styleCategories() const;

    QgsVectorLayerProperties::StyleType currentStyleType() const;

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

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsMapLayerStyleCategoriesModel *mModel;
    QString mSelectedStyleId;
    QString mSelectedStyleName;
    int mSectionLimit = 0;
    QPushButton *mDeleteButton = nullptr;
    QPushButton *mLoadButton = nullptr;
    QPushButton *mCancelButton = nullptr;
};

#endif //QGSLOADFILEFROMDBDIALOG_H
