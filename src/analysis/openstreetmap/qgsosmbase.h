#ifndef OSMBASE_H
#define OSMBASE_H

#include <QString>

#include "qgspoint.h"

#include <sqlite3.h>

typedef qint64 QgsOSMId;

class QgsOSMDatabase;

struct QgsOSMElementID
{
  enum Type { Invalid, Node, Way, Relation };

  Type type;
  QgsOSMId id;
};


/**
Elements (also data primitives) are the basic components in OpenStreetMap from which everything else
is defined. These consist of Nodes (which define a point in space), Ways (which define a linear features
and areas), and Relations - with an optional role - which are sometimes used to define the relation
between other elements. All of the above can have one of more associated tags.
*/
class ANALYSIS_EXPORT QgsOSMElement
{
  public:
    QgsOSMElement() { mElemID.type = QgsOSMElementID::Invalid; mElemID.id = 0; }
    QgsOSMElement( QgsOSMElementID::Type t, QgsOSMId id ) { mElemID.type = t; mElemID.id = id; }

    bool isValid() const { return mElemID.type != QgsOSMElementID::Invalid; }

    QgsOSMDatabase* database() const;

    // fetched automatically from DB
    QgsOSMElementID elemID() const { return mElemID; }
    int id() const { return mElemID.id; }
    //QString username() const;
    //QDateTime timestamp() const;
    //int version() const;

  private:
    QgsOSMElementID mElemID;
};



/**
A node is one of the core elements in the OpenStreetMap data model. It consists of a single geospatial
point using a latitude and longitude. A third optional dimension, altitude, can be recorded; key:ele
and a node can also be defined at a particular layer=* or level=*. Nodes can be used to define standalone
point features or be used to define the path of a way.
*/
class ANALYSIS_EXPORT QgsOSMNode : public QgsOSMElement
{
  public:
    QgsOSMNode() : mPoint() {}
    QgsOSMNode( QgsOSMId id, const QgsPoint& point ) : QgsOSMElement( QgsOSMElementID::Node, id ), mPoint( point ) {}

    QgsPoint point() const { return mPoint; }

    // fetched on-demand
    QList<QgsOSMElementID> ways() const; // where the node participates?
    QList<QgsOSMElementID> relations() const;

  private:
    QgsPoint mPoint;
};


/**
A way is an ordered list of nodes which normally also has at least one tag or is included within
a Relation. A way can have between 2 and 2,000 nodes, although it's possible that faulty ways with zero
or a single node exist. A way can be open or closed. A closed way is one whose last node on the way
is also the first on that way. A closed way may be interpreted either as a closed polyline, or an area,
or both.
*/
class ANALYSIS_EXPORT QgsOSMWay : public QgsOSMElement
{
  public:
    QgsOSMWay() {}
    QgsOSMWay( QgsOSMId id, const QList<QgsOSMId> nodes ) : QgsOSMElement( QgsOSMElementID::Way, id ), mNodes( nodes ) {}

    QList<QgsOSMId> nodes() const { return mNodes; }

    // fetched on-demand
    //QList<OSMElementID> relations() const;

  private:
    QList<QgsOSMId> mNodes;
};


#if 0
/**
A relation is one of the core data elements that consists of one or more tags and also an ordered list
of one or more nodes and/or ways as members which is used to define logical or geographic relationships
between other elements. A member of a relation can optionally have a role which describe the part that
a particular feature plays within a relation.
*/
class ANALYSIS_EXPORT QgsOSMRelation : public QgsOSMElement
{
  public:
    QString relationType() const;

    QList< QPair<QgsOSMElementID, QString> > members() const;
};
#endif

/**
 * This class is a container of tags for a node, way or a relation.
 */
class ANALYSIS_EXPORT QgsOSMTags
{
  public:
    QgsOSMTags() {}

    int count() const { return mMap.count(); }
    QList<QString> keys() const { return mMap.keys(); }
    bool contains( const QString& k ) const { return mMap.contains( k ); }
    void insert( const QString& k, const QString& v ) { mMap.insert( k, v ); }
    QString value( const QString& k ) const { return mMap.value( k ); }

  private:
    QMap<QString, QString> mMap;
};

#endif // OSMBASE_H
