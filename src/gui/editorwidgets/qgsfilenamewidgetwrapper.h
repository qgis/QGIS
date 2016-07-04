/***************************************************************************
    qgsfilenamewidgetwrapper.h
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

#ifndef QGSFILENAMEWIDGETWRAPPER_H
#define QGSFILENAMEWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>


/** \ingroup gui
 * Wraps a file name widget. Will offer a file browser to choose files.
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsFileNameWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsFileNameWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = nullptr, QWidget* parent = nullptr );

  private slots:
    void selectFileName();

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() const override;
    bool valid() const override;
    void showIndeterminateState() override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  public slots:
    void setValue( const QVariant& value ) override;

  private:
    void updateConstraintWidgetStatus( bool constraintValid ) override;

    QLineEdit* mLineEdit;
    QPushButton* mPushButton;
    QLabel* mLabel;
};

#endif // QGSFILENAMEWIDGETWRAPPER_H
