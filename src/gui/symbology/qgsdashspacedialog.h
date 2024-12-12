/***************************************************************************
    qgsdashspacedialog.h
    ---------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDASHSPACEDIALOG_H
#define QGSDASHSPACEDIALOG_H

#include "ui_qgsdashspacewidgetbase.h"

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "qgsunittypes.h"

#include <QDialog>

/**
 * \ingroup gui
 * \brief A widget to enter a custom dash space pattern for lines
 * \since QGIS 3.8
*/
class GUI_EXPORT QgsDashSpaceWidget : public QgsPanelWidget, private Ui::QgsDashSpaceWidgetBase
{
    Q_OBJECT
  public:
    //! Constructor for QgsDashSpaceWidget
    QgsDashSpaceWidget( const QVector<qreal> &vectorPattern, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Returns the dash pattern as a list of numbers
    QVector<qreal> dashDotVector() const;

    /**
     * Sets the unit type used for the dash space pattern (used to update interface labels)
     * \param unit the unit type
    */
    void setUnit( Qgis::RenderUnit unit );

  private slots:
    void mAddButton_clicked();
    void mRemoveButton_clicked();
};

/**
 * \ingroup gui
 * \brief A dialog to enter a custom dash space pattern for lines
*/
class GUI_EXPORT QgsDashSpaceDialog : public QDialog
{
    Q_OBJECT
  public:
    //! Constructor for QgsDashSpaceDialog
    QgsDashSpaceDialog( const QVector<qreal> &v, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    //! Returns the dash pattern as a list of numbers
    QVector<qreal> dashDotVector() const;

    /**
     * Sets the unit type used for the dash space pattern (used to update interface labels)
     * \param unit the unit type
     * \since QGIS 3.8
    */
    void setUnit( Qgis::RenderUnit unit );

  private:
    QgsDashSpaceWidget *mWidget = nullptr;
};

#endif // QGSDASHSPACEDIALOG_H
