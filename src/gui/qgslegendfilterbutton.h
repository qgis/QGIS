/***************************************************************************
    qgslegendfilterbutton.h - QToolButton for legend filter by map content
     --------------------------------------
    Date                 : June 2015
    Copyright            : (C) 2015 by Hugo Mercier at Oslandia
    Email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGS_LEGEND_FILTER_BUTTON_H
#define QGS_LEGEND_FILTER_BUTTON_H

#include <QToolButton>

class QgsVectorLayer;

/** \ingroup gui
 * \class QgsLegendFilterButton
 * A tool button that allows enabling or disabling legend filter by contents of the map.
 * An additional pop down menu allows defining a boolean expression to refine the filtering.
 * @note added in 2.14
 */

class GUI_EXPORT QgsLegendFilterButton: public QToolButton
{
    Q_OBJECT

  public:
    /**
     * Construct a new filter legend button
     *
     * @param parent The parent QWidget
     */
    QgsLegendFilterButton( QWidget* parent = nullptr );
    ~QgsLegendFilterButton();

    /**
     * Returns the current text used as filter expression
     */
    QString expressionText() const;

    /**
     * Sets the current text used as filter expression.
     * This will update the menu
     */
    void setExpressionText( const QString& expression );

    /**
     * Returns the current associated vectorLayer
     * May be null
     */
    QgsVectorLayer* vectorLayer() const;
    /**
     * Sets the associated vectorLayer
     * May be null
     */
    void setVectorLayer( QgsVectorLayer* layer );

  signals:
    /**
     * Emitted when the expression text changes
     */
    void expressionTextChanged();

  private:
    QMenu* mMenu;
    QAction* mSetExpressionAction;
    QAction* mClearExpressionAction;
    QString mExpression;

    void updateMenu();

    QgsVectorLayer* mLayer;
  private slots:
    void onSetLegendFilterExpression();
    void onClearFilterExpression();
    void onToggle( bool );
};

#endif // QGS_FILTER_LEGEND_BUTTON_H
