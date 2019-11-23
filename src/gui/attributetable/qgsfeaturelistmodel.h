/***************************************************************************
    qgsfeaturelistmodel.h
    ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITORMODEL_H
#define QGSATTRIBUTEEDITORMODEL_H

#include "qgsexpression.h"

#include <QSortFilterProxyModel>
#include <QVariant>
#include <QItemSelectionModel>

#include "qgsfeaturemodel.h"
#include "qgsfeature.h" // QgsFeatureId
#include "qgsexpressioncontext.h"
#include "qgsconditionalstyle.h"
#include "qgis_gui.h"

class QgsAttributeTableFilterModel;
class QgsAttributeTableModel;
class QgsVectorLayerCache;

/**
 * \ingroup gui
 * \class QgsFeatureListModel
 */
class GUI_EXPORT QgsFeatureListModel : public QSortFilterProxyModel, public QgsFeatureModel
{
    Q_OBJECT

  public:
    struct FeatureInfo
    {
      public:

        /**
         * Constructor for FeatureInfo.
         */
        FeatureInfo() = default;

        //! True if feature is a newly added feature.
        bool isNew = false;

        //! True if feature has been edited.
        bool isEdited = false;
    };

    enum Role
    {
      FeatureInfoRole = 0x1000, // Make sure no collisions with roles on QgsAttributeTableModel
      FeatureRole
    };

  public:

    //! Constructor for QgsFeatureListModel
    explicit QgsFeatureListModel( QgsAttributeTableFilterModel *sourceModel, QObject *parent SIP_TRANSFERTHIS = nullptr );

    virtual void setSourceModel( QgsAttributeTableFilterModel *sourceModel );

    /**
     * Returns the vector layer cache which is being used to populate the model.
     */
    QgsVectorLayerCache *layerCache();

    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * \brief If TRUE is specified, a NULL value will be injected
     * \param injectNull state of null value injection
     * \since QGIS 2.9
     */
    void setInjectNull( bool injectNull );

    /**
     * \brief Returns the current state of null value injection
     * \returns If a NULL value is added
     * \since QGIS 2.9
     */
    bool injectNull();

    QgsAttributeTableModel *masterModel();

    /**
     *  \param  expression   A QgsExpression compatible string.
     *  \returns TRUE if the expression could be set, FALSE if there was a parse error.
     *          If it fails, the old expression will still be applied. Call parserErrorString()
     *          for a meaningful error message.
     */
    bool setDisplayExpression( const QString &expression );

    /**
     * \brief Returns a detailed message about errors while parsing a QgsExpression.
     * \returns A message containing information about the parser error.
     */
    QString parserErrorString();

    QString displayExpression() const;
    bool featureByIndex( const QModelIndex &index, QgsFeature &feat );

    /**
     * Returns the feature ID corresponding to an \a index from the model.
     * \see fidToIdx()
     */
    QgsFeatureId idxToFid( const QModelIndex &index ) const;

    /**
     * Returns the model index corresponding to a feature ID.
     * \see idxToFid()
     */
    QModelIndex fidToIdx( QgsFeatureId fid ) const;

    QModelIndex mapToSource( const QModelIndex &proxyIndex ) const override;
    QModelIndex mapFromSource( const QModelIndex &sourceIndex ) const override;

    virtual QModelIndex mapToMaster( const QModelIndex &proxyIndex ) const;
    virtual QModelIndex mapFromMaster( const QModelIndex &sourceIndex ) const;

    virtual QItemSelection mapSelectionFromMaster( const QItemSelection &selection ) const;
    virtual QItemSelection mapSelectionToMaster( const QItemSelection &selection ) const;

    QModelIndex parent( const QModelIndex &child ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;

    QModelIndex fidToIndex( QgsFeatureId fid ) override;
    QModelIndexList fidToIndexList( QgsFeatureId fid );

    /**
     * Sort this model by its display expression.
     *
     * \since QGIS 3.2
     */
    bool sortByDisplayExpression() const;

    /**
     * Sort this model by its display expression.
     *
     * \note Not compatible with injectNull, if sorting by display expression is enabled,
     * injectNull will automatically turned off.
     *
     * \since QGIS 3.2
     */
    void setSortByDisplayExpression( bool sortByDisplayExpression );

  public slots:

    /**
     * Does nothing except for calling beginRemoveRows()
     *
     * \deprecated Use beginRemoveRows() instead
     */
    Q_DECL_DEPRECATED void onBeginRemoveRows( const QModelIndex &parent, int first, int last );

    /**
     * Does nothing except for calling endRemoveRows()
     *
     * \deprecated Use endRemoveRows() instead
     */
    Q_DECL_DEPRECATED void onEndRemoveRows( const QModelIndex &parent, int first, int last );

    /**
     * Does nothing except for calling beginInsertRows()
     *
     * \deprecated use beginInsertRows() instead
     */
    Q_DECL_DEPRECATED void onBeginInsertRows( const QModelIndex &parent, int first, int last );

    /**
     * Does nothing except for calling endInsertRows()
     *
     * \deprecated use endInsertRows() instead
     */
    Q_DECL_DEPRECATED void onEndInsertRows( const QModelIndex &parent, int first, int last );

  private slots:

    void conditionalStylesChanged();

  private:
    mutable QgsExpression mDisplayExpression;
    QgsAttributeTableFilterModel *mFilterModel = nullptr;
    QString mParserErrorString;
    bool mInjectNull = false;
    mutable QgsExpressionContext mExpressionContext;
    mutable QMap< QgsFeatureId, QList<QgsConditionalStyle> > mRowStylesMap;
    bool mSortByDisplayExpression = false;
    QPointer< QgsVectorLayer > mSourceLayer;
};

Q_DECLARE_METATYPE( QgsFeatureListModel::FeatureInfo )

#endif // QGSATTRIBUTEEDITORMODEL_H
