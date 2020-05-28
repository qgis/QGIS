/***************************************************************************
  qgstransactiongroup.h - QgsTransactionGroup

 ---------------------
 begin                : 15.1.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : mmatthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTRANSACTIONGROUP_H
#define QGSTRANSACTIONGROUP_H

#include "qgis_core.h"
#include <QObject>
#include <QSet>
#include <memory>
#include "qgstransaction.h"

class QgsVectorLayer;

/**
 * \ingroup core
 * \class QgsTransactionGroup
 */
class CORE_EXPORT QgsTransactionGroup : public QObject
{
    Q_OBJECT
  public:

    //! Constructor for QgsTransactionGroup
    explicit QgsTransactionGroup( QObject *parent = nullptr );

    /**
     * Add a layer to this transaction group.
     *
     * Will return TRUE if it is compatible and has been added.
     */
    bool addLayer( QgsVectorLayer *layer );

    /**
     * Gets the set of layers currently managed by this transaction group.
     *
     * \returns Layer set
     */
    QSet<QgsVectorLayer *> layers() const;

    /**
     * Returns TRUE if any of the layers in this group reports a modification.
     */
    bool modified() const;

    /**
     * Returns the connection string used by this transaction group.
     * Layers need be compatible when added.
     */
    QString connString() const;

    /**
     * Returns the provider key used by this transaction group.
     * Layers need be compatible when added.
     */
    QString providerKey() const;

    /**
     * Returns TRUE if there are no layers in this transaction group.
     */
    bool isEmpty() const;

  signals:

    /**
     * Will be emitted whenever there is a commit error
     */
    void commitError( const QString &msg );

  private slots:
    void onEditingStarted();
    void onLayerDeleted();
    void onCommitChanges();
    void onRollback();

  private:
    bool mEditingStarting = false;
    bool mEditingStopping = false;

    void disableTransaction();

    QSet<QgsVectorLayer *> mLayers;
    //! Only set while a transaction is active
    std::unique_ptr<QgsTransaction> mTransaction;
    //! Layers have to be compatible with the connection string
    QString mConnString;
    QString mProviderKey;
};

#endif // QGSTRANSACTIONGROUP_H
