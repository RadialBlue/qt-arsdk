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
#include "arnetdiscovery.h"

#include "arcontroller.h"
#include "ardiscoverydevice.h"

#include "common.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>

struct ARNetDiscoveryPrivate
{
    ARNetDiscoveryPrivate(ARController *pController)
        : controller(pController), socket(NULL)
    { /* ... */ }

    ARController *controller;
    QTcpSocket   *socket;

    QString errorString;
};

ARNetDiscovery::ARNetDiscovery(ARController *controller)
    : QObject(controller), d_ptr(new ARNetDiscoveryPrivate(controller))
{
    TRACE
}

ARNetDiscovery::~ARNetDiscovery()
{
    TRACE
    shutdown();
    delete d_ptr;
}

QString ARNetDiscovery::errorString() const
{
    Q_D(const ARNetDiscovery);
    return d->errorString;
}

bool ARNetDiscovery::connectToHost(const QString &address, quint16 port)
{
    TRACE
    Q_D(ARNetDiscovery);

    if(d->socket != NULL)
    {
        d->errorString = "Discovery socket already initialized!";
        emit error();
        return false;
    }

    d->socket = new QTcpSocket(this);

    QObject::connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError()));
    QObject::connect(d->socket, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    QObject::connect(d->socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));

    DEBUG_T(QString("Attempting to connect to: %1:%2").arg(address).arg(port));
    d->socket->connectToHost(address, port);

    return true;
}

void ARNetDiscovery::shutdown()
{
    TRACE
    Q_D(ARNetDiscovery);

    d->socket->close();
    d->socket->deleteLater();
    d->socket = NULL;
}

void ARNetDiscovery::onSocketError()
{
    TRACE
    Q_D(ARNetDiscovery);
    d->errorString = d->socket->errorString();
    emit error();
}

void ARNetDiscovery::onSocketConnected()
{
    TRACE
    Q_D(ARNetDiscovery);
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));

    QJsonObject mesg;
    mesg.insert(ARDISCOVERY_KEY_CONTROLLER_TYPE, d->controller->controllerType());
    mesg.insert(ARDISCOVERY_KEY_CONTROLLER_NAME, d->controller->controllerName());
    mesg.insert(ARDISCOVERY_KEY_D2CPORT,         d->controller->controllerPort());

    DEBUG_T("Discovery socket connected, sending registration.");
    QByteArray data = QJsonDocument(mesg).toJson(QJsonDocument::Compact);
    DEBUG_T(QString("TX: %1").arg(QString(data)));
    d->socket->write(data);
}

void ARNetDiscovery::onSocketDisconnected()
{
    TRACE
    DEBUG_T("Discovery socket disconnected.");
}

void ARNetDiscovery::onSocketReadyRead()
{
    TRACE
    Q_D(ARNetDiscovery);

    QByteArray data = d->socket->readAll();
    d->socket->close();
    d->socket->deleteLater();
    d->socket = NULL;

    DEBUG_T(QString("RX: %1").arg(QString(data)));

    if(data.at(data.length() - 1) == 0x00) data.chop(1);

    // Parse handshake response message.
    QJsonParseError parseError;
    QJsonDocument mesg = QJsonDocument::fromJson(data, &parseError);
    if(parseError.error != QJsonParseError::NoError) {
        WARNING_T(QString("KEY Parse Error: %1").arg(parseError.errorString()));

        d->errorString = "Failed to parse incoming message: " + parseError.errorString();
        emit error();
        return;
    }

    // Check return status of handshake response message.
    QJsonObject props = mesg.object();
    if(props.value("status").toInt(1) == 0)
    {
        // Build discovery device instance.
        ARDiscoveryDevice *device = new ARDiscoveryDevice(d->controller);
        device->setAddress(d->socket->peerAddress().toString());
        device->setParameters(props);

        DEBUG_T("Successfully discovered device!");
        emit discovered(device);
    }
    else
    {
        d->errorString = "Handshake returned error status.";
        emit error();
    }

    shutdown();
}
