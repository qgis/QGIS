/***************************************************************************
    qgsmaptoolshapeellipseabstract.cpp  -  map tool for adding ellipse
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lituus at free dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshapeellipseabstract.h"

#include "qgsgeometryrubberband.h"
#include "qgslinestring.h"
#include "qgsmaptoolcapture.h"

#include "moc_qgsmaptoolshapeellipseabstract.cpp"

void QgsMapToolShapeEllipseAbstract::addEllipseToParentTool()
{
  if ( !mParentTool || mEllipse.isEmpty() )
    return;

  mParentTool->clearCurve();

  std::unique_ptr<QgsLineString> ls( mEllipse.toLineString( segments() ) );

  mParentTool->addCurve( ls.release() );
}

void QgsMapToolShapeEllipseAbstract::clean()
{
  mEllipse = QgsEllipse();
  QgsMapToolShapeAbstract::clean();
}
