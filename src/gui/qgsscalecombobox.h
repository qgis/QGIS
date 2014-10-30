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

/** \ingroup gui
 * A combobox which lets the user select map scale from predefined list
 * and highlights nearest to current scale value
 **/
class GUI_EXPORT QgsScaleComboBox : public QComboBox
{
    Q_OBJECT
  public:
    QgsScaleComboBox( QWidget* parent = 0 );
    virtual ~QgsScaleComboBox();

    //! Function to read the selected scale as text
    QString scaleString();
    //! Function to set the selected scale from text
    bool setScaleString( QString scaleTxt );
    //! Function to read the selected scale as double
    double scale();
    //! Function to set the selected scale from double
    void setScale( double scale );

    //! Helper function to convert a double to scale string
    // Performs rounding, so an exact representation is not to
    // be expected.
    static QString toString( double scale );
    //! Helper function to convert a scale string to double
    static double toDouble( QString scaleString, bool *ok = NULL );

  signals:
    //! Signal is emitted when *user* has finished editing/selecting a new scale.
    void scaleChanged();

  public slots:
    void updateScales( const QStringList &scales = QStringList() );

  protected:
    void showPopup();

  private slots:
    void fixupScale();

  private:
    double mScale;
};

#endif // QGSSCALECOMBOBOX_H
