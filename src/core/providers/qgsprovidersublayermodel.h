/***************************************************************************
                             qgsprovidersublayermodel.h
                             ----------------------
    begin                : June 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#ifndef QGSPROVIDERSUBLAYERMODEL_H
#define QGSPROVIDERSUBLAYERMODEL_H

#include "qgstaskmanager.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

class QgsProviderSublayerDetails;

/**
 * \ingroup core
 *
 * \brief A model for representing the sublayers present in a URI.
 *
 * QgsProviderSublayerModel is designed to present a tree view of the sublayers
 * available for a URI, including any vector, raster or mesh sublayers present.
 *
 * Additionally, QgsProviderSublayerModel can include some non-sublayer items,
 * e.g. in order to represent other content available for a URI, such as
 * embedded project items. The non-sublayer items can be added by calling
 * addNonLayerItem().
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProviderSublayerModel: public QAbstractItemModel
{
    Q_OBJECT

  public:

    //! Custom model roles
    enum class Role : int
    {
      ProviderKey = Qt::UserRole + 1, //!< Provider key
      LayerType, //!< Layer type
      Uri, //!< Layer URI
      Name, //!< Layer name
      Description, //!< Layer description
      Path, //!< Layer path
      FeatureCount, //!< Feature count (for vector sublayers)
      WkbType, //!< WKB geometry type (for vector sublayers)
      GeometryColumnName, //!< Geometry column name (for vector sublayers)
      LayerNumber, //!< Layer number
      IsNonLayerItem, //!< TRUE if item is a non-sublayer item (e.g. an embedded project)
      NonLayerItemType, //!< Item type (for non-sublayer items)
      Flags, //!< Sublayer flags
    };

    //! Model columns
    enum class Column : int
    {
      Name = 0, //!< Layer name
      Description = 1, //!< Layer description
    };

    /**
     * \ingroup core
     *
     * \brief Contains details for a non-sublayer item to include in a QgsProviderSublayerModel.

     * \since QGIS 3.22
     */
    class CORE_EXPORT NonLayerItem
    {
      public:

        /**
         * Returns the item's type.
         * \see setType()
         */
        QString type() const;

        /**
         * Sets the item's \a type.
         * \see type()
         */
        void setType( const QString &type );

        /**
         * Returns the item's name.
         * \see setName()
         */
        QString name() const;

        /**
         * Sets the item's \a name.
         * \see setName()
         */
        void setName( const QString &name );

        /**
         * Returns the item's description.
         * \see setDescription()
         */
        QString description() const;

        /**
         * Sets the item's \a description.
         * \see setDescription()
         */
        void setDescription( const QString &description );

        /**
         * Returns the item's URI.
         * \see setUri()
         */
        QString uri() const;

        /**
         * Set the item's \a uri.
         * \see setUri()
         */
        void setUri( const QString &uri );

        /**
         * Returns the item's icon.
         * \see setIcon()
         */
        QIcon icon() const;

        /**
         * Sets the item's \a icon.
         * \see setIcon()
         */
        void setIcon( const QIcon &icon );

        bool operator==( const QgsProviderSublayerModel::NonLayerItem &other ) const;
        bool operator!=( const QgsProviderSublayerModel::NonLayerItem &other ) const;

#ifdef SIP_RUN
        SIP_PYOBJECT __repr__();
        % MethodCode
        QString str = QStringLiteral( "<QgsProviderSublayerModel.NonLayerItem: %1 - %2>" ).arg( sipCpp->type(), sipCpp->name() );
        sipRes = PyUnicode_FromString( str.toUtf8().constData() );
        % End
#endif

      private:

        QString mType;
        QString mName;
        QString mDescription;
        QString mUri;
        QIcon mIcon;

    };

    /**
     * Constructor for QgsProviderSublayerModel, with the specified \a parent object.
     */
    QgsProviderSublayerModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the sublayer \a details to show in the model.
     *
     * \see sublayerDetails()
     */
    void setSublayerDetails( const QList< QgsProviderSublayerDetails > &details );

    /**
     * Returns the sublayer details shown in the model.
     *
     * \see setSublayerDetails()
     */
    QList< QgsProviderSublayerDetails > sublayerDetails() const;

    /**
     * Returns the sublayer corresponding to the given \a index.
     */
    QgsProviderSublayerDetails indexToSublayer( const QModelIndex &index ) const;

    /**
     * Returns the non layer item corresponding to the given \a index.
     */
    QgsProviderSublayerModel::NonLayerItem indexToNonLayerItem( const QModelIndex &index ) const;

    /**
     * Adds a non-layer item (e.g. an embedded QGIS project item) to the model.
     */
    void addNonLayerItem( const QgsProviderSublayerModel::NonLayerItem &item );

    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

  protected:

    //! Sublayer list
    QList<QgsProviderSublayerDetails> mSublayers;

    //! Non layer item list
    QList<NonLayerItem> mNonLayerItems;

};


/**
 * \ingroup core
 *
 * \brief A QSortFilterProxyModel for filtering and sorting a QgsProviderSublayerModel.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProviderSublayerProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProviderSublayerProxyModel, with the specified \a parent object.
     */
    QgsProviderSublayerProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the filter string used for filtering items in the model.
     *
     * \see setFilterString()
     */
    QString filterString() const;

    /**
     * Sets the \a filter string used for filtering items in the model.
     *
     * \see filterString()
     */
    void setFilterString( const QString &filter );

    /**
     * Returns TRUE if system and internal tables will be shown in the model.
     *
     * \see setIncludeSystemTables()
     */
    bool includeSystemTables() const;

    /**
     * Sets whether system and internal tables will be shown in the model.
     *
     * \see includeSystemTables()
     */
    void setIncludeSystemTables( bool include );

  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;
    bool lessThan( const QModelIndex &source_left, const QModelIndex &source_right ) const override;

  private:

    QString mFilterString;
    bool mIncludeSystemTables = false;

};


#endif // QGSPROVIDERSUBLAYERMODEL_H
