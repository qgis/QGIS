/***************************************************************************
     qgspropertyoverridebutton.h
     ---------------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROPERTYOVERRIDEBUTTON_H
#define QGSPROPERTYOVERRIDEBUTTON_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QDialog>
#include <QFlags>
#include <QMap>
#include <QPointer>
#include <QToolButton>
#include "qgsproperty.h"
#include "qgspropertycollection.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextgenerator.h"

class QgsVectorLayer;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \class QgsPropertyOverrideButton
 * A button for controlling property overrides which may apply to a widget.
 *
 * QgsPropertyOverrideButton is designed to be used alongside the QGIS
 * properties framework (QgsProperty, QgsPropertyDefinition
 * and QgsPropertyCollection).
 *
 * It allows users to specify field or expression based overrides
 * which should be applied to a property of an object. Eg, this widget
 * is used for controlling data defined overrides in symbology, labeling
 * and layouts.
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsPropertyOverrideButton: public QToolButton
{
    Q_OBJECT
    Q_PROPERTY( QString usageInfo READ usageInfo WRITE setUsageInfo )
    Q_PROPERTY( bool active READ isActive WRITE setActive )

  public:

    //! Flags controlling button behavior
    enum Flag
    {
      FlagDisableCheckedWidgetOnlyWhenProjectColorSet = 1 << 1, //!< Indicates that registered widgets will only be disabled when the property is set to follow a project color
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsPropertyOverrideButton.
     * \param parent parent widget
     * \param layer associated vector layer
     */
    QgsPropertyOverrideButton( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                               const QgsVectorLayer *layer = nullptr );

    /**
     * Returns the button's flags, which control the button behavior.
     *
     * \see setFlags()
     * \since QGIS 3.6
     */
    Flags flags() const;

    /**
     * Sets the button's \a flags, which control the button behavior.
     *
     * \see flags()
     * \since QGIS 3.6
     */
    void setFlags( QgsPropertyOverrideButton::Flags flags );

    /**
     * Initialize a newly constructed property button (useful if button was included in a UI layout).
     * \param propertyKey key for corresponding property
     * \param property initial value of associated property to show in widget
     * \param definitions properties definitions for corresponding collection
     * \param layer associated vector layer
     * \param auxiliaryStorageEnabled If true, activate the button to store data defined in auxiliary storage
     */
    void init( int propertyKey,
               const QgsProperty &property,
               const QgsPropertiesDefinition &definitions,
               const QgsVectorLayer *layer = nullptr,
               bool auxiliaryStorageEnabled = false );

    /**
     * Initialize a newly constructed property button (useful if button was included in a UI layout).
     * \param propertyKey key for corresponding property
     * \param property initial value of associated property to show in widget
     * \param definition properties definition for button
     * \param layer associated vector layer
     * \param auxiliaryStorageEnabled If true, activate the button to store data defined in auxiliary storage
     */
    void init( int propertyKey,
               const QgsProperty &property,
               const QgsPropertyDefinition &definition,
               const QgsVectorLayer *layer = nullptr,
               bool auxiliaryStorageEnabled = false );

    /**
     * Initialize a newly constructed property button (useful if button was included in a UI layout).
     * \param propertyKey key for corresponding property
     * \param collection associated property collection
     * \param definitions properties definitions for collection
     * \param layer associated vector layer
     * \param auxiliaryStorageEnabled If true, activate the button to store data defined in auxiliary storage
     */
    void init( int propertyKey,
               const QgsAbstractPropertyCollection &collection,
               const QgsPropertiesDefinition &definitions,
               const QgsVectorLayer *layer = nullptr,
               bool auxiliaryStorageEnabled = false );

    /**
     * Returns a QgsProperty object encapsulating the current state of the
     * widget.
     * \see setToProperty()
     */
    QgsProperty toProperty() const;

    /**
     * Sets the widget to reflect the current state of a QgsProperty.
     */
    void setToProperty( const QgsProperty &property );

    /**
     * Returns the property key linked to the button.
     */
    int propertyKey() const { return mPropertyKey; }

    /**
     * Returns true if the button has an active property.
     */
    bool isActive() const { return mProperty && mProperty.isActive(); }

    /**
     * Returns the data type which the widget will accept. This is used to filter
     * out fields from the associated vector layer to only show fields which
     * are compatible with the property.
     */
    QgsPropertyDefinition::DataType validDataType() const { return mDataTypes; }

    /**
     * Returns the full definition description and current definition
     * (internally generated on a contextual basis).
     */
    QString fullDescription() const { return mFullDescription; }

    /**
     * Returns usage information for the property.
     * \see setUsageInfo()
     */
    QString usageInfo() const { return mUsageInfo; }

    /**
     * Set the usage information for the property.
     * \see usageInfo()
     */
    void setUsageInfo( const QString &info ) { mUsageInfo = info; updateGui(); }

    /**
     * Sets the vector layer associated with the button. This controls which fields are
     * displayed within the widget's pop up menu.
     * \see vectorLayer()
     */
    void setVectorLayer( const QgsVectorLayer *layer );

    /**
     * Returns the vector layer associated with the button. This controls which fields are
     * displayed within the widget's pop up menu.
     * \see setVectorLayer()
     */
    const QgsVectorLayer *vectorLayer() const { return mVectorLayer; }

    /**
     * Register a sibling \a widget that gets checked when the property is active.
     * if \a natural is false, widget gets unchecked when the property is active.
     * \note this should be called after calling init() to be correctly initialized.
     */
    void registerCheckedWidget( QWidget *widget, bool natural = true );

    /**
     * Register a sibling \a widget that gets enabled when the property is active, and disabled when the property is inactive.
     * if \a natural is false, widget gets disabled when the property is active, and enabled when the property is inactive.
     * \note this should be called after calling init() to be correctly initialized.
     */
    void registerEnabledWidget( QWidget *widget, bool natural = true );

    /**
     * Register a sibling \a widget that gets visible when the property is active, and hidden when the property is inactive.
     * if \a natural is false, widget gets hidden when the property is active, and visible when the property is inactive.
     * \note this should be called after calling init() to be correctly initialized.
     */
    void registerVisibleWidget( QWidget *widget, bool natural = true );

    /**
     * Register a sibling \a widget (line edit, text edit) that will receive the property as an expression
     * \note this should be called after calling init() to be correctly initialized.
     */
    void registerExpressionWidget( QWidget *widget );

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the button when required.
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

    /**
     * Updates list of fields.
     *
     * \since QGIS 3.0
     */
    void updateFieldLists();

    /**
     * Sets a symbol which can be used for previews inside the widget or in any dialog created
     * by the widget. If not specified, a default created symbol will be used instead.
     * \note not available in Python bindings
     */
    void setSymbol( std::shared_ptr< QgsSymbol > symbol ) { mSymbol = symbol; } SIP_SKIP

  public slots:

    /**
     * Set whether the current property override definition is to be used
     */
    void setActive( bool active );


    ///@cond PRIVATE

    // exposed to Python for testing only
    void aboutToShowMenu();
    void menuActionTriggered( QAction *action );

    ///@endcond

  signals:

    //! Emitted when property definition changes
    void changed();

    //! Emitted when the activated status of the widget changes
    void activated( bool isActive );

    //! Emitted when creating a new auxiliary field
    void createAuxiliaryField();

  protected:
    void mouseReleaseEvent( QMouseEvent *event ) override;

  private:

    void showDescriptionDialog();
    void showExpressionDialog();
    void showAssistant();
    void updateGui();

    /**
     * Sets the active status, emitting the activated signal where necessary (but never emitting the changed signal!).
     * Call this when you know you'll later be emitting the changed signal and want to avoid duplicate signals.
     */
    void setActivePrivate( bool active );


    int mPropertyKey = -1;

    const QgsVectorLayer *mVectorLayer = nullptr;

    QStringList mFieldNameList;
    QStringList mFieldTypeList;

    QString mExpressionString;
    QString mFieldName;

    QMenu *mDefineMenu = nullptr;
    QAction *mActionDataTypes = nullptr;
    QMenu *mFieldsMenu = nullptr;
    QMenu *mVariablesMenu = nullptr;
    QAction *mActionVariables = nullptr;
    QMenu *mColorsMenu = nullptr;
    QAction *mActionColors = nullptr;

    QAction *mActionActive = nullptr;
    QAction *mActionDescription = nullptr;
    QAction *mActionExpDialog = nullptr;
    QAction *mActionExpression = nullptr;
    QAction *mActionPasteExpr = nullptr;
    QAction *mActionCopyExpr = nullptr;
    QAction *mActionClearExpr = nullptr;
    QAction *mActionAssistant = nullptr;
    QAction *mActionCreateAuxiliaryField = nullptr;

    QgsPropertyDefinition mDefinition;

    QgsPropertyDefinition::DataType mDataTypes = QgsPropertyDefinition::DataTypeString;
    QString mDataTypesString;
    QString mInputDescription;
    QString mFullDescription;
    QString mUsageInfo;

    QgsExpressionContextGenerator *mExpressionContextGenerator = nullptr;

    enum SiblingType
    {
      SiblingCheckState,
      SiblingEnableState,
      SiblingVisibility,
      SiblingExpressionText,
    };
    struct SiblingWidget
    {
      SiblingWidget( const QPointer<QWidget> &widgetPointer, SiblingType siblingType, bool natural = true )
        : mWidgetPointer( widgetPointer )
        , mSiblingType( siblingType )
        , mNatural( natural )
      {}
      QPointer<QWidget> mWidgetPointer;
      SiblingType mSiblingType;
      bool mNatural;
    };
    QList< SiblingWidget > mSiblingWidgets;

    //! Internal property used for storing state of widget
    QgsProperty mProperty;

    bool mAuxiliaryStorageEnabled = false;

    std::shared_ptr< QgsSymbol > mSymbol;

    Flags mFlags = nullptr;

  private slots:

    void showHelp();
    void updateSiblingWidgets( bool state );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsPropertyOverrideButton::Flags )

#endif // QGSPROPERTYOVERRIDEBUTTON_H
