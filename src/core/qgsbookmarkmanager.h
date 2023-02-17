/***************************************************************************
    qgsbookmarkmanager.h
    ------------------
    Date                 : Septemeber 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBOOKMARKMANAGER_H
#define QGSBOOKMARKMANAGER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsreferencedgeometry.h"
#include <QObject>

class QgsProject;

/**
 * \ingroup core
 * \class QgsBookmark
 *
 * \brief Represents a spatial bookmark, with a name, CRS and extent.
 *
 * QgsBookmark objects are typically used alongside the QgsBookmarkManager class,
 * which handles storage of a set of bookmarks.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsBookmark
{

  public:

    /**
     * Default constructor, creates an empty bookmark.
     */
    QgsBookmark() = default;

    /**
     * Returns the bookmark's unique ID.
     * \see setId()
     */
    QString id() const;

    /**
     * Sets the bookmark's unique \a id.
     * \see id()
     */
    void setId( const QString &id );

    /**
     * Returns the bookmark's name, which is a user-visible string identifying
     * the bookmark.
     * \see setName()
     */
    QString name() const;

    /**
     * Sets the bookmark's \a name, which is a user-visible string identifying
     * the bookmark.
     * \see name()
     */
    void setName( const QString &name );

    /**
     * Returns the bookmark's group, which is a user-visible string identifying
     * the bookmark's category.
     * \see setGroup()
     */
    QString group() const;

    /**
     * Sets the bookmark's \a group, which is a user-visible string identifying
     * the bookmark's category.
     * \see group()
     */
    void setGroup( const QString &group );

    /**
     * Returns the bookmark's spatial extent.
     * \see setExtent()
     */
    QgsReferencedRectangle extent() const;

    /**
     * Sets the bookmark's spatial \a extent.
     * \see extent()
     */
    void setExtent( const QgsReferencedRectangle &extent );

    /**
     * Creates a bookmark using the properties from a DOM \a element.
     * \see writeXml()
     */
    static QgsBookmark fromXml( const QDomElement &element, const QDomDocument &doc );

    /**
     * Returns a DOM element representing the bookmark's properties.
     * \see fromXml()
     */
    QDomElement writeXml( QDomDocument &doc ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsBookmark: '%1' (%2 - %3)>" ).arg( sipCpp->name(), sipCpp->extent().asWktCoordinates(), sipCpp->extent().crs().authid() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    // TODO c++20 - replace with = default
    bool operator==( const QgsBookmark &other ) const;
    bool operator!=( const QgsBookmark &other ) const;

  private:

    QString mId;
    QString mName;
    QString mGroup;
    QgsReferencedRectangle mExtent;

};

