/***************************************************************************
    qgsgradientcolorrampdialog.h
    ----------------------------
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

#ifndef QGSGRADIENTCOLORRAMPDIALOG_H
#define QGSGRADIENTCOLORRAMPDIALOG_H

#include <QDialog>
#include "qgis_sip.h"

#include "ui_qgsgradientcolorrampdialogbase.h"
#include "qgshelp.h"
#include "qgis_gui.h"

class QgsGradientColorRamp;
class QwtPlot;
class QwtPlotCurve;
class QwtPlotMarker;
class QgsGradientPlotEventFilter;

/**
 * \ingroup gui
 * \class QgsGradientColorRampDialog
 * \brief A dialog which allows users to modify the properties of a QgsGradientColorRamp.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsGradientColorRampDialog : public QDialog, private Ui::QgsGradientColorRampDialogBase
{
    Q_OBJECT
    Q_PROPERTY( QgsGradientColorRamp ramp READ ramp WRITE setRamp )

  public:

    /**
     * Constructor for QgsGradientColorRampDialog.
     * \param ramp initial ramp to show in dialog
     * \param parent parent widget
     */
    QgsGradientColorRampDialog( const QgsGradientColorRamp &ramp, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsGradientColorRampDialog() override;

    /**
     * Returns a color ramp representing the current settings from the dialog.
     * \see setRamp()
     */
    QgsGradientColorRamp ramp() const { return mRamp; }

    /**
     * Sets the color ramp to show in the dialog.
     * \param ramp color ramp
     * \see ramp()
     */
    void setRamp( const QgsGradientColorRamp &ramp );

    /**
     * Returns a reference to the dialog's button box.
     * \since QGIS 3.10
     */
    QDialogButtonBox *buttonBox() const;

  signals:

    //! Emitted when the dialog settings change
    void changed();

  public slots:

    /**
     * Sets the start color for the gradient ramp.
     * \see setColor2()
     */
    void setColor1( const QColor &color );

    /**
     * Sets the end color for the gradient ramp.
     * \see setColor1()
     */
    void setColor2( const QColor &color );

  private slots:
    void cboType_currentIndexChanged( int index );
    void btnInformation_pressed();
    void updateRampFromStopEditor();
    void updateColorButtons();
    void updateStopEditor();
    void selectedStopChanged( const QgsGradientStop &stop );
    void colorWidgetChanged( const QColor &color );
    void mPositionSpinBox_valueChanged( double val );
    void mPlotHueCheckbox_toggled( bool checked );
    void mPlotLightnessCheckbox_toggled( bool checked );
    void mPlotSaturationCheckbox_toggled( bool checked );
    void mPlotAlphaCheckbox_toggled( bool checked );
    void plotMousePress( QPointF point );
    void plotMouseRelease( QPointF point );
    void plotMouseMove( QPointF point );
    void showHelp();

  private:

    QgsGradientColorRamp mRamp;
    QwtPlotCurve *mLightnessCurve = nullptr;
    QwtPlotCurve *mSaturationCurve = nullptr;
    QwtPlotCurve *mHueCurve = nullptr;
    QwtPlotCurve *mAlphaCurve = nullptr;
    QList< QwtPlotMarker * > mMarkers;
    QgsGradientPlotEventFilter *mPlotFilter = nullptr;
    int mCurrentPlotColorComponent;
    int mCurrentPlotMarkerIndex;
    int mBlockChanges = 0;

    void updatePlot();
    void addPlotMarker( double x, double y, const QColor &color, bool isSelected = false );
    void addMarkersForColor( double x, const QColor &color, bool isSelected = false );
};


#ifndef SIP_RUN
//
// NOTE:
// For private only, not part of stable api or exposed to Python bindings
//
/// @cond PRIVATE
class GUI_EXPORT QgsGradientPlotEventFilter: public QObject
{
    Q_OBJECT

  public:

    QgsGradientPlotEventFilter( QwtPlot *plot );

    bool eventFilter( QObject *object, QEvent *event ) override;

  signals:

    void mousePress( QPointF );
    void mouseRelease( QPointF );
    void mouseMove( QPointF );

  private:

    QwtPlot *mPlot = nullptr;
    QPointF mapPoint( QPointF point ) const;
};
///@endcond
#endif

#endif
