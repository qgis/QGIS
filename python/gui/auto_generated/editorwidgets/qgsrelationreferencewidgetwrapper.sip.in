/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsrelationreferencewidgetwrapper.h            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRelationReferenceWidgetWrapper : QgsEditorWidgetWrapper
{
%Docstring
Wraps a relation reference widget.

Options:
<ul>
<li><b>ShowForm</b> <i>If ``True``, an embedded form with the referenced feature will be shown.</i></li>
<li><b>MapIdentification</b> <i>Will offer a map tool to pick a referenced feature on the map canvas. Only use for layers with geometry.</i></li>
<li><b>ReadOnly</b> <i>If ``True``, will represent the referenced widget in a read-only line edit. Can speed up loading.</i></li>
<li><b>AllowNULL</b> <i>Will offer NULL as a value.</i></li>
<li><b>Relation</b> <i>The ID of the relation that will be used to define this widget.</i></li>
ReadOnly
</ul>
%End

%TypeHeaderCode
#include "qgsrelationreferencewidgetwrapper.h"
%End
  public:

    explicit QgsRelationReferenceWidgetWrapper( QgsVectorLayer *vl,
        int fieldIdx,
        QWidget *editor,
        QgsMapCanvas *canvas,
        QgsMessageBar *messageBar,
        QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsRelationReferenceWidgetWrapper
%End

    virtual QWidget *createWidget( QWidget *parent );

    virtual void initWidget( QWidget *editor );

    virtual QVariant value() const;

    virtual bool valid() const;

    virtual void showIndeterminateState();


  public slots:
    virtual void setValue( const QVariant &value );

    virtual void setEnabled( bool enabled );


  protected:

    virtual void updateConstraintWidgetStatus();


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsrelationreferencewidgetwrapper.h            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
