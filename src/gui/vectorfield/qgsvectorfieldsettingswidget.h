/***************************************************************************
    qgsvectorfieldsettingswidget.h
    -------------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORFIELDSETTINGSWIDGET_H
#define QGSVECTORFIELDSETTINGSWIDGET_H

#include "ui_qgsvectorfieldsettingswidgetbase.h"

#include "qgsvectorfieldsettings.h"

#include <QWidget>

SIP_NO_FILE

class QgsDoubleSpinBox;

/**
 * \ingroup gui
 * \class QgsVectorFieldSettingsWidget
 *
 * \brief A widget for setup of the vector field symbology settings.
 *
 * The widget knows nothing of where the vector field comes from: the caller describes the data
 * through setSupportsInterpolation(), setHasMagnitude() and setMagnitudeRange(), then hands over
 * the settings to edit with setSettings(). It is used by both the mesh layer and the raster layer
 * properties.
 *
 * \since QGIS 4.4
 */
class QgsVectorFieldSettingsWidget : public QWidget, private Ui::QgsVectorFieldSettingsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * A widget to hold the vector field symbology settings.
     * \param parent Parent object
     */
    QgsVectorFieldSettingsWidget( QWidget *parent = nullptr );

    //! Returns vector settings
    QgsVectorFieldSettings settings() const;

    //! Synchronizes the widgets state with \a settings
    void setSettings( const QgsVectorFieldSettings &settings );

    /**
     * Sets the \a minimum and \a maximum magnitude of the data, which the color ramp shader is
     * classified against when it is loaded.
     */
    void setMagnitudeRange( double minimum, double maximum );

    /**
     * Sets whether the vector field behind the widget can be sampled between its data points.
     *
     * When it cannot, which is the case for a mesh layer without faces, the streamline and trace
     * symbologies are not offered and the user grid options are hidden, as all three place vectors
     * at positions of their own rather than at the positions of the data. Arrows and wind barbs are
     * still offered, as they are drawn one per data point. Defaults to TRUE.
     */
    void setSupportsInterpolation( bool supported );

    /**
     * Sets whether the data behind the widget carries a magnitude.
     *
     * When it does not, which is the case for a raster of coded compass directions, every vector has
     * the same length: the magnitude filter, the magnitude dependent arrow scaling methods and the
     * color ramp are hidden, leaving a fixed shaft length and a single color. Defaults to TRUE.
     *
     * Must be set before setSettings().
     */
    void setHasMagnitude( bool hasMagnitude );

  signals:
    //! Vector field symbology settings changed
    void widgetChanged();

  private slots:
    void onSymbologyChanged();
    void onStreamLineSeedingMethodChanged( int currentIndex );
    void onWindBarbUnitsChanged( int currentIndex );
    void onColoringMethodChanged();
    void onColorRampMinMaxChanged();
    void loadColorRampShader();

  private:
    /**
     * Returns the value of the spin box, returns err_val if the
     * value is equal to the clear value.
     */
    double filterValue( const QgsDoubleSpinBox *spinBox, double err_val ) const;

    //! Shows or hides the widgets which only make sense when the data carries a magnitude
    void applyMagnitudeSupport();

    //! Fills the symbology combo box with the symbologies the data can be drawn with
    void populateSymbologies();

    //! Returns the symbology currently selected in the combo box
    Qgis::VectorFieldSymbology currentSymbology() const;

    bool mSupportsInterpolation = true;
    bool mHasMagnitude = true;
    bool mHasMagnitudeRange = false;
    double mMagnitudeMinimum = 0;
    double mMagnitudeMaximum = 0;
};

#endif // QGSVECTORFIELDSETTINGSWIDGET_H
