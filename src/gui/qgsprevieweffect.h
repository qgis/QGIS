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

/** \ingroup gui
 * A graphics effect which can be applied to a widget to simulate various printing and
 * color blindness modes.
 */

class GUI_EXPORT QgsPreviewEffect: public QGraphicsEffect
{
    Q_OBJECT

  public:
    enum PreviewMode
    {
      PreviewGrayscale,
      PreviewMono,
      PreviewProtanope,
      PreviewDeuteranope
    };

    QgsPreviewEffect( QObject* parent );
    ~QgsPreviewEffect();

    /** Sets the mode for the preview effect, which controls how the effect modifies a widgets appearance.
     * @param mode PreviewMode to use to draw the widget
     * @note added in 2.3
     * @see mode
     */
    void setMode( PreviewMode mode );
    /** Returns the mode used for the preview effect.
     * @returns PreviewMode currently used by the effect
     * @note added in 2.3
     * @see setMode
     */
    PreviewMode mode() const { return mMode; }

  protected:
    virtual void draw( QPainter *painter ) override;

  private:

    PreviewMode mMode;

    QRgb simulateColorBlindness( QRgb &originalColor, PreviewMode type );
    void simulateProtanopeLMS( double &L, double &M, double &S );
    void simulateDeuteranopeLMS( double &L, double &M, double &S );
};

#endif // QGSPREVIEWEFFECT_H
