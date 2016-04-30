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
#include "arcontroller.h"
#include "common.h"

#include "arnetdiscovery.h"
#include "ardiscoverydevice.h"
#include "arcontrolconnection.h"

#include "arcommanddictionary.h"
#include "arcommandlistener.h"

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QStandardPaths>

#include <QtEndian>
#include <QDataStream>
#include <QHostInfo>
#include <QUdpSocket>

#include <QJSEngine>

struct ARControllerPrivate
{
    ARControllerPrivate()
        : controllerType(ARCONTROLLER_DEFAULT_TYPE),
          controllerName(QHostInfo::localHostName()),
          controllerAddress(ARCONTROLLER_DEFAULT_ADDR),
          controllerPort(ARCONTROLLER_DEFAULT_PORT),

          discovery(NULL),
          discoveryDevice(NULL),

          connection(NULL),

          currentCommandListenerId(0),

          status(ARController::Uninitialized),

          commsLogEnabled(false)
    {/* ... */}

    // Controller details to report to device.
    QString controllerType;
    QString controllerName;
    QString controllerAddress;
    quint16 controllerPort;

    // Device information and discovery.
    ARNetDiscovery      *discovery;
    ARDiscoveryDevice   *discoveryDevice;

    // Device control connection.
    ARControlConnection *connection;

    QList<ARCommandListener*> listeners;
    // Command listener ID counter.
    int currentCommandListenerId;

    // Controller status.
    ARController::ControllerStatus status;

    QString errorString;

    // TODO: Refactor out to Logger
    QFile commsLog;
    bool commsLogEnabled;
    QString commsLogLocation;
};

ARController::ARController(QObject *parent)
    : QObject(parent), d_ptr(new ARControllerPrivate)
{
    TRACE
    Q_D(ARController);

    //TODO: Refactor into separate "Logger" module.
    QDir logdir = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QString filename = "qt-arsdk-commslog-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".log";
    d->commsLogLocation = logdir.absoluteFilePath(filename);
    d->commsLogEnabled  = false;
}

ARController::~ARController()
{
    TRACE
    shutdown();
    delete d_ptr;
}

QString ARController::controllerType() const
{
    Q_D(const ARController);
    return d->controllerType;
}

void ARController::setControllerType(const QString &controllerType)
{
    TRACE
    Q_D(ARController);
    if(d->controllerType != controllerType) {
        d->controllerType = controllerType;
        emit controllerTypeChanged();
    }
}

QString ARController::controllerName() const
{
    Q_D(const ARController);
    return d->controllerName;
}

void ARController::setControllerName(const QString &controllerName)
{
    TRACE
    Q_D(ARController);
    if(d->controllerName != controllerName) {
        d->controllerName = controllerName;
        emit controllerNameChanged();
    }
}

QString ARController::controllerAddress() const
{
    Q_D(const ARController);
    return d->controllerAddress;
}

void ARController::setControllerAddress(const QString &controllerAddress)
{
    Q_D(ARController);
    if(d->controllerAddress != controllerAddress)
    {
        d->controllerAddress = controllerAddress;
        emit controllerAddressChanged();
    }
}

quint16 ARController::controllerPort() const
{
    Q_D(const ARController);
    return d->controllerPort;
}

void ARController::setControllerPort(quint16 controllerPort)
{
    TRACE
    Q_D(ARController);

    if(d->controllerPort != controllerPort) {
        d->controllerPort = controllerPort;
        emit controllerPortChanged();
    }
}

QQmlListProperty<ARCommandListener> ARController::commandListeners()
{
    Q_D(ARController);
    return QQmlListProperty<ARCommandListener>(this, d->listeners);
}

int ARController::appendCommandListener(const QString &command, QVariant param)
{
    Q_D(ARController);

    // Check that provided parameter is invokable.
    QJSValue jsvalue;
    if(param.userType() != qMetaTypeId<QJSValue>()) return -1;
    jsvalue = param.value<QJSValue>();
    if(!jsvalue.isCallable()) return -1;

    // Build command listener.
    ARCommandListener *listener = new ARCommandListener(this);
    listener->setListenerId(d->currentCommandListenerId++);
    listener->setCommandName(command);
    listener->setCallback(param);

    // Register command listener
    d->listeners.append(listener);
    emit commandListenersChanged();

    return d->currentCommandListenerId - 1;
}

void ARController::removeCommandListener(int handlerId)
{
    Q_D(ARController);
    foreach(ARCommandListener *target, d->listeners) {
        if(target->listenerId() == handlerId)
        {
            d->listeners.removeOne(target);
            emit commandListenersChanged();
            break;
        }
    }
}

ARDiscoveryDevice* ARController::discoveryDevice() const
{
    Q_D(const ARController);
    return d->discoveryDevice;
}

