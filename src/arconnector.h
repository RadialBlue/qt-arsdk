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
#ifndef ARCONNECTOR_H
#define ARCONNECTOR_H

#include <QObject>

#include "config.h"

class ARController;
class ARDevice;

class ARConnector : public QObject
{
    Q_OBJECT

public:
    explicit ARConnector(ARController *controller);
            ~ARConnector();

public Q_SLOTS:
    bool connect(const QString &address, quint16 port = ARDISCOVERY_DEFAULT_PORT);
    void stop();

Q_SIGNALS:
    void connected(ARDevice *device);
    void failed(const QString &reason);

protected Q_SLOTS:
    void onSocketError();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();

private:
    class ARConnectorPrivate *d_ptr;

    Q_DECLARE_PRIVATE(ARConnector)
};

#endif // ARCONNECTOR_H
