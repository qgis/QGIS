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

#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>

#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsRelationReferenceWidgetFactory;
class QgsMapCanvas;
class QgsRelationReferenceWidget;

/**
 * \ingroup gui
 * \class QgsRelationReferenceSearchWidgetWrapper
 * \brief Wraps a relation reference search widget.
 */

class GUI_EXPORT QgsRelationReferenceSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRelationReferenceSearchWidgetWrapper
     * \param vl associated vector layer
     * \param fieldIdx associated field index
     * \param canvas optional map canvas
     * \param parent parent widget
     */
    explicit QgsRelationReferenceSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Returns a variant representing the current state of the widget.
     */
    QVariant value() const;

    bool applyDirectly() override;
    QString expression() const override;
    bool valid() const override;
    QgsSearchWidgetWrapper::FilterFlags supportedFlags() const override;

    /**
     * Returns the default flags (equalTo)
     */
    QgsSearchWidgetWrapper::FilterFlags defaultFlags() const override;
    QString createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const override;

  public slots:

    void clearWidget() override;
    void setEnabled( bool enabled ) override;

  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;

  public slots:

    /**
     * Called when current value of search widget changes
     * \deprecated QGIS 3.10. Made private.
     */
    Q_DECL_DEPRECATED void onValueChanged( const QVariant &value ) SIP_DEPRECATED;

  protected slots:
    void setExpression( const QString &exp ) override;

  private:
    void onValuesChanged( const QVariantList &values );

    QgsRelationReferenceWidget *mWidget = nullptr;
    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    friend class QgsRelationReferenceWidgetFactory;
};

#endif // QGSRELATIONREFERENCESEARCHWIDGETWRAPPER_H
