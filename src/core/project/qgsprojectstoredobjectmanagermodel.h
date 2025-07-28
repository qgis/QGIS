/***************************************************************************
    qgsprojectstoredobjectmanagermodel.h
    ------------------
    Date                 : January 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTSTOREDOBJECTMANAGERMODEL_H
#define QGSPROJECTSTOREDOBJECTMANAGERMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include "qgsprojectstoredobjectmanager.h"

/**
 * \ingroup core
 * \class QgsProjectStoredObjectManagerModelBase
 *
 * \brief Base class for list models representing the objects available in a
 * QgsAbstractProjectStoredObjectManager.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsProjectStoredObjectManagerModelBase : public QAbstractListModel
{
    Q_OBJECT

  public:

    // *INDENT-OFF*

    /**
     * Custom model roles.
     */
    enum class CustomRole : int
    {
      Object = Qt::UserRole + 1, //!< Object
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    QgsProjectStoredObjectManagerModelBase( QObject *parent SIP_TRANSFERTHIS = nullptr );

    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

  protected slots:

///@cond PRIVATE
#ifndef SIP_RUN
    void objectAboutToBeAdded( const QString &name );
    void objectAboutToBeRemoved( const QString &name );
    void objectAdded( const QString &name );
    void objectRemoved( const QString &name );
#endif
///@endcond

  protected:

///@cond PRIVATE
#ifndef SIP_RUN
    virtual void objectAboutToBeAddedInternal( const QString & ) {}
    virtual void objectAboutToBeRemovedInternal( const QString & ) {}
    virtual void objectAddedInternal( const QString & ) {}
    virtual void objectRemovedInternal( const QString & ) {}

    virtual int rowCountInternal( const QModelIndex & ) const { return 0; }
    virtual QVariant dataInternal( const QModelIndex &, int ) const { return QVariant(); }
    virtual bool setDataInternal( const QModelIndex &, const QVariant &, int = Qt::EditRole ) { return false; }
    virtual Qt::ItemFlags flagsInternal( const QModelIndex & ) const { return Qt::ItemFlags();}
#endif
///@endcond
};

/**
 * \ingroup core
 * \class QgsProjectStoredObjectManagerModel
 *
 * \brief Template class for models representing the objects available in a
 * QgsAbstractProjectStoredObjectManager.
 *
 * \since QGIS 4.0
 */
template<class T>
class CORE_EXPORT QgsProjectStoredObjectManagerModel : public QgsProjectStoredObjectManagerModelBase
{

  public:

    /**
     * Constructor for QgsProjectStoredObjectManagerModel, showing the objects from the specified \a manager.
     */
    explicit QgsProjectStoredObjectManagerModel( QgsAbstractProjectStoredObjectManager<T> *manager, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the object at the corresponding \a index.
     * \see indexFromObject()
     */
    T *objectFromIndex( const QModelIndex &index ) const;

    /**
     * Returns the model index corresponding to an \a object.
     * \see objectFromIndex()
     */
    QModelIndex indexFromObject( T *object ) const;

    /**
     * Sets whether an optional empty object ("not set") option is present in the model.
     * \see allowEmptyObject()
     */
    void setAllowEmptyObject( bool allowEmpty );

    /**
     * Returns TRUE if the model allows the empty object ("not set") choice.
     * \see setAllowEmptyObject()
     */
    bool allowEmptyObject() const { return mAllowEmpty; }

  protected:

///@cond PRIVATE
    int rowCountInternal( const QModelIndex &parent ) const override;
    QVariant dataInternal( const QModelIndex &index, int role ) const override;
    bool setDataInternal( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flagsInternal( const QModelIndex &index ) const override;

    void objectAboutToBeAddedInternal( const QString &name ) override;
    void objectAboutToBeRemovedInternal( const QString &name ) override;
    void objectAddedInternal( const QString &name ) override;
    void objectRemovedInternal( const QString &name ) override;
    void objectRenamedInternal( T *object, const QString &newName );
    QVariant objectToVariant( T *object ) const;
///@endcond PRIVATE

    //! Object manager
    QgsAbstractProjectStoredObjectManager<T> *mObjectManager = nullptr;

  private:
    bool mAllowEmpty = false;
};


#endif // QGSPROJECTSTOREDOBJECTMANAGERMODEL_H
