/***************************************************************************
    qgskeyvaluewidgetfactory.h
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

#ifndef QGSKEYVALUEEDITFACTORY_H
#define QGSKEYVALUEEDITFACTORY_H

#include "qgseditorwidgetfactory.h"

/** @ingroup gui
 * Factory for widgets for editing a QVariantMap
 * @note added in QGIS 3.0
 * @note not available in Python bindings
 */
class GUI_EXPORT QgsKeyValueWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    /**
     * Constructor.
     */
    QgsKeyValueWidgetFactory( const QString& name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper* create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const override;
    //QgsSearchWidgetWrapper* createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const override;
    QgsEditorConfigWidget* configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const override;
    QgsEditorWidgetConfig readConfig( const QDomElement &configElement, QgsVectorLayer *layer, int fieldIdx ) override;
    void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx ) override;
    QString representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const override;
    Qt::AlignmentFlag alignmentFlag( QgsVectorLayer *vl, int fieldIdx, const QgsEditorWidgetConfig &config ) const override;
    unsigned int fieldScore( const QgsVectorLayer* vl, int fieldIdx ) const override;
};

#endif // QGSKEYVALUEEDITFACTORY_H
