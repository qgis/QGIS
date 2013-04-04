class QgsPieDiagram: QgsDiagram
{
%TypeHeaderCode
#include <qgspiediagram.h>
%End
  public:
    QgsPieDiagram();
    ~QgsPieDiagram();

    void renderDiagram( const QgsAttributes& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );
    QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s );
    QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is );
    QString diagramName() const;
};
