/***************************************************************************
    qgsrangeconfigdlg.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRANGECONFIGDLG_H
#define QGSRANGECONFIGDLG_H

#include "ui_qgsrangeconfigdlgbase.h"
#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsRangeConfigDlg
 * \brief Configuration widget for range widgets.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsRangeConfigDlg : public QgsEditorConfigWidget, private Ui::QgsRangeConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsRangeConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent );
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;

  protected slots:
    void rangeWidgetChanged( int index );

    /**
     * Sets the precision of minimum value, maximum value, step size UI elements
     * \param precision the precision
     */
    void setPrecision( int precision );
};

#endif // QGSRANGECONFIGDLG_H
