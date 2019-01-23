/***************************************************************************
                              qgsscalecombobox.h
                              ------------------------
  begin                : January 7, 2012
  copyright            : (C) 2012 by Alexander Bruy
  email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCALECOMBOBOX_H
#define QGSSCALECOMBOBOX_H

#include <QComboBox>
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * A combobox which lets the user select map scale from predefined list
 * and highlights nearest to current scale value
 **/
class GUI_EXPORT QgsScaleComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( double scale READ scale WRITE setScale NOTIFY scaleChanged )
    Q_PROPERTY( double minScale READ minScale WRITE setMinScale )

  public:

    /**
     * Constructor for QgsScaleComboBox.
     */
    QgsScaleComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the selected scale as a string, e.g. "1:150".
     * \see setScaleString()
     */
    QString scaleString() const;

    /**
     * Set the selected scale from a \a string, e.g. "1:150".
     * \see scaleString()
     */
    bool setScaleString( const QString &string );

    /**
     * Returns the selected scale as a double.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see setScale()
     */
    double scale() const;

    /**
     * Returns the minimum scale, or 0 if no minimum scale set.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * Any scale lower than the minimum scale will automatically be converted to the minimum scale.
     * Except for 0 which is always allowed.
     */
    double minScale() const { return mMinScale; }

    /**
     * Helper function to convert a \a scale double to scale string.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     *
     * The returned string will be rounded (e.g. 1:1000, not 1:1000.345).
     * \see toDouble()
     */
    static QString toString( double scale );

    /**
     * Helper function to convert a scale \a string to double.
     * The returned value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * If specified, \a ok will be set to true if the string was successfully interpreted as a scale.
     * \see toString()
     */
    static double toDouble( const QString &string, bool *ok = nullptr );

  signals:

    /**
     * Emitted when *user* has finished editing/selecting a new scale.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     */
    void scaleChanged( double scale );

  public slots:

    /**
     * Sets the list of predefined \a scales to show in the combobox. List elements
     * are expected to be valid scale strings, such as "1:1000000".
     */
    void updateScales( const QStringList &scales = QStringList() );

    /**
     * Set the selected scale from a double.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see scale()
     */
    void setScale( double scale );

    /**
     * Set the minimum allowed \a scale. Set to 0 to disable the minimum scale.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * Any scale lower than the minimum scale will automatically be converted to the minimum scale.
     * Except for 0 which is always allowed.
     */
    void setMinScale( double scale );

  protected:
    void showPopup() override;

  private slots:
    void fixupScale();

  private:
    double mScale = 1.0;
    double mMinScale = 0.0;
};

#endif // QGSSCALECOMBOBOX_H
