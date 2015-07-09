/***************************************************************************
    qgsvaluemapwidgetwrapper.h
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

#ifndef QGSVALUEMAPWIDGETWRAPPER_H
#define QGSVALUEMAPWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QComboBox>


/**
 * Wraps a value map widget.
 *
 * Options:
 * <ul>
 * <li><b>[Key]</b> <i>Value</i></li>
 * </ul>
 *
 * Any option will be treated as entry in the value map.
 *
 */

class GUI_EXPORT QgsValueMapWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsValueMapWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent = 0 );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  public slots:
    void setValue( const QVariant& value ) override;

  private:
    QComboBox* mComboBox;
};

#endif // QGSVALUEMAPWIDGETWRAPPER_H
