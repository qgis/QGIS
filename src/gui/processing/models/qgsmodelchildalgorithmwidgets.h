/***************************************************************************
                         qgsmodelchildalgorithmwidgets.h
                         ----------------------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROCESSINGMODELCHILDALGORITHMNWIDGETS_H
#define QGSPROCESSINGMODELCHILDALGORITHMNWIDGETS_H

#include <memory>

#include "qgis_gui.h"
#include "qgsmodeldesignerconfigwidget.h"
#include "qgspanelwidget.h"
#include "qgsprocessingwidgetwrapper.h"

#include <QDialog>

#define SIP_NO_FILE

class QgsProcessingAlgorithm;
class QgsProcessingModelAlgorithm;
class QgsProcessingContext;
class QgsProcessingModelChildAlgorithm;
class QgsProcessingModelOutput;
class QLineEdit;
class QgsMessageBar;
class QgsProcessingAlgorithmConfigurationWidget;
class QgsProcessingModelerParameterWidget;
class QgsModelChildDependenciesWidget;
class QgsProcessingContextGenerator;
class QTabWidget;
class QgsPanelWidgetStack;
class QTextEdit;
class QgsColorButton;
class QDialogButtonBox;


/**
 * A panel widget displaying the configuration for a child algorithm in a Processing model.
 *
 * \note Not available in Python bindings
 *
 * \ingroup gui
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsProcessingModelerParametersPanelWidget : public QgsPanelWidget, public QgsProcessingContextGenerator
{
    Q_OBJECT

  public:
    /**
   * Constructor for QgsProcessingModelerParametersPanelWidget.
   */
    QgsProcessingModelerParametersPanelWidget(
      const QgsProcessingAlgorithm *childAlgorithm,
      QgsProcessingModelAlgorithm *model,
      QgsProcessingContext &context,
      const QString &childId = QString(),
      const QVariantMap &configuration = QVariantMap(),
      QWidget *parent = nullptr,
      QWidget *dialog = nullptr
    );

    ~QgsProcessingModelerParametersPanelWidget() override;

    /**
   * Returns the algorithm associated with the widget.
   */
    const QgsProcessingAlgorithm *algorithm() const;

    /**
   * Creates the child algorithm instance populated with current widget values.
   */
    std::unique_ptr< QgsProcessingModelChildAlgorithm > createAlgorithm();

    QgsProcessingContext *processingContext() const override;

    /**
     * Sets the \a context in which the panel is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     */
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

    /**
     * Sets widget state from the existing child algorithm definition in the model.
     */
    void setStateFromChildAlgorithm();

  private:
    void setupUi();
    void emitChangedSignal();

    std::unique_ptr< QgsProcessingAlgorithm > mAlgorithm;
    QgsProcessingModelAlgorithm *mModel = nullptr;
    QString mChildId;
    QVariantMap mConfiguration;
    QgsProcessingContext &mContext;
    QWidget *mDialog = nullptr;

    QMap< QString, QgsProcessingModelOutput > mPreviousOutputDefinitions;
    int mBlockChangesSignal = 0;

    QLineEdit *mDescriptionBox = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QgsProcessingAlgorithmConfigurationWidget *mAlgorithmItem = nullptr;
    QMap< QString, QgsProcessingModelerParameterWidget * > mWrappers;
    QgsModelChildDependenciesWidget *mDependenciesPanel = nullptr;

    friend class TestQgsProcessingModelGui;
};

/**
 * A panel config widget combining parameter settings and comments for a child algorithm in a Processing model.
 *
 * \note Not available in Python bindings
 *
 * \ingroup gui
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsProcessingModelerParametersWidget : public QgsProcessingModelConfigWidget
{
    Q_OBJECT

  public:
    /**
   * Constructor for QgsProcessingModelerParametersWidget.
   */
    QgsProcessingModelerParametersWidget(
      const QgsProcessingAlgorithm *childAlgorithm,
      QgsProcessingModelAlgorithm *model,
      QgsProcessingContext &context,
      const QString &childId = QString(),
      const QVariantMap &configuration = QVariantMap(),
      QWidget *parent = nullptr,
      QWidget *dialog = nullptr
    );

    ~QgsProcessingModelerParametersWidget() override;

    /**
   * Returns the algorithm associated with the widget.
   */
    const QgsProcessingAlgorithm *algorithm() const;

    /**
   * Sets the comment \a text.
   *
   * \see comments()
   */
    void setComments( const QString &text );

    /**
   * Returns the comment text.
   *
   * \see setComments()
   */
    QString comments() const;

    /**
   * Sets the comment's \a color.
   *
   * \see commentColor()
   */
    void setCommentColor( const QColor &color );

    /**
   * Returns the comment's color.
   *
   * \see setCommentColor()
   */
    QColor commentColor() const;

    /**
   * Focuses the widget on the comment editing tab.
   */
    void switchToCommentTab();

    /**
     * Sets the \a context in which the panel is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     */
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

    /**
   * Sets widget state from the existing child algorithm definition in the model.
   */
    void setStateFromChildAlgorithm();

    /**
   * Creates the child algorithm instance, populated with the current widget parameter values and comments.
   */
    std::unique_ptr< QgsProcessingModelChildAlgorithm > createAlgorithm();

  private:
    void setupUi();

    QTabWidget *mTabWidget = nullptr;
    QgsPanelWidgetStack *mPanelWidgetStack = nullptr;
    QgsProcessingModelerParametersPanelWidget *mParametersPanel = nullptr;
    QTextEdit *mCommentEdit = nullptr;
    QgsColorButton *mCommentColorButton = nullptr;

    friend class TestQgsProcessingModelGui;
};

/**
 * A dialog for configuring parameter settings and comments for a child algorithm in a Processing model.
 * \ingroup gui
 * \note Not available in Python bindings
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsProcessingModelerParametersDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
   * Constructor for QgsProcessingModelerParametersDialog.
   */
    QgsProcessingModelerParametersDialog(
      const QgsProcessingAlgorithm *childAlgorithm,
      QgsProcessingModelAlgorithm *model,
      QgsProcessingContext &context,
      const QString &childId = QString(),
      const QVariantMap &configuration = QVariantMap(),
      QWidget *parent = nullptr
    );

    ~QgsProcessingModelerParametersDialog() override;

    /**
   * Returns the algorithm associated with the dialog.
   */
    const QgsProcessingAlgorithm *algorithm() const;

    /**
   * Sets the algorithm's \a comments.
   *
   * \see comments()
   */
    void setComments( const QString &comments );

    /**
   * Returns the algorithm's comments.
   *
   * \see setComments()
   */
    QString comments() const;

    /**
   * Sets the algorithm's comment \a color.
   *
   * \see commentColor()
   */
    void setCommentColor( const QColor &color );

    /**
   * Returns the algorithm's comment color.
   *
   * \see setCommentColor()
   */
    QColor commentColor() const;

    /**
   * Focuses the dialog on the comment editing tab.
   */
    void switchToCommentTab();

    /**
     * Sets the \a context in which the dialog is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     */
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

    /**
   * Sets widget state from the existing child algorithm definition in the model.
   */
    void setStateFromChildAlgorithm();

    /**
   * Creates the child algorithm instance, populated with the current dialog parameter values and comments.
   */
    std::unique_ptr< QgsProcessingModelChildAlgorithm > createAlgorithm();

  private slots:
    void okPressed();
    void openHelp();

  private:
    QgsProcessingModelerParametersWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};


#endif // QGSPROCESSINGMODELCHILDALGORITHMNWIDGETS_H
