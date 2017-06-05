import QtQuick 2.0

QtObject {
        property color headerColor: "#589632"
        property color minorHeaderColor: "#93b023"
        property color importantColor: "#ee7913"

        property font headerFont: Qt.font({
            family: 'Encode Sans',
            weight: Font.Black,
            italic: false,
            pointSize: 20,
        })

        property font normalFont: Qt.font({
            family: 'Encode Sans',
            pointSize: 12,
        })
        property font infoFont: Qt.font({
            family: 'Encode Sans',
            italic: true,
            pointSize: 12,
        })

}
