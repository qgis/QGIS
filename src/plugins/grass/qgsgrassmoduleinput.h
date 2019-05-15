/***************************************************************************
                           qgsgrassmoduleinput.h
                             -------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSMODULEINPUT_H
#define QGSGRASSMODULEINPUT_H

#include <QAbstractProxyModel>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QFileSystemModel>
#include <QFileSystemWatcher>
#include <QGroupBox>
#include <QListView>
#include <QMap>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTreeView>

#include "qgis.h"

#include "qgsgrass.h"
#include "qgsgrassmodule.h"
#include "qgsgrassmoduleparam.h"
#include "qgsgrassplugin.h"
#include "qgsgrassprovider.h"
#include "qgsgrassvector.h"

extern "C"
{
#include <grass/vector.h>
}

class QgsGrassModuleInputModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    enum Role
    {
      MapRole = Qt::UserRole,
      MapsetRole = Qt::UserRole + 1,
      TypeRole = Qt::UserRole + 2 // QgsGrassObject::Type
    };

    explicit QgsGrassModuleInputModel( QObject *parent = nullptr );

    //! Gets singleton instance of this class.
    static QgsGrassModuleInputModel *instance();

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;


  public slots:
    //! Reload current mapset
    void reload();

    void onMapsetChanged();

    void onDirectoryChanged( const QString &path );
    void onFileChanged( const QString &path );
    void onMapsetSearchPathChanged();

  private:
    void addMapset( const QString &mapset );
    void refreshMapset( QStandardItem *mapsetItem, const QString &mapset, const QList<QgsGrassObject::Type> &types = QList<QgsGrassObject::Type>() );
    // Add to watched paths if exists and if not yet watched
    void watch( const QString &path );
    QString mLocationPath;
    // mapset watched dirs
    QStringList watchedDirs() { QStringList l; l << QStringLiteral( "cellhd" ) << QStringLiteral( "vector" ) << QStringLiteral( "tgis" ); return l; }
    // names of
    QStringList locationDirNames();
    QFileSystemWatcher *mWatcher = nullptr;

};

// Filter maps by type
class QgsGrassModuleInputProxy : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    explicit QgsGrassModuleInputProxy( QgsGrassModuleInputModel *sourceModel, QgsGrassObject::Type type, QObject *parent = nullptr );

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  private:
    QgsGrassModuleInputModel *mSourceModel = nullptr;
    QgsGrassObject::Type mType;
};

class QgsGrassModuleInputTreeView : public QTreeView
{
    Q_OBJECT
  public:
    explicit QgsGrassModuleInputTreeView( QWidget *parent = nullptr );

    void resetState();
};

class QgsGrassModuleInputPopup : public QTreeView
{
    Q_OBJECT
  public:
    explicit QgsGrassModuleInputPopup( QWidget *parent = nullptr );

    void setModel( QAbstractItemModel *model ) override;
};

// Flattens tree to list of maps for completer
class QgsGrassModuleInputCompleterProxy : public QAbstractProxyModel
{
    Q_OBJECT
  public:
    explicit QgsGrassModuleInputCompleterProxy( QObject *parent = nullptr );

    int columnCount( const QModelIndex &parent = QModelIndex() ) const override { Q_UNUSED( parent ) return 1; }
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;


    void setSourceModel( QAbstractItemModel *sourceModel ) override;

    QModelIndex mapFromSource( const QModelIndex &sourceIndex ) const override;
    QModelIndex mapToSource( const QModelIndex &proxyIndex ) const override;

  private:
    void refreshMapping();
    void map( const QModelIndex &parent, int level = 0 );
    QMap<int, QModelIndex> mIndexes;
    QMap<QModelIndex, int> mRows;
};

class QgsGrassModuleInputCompleter : public QCompleter
{
    Q_OBJECT

  public:
    explicit QgsGrassModuleInputCompleter( QAbstractItemModel *model, QWidget *parent = nullptr );

    bool eventFilter( QObject *watched, QEvent *event ) override;
};

class QgsGrassModuleInputComboBox : public QComboBox
{
    Q_OBJECT

  public:
    explicit QgsGrassModuleInputComboBox( QgsGrassObject::Type type, QWidget *parent = nullptr );

    bool eventFilter( QObject *watched, QEvent *event ) override;
    void showPopup() override;
    void hidePopup() override;

    // set current index
    void setCurrent( const QModelIndex &proxyIndex );

    // set current item to map/mapset if exists
    bool setCurrent( const QString &map, const QString &mapset = QString() );

    // set to first map if exists
    bool setFirst();

  protected:
    QgsGrassObject::Type mType;
    QgsGrassModuleInputModel *mModel = nullptr;
    QgsGrassModuleInputProxy *mProxy = nullptr;
    QgsGrassModuleInputTreeView *mTreeView = nullptr;
    // Skip next hidePopup
    bool mSkipHide;
};

class QgsGrassModuleInputSelectedDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    explicit QgsGrassModuleInputSelectedDelegate( QObject *parent = nullptr );

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

  public slots:
    void handlePressed( const QModelIndex &index );

  private:
    mutable QModelIndex mPressedIndex;
};

class QgsGrassModuleInputSelectedView : public QTreeView
{
    Q_OBJECT
  public:
    explicit QgsGrassModuleInputSelectedView( QWidget *parent = nullptr );

    void setModel( QAbstractItemModel *model ) override;

  signals:
    void deleteItem( const QModelIndex &index );

  protected:
    bool eventFilter( QObject *obj, QEvent *event ) override;

  private:
    QgsGrassModuleInputSelectedDelegate *mDelegate = nullptr;
};


/**
 * \class QgsGrassModuleInput
 *  \brief Class representing raster or vector module input
 */
