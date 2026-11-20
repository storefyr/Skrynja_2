import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: dialog

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: kolonne.height

        Column {
            id: kolonne
            width: parent.width
            spacing: Theme.paddingMedium

            DialogHeader {
                title: "Filændringer"
            }

            Repeater {
                model: acp.diffs
                delegate: Column {
                    width: parent.width
                    Label {
                        x: Theme.horizontalPageMargin
                        text: modelData.sti
                        font.bold: true
                    }
                    Label {
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        wrapMode: Text.Wrap
                        font.family: "monospace"
                        text: modelData.tekst
                    }
                }
            }

            Repeater {
                model: acp.outputs
                delegate: Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    text: modelData
                }
            }
        }
    }
}
