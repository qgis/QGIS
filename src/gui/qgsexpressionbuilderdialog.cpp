#include "qgsexpressionbuilderdialog.h"
#include <QSettings>

QgsExpressionBuilderDialog::QgsExpressionBuilderDialog( QgsVectorLayer* layer, QString startText, QWidget* parent)
    :QDialog( parent )
{
    setupUi( this );

    QPushButton* okButuon = buttonBox->button(QDialogButtonBox::Ok);
    connect(builder, SIGNAL(expressionParsed(bool)), okButuon, SLOT(setEnabled(bool)));

    builder->setLayer( layer );
    builder->setExpressionString(startText);
    builder->loadFieldNames();

    QSettings settings;
    restoreGeometry( settings.value( "/Windows/ExpressionBuilderDialog/geometry" ).toByteArray() );
}

QgsExpressionBuilderWidget* QgsExpressionBuilderDialog::expressionBuilder()
{
    return builder;
}

void QgsExpressionBuilderDialog::setExpressionText( QString text )
{
    builder->setExpressionString( text );
}

void QgsExpressionBuilderDialog::closeEvent( QCloseEvent *event )
{
    QDialog::closeEvent( event );

    // TODO Work out why this is not working yet.
    QSettings settings;
    settings.setValue( "/Windows/ExpressionBuilderDialog/geometry", saveGeometry() );
}
