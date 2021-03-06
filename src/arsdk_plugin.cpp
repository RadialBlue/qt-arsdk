/*
    The file is part of the qt-arsdk project.

    Copyright (C) 2015-2016 Tom Swindell <t.swindell@rubyx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include "arsdk_plugin.h"

#include <qqml.h>

#include "arcontroller.h"
#include "arcontrolconnection.h"
#include "arcommandlistener.h"
#include "ardiscoverydevice.h"

void ARSDKPlugin::registerTypes(const char *uri)
{
    // @uri arsdk
    qmlRegisterType<ARController>(uri, 1, 0, "ARController");
    qmlRegisterUncreatableType<ARControlConnection>(uri, 1, 0, "ARControlConnection", "Uncreatable type");
    qmlRegisterType<ARCommandListener>(uri, 1, 0, "ARCommandListener");
    qmlRegisterType<ARDiscoveryDevice>(uri, 1, 0, "ARDiscoveryDevice");
}
