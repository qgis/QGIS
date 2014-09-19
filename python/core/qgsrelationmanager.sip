/**
 * This class manages a set of relations between layers.
 */
class QgsRelationManager : QObject
{
%TypeHeaderCode
#include "qgsrelationmanager.h"
%End
  public:
    explicit QgsRelationManager( QgsProject *project );

    void setRelations( const QList<QgsRelation>& relations );
    const QMap<QString, QgsRelation>& relations() const;
    void addRelation( const QgsRelation& relation );
    void removeRelation( const QString& id );
    void removeRelation( const QgsRelation& relation );
    QgsRelation relation( const QString& id ) const;
    void clear();

    QList<QgsRelation> referencingRelations( QgsVectorLayer *layer = 0, int fieldIdx = -2 ) const;
    QList<QgsRelation> referencedRelations( QgsVectorLayer *layer = 0 ) const;

  signals:
    /** this signal is emitted when the relations were loaded after reading a project */
    void relationsLoaded();

    /** this signal is emitted whenever the relations in the project changed */
    void relationsChanged();
};
