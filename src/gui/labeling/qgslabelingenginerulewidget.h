/***************************************************************************
    qgslabelingenginerulewidget.h
    ------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELINGENGINERULEWIDGET_H
#define QGSLABELINGENGINERULEWIDGET_H

#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "qgsguiutils.h"
#include "ui_qgslabelingruleavoidoverlapwidgetbase.h"
#include "ui_qgslabelingrulemindistancelabeltofeaturewidgetbase.h"
#include "ui_qgslabelingrulemaxdistancelabeltofeaturewidgetbase.h"
#include "ui_qgslabelingrulemindistancelabeltolabelwidgetbase.h"

#include <QDialog>

class QgsAbstractLabelingEngineRule;
class QDialogButtonBox;

/**
 * \ingroup gui
 * \class QgsLabelingEngineRuleWidget
 * \brief Base class for widgets which allow control over the properties of QgsAbstractLabelingEngineRule subclasses
 *
 * \see QgsLabelingEngineRulesWidget for a widget for configuring multiple rules
 * \see QgsLabelingEngineRuleDialog
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsLabelingEngineRuleWidget : public QgsPanelWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelingEngineRuleWidget.
     */
    QgsLabelingEngineRuleWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QgsPanelWidget( parent )
    {}

    /**
     * Sets the \a rule to show in the widget. Ownership is not transferred.
     * \see rule()
     */
    virtual void setRule( const QgsAbstractLabelingEngineRule *rule ) = 0;

    /**
     * Returns the rule defined by the current settings in the widget.
     *
     * Ownership of the returned object is transferred to the caller
     *
     * \see setRule()
     */
    virtual QgsAbstractLabelingEngineRule *rule() = 0 SIP_TRANSFERBACK;

  signals:

    /**
     * Emitted whenever the configuration of the rule is changed.
     */
    void changed();
};

#ifndef SIP_RUN

/**
 * \class QgsLabelingEngineRuleDialog
 * \ingroup gui
 * \brief A simple dialog for customizing a labeling engine rule.
 *
 * \note Not available in Python bindings
 *
 * \see QgsLabelingEngineRuleWidget()
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsLabelingEngineRuleDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelingEngineRuleDialog.
     * \param widget rule widget to show in dialog
     * \param parent parent widget
     * \param flags window flags for dialog
     */
    QgsLabelingEngineRuleDialog( QgsLabelingEngineRuleWidget *widget, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = QgsGuiUtils::ModalDialogFlags );

    /**
     * Sets the \a rule to show in the dialog. Ownership is not transferred.
     * \see rule()
     */
    void setRule( const QgsAbstractLabelingEngineRule *rule );

    /**
     * Returns the rule defined by the current settings in the dialog.
     *
     * Ownership of the returned object is transferred to the caller
     *
     * \see setRule()
     */
    QgsAbstractLabelingEngineRule *rule() SIP_TRANSFERBACK;

  private:
    QgsLabelingEngineRuleWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};


///@cond PRIVATE
class GUI_EXPORT QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget : public QgsLabelingEngineRuleWidget, private Ui_QgsLabelingRuleAvoidLabelOverlapWithFeatureWidgetBase
{
    Q_OBJECT

  public:
    QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget( QWidget *parent = nullptr );

    void setRule( const QgsAbstractLabelingEngineRule *rule ) override;
    QgsAbstractLabelingEngineRule *rule() override SIP_TRANSFERBACK;

  private slots:

    void onChanged();

  private:
    bool mBlockSignals = false;
};

class GUI_EXPORT QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget : public QgsLabelingEngineRuleWidget, private Ui_QgsLabelingRuleMinimumDistanceLabelToFeatureWidgetBase
{
    Q_OBJECT

  public:
    QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget( QWidget *parent = nullptr );

    void setRule( const QgsAbstractLabelingEngineRule *rule ) override;
    QgsAbstractLabelingEngineRule *rule() override SIP_TRANSFERBACK;

  private slots:

    void onChanged();

  private:
    bool mBlockSignals = false;
};

class GUI_EXPORT QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget : public QgsLabelingEngineRuleWidget, private Ui_QgsLabelingRuleMaximumDistanceLabelToFeatureWidgetBase
{
    Q_OBJECT

  public:
    QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget( QWidget *parent = nullptr );

    void setRule( const QgsAbstractLabelingEngineRule *rule ) override;
    QgsAbstractLabelingEngineRule *rule() override SIP_TRANSFERBACK;

  private slots:

    void onChanged();

  private:
    bool mBlockSignals = false;
};

class GUI_EXPORT QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget : public QgsLabelingEngineRuleWidget, private Ui_QgsLabelingRuleMinimumDistanceLabelToLabelWidgetBase
{
    Q_OBJECT

  public:
    QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget( QWidget *parent = nullptr );

    void setRule( const QgsAbstractLabelingEngineRule *rule ) override;
    QgsAbstractLabelingEngineRule *rule() override SIP_TRANSFERBACK;

  private slots:

    void onChanged();

  private:
    bool mBlockSignals = false;
};

///@endcond


#endif


#endif // QGSLABELINGENGINERULEWIDGET_H
