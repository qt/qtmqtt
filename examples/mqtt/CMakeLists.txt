# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET Qt::Gui AND TARGET Qt::Widgets)
    qt_internal_add_example(simpleclient)
    qt_internal_add_example(subscriptions)
endif()
if(TARGET Qt::Quick)
    qt_internal_add_example(quicksubscription)
    qt_internal_add_example(quickpublication)
endif()
if(TARGET Qt::WebSockets)
    qt_internal_add_example(websocketsubscription)
endif()
