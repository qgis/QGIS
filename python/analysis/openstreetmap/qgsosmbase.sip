/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/openstreetmap/qgsosmbase.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




typedef qint64 QgsOSMId;


struct QgsOSMElementID
{
  enum Type { Invalid, Node, Way, Relation };

  Type type;
  QgsOSMId id;
};


class QgsOSMElement
{
%Docstring
Elements (also data primitives) are the basic components in OpenStreetMap from which everything else
is defined. These consist of Nodes (which define a point in space), Ways (which define a linear features
and areas), and Relations - with an optional role - which are sometimes used to define the relation
between other elements. All of the above can have one of more associated tags.
%End

%TypeHeaderCode
#include "qgsosmbase.h"
%End
  public:
    QgsOSMElement();
    QgsOSMElement( QgsOSMElementID::Type t, QgsOSMId id );

    bool isValid() const;
%Docstring
 :rtype: bool
%End

    QgsOSMElementID elemId() const;
%Docstring
 :rtype: QgsOSMElementID
%End
    QgsOSMId id() const;
%Docstring
 :rtype: QgsOSMId
%End

};



class QgsOSMNode : QgsOSMElement
{
%Docstring
A node is one of the core elements in the OpenStreetMap data model. It consists of a single geospatial
point using a latitude and longitude. A third optional dimension, altitude, can be recorded; key:ele
and a node can also be defined at a particular layer=* or level=*. Nodes can be used to define standalone
point features or be used to define the path of a way.
%End

%TypeHeaderCode
#include "qgsosmbase.h"
%End
  public:
    QgsOSMNode();
    QgsOSMNode( QgsOSMId id, const QgsPoint &point );

    QgsPoint point() const;
%Docstring
 :rtype: QgsPoint
%End

};


class QgsOSMWay : QgsOSMElement
{
%Docstring
A way is an ordered list of nodes which normally also has at least one tag or is included within
a Relation. A way can have between 2 and 2,000 nodes, although it's possible that faulty ways with zero
or a single node exist. A way can be open or closed. A closed way is one whose last node on the way
is also the first on that way. A closed way may be interpreted either as a closed polyline, or an area,
or both.
%End

%TypeHeaderCode
#include "qgsosmbase.h"
%End
  public:
    QgsOSMWay();
    QgsOSMWay( QgsOSMId id, const QList<QgsOSMId> &nodes );

    QList<QgsOSMId> nodes() const;
%Docstring
 :rtype: list of QgsOSMId
%End


};



class QgsOSMTags
{
%Docstring
 This class is a container of tags for a node, way or a relation.
%End

%TypeHeaderCode
#include "qgsosmbase.h"
%End
  public:
    QgsOSMTags();

    int count() const;
%Docstring
 :rtype: int
%End
    QList<QString> keys() const;
%Docstring
 :rtype: list of str
%End
    bool contains( const QString &k ) const;
%Docstring
 :rtype: bool
%End
    void insert( const QString &k, const QString &v );
    QString value( const QString &k ) const;
%Docstring
 :rtype: str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/openstreetmap/qgsosmbase.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
