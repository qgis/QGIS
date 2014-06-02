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
    virtual QgsDiagram* clone() const;

    void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );

    QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s );
    QSizeF diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is );

    QString diagramName() const;
};
