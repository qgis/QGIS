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
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"

#include <functional>

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

#ifndef SIP_RUN

    /**
     * Sets the widget to run using a custom preview generator.
     *
     * In this mode, the widget will call a callback function to generate a new QgsExpressionContext
     * as the previewed object changes. This can be used to provide custom preview values for different
     * objects (i.e. for objects which aren't vector layer features).
     *
     * \param label The label to display for the combo box presenting choices of objects. This should be a representative name, eg "Band" if the widget is showing choices of raster layer bands
     * \param choices A list of choices to present to the user. Each choice is a pair of a human-readable label and a QVariant representing the object to preview.
     * \param previewContextGenerator A function which takes a QVariant representing the object to preview, and returns a QgsExpressionContext to use for previewing the object.
     *
     * \since QGIS 3.38
     */
    void setCustomPreviewGenerator( const QString &label, const QList< QPair< QString, QVariant > > &choices, const std::function< QgsExpressionContext( const QVariant & ) > &previewContextGenerator );
#else

    /**
     * Sets the widget to run using a custom preview generator.
     *
     * In this mode, the widget will call a callback function to generate a new QgsExpressionContext
     * as the previewed object changes. This can be used to provide custom preview values for different
     * objects (i.e. for objects which aren't vector layer features).
     *
     * \param label The label to display for the combo box presenting choices of objects. This should be a representative name, eg "Band" if the widget is showing choices of raster layer bands
     * \param choices A list of choices to present to the user. Each choice is a pair of a human-readable label and a QVariant representing the object to preview.
     * \param previewContextGenerator A function which takes a QVariant representing the object to preview, and returns a QgsExpressionContext to use for previewing the object.
     *
     * \since QGIS 3.38
     */
    void setCustomPreviewGenerator( const QString &label, const QList< QPair< QString, QVariant > > &choices, SIP_PYCALLABLE );
    % MethodCode
    Py_XINCREF( a2 );
    Py_BEGIN_ALLOW_THREADS
    sipCpp->setCustomPreviewGenerator( *a0, *a1, [a2]( const QVariant &value )->QgsExpressionContext
    {
      QgsExpressionContext res;
      SIP_BLOCK_THREADS
      PyObject *s = sipCallMethod( NULL, a2, "D", &value, sipType_QVariant, NULL );
      int state;
      int sipIsError = 0;
      QgsExpressionContext *t1 = reinterpret_cast<QgsExpressionContext *>( sipConvertToType( s, sipType_QgsExpressionContext, 0, SIP_NOT_NONE, &state, &sipIsError ) );
      if ( sipIsError == 0 )
      {
        res = QgsExpressionContext( *t1 );
      }
      sipReleaseType( t1, sipType_QgsExpressionContext, state );
      SIP_UNBLOCK_THREADS
      return res;
    } );

    Py_END_ALLOW_THREADS
    % End
#endif

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

    //! Returns the expression parser errors
    QList<QgsExpression::ParserError> parserErrors() const {return mExpression.parserErrors();}

    /**
     * Returns the current expression result preview text.
     *
     * \since QGIS 3.38
     */
    QString currentPreviewText() const;

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
    void setCustomChoice( int );

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

    std::function< QgsExpressionContext( const QVariant & ) > mCustomPreviewGeneratorFunction;
};

#endif // QGSEXPRESSIONPREVIEWWIDGET_H
