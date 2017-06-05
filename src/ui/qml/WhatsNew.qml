import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtWebKit 3.0

Item {
    Text {
        id: headerText
        text: qsTr("What's New!")
        font: theme.headerFont
        color: theme.headerColor

        anchors.horizontalCenter: parent.horizontalCenter
    }
}
