/***************************************************************************
    qgstabpositionwidget.h
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTABPOSITIONWIDGET_H
#define QGSTABPOSITIONWIDGET_H

#include "ui_qgstabpositionwidgetbase.h"

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "qgstextformat.h"

#include <QDialog>

/**
 * \ingroup gui
 * \brief A widget for configuring QgsTextFormat tab positions.
 * \since QGIS 3.42
*/
class GUI_EXPORT QgsTabPositionWidget : public QgsPanelWidget, private Ui::QgsTabPositionWidgetBase
{
    Q_OBJECT
  public:
    //! Constructor for QgsTabPositionWidget, with the specified \a parent widget
    QgsTabPositionWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the tab \a positions to show in the widget.
     *
     * \see positions()
     */
    void setPositions( const QList<QgsTextFormat::Tab> &positions );

    /**
     * Returns the tab positions defined in the widget.
     *
     * \see setPositions()
     */
    QList<QgsTextFormat::Tab> positions() const;

    /**
     * Sets the unit type used for the tab positions (used to update interface labels).
    */
    void setUnit( Qgis::RenderUnit unit );

  signals:

    /**
     * Emitted when positions are changed in the widget.
     */
    void positionsChanged( const QList<QgsTextFormat::Tab> &positions );

  private slots:
    void mAddButton_clicked();
    void mRemoveButton_clicked();

    void emitPositionsChanged();
};

/**
 * \ingroup gui
 * \brief A dialog to enter a custom dash space pattern for lines
  * \since QGIS 3.42
*/
class GUI_EXPORT QgsTabPositionDialog : public QDialog
{
    Q_OBJECT
  public:
    //! Constructor for QgsTabPositionDialog
    QgsTabPositionDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    /**
     * Sets the tab \a positions to show in the dialog.
     *
     * \see positions()
     */
    void setPositions( const QList<QgsTextFormat::Tab> &positions );

    /**
    * Returns the tab positions defined in the dialog.
    *
    * \see setPositions()
    */
    QList<QgsTextFormat::Tab> positions() const;

    /**
     * Sets the unit type used for the tab positions (used to update interface labels).
    */
    void setUnit( Qgis::RenderUnit unit );

  private:
    QgsTabPositionWidget *mWidget = nullptr;
};

#endif // QGSTABPOSITIONWIDGET_H
