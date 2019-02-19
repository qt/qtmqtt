lessThan(QT_MAJOR_VERSION, 5) {
    error("Cannot build current Qt MQTT sources with Qt version $${QT_VERSION}.")
}

load(configure)
load(qt_parts)
