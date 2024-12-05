/***************************************************************************
   qgsscalewidget.h
    --------------------------------------
   Date                 : 08.01.2015
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSSCALEWIDGET_H
#define QGSSCALEWIDGET_H

#include <QWidget>
#include <QToolButton>


#include "qgsscalecombobox.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsMapCanvas;

/**
 * \ingroup gui
 * \brief A combobox which lets the user select map scale from predefined list
 * and highlights nearest to current scale value
 */
class GUI_EXPORT QgsScaleWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool showCurrentScaleButton READ showCurrentScaleButton WRITE setShowCurrentScaleButton )
    Q_PROPERTY( bool scale READ scale WRITE setScale NOTIFY scaleChanged )
    Q_PROPERTY( bool minScale READ minScale WRITE setMinScale )

  public:
    /**
     * \brief QgsScaleWidget creates a combobox which lets the user select map scale from predefined list
     * and highlights nearest to current scale value
     */
    explicit QgsScaleWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets whether to show a button to set the scale to the current scale of the map canvas next to the combobox.
     * \note the map canvas must be defined to show the button
     * \see showCurrentScaleButton()
     * \see setMapCanvas()
     */
    void setShowCurrentScaleButton( bool showCurrentScaleButton );

    /**
     * Returns whether a button to set the scale from map canvas is shown or not.
     * \see setShowCurrentScaleButton()
     */
    bool showCurrentScaleButton() { return mShowCurrentScaleButton; }

    /**
     * Set the map \a canvas associated to the current button.
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the selected scale as a string, e.g. "1:150".
     * \see setScaleString()
     */
    QString scaleString() const { return mScaleComboBox->scaleString(); }

    /**
     * Set the selected scale from a \a string, e.g. "1:150".
     * \see scaleString()
     */
    bool setScaleString( const QString &string ) { return mScaleComboBox->setScaleString( string ); }

    /**
     * Returns the selected scale as a double.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see setScale()
     */
    double scale() const { return mScaleComboBox->scale(); }

    /**
     * Returns TRUE if the widget is currently set to a "null" value.
     *
     * \see setAllowNull()
     * \see setNull()
     * \since QGIS 3.8
     */
    bool isNull() const;

    /**
     * Returns the minimum scale, or 0 if no minimum scale set.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * Any scale lower than the minimum scale will automatically be converted to the minimum scale.
     * Except for 0 which is always allowed.
     */
    double minScale() const { return mScaleComboBox->minScale(); }

    /**
     * Helper function to convert a \a scale double to scale string.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     *
     * The returned string will be rounded (e.g. 1:1000, not 1:1000.345).
     * \see toDouble()
     */
    static QString toString( double scale ) { return QgsScaleComboBox::toString( scale ); }

    /**
     * Helper function to convert a scale \a string to double.
     * The returned value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * If specified, \a ok will be set to TRUE if the string was successfully interpreted as a scale.
     * \see toString()
     */
    static double toDouble( const QString &scaleString, bool *ok = nullptr ) { return QgsScaleComboBox::toDouble( scaleString, ok ); }

    /**
     * Sets whether the scale widget can be set to a NULL value.
     * \see allowNull()
     * \see isNull()
     * \see setNull()
     * \since QGIS 3.8
     */
    void setAllowNull( bool allowNull );

    /**
     * Returns TRUE if the widget can be set to a NULL value.
     * \see setAllowNull()
     * \see isNull()
     * \see setNull()
     * \since QGIS 3.8
     */
    bool allowNull() const;

    /**
     * Sets the list of predefined \a scales to show in the widget. List elements
     * are expected to be scale denominators, e.g. 1000.0 for a 1:1000 map.
     *
     * If \a scales is empty then the default user scale options will be used instead.
     *
     * \since QGIS 3.38
    */
    void setPredefinedScales( const QVector<double> &scales );

  public slots:

    /**
     * Set the selected scale from a double.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see scale()
     */
    void setScale( double scale );

    /**
     * Sets the list of predefined \a scales to show in the widget. List elements
     * are expected to be valid scale strings, such as "1:1000000".
     */
    void updateScales( const QStringList &scales = QStringList() ) { mScaleComboBox->updateScales( scales ); }

    /**
     * Assigns the current scale from the map canvas, if set.
     * \see setMapCanvas()
     */
    void setScaleFromCanvas();

    /**
     * Set the minimum allowed \a scale. Set to 0 to disable the minimum scale.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * Any scale lower than the minimum scale will automatically be converted to the minimum scale.
     * Except for 0 which is always allowed.
     */
    void setMinScale( double scale ) { mScaleComboBox->setMinScale( scale ); }

    /**
     * Sets the widget to the null value.
     *
     * This only has an effect if allowNull() is TRUE.
     *
     * \see allowNull()
     * \see isNull()
     * \since QGIS 3.8
     */
    void setNull();

  signals:

    /**
     * Emitted when *user* has finished editing/selecting a new scale.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     */
    void scaleChanged( double scale );

  private slots:

    void menuAboutToShow();

  private:
    QgsScaleComboBox *mScaleComboBox = nullptr;
    QToolButton *mCurrentScaleButton = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QMenu *mMenu = nullptr;
    bool mShowCurrentScaleButton = false;
};

#endif // QGSSCALEWIDGET_H
