// Dovetail — QML shell. Slice-1: a window with the project name.
// The real timeline view grows here slice by slice.
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1280
    height: 720
    visible: true
    title: qsTr("Dovetail")

    Label {
        anchors.centerIn: parent
        text: qsTr("Dovetail — cut the cord, keep the muscle memory")
    }
}