/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgslayoutmanager.h                                 *
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
rather accessed through a QgsProject via QgsProject.layoutManager().

QgsLayoutManager retains ownership of all the layouts contained
in the manager.
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

    bool addComposition( QgsComposition *composition /Transfer/ );
%Docstring
Adds a composition to the manager. Ownership of the composition is transferred to the manager.
Returns true if the addition was successful, or false if the composition could not be added (eg
as a result of a duplicate composition name).

:rtype: bool

.. seealso:: :py:func:`removeComposition()`

.. seealso:: :py:func:`compositionAdded()`
%End

    bool addLayout( QgsLayout *layout /Transfer/ );
%Docstring
Adds a ``layout`` to the manager. Ownership of the layout is transferred to the manager.
Returns true if the addition was successful, or false if the layout could not be added (eg
as a result of a duplicate layout name).

:rtype: bool

.. seealso:: :py:func:`removeLayout()`

.. seealso:: :py:func:`layoutAdded()`
%End

    bool removeComposition( QgsComposition *composition );
%Docstring
Removes a composition from the manager. The composition is deleted.
Returns true if the removal was successful, or false if the removal failed (eg as a result
of removing a composition which is not contained in the manager).

:rtype: bool

.. seealso:: :py:func:`addComposition()`

.. seealso:: :py:func:`compositionRemoved()`

.. seealso:: :py:func:`compositionAboutToBeRemoved()`

.. seealso:: :py:func:`clear()`
%End

    bool removeLayout( QgsLayout *layout );
%Docstring
Removes a ``layout`` from the manager. The layout is deleted.
Returns true if the removal was successful, or false if the removal failed (eg as a result
of removing a layout which is not contained in the manager).

:rtype: bool

.. seealso:: :py:func:`addLayout()`

.. seealso:: :py:func:`layoutRemoved()`

.. seealso:: :py:func:`layoutAboutToBeRemoved()`

.. seealso:: :py:func:`clear()`
%End

    void clear();
%Docstring
Removes and deletes all layouts from the manager.

.. seealso:: :py:func:`removeLayout()`
%End

    QList< QgsComposition * > compositions() const;
%Docstring
Returns a list of all compositions contained in the manager.

:rtype: list of QgsComposition
%End

    QList< QgsLayout * > layouts() const;
%Docstring
Returns a list of all layouts contained in the manager.

:rtype: list of QgsLayout
%End

    QgsComposition *compositionByName( const QString &name ) const;
%Docstring
Returns the composition with a matching name, or None if no matching compositions
were found.

:rtype: QgsComposition
%End

    QgsLayout *layoutByName( const QString &name ) const;
%Docstring
Returns the layout with a matching name, or None if no matching layouts
were found.

:rtype: QgsLayout
%End

    bool readXml( const QDomElement &element, const QDomDocument &doc );
%Docstring
Reads the manager's state from a DOM element, restoring all layouts
present in the XML document.

:rtype: bool

.. seealso:: :py:func:`writeXml()`
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
Returns a DOM element representing the state of the manager.

:rtype: QDomElement

.. seealso:: :py:func:`readXml()`
%End

    bool saveAsTemplate( const QString &name, QDomDocument &doc ) const;
%Docstring
Saves the composition with matching ``name`` in template format.
Returns true if save was successful.

:rtype: bool
%End

    QgsComposition *duplicateComposition( const QString &name, const QString &newName );
%Docstring
Duplicates an existing composition from the manager. The new
composition will automatically be stored in the manager.
Returns new composition if duplication was successful.

:rtype: QgsComposition
%End

    QgsLayout *duplicateLayout( const QgsLayout *layout, const QString &newName );
%Docstring
Duplicates an existing ``layout`` from the manager. The new
layout will automatically be stored in the manager.
Returns new the layout if duplication was successful.

:rtype: QgsLayout
%End

    QString generateUniqueComposerTitle() const;
%Docstring
Generates a unique title for a new composition, which does not
clash with any already contained by the manager.

:rtype: str
%End

    QString generateUniqueTitle() const;
%Docstring
Generates a unique title for a new layout, which does not
clash with any already contained by the manager.

:rtype: str
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

    void compositionRenamed( QgsComposition *composition, const QString &newName );
%Docstring
Emitted when a composition is renamed
%End

    void layoutRenamed( QgsLayout *layout, const QString &newName );
%Docstring
Emitted when a layout is renamed
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgslayoutmanager.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
