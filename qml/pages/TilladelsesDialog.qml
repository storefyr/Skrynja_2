import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: dialog
    property var anmodning: acp.pendingPermission

    canAccept: false

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: kolonne.height

        Column {
            id: kolonne
            width: parent.width
            spacing: Theme.paddingMedium

            DialogHeader {
                title: "Tilladelse nødvendig"
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                text: anmodning && anmodning.toolCall ? anmodning.toolCall.title : ""
            }

            Repeater {
                model: anmodning && anmodning.options ? anmodning.options : []
                delegate: Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: modelData.label
                    onClicked: {
                        acp.answerPermission(modelData.optionId)
                        dialog.accept()
                    }
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Afvis"
                onClicked: {
                    acp.afvisPermission()
                    dialog.accept()
                }
            }
        }
    }
}
