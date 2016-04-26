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
#include "ardevice.h"

#include "common.h"
#include "config.h"

struct ARDevicePrivate {
    ARDevicePrivate()
        : address(ARDEVICE_DEFAULT_ADDR),
          port(54321), updatePort(51), userPort(21),
          streamFragmentSize(1000), streamFragmentMax(128), streamMaxAckInterval(0)
    { /* ... */ }

    QString address;

    quint16 port;
    quint16 updatePort;
    quint16 userPort;

    int streamFragmentSize;
    int streamFragmentMax;
    int streamMaxAckInterval;
};

ARDevice::ARDevice(QObject *parent)
    : QObject(parent), d_ptr(new ARDevicePrivate)
{
    TRACE
}

ARDevice::~ARDevice()
{
    TRACE
    delete d_ptr;
}

QString ARDevice::address() const
{
    Q_D(const ARDevice);
    return d->address;
}

void ARDevice::setAddress(const QString &address)
{
    TRACE
    Q_D(ARDevice);

    if(d->address != address) {
        d->address = address;
        emit this->addressChanged();
    }
}

quint16 ARDevice::port() const
{
    Q_D(const ARDevice);
    return d->port;
}

void ARDevice::setPort(quint16 port)
{
    TRACE
    Q_D(ARDevice);
    if(d->port != port)
    {
        d->port = port;
        emit portChanged();
    }
}

quint16 ARDevice::updatePort() const
{
    Q_D(const ARDevice);
    return d->updatePort;
}

void ARDevice::setUpdatePort(quint16 port)
{
    TRACE
    Q_D(ARDevice);
    if(d->updatePort != port)
    {
        d->updatePort = port;
        emit updatePortChanged();
    }
}

quint16 ARDevice::userPort() const
{
    Q_D(const ARDevice);
    return d->userPort;
}

void ARDevice::setUserPort(quint16 port)
{
    TRACE
    Q_D(ARDevice);
    if(d->userPort != port)
    {
        d->userPort = port;
        emit userPortChanged();
    }
}

int ARDevice::streamFragmentSize() const
{
    Q_D(const ARDevice);
    return d->streamFragmentSize;
}

void ARDevice::setStreamFragmentSize(int size)
{
    TRACE
    Q_D(ARDevice);
    if(d->streamFragmentSize != size)
    {
        d->streamFragmentSize = size;
        emit streamFragmentSizeChanged();
    }
}

int ARDevice::streamFragmentMax() const
{
    Q_D(const ARDevice);
    return d->streamFragmentMax;
}

void ARDevice::setStreamFragmentMax(int num)
{
    TRACE
    Q_D(ARDevice);
    if(d->streamFragmentMax != num)
    {
        d->streamFragmentMax = num;
        emit streamFragmentMaxChanged();
    }
}

int ARDevice::streamMaxAckInterval() const
{
    Q_D(const ARDevice);
    return d->streamMaxAckInterval;
}

void ARDevice::setStreamMaxAckInterval(int interval)
{
    TRACE
    Q_D(ARDevice);
    if(d->streamMaxAckInterval != interval)
    {
        d->streamMaxAckInterval = interval;
        emit streamMaxAckIntervalChanged();
    }
}
