import QtQuick 2.7

Item {
    id: item1
    width: 600
    height: 400
    property alias text1: text1

    Text {
        id: text1
        x: 71
        color: "#589632"
        text: qsTr("Welcome to QGIS 3")
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 41
        anchors.top: image1.bottom
        anchors.topMargin: 6
        anchors.horizontalCenterOffset: 6
        anchors.horizontalCenter: parent.horizontalCenter
        font.family: "Tahoma"
        font.bold: true
        clip: false
        font.pixelSize: 30
    }

    Image {
        id: image1
        x: -56
        y: -76
        width: 713
        anchors.verticalCenterOffset: -56
        anchors.horizontalCenterOffset: 7
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        source: "qgis-logo.svg"
    }
}
