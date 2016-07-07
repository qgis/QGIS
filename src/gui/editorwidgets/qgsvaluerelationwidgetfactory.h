/***************************************************************************
    qgsvaluerelationwidgetfactory.h
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

#ifndef QGSVALUERELATIONWIDGETFACTORY_H
#define QGSVALUERELATIONWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"
#include "qgsvaluerelationwidgetwrapper.h"

#include <QMap>

/** \ingroup gui
 * \class QgsValueRelationWidgetFactory
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsValueRelationWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    explicit QgsValueRelationWidgetFactory( const QString& name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper* create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const override;
    QgsSearchWidgetWrapper* createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const override;
    QgsEditorConfigWidget* configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const override;
    QgsEditorWidgetConfig readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx ) override;
    void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx ) override;
    QString representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const override;
    QVariant sortValue( QgsVectorLayer *vl, int fieldIdx, const QgsEditorWidgetConfig &config, const QVariant &cache, const QVariant &value ) const override;
    Qt::AlignmentFlag alignmentFlag( QgsVectorLayer *vl, int fieldIdx, const QgsEditorWidgetConfig &config ) const override;
    QVariant createCache( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config ) override;
};

#endif // QGSVALUERELATIONWIDGETFACTORY_H
