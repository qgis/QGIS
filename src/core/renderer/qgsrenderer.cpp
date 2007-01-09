
#include "qgsrenderer.h"

#include <QColor>
#include <QString>


QColor QgsRenderer::mSelectionColor=QColor(0,0,0);

QgsRenderer::QgsRenderer()
{

}

QgsRenderer::~QgsRenderer()
{
}

void QgsRenderer::setSelectionColor(QColor color)
{
  mSelectionColor = color;
}

bool QgsRenderer::containsPixmap() const
{
  //default implementation returns true only for points
  switch(mVectorType)
    {
    case QGis::Point:
       return true;
    default:
      return false;
    }
}
