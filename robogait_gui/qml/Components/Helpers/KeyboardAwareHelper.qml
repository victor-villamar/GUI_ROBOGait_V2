import QtQuick 2.15

QtObject {
    id: helper

    property var target

    property real maxDialogHeight: 0
    property int margin: 20

    property real contentHeight: 0
    property int baseCenterOffset: 0
    property int topPadding: 20
    property int keyboardPadding: 12

    readonly property real keyboardHeight: {
        if (Qt.inputMethod.visible && Qt.inputMethod.keyboardRectangle.height > 0) {
            return Qt.inputMethod.keyboardRectangle.height
        }
        var win = Qt.application.activeWindow
        return win && win.keyboardVisible ? win.keyboardHeight : 0
    }

    readonly property real windowHeight: {
        if (target && target.parent && target.parent.height > 0) {
            return target.parent.height
        }
        var win = Qt.application.activeWindow
        return win ? win.height : 0
    }
    readonly property real containerHeight: target ? target.height : 0

    readonly property real availableHeight: windowHeight > 0
        ? Math.max(0, windowHeight - keyboardHeight - margin)
        : 0

    readonly property real computedHeight: {
        var maxH = maxDialogHeight > 0 ? maxDialogHeight : availableHeight
        if (availableHeight > 0) {
            return Math.min(maxH, availableHeight)
        }
        return maxH > 0 ? maxH : 0
    }

    readonly property real computedY: {
        var h = windowHeight
        if (h <= 0) {
            return 0
        }
        var dialogH = target && target.height > 0 ? target.height : computedHeight
        var base = (h - dialogH) / 2
        if (keyboardHeight > 0) {
            var safeBottom = h - keyboardHeight - keyboardPadding
            var bottom = base + dialogH
            if (bottom > safeBottom) {
                base -= (bottom - safeBottom)
            }
        }
        if (base < topPadding) {
            base = topPadding
        }
        return Math.round(base)
    }

    readonly property int contentCenterOffset: {
        var h = containerHeight
        var contentH = contentHeight
        var base = baseCenterOffset
        if (h <= 0 || contentH <= 0) {
            return base
        }
        var offset = base
        if (keyboardHeight > 0) {
            var safeBottom = h - keyboardHeight - keyboardPadding
            var centerY = (h / 2) + offset
            var contentBottom = centerY + (contentH / 2)
            if (contentBottom > safeBottom) {
                offset -= (contentBottom - safeBottom)
            }
        }
        var minOffset = (-h / 2) + (contentH / 2) + topPadding
        if (offset < minOffset) {
            offset = minOffset
        }
        return Math.round(offset)
    }
}
