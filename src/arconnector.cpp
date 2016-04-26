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
#include "arconnector.h"

#include "arcontroller.h"
#include "ardevice.h"

#include "common.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>

struct ARConnectorPrivate
{
    ARConnectorPrivate(ARController *pController)
        : controller(pController), socket(NULL)
    { /* ... */ }

    ARController *controller;
    QTcpSocket   *socket;
};

ARConnector::ARConnector(ARController *controller)
    : QObject(controller), d_ptr(new ARConnectorPrivate(controller))
{
    TRACE
}

ARConnector::~ARConnector()
{
    TRACE
    delete d_ptr;
}

bool ARConnector::connect(const QString &address, quint16 port)
{
    TRACE
    Q_D(ARConnector);

    if(d->socket != NULL)
    {
        emit failed("Discovery socket already initialized!");
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

void ARConnector::stop()
{
    TRACE
    Q_D(ARConnector);

    d->socket->close();
    d->socket->deleteLater();
    d->socket = NULL;
}

void ARConnector::onSocketError()
{
    TRACE
    Q_D(ARConnector);
    WARNING_T(d->socket->errorString());
    emit failed(d->socket->errorString());
}

void ARConnector::onSocketConnected()
{
    TRACE
    Q_D(ARConnector);
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));

    QJsonObject mesg;
    mesg.insert("controller_type", d->controller->controllerType());
    mesg.insert("controller_name", d->controller->controllerName());
    mesg.insert("d2c_port",        d->controller->controllerPort());

    DEBUG_T("Discovery socket connected, sending registration.");
    QByteArray data = QJsonDocument(mesg).toJson(QJsonDocument::Compact);
    DEBUG_T(QString("SENT: %1").arg(QString(data)));
    d->socket->write(data);
}

void ARConnector::onSocketDisconnected()
{
    TRACE
    DEBUG_T("Discovery socket disconnected.");
}

void ARConnector::onSocketReadyRead()
{
    TRACE
    Q_D(ARConnector);
    QByteArray data = d->socket->readLine();
    DEBUG_T(QString("RECV: %1").arg(QString(data)));

    if(data.at(data.length() - 1) == 0x00) data.chop(1);

    QJsonParseError error;
    QJsonDocument mesg = QJsonDocument::fromJson(data, &error);
    if(error.error != QJsonParseError::NoError) {
        WARNING_T(QString("JSON Parse Error: %1").arg(error.errorString()));
        return;
    }

    QJsonObject props = mesg.object();
    if(props.value("status").toInt(1) != 0) {
        d->socket->close();
        d->socket->deleteLater();
        d->socket = NULL;

        emit failed("Registration Failed: HANDSHAKE NOT SUCCESSFUL!");
        return;
    }

    ARDevice *device = new ARDevice(d->controller);
    // Device Properties
    device->setAddress(d->socket->peerAddress().toString());

    // Controller -> Device ports.
    device->setPort(props.value("c2d_port").toInt());
    device->setUpdatePort(props.value("c2d_update_port").toInt());
    device->setUserPort(props.value("c2d_user_port").toInt());

    // Device stream properties.
    device->setStreamFragmentSize(props.value("arstream_fragment_size").toInt());
    device->setStreamFragmentMax(props.value("arstream_fragment_maximum_number").toInt());
    device->setStreamMaxAckInterval(props.value("arstream_max_ack_interval").toInt());

    DEBUG_T("Registration successfull!");
    d->socket->close();
    emit connected(device);
}
