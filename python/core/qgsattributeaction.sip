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

    bool runable() const;
};

class QgsAttributeAction
{
%TypeHeaderCode
#include "qgsattributeaction.h"
%End
  public:
    QgsAttributeAction();

    //! Destructor
    virtual ~QgsAttributeAction();

    //! Add an action with the given name and action details.
    // Will happily have duplicate names and actions. If
    // capture is true, when running the action using doAction(),
    // any stdout from the process will be captured and displayed in a
    // dialog box.
    void addAction( QgsAction::ActionType type, QString name, QString action, bool capture = false );

/*
    //! Does the action using the given values. defaultValueIndex is an
    // index into values which indicates which value in the values vector
    // is to be used if the action has a default placeholder.
    void doAction( int index, const QList< QPair<QString, QString> > &values,
                   int defaultValueIndex = 0, void *executePython = 0 );
 */

    //! Removes all actions
    void clearActions();

    //! Expands the given action, replacing all %'s with the value as
    // given.
    static QString expandAction( QString action, const QList< QPair<QString, QString> > &values,
                                 uint defaultValueIndex );

    //! Writes the actions out in XML format
    bool writeXML( QDomNode& layer_node, QDomDocument& doc ) const;

    //! Reads the actions in in XML format
    bool readXML( const QDomNode& layer_node );

    //! interface to inherited methods from QList<QgsAction>
    const QgsAction &at( int idx );
    const int size();
};
