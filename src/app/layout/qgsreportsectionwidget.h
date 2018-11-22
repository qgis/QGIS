/***************************************************************************
                             qgsreportsectionwidget.h
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

#ifndef QGSREPORTSECTIONWIDGET_H
#define QGSREPORTSECTIONWIDGET_H

#include "ui_qgsreportwidgetsectionbase.h"

class QgsLayoutDesignerDialog;
class QgsReport;
class QgsReportOrganizerWidget;

class QgsReportSectionWidget: public QWidget, private Ui::QgsReportWidgetSectionBase
{
    Q_OBJECT
  public:
    QgsReportSectionWidget( QgsReportOrganizerWidget *parent, QgsLayoutDesignerDialog *designer, QgsReport *section );

  private slots:

    void toggleHeader( bool enabled );
    void toggleFooter( bool enabled );
    void editHeader();
    void editFooter();

  private:

    QgsReportOrganizerWidget *mOrganizer = nullptr;
    QgsReport *mSection = nullptr;
    QgsLayoutDesignerDialog *mDesigner = nullptr;

};

#endif // QGSREPORTSECTIONWIDGET_H
