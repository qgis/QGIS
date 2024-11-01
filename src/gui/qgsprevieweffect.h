/***************************************************************************
                         qgsprevieweffect.h
                             -------------------
    begin                : March 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPREVIEWEFFECT_H
#define QGSPREVIEWEFFECT_H

#include <QGraphicsEffect>
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief A graphics effect which can be applied to a widget to simulate various printing and
 * color blindness modes.
 */

class GUI_EXPORT QgsPreviewEffect : public QGraphicsEffect
{
    Q_OBJECT

  public:
    enum PreviewMode
    {
      PreviewGrayscale,
      PreviewMono,
      PreviewProtanope,
      PreviewDeuteranope,
      PreviewTritanope
    };

    QgsPreviewEffect( QObject *parent SIP_TRANSFERTHIS );

    /**
     * Sets the mode for the preview effect, which controls how the effect modifies a widgets appearance.
     * \param mode PreviewMode to use to draw the widget
     * \see mode
     */
    void setMode( PreviewMode mode );

    /**
     * Returns the mode used for the preview effect.
     * \returns PreviewMode currently used by the effect
     * \see setMode
     */
    PreviewMode mode() const { return mMode; }

  protected:
    void draw( QPainter *painter ) override;

  private:
    PreviewMode mMode;

    QRgb simulateColorBlindness( QRgb &originalColor, PreviewMode type );
    void simulateGrayscale( int &r, int &g, int &b, int &red, int &green, int &blue );
    void simulateProtanope( int &r, int &g, int &b, int &red, int &green, int &blue );
    void simulateDeuteranope( int &r, int &g, int &b, int &red, int &green, int &blue );
    void simulateTritanope( int &r, int &g, int &b, int &red, int &green, int &blue );
};

#endif // QGSPREVIEWEFFECT_H
