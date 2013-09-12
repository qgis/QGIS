/**
 * Holds a set of configuration parameters for a editor widget wrapper.
 * It's basically a set of key => value pairs.
 *
 * If you need more advanced structures than a simple key => value pair,
 * you can use a value to hold any structure a QVariant can handle (and that's
 * about anything you get through your compiler)
 *
 * These are the user configurable options in the field properties tab of the
 * vector layer properties. They are saved in the project file per layer and field.
 * You get these passed, for every new widget wrapper.
 */

typedef QMap<QString, QVariant> QgsEditorWidgetConfig;
