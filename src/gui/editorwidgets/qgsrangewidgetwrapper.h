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

class GUI_EXPORT QgsRangeWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsRangeWidgetWrapper ( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent = 0 );

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
