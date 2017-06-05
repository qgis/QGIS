import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

Rectangle {
    Text {
        id: headerText
        text: qsTr("Some options before we start")
        font: theme.headerFont
        color: theme.headerColor

        anchors.horizontalCenter: parent.horizontalCenter
    }

    ColumnLayout {
        anchors.top: headerText.bottom

        CheckBox {
            id: migrateCheck
            text: qsTr("Migrate QGIS 2 Settings.")
            font: theme.normalFont
            checked: true
            onCheckStateChanged: {
                settings.setValue("/migrate", checked)
            }
        }
        Text {
            id: migrateText
            text: qsTr("Cool! We will migrate your QGIS 2 settings into a new user profile for QGIS 3")
            wrapMode: Text.WordWrap
            font: theme.infoFont
            visible: migrateCheck.checked
        }
        CheckBox {
            id: improveQGIS
            text: qsTr("Help us improve QGIS by sending anonymous usage stats")
            font: theme.normalFont
        }
        Text {
            id: usageNote
            text: qsTr("We will never collect anything personal. In fact you can view all all the data with a built in data viewer in QGIS")
            wrapMode: Text.WordWrap
            font: theme.infoFont
            visible: improveQGIS.checked
            width: parent.width
        }
    }
}
