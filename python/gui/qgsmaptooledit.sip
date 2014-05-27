class QgsMapToolEdit: public QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptooledit.h>
%End
  public:
    QgsMapToolEdit( QgsMapCanvas* canvas /TransferThis/ );
    virtual ~QgsMapToolEdit();

    virtual bool isEditTool();

  protected:
    /**Inserts vertices to the snapped segments of the editing layer.
     This is useful for topological editing if snap to segment is enabled.
     @param snapResults results collected from the snapping operation
     @param editedLayer pointer to the editing layer
     @return 0 in case of success*/
    int insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults, QgsVectorLayer* editedLayer );

    /**Extracts a single snapping point from a set of snapping results.
       This is useful for snapping operations that just require a position to snap to and not all the
       snapping results. If the list is empty, the screen coordinates are transformed into map coordinates and returned
       @param snapResults results collected from the snapping operation.
       @return the snapped point in map coordinates*/
    QgsPoint snapPointFromResults( const QList<QgsSnappingResult>& snapResults, const QPoint& screenCoords );

    /** Creates a rubber band with the color/line width from
    *   the QGIS settings. The caller takes ownership of the
    *   returned object
    *   @param geometryType
    *   @param alternativeBand if true, rubber band will be set with more transparency and a dash pattern. defaut is false.
    */
    QgsRubberBand* createRubberBand( QGis::GeometryType geometryType = QGis::Line , bool alternativeBand = false );

    /**Returns the current vector layer of the map canvas or 0*/
    QgsVectorLayer* currentVectorLayer();

    /**Adds vertices to other features to keep topology up to date, e.g. to neighbouring polygons.
       @param geom list of points (in layer coordinate system)
       @return 0 in case of success*/
    int addTopologicalPoints( const QList<QgsPoint>& geom );

    /**Display a timed message bar noting the active layer is not vector.
       @note added in QGIS 1.9*/
    void notifyNotVectorLayer();
    /**Display a timed message bar noting the active vector layer is not editable.
       @note added in QGIS 1.9*/
    void notifyNotEditableLayer();
};
