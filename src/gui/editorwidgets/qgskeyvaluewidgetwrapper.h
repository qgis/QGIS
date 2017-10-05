/***************************************************************************
    qgskeyvaluewidgetwrapper.h
     --------------------------------------
    Date                 : 08.2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi@camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSKEYVALUEWIDGETWRAPPER_H
#define QGSKEYVALUEWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"
#include "qgis_gui.h"

SIP_NO_FILE

class QgsKeyValueWidget;

/**
 * \ingroup gui
 * Wraps a key/value widget.
 * \since QGIS 3.0
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsKeyValueWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:

    /**
     * Constructor.
     */
    explicit QgsKeyValueWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor = nullptr, QWidget *parent = nullptr );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() const override;
    void showIndeterminateState() override;

  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    bool valid() const override;

  public slots:
    void setValue( const QVariant &value ) override;

  private slots:
    void onValueChanged();

  private:
    void updateConstraintWidgetStatus( ConstraintResult status ) override;

    QgsKeyValueWidget *mWidget = nullptr;
};

#endif // QGSKEYVALUEWIDGETWRAPPER_H
