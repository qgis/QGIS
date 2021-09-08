/***************************************************************************
                             qgscreateannotationitemmaptool_impl.h
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCREATEANNOTATIONITEMMAPTOOLIMPL_H
#define QGSCREATEANNOTATIONITEMMAPTOOLIMPL_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgscreateannotationitemmaptool.h"

class QgsAnnotationPointTextItem;

#define SIP_NO_FILE

///@cond PRIVATE

class QgsCreatePointTextItemMapTool: public QgsMapToolAdvancedDigitizing, public QgsCreateAnnotationItemMapToolInterface
{
    Q_OBJECT

  public:

    QgsCreatePointTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );
    ~QgsCreatePointTextItemMapTool() override;

    void cadCanvasPressEvent( QgsMapMouseEvent *event ) override;
    QgsCreateAnnotationItemMapToolHandler *handler() override;
    QgsMapTool *mapTool() override;

    QgsAnnotationItem *takeCreatedItem() override;

  private:

    QgsCreateAnnotationItemMapToolHandler *mHandler = nullptr;

};


///@endcond PRIVATE

#endif // QGSCREATEANNOTATIONITEMMAPTOOLIMPL_H
