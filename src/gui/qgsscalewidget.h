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

  public:
    explicit QgsScaleWidget( QWidget *parent = 0 );

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
    bool setScaleString( QString scaleTxt ) { return mScaleComboBox->setScaleString( scaleTxt ); }
    //! Function to read the selected scale as double
    double scale() { return mScaleComboBox->scale();}
    //! Function to set the selected scale from double
    void setScale( double scale ) { return mScaleComboBox->setScale( scale ); }

    //! Helper function to convert a double to scale string
    // Performs rounding, so an exact representation is not to
    // be expected.
    static QString toString( double scale ) { return QgsScaleComboBox::toString( scale ); }
    //! Helper function to convert a scale string to double
    static double toDouble( QString scaleString, bool *ok = 0 ) { return QgsScaleComboBox::toDouble( scaleString, ok ); }

  public slots:
    void updateScales( const QStringList &scales = QStringList() ) { return mScaleComboBox->updateScales( scales ); }

    //! assign the current scale from the map canvas
    void setScaleFromCanvas();

  signals:
    //! Signal is emitted when *user* has finished editing/selecting a new scale.
    void scaleChanged();

  private:
    QgsScaleComboBox* mScaleComboBox;
    QToolButton* mCurrentScaleButton;
    QgsMapCanvas* mCanvas;
    bool mShowCurrentScaleButton;
};

#endif // QGSSCALEWIDGET_H
