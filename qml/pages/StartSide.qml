import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: side

    SilicaFlickable {
        id: contents
        anchors.fill: parent
        contentHeight: kolonne.height

        Column {
            id: kolonne
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: "Skrynja 2"
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                text: acp.hadRestoredHistory ? "Samtale genoptaget" : "Ny samtale"
            }

            TextField {
                id: promptFelt
                width: parent.width
                placeholderText: "Skriv en besked til agenten"
                label: "Prompt"
                EnterKey.enabled: text.length > 0
                EnterKey.onClicked: acp.sendSessionPrompt(text)
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingMedium

                Button {
                    text: "Send"
                    enabled: promptFelt.text.length > 0
                    onClicked: acp.sendSessionPrompt(promptFelt.text)
                }
                Button {
                    text: "Sk\u00e6rmbillede"
                    enabled: promptFelt.text.length > 0
                    onClicked: side.grabToImage(function(resultat) {
                        var sti = "/tmp/skrynja-2-skaermbillede.png"
                        resultat.saveToFile(sti)
                        acp.sendSessionPromptMedBilledeFraFil(promptFelt.text, sti)
                    })
                }
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingMedium

                Button {
                    text: "Konfiguration"
                    onClicked: pageStack.push(Qt.resolvedUrl("KonfigurationsSide.qml"))
                }
                Button {
                    text: "Diff"
                    onClicked: pageStack.push(Qt.resolvedUrl("DiffVisning.qml"))
                }
                Button {
                    text: "Stop runde"
                    onClicked: acp.sessionCancel()
                }
                Button {
                    text: "Stop agent"
                    onClicked: acp.stopAgent()
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                text: "Status: " + acp.conversationStatus + " (runde " + acp.conversationRound + ")"
            }

            Repeater {
                model: acp.toolCalls
                delegate: Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    text: modelData.title + " — " + modelData.status
                    color: modelData.set ? Theme.secondaryColor : Theme.primaryColor
                    MouseArea {
                        anchors.fill: parent
                        onClicked: acp.acknowledgeToolCall(modelData.toolCallId)
                    }
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                text: acp.conversationText
            }
        }
    }
}