/**
 * \ingroup core
 * \class QgsBookmarkManager
 *
 * \brief Manages storage of a set of bookmarks.
 *
 * QgsBookmarkManager handles the storage, serializing and deserializing
 * of geographic bookmarks. Usually this class is not constructed directly, but
 * rather accessed through a QgsProject via QgsProject::bookmarkManager(), or via
 * the application-wide bookmark store at QgsApplication::bookmarkManager().
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsBookmarkManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Returns a newly created QgsBookmarkManager using a project-based bookmark store, linked to the specified \a project.
     *
     * The returned object is parented to the \a project.
     */
    static QgsBookmarkManager *createProjectBasedManager( QgsProject *project );

    /**
     * Constructor for QgsBookmarkManager, with the specified \a parent object.
     *
     * This constructor creates a bookmark manager which stores bookmarks in an XML file. A call to
     * initialize() is required to initialize the manager and set the corresponding file path.
     */
    explicit QgsBookmarkManager( QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsBookmarkManager() override;

    /**
     * Initializes the bookmark manager.
     */
    void initialize( const QString &filePath );

    /**
     * Adds a \a bookmark to the manager.
     *
     * \param bookmark the bookmark to add
     * \param ok if specified, will be set to TRUE if the bookmark was successfully added, or
     * FALSE if the bookmark could not be added (eg as a result of a duplicate bookmark ID).
     *
     * \returns The bookmark's ID (or newly generated ID, if no ID was originally set and one was automatically generated)
     *
     * \see removeBookmark()
     * \see bookmarkAdded()
     */
    QString addBookmark( const QgsBookmark &bookmark, bool *ok SIP_OUT = nullptr );

    /**
     * Removes the bookmark with matching \a id from the manager.
     *
     * Returns TRUE if the removal was successful, or FALSE if the removal failed (eg as a result
     * of removing a bookmark which is not contained in the manager).
     *
     * \see addBookmark()
     * \see bookmarkRemoved()
     * \see bookmarkAboutToBeRemoved()
     * \see clear()
     */
    bool removeBookmark( const QString &id );

    /**
     * Updates the definition of a \a bookmark in the manager.
     *
     * Replaces the current definition of the bookmark with matching ID in the manager with
     * a new definition (new bookmark name or extent).
     *
     * Returns TRUE if the bookmark was successfully updated, or
     * FALSE if the bookmark could not be updated (eg bookmark is not stored in the manager).
     *
     * \see bookmarkChanged()
     */
    bool updateBookmark( const QgsBookmark &bookmark );

    /**
     * Removes and deletes all bookmarks from the manager.
     * \see removeBookmark()
     */
    void clear();

    /**
     * Returns a list of all bookmark groups contained in the manager.
     */
    QStringList groups() const;

    /**
     * Renames an existing group from \a oldName to \a newName. This updates
     * all existing bookmarks to reflect the new name.
     */
    void renameGroup( const QString &oldName, const QString &newName );

    /**
     * Returns a list of all bookmarks contained in the manager.
     */
    QList< QgsBookmark > bookmarks() const;

    /**
     * Returns the bookmark with a matching \a id, or an empty bookmark if no matching bookmarks
     * were found.
     */
    QgsBookmark bookmarkById( const QString &id ) const;

    /**
     * Returns a list of bookmark with a matching \a group, or an empty list if no matching bookmarks
     * were found.
     */
    QList< QgsBookmark > bookmarksByGroup( const QString &group );

    /**
     * Reads the manager's state from a DOM element, restoring all bookmarks
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
     * Moves the bookmark with matching \a id from this manager to a \a destination manager.
     *
     * Returns TRUE if the bookmark was successfully moved.
     */
    bool moveBookmark( const QString &id, QgsBookmarkManager *destination );

    /**
     * Exports all bookmarks from a list of \a managers to an xml file at the specified \a path.
     *
     * If \a group is set then only bookmarks from the matching group will be exported.
     *
     * Returns TRUE if the export was successful.
     *
     * \see importFromFile()
     */
    static bool exportToFile( const QString &path, const QList<const QgsBookmarkManager *> &managers, const QString &group = QString() );


    /**
     * Imports the bookmarks from an xml file at the specified \a path.
     *
     * Returns TRUE if the import was successful.
     *
     * \see exportToFile()
     */
    bool importFromFile( const QString &path );

  signals:

    //! Emitted when a bookmark is about to be added to the manager
    void bookmarkAboutToBeAdded( const QString &id );

    //! Emitted when a bookmark has been added to the manager
    void bookmarkAdded( const QString &id );

    //! Emitted when a bookmark was removed from the manager
    void bookmarkRemoved( const QString &id );

    //! Emitted when a bookmark is about to be removed from the manager
    void bookmarkAboutToBeRemoved( const QString &id );

    //! Emitted when a bookmark is changed
    void bookmarkChanged( const QString &id );

  private:

    QgsProject *mProject = nullptr;
    QString mFilePath;
    QList< QgsBookmark > mBookmarks;
    QStringList mGroups;

    void store();
    bool mInitialized = false;

};

#endif // QGSBOOKMARKMANAGER_H
