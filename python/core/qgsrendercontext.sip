
class QgsRenderContext
{

%TypeHeaderCode
#include <qgsrendercontext.h>
%End

 public:
  QgsRenderContext();
  ~QgsRenderContext();

  //getters

  QPainter* painter();

  const QgsCoordinateTransform* coordinateTransform() const;

  const QgsRect& extent() const;

  const QgsMapToPixel& mapToPixel() const;

  double scaleFactor() const;

  double rasterScaleFactor() const;
  
  bool renderingStopped() const;

  bool forceVectorOutput() const;

  bool drawEditingInformation() const;

  //setters

  /**Sets coordinate transformation. QgsRenderContext takes ownership and deletes if necessary*/
  void setCoordTransform(QgsCoordinateTransform* t);
  void setMapToPixel(const QgsMapToPixel& mtp);
  void setExtent(const QgsRect& extent);
  void setDrawEditingInformation(bool b);
  void setRenderingStopped(bool stopped);
  void setScaleFactor(double factor);
  void setRasterScaleFactor(double factor);
  void setPainter(QPainter* p);
};
