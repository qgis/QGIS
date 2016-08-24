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

#include "ui_qgsgradientcolorrampdialogbase.h"

class QgsGradientColorRamp;
class QwtPlot;
class QwtPlotCurve;
class QwtPlotMarker;
class QgsGradientPlotEventFilter;

/** \ingroup gui
 * \class QgsGradientColorRampDialog
 */
class GUI_EXPORT QgsGradientColorRampDialog : public QDialog, private Ui::QgsGradientColorRampDialogBase
{
    Q_OBJECT

  public:
    QgsGradientColorRampDialog( const QgsGradientColorRamp& ramp, QWidget* parent = nullptr );
    ~QgsGradientColorRampDialog();

    QgsGradientColorRamp ramp() const { return mRamp; }

  public slots:
    void setColor1( const QColor& color );
    void setColor2( const QColor& color );

  protected slots:
    void on_cboType_currentIndexChanged( int index );
    void on_btnInformation_pressed();

  protected:
    QgsGradientColorRamp mRamp;

  private slots:

    void updateRampFromStopEditor();
    void updateColorButtons();
    void updateStopEditor();
    void selectedStopChanged( const QgsGradientStop& stop );
    void colorWidgetChanged( const QColor& color );
    void on_mPositionSpinBox_valueChanged( double val );
    void on_mPlotHueCheckbox_toggled( bool checked );
    void on_mPlotLightnessCheckbox_toggled( bool checked );
    void on_mPlotSaturationCheckbox_toggled( bool checked );
    void on_mPlotAlphaCheckbox_toggled( bool checked );
    void plotMousePress( QPointF point );
    void plotMouseRelease( QPointF point );
    void plotMouseMove( QPointF point );

  private:

    QwtPlotCurve* mLightnessCurve;
    QwtPlotCurve* mSaturationCurve;
    QwtPlotCurve* mHueCurve;
    QwtPlotCurve* mAlphaCurve;
    QList< QwtPlotMarker* > mMarkers;
    QgsGradientPlotEventFilter* mPlotFilter;
    int mCurrentPlotColorComponent;
    int mCurrentPlotMarkerIndex;

    void updatePlot();
    void addPlotMarker( double x, double y, const QColor &color, bool isSelected = false );
    void addMarkersForColor( double x, const QColor &color, bool isSelected = false );
};


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

    virtual ~QgsGradientPlotEventFilter() {}

    virtual bool eventFilter( QObject* object, QEvent* event ) override;

  signals:

    void mousePress( QPointF );
    void mouseRelease( QPointF );
    void mouseMove( QPointF );

  private:

    QwtPlot* mPlot;
    QPointF mapPoint( QPointF point ) const;
};
///@endcond

#endif
