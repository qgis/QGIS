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

class QgsPointCloudLayer;

/**
 * \ingroup core
 *
 * A model for display of available attributes from a point cloud.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudAttributeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    //! Roles utilized by the model
    enum FieldRoles
    {
      AttributeNameRole = Qt::UserRole + 1,  //!< Attribute name
      AttributeSizeRole, //!< Attribute size
      AttributeTypeRole, //!< Attribute type, see QgsPointCloudAttribute::DataType
      IsEmptyRole, //!< TRUE if the index corresponds to the empty value
      IsNumericRole, //!< TRUE if the index corresponds to a numeric attributre
    };

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



#endif // QGSPOINTCLOUDATTRIBUTEMODEL_H
