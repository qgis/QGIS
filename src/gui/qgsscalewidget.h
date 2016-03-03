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

class QgsMapCanvas;

/** \ingroup gui
 * A combobox which lets the user select map scale from predefined list
 * and highlights nearest to current scale value
 **/
class GUI_EXPORT QgsScaleWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool showCurrentScaleButton READ showCurrentScaleButton WRITE setShowCurrentScaleButton )
    Q_PROPERTY( bool scale READ scale WRITE setScale NOTIFY scaleChanged )
    Q_PROPERTY( bool minScale READ minScale WRITE setMinScale )

  public:
    explicit QgsScaleWidget( QWidget *parent = nullptr );

    virtual ~QgsScaleWidget();

    //! shows a button to set the scale to the current scale of the map canvas next to the combobox
    //! @note the map canvas must be defined to show the button
    void setShowCurrentScaleButton( bool showCurrentScaleButton );
    bool showCurrentScaleButton() { return mShowCurrentScaleButton;}

    //! set the map canvas associated to the current button
    void setMapCanvas( QgsMapCanvas* canvas );

    //! Function to read the selected scale as text
    QString scaleString() { return mScaleComboBox->scaleString(); }
    //! Function to set the selected scale from text
    bool setScaleString( const QString& scaleTxt ) { return mScaleComboBox->setScaleString( scaleTxt ); }
    //! Function to read the selected scale as double
    double scale() const { return mScaleComboBox->scale();}
    //! Function to set the selected scale from double
    void setScale( double scale ) { return mScaleComboBox->setScale( scale ); }
    //! Function to read the min scale
    double minScale() const { return mScaleComboBox->minScale(); }

    //! Helper function to convert a double to scale string
    // Performs rounding, so an exact representation is not to
    // be expected.
    static QString toString( double scale ) { return QgsScaleComboBox::toString( scale ); }
    //! Helper function to convert a scale string to double
    static double toDouble( const QString& scaleString, bool *ok = nullptr ) { return QgsScaleComboBox::toDouble( scaleString, ok ); }

  public slots:
    void updateScales( const QStringList &scales = QStringList() ) { return mScaleComboBox->updateScales( scales ); }

    //! assign the current scale from the map canvas
    void setScaleFromCanvas();

    //! Function to set the min scale
    void setMinScale( double scale ) { mScaleComboBox->setMinScale( scale ); }

  signals:
    //! Signal is emitted when *user* has finished editing/selecting a new scale.
    void scaleChanged( double scale );

  private:
    QgsScaleComboBox* mScaleComboBox;
    QToolButton* mCurrentScaleButton;
    QgsMapCanvas* mCanvas;
    bool mShowCurrentScaleButton;
};

#endif // QGSSCALEWIDGET_H
