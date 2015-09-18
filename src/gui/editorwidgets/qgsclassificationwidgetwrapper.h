/***************************************************************************
    qgsclassificationwidgetwrapper.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLASSIFICATIONWIDGETWRAPPER_H
#define QGSCLASSIFICATIONWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QComboBox>

class GUI_EXPORT QgsClassificationWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsClassificationWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent = 0 );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() override;

  protected:
    QWidget*createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;
    bool valid() override;

  public slots:
    void setValue( const QVariant& value ) override;

  private:
    QComboBox* mComboBox;
};

#endif // QGSCLASSIFICATIONWIDGETWRAPPER_H