class QgsGrassModuleInput : public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:

    /**
     * \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleInput( QgsGrassModule *module,
                         QgsGrassModuleStandardOptions *options, QString key,
                         QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                         bool direct, QWidget *parent = nullptr );

    //! Returns list of options which will be passed to module
    QStringList options() override;

    // ! Return vector of attribute fields of current vector
    QgsFields currentFields();

    //! Returns pointer to currently selected layer or null

    QgsGrassObject currentGrassObject();

    QString currentMap();

    QgsGrassVectorLayer *currentLayer();

    QStringList currentGeometryTypeNames();

    QString ready() override;

    //! Does this options causes use of region?
    //  Raster input/output uses region by default
    //  Use of region can be forced by 'region' attribute in qgm
    bool usesRegion() { return mUsesRegion; }

    //! Should be used region of this input
    bool useRegion();

    QgsGrassObject::Type type() { return mType; }

    void setGeometryTypeOption( const QString &optionName ) { mGeometryTypeOption = optionName; }
    QString geometryTypeOption() const { return mGeometryTypeOption; }

    // list of selected layers in <field>_<type> form, used by QgsGrassModuleSelection
    QStringList currentLayerCodes();

  public slots:
    void onActivated( const QString &text );

    void onChanged( const QString &text );

    void onLayerChanged();

    void deleteSelectedItem( const QModelIndex &index );

  signals:
    // emitted when value changed/selected
    void valueChanged();

  private:
    //! Input type
    QgsGrassObject::Type mType;

    // Module options
    QgsGrassModuleStandardOptions *mModuleStandardOptions = nullptr;

    //! Vector type mask read from option defined by "typeoption" tag, used for QGIS layers in combo
    //  + type mask defined in configuration file
    int mGeometryTypeMask;

    //! Name of vector type option associated with this input
    QString mGeometryTypeOption;

    //! Name of vector layer option associated with this input
    QString mVectorLayerOption;

    //! Model used in combo
    QgsGrassModuleInputModel *mModel = nullptr;

    //! Model containing currently selected maps
    QStandardItemModel *mSelectedModel = nullptr;

    //! Combo box with GRASS layers
    QgsGrassModuleInputComboBox *mComboBox = nullptr;

    //! Region button
    QPushButton *mRegionButton = nullptr;

    //! Vector sublayer label
    QLabel *mLayerLabel = nullptr;

    //! Vector sublayer combo
    QComboBox *mLayerComboBox = nullptr;

    //! List of multiple selected maps
    QgsGrassModuleInputSelectedView *mSelectedTreeView = nullptr;

    // Vector type checkboxes
    QMap<int, QCheckBox *> mTypeCheckBoxes;

    //! Optional map option id, if defined, only the layers from the
    //  map currently selected in that option are available.
    //  This is used by nodes layer option for networks.
    QString mMapId;

    // Currently selected vector
    QgsGrassVector *mVector = nullptr;

    // List of vector layers matching mGeometryTypes for currently selected vector
    QList<QgsGrassVectorLayer *> mLayers;

    //! The input map will be updated -> must be from current mapset
    // TODO
    bool mUpdate;

    //! Uses region
    bool mUsesRegion;

    //! Required field
    bool mRequired;
};


#endif // QGSGRASSMODULEINPUT_H
