/***************************************************************************
                             qgsreportfieldgroupsectionwidget.h
                             ----------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSREPORTFIELDGROUPSECTIONWIDGET_H
#define QGSREPORTFIELDGROUPSECTIONWIDGET_H

#include "ui_qgsreportwidgetfieldgroupsectionbase.h"

class QgsLayoutDesignerDialog;
class QgsReportSectionFieldGroup;
class QgsReportOrganizerWidget;

class QgsReportSectionFieldGroupWidget: public QWidget, private Ui::QgsReportWidgetFieldGroupSectionBase
{
    Q_OBJECT
  public:
    QgsReportSectionFieldGroupWidget( QgsReportOrganizerWidget *parent, QgsLayoutDesignerDialog *designer, QgsReportSectionFieldGroup *section );

  private slots:

    void toggleHeader( bool enabled );
    void toggleHeaderAlwaysVisible( bool enabled );
    void toggleFooter( bool enabled );
    void toggleFooterAlwaysVisible( bool enabled );
    void editHeader();
    void editFooter();
    void toggleBody( bool enabled );
    void editBody();
    void sortAscendingToggled( bool checked );
    void setLayer( QgsMapLayer *layer );
    void setField( const QString &field );

  private:

    QgsReportOrganizerWidget *mOrganizer = nullptr;
    QgsReportSectionFieldGroup *mSection = nullptr;
    QgsLayoutDesignerDialog *mDesigner = nullptr;

};

#endif // QGSREPORTFIELDGROUPSECTIONWIDGET_H
