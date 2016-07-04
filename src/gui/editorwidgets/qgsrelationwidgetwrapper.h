/***************************************************************************
    qgsrelationwidgetwrapper.h
     --------------------------------------
    Date                 : 14.5.2014
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

#ifndef QGSRELATIONWIDGETWRAPPER_H
#define QGSRELATIONWIDGETWRAPPER_H

#include "qgswidgetwrapper.h"

class QgsRelationEditorWidget;

/** \ingroup gui
 * \class QgsRelationWidgetWrapper
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsRelationWidgetWrapper : public QgsWidgetWrapper
{
    Q_OBJECT

  public:
    explicit QgsRelationWidgetWrapper( QgsVectorLayer* vl, const QgsRelation& relation, QWidget* editor = nullptr, QWidget* parent = nullptr );

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;
    bool valid() const override;

  public slots:
    void setFeature( const QgsFeature& feature ) override;

    /** Sets the visibility of the wrapper's widget.
     * @param visible set to true to show widget, false to hide widget
     * @note added in QGIS 2.16
     */
    void setVisible( bool visible );

  private:
    QgsRelation mRelation;
    QgsRelationEditorWidget* mWidget;
};

#endif // QGSRELATIONWIDGETWRAPPER_H
