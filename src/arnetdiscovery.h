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
#ifndef ARNETDISCOVERY_H
#define ARNETDISCOVERY_H

#include <QObject>

#include "config.h"

class ARController;
class ARDiscoveryDevice;

class ARNetDiscovery : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString errorString READ errorString NOTIFY error)

public:
    explicit ARNetDiscovery(ARController *controller);
            ~ARNetDiscovery();

    QString errorString() const;

public Q_SLOTS:
    bool connectToHost(const QString &address, quint16 port = ARDISCOVERY_DEFAULT_PORT);
    void shutdown();

Q_SIGNALS:
    void discovered(ARDiscoveryDevice *device);
    void error();

protected Q_SLOTS:
    void onSocketError();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();

private:
    class ARNetDiscoveryPrivate *d_ptr;

    Q_DECLARE_PRIVATE(ARNetDiscovery)
};

#endif // ARCONNECTOR_H