ARControlConnection* ARController::connection() const
{
    Q_D(const ARController);
    return d->connection;
}

QString ARController::errorString() const
{
    Q_D(const ARController);
    return d->errorString;
}

ARController::ControllerStatus ARController::status() const
{
    Q_D(const ARController);
    return d->status;
}

bool ARController::isConnected() const
{
    Q_D(const ARController);
    return d->status == ARController::Connected;
}

bool ARController::commsLogEnabled() const
{
    Q_D(const ARController);
    return d->commsLogEnabled;
}
bool ARController::connectToDevice(const QString &address, quint16 port)
{
    TRACE
    Q_D(ARController);

    if(isConnected())
    {
        d->errorString = "Already connected to a device";
        WARNING_T(d->errorString);
        emit error();
        return false;
    }

    if(d->discovery)
    {
        d->errorString = "Device discovery already in progress";
        WARNING_T(d->errorString);
        emit error();
        return false;
    }

    DEBUG_T("Creating network discovery connector...")
    // Setup TCP discovery connector.
    d->discovery = new ARNetDiscovery(this);

    QObject::connect(d->discovery, SIGNAL(discovered(ARDiscoveryDevice*)), this, SLOT(onDiscovered(ARDiscoveryDevice*)));
    QObject::connect(d->discovery, SIGNAL(error()), this, SLOT(onDiscoveryError()));

    DEBUG_T(QString("Attempting discovery of %1:%2").arg(address).arg(port));
    d->discovery->connectToHost(address, port);

    d->status = ARController::Discovering;
    emit statusChanged();
    return true;
}

void ARController::shutdown()
{
    TRACE
    Q_D(ARController);

    if(d->discovery != NULL)
    {
        DEBUG_T("Destroying device discovery connector.");
        d->discovery->shutdown();
        d->discovery->deleteLater();
        d->discovery = NULL;
    }

    if(d->discoveryDevice != NULL)
    {
        DEBUG_T("Destroying device handle.");
        d->discoveryDevice->deleteLater();
        d->discoveryDevice = NULL;
    }

    if(d->connection != NULL)
    {
        DEBUG_T("Destroying control connection.");
        d->connection->deleteLater();
        d->connection = NULL;
    }

    d->status = ARController::Disconnected;
    emit statusChanged();
}

void ARController::onDiscovered(ARDiscoveryDevice *device)
{
    TRACE
    Q_D(ARController);
    d->discoveryDevice = device;
    d->discovery->deleteLater();
    d->discovery = NULL;

    DEBUG_T("Device discovered, connecting...");
    d->connection = new ARControlConnection(this);

    d->status = ARController::Connecting;
    emit statusChanged();

    emit connectionChanged();
}

void ARController::onDiscoveryError()
{
    TRACE
    Q_D(ARController);

    DEBUG_T(QString("Discovery failed: %1").arg(d->discovery->errorString()));

    d->errorString = d->discovery->errorString();
    emit error();

    d->discovery->deleteLater();
    d->discovery = NULL;

    d->status = ARController::DiscoveryError;
    emit statusChanged();
}

void ARController::onCommandReceived(const ARCommandInfo &command, const QVariantMap &params)
{
    TRACE
    Q_D(const ARController);

    foreach(ARCommandListener *listener, d->listeners)
    {
        if(listener->commandName() == command.name)
        {
            if(!listener->callback().isNull()) {
                QJSValue callback = qvariant_cast<QJSValue>(listener->callback());
                QJSValue jsParams = callback.engine()->newObject();

                foreach(QString key, params.keys()) {
                    jsParams.setProperty(key, params.value(key).toString());
                }

                callback.call(QJSValueList() << jsParams);
            }

            emit listener->received(params);
        }
    }
}

void ARController::setCommsLogEnabled(bool enabled)
{
    Q_D(ARController);
    if(d->commsLogEnabled != enabled)
    {
        if(enabled) {
            d->commsLog.setFileName(d->commsLogLocation);

            DEBUG_T("Using communications log filepath: " + d->commsLog.fileName());
            if(!d->commsLog.open(QIODevice::Append)) {
                WARNING_T("Failed to open log file for writing!");
                WARNING_T(d->commsLog.errorString());
            }
        } else {
            if(d->commsLog.isOpen()) d->commsLog.close();
        }

        d->commsLogEnabled = d->commsLog.isOpen();
        emit commsLogEnabledChanged();
    }
}

QString ARController::commsLogLocation() const
{
    Q_D(const ARController);
    return d->commsLogLocation;
}

void ARController::setCommsLogLocation(const QString &location)
{
    Q_D(ARController);
    if(d->commsLogLocation != location)
    {
        d->commsLogLocation = location;
        emit commsLogLocationChanged();
    }
}
