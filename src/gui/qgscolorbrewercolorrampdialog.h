/***************************************************************************
    qgscolorbrewercolorrampdialog.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORBREWERCOLORRAMPDIALOG_H
#define QGSCOLORBREWERCOLORRAMPDIALOG_H

#include <QDialog>
#include "qgspanelwidget.h"
#include "qgscolorramp.h"
#include "ui_qgscolorbrewercolorrampwidgetbase.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsColorBrewerColorRamp;

/**
 * \ingroup gui
 * \class QgsColorBrewerColorRampWidget
 * A widget which allows users to modify the properties of a QgsColorBrewerColorRamp.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsColorBrewerColorRampWidget : public QgsPanelWidget, private Ui::QgsColorBrewerColorRampWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QgsColorBrewerColorRamp ramp READ ramp WRITE setRamp )

  public:

    /**
     * Constructor for QgsColorBrewerColorRampWidget.
     * \param ramp initial ramp to show in dialog
     * \param parent parent widget
     */
    QgsColorBrewerColorRampWidget( const QgsColorBrewerColorRamp &ramp, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a color ramp representing the current settings from the dialog.
     * \see setRamp()
     */
    QgsColorBrewerColorRamp ramp() const { return mRamp; }

    /**
     * Sets the color ramp to show in the dialog.
     * \param ramp color ramp
     * \see ramp()
     */
    void setRamp( const QgsColorBrewerColorRamp &ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();

  private slots:
    void setSchemeName();
    void setColors();
    void populateVariants();

  private:

    void updatePreview();
    void updateUi();

    QgsColorBrewerColorRamp mRamp;
};

/**
 * \ingroup gui
 * \class QgsColorBrewerColorRampDialog
 * A dialog which allows users to modify the properties of a QgsColorBrewerColorRamp.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsColorBrewerColorRampDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QgsColorBrewerColorRamp ramp READ ramp WRITE setRamp )

  public:

    /**
     * Constructor for QgsColorBrewerColorRampDialog.
     * \param ramp initial ramp to show in dialog
     * \param parent parent widget
     */
    QgsColorBrewerColorRampDialog( const QgsColorBrewerColorRamp &ramp, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a color ramp representing the current settings from the dialog.
     * \see setRamp()
     */
    QgsColorBrewerColorRamp ramp() const { return mWidget->ramp(); }

    /**
     * Sets the color ramp to show in the dialog.
     * \param ramp color ramp
     * \see ramp()
     */
    void setRamp( const QgsColorBrewerColorRamp &ramp ) { mWidget->setRamp( ramp ); }

  signals:

    //! Emitted when the dialog settings change
    void changed();

  private:

    QgsColorBrewerColorRampWidget *mWidget = nullptr;

  private slots:

    void showHelp();

};

#endif
