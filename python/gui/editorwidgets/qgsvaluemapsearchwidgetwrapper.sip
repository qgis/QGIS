/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsvaluemapsearchwidgetwrapper.h               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsValueMapSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%Docstring
 Wraps a value map search widget. This widget will offer a combobox with values from another layer
 referenced by a foreign key (a constraint may be set but is not required on data level).
 It will be used as a search widget and produces expression to look for in the layer.
%End

%TypeHeaderCode
#include "qgsvaluemapsearchwidgetwrapper.h"
%End
  public:
    explicit QgsValueMapSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent /TransferThis/ = 0 );
    virtual bool applyDirectly();

    virtual QString expression() const;

    virtual bool valid() const;

    virtual QgsSearchWidgetWrapper::FilterFlags supportedFlags() const;

    virtual QgsSearchWidgetWrapper::FilterFlags defaultFlags() const;

    virtual QString createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const;

  public slots:

    virtual void clearWidget();
    virtual void setEnabled( bool enabled );

  protected:
    virtual QWidget *createWidget( QWidget *parent );

    virtual void initWidget( QWidget *editor );


  protected slots:
    virtual void setExpression( const QString &exp );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsvaluemapsearchwidgetwrapper.h               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
