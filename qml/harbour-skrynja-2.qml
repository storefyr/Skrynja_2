import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"

ApplicationWindow {
    id: vindue
    initialPage: Component { StartSide {} }

    Connections {
        target: acp
        onAgentError: console.warn("agentfejl: " + error)
        onPermissionRequested: pageStack.push(Qt.resolvedUrl("pages/TilladelsesDialog.qml"))
    }
}
