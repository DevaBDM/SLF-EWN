pragma Singleton
import QtQuick

QtObject {
    readonly property color bgDeep: "#12161C"
    readonly property color bgSidebar: "#0E1218"
    readonly property color bgTopBar: "#161B22"
    readonly property color bgCard: "#1A1F27"
    readonly property color bgCardHover: "#1E242E"
    readonly property color bgInset: "#151A21"
    readonly property color border: "#2A303B"
    readonly property color borderSoft: "#232933"
    readonly property color safe: "#3DDC97"
    readonly property color warning: "#F0A63A"
    readonly property color critical: "#E4483C"
    readonly property color action: "#4A9EFF"
    readonly property color text: "#E6E9EF"
    readonly property color textMuted: "#8B93A1"
    readonly property color textDim: "#5C6573"
    readonly property color textFaint: "#3D4450"
    readonly property color chartTeal: "#3DDC97"
    readonly property color chartX: "#5B9BD5"
    readonly property color chartY: "#C4A35A"
    readonly property color chartZ: "#8FA6B2"
    readonly property color chartMag: "#E6E9EF"

    function statusColor(s) {
        switch (s) {
        case 1: return safe
        case 2: return warning
        case 3: return critical
        default: return textDim
        }
    }

    function statusLabel(s) {
        switch (s) {
        case 1: return "SAFE"
        case 2: return "WARNING"
        case 3: return "CRITICAL"
        default: return "NO DATA"
        }
    }
}
