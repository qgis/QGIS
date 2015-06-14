/***************************************************************************
    qgsvaluemapsearchwidgetwrapper.h
     --------------------------------------
    Date                 : 21.5.2015
    Copyright            : (C) 2015 Karolina Alexiou
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVALUEMAPSEARCHWIDGETWRAPPER_H
#define QGSVALUEMAPSEARCHWIDGETWRAPPER_H

#include "qgsdefaultsearchwidgetwrapper.h"
#include <QComboBox>



class GUI_EXPORT QgsValueMapSearchWidgetWrapper : public QgsDefaultSearchWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsValueMapSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent = 0 );
    bool applyDirectly() override;
    
  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  private slots:
      void comboBoxIndexChanged(int);

  private:
    QComboBox * mComboBox;
};

#endif // QGSVALUEMAPSEARCHWIDGETWRAPPER_H
