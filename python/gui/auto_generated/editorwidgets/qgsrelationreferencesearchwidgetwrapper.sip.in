/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsrelationreferencesearchwidgetwrapper.h      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRelationReferenceSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%Docstring
Wraps a relation reference search widget.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsrelationreferencesearchwidgetwrapper.h"
%End
  public:

    explicit QgsRelationReferenceSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QgsMapCanvas *canvas, QWidget *parent = 0 );
%Docstring
Constructor for QgsRelationReferenceSearchWidgetWrapper

:param vl: associated vector layer
:param fieldIdx: associated field index
:param canvas: optional map canvas
:param parent: parent widget
%End

    QVariant value() const;
%Docstring
Returns a variant representing the current state of the widget.
%End

    virtual bool applyDirectly();

    virtual QString expression() const;

    virtual bool valid() const;

    virtual QgsSearchWidgetWrapper::FilterFlags supportedFlags() const;

    virtual QString createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const;


  public slots:

    virtual void clearWidget();

    virtual void setEnabled( bool enabled );


  protected:
    virtual QWidget *createWidget( QWidget *parent );

    virtual void initWidget( QWidget *editor );


  public slots:

    void onValueChanged( const QVariant &value );
%Docstring
Called when current value of search widget changes
%End

  protected slots:
    virtual void setExpression( const QString &exp );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsrelationreferencesearchwidgetwrapper.h      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
