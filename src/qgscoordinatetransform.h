#ifndef QGSCOORDINATETRANSFORM_H
#define QGSCOORDINATETRANSFORM_H
class QgsPoint;

class QgsCoordinateTransform{
 public:
    QgsCoordinateTransform();
    ~QgsCoordinateTransform();
    QgsPoint transform(QgsPoint p);
    QgsPoint transform(double x, double y);
 private:
    double mapUnitsPerPixel;
    double yMax;
    double yMin;
    double xMin;

};
#endif // QGSCOORDINATETRANSFORM_H
