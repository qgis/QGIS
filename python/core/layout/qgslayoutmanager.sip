/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmanager.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutManager : QObject
{
%Docstring

.. versionadded:: 3.0

 Manages storage of a set of layouts.

QgsLayoutManager handles the storage, serializing and deserializing
of QgsLayouts. Usually this class is not constructed directly, but
rather accessed through a QgsProject via :py:func:`QgsProject.layoutManager()`

QgsLayoutManager retains ownership of all the layouts contained
in the manager.

.. note::

   Not available in Python bindings
%End

%TypeHeaderCode
#include "qgslayoutmanager.h"
%End
  public:

    explicit QgsLayoutManager( QgsProject *project /TransferThis/ = 0 );
%Docstring
Constructor for QgsLayoutManager. The project will become the parent object for this
manager.
%End

    ~QgsLayoutManager();


    bool addLayout( QgsMasterLayoutInterface *layout /Transfer/ );
%Docstring
Adds a ``layout`` to the manager. Ownership of the layout is transferred to the manager.
Returns true if the addition was successful, or false if the layout could not be added (eg
as a result of a duplicate layout name).

.. seealso:: :py:func:`removeLayout`

.. seealso:: :py:func:`layoutAdded`
%End


    bool removeLayout( QgsMasterLayoutInterface *layout );
%Docstring
Removes a ``layout`` from the manager. The layout is deleted.
Returns true if the removal was successful, or false if the removal failed (eg as a result
of removing a layout which is not contained in the manager).

.. seealso:: :py:func:`addLayout`

.. seealso:: :py:func:`layoutRemoved`

.. seealso:: :py:func:`layoutAboutToBeRemoved`

.. seealso:: :py:func:`clear`
%End

    void clear();
%Docstring
Removes and deletes all layouts from the manager.

.. seealso:: :py:func:`removeLayout`
%End


    QList< QgsMasterLayoutInterface * > layouts() const;
%Docstring
Returns a list of all layouts contained in the manager.
%End


    QgsMasterLayoutInterface *layoutByName( const QString &name ) const;
%Docstring
Returns the layout with a matching name, or None if no matching layouts
were found.
%End

    bool readXml( const QDomElement &element, const QDomDocument &doc );
%Docstring
Reads the manager's state from a DOM element, restoring all layouts
present in the XML document.

.. seealso:: :py:func:`writeXml`
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
Returns a DOM element representing the state of the manager.

.. seealso:: :py:func:`readXml`
%End

    bool saveAsTemplate( const QString &name, QDomDocument &doc ) const;
%Docstring
Saves the composition with matching ``name`` in template format.
Returns true if save was successful.
%End

    QgsMasterLayoutInterface *duplicateLayout( const QgsMasterLayoutInterface *layout, const QString &newName );
%Docstring
Duplicates an existing ``layout`` from the manager. The new
layout will automatically be stored in the manager.
Returns new the layout if duplication was successful.
%End

    QString generateUniqueComposerTitle() const;
%Docstring
Generates a unique title for a new composition, which does not
clash with any already contained by the manager.
%End

    QString generateUniqueTitle( QgsMasterLayoutInterface::Type type = QgsMasterLayoutInterface::PrintLayout ) const;
%Docstring
Generates a unique title for a new layout of the specified ``type``, which does not
clash with any already contained by the manager.
%End

  signals:

    void compositionAboutToBeAdded( const QString &name );
%Docstring
Emitted when a composition is about to be added to the manager
%End

    void layoutAboutToBeAdded( const QString &name );
%Docstring
Emitted when a layout is about to be added to the manager
%End

    void compositionAdded( const QString &name );
%Docstring
Emitted when a composition has been added to the manager
%End

    void layoutAdded( const QString &name );
%Docstring
Emitted when a layout has been added to the manager
%End

    void compositionRemoved( const QString &name );
%Docstring
Emitted when a composition was removed from the manager
%End

    void layoutRemoved( const QString &name );
%Docstring
Emitted when a layout was removed from the manager
%End

    void compositionAboutToBeRemoved( const QString &name );
%Docstring
Emitted when a composition is about to be removed from the manager
%End

    void layoutAboutToBeRemoved( const QString &name );
%Docstring
Emitted when a layout is about to be removed from the manager
%End


    void layoutRenamed( QgsMasterLayoutInterface *layout, const QString &newName );
%Docstring
Emitted when a layout is renamed
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmanager.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
