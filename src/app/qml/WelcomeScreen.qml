import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Controls.Material

import org.qgis.app

import "components"

Item {
  id: welcomeScreen

  property bool narrowLayout: height < 350 || width < 480

  visible: height >= 300 && width >= 360
  width: 1100
  height: 720

  Material.accent: "#93b023"
  Material.foreground: "#ffffff"

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 20

    Rectangle {
      id: mainCard
      Layout.fillWidth: true
      Layout.fillHeight: true
      radius: 10
      color: "#002033"
      clip: true

      GridLayout {
        anchors {
          top: parent.top
          left: parent.left
          right: parent.right
          bottom: footer.top
          topMargin: 28
          leftMargin: 28
          rightMargin: 28
          bottomMargin: 8
        }
        columns: welcomeScreen.narrowLayout ? 1 : 2
        columnSpacing: 20
        rowSpacing: 10

        Column {
          Layout.maximumWidth: welcomeScreen.narrowLayout ? parent.width : parent.width * 0.52
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignTop
          spacing: 10

          Image {
            source: "images/qgis.svg"
            height: welcomeScreen.narrowLayout ? 35 : 50
            width: 160
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignLeft
            smooth: true
            mipmap: true
          }

          Text {
            width: parent.width
            text: qsTr("Spatial without Compromise")
            font.pointSize: Application.font.pointSize
            font.bold: true
            wrapMode: Text.WordWrap

            color: "#e0e9ed"
          }
          
          Rectangle {
            width: parent.width
            height: 1
            color: "#566775"
          }
        }


        ColumnLayout {
          Layout.fillWidth: true
          Layout.fillHeight: true
          Layout.rowSpan: 2
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
                font.pointSize: Application.font.pointSize * 1.1
                font.bold: true
                visible: recentProjectsListView.count > 0
                width: recentProjectsListView.count > 0 ? implicitWidth : 0
                background: null
              }
              TabButton {
                text: qsTr("Templates")
                width: implicitWidth
                font.pointSize: Application.font.pointSize * 1.1
                font.bold: true
                background: null
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
                width: recentProjectsListView.width - 12
                title: Title || ""
                // Add invisible spaces after slash and backslash characters to help wrapping paths
                subtitle: (ProjectNativePath || ProjectPath || "").replace(/([\\\/])/g,"$1\u200b")
                crs: Crs
                imageSource: PreviewImagePath || ""
                isEnabled: Exists
                isPinned: Pinned
                isSelected: recentProjectsListView.currentIndex === index
                radius: 6

                onClicked: (mouse) => {
                             if (mouse.button == Qt.LeftButton && isEnabled) {
                               welcomeScreenController.openProject(ProjectPath);
                             } else if (mouse.button == Qt.RightButton) {
                               recentProjectsMenu.projectIndex = index;
                               recentProjectsMenu.projectPinned = Pinned;
                               recentProjectsMenu.projectExists = Exists;
                               recentProjectsMenu.projectHasNativePath = ProjectNativePath != "";
                               
                               const point = mapToItem(recentProjectsListView, mouse.x, mouse.y);
                               recentProjectsMenu.popup(point.x, point.y);
                             }
                           }
              }

              ScrollBar.vertical: CustomScrollBar {
                policy: ScrollBar.AsNeeded
              }
              
              onCountChanged: {
                if (count == 0 && tabBar.currentIndex == 0) {
                  tabBar.currentIndex = 1;
                }
              }

              Menu {
                id: templateProjectsMenu
                
                property int projectIndex: 0
                
                background: Rectangle {
                  implicitWidth: 200
                  implicitHeight: templateProjectsMenu.Material.menuItemHeight
                  radius: templateProjectsMenu.Material.roundedScale
                  color: templateProjectsMenu.Material.dialogColor
                  border.color: templateProjectsMenu.Material.listHighlightColor
                  border.width: 1
                }
                
                MenuItem {
                  text: qsTr("Delete Template…")
                  onClicked: {
                    welcomeScreenController.removeTemplateProject(templateProjectsMenu.projectIndex)
                  }
                  background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: parent.Material.menuItemHeight
                    color: parent.highlighted ? parent.Material.listHighlightColor : "transparent"
                  }
                }
              }

              Menu {
                id: recentProjectsMenu

                property int projectIndex: 0
                property bool projectPinned: false
                property bool projectExists: false
                property bool projectHasNativePath: false

                background: Rectangle {
                  implicitWidth: 200
                  implicitHeight: templateProjectsMenu.Material.menuItemHeight
                  radius: templateProjectsMenu.Material.roundedScale
                  color: templateProjectsMenu.Material.dialogColor
                  border.color: templateProjectsMenu.Material.listHighlightColor
                  border.width: 1
                }
                
                MenuItem {
                  text: recentProjectsMenu.projectPinned? qsTr("Unpin from List") : qsTr("Pin to List")
                  onClicked: {
                    if (recentProjectsMenu.projectPinned) {
                      recentProjectsModel.unpinProject(recentProjectsMenu.projectIndex);
                    } else {
                      recentProjectsModel.pinProject(recentProjectsMenu.projectIndex);
                    }
                  }
                  background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: parent.Material.menuItemHeight
                    color: parent.highlighted ? parent.Material.listHighlightColor : "transparent"
                  }
                }
                MenuItem {
                  text: qsTr("Refresh")
                  enabled: !recentProjectsMenu.projectExists
                  visible: enabled
                  height: enabled ? implicitHeight : 0
                  onClicked: {
                    recentProjectsModel.recheckProject(recentProjectsMenu.projectIndex);
                  }
                  background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: parent.Material.menuItemHeight
                    color: parent.highlighted ? parent.Material.listHighlightColor : "transparent"
                  }
                }
                MenuItem {
                  text: qsTr("Open Directory…")
                  enabled: recentProjectsMenu.projectExists && recentProjectsMenu.projectHasNativePath
                  visible: enabled
                  height: enabled ? implicitHeight : 0
                  onClicked: {
                    recentProjectsModel.openProject(recentProjectsMenu.projectIndex);
                  }
                  background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: parent.Material.menuItemHeight
                    color: parent.highlighted ? parent.Material.listHighlightColor : "transparent"
                  }
                }
                MenuItem {
                  text: qsTr("Remove from List")
                  onClicked: {
                    recentProjectsModel.removeProject(recentProjectsMenu.projectIndex);
                  }
                  background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: parent.Material.menuItemHeight
                    color: parent.highlighted ? parent.Material.listHighlightColor : "transparent"
                  }
                }
                MenuSeparator {}
                MenuItem {
                  text: qsTr("Clear List")
                  onClicked: {
                    welcomeScreenController.clearRecentProjects();
                  }
                  background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: parent.Material.menuItemHeight
                    color: parent.highlighted ? parent.Material.listHighlightColor : "transparent"
                  }
                }
              }
            }

            ListView {
              id: templatesListView
              anchors.fill: parent
              spacing: 8
              clip: true
              visible: tabBar.currentIndex === 1

              model: templateProjectsModel

              delegate: ProjectCard {
                width: templatesListView.width - 12
                title: Title || ""
                subtitle: Crs || ""
                imageSource: {
                  switch (Type) {
                  case TemplateProjectsModel.TemplateType.Blank:
                    return "../images/blank.jpg";
                  case TemplateProjectsModel.TemplateType.Basemap:
                    return "../images/basemap.jpg";
                  default:
                    return PreviewImagePath || "";
                  }
                }
                isSelected: templatesListView.currentIndex === index
                radius: 6

                onClicked: (mouse) => {
                  if (mouse.button == Qt.LeftButton) {
                    switch (Type) {
                    case TemplateProjectsModel.TemplateType.Blank:
                      welcomeScreenController.createBlankProject(); //#spellok
                      return;
                    case TemplateProjectsModel.TemplateType.Basemap:
                      welcomeScreenController.createProjectFromBasemap();
                      return;
                    default:
                      welcomeScreenController.createProjectFromTemplate(TemplateNativePath || ""); //#spellok
                      return;
                    }
                  } else if (mouse.button == Qt.RightButton) {
                    if (Type == TemplateProjectsModel.TemplateType.File && Writable) {
                      templateProjectsMenu.projectIndex = index;
                      
                      const point = mapToItem(templatesListView, mouse.x, mouse.y);
                      templateProjectsMenu.popup(point.x, point.y);
                    }
                  }
                }
              }

              ScrollBar.vertical: CustomScrollBar {
                policy: ScrollBar.AsNeeded
              }
            }
          }
        }

        ColumnLayout {
          id: welcomeNewsLayout
          Layout.maximumWidth: parent.width * 0.52
          Layout.fillWidth: true
          Layout.fillHeight: true
          spacing: 12
          visible: !welcomeScreen.narrowLayout

          RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
              Layout.fillWidth: true
              text: newsSwitch.checked && newsListView.count != 0 ? qsTr("Latest news") : qsTr("Welcome to QGIS!")
              font.pointSize: Application.font.pointSize * 1.3
              font.bold: true
              color: "#ffffff"
              elide: Text.ElideRight
            }

            BusyIndicator {
              Layout.preferredWidth: 28
              Layout.preferredHeight: 28
              running: newsFeedParser.isFetching
            }

            Rectangle {
              id: newsSwitchBackground
              Layout.rightMargin: 12
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
                font.pointSize: Application.font.pointSize * 0.8
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
                  if (newsFeedParser.enabled) {
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

          ScrollView {
            id: welcomeView
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !newsSwitch.checked || newsListView.count == 0
            contentWidth: welcomeLayout.width
            contentHeight: welcomeLayout.height
            rightPadding: 12
            clip: true

            ScrollBar.vertical: CustomScrollBar {
              policy: ScrollBar.AsNeeded
              anchors.top: parent.top
              anchors.bottom: parent.bottom
              anchors.right: parent.right
            }
            
            ColumnLayout {
              id: welcomeLayout
              width: welcomeNewsLayout.width - welcomeView.rightPadding
              spacing: 12

              Rectangle {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: welcomeDescription.contentHeight + 32
                radius: 6
                color: "#ffffff"

                Text {
                  id: welcomeDescription
                  anchors.fill: parent
                  anchors.margins: 16
                  text: qsTr("The free and open-source geographic information system that empowers users worldwide to create, edit, visualize, analyze, and share geospatial data. Whether you're a beginner or a seasoned GIS expert, QGIS gives you the tools to turn spatial data into impactful maps and insights. Join our vibrant global community and start exploring the world through the power of open-source geospatial technology.")
                  font.pointSize: Application.font.pointSize * 0.8
                  color: "black"
                  wrapMode: Text.WordWrap
                  lineHeight: 1.3
                }
              }

              Rectangle {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: stayUpdateLayout.childrenRect.height + 32
                radius: 6
                color: "#ffffff"
                visible: !newsSwitch.checked

                ColumnLayout {
                  id: stayUpdateLayout
                  anchors.fill: parent
                  anchors.margins: 16
                  spacing: 10

                  Text {
                    text: qsTr("Stay up to date!")
                    font.pointSize: Application.font.pointSize
                    font.bold: true
                    color: "black"
                  }

                  Text {
                    Layout.fillWidth: true
                    text: qsTr("Would you like to enable the QGIS news feed to stay updated on new features, releases, and community highlights?")
                    font.pointSize: Application.font.pointSize * 0.8
                    color: "black"
                    wrapMode: Text.WordWrap
                  }

                  Rectangle {
                    width: enableNewsText.implicitWidth + 24
                    height: 24
                    radius: 12
                    color: "transparent"
                    border.width: 1
                    border.color: "#93b023"

                    Text {
                      id: enableNewsText
                      anchors.centerIn: parent
                      text: qsTr("Enable news feed")
                      font.pointSize: Application.font.pointSize * 0.8
                      color: "black"
                    }

                    MouseArea {
                      anchors.fill: parent
                      cursorShape: Qt.PointingHandCursor
                      hoverEnabled: true
                      onClicked: {
                        newsFeedParser.enabled = true;
                        newsFeedParser.fetch();
                      }
                    }
                  }
                }
              }
            }
          }

          ListView {
            id: newsListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
            clip: true
            visible: newsSwitch.checked

            model: newsFeedModel

            delegate: NewsCard {
              radius: 6
              width: newsListView.width - 12
              title: Title
              description: Content
              imageSource: ImageUrl
              showCloseButton: true

              onReadMoreClicked: {
                Qt.openUrlExternally(Link);
              }
              
              onCloseClicked: {
                newsFeedParser.dismissEntry(Key);
              }
            }

            ScrollBar.vertical: CustomScrollBar {
              policy: ScrollBar.AsNeeded
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
        anchors {
          left: parent.left
          leftMargin: 28
          right: parent.right
          rightMargin: 28
          bottom: parent.bottom
          bottomMargin: 0
        }

        onSupportClicked: {
          Qt.openUrlExternally("https://www.qgis.org/funding/donate/")
        }

        onWebsiteClicked: {
          Qt.openUrlExternally("https://www.qgis.org/")
        }
      }
      
      RoundButton {
        anchors{
          top: parent.top
          right: parent.right
          margins: 5
        }
      
        flat: true
        icon.source: "images/close.svg"
        icon.color: "white"
        icon.width: 20
        icon.height: 20
        onClicked: {
          welcomeScreenController.hideScene();
        }
      }
    }

    UpdateNotificationBar {
      id: pluginsUpdateBar
      Layout.fillWidth: true
      Layout.preferredHeight: 50
      radius: 16
      visible: false
      color: mainCard.color

      onInstallClicked: {
        welcomeScreenController.showPluginManager();
      }
    }

    UpdateNotificationBar {
      id: qgisUpdateBar
      Layout.fillWidth: true
      Layout.preferredHeight: 50
      radius: 16
      visible: false
      color: mainCard.color

      onInstallClicked: {
        Qt.openUrlExternally("https://download.qgis.org/")
      }
    }
  }
  
  DropArea {
    anchors.fill: parent

    onDropped: drop => {
      let formatsData = {};
      for(const format of drop.formats) {
        formatsData[format] = drop.getDataAsArrayBuffer(format);
      }
      welcomeScreenController.forwardDrop(drop.text, drop.urls, formatsData);
    }
  }
  
  Connections {
    target: welcomeScreenController
    
    function onNewVersionAvailable(versionString) {
      qgisUpdateBar.message = qsTr("QGIS %1 is out!").arg(versionString);
      qgisUpdateBar.visible = true;
    }
    
    function onPluginUpdatesAvailable(plugins) {
      pluginsUpdateBar.message = qsTr("The following plugin(s) have available updates: %1", "", plugins.length).arg(plugins.join(", "));
      pluginsUpdateBar.visible = true;
    }
  }

  Component.onCompleted: {
    tabBar.currentIndex = recentProjectsListView.count > 0 ? 0 : 1
    if (newsFeedParser.enabled) {
      newsFeedParser.fetch();
    }
  }
}
