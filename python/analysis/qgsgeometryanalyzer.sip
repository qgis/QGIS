/** \ingroup analysis
 * The QGis class provides vector geometry analysis functions
 */

class QgsGeometryAnalyzer
{
%TypeHeaderCode
#include <qgsgeometryanalyzer.h>
%End

  public:

    /**
     * Simplify vector layer using (a modified) Douglas-Peucker algorithm
     * and write it to a new shape file
     */
    bool simplify( QgsVectorLayer* layer, const QString& shapefileName, double tolerance,
                 bool onlySelectedFeatures = false );

    /**Calculate the true centroids, or 'center of mass' for a vector layer and 
       write it to a new shape file
    */
    bool centroids( QgsVectorLayer* layer, const QString& shapefileName,
                 bool onlySelectedFeatures = false );

    /**Create a polygon based on the extent of all (selected) features and write it to a new shape file
    */
    bool extent( QgsVectorLayer* layer, const QString& shapefileName, 
                 bool onlySelectedFeatures = false );

    /**Create buffers for a vector layer and write it to a new shape file
    */
    bool buffer( QgsVectorLayer* layer, const QString& shapefileName, double bufferDistance,
                 bool onlySelectedFeatures = false, bool dissolve = false, 
                 int bufferDistanceField = -1 );

    /**Create convex hull(s) of a vector layer and write it to a new shape file
    */
    bool convexHull( QgsVectorLayer* layer, const QString& shapefileName, 
                     bool onlySelectedFeatures = false,
                     int uniqueIdField = -1 );

    /**Dissolve a vector layer and write it to a new shape file
    */
    bool dissolve( QgsVectorLayer* layer, const QString& shapefileName, 
                   bool onlySelectedFeatures = false,
                     int uniqueIdField = -1 );

    /**Creates an event layer (multipoint or multiline). Note that currently (until QgsGeometry supports m-values) the z-coordinate of the line layer is used for linear referencing
      @param lineLayer layer with the line geometry
      @param eventLayer layer with features and location field
      @param lineField join index in line layer
      @param eventField join index in event layer
      @param outputLayer name of output file (can be empty if a memory layer is used)
      @param outputFormat name of output format (can be empty if a memory provider is used to store the results)
      @param unlocatedFeatureIds out: ids of event features where linear referencing was not successful
      @param locationField1 attribute index of location field in event layer
      @param locationField2 attribute index of location end field (or -1 for point layer)
      @param memoryProvider memory provider to write output to (can be 0 if output is written to a file)
      @param p progress dialog or 0 if no progress dialog should be shown
    */
    bool eventLayer( QgsVectorLayer* lineLayer, QgsVectorLayer* eventLayer, int lineField, int eventField, QList<int>& unlocatedFeatureIds /Out/,
                     const QString& outputLayer, const QString& outputFormat, int locationField1, int locationField2 = -1, int offsetField = -1, double offsetScale = 1.0,
                     bool forceSingleGeometry = false, QgsVectorDataProvider* memoryProvider = 0, QProgressDialog* p = 0 );

    /**Returns multilinestring*/
    QgsGeometry* locateBetweenMeasures( double fromMeasure, double toMeasure, QgsGeometry* lineGeom );
    /**Returns multipoint*/
    QgsGeometry* locateAlongMeasure( double measure, QgsGeometry* lineGeom );

  private:

    QList<double> simpleMeasure( QgsGeometry* geometry );
    double perimeterMeasure( QgsGeometry* geometry, QgsDistanceArea& measure );
    /**Helper function to simplify an individual feature*/
    void simplifyFeature( QgsFeature& f, QgsVectorFileWriter* vfw, double tolerance );
    /**Helper function to get the cetroid of an individual feature*/
    void centroidFeature( QgsFeature& f, QgsVectorFileWriter* vfw );
    /**Helper function to buffer an individual feature*/
    void bufferFeature( QgsFeature& f, int nProcessedFeatures, QgsVectorFileWriter* vfw, 
                        bool dissolve, QgsGeometry** dissolveGeometry,
                        double bufferDistance, int bufferDistanceField );
    /**Helper function to get the convex hull of feature(s)*/
    void convexFeature( QgsFeature& f, int nProcessedFeatures, 
                        QgsGeometry** dissolveGeometry );
    /**Helper function to dissolve feature(s)*/
    void dissolveFeature( QgsFeature& f, int nProcessedFeatures, 
                          QgsGeometry** dissolveGeometry );
};
