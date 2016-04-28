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
#include "ardiscoverydevice.h"

#include "common.h"
#include "config.h"

struct ARDiscoveryDevicePrivate {
    QString     address;
    QJsonObject parameters;
};

ARDiscoveryDevice::ARDiscoveryDevice(QObject *parent)
    : QObject(parent), d_ptr(new ARDiscoveryDevicePrivate)
{
    TRACE
}

ARDiscoveryDevice::~ARDiscoveryDevice()
{
    TRACE
    delete d_ptr;
}

QString ARDiscoveryDevice::address() const
{
    Q_D(const ARDiscoveryDevice);
    return d->address;
}

void ARDiscoveryDevice::setAddress(const QString &address)
{
    TRACE
    Q_D(ARDiscoveryDevice);

    if(d->address != address) {
        d->address = address;
        emit this->addressChanged();
    }
}

QJsonObject ARDiscoveryDevice::parameters() const
{
    Q_D(const ARDiscoveryDevice);
    return d->parameters;
}

void ARDiscoveryDevice::setParameters(const QJsonObject &parameters)
{
    TRACE
    Q_D(ARDiscoveryDevice);
    if(d->parameters != parameters)
    {
        d->parameters = parameters;
        emit parametersChanged();
    }
}
