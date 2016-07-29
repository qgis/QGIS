/***************************************************************************
    qgsexternalresourcewidgetwrapper.h
     --------------------------------------
 begin                : 16.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTERNALRESOURCEWIDGETWRAPPER_H
#define QGSEXTERNALRESOURCEWIDGETWRAPPER_H

class QgsExternalResourceWidget;

class QLabel;
class QLineEdit;

#include "qgseditorwidgetwrapper.h"



/** \ingroup gui
 * Wraps a file name widget. Will offer a file browser to choose files.
 * \note not available in Python bindings
 */

/**
 * @brief The QgsExternalResourceWidgetWrapper class wraps a external resource widget
 */
class GUI_EXPORT QgsExternalResourceWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsExternalResourceWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = nullptr, QWidget* parent = nullptr );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() const override;
    void showIndeterminateState() override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;
    bool valid() const override;

  public slots:
    void setValue( const QVariant& value ) override;
    void setEnabled( bool enabled ) override;

  private:
    void updateConstraintWidgetStatus( bool constraintValid ) override;

    QLineEdit* mLineEdit;
    QLabel* mLabel;
    QgsExternalResourceWidget* mQgsWidget;


};

#endif // QGSEXTERNALRESOURCEWIDGETWRAPPER_H
