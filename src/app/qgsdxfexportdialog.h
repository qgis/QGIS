/***************************************************************************
                         qgsdxfexportdialog.h
                         --------------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFEXPORTDIALOG_H
#define QGSDXFEXPORTDIALOG_H

#include "ui_qgsdxfexportdialogbase.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeview.h"
#include "qgsdxfexport.h"
#include "qgssettingstree.h"
#include "qgssettingsentryimpl.h"

#include <QList>
#include <QPair>
#include <QSet>
#include <QItemDelegate>

class QgsLayerTreeGroup;
class QgsLayerTreeNode;

class FieldSelectorDelegate : public QItemDelegate
{
    Q_OBJECT
  public:
    explicit FieldSelectorDelegate( QObject *parent = nullptr );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
  private:
    QgsVectorLayer *indexToLayer( const QAbstractItemModel *model, const QModelIndex &index ) const;
    int attributeIndex( const QAbstractItemModel *model, const QgsVectorLayer *vl ) const;
};

class QgsVectorLayerAndAttributeModel : public QgsLayerTreeModel
{
    Q_OBJECT
  public:
    QgsVectorLayerAndAttributeModel( QgsLayerTree *rootNode, QObject *parent = nullptr );

    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    QList< QgsDxfExport::DxfLayer > layers() const;

    void saveLayersOutputAttribute( QgsLayerTreeNode *node );
    void loadLayersOutputAttribute( QgsLayerTreeNode *node );

    QgsVectorLayer *vectorLayer( const QModelIndex &index ) const;
    int attributeIndex( const QgsVectorLayer *vl ) const;

    void applyVisibilityPreset( const QString &name );

    void selectAll();
    void deSelectAll();
    void selectDataDefinedBlocks();
    void deselectDataDefinedBlocks();

  private:
    QHash<const QgsVectorLayer *, int> mAttributeIdx;
    QHash<const QgsVectorLayer *, bool> mCreateDDBlockInfo;
    QHash<const QgsVectorLayer *, int>  mDDBlocksMaxNumberOfClasses;
    QSet<QModelIndex> mCheckedLeafs;

    void applyVisibility( QSet<QString> &visibleLayers, QgsLayerTreeNode *node );
    void retrieveAllLayers( QgsLayerTreeNode *node, QSet<QString> &layers );
    void enableDataDefinedBlocks( bool enabled );
};

class QgsDxfExportLayerTreeView : public QgsLayerTreeView
{
    Q_OBJECT
  public:
    explicit QgsDxfExportLayerTreeView( QWidget *parent = nullptr );

  protected:
    void resizeEvent( QResizeEvent *event ) override;
};

class QgsDxfExportDialog : public QDialog, private Ui::QgsDxfExportDialogBase
{
    Q_OBJECT
  public:
    static inline QgsSettingsTreeNode *sTreeAppDdxf = QgsSettingsTree::sTreeApp->createChildNode( QStringLiteral( "dxf" ) );
    static const inline QgsSettingsEntryBool *settingsDxfEnableDDBlocks = new QgsSettingsEntryBool( QStringLiteral( "enable-datadefined-blocks" ), sTreeAppDdxf,  false );

    QgsDxfExportDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
    ~QgsDxfExportDialog() override;

    QList< QgsDxfExport::DxfLayer > layers() const;

    double symbologyScale() const;
    Qgis::FeatureSymbologyExport symbologyMode() const;
    QString saveFile() const;
    bool exportMapExtent() const;
    bool selectedFeaturesOnly() const;
    bool layerTitleAsName() const;
    bool force2d() const;
    bool useMText() const;
    QString mapTheme() const;
    QString encoding() const;
    QgsCoordinateReferenceSystem crs() const;

  public slots:
    //! Change the selection of layers in the list
    void selectAll();
    void deSelectAll();
    void selectDataDefinedBlocks();
    void deselectDataDefinedBlocks();

  private slots:
    void setOkEnabled();
    void saveSettings();
    void mVisibilityPresets_currentIndexChanged( int index );
    void mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs );
    void showHelp();

  private:
    void cleanGroup( QgsLayerTreeNode *node );
    QgsLayerTree *mLayerTreeGroup = nullptr;
    FieldSelectorDelegate *mFieldSelectorDelegate = nullptr;
    QgsVectorLayerAndAttributeModel *mModel = nullptr;
    QgsDxfExportLayerTreeView *mTreeView = nullptr;

    QgsCoordinateReferenceSystem mCRS;
};

#endif // QGSDXFEXPORTDIALOG_H
