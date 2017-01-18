/***************************************************************************
     qgsdatadefinedbuttonv2.h
     ------------------------
    Date                 : March 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATADEFINEDBUTTONV2_H
#define QGSDATADEFINEDBUTTONV2_H

#include "qgis_gui.h"
#include <QDialog>
#include <QFlags>
#include <QMap>
#include <QPointer>
#include <QToolButton>
#include <QScopedPointer>
#include "qgsproperty.h"
#include "qgspropertycollection.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextgenerator.h"

class QgsVectorLayer;
class QgsDataDefined;
class QgsMapCanvas;
class QgsDataDefinedAssistant;

/** \ingroup gui
 * \class QgsDataDefinedButtonV2
 * A button for defining data source field mappings or expressions.
 */

class GUI_EXPORT QgsDataDefinedButtonV2: public QToolButton
{
    Q_OBJECT
    Q_PROPERTY( QString usageInfo READ usageInfo WRITE setUsageInfo )
    Q_PROPERTY( bool active READ isActive WRITE setActive )

  public:

    /**
     * Constructor for QgsDataDefinedButtonV2.
     * @param parent parent widget
     * @param layer associated vector layer
     */
    QgsDataDefinedButtonV2( QWidget* parent = nullptr,
                            const QgsVectorLayer* layer = nullptr );

    /**
     * Initialize a newly constructed property button (useful if button was included in a form layout).
     * @param propertyKey key for corresponding property
     * @param property associated property
     * @param definitions properties definitions for collection
     * @param layer associated vector layer
     */
    void init( int propertyKey,
               const QgsProperty& property,
               const QgsPropertiesDefinition& definitions,
               const QgsVectorLayer* layer = nullptr );

    /**
     * Initialize a newly constructed property button (useful if button was included in a form layout).
     * @param propertyKey key for corresponding property
     * @param collection associated property collection
     * @param definitions properties definitions for collection
     * @param layer associated vector layer
     */
    void init( int propertyKey,
               const QgsAbstractPropertyCollection& collection,
               const QgsPropertiesDefinition& definitions,
               const QgsVectorLayer* layer = nullptr );

    QgsProperty toProperty() const;

    void setToProperty( const QgsProperty& property );

    int propertyKey() const { return mPropertyKey; }

    /**
     * Returns true if the button has an active property.
     */
    bool isActive() const { return mActive; }

    /**
     * The valid data types that will work for the definition (QVariant-coercible to expected type)
     * Compared against the variant type of the QgsField from data source and expression result
     */
    QgsPropertyDefinition::DataType validDataType() const { return mDataTypes; }

    /**
     * The full definition description and current definition (internally generated on a contextual basis)
     */
    QString fullDescription() const { return mFullDescription; }

    /**
     * The usage information about this data definition
     */
    QString usageInfo() const { return mUsageInfo; }

    /**
     * Set the usage information about this data definition
     */
    void setUsageInfo( const QString& info ) { mUsageInfo = info; updateGui(); }

    /**
     * Sets the vector layer associated with the button. This controls which fields are
     * displayed within the widget's pop up menu.
     * @see vectorLayer()
     */
    void setVectorLayer( const QgsVectorLayer* layer );

    /**
     * Returns the vector layer associated with the button. This controls which fields are
     * displayed within the widget's pop up menu.
     * @see setVectorLayer()
     */
    const QgsVectorLayer* vectorLayer() const { return mVectorLayer; }

    //TODO

    /**
     * Register a sibling widget that get checked when data definition or expression is active
     */
    void registerCheckedWidget( QWidget* widget );

    /**
     * Sets an assistant used to define the data defined object properties.
     * Ownership of the assistant is transferred to the widget.
     * @param title menu title for the assistant
     * @param assistant data defined assistant. Set to null to remove the assistant
     * option from the button.
     * @note added in 2.10
     * @see assistant()
     */
    void setAssistant( const QString& title, QgsDataDefinedAssistant * assistant );

    /** Returns the assistant used to defined the data defined object properties, if set.
     * @see setAssistant()
     * @note added in QGIS 2.12
     */
    QgsDataDefinedAssistant* assistant();

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the button.
     * @param generator A QgsExpressionContextGenerator class that will be used to
     *                  create an expression context when required.
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator* generator );

  public slots:

    /**
     * Set whether the current data definition or expression is to be used
     */
    void setActive( bool active );

  signals:

    //! Emitted when property definition changes
    void changed();

    //! Emitted when the activated status of the widget changes
    void activated( bool isActive );

  protected:
    void mouseReleaseEvent( QMouseEvent *event ) override;

  private:

    void updateFieldLists();

    void showDescriptionDialog();
    void showExpressionDialog();
    void updateGui();

    /**
     * Sets the active status, emitting the activated signal where necessary (but never emitting the changed signal!).
     * Call this when you know you'll later be emitting the changed signal and want to avoid duplicate signals.
     */
    void setActivePrivate( bool active );

    int mPropertyKey = -1;

    const QgsVectorLayer* mVectorLayer = nullptr;

    QStringList mFieldNameList;
    QStringList mFieldTypeList;

    bool mActive;
    bool mUseExpression;
    QString mExpressionString;
    QString mFieldName;

    QMenu* mDefineMenu;
    QAction* mActionDataTypes;
    QMenu* mFieldsMenu;
    QMenu* mVariablesMenu;
    QAction* mActionVariables;

    QAction* mActionActive;
    QAction* mActionDescription;
    QAction* mActionExpDialog;
    QAction* mActionExpression;
    QAction* mActionPasteExpr;
    QAction* mActionCopyExpr;
    QAction* mActionClearExpr;
    QAction* mActionAssistant;

    QgsPropertyDefinition::DataType mDataTypes = QgsPropertyDefinition::DataTypeString;
    QString mDataTypesString;
    QString mInputDescription;
    QString mFullDescription;
    QString mUsageInfo;

    QgsExpressionContextGenerator* mExpressionContextGenerator;

  private slots:
    void aboutToShowMenu();
    void menuActionTriggered( QAction* action );
};


#endif // QGSDATADEFINEDBUTTONV2_H
