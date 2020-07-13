/***************************************************************************
  qgsnewvectortabledialog.h - QgsNewVectorTableDialog

 ---------------------
 begin                : 12.7.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNEWVECTORTABLEDIALOG_H
#define QGSNEWVECTORTABLEDIALOG_H

#include <QStyledItemDelegate>

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsfields.h"
#include "qgswkbtypes.h"
#include "qgsvectordataprovider.h"
#include "qgsfieldmodel.h"
#include "qgsabstractdatabaseproviderconnection.h"

#include "ui_qgsnewvectortabledialogbase.h"

class QgsNewVectorTableFieldModel;

/**
 * The QgsNewVectorTableDialog class is a provider-agnostic database vector
 * and aspatial table designer dialog.
 *
 * It allows to design a new vector or aspatial database table by defining the schema
 * (if supported by the provider) and table names, the list of QgsFields,
 * the geometry type and the SRID (if the table is not aspatial).
 *
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsNewVectorTableDialog : public QDialog, private Ui_QgsNewVectorTableDialogBase
{
  public:

    /**
     * QgsNewVectorTableDialog constructor
     * \param connection DB connection, ownership is NOT transfered
     * \param parent optional parent
     */
    QgsNewVectorTableDialog( QgsAbstractDatabaseProviderConnection *conn, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QString tableName() const;

    QString schemaName() const;

    QgsFields fields() const;

    QgsWkbTypes::Type geometryType() const;

    void setFields( const QgsFields &fields );

  private:

    // In
    QgsAbstractDatabaseProviderConnection *mConnection;

    // Out
    QString mTableName;
    QString mSchemaName;
    // Default to aspatial
    QgsWkbTypes::Type mGeometryType = QgsWkbTypes::Type::NoGeometry;

    // Internal
    QgsNewVectorTableFieldModel *mFieldModel;
    int mCurrentRow = -1;

    void updateButtons();
    void selectRow( int row );


};

/// @cond private

#ifndef SIP_RUN
class QgsNewVectorTableDialogFieldsDelegate: public QStyledItemDelegate
{
  public:

    QgsNewVectorTableDialogFieldsDelegate( const QList< QgsVectorDataProvider::NativeType> &typeList, QObject *parent = nullptr );

    // QAbstractItemDelegate interface
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

  private:

    const QList< QgsVectorDataProvider::NativeType> mTypeList;

};


class QgsNewVectorTableFieldModel: public QgsFieldModel
{

  public:

    enum ColumnHeaders
    {
      Name,
      Type,
      Comment
    };

    QgsNewVectorTableFieldModel( const QList< QgsVectorDataProvider::NativeType> &typeList,  QObject *parent = nullptr );

    // QAbstractItemModel interface
    int columnCount( const QModelIndex & ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    QList<QgsVectorDataProvider::NativeType> typeList() const;

  private:

    const QList< QgsVectorDataProvider::NativeType> mTypeList;

    QString typeDesc( const QString &typeName ) const;
    QVariant::Type type( const QString &typeName ) const;

};




#endif

/// @endcond

#endif // QGSNEWVECTORTABLEDIALOG_H
