import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Welcome to QGIS 3")

    Theme {
        id: theme
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        WelcomePage {

        }

        OptionPage {

        }


        WhatsNew {

        }

        Page {
        }
    }

    PageIndicator {
            id: indicator

            count: swipeView.count
            currentIndex: swipeView.currentIndex

            anchors.bottom: swipeView.bottom
            anchors.horizontalCenter: parent.horizontalCenter
    }


    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
        TabButton {
            text: qsTr("Welcome")
        }
        TabButton {
            text: qsTr("Getting Started")
        }
        TabButton {
            text: qsTr("What's New")
        }
        TabButton {
            text: qsTr("Lets do this!")
        }
    }
}
