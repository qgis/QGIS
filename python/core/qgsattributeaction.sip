/***************************************************************************
                          qgsattributeaction.sip

 This is a legacy header to keep backwards compatibility until QGIS 3.

                             -------------------
    begin                : Oct 24 2004
    copyright            : (C) 2004 by Gavin Macaulay
    email                : gavin at macaulay dot co dot nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \class QgsAttributeAction
 * QgsAttributeAction is deprecated and has been replaced with
 * QgsActionManager. This legacy typedef will be removed with QGIS 3.
 */

class QgsAttributeAction
{
%TypeHeaderCode
#include <qgsattributeaction.h>
%End
  public:
    //! Constructor
    QgsAttributeAction( QgsVectorLayer *layer );

    //! Destructor
    virtual ~QgsAttributeAction();

    /** Add an action with the given name and action details.
     * Will happily have duplicate names and actions. If
     * capture is true, when running the action using doAction(),
     * any stdout from the process will be captured and displayed in a
     * dialog box.
     */
    void addAction( QgsAction::ActionType type, const QString& name, const QString& action, bool capture = false );

    /** Add an action with the given name and action details.
     * Will happily have duplicate names and actions. If
     * capture is true, when running the action using doAction(),
     * any stdout from the process will be captured and displayed in a
     * dialog box.
     */
    void addAction( QgsAction::ActionType type, const QString& name, const QString& action, const QString& icon, bool capture = false );

    //! Remove an action at given index
    void removeAction( int index );

    /** Does the given values. defaultValueIndex is the index of the
     *  field to be used if the action has a $currfield placeholder.
     *  @note available in python bindings as doActionFeature
     */
    void doAction( int index,
                   const QgsFeature &feat,
                   int defaultValueIndex = 0 ) /PyName=doActionFeature/;

    /** Does the action using the expression engine to replace any embedded expressions
     * in the action definition.
     * @param index action index
     * @param feature feature to run action for
     * @param context expression context to evalute expressions under
     * @param substitutionMap deprecated - kept for compatibility with projects, will be removed for 3.0
     */
    // TODO QGIS 3.0 remove substition map - force use of expression variables
    void doAction( int index,
                   const QgsFeature& feature,
                   const QgsExpressionContext& context,
                   const QMap<QString, QVariant> *substitutionMap = nullptr );


    /** Does the action using the expression builder to expand it
     *  and getting values from the passed feature attribute map.
     *  substitutionMap is used to pass custom substitutions, to replace
     *  each key in the map with the associated value
     *  @note available in python bindings as doActionFeatureWithSubstitution
     *  @deprecated use QgsExpressionContext variant instead
     */
    void doAction( int index,
                   const QgsFeature &feat,
                   const QMap<QString, QVariant> *substitutionMap ) /Deprecated,PyName=doActionFeatureWithSubstitution/;

    //! Removes all actions
    void clearActions();

    //! List all actions
    QList<QgsAction> listActions() const;

    //! Return the layer
    QgsVectorLayer* layer() const;

    /** Expands the given action, replacing all %'s with the value as
     *  given.
     */
    QString expandAction( QString action, const QMap<int, QVariant> &attributes, uint defaultValueIndex );

    /** Expands the given action using the expression builder
     *  This function currently replaces each expression between [% and %]
     *  placeholders in the action with the result of its evaluation on
     *  the feature passed as argument.
     *
     *  Additional substitutions can be passed through the substitutionMap
     *  parameter
     */
    QString expandAction( const QString& action,
                          QgsFeature &feat,
                          const QMap<QString, QVariant> *substitutionMap = 0 );


    //! Writes the actions out in XML format
    bool writeXML( QDomNode& layer_node, QDomDocument& doc ) const;

    //! Reads the actions in in XML format
    bool readXML( const QDomNode& layer_node );

    int size() const;
    /**
     * Get the action at the specified index.
     */
    QgsAction at( int idx ) const;
    /**
     * Get the action at the specified index.
     */
    QgsAction operator[]( int idx ) const;

    /** @deprecated Initialize QgsPythonRunner instead
     * @note not available in Python bindings
     */
    // Q_DECL_DEPRECATED static void setPythonExecute( void ( * )( const QString & ) );

    /**
     * Get the index of the default action
     */
    int defaultAction() const;
    /**
     * Set the index of the default action
     */
    void setDefaultAction( int actionNumber );
};
