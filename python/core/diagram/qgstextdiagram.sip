class QgsTextDiagram: QgsDiagram
{
%TypeHeaderCode
#include <qgstextdiagram.h>
%End

  public:
    enum Shape
    {
      Circle = 0,
      Rectangle,
      Triangle
    };

    enum Orientation
    {
      Horizontal = 0,
      Vertical
    };

    QgsTextDiagram();
    ~QgsTextDiagram();
    void renderDiagram( const QgsAttributes& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );
    QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s );
    QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is );

    QString diagramName() const;
};
