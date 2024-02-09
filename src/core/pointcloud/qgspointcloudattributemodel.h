/***************************************************************************
       qgspointcloudattributemodel.h
                         ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDATTRIBUTEMODEL_H
#define QGSPOINTCLOUDATTRIBUTEMODEL_H

#include "qgspointcloudattribute.h"

#include <QAbstractItemModel>
#include <QPointer>
#include <QSortFilterProxyModel>

class QgsPointCloudLayer;

/**
 * \ingroup core
 *
 * \brief A model for display of available attributes from a point cloud.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudAttributeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    // *INDENT-OFF*

     /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsPointCloudAttributeModel::FieldRoles
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPointCloudAttributeModel, FieldRoles ) : int
    {
      AttributeName SIP_MONKEYPATCH_COMPAT_NAME(AttributeNameRole)= Qt::UserRole + 1,  //!< Attribute name
      AttributeIndex SIP_MONKEYPATCH_COMPAT_NAME(AttributeIndexRole), //!< Attribute index if index corresponds to an attribute
      AttributeSize SIP_MONKEYPATCH_COMPAT_NAME(AttributeSizeRole), //!< Attribute size
      AttributeType SIP_MONKEYPATCH_COMPAT_NAME(AttributeTypeRole), //!< Attribute type, see QgsPointCloudAttribute::DataType
      IsEmpty SIP_MONKEYPATCH_COMPAT_NAME(IsEmptyRole), //!< TRUE if the index corresponds to the empty value
      IsNumeric SIP_MONKEYPATCH_COMPAT_NAME(IsNumericRole), //!< TRUE if the index corresponds to a numeric attributre
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsPointCloudAttributeModel, with the specified \a parent object.
     */
    explicit QgsPointCloudAttributeModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the \a layer associated with the model.
     *
     * \see setAttributes()
     */
    void setLayer( QgsPointCloudLayer *layer );

    /**
     * Returns the layer associated with the model.
     *
     * \see setLayer()
     */
    QgsPointCloudLayer *layer();

    /**
     * Sets the \a attributes to include in the model.
     *
     * \see setLayer()
     * \see attributes()
     */
    void setAttributes( const QgsPointCloudAttributeCollection &attributes );

    /**
     * Returns the attributes associated with the model.
     *
     * \see setAttributes()
     */
    QgsPointCloudAttributeCollection attributes() const { return mAttributes; }

    /**
     * Sets whether an optional empty attribute ("not set") option is present in the model.
     * \see allowEmptyAttributeName()
     */
    void setAllowEmptyAttributeName( bool allowEmpty );

    /**
     * Returns the index corresponding to a given attribute \a name.
     */
    QModelIndex indexFromName( const QString &name );

    /**
     * Returns TRUE if the model allows the empty attribute ("not set") choice.
     * \see setAllowEmptyAttributeName()
     */
    bool allowEmptyAttributeName() const { return mAllowEmpty; }

    // QAbstractItemModel interface
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

    /**
     * Returns a HTML formatted tooltip string for a \a attribute, containing details
     * like the attribute name and type.
     */
    static QString attributeToolTip( const QgsPointCloudAttribute &attribute );

    /**
     * Returns an icon corresponding to an attribute \a type
     */
    static QIcon iconForAttributeType( QgsPointCloudAttribute::DataType type );

  private:

    QgsPointCloudAttributeCollection mAttributes;
    bool mAllowEmpty = false;
    QPointer< QgsPointCloudLayer > mLayer;
};


/**
 * \ingroup core
 *
 * \brief A proxy model for filtering available attributes from a point cloud attribute model.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudAttributeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:

    //! Attribute type filters
    enum Filter SIP_ENUM_BASETYPE( IntFlag )
    {
      Char = 1 << 0, //!< Character attributes
      Short = 1 << 1, //!< Short attributes
      Int32 = 1 << 2, //!< Int32 attributes
      Float = 1 << 3, //!< Float attributes
      Double = 1 << 4, //!< Double attributes
      Numeric = Short | Int32 | Float | Double, //!< All numeric attributes
      AllTypes = Numeric | Char, //!< All attribute types
    };
    Q_DECLARE_FLAGS( Filters, Filter )
    Q_FLAG( Filters )

    /**
     * Constructor for QgsPointCloudAttributeProxyModel, with the specified \a source
     * model and \a parent object.
     */
    explicit QgsPointCloudAttributeProxyModel( QgsPointCloudAttributeModel *source, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the QgsPointCloudAttributeModel used in this QSortFilterProxyModel.
     */
    QgsPointCloudAttributeModel *sourceAttributeModel() { return mModel; }

    /**
     * Set flags that affect how fields are filtered in the model.
     * \see filters()
     */
    QgsPointCloudAttributeProxyModel *setFilters( QgsPointCloudAttributeProxyModel::Filters filters );

    /**
     * Returns the filters controlling displayed attributes.
     * \see setFilters()
     */
    Filters filters() const { return mFilters; }

  private:

    QgsPointCloudAttributeModel *mModel = nullptr;
    Filters mFilters = AllTypes;

    // QSortFilterProxyModel interface
  public:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;
};
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsPointCloudAttributeProxyModel::Filters )


#endif // QGSPOINTCLOUDATTRIBUTEMODEL_H
