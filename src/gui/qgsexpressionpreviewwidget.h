/***************************************************************************
    qgsexpressionpreviewwidget.h
     --------------------------------------
    Date                 : march 2020 - quarantine day 12
    Copyright            : (C) 2020 by Denis Rouzaud
    Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONPREVIEWWIDGET_H
#define QGSEXPRESSIONPREVIEWWIDGET_H

#include <QWidget>

#include "ui_qgsexpressionpreviewbase.h"

#include "qgis_gui.h"
#include "qgsdistancearea.h"

class QAction;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \brief QgsExpressionPreviewWidget is a widget to preview an expression result.
 * If the layer is set, one can browse across features to see the different outputs.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsExpressionPreviewWidget : public QWidget, private Ui::QgsExpressionPreviewWidgetBase
{
    Q_OBJECT
  public:
    //! Constructor
    explicit QgsExpressionPreviewWidget( QWidget *parent = nullptr );

    //! Sets the layer used in the preview
    void setLayer( QgsVectorLayer *layer );

    //! Sets the expression
    void setExpressionText( const QString &expression );

    /**
     * Returns the expression context for the widget. The context is used for the expression
     * preview result and for populating the list of available functions and variables.
     * \see setExpressionContext
     */
    QgsExpressionContext expressionContext() const { return mExpressionContext; }

    /**
     * Sets the expression context for the widget. The context is used for the expression
     * preview result and to populate the list of available functions and variables.
     * \param context expression context
     */
    void setExpressionContext( const QgsExpressionContext &context );

    //! Sets geometry calculator used in distance/area calculations.
    void setGeomCalculator( const QgsDistanceArea &da );

    /**
     * Will be set to TRUE if the current expression text reported an eval error
     * with the context.
     */
    bool evalError() const;

    /**
     * Will be set to TRUE if the current expression text reports a parser error
     * with the context.
     */
    bool parserError() const;

    //! Returns the root node of the expression
    const QgsExpressionNode *rootNode() const {return mExpression.rootNode();}

    //! Returns the expression parser erros
    QList<QgsExpression::ParserError> parserErrors() const {return mExpression.parserErrors();}

  signals:

    /**
     * Emitted when the user changes the expression in the widget.
     * Users of this widget should connect to this signal to decide if to let the user
     * continue.
     * \param isValid Is TRUE if the expression the user has typed is valid.
     */
    void expressionParsed( bool isValid );

    /**
     * Will be set to TRUE if the current expression text reported an eval error
     * with the context.
     */
    void evalErrorChanged();

    /**
     * Will be set to TRUE if the current expression text reported a parser error
     * with the context.
     */
    void parserErrorChanged();

    //! Emitted whenever the tool tip changed
    void toolTipChanged( const QString &toolTip );

  public slots:
    //! sets the current feature used
    void setCurrentFeature( const QgsFeature &feature );


  private slots:
    void linkActivated( const QString & );
    void setEvalError( bool evalError );
    void setParserError( bool parserError );
    void copyFullExpressionValue();

  private:
    void setExpressionToolTip( const QString &toolTip );
    void refreshPreview();

    QgsVectorLayer *mLayer = nullptr;
    QgsExpressionContext mExpressionContext;
    QgsDistanceArea mDa;
    bool mUseGeomCalculator = false;
    QString mToolTip;
    bool mEvalError = true;
    bool mParserError = true;
    QString mExpressionText;
    QgsExpression mExpression;
    QAction *mCopyPreviewAction = nullptr;
};

#endif // QGSEXPRESSIONPREVIEWWIDGET_H
