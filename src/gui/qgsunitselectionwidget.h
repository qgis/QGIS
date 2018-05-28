/***************************************************************************
    qgsunitselectionwidget.h
    -------------------
    begin                : Mar 24, 2014
    copyright            : (C) 2014 Sandro Mani
    email                : smani@sourcepole.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSUNITSELECTIONWIDGET_H
#define QGSUNITSELECTIONWIDGET_H

#include <QWidget>
#include "qgis.h"
#include <QDialog>
#include "qgspanelwidget.h"
#include "qgssymbol.h"
#include "ui_qgsunitselectionwidget.h"
#include "ui_qgsmapunitscalewidgetbase.h"
#include "qgis_gui.h"

class QgsMapCanvas;

/**
 * \class QgsMapUnitScaleWidget
 * \ingroup gui
 * A widget which allows the user to choose the minimum and maximum scale of an object in map units
 * and millimeters. This widget is designed to allow users to edit the properties of a
 * QgsMapUnitScale object.
 * \see QgsMapUnitScaleDialog
 * \see QgsUnitSelectionWidget
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsMapUnitScaleWidget : public QgsPanelWidget, private Ui::QgsMapUnitScaleWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QgsMapUnitScale mapUnitScale READ mapUnitScale WRITE setMapUnitScale NOTIFY mapUnitScaleChanged )

  public:

    /**
     * Constructor for QgsMapUnitScaleWidget.
     * \param parent parent widget
     */
    QgsMapUnitScaleWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a QgsMapUnitScale representing the settings shown in the
     * widget.
     * \see setMapUnitScale()
     * \see mapUnitScaleChanged()
     */
    QgsMapUnitScale mapUnitScale() const;

    /**
     * Updates the widget to reflect the settings from the specified
     * QgsMapUnitScale object.
     * \param scale map unit scale to show in widget
     * \see mapUnitScale()
     * \see mapUnitScaleChanged()
     */
    void setMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Sets the map canvas associated with the widget. This allows the
     * widget to retrieve the current map scale from the canvas.
     * \param canvas map canvas
     */
    void setMapCanvas( QgsMapCanvas *canvas );

  signals:

    /**
     * Emitted when the settings in the widget are modified.
     * \param scale QgsMapUnitScale reflecting new settings from the widget
     */
    void mapUnitScaleChanged( const QgsMapUnitScale &scale );

  private slots:
    void configureMinComboBox();
    void configureMaxComboBox();
    void settingsChanged();

  private:

    bool mBlockSignals = true;

};

/**
 * \class QgsMapUnitScaleDialog
 * \ingroup gui
 * A dialog which allows the user to choose the minimum and maximum scale of an object in map units
 * and millimeters. This dialog is designed to allow users to edit the properties of a
 * QgsMapUnitScale object.
 * \see QgsMapUnitScaleWidget
 * \see QgsUnitSelectionWidget
*/
class GUI_EXPORT QgsMapUnitScaleDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QgsMapUnitScale mapUnitScale READ getMapUnitScale WRITE setMapUnitScale )

  public:

    /**
     * Constructor for QgsMapUnitScaleDialog.
     * \param parent parent widget
     */
    QgsMapUnitScaleDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a QgsMapUnitScale representing the settings shown in the
     * dialog.
     * \see setMapUnitScale()
     */
    QgsMapUnitScale getMapUnitScale() const;

    /**
     * Updates the dialog to reflect the settings from the specified
     * QgsMapUnitScale object.
     * \param scale map unit scale to show in dialog
     * \see mapUnitScale()
     */
    void setMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Sets the map canvas associated with the dialog. This allows the dialog to retrieve the current
     * map scale from the canvas.
     * \param canvas map canvas
     * \since QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas *canvas );

  private:

    QgsMapUnitScaleWidget *mWidget = nullptr;

};

/**
 * \class QgsUnitSelectionWidget
 * \ingroup gui
 * A widget displaying a combobox allowing the user to choose between various display units,
 * such as millimeters or map unit. If the user chooses map units, a button appears allowing
 * adjustment of minimum and maximum scaling.
 * \see QgsMapUnitScaleWidget
 * \see QgsMapUnitScaleDialog
 */
class GUI_EXPORT QgsUnitSelectionWidget : public QWidget, private Ui::QgsUnitSelectionWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsUnitSelectionWidget.
     * \param parent parent widget
     */
    QgsUnitSelectionWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the units which the user can choose from in the combobox.
     * \param units list of strings for custom units to display in the widget
     * \param mapUnitIdx specifies which entry corresponds to the map units, or -1 if none
     */
    void setUnits( const QStringList &units, int mapUnitIdx );

    /**
     * Sets the units which the user can choose from in the combobox. Clears any existing units.
     * \param units list of valid units
     * \since QGIS 2.9
     */
    void setUnits( const QgsUnitTypes::RenderUnitList &units );

    //! Gets the selected unit index
    int getUnit() const { return mUnitCombo->currentIndex(); }

    /**
     * Returns the current predefined selected unit (if applicable).
     * \returns selected output unit, or QgsUnitTypes::RenderUnknownUnit if the widget was populated with custom unit types
     * \since QGIS 2.9
     */
    QgsUnitTypes::RenderUnit unit() const;

    /**
     * Sets the selected unit index
     * \param unitIndex index of unit to set as current
     * \note available in Python bindings as setUnitIndex
     */
    void setUnit( int unitIndex ) SIP_PYNAME( setUnitIndex );

    /**
     * Sets the selected unit
     * \param unit predefined unit to set as current
     */
    void setUnit( QgsUnitTypes::RenderUnit unit );

    //! Returns the map unit scale
    QgsMapUnitScale getMapUnitScale() const { return mMapUnitScale; }

    //! Sets the map unit scale
    void setMapUnitScale( const QgsMapUnitScale &scale ) { mMapUnitScale = scale; }

    /**
     * Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map scale from the canvas.
     * \param canvas map canvas
     * \since QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas *canvas );

  signals:
    void changed();

  private slots:
    void showDialog();
    void toggleUnitRangeButton();
    void widgetChanged( const QgsMapUnitScale &scale );

  private:
    QgsMapUnitScale mMapUnitScale;
    int mMapUnitIdx;
    QgsMapCanvas *mCanvas = nullptr;

};

#endif // QGSUNITSELECTIONWIDGET_H
