/***************************************************************************
    qgsattributeformrelationeditorwidget.h
     --------------------------------------
    Date                 : Nov 2017
    Copyright            : (C) 2017 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
 * Widget to show for child relations on an attribute form.
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

  private:
    QgsRelationAggregateSearchWidgetWrapper *mSearchWidget = nullptr;
    QgsRelationWidgetWrapper *mWrapper = nullptr;
};

#endif // QGSATTRIBUTEFORMRELATIONEDITORWIDGET_H
