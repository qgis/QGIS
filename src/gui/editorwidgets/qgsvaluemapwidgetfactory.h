/***************************************************************************
    qgsvaluemapwidgetfactory.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVALUEMAPWIDGETFACTORY_H
#define QGSVALUEMAPWIDGETFACTORY_H

#include "qgseditorwidgetfactory.h"

class GUI_EXPORT QgsValueMapWidgetFactory : public QgsEditorWidgetFactory
{
  public:
    QgsValueMapWidgetFactory( const QString& name );

    // QgsEditorWidgetFactory interface
  public:
    QgsEditorWidgetWrapper* create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const override;
    QgsEditorConfigWidget* configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const override;
    QgsEditorWidgetConfig readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx ) override;
    void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx ) override;
    QString representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const override;
};

#endif // QGSVALUEMAPWIDGETFACTORY_H
