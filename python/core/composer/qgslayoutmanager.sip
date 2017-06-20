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

 Manages storage of a set of compositions.

 QgsLayoutManager handles the storage, serializing and deserializing
 of QgsCompositions. Usually this class is not constructed directly, but
 rather accessed through a QgsProject via QgsProject.layoutManager().

 QgsLayoutManager retains ownership of all the compositions contained
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
.. seealso:: removeComposition()
.. seealso:: compositionAdded()
 :rtype: bool
%End

    bool removeComposition( QgsComposition *composition );
%Docstring
 Removes a composition from the manager. The composition is deleted.
 Returns true if the removal was successful, or false if the removal failed (eg as a result
 of removing a composition which is not contained in the manager).
.. seealso:: addComposition()
.. seealso:: compositionRemoved()
.. seealso:: compositionAboutToBeRemoved()
.. seealso:: clear()
 :rtype: bool
%End

    void clear();
%Docstring
 Removes and deletes all compositions from the manager.
.. seealso:: removeComposition()
%End

    QList< QgsComposition * > compositions() const;
%Docstring
 Returns a list of all compositions contained in the manager.
 :rtype: list of QgsComposition
%End

    QgsComposition *compositionByName( const QString &name ) const;
%Docstring
 Returns the composition with a matching name, or None if no matching compositions
 were found.
 :rtype: QgsComposition
%End

    bool readXml( const QDomElement &element, const QDomDocument &doc );
%Docstring
 Reads the manager's state from a DOM element, restoring all compositions
 present in the XML document.
.. seealso:: writeXml()
 :rtype: bool
%End

    QDomElement writeXml( QDomDocument &doc ) const;
%Docstring
 Returns a DOM element representing the state of the manager.
.. seealso:: readXml()
 :rtype: QDomElement
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

    QString generateUniqueTitle() const;
%Docstring
 Generates a unique title for a new composition, which does not
 clash with any already contained by the manager.
 :rtype: str
%End

  signals:

    void compositionAboutToBeAdded( const QString &name );
%Docstring
Emitted when a composition is about to be added to the manager
%End

    void compositionAdded( const QString &name );
%Docstring
Emitted when a composition has been added to the manager
%End

    void compositionRemoved( const QString &name );
%Docstring
Emitted when a composition was removed from the manager
%End

    void compositionAboutToBeRemoved( const QString &name );
%Docstring
Emitted when a composition is about to be removed from the manager
%End

    void compositionRenamed( QgsComposition *composition, const QString &newName );
%Docstring
Emitted when a composition is renamed
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgslayoutmanager.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
