/***************************************************************************
    qgsstylemodel.h
    ---------------
    begin                : September 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTYLEMODEL_H
#define QGSSTYLEMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QIcon>
#include <QHash>

class QgsSymbol;

/**
 * \ingroup core
 * \class QgsStyleModel
 *
 * A QAbstractItemModel subclass for showing symbol and color ramp entities contained
 * within a QgsStyle database.
 *
 * \see QgsStyleProxyModel
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsStyleModel: public QAbstractItemModel
{
    Q_OBJECT

  public:

    //! Model columns
    enum Column
    {
      Name = 0, //!< Name column
      Tags, //!< Tags column
    };

    //! Custom model roles
    enum Role
    {
      TypeRole = Qt::UserRole + 1, //!< Style entity type, see QgsStyle::StyleEntity
      TagRole, //!< String list of tags
      SymbolTypeRole, //!< Symbol type (for symbol entities)
    };

    /**
     * Constructor for QgsStyleModel, for the specified \a style and \a parent object.
     *
     * The \a style object must exist for the lifetime of this model.
     */
    explicit QgsStyleModel( QgsStyle *style, QObject *parent SIP_TRANSFERTHIS = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
    QModelIndex index( int row, int column,
                       const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    /**
     * Adds an additional icon \a size to generate for Qt::DecorationRole data.
     *
     * This allows style icons to be generated at an icon size which
     * corresponds exactly to the view's icon size in which this model is used.
     */
    void addDesiredIconSize( QSize size );

  private slots:

    void onSymbolAdded( const QString &name, QgsSymbol *symbol );
    void onSymbolRemoved( const QString &name );
    void onSymbolChanged( const QString &name );
    void onSymbolRename( const QString &oldName, const QString &newName );
    void onRampAdded( const QString &name );
    void onRampRemoved( const QString &name );
    void onRampChanged( const QString &name );
    void onRampRename( const QString &oldName, const QString &newName );
    void onTagsChanged( int entity, const QString &name, const QStringList &tags );
    void rebuildSymbolIcons();

  private:

    QgsStyle *mStyle = nullptr;
    QStringList mSymbolNames;
    QStringList mRampNames;
    QList< QSize > mAdditionalSizes;

    mutable QHash< QString, QIcon > mSymbolIconCache;
    mutable QHash< QString, QIcon > mColorRampIconCache;

};

/**
 * \ingroup core
 * \class QgsStyleProxyModel
 *
 * A QSortFilterProxyModel subclass for showing filtered symbol and color ramps entries from a QgsStyle database.
 *
 * \see QgsStyleModel
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsStyleProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsStyleProxyModel, for the specified \a style and \a parent object.
     *
     * The \a style object must exist for the lifetime of this model.
     */
    explicit QgsStyleProxyModel( QgsStyle *style, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the current filter string, if set.
     *
     * \see setFilterString()
     */
    QString filterString() const { return mFilterString; }

    /**
     * Returns the style entity type filter.
     *
     * \note This filter is only active if entityFilterEnabled() is TRUE.
     * \see setEntityFilter()
     */
    QgsStyle::StyleEntity entityFilter() const;

    /**
     * Sets the style entity type \a filter.
     *
     * \note This filter is only active if entityFilterEnabled() is TRUE.
     *
     * \see entityFilter()
     */
    void setEntityFilter( QgsStyle::StyleEntity filter );

    /**
     * Returns TRUE if filtering by entity type is enabled.
     *
     * \see setEntityFilterEnabled()
     * \see entityFilter()
     */
    bool entityFilterEnabled() const;

    /**
     * Sets whether filtering by entity type is \a enabled.
     *
     * If \a enabled is FALSE, then the value of entityFilter() will have no
     * effect on the model filtering.
     *
     * \see entityFilterEnabled()
     * \see setEntityFilter()
     */
    void setEntityFilterEnabled( bool enabled );

    /**
     * Returns the symbol type filter.
     *
     * \note This filter is only active if symbolTypeFilterEnabled() is TRUE, and has
     * no effect on non-symbol entities (i.e. color ramps).
     *
     * \see setSymbolType()
     */
    QgsSymbol::SymbolType symbolType() const;

    /**
     * Sets the symbol \a type filter.
     *
     * \note This filter is only active if symbolTypeFilterEnabled() is TRUE.
     *
     * \see symbolType()
     */
    void setSymbolType( QgsSymbol::SymbolType type );

    /**
     * Returns TRUE if filtering by symbol type is enabled.
     *
     * \see setSymbolTypeFilterEnabled()
     * \see symbolType()
     */
    bool symbolTypeFilterEnabled() const;

    /**
     * Sets whether filtering by symbol type is \a enabled.
     *
     * If \a enabled is FALSE, then the value of symbolType() will have no
     * effect on the model filtering. This has
     * no effect on non-symbol entities (i.e. color ramps).
     *
     * \see symbolTypeFilterEnabled()
     * \see setSymbolType()
     */
    void setSymbolTypeFilterEnabled( bool enabled );

    /**
     * Sets a tag \a id to filter style entities by. Only entities with the given
     * tag will be shown in the model.
     *
     * Set \a id to -1 to disable tag filtering.
     *
     * \see tagId()
     */
    void setTagId( int id );

    /**
     * Returns the tag id used to filter style entities by.
     *
     * If returned value is -1, then no tag filtering is being conducted.
     *
     * \see setTagId()
     */
    int tagId() const;

    /**
     * Sets a smart group \a id to filter style entities by. Only entities within the given
     * smart group will be shown in the model.
     *
     * Set \a id to -1 to disable smart group filtering.
     *
     * \see smartGroupId()
     */
    void setSmartGroupId( int id );

    /**
     * Returns the smart group id used to filter style entities by.
     *
     * If returned value is -1, then no smart group filtering is being conducted.
     *
     * \see setSmartGroupId()
     */
    int smartGroupId() const;

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

    /**
     * Returns TRUE if the model is showing only favorited entities.
     *
     * \see setFavoritesOnly()
     */
    bool favoritesOnly() const;

    /**
     * Sets whether the model should show only favorited entities.
     *
     * \see setFavoritesOnly()
     */
    void setFavoritesOnly( bool favoritesOnly );

    /**
     * Adds an additional icon \a size to generate for Qt::DecorationRole data.
     *
     * This allows style icons to be generated at an icon size which
     * corresponds exactly to the view's icon size in which this model is used.
     */
    void addDesiredIconSize( QSize size );

  public slots:

    /**
     * Sets a \a filter string, such that only symbol entities with names matching the
     * specified string will be shown.
     *
     * \see filterString()
    */
    void setFilterString( const QString &filter );

  private:

    QgsStyleModel *mModel = nullptr;
    QgsStyle *mStyle = nullptr;

    QString mFilterString;

    int mTagId = -1;
    QStringList mTaggedSymbolNames;

    int mSmartGroupId = -1;
    QStringList mSmartGroupSymbolNames;

    bool mFavoritesOnly = false;
    QStringList mFavoritedSymbolNames;

    bool mEntityFilterEnabled = false;
    QgsStyle::StyleEntity mEntityFilter = QgsStyle::SymbolEntity;

    bool mSymbolTypeFilterEnabled = false;
    QgsSymbol::SymbolType mSymbolType = QgsSymbol::Marker;

};

#endif //QGSSTYLEMODEL_H
