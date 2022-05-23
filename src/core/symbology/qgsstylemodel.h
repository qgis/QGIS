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
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QIcon>
#include <QHash>

class QgsSymbol;
class QgsCombinedStyleModel;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \class QgsAbstractStyleEntityIconGenerator
 *
 * \brief An abstract base class for icon generators for a QgsStyleModel.
 *
 * This base class allows for creation of specialized icon generators for
 * entities in a style database, and allows for deferred icon generation.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAbstractStyleEntityIconGenerator : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAbstractStyleEntityIconGenerator, with the specified \a parent
     * object.
     */
    QgsAbstractStyleEntityIconGenerator( QObject *parent );

    /**
     * Triggers generation of an icon for an entity from the specified \a style database,
     * with matching entity \a type and \a name.
     */
    virtual void generateIcon( QgsStyle *style, QgsStyle::StyleEntity type, const QString &name ) = 0;

    /**
     * Sets the list of icon \a sizes to generate.
     *
     * \see iconSizes()
     */
    void setIconSizes( const QList< QSize > &sizes );

    /**
     * Returns the list of icon \a sizes to generate.
     *
     * \see setIconSizes()
     */
    QList< QSize > iconSizes() const;

  signals:

    /**
     * Emitted when the \a icon for the style entity with matching \a type and \a name
     * has been generated.
     */
    void iconGenerated( QgsStyle::StyleEntity type, const QString &name, const QIcon &icon );

  private:

    QList< QSize > mIconSizes;

};

#endif

/**
 * \ingroup core
 * \class QgsStyleModel
 *
 * \brief A QAbstractItemModel subclass for showing symbol and color ramp entities contained
 * within a QgsStyle database.
 *
 * If you are creating a style model for the default application style (see QgsStyle::defaultStyle()),
 * consider using the shared style model available at QgsApplication::defaultStyleModel() for performance
 * instead.
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
      EntityName, //!< Entity name (since QGIS 3.26)
      SymbolTypeRole, //!< Symbol type (for symbol or legend patch shape entities)
      IsFavoriteRole, //!< Whether entity is flagged as a favorite
      LayerTypeRole, //!< Layer type (for label settings entities)
      CompatibleGeometryTypesRole, //!< Compatible layer geometry types (for 3D symbols)
      StyleName, //!< Name of associated QgsStyle (QgsStyle::name()) (since QGIS 3.26)
      StyleFileName, //!< File name of associated QgsStyle (QgsStyle::fileName()) (since QGIS 3.26)
      IsTitleRole, //!< True if the index corresponds to a title item (since QGIS 3.26)
    };

    /**
     * Constructor for QgsStyleModel, for the specified \a style and \a parent object.
     *
     * The \a style object must exist for the lifetime of this model.
     */
    explicit QgsStyleModel( QgsStyle *style, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the style managed by the model.
     *
     * \since QGIS 3.10
     */
    QgsStyle *style() { return mStyle; }

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

    /**
     * Sets the icon \a generator to use for deferred style entity icon generation.
     *
     * Currently this is used for 3D symbol icons only.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.16
     */
    static void setIconGenerator( QgsAbstractStyleEntityIconGenerator *generator ) SIP_SKIP;

  private slots:

    void onEntityAdded( QgsStyle::StyleEntity type, const QString &name );
    void onEntityRemoved( QgsStyle::StyleEntity type, const QString &name );
    void onEntityChanged( QgsStyle::StyleEntity type, const QString &name );
    void onFavoriteChanged( QgsStyle::StyleEntity type, const QString &name, bool isFavorite );
    void onEntityRename( QgsStyle::StyleEntity type, const QString &oldName, const QString &newName );
    void onTagsChanged( int entity, const QString &name, const QStringList &tags );
    void rebuildSymbolIcons();
    void iconGenerated( QgsStyle::StyleEntity type, const QString &name, const QIcon &icon );

  private:

    QgsStyle *mStyle = nullptr;

    QHash< QgsStyle::StyleEntity, QStringList > mEntityNames;

    QList< QSize > mAdditionalSizes;
    mutable std::unique_ptr< QgsExpressionContext > mExpressionContext;

    mutable QHash< QgsStyle::StyleEntity, QHash< QString, QIcon > > mIconCache;

    static QgsAbstractStyleEntityIconGenerator *sIconGenerator;
    mutable QSet< QString > mPending3dSymbolIcons;

    QgsStyle::StyleEntity entityTypeFromRow( int row ) const;

    int offsetForEntity( QgsStyle::StyleEntity entity ) const;
    static QVariant headerDataStatic( int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole );

    friend class QgsCombinedStyleModel;
};

/**
 * \ingroup core
 * \class QgsStyleProxyModel
 *
 * \brief A QSortFilterProxyModel subclass for showing filtered symbol and color ramps entries from a QgsStyle database.
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
     * Constructor for QgsStyleProxyModel, using the specified source \a model and \a parent object.
     *
     * The source \a model object must exist for the lifetime of this model.
     */
    explicit QgsStyleProxyModel( QgsStyleModel *model, QObject *parent SIP_TRANSFERTHIS = nullptr );

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    SIP_IF_FEATURE( CONCATENATED_TABLES_MODEL )

    /**
     * Constructor for QgsStyleProxyModel, using the specified source combined \a model and \a parent object.
     *
     * The source \a model object must exist for the lifetime of this model.
     *
     * \note This is only available on builds based on Qt 5.13 or later.
     *
     * \since QGIS 3.26
     */
    explicit QgsStyleProxyModel( QgsCombinedStyleModel *model, QObject *parent SIP_TRANSFERTHIS = nullptr );
    SIP_END
