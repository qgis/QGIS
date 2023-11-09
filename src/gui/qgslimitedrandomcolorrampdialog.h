/***************************************************************************
    qgslimitedrandomcolorrampdialog.h
    ---------------------
    begin                : December 2009
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

#ifndef QGsLIMITEDRANDOMCOLORRAMPDIALOG_H
#define QGsLIMITEDRANDOMCOLORRAMPDIALOG_H

#include <QDialog>
#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "qgscolorrampimpl.h"
#include "ui_qgslimitedrandomcolorrampwidgetbase.h"
#include "qgis_gui.h"

class QDialogButtonBox;

/**
 * \ingroup gui
 * \class QgsLimitedRandomColorRampWidget
 * \brief A widget which allows users to modify the properties of a QgsLimitedRandomColorRamp.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLimitedRandomColorRampWidget : public QgsPanelWidget, private Ui::QgsLimitedRandomColorRampWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QgsLimitedRandomColorRamp ramp READ ramp WRITE setRamp )

  public:

    /**
     * Constructor for QgsLimitedRandomColorRampWidget.
     * \param ramp initial ramp to show in dialog
     * \param parent parent widget
     */
    QgsLimitedRandomColorRampWidget( const QgsLimitedRandomColorRamp &ramp, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a color ramp representing the current settings from the dialog.
     * \see setRamp()
     */
    QgsLimitedRandomColorRamp ramp() const { return mRamp; }

    /**
     * Sets the color ramp to show in the dialog.
     * \param ramp color ramp
     * \see ramp()
     */
    void setRamp( const QgsLimitedRandomColorRamp &ramp );

  signals:

    //! Emitted when the dialog settings change
    void changed();

  public slots:

    //! Sets the number of colors to create in the ramp
    void setCount( int val );
    //! Sets the minimum hue for colors in the ramp
    void setHue1( int val );
    //! Sets the maximum hue for colors in the ramp
    void setHue2( int val );
    //! Sets the minimum saturation for colors in the ramp
    void setSat1( int val );
    //! Sets the maximum saturation for colors in the ramp
    void setSat2( int val );
    //! Sets the minimum value for colors in the ramp
    void setVal1( int val );
    //! Sets the maximum value for colors in the ramp
    void setVal2( int val );

  private:

    void updatePreview();
    void updateUi();

    QgsLimitedRandomColorRamp mRamp;
};


/**
 * \ingroup gui
 * \class QgsLimitedRandomColorRampDialog
 * \brief A dialog which allows users to modify the properties of a QgsLimitedRandomColorRamp.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLimitedRandomColorRampDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QgsLimitedRandomColorRamp ramp READ ramp WRITE setRamp )

  public:

    /**
     * Constructor for QgsLimitedRandomColorRampDialog.
     * \param ramp initial ramp to show in dialog
     * \param parent parent widget
     */
    QgsLimitedRandomColorRampDialog( const QgsLimitedRandomColorRamp &ramp, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a color ramp representing the current settings from the dialog.
     * \see setRamp()
     */
    QgsLimitedRandomColorRamp ramp() const { return mWidget->ramp(); }

    /**
     * Sets the color ramp to show in the dialog.
     * \param ramp color ramp
     * \see ramp()
     */
    void setRamp( const QgsLimitedRandomColorRamp &ramp ) { mWidget->setRamp( ramp ); }

    /**
     * Returns a reference to the dialog's button box.
     * \since QGIS 3.10
     */
    QDialogButtonBox *buttonBox() const;

  signals:

    //! Emitted when the dialog settings change
    void changed();

  private:

    QgsLimitedRandomColorRampWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;

  private slots:

    void showHelp();

};

#endif
