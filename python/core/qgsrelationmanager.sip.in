/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrelationmanager.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRelationManager : QObject
{
%Docstring
This class manages a set of relations between layers.
%End

%TypeHeaderCode
#include "qgsrelationmanager.h"
%End
  public:

    explicit QgsRelationManager( QgsProject *project = 0 );
%Docstring
Constructor for QgsRelationManager.

:param project: associated project (used to notify project of changes)
%End

    void setRelations( const QList<QgsRelation> &relations );
%Docstring
Will set the specified relations and remove any relation currently set.

:param relations: A list of relations to set.
%End

    QMap<QString, QgsRelation> relations() const;
%Docstring
Get access to the relations managed by this class.

:return: A QMap where the key is the relation id, the value the relation object.
%End

    void addRelation( const QgsRelation &relation );
%Docstring
Add a relation.

:param relation: The relation to add.
%End

    void removeRelation( const QString &id );
%Docstring
Remove a relation.

:param id: The id of the relation to remove.
%End

    void removeRelation( const QgsRelation &relation );
%Docstring
Remove a relation.

:param relation: The relation to remove.
%End

    QgsRelation relation( const QString &id ) const;
%Docstring
Get access to a relation by its id.

:param id: The id to search for

:return: A relation. Invalid if not found.

.. seealso:: :py:func:`relationsByName`
%End

    QList<QgsRelation> relationsByName( const QString &name ) const;
%Docstring
Returns a list of relations with matching names.

:param name: relation name to search for. Searching is case insensitive.

:return: a list of matching relations

.. versionadded:: 2.16

.. seealso:: :py:func:`relation`
%End

    void clear();
%Docstring
Remove any relation managed by this class.
%End

    QList<QgsRelation> referencingRelations( const QgsVectorLayer *layer = 0, int fieldIdx = -2 ) const;
%Docstring
Get all relations where the specified layer (and field) is the referencing part (i.e. the child table with the foreign key).

:param layer:     The layer which should be searched for.
:param fieldIdx:  The field which should be part of the foreign key. If not set will return all relations.

:return: A list of relations matching the given layer and fieldIdx.
%End

    QList<QgsRelation> referencedRelations( QgsVectorLayer *layer = 0 ) const;
%Docstring
Get all relations where this layer is the referenced part (i.e. the parent table with the primary key being referenced from another layer).

:param layer:   The layer which should be searched for.

:return: A list of relations where the specified layer is the referenced part.
%End

    static QList<QgsRelation> discoverRelations( const QList<QgsRelation> &existingRelations, const QList<QgsVectorLayer *> &layers );
%Docstring
Discover all the relations available from the current layers.

:param existingRelations: the existing relations to filter them out
:param layers: the current layers

:return: the list of discovered relations

.. versionadded:: 3.0
%End

  signals:
    void relationsLoaded();
%Docstring
This signal is emitted when the relations were loaded after reading a project
%End

    void changed();
%Docstring
Emitted when relations are added or removed to the manager.

.. versionadded:: 2.5
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrelationmanager.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
