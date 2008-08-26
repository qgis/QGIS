
/*! \class QgsMapToPixel
* \brief Class for doing transforms between map coordinates and device coordinates.
*
* This class can convert device coordinates to map coordinates and vice versa.
*/
class QgsMapToPixel
{
%TypeHeaderCode
#include <qgsmaptopixel.h>
%End

 public:
 /* Constructor
 * @param mapUnitsPerPixel Map units per pixel
 * @param ymax Maximum y value of the map canvas
 * @param ymin Minimum y value of the map canvas
 * @param xmin Minimum x value of the map canvas
 */
    QgsMapToPixel(double mapUnitsPerPixel=0, double ymax = 0, double ymin=0,
			   double xmin = 0);
		 //! destructor
    ~QgsMapToPixel();
    /*! Transform the point from map (world) coordinates to device coordinates
    * @param p Point to transform
    * @return QgsPoint in device coordinates
    */
    QgsPoint transform(const QgsPoint& p);
    void transform(QgsPoint* p);
    /*! Transform the point specified by x,y from map (world) 
     * coordinates to device coordinates
     * @param x x cordinate o point to transform
     * @param y y coordinate of point to transform
     * @return QgsPoint in device coordinates
    */
    QgsPoint transform(double x, double y);
       /*! Tranform device coordinates to map (world)  coordinates
    * @param x x coordinate of point to be converted to map cooordinates
    * @param y y coordinate of point to be converted to map cooordinates
    * @return QgsPoint in map coordinates
    */

    /* Transform device coordinates to map coordinates. Modifies the
       given coordinates in place. Intended as a fast way to do the
       transform. */ 
    void transformInPlace(double& x, double& y);

    /* Transform device coordinates to map coordinates. Modifies the
       given coordinates in place. Intended as a fast way to do the
       transform. */ 
    // TODO: not supported argument, must solve manually
    //void transformInPlace(std::vector<double>& x, 
		//	  std::vector<double>& y);

    QgsPoint toMapCoordinates(int x, int y);
     /*! Tranform device coordinates to map (world)  coordinates
    * @param p Point to be converted to map cooordinates
    * @return QgsPoint in map coorndiates
    */
    QgsPoint toMapCoordinates(QPoint p);
    
    QgsPoint toMapPoint(int x, int y);
    /*! Set map units per pixel
    * @param mapUnitsPerPixel Map units per pixel
    */
    void setMapUnitsPerPixel(double mapUnitsPerPixel);
    
    //! Return current map units per pixel
    double mapUnitsPerPixel();

    //! Set maximum y value
    void setYMaximum(double ymax);
    //! Set minimum y value
    void setYMinimum(double ymin);
    //! set minimum x value
    void setXMinimum(double xmin);
    /*! Set parameters for use in tranfsorming coordinates
    * @param mapUnitsPerPixel Map units per pixel
    * @param xmin Minimum x value
    * @param ymin Minimum y value
    * @param ymax Maximum y value
    */
    void setParameters(double mapUnitsPerPixel, double xmin, double ymin, double ymax);
    //! String representation of the parameters used in the transform
    QString showParameters();

};


