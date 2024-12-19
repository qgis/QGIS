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
#include "qgis_sip.h"
#include <QPainter> // For QPainter::CompositionMode enum
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief A combobox which lets the user select blend modes from a predefined list
 */
class GUI_EXPORT QgsBlendModeComboBox : public QComboBox
{
    Q_OBJECT
  public:

    //! Constructor for QgsBlendModeComboBox
    QgsBlendModeComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the selected blend mode.
     * \see setBlendMode()
     */
    QPainter::CompositionMode blendMode();

    /**
     * Sets the selected blend mode.
     * \see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode blendMode );

    /**
     * Sets whether composition modes which cause clipping are shown in the combo box.
     *
     * By default, these composition modes (such as QPainter::CompositionMode::CompositionMode_DestinationIn )
     * are not shown in the combo box, as they can only be used with predictable results in a limited
     * set of circumstances. By setting \a show to TRUE these additional composition modes
     * will be shown in the combo box.
     *
     * \see showClippingModes()
     * \since QGIS 3.24
     */
    void setShowClippingModes( bool show );

    /**
     * Returns TRUE if composition modes which cause clipping are shown in the combo box.
     *
     * By default, these composition modes (such as QPainter::CompositionMode::CompositionMode_DestinationIn )
     * are not shown in the combo box, as they can only be used with predictable results in a limited
     * set of circumstances.
     *
     * \see setShowClippingModes()
     * \since QGIS 3.24
     */
    bool showClippingModes() const;

  private:

    bool mShowClipModes = false;

  public slots:

    /**
     * Populates the blend mode combo box, and sets up mapping for
    * blend modes to combo box indexes
    */
    void updateModes();

};

#endif // QGSBLENDMODECOMBOBOX_H
