/***************************************************************************
  qgsaction.h - QgsAction

 ---------------------
 begin                : 18.4.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \ingroup core
 * Utility class that encapsulates an action based on vector attributes.
 */
class QgsAction
{
%TypeHeaderCode
#include <qgsaction.h>
%End
  public:
    enum ActionType
    {
      Generic,
      GenericPython,
      Mac,
      Windows,
      Unix,
      OpenUrl,
    };

    /**
     * Default constructor
     */
    QgsAction();

    /**
     * Create a new QgsAction
     *
     * @param type          The type of this action
     * @param description   A human readable description string
     * @param command       The action text. Its interpretation depends on the type
     * @param capture       If this is set to true, the output will be captured when an action is run
     */
    QgsAction( ActionType type, const QString& description, const QString& command, bool capture = false );

    /**
     * Create a new QgsAction
     *
     * @param type                 The type of this action
     * @param description          A human readable description string
     * @param action               The action text. Its interpretation depends on the type
     * @param icon                 Path to an icon for this action
     * @param capture              If this is set to true, the output will be captured when an action is run
     * @param shortTitle           A short string used to label user interface elements like buttons
     * @param actionScopes         A set of scopes in which this action will be available
     */
    QgsAction( ActionType type, const QString& description, const QString& action, const QString& icon, bool capture, const QString& shortTitle = QString(), const QSet<QString>& actionScopes = QSet<QString>() );

    //! The name of the action. This may be a longer description.
    QString name() const;

    //! The short title is used to label user interface elements like buttons
    QString shortTitle() const;

    /**
     * Returns a unique id for this action.
     *
     * @note Added in QGIS 3.0
     */
    QUuid id() const;

    /**
     * Returns true if this action was a default constructed one.
     *
     * @note Added in QGIS 3.0
     */
    bool isValid() const;

    //! The path to the icon
    QString iconPath() const;

    //! The icon
    QIcon icon() const;

    //! The command
    QString command() const;

    //! The action type
    ActionType type() const;

    //! Whether to capture output for display when this action is run
    bool capture() const;

    //! Checks if the action is runable on the current platform
    bool runable() const;

    /**
     * Run this action.
     *
     * @note Added in QGIS 3.0
     */
    void run( QgsVectorLayer* layer, const QgsFeature& feature, const QgsExpressionContext& expressionContext ) const;

    /**
     * Run this action.
     *
     * @note Added in QGIS 3.0
     */
    void run( const QgsExpressionContext& expressionContext ) const;

    /**
     * The action scopes define where an action will be available.
     * Action scopes may offer additional variables like the clicked
     * coordinate.
     *
     * @see QgsActionScope
     * @note Added in QGIS 3.0
     */
    QSet<QString> actionScopes() const;

    /**
     * The action scopes define where an action will be available.
     * Action scopes may offer additional variables like the clicked
     * coordinate.
     *
     * @note Added in QGIS 3.0
     */
    void setActionScopes( const QSet<QString>& actionScopes );

    /**
     * Reads an XML definition from actionNode
     * into this object.
     *
     * @note Added in QGIS 3.0
     */
    void readXml( const QDomNode& actionNode );

    /**
     * Appends an XML definition for this action as a new
     * child node to actionsNode.
     *
     * @note Added in QGIS 3.0
     */
    void writeXml(QDomNode& actionsNode ) const;
};
