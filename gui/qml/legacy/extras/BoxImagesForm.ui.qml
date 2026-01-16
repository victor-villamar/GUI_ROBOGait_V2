import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: boxImageForm
    width: 300
    height: 200
    property alias imageText_bif_check_black: imageText_bif_check_black

    property alias bif_text_clear: imageText_bif_clear.text_label
    property alias bif_text_draw_path: imageText_bif_draw_path.text_label
    property alias bif_text_edit: imageText_bif_edit.text_label
    property alias bif_text_check: imageText_bif_check.text_label
    property alias bif_text_check_black: imageText_bif_check_black.text_label

    property alias bif_clear: imageText_bif_clear.button_id
    property alias bif_draw_path: imageText_bif_draw_path.button_id
    property alias bif_edit: imageText_bif_edit.button_id
    property alias bif_check: imageText_bif_check.button_id
    property alias bif_check_black: imageText_bif_check_black.button_id

    property string image_source_clear
    property string image_source_draw_path
    property string image_source_edit
    property string image_source_check
    property string image_source_check_black

    ImageText {
        id: imageText_bif_clear
        width: 120
        height: 100
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: imageText_bif_edit.right
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.leftMargin: 10
        anchors.verticalCenterOffset: -50
        anchors.horizontalCenter: parent.horizontalCenter
        // button_id: bif_clear
        // text_label: bif_text_clear
        image_source: image_source_clear
    }

    ImageText {
        id: imageText_bif_edit
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.horizontalCenterOffset: -90
        anchors.topMargin: 0
        anchors.leftMargin: 0
        //        anchors.horizontalCenterOffset: -126
        anchors.verticalCenterOffset: -40
        anchors.horizontalCenter: parent.horizontalCenter
        // button_id: bif_edit
        // text_label: bif_text_edit
        image_source: image_source_edit
    }

    ImageText {
        id: imageText_bif_draw_path
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 0
        anchors.topMargin: 0
        anchors.verticalCenterOffset: -50
        anchors.horizontalCenterOffset: -90
        anchors.horizontalCenter: parent.horizontalCenter
        // button_id: bif_draw_path
        // text_label: bif_text_draw_path
        image_source: image_source_draw_path
    }

    ImageText {
        id: imageText_bif_check
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: imageText_bif_check_black.right
        anchors.top: imageText_bif_clear.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 10
        anchors.leftMargin: 10
        // button_id: bif_check
        // text_label: bif_text_check
        image_source: image_source_check
    }

    ImageText {
        id: imageText_bif_check_black
        anchors.left: parent.left
        anchors.top: imageText_bif_edit.bottom
        anchors.leftMargin: 0
        anchors.topMargin: 10
        // button_id: bif_check_black
        // text_label: bif_text_check_black
        image_source: image_source_check_black
    }
}



