
/**Describes the result of a snapping operation*/
struct QgsSnappingResult
{
%TypeHeaderCode
#include <qgssnapper.h>
%End
  
  /**The coordinates of the snapping result*/
  QgsPoint snappedVertex;
  /**The vertex index of snappedVertex 
   or -1 if no such vertex number (e.g. snap to segment)*/
  int snappedVertexNr;
  /**The layer coordinates of the vertex before snappedVertex*/
  QgsPoint beforeVertex;
  /**The index of the vertex before snappedVertex
   or -1 if no such vertex*/
  int beforeVertexNr;
  /**The layer coordinates of the vertex after snappedVertex*/
  QgsPoint afterVertex;
  /**The index of the vertex after snappedVertex
   or -1 if no such vertex*/
  int afterVertexNr;
  /**Index of the snapped geometry*/
  int snappedAtGeometry;
  /**Layer where the snap occured*/
  const QgsVectorLayer* layer;
};



/**A class that allows advanced snapping operations on a set of vector layers*/
class QgsSnapper
{
%TypeHeaderCode
#include <qgssnapper.h>
%End
  
public:
  /**Snap to vertex, to segment or both*/
  enum SNAP_TO
    {
      SNAP_TO_VERTEX,
      SNAP_TO_SEGMENT,
      //snap to vertex and also to segment if no vertex is within the search tolerance
      SNAP_TO_VERTEX_AND_SEGMENT
    };
  
  enum SNAP_MODE
    {
      /**Only one snapping result is retured*/
      ONE_RESULT,
      /**Several snapping results which have the same position are returned. This is usefull for topological
       editing*/
      SEVERAL_RESULTS_SAME_POSITION,
      /**All results within the given layer tolerances are returned*/
      ALL_RESULTS_WITHIN_GIVEN_TOLERANCES
    };

  QgsSnapper(QgsMapRender* mapRender);
  ~QgsSnapper();
  /**Does the snapping operation
   @param startPoint the start point for snapping (in pixel coordinates)
  @param snappingResult the list where the results are inserted (everything in map coordinate system)
  @return 0 in case of success*/
  int snapPoint(const QPoint& startPoint, QList<QgsSnappingResult>& snappingResult);

  //setters
  void setLayersToSnap(const QList<QgsVectorLayer*>& layerList);
  void setTolerances(const QList<double>& toleranceList);
  // TODO wrap void setSnapToList(const QList<QgsSnapper::SNAP_TO>& snapToList);
  void setSnapMode(QgsSnapper::SNAP_MODE snapMode);

};
