QT -= gui
QT += mqtt testlib
QT += mqtt-private

SOURCES += \
        main.cpp

LIBS += -fsanitize=fuzzer
