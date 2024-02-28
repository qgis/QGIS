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
 * and aspatial table designer dialog based on the connections API.
 *
 * It allows designing a new vector or aspatial database table by defining the schema
 * (if supported by the provider) and table name, the list of QgsFields,
 * the optional geometry type and SRID.
 *
 * The actual creation of the table is delegated to the connections API method
 * QgsAbstractDatabaseProviderConnection::createVectorTable()
 *
 * \ingroup gui
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsNewVectorTableDialog : public QDialog, private Ui_QgsNewVectorTableDialogBase
{
    Q_OBJECT
  public:

    /**
     * QgsNewVectorTableDialog constructor
     * \param conn DB connection, ownership is NOT transferred
     * \param parent optional parent
     */
    QgsNewVectorTableDialog( QgsAbstractDatabaseProviderConnection *conn, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the schema \a name
     */
    void setSchemaName( const QString &name );

    /**
     * Sets the table \a name
     */
    void setTableName( const QString &name );

    /**
     * Sets the geometry \a type
     */
    void setGeometryType( Qgis::WkbType type );

    /**
     * Sets the CRS to \a crs
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the CRS
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Returns the table name
     */
    QString tableName() const;

    /**
     * Returns the schema name
     */
    QString schemaName() const;

    /**
     * Returns the geometry column name
     */
    QString geometryColumnName() const;

    /**
     * Returns the fields
     */
    QgsFields fields() const;

    /**
     * Returns the geometry type
     */
    Qgis::WkbType geometryType() const;

    /**
     * Sets the fields to \a fields
     */
    void setFields( const QgsFields &fields );

    /**
     * Returns TRUE if spatialindex checkbox is checked
     * @return
     */
    bool createSpatialIndex();

    /**
     * Returns the validation errors or an empty list if the dialog is valid
     */
    QStringList validationErrors() const;

  private:

    QgsAbstractDatabaseProviderConnection *mConnection = nullptr;
    QgsNewVectorTableFieldModel *mFieldModel = nullptr;
    int mCurrentRow = -1;
    // Used by validator
    QStringList mTableNames;
    QStringList mValidationErrors;

    QSet<QString> mIllegalFieldNames;

    void updateButtons();
    void selectRow( int row );
    void validate();

    // QWidget interface
  protected:
    void showEvent( QShowEvent *event ) override;
};



/// @cond private

#ifndef SIP_RUN
class QgsNewVectorTableDialogFieldsDelegate: public QStyledItemDelegate
{
    Q_OBJECT
  public:

    QgsNewVectorTableDialogFieldsDelegate( const QList< QgsVectorDataProvider::NativeType> &typeList, QObject *parent = nullptr );

    // QAbstractItemDelegate interface
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

  public slots:

    void onFieldTypeChanged( int index );

  private:

    const QList< QgsVectorDataProvider::NativeType> mTypeList;

};


class QgsNewVectorTableFieldModel: public QgsFieldModel
{
    Q_OBJECT

  public:

    enum ColumnHeaders
    {
      Name,
      Type,
      ProviderType,
      Length,
      Precision,
      Comment
    };

    QgsNewVectorTableFieldModel( const QList< QgsVectorDataProvider::NativeType> &nativeTypes,  QObject *parent = nullptr );

    // QAbstractItemModel interface
    int columnCount( const QModelIndex & ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    QList<QgsVectorDataProvider::NativeType> nativeTypes() const;
    QgsVectorDataProvider::NativeType nativeType( const QString &typeName ) const;
    QgsVectorDataProvider::NativeType nativeType( int row ) const;

  private:

    const QList< QgsVectorDataProvider::NativeType> mNativeTypes;
    QString typeDesc( const QString &typeName ) const;
    QVariant::Type type( const QString &typeName ) const;

};


#endif

/// @endcond

#endif // QGSNEWVECTORTABLEDIALOG_H
