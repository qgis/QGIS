class QgsTextDiagram: QgsDiagram
{
%TypeHeaderCode
#include <qgstextdiagram.h>
%End

  public:
    enum Shape
    {
      Circle,
      Rectangle,
      Triangle
    };

    enum Orientation
    {
      Horizontal,
      Vertical
    };

    QgsTextDiagram();
    ~QgsTextDiagram();
    virtual QgsTextDiagram* clone() const /Factory/;

    void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, QPointF position );

    QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s );
    QSizeF diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is );
    double legendSize( double value, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is ) const;

    QString diagramName() const;
};
