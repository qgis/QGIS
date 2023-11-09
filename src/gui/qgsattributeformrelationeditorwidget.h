/***************************************************************************
    qgsattributeformrelationeditorwidget.h
     --------------------------------------
    Date                 : Nov 2017
    Copyright            : (C) 2017 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEFORMRELATIONEDITORWIDGET_H
#define QGSATTRIBUTEFORMRELATIONEDITORWIDGET_H

#include "qgis_gui.h"
#include "qgsattributeformwidget.h"

class QgsRelationWidgetWrapper;
class QgsRelationAggregateSearchWidgetWrapper;


/**
 * \ingroup gui
 *
 * \brief Widget to show for child relations on an attribute form.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsAttributeFormRelationEditorWidget : public QgsAttributeFormWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor
     */
    explicit QgsAttributeFormRelationEditorWidget( QgsRelationWidgetWrapper *wrapper, QgsAttributeForm *form );

    void createSearchWidgetWrappers( const QgsAttributeEditorContext &context = QgsAttributeEditorContext() ) override;
    QString currentFilterExpression() const override;

    /**
     * Set multiple feature to edit simultaneously.
     * \param fids Multiple Id of features to edit
     * \since QGIS 3.24
     */
    void setMultiEditFeatureIds( const QgsFeatureIds &fids );

  private:
    QgsRelationAggregateSearchWidgetWrapper *mSearchWidget = nullptr;
    QgsRelationWidgetWrapper *mWrapper = nullptr;
};

#endif // QGSATTRIBUTEFORMRELATIONEDITORWIDGET_H
