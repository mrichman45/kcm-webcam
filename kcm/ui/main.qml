import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import QtMultimedia

KCM.SimpleKCM {
    id: root

    // Pick the Qt camera that matches the camera selected on this page.
    function matchingDevice() {
        const inputs = mediaDevices.videoInputs;
        const name = kcm.cameraNames[kcm.currentCamera];
        for (let i = 0; i < inputs.length; ++i) {
            if (inputs[i].description === name) {
                return inputs[i];
            }
        }
        return mediaDevices.defaultVideoInput;
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        MediaDevices {
            id: mediaDevices
        }

        CaptureSession {
            camera: Camera {
                id: camera
                cameraDevice: root.matchingDevice()
                active: previewSwitch.checked && kcm.cameraNames.length > 0 && root.visible
                onActiveChanged: if (active) previewError.text = ""
                onErrorOccurred: (error, errorString) => {
                    previewError.text = errorString;
                }
            }
            videoOutput: videoOutput
        }

        QQC2.Switch {
            id: previewSwitch
            Layout.alignment: Qt.AlignHCenter
            visible: kcm.cameraNames.length > 0
            text: i18n("Show preview")
            checked: true
        }

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(root.width - Kirigami.Units.gridUnit * 2, Kirigami.Units.gridUnit * 30)
            Layout.preferredHeight: Layout.preferredWidth * 9 / 16
            visible: previewSwitch.visible && previewSwitch.checked
            color: "black"
            radius: Kirigami.Units.smallSpacing

            VideoOutput {
                id: videoOutput
                anchors.fill: parent
                fillMode: VideoOutput.PreserveAspectFit
            }

            QQC2.Label {
                id: previewError
                anchors.centerIn: parent
                width: parent.width - Kirigami.Units.gridUnit * 2
                visible: text !== ""
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                color: "white"
                text: ""
            }
        }

        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter
            visible: previewError.text !== ""
            text: i18n("If another app is using the camera, close it and turn the preview off and on.")
            opacity: 0.7
        }

        Kirigami.PlaceholderMessage {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.gridUnit * 4
            visible: kcm.cameraNames.length === 0
            icon.name: "camera-web"
            text: i18n("No webcam found")
            explanation: i18n("If your laptop has a camera switch, slide it to the on position. This page updates by itself when the camera appears.")
        }

        ColumnLayout {
            id: controlsColumn
            Layout.alignment: Qt.AlignHCenter
            visible: kcm.cameraNames.length > 0
            spacing: Kirigami.Units.smallSpacing

            readonly property real labelWidth: Kirigami.Units.gridUnit * 13

            RowLayout {
                visible: kcm.cameraNames.length > 1
                spacing: Kirigami.Units.largeSpacing
                QQC2.Label {
                    Layout.preferredWidth: controlsColumn.labelWidth
                    horizontalAlignment: Text.AlignRight
                    text: i18n("Camera:")
                }
                QQC2.ComboBox {
                    model: kcm.cameraNames
                    currentIndex: kcm.currentCamera
                    onActivated: kcm.currentCamera = currentIndex
                }
            }

            Repeater {
                model: kcm.controls

                delegate: RowLayout {
                    id: row
                    required property var modelData
                    spacing: Kirigami.Units.largeSpacing

                    QQC2.Label {
                        Layout.preferredWidth: controlsColumn.labelWidth
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                        text: row.modelData.name + ":"
                        enabled: !row.modelData.inactive
                    }

                    QQC2.Slider {
                        id: slider
                        visible: row.modelData.type === "int"
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 14
                        from: row.modelData.min
                        to: row.modelData.max
                        stepSize: row.modelData.step
                        value: row.modelData.value
                        enabled: !row.modelData.inactive
                        onMoved: kcm.setControl(row.modelData.id, Math.round(value))
                    }
                    QQC2.Label {
                        visible: slider.visible
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 3
                        text: Math.round(slider.value)
                        enabled: slider.enabled
                    }

                    QQC2.CheckBox {
                        visible: row.modelData.type === "bool"
                        checked: row.modelData.value !== 0
                        enabled: !row.modelData.inactive
                        onToggled: kcm.setControl(row.modelData.id, checked ? 1 : 0)
                    }

                    QQC2.ComboBox {
                        visible: row.modelData.type === "menu"
                        model: row.modelData.options
                        textRole: "text"
                        valueRole: "value"
                        enabled: !row.modelData.inactive
                        Component.onCompleted: currentIndex = indexOfValue(row.modelData.value)
                        onActivated: kcm.setControl(row.modelData.id, currentValue)
                    }

                    QQC2.Label {
                        visible: row.modelData.inactive
                        text: i18n("(set automatically)")
                        opacity: 0.7
                    }
                }
            }
        }
    }
}
