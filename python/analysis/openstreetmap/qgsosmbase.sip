typedef qint64 QgsOSMId;

struct QgsOSMElementID
{
%TypeHeaderCode
#include <qgsosmbase.h>
%End
  enum Type { Invalid, Node, Way, Relation };

  Type type;
  qint64 id;
};


/**
Elements (also data primitives) are the basic components in OpenStreetMap from which everything else
is defined. These consist of Nodes (which define a point in space), Ways (which define a linear features
and areas), and Relations - with an optional role - which are sometimes used to define the relation
between other elements. All of the above can have one of more associated tags.
*/
class QgsOSMElement
{

%TypeHeaderCode
#include <qgsosmbase.h>
%End

  public:
    QgsOSMElement();
    QgsOSMElement( QgsOSMElementID::Type t, qint64 id );

    bool isValid() const;

    // fetched automatically from DB
    QgsOSMElementID elemID() const;
    qint64 id() const;
    //QString username() const;
    //QDateTime timestamp() const;
    //int version() const;

};



/**
A node is one of the core elements in the OpenStreetMap data model. It consists of a single geospatial
point using a latitude and longitude. A third optional dimension, altitude, can be recorded; key:ele
and a node can also be defined at a particular layer=* or level=*. Nodes can be used to define standalone
point features or be used to define the path of a way.
*/
class QgsOSMNode : QgsOSMElement
{
%TypeHeaderCode
#include <qgsosmbase.h>
%End
  public:
    QgsOSMNode();
    QgsOSMNode( qint64 id, const QgsPoint& point );

    QgsPoint point() const;

};


/**
A way is an ordered list of nodes which normally also has at least one tag or is included within
a Relation. A way can have between 2 and 2,000 nodes, although it's possible that faulty ways with zero
or a single node exist. A way can be open or closed. A closed way is one whose last node on the way
is also the first on that way. A closed way may be interpreted either as a closed polyline, or an area,
or both.
*/
class QgsOSMWay : QgsOSMElement
{
%TypeHeaderCode
#include <qgsosmbase.h>
%End
  public:
    QgsOSMWay();
    QgsOSMWay( qint64 id, const QList<qint64>& nodes );

    QList<qint64> nodes() const;

    // fetched on-demand
    //QList<OSMElementID> relations() const;

};

/**
 * This class is a container of tags for a node, way or a relation.
 */
class QgsOSMTags
{
%TypeHeaderCode
#include <qgsosmbase.h>
%End
  public:
    QgsOSMTags();

    int count() const;
    QList<QString> keys() const;
    bool contains( const QString& k ) const;
    void insert( const QString& k, const QString& v );
    QString value( const QString& k ) const;

};
