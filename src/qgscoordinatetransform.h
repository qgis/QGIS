#ifndef QGSCOORDINATETRANSFORM_H
#define QGSCOORDINATETRANSFORM_H
class QgsPoint;

class QgsCoordinateTransform{
 public:
    QgsCoordinateTransform(double mupp=0, double ymax = 0, double ymin=0,
			   double xmin = 0);
    ~QgsCoordinateTransform();
    QgsPoint transform(QgsPoint p);
    QgsPoint transform(double x, double y);
    void setMapUnitsPerPixel(double mupp);
    void setYmax(double ymax);
    void setYmin(double ymin);
    void setXmin(double xmin);
    void setParameters(double mupp, double xmin, double ymin, double ymax);
    QString showParameters();
 private:
    double mapUnitsPerPixel;
    double yMax;
    double yMin;
    double xMin;

};
#endif // QGSCOORDINATETRANSFORM_H
