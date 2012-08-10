/***************************************************************************
    qgshistogramdiagram.h
    ---------------------
    begin                : August 2012
    copyright            : (C) 2012 by Matthias Kuhn
    email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHISTOGRAMDIAGRAM_H
#define QGSHISTOGRAMDIAGRAM_H

#include "qgsdiagram.h"
#include "qgsfeature.h"
#include <QPen>
#include <QBrush>

class QPainter;
class QPointF;
struct QgsDiagramSettings;
struct QgsDiagramInterpolationSettings;

class QgsRenderContext;


class CORE_EXPORT QgsHistogramDiagram: public QgsDiagram
{
  public:
    QgsHistogramDiagram();
    ~QgsHistogramDiagram();

    void renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );
    QSizeF diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s );
    QSizeF diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is );
    QString diagramName() const { return "Histogram"; }

  private:
    QBrush mCategoryBrush;
    QPen   mPen;
    double mScaleFactor;
};


#endif // QGSHISTOGRAMDIAGRAM_H
