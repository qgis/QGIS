class QgsPieDiagram: QgsDiagram
{
%TypeHeaderCode
#include <qgspiediagram.h>
%End
  public:
    QgsPieDiagram();
    ~QgsPieDiagram();

    virtual QgsDiagram* clone() const;

    void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );

    QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s );
    QSizeF diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is );
    QString diagramName() const;
};
