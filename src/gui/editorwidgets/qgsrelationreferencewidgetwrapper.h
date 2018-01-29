/***************************************************************************
    qgsrelationreferencewidgetwrapper.h
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONREFERENCEWIDGETWRAPPER_H
#define QGSRELATIONREFERENCEWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"
#include "qgis.h"
#include "qgis_gui.h"

class QgsRelationReferenceWidget;
class QgsMapCanvas;
class QgsMessageBar;

/**
 * \ingroup gui
 * Wraps a relation reference widget.
 *
 * Options:
 * <ul>
 * <li><b>ShowForm</b> <i>If True, an embedded form with the referenced feature will be shown.</i></li>
 * <li><b>MapIdentification</b> <i>Will offer a map tool to pick a referenced feature on the map canvas. Only use for layers with geometry.</i></li>
 * <li><b>ReadOnly</b> <i>If True, will represent the referenced widget in a read-only line edit. Can speed up loading.</i></li>
 * <li><b>AllowNULL</b> <i>Will offer NULL as a value.</i></li>
 * <li><b>Relation</b> <i>The ID of the relation that will be used to define this widget.</i></li>
 * ReadOnly
 * </ul>
 *
 */

class GUI_EXPORT QgsRelationReferenceWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:

    //! Constructor for QgsRelationReferenceWidgetWrapper
    explicit QgsRelationReferenceWidgetWrapper( QgsVectorLayer *vl,
        int fieldIdx,
        QWidget *editor,
        QgsMapCanvas *canvas,
        QgsMessageBar *messageBar,
        QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    QVariant value() const override;
    bool valid() const override;
    void showIndeterminateState() override;

  public slots:
    void setValue( const QVariant &value ) override;
    void setEnabled( bool enabled ) override;

  private slots:
    void foreignKeyChanged( QVariant value );

  protected:

    void updateConstraintWidgetStatus() override;

  private:
    QgsRelationReferenceWidget *mWidget = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    bool mIndeterminateState;
};

#endif // QGSRELATIONREFERENCEWIDGETWRAPPER_H
