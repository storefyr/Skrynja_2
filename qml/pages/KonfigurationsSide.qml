import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: side

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: kolonne.height

        Column {
            id: kolonne
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: "Agent-konfiguration"
            }

            TextField {
                id: stiFelt
                width: parent.width
                label: "Agentsti"
                text: acp.agentPath
                placeholderText: "/home/defaultuser/.local/bin/vibe-acp"
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Gem agentsti"
                onClicked: acp.setAgentsti(stiFelt.text)
            }

            Timer {
                id: configDebounce
                interval: 300
                repeat: false
                property string configId: ""
                property string vaerdi: ""
                onTriggered: acp.setConfigOption(configId, vaerdi)
            }

            Repeater {
                model: acp.configOptions
                delegate: Column {
                    width: parent.width
                    Label {
                        x: Theme.horizontalPageMargin
                        text: modelData.name ? modelData.name : modelData.configId
                        font.bold: true
                    }
                    TextField {
                        width: parent.width
                        label: modelData.configId
                        text: String(modelData.value)
                        onTextChanged: {
                            configDebounce.configId = modelData.configId
                            configDebounce.vaerdi = text
                            configDebounce.restart()
                        }
                    }
                }
            }
        }
    }
}
