/***************************************************************************
                              qgsblendmodecombobox.h
                              ------------------------
  begin                : March 21, 2013
  copyright            : (C) 2013 by Nyall Dawson
  email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBLENDMODECOMBOBOX_H
#define QGSBLENDMODECOMBOBOX_H

#include <QComboBox>
#include "qgis.h"
#include <QPainter> // For QPainter::CompositionMode enum
#include "qgis_gui.h"

/**
 * \ingroup gui
 * A combobox which lets the user select blend modes from a predefined list
 **/
class GUI_EXPORT QgsBlendModeComboBox : public QComboBox
{
    Q_OBJECT
  public:

    //! Constructor for QgsBlendModeComboBox
    QgsBlendModeComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Function to read the selected blend mode as QPainter::CompositionMode
    QPainter::CompositionMode blendMode();
    //! Function to set the selected blend mode from QPainter::CompositionMode
    void setBlendMode( QPainter::CompositionMode blendMode );
  private:

    /**
     * Returns a QStringList of the translated blend modes
    * "-" is used to indicate the position of a separator in the list
    * This list is designed to emulate GIMP's layer modes, where
    * blending modes are grouped by their effect (lightening, darkening, etc)
    */
    QStringList blendModesList() const;

    //! Used to map blend modes across to their corresponding
    //  index within the combo box
    std::vector<int> mBlendModeToListIndex;
    std::vector<int> mListIndexToBlendMode;

  public slots:

    /**
     * Populates the blend mode combo box, and sets up mapping for
    * blend modes to combo box indexes
    */
    void updateModes();

};

#endif // QGSBLENDMODECOMBOBOX_H
