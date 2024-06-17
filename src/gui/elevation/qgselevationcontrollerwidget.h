/***************************************************************************
                          qgselevationcontrollerwidget.h
                          ---------------
    begin                : March 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#ifndef QGSELEVATIONCONTROLLERWIDGET_H
#define QGSELEVATIONCONTROLLERWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include <QWidget>
#include <QWidgetAction>

class QgsRangeSlider;
class QgsDoubleSpinBox;
class QToolButton;
class QMenu;

///@cond PRIVATE

class GUI_EXPORT QgsElevationControllerLabels : public QWidget SIP_SKIP
{
    Q_OBJECT

  public:

    QgsElevationControllerLabels( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void paintEvent( QPaintEvent *event ) override;

    void setLimits( const QgsDoubleRange &limits );
    void setRange( const QgsDoubleRange &range );
    void setInverted( bool inverted );
    void setSignificantElevations( const QList< double > &elevations );

  private:

    QgsDoubleRange mLimits;
    QgsDoubleRange mRange;
    bool mInverted = false;
    QList< double > mSignificantElevations;

};

class GUI_EXPORT QgsElevationControllerSettingsAction: public QWidgetAction
{
    Q_OBJECT

  public:

    QgsElevationControllerSettingsAction( QWidget *parent = nullptr );

    QgsDoubleSpinBox *sizeSpin();

  private:

    QgsDoubleSpinBox *mSizeSpin = nullptr;
};


///@endcond PRIVATE

/**
 * \ingroup gui
 * \brief A widget for configuring vertical elevation slicing behavior for maps.
 *
 * \since QGIS 3.38
 */
class GUI_EXPORT QgsElevationControllerWidget : public QWidget
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsElevationControllerWidget *>( sipCpp ) != nullptr )
      sipType = sipType_QgsElevationControllerWidget;
    else
      sipType = nullptr;
    SIP_END
#endif

    Q_OBJECT

  public:

    /**
     * Constructor for QgsElevationControllerWidget, with the specified \a parent widget.
     */
    QgsElevationControllerWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void resizeEvent( QResizeEvent *event ) override;

    /**
     * Returns the current visible range from the widget.
     *
     * \see setRange()
     * \see rangeChanged()
     */
    QgsDoubleRange range() const;

    /**
     * Returns the limits of the elevation range which can be selected by the widget.
     *
     * \see rangeLimits()
     */
    QgsDoubleRange rangeLimits() const;

    /**
     * Returns a reference to the slider component of the widget.
     */
    QgsRangeSlider *slider();

    /**
     * Returns a reference to the widget's configuration menu, which can be used
     * to add actions to the menu.
     */
    QMenu *menu();

    /**
     * Returns the fixed range size, or -1 if no fixed size is set.
     *
     * A fixed size forces the selected elevation range to have a matching difference between
     * the upper and lower elevation.
     *
     * \see setFixedRangeSize()
     */
    double fixedRangeSize() const;

  public slots:

    /**
     * Sets the current visible \a range for the widget.
     *
     * \see range()
     * \see rangeChanged()
     */
    void setRange( const QgsDoubleRange &range );

    /**
     * Sets the limits of the elevation range which can be selected by the widget.
     *
     * \see rangeLimits()
     */
    void setRangeLimits( const QgsDoubleRange &limits );

    /**
     * Sets the fixed range \a size. Set to -1 if no fixed size is desired.
     *
     * A fixed size forces the selected elevation range to have a matching difference between
     * the upper and lower elevation.
     *
     * \see fixedRangeSize()
     * \see fixedRangeSizeChanged()
     */
    void setFixedRangeSize( double size );

    /**
     * Sets whether the elevation slider should be inverted.
     *
     * \see invertedChanged()
     */
    void setInverted( bool inverted );

    /**
     * Sets a list of significant \a elevations to highlight in the widget.
     */
    void setSignificantElevations( const QList< double > &elevations );

  signals:

    /**
     * Emitted when the visible range from the widget is changed.
     *
     * \see setRange()
     * \see range()
     */
    void rangeChanged( const QgsDoubleRange &range );

    /**
     * Emitted when the fixed range size is changed from the widget.
     *
     * \see fixedRangeSize()
     * \see setFixedRangeSize()
     */
    void fixedRangeSizeChanged( double size );

    /**
     * Emitted when the elevation filter slider is inverted.
     *
     * \see setInverted()
     */
    void invertedChanged( bool inverted );

  private:

    void updateWidgetMask();

    QToolButton *mConfigureButton = nullptr;
    QgsElevationControllerSettingsAction *mSettingsAction = nullptr;
    QMenu *mMenu = nullptr;
    QAction *mInvertDirectionAction = nullptr;
    QgsRangeSlider *mSlider = nullptr;
    QgsElevationControllerLabels *mSliderLabels = nullptr;
    QgsDoubleRange mRangeLimits;
    QgsDoubleRange mCurrentRange;
    double mFixedRangeSize = -1;
    int mBlockSliderChanges = 0;
    double mSliderPrecision = 100;

};

#endif // QGSELEVATIONCONTROLLERWIDGET_H
