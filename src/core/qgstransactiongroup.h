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

#include <QObject>
#include <QSet>

class QgsVectorLayer;
class QgsTransaction;

/** \ingroup core
 * \class QgsTransactionGroup
 */
class CORE_EXPORT QgsTransactionGroup : public QObject
{
    Q_OBJECT
  public:
    explicit QgsTransactionGroup( QObject *parent = 0 );

    ~QgsTransactionGroup();

    /**
     * Add a layer to this transaction group.
     *
     * Will return true if it is compatible and has been added.
     */
    bool addLayer( QgsVectorLayer* layer );

    /**
     * Get the set of layers currently managed by this transaction group.
     *
     * @return Layer set
     */
    QSet<QgsVectorLayer*> layers() const;

    /**
     * Returns true if any of the layers in this group reports a modification.
     */
    bool modified() const;

    /**
     * Return the connection string used by this transaction group.
     * Layers need be compatible when added.
     */
    QString connString() const;

    /**
     * Return the provider key used by this transaction group.
     * Layers need be compatible when added.
     */
    QString providerKey() const;

    /**
     * Returns true if there are no layers in this transaction group.
     */
    bool isEmpty() const;

  signals:
    /**
     * Will be emitted whenever there is a commit error
     */
    void commitError( const QString& msg );

  private slots:
    void onEditingStarted();
    void onLayerDeleted();
    void onCommitChanges();
    void onRollback();

  private:
    bool mEditingStarting;
    bool mEditingStopping;

    void disableTransaction();

    QSet<QgsVectorLayer*> mLayers;
    //! Only set while a transaction is active
    QScopedPointer<QgsTransaction> mTransaction;
    //! Layers have to be compatible with the connection string
    QString mConnString;
    QString mProviderKey;
};

#endif // QGSTRANSACTIONGROUP_H
