/***************************************************************************
    qgsrelationreferencesearchwidgetwrapper.h
     ----------------------------------------
    Date                 : 2016-05-25
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONREFERENCESEARCHWIDGETWRAPPER_H
#define QGSRELATIONREFERENCESEARCHWIDGETWRAPPER_H

#include "qgssearchwidgetwrapper.h"
#include "qgsrelationreferencewidgetwrapper.h"
#include "qgsrelationreferencewidget.h"

#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>

class QgsRelationReferenceWidgetFactory;

/** \ingroup gui
 * \class QgsRelationReferenceSearchWidgetWrapper
 * Wraps a relation reference search widget.
 * \note Added in version 2.16
 */

class GUI_EXPORT QgsRelationReferenceSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT

  public:

    /** Constructor for QgsRelationReferenceSearchWidgetWrapper
     * @param vl associated vector layer
     * @param fieldIdx associated field index
     * @param canvas optional map canvas
     * @param parent parent widget
     */
    explicit QgsRelationReferenceSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QgsMapCanvas* canvas, QWidget* parent = nullptr );

    /** Returns a variant representing the current state of the widget.
     */
    QVariant value() const;

    bool applyDirectly() override;
    QString expression() override;
    bool valid() const override;
    FilterFlags supportedFlags() const override;
    FilterFlags defaultFlags() const override;
    virtual QString createExpression( FilterFlags flags ) const override;

  public slots:

    virtual void clearWidget() override;
    virtual void setEnabled( bool enabled ) override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  public slots:

    //! Called when current value of search widget changes
    void onValueChanged( QVariant value );

  protected slots:
    void setExpression( QString exp ) override;

  private:

    QgsRelationReferenceWidget* mWidget;
    QgsVectorLayer* mLayer;
    QgsMapCanvas* mCanvas;

    friend class QgsRelationReferenceWidgetFactory;
};

#endif // QGSRELATIONREFERENCESEARCHWIDGETWRAPPER_H
