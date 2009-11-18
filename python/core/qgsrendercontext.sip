
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

  const QgsRectangle& extent() const;

  const QgsMapToPixel& mapToPixel() const;

  double scaleFactor() const;

  double rasterScaleFactor() const;
  
  bool renderingStopped() const;

  bool forceVectorOutput() const;

  bool drawEditingInformation() const;

  double rendererScale() const;

  //! Added in QGIS v1.4
  QgsLabelingEngineInterface* labelingEngine();

  //setters

  /**Sets coordinate transformation. QgsRenderContext takes ownership and deletes if necessary*/
  void setCoordinateTransform(QgsCoordinateTransform* t);
  void setMapToPixel(const QgsMapToPixel& mtp);
  void setExtent(const QgsRectangle& extent);
  void setDrawEditingInformation(bool b);
  void setRenderingStopped(bool stopped);
  void setScaleFactor(double factor);
  void setRasterScaleFactor(double factor);
  void setRendererScale( double scale );
  void setPainter(QPainter* p);
  //! Added in QGIS v1.4
  void setLabelingEngine(QgsLabelingEngineInterface* iface);
};
