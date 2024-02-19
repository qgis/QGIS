/***************************************************************************
                            qgsfavoritesitem.h
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFAVORITESITEM_H
#define QGSFAVORITESITEM_H

#include "qgis_sip.h"
#include "qgis_core.h"

#include "qgis.h"
#include "qgsdatacollectionitem.h"
#include "qgsdirectoryitem.h"

/**
 * \ingroup core
 * \brief Contains various Favorites directories
*/
class CORE_EXPORT QgsFavoritesItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFavoritesItem. Accepts a path argument specifying the file path associated with
     * the item.
     */
    QgsFavoritesItem( QgsDataItem *parent, const QString &name, const QString &path = QString() );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsFavoritesItem: \"%1\">" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QVector<QgsDataItem *> createChildren() override;

    /**
     * Adds a new \a directory to the favorites group.
     *
     * If \a name is specified, it will be used as the favorite's name. Otherwise
     * the name will be set to match \a directory.
     *
     * \see removeDirectory()
     */
    void addDirectory( const QString &directory, const QString &name = QString() );

    /**
     * Removes an existing directory from the favorites group.
     * \see addDirectory()
     */
    void removeDirectory( QgsDirectoryItem *item );

    /**
     * Renames the stored favorite with corresponding \a path a new \a name.
     */
    void renameFavorite( const QString &path, const QString &name );

    //! Icon for favorites group
    static QIcon iconFavorites();

    QVariant sortKey() const override;

  private:
    QVector<QgsDataItem *> createChildren( const QString &directory, const QString &name );
};

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief A directory item showing the a single favorite directory.
 * \note Not available in Python bindings
*/
Q_NOWARN_DEPRECATED_PUSH  // rename is deprecated
class CORE_EXPORT QgsFavoriteItem : public QgsDirectoryItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFavoriteItem.
     *
     * \param parent parent favorites item
     * \param name visible name for item
     * \param dirPath corresponding directory (folder) path
     * \param path unique data item path
     */
    QgsFavoriteItem( QgsFavoritesItem *parent, const QString &name, const QString &dirPath, const QString &path );

    // TODO QGIS 4.0 - don't remove this method when the deprecated base class virtual method is removed, but instead
    // remove the override!

    /**
     * Sets a new \a name for the favorite.
     */
    bool rename( const QString &name ) override;

  private:

    QgsFavoritesItem *mFavorites = nullptr;

};
Q_NOWARN_DEPRECATED_POP

#endif

#endif // QGSFAVORITESITEM_H


