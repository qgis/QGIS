/***************************************************************************
    qgsdefaultsearchwidgetwrapper.h
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

#ifndef QGSDEFAULTSEARCHWIDGETWRAPPER_H
#define QGSDEFAULTSEARCHWIDGETWRAPPER_H

#include "qgssearchwidgetwrapper.h"
#include <qgsfilterlineedit.h>

#include <QCheckBox>

/**
 * Wraps a search widget. Default form is just a QgsLineFilterEdit
 *
 */

class GUI_EXPORT QgsDefaultSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsDefaultSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent = 0 );

    // QgsSearchWidgetWrapper interface
  public:
    QString expression() override;
    bool applyDirectly() override;
  protected slots:
    void setExpression(QString exp) override;

  private slots:
    void setCaseString(int);
    
  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  private:
    QgsFilterLineEdit* mLineEdit;
    QCheckBox* mCheckbox;
    QWidget* mContainer;
    QString mCaseString;
};

#endif // QGSDEFAULTSEARCHWIDGETWRAPPER_H
