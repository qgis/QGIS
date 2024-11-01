/***************************************************************************
                             qgslayoutunitscombobox.h
                             ------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTUNITSCOMBOBOX_H
#define QGSLAYOUTUNITSCOMBOBOX_H

#include <QComboBox>
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsunittypes.h"
#include <QDoubleSpinBox>
#include <QPointer>

class QgsLayoutMeasurementConverter;

/**
 * \ingroup gui
 * \brief A custom combo box for selecting units for layout settings.
 *
 */
class GUI_EXPORT QgsLayoutUnitsComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( Qgis::LayoutUnit unit READ unit WRITE setUnit NOTIFY changed )

  public:
    /**
     * Constructor for QgsLayoutUnitsComboBox.
     */
    QgsLayoutUnitsComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the unit currently selected in the combo box.
     * \see setUnit()
     */
    Qgis::LayoutUnit unit() const;

    /**
     * Sets the \a unit currently selected in the combo box.
     * \see unit()
     */
    void setUnit( Qgis::LayoutUnit unit );

    /**
     * Registers a spin box \a widget as linked with the combo box.
     *
     * Registered spin boxes will automatically be upodated whenever the unit is changed. I.e. a
     * spin box with a value of 100 will be set to 1 when the unit is changed from centimeters to meters.
     *
     * A measurement converter() must be set in order for the automatic unit conversion to occur.
     *
     * \see setConverter()
     */
    void linkToWidget( QDoubleSpinBox *widget );

    /**
     * Returns the converter used when automatically converting units for linked widgets.
     * \see setConverter()
     */
    QgsLayoutMeasurementConverter *converter() const;

    /**
     * Sets a \a converter to use when automatically converting units for linked widgets.
     * The ownership of \a converter is not transferred, and converter must exist for the
     * life of the combo box.
     * \see converter()
     */
    void setConverter( QgsLayoutMeasurementConverter *converter );

  signals:

#ifndef SIP_RUN

    /**
     * Emitted when the \a unit is changed.
     */
    void unitChanged( Qgis::LayoutUnit unit );
#endif

    /**
     * Emitted when the \a unit is changed.
     */
    void changed( int unit );

  private slots:

    void indexChanged( int index );

  private:
    QgsLayoutMeasurementConverter *mConverter = nullptr;

    Qgis::LayoutUnit mOldUnit = Qgis::LayoutUnit::Millimeters;

    QList<QPointer<QDoubleSpinBox>> mLinkedSpinBoxes;
};

#endif // QGSLAYOUTUNITSCOMBOBOX_H
