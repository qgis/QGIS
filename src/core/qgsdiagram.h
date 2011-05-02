#ifndef QGSDIAGRAM_H
#define QGSDIAGRAM_H

#include "qgsfeature.h"
#include <QPen>
#include <QBrush>

class QPainter;
class QPointF;
struct QgsDiagramSettings;

class QgsRenderContext;

/**Base class for all diagram types*/
class CORE_EXPORT QgsDiagram
{
  public:
    /**Draws the diagram at the given position (in pixel coordinates)*/
    virtual void renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position ) = 0;
    virtual QString diagramName() const = 0;

  protected:
    void setPenWidth( QPen& pen, const QgsDiagramSettings& s, const QgsRenderContext& c );
    QSizeF sizePainterUnits( const QSizeF& size, const QgsDiagramSettings& s, const QgsRenderContext& c );
    QFont scaledFont( const QgsDiagramSettings& s, const QgsRenderContext& c );
};

class CORE_EXPORT QgsTextDiagram: public QgsDiagram
{
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
    void renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );

    QString diagramName() const { return "Text"; }

  private:
    Orientation mOrientation;
    Shape mShape;
    QBrush mBrush; //transparent brush
    QPen mPen;

    /**Calculates intersection points between a line and an ellipse
      @return intersection points*/
    void lineEllipseIntersection( const QPointF& lineStart, const QPointF& lineEnd, const QPointF& ellipseMid, double r1, double r2, QList<QPointF>& result ) const;
};

class CORE_EXPORT QgsPieDiagram: public QgsDiagram
{
  public:
    QgsPieDiagram();
    ~QgsPieDiagram();

    void renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );
    QString diagramName() const { return "Pie"; }

  private:
    QBrush mCategoryBrush;
    QPen mPen;
};

#endif // QGSDIAGRAM_H
