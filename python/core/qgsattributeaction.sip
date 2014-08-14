class QgsAction
{
%TypeHeaderCode
#include "qgsattributeaction.h"
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

    QgsAction( ActionType type, QString name, QString action, bool capture );

    //! The name of the action
    QString name() const;

    //! The action
    QString action() const;

    //! The action type
    ActionType type() const;

    //! Whether to capture output for display when this action is run
    bool capture() const;

    //! Whether the action is runable on the current platform
    bool runable() const;
};

/*! \class QgsAttributeAction
 * \brief Storage and management of actions associated with Qgis layer
 * attributes.
 */

class QgsAttributeAction
{
%TypeHeaderCode
#include "qgsattributeaction.h"
%End
  public:
    //! Constructor
    QgsAttributeAction( QgsVectorLayer *layer );

    //! Destructor
    virtual ~QgsAttributeAction();

    //! Add an action with the given name and action details.
    // Will happily have duplicate names and actions. If
    // capture is true, when running the action using doAction(),
    // any stdout from the process will be captured and displayed in a
    // dialog box.
    void addAction( QgsAction::ActionType type, QString name, QString action, bool capture = false );

    //! Remove an action at given index
    void removeAction( int index );

    /*! Does the given values. defaultValueIndex is the index of the
     *  field to be used if the action has a $currfield placeholder.
     *  @note added in 1.9
     *  @note available in python bindings as doActionFeature
     */
    void doAction( int index,
                   QgsFeature &feat,
                   int defaultValueIndex = 0 ) /PyName=doActionFeature/;

    /*! Does the action using the expression builder to expand it
     *  and getting values from the passed feature attribute map.
     *  substitutionMap is used to pass custom substitutions, to replace
     *  each key in the map with the associated value
     *  @note added in 1.9
     *  @note available in python bindings as doActionFeatureWithSubstitution
     */
    void doAction( int index,
                   QgsFeature &feat,
                   const QMap<QString, QVariant> *substitutionMap ) /PyName=doActionFeatureWithSubstitution/;

    //! Removes all actions
    void clearActions();

    //! List all actions
    const QList<QgsAction>& listActions();

    //! Return the layer
    QgsVectorLayer *layer();

    /*! Expands the given action, replacing all %'s with the value as
     *  given.
     */
    QString expandAction( QString action, const QMap<int, QVariant> &attributes, uint defaultValueIndex );

    /*! Expands the given action using the expression builder
     *  This function currently replaces each expression between [% and %]
     *  placeholders in the action with the result of its evaluation on
     *  the feature passed as argument.
     *
     *  Additional substitutions can be passed through the substitutionMap
     *  parameter
     *
     *  @note added in 1.9
     */
    QString expandAction( QString action,
                          QgsFeature &feat,
                          const QMap<QString, QVariant> *substitutionMap = 0 );


    //! Writes the actions out in XML format
    bool writeXML( QDomNode& layer_node, QDomDocument& doc ) const;

    //! Reads the actions in in XML format
    bool readXML( const QDomNode& layer_node );

    int size() const;
    QgsAction &at( int idx );
    QgsAction &operator[]( int idx );

    //! Whether the action is the default action
    int defaultAction() const;
    void setDefaultAction( int actionNumber );
};
