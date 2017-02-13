class QgsPropertyAssistantWidget : public QgsPanelWidget
{
%TypeHeaderCode
  #include <qgspropertyassistantwidget.h>
%End

  public:

    QgsPropertyAssistantWidget( QWidget* parent /TransferThis/ = 0, const QgsPropertyDefinition& definition = QgsPropertyDefinition(),
                                const QgsProperty& initialState = QgsProperty(),
                                const QgsVectorLayer* layer = 0 );

    void registerExpressionContextGenerator( QgsExpressionContextGenerator* generator );

    void updateProperty( QgsProperty& property );

    void setDockMode( bool dockMode );
};
