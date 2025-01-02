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
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsRelationReferenceWidget;
class QgsMapCanvas;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief Wraps a relation reference widget.
 *
 * Options:
 *
 * - **ShowForm** If TRUE, an embedded form with the referenced feature will be shown.
 * - **MapIdentification** Will offer a map tool to pick a referenced feature on the map canvas. Only use for layers with geometry.
 * - **ReadOnly** If TRUE, will represent the referenced widget in a read-only line edit. Can speed up loading.
 * - **AllowNULL** Will offer NULL as a value.
 * - **Relation** The ID of the relation that will be used to define this widget.
 * - **ReadOnly**
 *
 */
class GUI_EXPORT QgsRelationReferenceWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    //! Constructor for QgsRelationReferenceWidgetWrapper
    explicit QgsRelationReferenceWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    QVariant value() const override;
    bool valid() const override;
    void showIndeterminateState() override;
    QVariantList additionalFieldValues() const override;
    QStringList additionalFields() const override;

  public slots:
    void setEnabled( bool enabled ) override;

  private slots:
    void foreignKeysChanged( const QVariantList &values );

  protected:
    void updateConstraintWidgetStatus() override;

  private:
    void updateValues( const QVariant &val, const QVariantList &additionalValues = QVariantList() ) override;

    QString mExpression;

    QgsRelationReferenceWidget *mWidget = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    bool mIndeterminateState;
    int mBlockChanges = 0;
};

#endif // QGSRELATIONREFERENCEWIDGETWRAPPER_H
