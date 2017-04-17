/** \ingroup core
 * Represents the result of a snapping operation.
 * */
struct QgsSnappingResult
{
%TypeHeaderCode
#include <qgssnapper.h>
%End

  /** Snap to vertex, to segment or both*/
  enum SnappingType
  {
    SnapToVertex,
    SnapToSegment,
    //snap to vertex and also to segment if no vertex is within the search tolerance
    SnapToVertexAndSegment
  };

  /** The coordinates of the snapping result*/
  QgsPoint snappedVertex;
  /** The vertex index of snappedVertex
   or -1 if no such vertex number (e.g. snap to segment)*/
  int snappedVertexNr;
  /** The layer coordinates of the vertex before snappedVertex*/
  QgsPoint beforeVertex;
  /** The index of the vertex before snappedVertex
   or -1 if no such vertex*/
  int beforeVertexNr;
  /** The layer coordinates of the vertex after snappedVertex*/
  QgsPoint afterVertex;
  /** The index of the vertex after snappedVertex
   or -1 if no such vertex*/
  int afterVertexNr;
  /** Index of the snapped geometry*/
  qint64 snappedAtGeometry;
  /** Layer where the snap occurred*/
  const QgsVectorLayer* layer;
};
