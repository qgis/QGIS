/***************************************************************************
    qgsrangewidgetwrapper.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRANGEWIDGETWRAPPER_H
#define QGSRANGEWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QSpinBox>
#include <QDoubleSpinBox>

#include "qgsdial.h"
#include "qgsslider.h"

/**
 * Wraps a range widget.
 *
 * Options:
 * <ul>
 * <li><b>Style</b> <i>Can be "Dial" or "Slider"</i></li>
 * <li><b>Min</b> <i>The minimal allowed value</i></li>
 * <li><b>Max</b> <i>The maximal allowed value</i></li>
 * <li><b>Step</b> <i>The step size when incrementing/decrementing the value</i></li>
 * </ul>
 *
 */

class GUI_EXPORT QgsRangeWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsRangeWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent = 0 );

    // QgsEditorWidgetWrapper interface
  public:
    virtual QVariant value();

  protected:
    virtual QWidget* createWidget( QWidget* parent );
    virtual void initWidget( QWidget* editor );

  public slots:
    virtual void setValue( const QVariant& value );

  private:
    QSpinBox* mIntSpinBox;
    QDoubleSpinBox* mDoubleSpinBox;
    QSlider* mSlider;
    QDial* mDial;
};

#endif // QGSRANGEWIDGETWRAPPER_H