#endif

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
     * Sets the style entity type \a filters.
     *
     * \note These filters are only active if entityFilterEnabled() is TRUE.
     * \note Not available in Python bindings
     *
     * \see setEntityFilter()
     * \since QGIS 3.10
     */
    void setEntityFilters( const QList<QgsStyle::StyleEntity> &filters ) SIP_SKIP;

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
     * \note This filter is only active if Qgis::SymbolTypeFilterEnabled() is TRUE, and has
     * no effect on non-symbol entities (i.e. color ramps).
     *
     * \see setSymbolType()
     */
    Qgis::SymbolType symbolType() const;

    /**
     * Sets the symbol \a type filter.
     *
     * \note This filter is only active if Qgis::SymbolTypeFilterEnabled() is TRUE.
     *
     * \see Qgis::SymbolType()
     */
    void setSymbolType( Qgis::SymbolType type );

    /**
     * Returns TRUE if filtering by symbol type is enabled.
     *
     * \see setSymbolTypeFilterEnabled()
     * \see Qgis::SymbolType()
     */
    bool symbolTypeFilterEnabled() const;

    /**
     * Sets whether filtering by symbol type is \a enabled.
     *
     * If \a enabled is FALSE, then the value of Qgis::SymbolType() will have no
     * effect on the model filtering. This has
     * no effect on non-symbol entities (i.e. color ramps).
     *
     * \see Qgis::SymbolTypeFilterEnabled()
     * \see setSymbolType()
     */
    void setSymbolTypeFilterEnabled( bool enabled );

    /**
     * Returns the layer type filter, or QgsWkbTypes::UnknownGeometry if no
     * layer type filter is present.
     *
     * This setting has an effect on label settings entities and 3d symbols only.
     *
     * \see setLayerType()
     */
    QgsWkbTypes::GeometryType layerType() const;

    /**
     * Sets the layer \a type filter. Set \a type to QgsWkbTypes::UnknownGeometry if no
     * layer type filter is desired.
     *
     * \see layerType()
     */
    void setLayerType( QgsWkbTypes::GeometryType type );

    /**
     * Sets a tag \a id to filter style entities by. Only entities with the given
     * tag will be shown in the model.
     *
     * Set \a id to -1 to disable tag filtering.
     *
     * \note This method has no effect for models created using QgsCombinedStyleModel source models. Use setTagString() instead.
     *
     * \see tagId()
     */
    void setTagId( int id );

    /**
     * Returns the tag id used to filter style entities by.
     *
     * If returned value is -1, then no tag filtering is being conducted.
     *
     * \note This method has no effect for models created using QgsCombinedStyleModel source models. Use tagString() instead.
     *
     * \see setTagId()
     */
    int tagId() const;

    /**
     * Sets a \a tag to filter style entities by. Only entities with the given
     * tag will be shown in the model.
     *
     * Set \a tag to an empty string to disable tag filtering.
     *
     * \see tagString()
     * \since QGIS 3.26
     */
    void setTagString( const QString &tag );

    /**
     * Returns the tag string used to filter style entities by.
     *
     * If returned value is empty, then no tag filtering is being conducted.
     *
     * \see setTagString()
     * \since QGIS 3.26
     */
    QString tagString() const;

    /**
     * Sets a smart group \a id to filter style entities by. Only entities within the given
     * smart group will be shown in the model.
     *
     * Set \a id to -1 to disable smart group filtering.
     *
     * \note This method has no effect for models created using QgsCombinedStyleModel source models.
     *
     * \see smartGroupId()
     */
    void setSmartGroupId( int id );

    /**
     * Returns the smart group id used to filter style entities by.
     *
     * If returned value is -1, then no smart group filtering is being conducted.
     *
     * \note This method has no effect for models created using QgsCombinedStyleModel source models.
     *
     * \see setSmartGroupId()
     */
    int smartGroupId() const;

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

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

    void initialize();

    QgsStyleModel *mModel = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    QgsCombinedStyleModel *mCombinedModel = nullptr;
#endif
    QgsStyle *mStyle = nullptr;

    QString mFilterString;

    int mTagId = -1;
    QStringList mTaggedSymbolNames;

    QString mTagFilter;

    int mSmartGroupId = -1;
    QStringList mSmartGroupSymbolNames;

    bool mFavoritesOnly = false;

    bool mEntityFilterEnabled = false;
    QList< QgsStyle::StyleEntity > mEntityFilters = QList< QgsStyle::StyleEntity >() << QgsStyle::SymbolEntity;

    bool mSymbolTypeFilterEnabled = false;
    Qgis::SymbolType mSymbolType = Qgis::SymbolType::Marker;

    QgsWkbTypes::GeometryType mLayerType = QgsWkbTypes::UnknownGeometry;

};

#endif //QGSSTYLEMODEL_H
