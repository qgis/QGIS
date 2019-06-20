/***************************************************************************
    qgslayoutmanager.h
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

#ifndef QGSLAYOUTMANAGER_H
#define QGSLAYOUTMANAGER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmasterlayoutinterface.h"
#include <QObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class QgsProject;
class QgsPrintLayout;

/**
 * \ingroup core
 * \class QgsLayoutManager
 *
 * \brief Manages storage of a set of layouts.
 *
 * QgsLayoutManager handles the storage, serializing and deserializing
 * of print layouts and reports. Usually this class is not constructed directly, but
 * rather accessed through a QgsProject via QgsProject::layoutManager().
 *
 * QgsLayoutManager retains ownership of all the layouts contained
 * in the manager.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutManager. The project will become the parent object for this
     * manager.
     */
    explicit QgsLayoutManager( QgsProject *project SIP_TRANSFERTHIS = nullptr );

    ~QgsLayoutManager() override;

    /**
     * Adds a \a layout to the manager. Ownership of the layout is transferred to the manager.
     * Returns TRUE if the addition was successful, or FALSE if the layout could not be added (eg
     * as a result of a duplicate layout name).
     * \see removeLayout()
     * \see layoutAdded()
     */
    bool addLayout( QgsMasterLayoutInterface *layout SIP_TRANSFER );

    /**
     * Removes a \a layout from the manager. The layout is deleted.
     * Returns TRUE if the removal was successful, or FALSE if the removal failed (eg as a result
     * of removing a layout which is not contained in the manager).
     * \see addLayout()
     * \see layoutRemoved()
     * \see layoutAboutToBeRemoved()
     * \see clear()
     */
    bool removeLayout( QgsMasterLayoutInterface *layout );

    /**
     * Removes and deletes all layouts from the manager.
     * \see removeLayout()
     */
    void clear();

    /**
     * Returns a list of all layouts contained in the manager.
     */
    QList< QgsMasterLayoutInterface * > layouts() const;

    /**
     * Returns a list of all print layouts contained in the manager.
     */
    QList< QgsPrintLayout * > printLayouts() const;

    /**
     * Returns the layout with a matching name, or NULLPTR if no matching layouts
     * were found.
     */
    QgsMasterLayoutInterface *layoutByName( const QString &name ) const;

    /**
     * Reads the manager's state from a DOM element, restoring all layouts
     * present in the XML document.
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QDomDocument &doc );

    /**
     * Returns a DOM element representing the state of the manager.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc ) const;

    /**
     * Duplicates an existing \a layout from the manager. The new
     * layout will automatically be stored in the manager.
     * Returns new the layout if duplication was successful.
     */
    QgsMasterLayoutInterface *duplicateLayout( const QgsMasterLayoutInterface *layout, const QString &newName );

    /**
     * Generates a unique title for a new layout of the specified \a type, which does not
     * clash with any already contained by the manager.
     */
    QString generateUniqueTitle( QgsMasterLayoutInterface::Type type = QgsMasterLayoutInterface::PrintLayout ) const;

  signals:

    //! Emitted when a layout is about to be added to the manager
    void layoutAboutToBeAdded( const QString &name );

    //! Emitted when a layout has been added to the manager
    void layoutAdded( const QString &name );

    //! Emitted when a layout was removed from the manager
    void layoutRemoved( const QString &name );

    //! Emitted when a layout is about to be removed from the manager
    void layoutAboutToBeRemoved( const QString &name );

    //! Emitted when a layout is renamed
    void layoutRenamed( QgsMasterLayoutInterface *layout, const QString &newName );

  private:

    QgsProject *mProject = nullptr;

    QList< QgsMasterLayoutInterface * > mLayouts;

};


/**
 * \ingroup core
 * \class QgsLayoutManagerModel
 *
 * List model representing the print layouts and reports available in a
 * layout manager.
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsLayoutManagerModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    //! Custom model roles
    enum Role
    {
      LayoutRole = Qt::UserRole + 1, //!< Layout object
    };

    /**
     * Constructor for QgsLayoutManagerModel, showing the layouts from the specified \a manager.
     */
    explicit QgsLayoutManagerModel( QgsLayoutManager *manager, QObject *parent SIP_TRANSFERTHIS = nullptr );

    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Returns the layout at the corresponding \a index.
     * \see indexFromLayout()
     */
    QgsMasterLayoutInterface *layoutFromIndex( const QModelIndex &index ) const;

    /**
     * Returns the model index corresponding to a \a layout.
     * \see layoutFromIndex()
     */
    QModelIndex indexFromLayout( QgsMasterLayoutInterface *layout ) const;

    /**
     * Sets whether an optional empty layout ("not set") option is present in the model.
     * \see allowEmptyLayout()
     */
    void setAllowEmptyLayout( bool allowEmpty );

    /**
     * Returns TRUE if the model allows the empty layout ("not set") choice.
     * \see setAllowEmptyLayout()
     */
    bool allowEmptyLayout() const { return mAllowEmpty; }

  private slots:
    void layoutAboutToBeAdded( const QString &name );
    void layoutAboutToBeRemoved( const QString &name );
    void layoutAdded( const QString &name );
    void layoutRemoved( const QString &name );
    void layoutRenamed( QgsMasterLayoutInterface *layout, const QString &newName );
  private:
    QgsLayoutManager *mLayoutManager = nullptr;
    bool mAllowEmpty = false;
};


/**
 * \ingroup core
 * \class QgsLayoutManagerProxyModel
 *
 * QSortFilterProxyModel subclass for QgsLayoutManagerModel
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsLayoutManagerProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    //! Available filter flags for filtering the model
    enum Filter
    {
      FilterPrintLayouts = 1 << 1, //!< Includes print layouts
      FilterReports = 1 << 2, //!< Includes reports
    };
    Q_DECLARE_FLAGS( Filters, Filter )
    Q_FLAG( Filters )

    /**
     * Constructor for QgsLayoutManagerProxyModel.
     */
    explicit QgsLayoutManagerProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    /**
     * Returns the current filters used for filtering available layouts.
     *
     * \see setFilters()
     */
    QgsLayoutManagerProxyModel::Filters filters() const;

    /**
     * Sets the current \a filters used for filtering available layouts.
     *
     * \see filters()
     */
    void setFilters( QgsLayoutManagerProxyModel::Filters filters );

  private:

    Filters mFilters = Filters( FilterPrintLayouts | FilterReports );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLayoutManagerProxyModel::Filters )

#endif // QGSLAYOUTMANAGER_H
