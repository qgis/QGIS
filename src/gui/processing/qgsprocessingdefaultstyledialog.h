/***************************************************************************
                             qgsprocessingdefaultstyledialog.h
                             ----------------------------------
    Date                 : September 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGDEFAULTSTYLEDIALOG_H
#define QGSPROCESSINGDEFAULTSTYLEDIALOG_H

#include "ui_qgsprocessingdefaultstylesdialogbase.h"

#include "qgis.h"
#include "qgis_gui.h"

#include <QDialog>
#include <QStyledItemDelegate>

#define SIP_NO_FILE

class QgsProcessingAlgorithm;

///@cond PRIVATE

class QgsProcessingDefaultStyleDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsProcessingDefaultStyleDelegate( QObject *parent = nullptr );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;

    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};


class QgsProcessingDefaultStylesModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    explicit QgsProcessingDefaultStylesModel( const QgsProcessingAlgorithm *algorithm, QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    void saveStyles();

  private:
    enum Column
    {
      OutputName = 0,
      StylePath = 1
    };

    struct OutputItem
    {
        QString name;
        QString description;
        QString stylePath;
    };

    QString mAlgorithmId;
    QVector<OutputItem> mItems;
};
///@endcond

/**
 * \ingroup gui
 * \brief A dialog for configuring default styles for Processing outputs.
 * \note Not available in Python bindings.
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsProcessingDefaultStyleDialog : public QDialog, private Ui::QgsProcessingDefaultStylesDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingDefaultStyleDialog.
     */
    QgsProcessingDefaultStyleDialog( const QgsProcessingAlgorithm *algorithm, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void accept() override;

  protected:
    void showEvent( QShowEvent *event ) override;

  private:
    const QgsProcessingAlgorithm *mAlgorithm = nullptr;
    QgsProcessingDefaultStylesModel *mModel = nullptr;
    QgsProcessingDefaultStyleDelegate *mDelegate = nullptr;
    bool mInitialWidthsSet = false;
};

#endif // QGSPROCESSINGDEFAULTSTYLEDIALOG_H
