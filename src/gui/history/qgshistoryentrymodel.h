/***************************************************************************
                            qgshistoryentrymodel.h
                            --------------------------
    begin                : April 2023
    copyright            : (C) 2023 by Nyall Dawson
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
#ifndef QGSHISTORYENTRYMODEL_H
#define QGSHISTORYENTRYMODEL_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgshistorywidgetcontext.h"
#include <QAbstractItemModel>

class QWidget;
class QAction;

class QgsHistoryEntryGroup;
class QgsHistoryEntryNode;
class QgsHistoryEntry;
class QgsHistoryProviderRegistry;
class QgsHistoryEntryRootNode;

class QgsHistoryEntryDateGroupNode;

/**
 * An item model representing history entries in a hierarchical tree structure.
 *
 * \ingroup gui
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsHistoryEntryModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsHistoryEntryModel, with the specified \a parent object.
     *
     * If \a providerId is specified then the model will contain only items from the matching
     * history provider.
     * If \a backends is specified then the model will be filtered to only matching backends.
     *
     * If no \a registry is specified then the singleton QgsHistoryProviderRegistry from QgsGui::historyProviderRegistry()
     * will be used.
     */
    QgsHistoryEntryModel( const QString &providerId = QString(), Qgis::HistoryProviderBackends backends = Qgis::HistoryProviderBackend::LocalProfile, QgsHistoryProviderRegistry *registry = nullptr, const QgsHistoryWidgetContext &context = QgsHistoryWidgetContext(), QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsHistoryEntryModel() override;
    // Implementation of virtual functions from QAbstractItemModel

    int rowCount( const QModelIndex &parent = QModelIndex() ) const final;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const final;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const final;
    QModelIndex parent( const QModelIndex &child ) const final;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Returns node for given index. Returns root node for invalid index.
     */
    QgsHistoryEntryNode *index2node( const QModelIndex &index ) const;

  private slots:

    void entryAdded( long long id, const QgsHistoryEntry &entry, Qgis::HistoryProviderBackend backend );
    void entryUpdated( long long id, const QVariantMap &entry, Qgis::HistoryProviderBackend backend );
    void historyCleared( Qgis::HistoryProviderBackend backend, const QString &providerId );

  private:
    //! Returns index for a given node
    QModelIndex node2index( QgsHistoryEntryNode *node ) const;
    QModelIndex indexOfParentNode( QgsHistoryEntryNode *parentNode ) const;

    QgsHistoryWidgetContext mContext;

    std::unique_ptr<QgsHistoryEntryRootNode> mRootNode;
    QgsHistoryProviderRegistry *mRegistry = nullptr;
    QString mProviderId;
    Qgis::HistoryProviderBackends mBackends;
    QHash<long long, QgsHistoryEntryNode *> mIdToNodeHash;

    friend class QgsHistoryEntryRootNode;
};

#endif // QGSHISTORYENTRYMODEL_H
