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

    //! Valid data types accepted by property
    enum DataType
    {
      String = 1,
      Int = 1 << 1,
      Double = 1 << 2,
      AnyType = String | Int | Double
    };
    Q_DECLARE_FLAGS( DataTypes, DataType )

    /**
     * Constructor for QgsDataDefinedButtonV2.
     * @param parent parent widget
     * @param layer associated vector layer
     * @param propertyCollection associated property collection
     * @param propertyKey key for corresponding property
     * @param dataTypes expected data type for property values
     * @param description string description of expected input data
     */
    QgsDataDefinedButtonV2( QWidget* parent = nullptr,
                            const QgsVectorLayer* layer = nullptr,
                            const QgsAbstractProperty* property = nullptr,
                            const QgsDataDefinedButtonV2::DataTypes& dataTypes = AnyType,
                            const QString& description = QString() );

    /**
     * Initialize a newly constructed data defined button (useful if button was included in a form layout).
     * @param layer associated vector layer
     * @param propertyCollection associated property collection
     * @param propertyKey key for corresponding property
     * @param dataTypes expected data type for property values
     * @param description string description of expected input data
     */
    void init( const QgsVectorLayer* layer,
               const QgsAbstractProperty* property = nullptr,
               const QgsDataDefinedButtonV2::DataTypes& dataTypes = AnyType,
               const QString& description = QString() );

    QgsAbstractProperty* toProperty();

    void setToProperty( const QgsAbstractProperty* property );

    /**
     * Returns true if the button has an active property.
     */
    bool isActive() const { return mActive; }

    /**
     * The valid data types that will work for the definition (QVariant-coercible to expected type)
     * Compared against the variant type of the QgsField from data source and expression result
     */
    const DataTypes& validDataTypes() const { return mDataTypes; }

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


    //! Callback function for retrieving the expression context for the button
    typedef QgsExpressionContext( *ExpressionContextCallback )( const void* context );

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the button.
     * @param generator A QgsExpressionContextGenerator class that will be used to
     *                  create an expression context when required.
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator* generator );

    /**
     * Common descriptions for expected input values
     */
    static QString trString();
    static QString charDesc();
    static QString boolDesc();
    static QString anyStringDesc();
    static QString intDesc();
    static QString intPosDesc();
    static QString intPosOneDesc();
    static QString doubleDesc();
    static QString doublePosDesc();
    static QString double0to1Desc();
    static QString doubleXYDesc();
    static QString double180RotDesc();
    static QString intTranspDesc();
    static QString unitsMmMuDesc();
    static QString unitsMmMuPercentDesc();
    static QString colorNoAlphaDesc();
    static QString colorAlphaDesc();
    static QString textHorzAlignDesc();
    static QString textVertAlignDesc();
    static QString penJoinStyleDesc();
    static QString blendModesDesc();
    static QString svgPathDesc();
    static QString filePathDesc();
    static QString paperSizeDesc();
    static QString paperOrientationDesc();
    static QString horizontalAnchorDesc();
    static QString verticalAnchorDesc();
    static QString gradientTypeDesc();
    static QString gradientCoordModeDesc();
    static QString gradientSpreadDesc();
    static QString lineStyleDesc();
    static QString capStyleDesc();
    static QString fillStyleDesc();
    static QString markerStyleDesc();
    static QString customDashDesc();

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

    DataTypes mDataTypes;
    QString mDataTypesString;
    QString mInputDescription;
    QString mFullDescription;
    QString mUsageInfo;

    QgsExpressionContextGenerator* mExpressionContextGenerator;

  private slots:
    void aboutToShowMenu();
    void menuActionTriggered( QAction* action );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsDataDefinedButtonV2::DataTypes )


#endif // QGSDATADEFINEDBUTTONV2_H
