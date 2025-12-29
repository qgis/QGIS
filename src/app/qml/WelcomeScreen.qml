import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Controls.Material

import "components"

Item {
  id: welcomeScreen

  width: 1100
  height: 720

  Material.accent: "#93b023"
  Material.foreground: "#ffffff"

  Rectangle {
    id: mainCard
    anchors.top: parent.top
    anchors.topMargin: 30
    anchors.left: parent.left
    anchors.leftMargin: 30
    width: parent.width - 60
    height: parent.height - (updateBar.visible ? updateBar.height - 10 : 0) - 80
    radius: 10
    color: "#252a34"
    clip: true

    layer.enabled: true
    layer.effect: MultiEffect {
      shadowEnabled: true
      shadowColor: "#80000000"
      shadowBlur: 1.0
      shadowVerticalOffset: 8
      shadowHorizontalOffset: 0
    }

    RowLayout {
      anchors {
        top: parent.top
        bottom: footer.top
        left: parent.left
        right: parent.right
        bottomMargin: 8
      }
      spacing: 0

      ColumnLayout {
        Layout.maximumWidth: parent.width * 0.48
        Layout.fillHeight: true
        Layout.topMargin: 28
        Layout.leftMargin: 28
        Layout.rightMargin: 28
        spacing: 10

        Column {
          Layout.fillWidth: true
          spacing: 16

          Image {
            source: "images/qgis.svg"
            height: 50
            width: 160
            sourceSize: Qt.size(50, 160)
            fillMode: Image.PreserveAspectFit
          }

          Text {
            text: qsTr("Spatial without Compromise")
            font.pointSize: 12
            font.bold: true

            color: "#e0e9ed"
          }
        }

        Rectangle {
          Layout.fillWidth: true
          Layout.preferredHeight: 1
          color: "#566775"
        }

        Column {
          Layout.fillWidth: true
          Layout.fillHeight: true
          spacing: 12

          RowLayout {
            width: parent.width
            spacing: 12

            Text {
              text: newsSwitch.checked ? qsTr("Latest news") : qsTr("Welcome to QGIS!")
              font.pointSize: 16
              font.bold: true
              color: "#ffffff"
              Layout.fillWidth: true
            }
            
            BusyIndicator {
              width: 22
              height: 22
              running: newsFeedParser.isFetching
            }

            Rectangle {
              id: newsSwitchBackground
              width: 70
              height: 28
              radius: 14

              gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                  position: 0.0
                  color: newsSwitch.checked ? "#589632" : "#333333"
                }
                GradientStop {
                  position: 1.0
                  color: newsSwitch.checked ? "#93b023" : "#ffffff"
                }
              }

              Text {
                x: newsSwitch.checked ? 10 : 30
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("News")
                font.pointSize: 8
                font.bold: true
                color: newsSwitch.checked ? "#ffffff" : "#666666"
              }

              Rectangle {
                id: switchHandle
                width: 22
                height: 22
                radius: 11
                color: "#ffffff"
                anchors.verticalCenter: parent.verticalCenter
                x: newsSwitch.checked ? parent.width - width - 3 : 3

                Behavior on x {
                  NumberAnimation {
                    duration: 150
                  }
                }
              }

              MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                  newsFeedParser.enabled = !newsFeedParser.enabled
                  if (newsFeedParser.enabled && newsListView.count == 0) {
                    newsFeedParser.fetch();
                  }
                }
              }

              Switch {
                id: newsSwitch
                visible: false
                checked: newsFeedParser.enabled
              }
            }
          }

          Column {
            width: parent.width
            height: parent.height - 50
            spacing: 12
            visible: !newsSwitch.checked

            Rectangle {
              width: parent.width
              height: welcomeDescription1.implicitHeight * 1.25
              radius: 10
              color: "#ffffff"

              Text {
                id: welcomeDescription1
                anchors.fill: parent
                anchors.margins: 16
                text: qsTr("The free and open-source geographic information system that empowers users worldwide to create, edit, visualize, analyze, and share geospatial data. Whether you're a beginner or a seasoned GIS expert, QGIS gives you the tools to turn spatial data into impactful maps and insights. Join our vibrant global community and start exploring the world through the power of open-source geospatial technology.")
                font.pointSize: 9
                color: "black"
                wrapMode: Text.WordWrap
                lineHeight: 1.3
              }
            }

            Rectangle {
              width: parent.width
              height: stayUpdateLayout.implicitHeight * 1.25
              radius: 10
              color: "#ffffff"

              ColumnLayout {
                id: stayUpdateLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 10

                Text {
                  text: qsTr("Stay up to date!")
                  font.pointSize: 11
                  font.bold: true
                  color: "black"
                }

                Text {
                  Layout.fillWidth: true
                  text: qsTr("Would you like to enable the QGIS news feed to stay updated on new features, releases, and community highlights?")
                  font.pointSize: 9
                  color: "black"
                  wrapMode: Text.WordWrap
                }

                Rectangle {
                  width: enableNewsText.implicitWidth + 24
                  height: 25
                  radius: 10
                  color: "transparent"
                  border.width: 1
                  border.color: "#93b023"

                  Text {
                    id: enableNewsText
                    anchors.centerIn: parent
                    text: qsTr("Enable news feed")
                    font.pointSize: 9
                    color: "black"
                  }

                  MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: newsSwitch.checked = true
                  }
                }
              }
            }
          }

          Item {
            width: parent.width
            height: parent.height - 50

            ListView {
              id: newsListView
              anchors.fill: parent
              spacing: 12
              clip: true
              visible: newsSwitch.checked

              model: newsFeedModel

              delegate: NewsCard {
                width: newsListView.width
                title: Title
                description: Content
                showCloseButton: true

                onReadMoreClicked: {
                  Qt.openUrlExternally(Link);
                }
              }

              ScrollBar.vertical: newsScrollBar
            }

            ScrollBar {
              id: newsScrollBar
              policy: ScrollBar.AsNeeded
              anchors.left: parent.right
              anchors.leftMargin: 10
              height: parent.height
              visible: newsSwitch.checked
            }
          }
        }
      }

      ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.topMargin: 28
        Layout.leftMargin: 28
        Layout.rightMargin: 28
        spacing: 8

        Item {
          Layout.fillWidth: true
          Layout.preferredHeight: tabBar.implicitHeight

          Rectangle {
            width: parent.width
            height: 1
            color: "#566775"
            anchors.bottom: parent.bottom
          }

          TabBar {
            id: tabBar
            background: Item {
              anchors.fill: parent
            }

            TabButton {
              text: qsTr("Recent")
              font.pointSize: 13
              font.bold: true
            }
            TabButton {
              text: qsTr("Templates")
              width: implicitWidth
              font.pointSize: 13
              font.bold: true
            }
          }
        }

        Item {
          Layout.fillWidth: true
          Layout.fillHeight: true

          ListView {
            id: recentProjectsListView
            anchors.fill: parent
            spacing: 8
            clip: true
            visible: tabBar.currentIndex === 0

            model: recentProjectsModel

            delegate: ProjectCard {
              width: recentProjectsListView.width
              title: Title || ""
              subtitle: ProjectNativePath || ProjectPath || ""
              imageSource: PreviewImagePath || ""
              isSelected: recentProjectsListView.currentIndex === index
              radius: 10

              onClicked: {
                welcomeScreenController.openProject(ProjectPath);
              }
            }

            ScrollBar.vertical: recentScrollBar
          }

          ListView {
            id: templatesListView
            anchors.fill: parent
            spacing: 8
            clip: true
            visible: tabBar.currentIndex === 1

            model: templateProjectsModel

            delegate: ProjectCard {
              width: templatesListView.width
              title: Title || ""
              subtitle: TemplateNativePath || ""
              imageSource: PreviewImagePath || ""
              isSelected: templatesListView.currentIndex === index
              radius: 10

              onClicked: {
                welcomeScreenController.createProjectFromTemplate(TemplateNativePath || "")
              }
            }

            ScrollBar.vertical: templatesScrollBar
          }

          ScrollBar {
            id: recentScrollBar
            policy: ScrollBar.AsNeeded
            anchors.left: parent.right
            anchors.leftMargin: 10
            height: parent.height
            visible: tabBar.currentIndex === 0
          }

          ScrollBar {
            id: templatesScrollBar
            policy: ScrollBar.AsNeeded
            anchors.left: parent.right
            anchors.leftMargin: 10
            height: parent.height
            visible: tabBar.currentIndex === 1
          }
        }
      }
    }

    Rectangle {
      width: parent.width
      height: 1
      color: "#566775"
      anchors {
        bottom: footer.top
        left: parent.left
        right: parent.right
        leftMargin: 28
        rightMargin: 28
      }
    }

    FooterBar {
      id: footer
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.bottomMargin: 0

      onSupportClicked: {
        Qt.openUrlExternally("https://qgis.org/funding/donate/")
      }

      onWebsiteClicked: {
        Qt.openUrlExternally("https://qgis.org")
      }
    }
  }

  UpdateNotificationBar {
    id: updateBar
    width: parent.width - 60
    height: 50
    radius: 16
    anchors.left: parent.left
    anchors.leftMargin: 30
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 30
    visible: false
    color: mainCard.color

    layer.enabled: true
    layer.effect: MultiEffect {
      shadowEnabled: true
      shadowColor: "#80000000"
      shadowBlur: 1.0
      shadowVerticalOffset: 8
      shadowHorizontalOffset: 0
    }

    onInstallClicked: {
      console.log("Install updates clicked")
    }

    onCloseClicked: {
      updateBar.visible = false
    }
  }
  
  Component.onCompleted: {
    if (newsFeedParser.enabled) {
      newsFeedParser.fetch();
    }
  }
}
