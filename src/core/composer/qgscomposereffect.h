/***************************************************************************
                           qgscomposereffect.h
                             -------------------
    begin                : March 2013
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

#ifndef QGSCOMPOSEREFFECT_H
#define QGSCOMPOSEREFFECT_H

#include <QGraphicsEffect>
#include <QPainter>

class CORE_EXPORT QgsComposerEffect : public QGraphicsEffect
{
    Q_OBJECT

  public:
    QgsComposerEffect();
    ~QgsComposerEffect();

    void setCompositionMode( const QPainter::CompositionMode &compositionMode );

  protected:
    /** Called whenever source needs to be drawn */
    virtual void draw( QPainter *painter ) override;

  private:

    QPainter::CompositionMode mCompositionMode;
};

#endif // QGSCOMPOSEREFFECT_H

