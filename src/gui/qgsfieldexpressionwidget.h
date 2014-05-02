/***************************************************************************
   qgsfieldexpressionwidget.h
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSFIELDEXPRESSIONWIDGET_H
#define QGSFIELDEXPRESSIONWIDGET_H

#include <QSharedPointer>
#include <QWidget>
#include <QToolButton>
#include <QComboBox>
#include <QColor>

#include "qgsdistancearea.h"

class QgsMapLayer;
class QgsVectorLayer;
class QgsFieldModel;


class GUI_EXPORT QgsFieldExpressionWidget : public QWidget
{
    Q_OBJECT
  public:
    /**
     * @brief QgsFieldExpressionWidget creates a widget with a combo box to display the fields and expression and a button to open the expression dialog
     */
    explicit QgsFieldExpressionWidget( QWidget *parent = 0 );

    //! define the title used in the expression dialog
    void setExpressionDialogTitle( QString title );

    //! set the geometry calculator used in the expression dialog
    void setGeomCalculator( const QgsDistanceArea &da );

    /**
     * @brief currentField returns the currently selected field or expression if allowed
     * @param isExpression determines if the string returned is the name of a field or an expression
     */
    QString currentField( bool *isExpression = 0 );

    //! Returns the currently used layer
    QgsVectorLayer* layer();

  signals:
    //! the signal is emitted when the currently selected field changes
    void fieldChanged( QString fieldName );

    //! fieldChanged signal with indication of the validity of the expression
    void fieldChanged( QString fieldName, bool isValid );

  public slots:
    //! set the layer used to display the fields and expression
    void setLayer( QgsVectorLayer* layer );

    //! convenience slot to connect QgsMapLayerComboBox layer signal
    void setLayer( QgsMapLayer* layer );

    //! sets the current field or expression in the widget
    void setField( QString fieldName );

  protected slots:
    //! open the expression dialog to edit the current or add a new expression
    void editExpression();

    //! when expression is edited by the user in the line edit
    void expressionEdited( QString expression );

    void indexChanged( int i );

  private:
    QComboBox* mCombo;
    QToolButton* mButton;
    QgsFieldModel* mFieldModel;
    QString mExpressionDialogTitle;
    QSharedPointer<const QgsDistanceArea> mDa;

    QString color2rgbaStr( QColor color );
};

#endif // QGSFIELDEXPRESSIONWIDGET_H
