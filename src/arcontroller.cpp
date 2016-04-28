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

#include "ardevice.h"
#include "arnetdiscovery.h"

#include "arcommandcodec.h"
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
        : device(NULL),
          discovery(NULL),
          c2d(NULL),
          d2c(NULL),
          codec(NULL),
          commands(NULL),
          currentCommandListenerId(0),
          commsLogEnabled(false),
          frameNumber(0),
          hiAck(0),
          loAck(0)
    {/* ... */}

    // Controller details to report to device.
    QString controllerType;
    QString controllerName;
    quint16 controllerPort;

    // Device information and discovery.
    ARDevice    *device;
    ARNetDiscovery *discovery;

    QString     errorString;

    // Low-Level UDP communications handles.
    QUdpSocket *c2d;
    QUdpSocket *d2c;

    // TODO: Refactor out to NavdataCommandProcessor
    // Command resolution, parsing and processing data.
    ARCommandCodec       *codec;
    ARCommandDictionary  *commands;

    QList<ARCommandListener*> listeners;

    // Command listener ID counter.
    int currentCommandListenerId;

    // Stores the current sequence ids for each navdata buffer.
    QHash<quint8, quint8> sequenceIds;

    // TODO: Refactor out to Logger
    QFile commsLog;
    bool commsLogEnabled;
    QString commsLogLocation;

    // TODO: Refactor out into FrameDataProcessor (This is deprecated, StreamV2 ftw)
    // Video data properties.
    quint16 frameNumber;
    quint64 hiAck;
    quint64 loAck;
};

ARController::ARController(QObject *parent)
    : QObject(parent), d_ptr(new ARControllerPrivate)
{
    TRACE
    Q_D(ARController);

    // Default controller properties.
    d->controllerType = "qt-arsdk";
    d->controllerName = QHostInfo::localHostName();
    d->controllerPort = 43210;

    // For encoding/decoding command frame parameters from payloads.
    d->codec = new ARCommandCodec(this);

    // Navdata command introspection specs used by ARCommandCodec to encode/decode
    // datagrams.
    d->commands = new ARCommandDictionary(this);
    d->commands->import(":/ARSDK/packages/libARCommands/Xml/ARDrone3_commands.xml");
    d->commands->import(":/ARSDK/packages/libARCommands/Xml/common_commands.xml");
    d->commands->import(":/ARSDK/packages/libARCommands/Xml/common_debug.xml");
    d->commands->import(":/ARSDK/packages/libARCommands/Xml/SkyController_commands.xml");

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

ARDevice* ARController::device() const
{
    Q_D(const ARController);
    return d->device;
}

QString ARController::errorString() const
{
    Q_D(const ARController);
    return d->errorString;
}

ARController::ControllerStatus ARController::status() const
{
    Q_D(const ARController);

    if(d->discovery != NULL)
    {
        return ARController::Connecting;
    }
    else if(d->device != NULL && d->c2d != NULL && d->d2c != NULL)
    {
        return ARController::Connected;
    }

    return ARController::Disconnected;
}

bool ARController::isConnected() const
{
    Q_D(const ARController);
    return d->device != NULL && d->c2d != NULL;
}

bool ARController::commsLogEnabled() const
{
    Q_D(const ARController);
    return d->commsLogEnabled;
}

QQmlListProperty<ARCommandListener> ARController::commandListeners()
{
    Q_D(ARController);
    return QQmlListProperty<ARCommandListener>(this, d->listeners);
}

int ARController::appendCommandListener(int projId, const QString &className, const QString &command, QVariant param)
{
    Q_D(ARController);

    // Check that provided parameter is invokable.
    QJSValue jsvalue;
    if(param.userType() != qMetaTypeId<QJSValue>()) return -1;
    jsvalue = param.value<QJSValue>();
    if(!jsvalue.isCallable()) return -1;

    // Build command listener.
    ARCommandListener *listener = new ARCommandListener(this);
    listener->setProjectId(projId);
    listener->setClassName(className);
    listener->setCommandName(command);
    listener->setListenerId(d->currentCommandListenerId++);
    listener->setCallback(param);

    // Register command listener
    d->listeners.append(listener);
    return d->currentCommandListenerId - 1;
}

void ARController::removeCommandListener(int handlerId)
{
    Q_D(ARController);
    foreach(ARCommandListener *target, d->listeners) {
        if(target->listenerId() == handlerId) d->listeners.removeOne(target);
    }
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

    // Setup UDP port for D2C comms.
    d->d2c = new QUdpSocket(this);
    d->d2c->bind(QHostAddress::Any, d->controllerPort);

    QObject::connect(d->d2c, SIGNAL(readyRead()), this, SLOT(d2cRead()));

    // Setup TCP discovery connector.
    d->discovery = new ARNetDiscovery(this);
    d->discovery->connectToHost(address, port);

    QObject::connect(d->discovery, SIGNAL(discovered(ARDevice*)), this, SLOT(onDiscovered(ARDevice*)));
    QObject::connect(d->discovery, SIGNAL(failed(QString)), this, SLOT(onDiscoveryFailed(QString)));

    emit statusChanged();
    return true;
}

void ARController::shutdown()
{
    TRACE
    Q_D(ARController);

    if(d->discovery)
    {
        DEBUG_T("Destroying device discovery connector.");
        d->discovery->stop();
        d->discovery->deleteLater();
        d->discovery = NULL;
    }

    if(d->c2d)
    {
        DEBUG_T("Destroying C2D UDP socket.");
        d->c2d->close();
        d->c2d->deleteLater();
        d->c2d = NULL;
    }

    if(d->d2c)
    {
        DEBUG_T("Destroying D2C UDP socket.");
        d->d2c->close();
        d->d2c->deleteLater();
        d->d2c = NULL;
    }

    if(d->device)
    {
        DEBUG_T("Destroying device handle.");
        d->device->deleteLater();
        d->device = NULL;
    }

    emit statusChanged();
}

void ARController::onDiscovered(ARDevice *device)
{
    TRACE
    Q_D(ARController);
    d->device = device;

    // Setup UDP port for C2D comms.
    d->c2d = new QUdpSocket(this);
    d->c2d->connectToHost(device->address(), device->port());

    d->discovery->deleteLater();
    d->discovery = NULL;
    emit statusChanged();
}

void ARController::onDiscoveryFailed(const QString &reason)
{
    TRACE
    Q_D(ARController);

    d->d2c->deleteLater();
    d->d2c = NULL;

    d->discovery->deleteLater();
    d->discovery = NULL;

    d->errorString = reason;

    emit error();
    emit statusChanged();
}

bool ARController::sendFrame(quint8 type, quint8 id, const char* data, quint32 dataSize)
{
    TRACE
    Q_D(ARController);

    if(!d->c2d)
    {
        d->errorString = "Not connected to remote device";
        WARNING_T(d->errorString);
        emit error();
        return false;
    }

    QByteArray datagram(ARNETWORK_FRAME_HEADER_SIZE + dataSize, 0x00);
    QDataStream stream(&datagram, QIODevice::WriteOnly);

    if(!d->sequenceIds.contains(id)) d->sequenceIds.insert(id, 0x00);

    stream.setByteOrder(QDataStream::LittleEndian);
    // Frame Header
    stream  << type
            << id
            << d->sequenceIds.value(id)
            << (quint32)datagram.size();

    // If we have payload data, write to buffer.
    if(dataSize > 0) stream.writeRawData(data, dataSize);

    qint64 result = d->c2d->write(datagram.constData(), datagram.size());

    if(result != datagram.size())
    {
        // Should probably implement a retry timer, maybe queue based datagram sending.
        d->errorString = QString("TX ERROR:").arg(d->c2d->errorString());
        WARNING_T(d->errorString);
        emit error();
        return false;
    }

    if(type != ARController::LowLatencyData)
    {
        DEBUG_T(QString(">> %1:%2 [%3]")
                .arg(d->c2d->peerAddress().toString())
                .arg(d->c2d->peerPort())
                .arg(QString(datagram.toHex())));
    }

    d->sequenceIds.insert(id, d->sequenceIds.value(id) + 1);
    return true;
}

bool ARController::sendCommand(ARCommandInfo *command, const QVariantMap &params)
{
    TRACE
    Q_D(ARController);

    // Get encoded parameters for specified command.
    QByteArray paramData = d->codec->encode(command, params);

    // Encode command header and data into payload.
    QByteArray payload(ARNETWORK_COMMAND_HEADER_SIZE + paramData.size(), 0x00);
    QDataStream stream(&payload, QIODevice::WriteOnly);

    stream.setByteOrder(QDataStream::LittleEndian);

    stream << (quint8)command->klass->project;
    stream << (quint8)command->klass->id;
    stream << (quint16)command->id;

    stream.writeRawData(paramData.constData(), paramData.length());

    // Push frame for transmission.
    if(!sendFrame(ARController::Data, command->bufferId, payload.constData(), payload.size())) return false;
    return true;
}

bool ARController::sendCommand(int projId, int classId, int commandId, const QVariantMap &params)
{
    TRACE
    Q_D(ARController);

    ARCommandInfo *command = d->commands->find(projId, classId, commandId);
    if(!command)
    {
        d->errorString = "Failed to construct unknown command";
        WARNING_T(d->errorString);
        emit error();
        return false;
    }

    return sendCommand(command, params);
}

bool ARController::sendCommand(int projId, const QString &className, const QString &commandName, const QVariantMap &params)
{
    TRACE
    Q_D(ARController);

    ARCommandInfo *command = d->commands->find(projId, className, commandName);
    if(!command)
    {
        d->errorString = "Failed to construct unknown command";
        WARNING_T(d->errorString);
        emit error();
        return false;
    }

    return sendCommand(command, params);
}

struct ARFrame {
    quint8     type;
    quint8     id;
    quint8     seq;
    quint32    size;
    QByteArray payload;
};

void ARController::d2cRead()
{
    TRACE
    Q_D(ARController);

    while(d->d2c->hasPendingDatagrams())
    {
        QHostAddress remoteAddr;
        quint16      remotePort;

        if(d->d2c->pendingDatagramSize() < ARNETWORK_FRAME_HEADER_SIZE) {
            WARNING_T("Datagram incomplete?");
            return;
        }

        QByteArray datagram(d->d2c->pendingDatagramSize(), 0x00);
        qint32 frameOffset = 0;

        d->d2c->readDatagram(datagram.data(), datagram.size(), &remoteAddr, &remotePort);

        if(d->commsLog.isOpen()) {
            qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
            d->commsLog.write(QString::number(timestamp).toLatin1());
            d->commsLog.write("\n");
            d->commsLog.write(datagram.toBase64());
            d->commsLog.write("\n\n");
            d->commsLog.flush();
        }

        // While we still have frame data to process.
        while(frameOffset < datagram.size())
        {
            ARFrame frame;
            frame.type = static_cast<quint8>(datagram[frameOffset + 0]);
            frame.id   = static_cast<quint8>(datagram[frameOffset + 1]);
            frame.seq  = static_cast<quint8>(datagram[frameOffset + 2]);
            frame.size = qFromLittleEndian(*((quint32*)(datagram.constData() + frameOffset + 3)));

            // Test frame size for potential payload and copy if present.
            if(frame.size > ARNETWORK_FRAME_HEADER_SIZE)
            {
                const char *data = datagram.constData();
                frame.payload.insert(0, data + frameOffset + ARNETWORK_FRAME_HEADER_SIZE, frame.size - ARNETWORK_FRAME_HEADER_SIZE);
            }

            if(frame.id != ARNET_D2C_VIDEO_DATA_ID)
            {
                QString debugOut(datagram.toHex());
                DEBUG_T(QString("<< %1:%2 [%3]")
                        .arg(remoteAddr.toString())
                        .arg(remotePort)
                        .arg(QString(datagram.toHex())));
            }

            // Process frame depending on buffer id.
            if(frame.id == ARNET_D2C_PING_ID)
            {
                onPing(frame);
            }
            else if(frame.id == ARNET_D2C_EVENT_ID || frame.id == ARNET_D2C_NAVDATA_ID)
            {
                onNavdata(frame);
            }
            else if(frame.id == ARNET_D2C_VIDEO_DATA_ID)
            {
                onVideoData(frame);
                break;
            }
            else
            {
                WARNING_T(QString("Unhandled frame type: %1").arg(frame.id));
            }

            // Increment datagram offset for next frame.
            frameOffset += frame.size;
        }
    }
}

// TODO: Keep-Alive timer for connectivity monitoring.
//       Monitor latency/frequency for signal quality.
void ARController::onPing(const ARFrame &frame)
{
    TRACE
    Q_D(ARController);

    if(!d->c2d)
    {
        WARNING_T("Not connected to remote device.");
        return;
    }

    QByteArray response(frame.size, 0x00);
    QDataStream datastream(&response, QIODevice::WriteOnly);

    // Construct reply.
    datastream.setByteOrder(QDataStream::LittleEndian);
    datastream
            << frame.type
            << (quint8)ARNET_C2D_PONG_ID
            << frame.seq
            << frame.size;

    datastream.writeRawData(frame.payload.data(), frame.payload.size());

    DEBUG_T(QString(">> PONG %1:%2 [%3]")
            .arg(d->c2d->peerAddress().toString())
            .arg(d->c2d->peerPort())
            .arg(QString(response.toHex())));

    // Send frame to device.
    d->c2d->write(response);
}

void ARController::onNavdata(const ARFrame &frame)
{
    TRACE
    Q_D(ARController);

    if(frame.type == ARController::AcknowledgeData)
    {
        QByteArray payload(1, 0x00);
        QDataStream datastream(&payload, QIODevice::WriteOnly);

        // Construct reply.
        datastream.setByteOrder(QDataStream::LittleEndian);
        datastream << (quint8)frame.seq;

        sendFrame(ARController::Acknowledge, ARNET_C2D_NAVDATA_ACK_ID, payload.constData(), payload.size());
    }

    // Decode command header.
    quint8  project  = static_cast<quint8>(frame.payload[0]);
    quint8  klass    = static_cast<quint8>(frame.payload[1]);
    quint16 id       = qFromLittleEndian(static_cast<quint16>(frame.payload[2]));
    const char *data = frame.payload.data() + 4;

    // Resolve command meta-type information.
    ARCommandInfo *command = d->commands->find(project, klass, id);
    if(command == NULL)
    {
        d->errorString = QString("Unrecognised command: %1 %2 %3")
                .arg(project)
                .arg(klass)
                .arg(id);
        DEBUG_T(d->errorString);
        emit error();
        return;
    }

    // Use command codec to decode command parameters.
    QVariantMap params = d->codec->decode(command, data);
    DEBUG_T(QString("Decoded Command %1 %2 %3").arg(command->klass->project).arg(command->klass->name).arg(command->name));

    foreach(ARCommandListener *listener, d->listeners)
    {
        if(listener->projectId() == command->klass->project && listener->className() == command->klass->name && listener->commandName() == command->name)
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

    emit commandReceived(command->klass->project, command->klass->name, command->name, params);
}

void ARController::onVideoData(const ARFrame &frame)
{
    TRACE
    Q_D(ARController);

    quint16 frameNumber = qFromLittleEndian(*((quint16*)(frame.payload.constData())));
    quint8  frameFlags = static_cast<quint8>(frame.payload[2]);
    quint8  fragmentNumber = static_cast<quint8>(frame.payload[3]);
    quint8  fragsPerFrame = static_cast<quint8>(frame.payload[4]);

    if(frameNumber != d->frameNumber)
    {
        if(frameFlags == 0x01)
        {
            DEBUG_T(QString("Video Key Frame Header [%1] (%2 %3 %4)")
                    .arg(QString(QByteArray().append(frame.payload.constData(), 5).toHex()))
                    .arg(frameNumber)
                    .arg(fragmentNumber)
                    .arg(fragsPerFrame));
        }
        /*else
        {
            DEBUG_T(QString("Video Frame Header (%2 %3 %4)")
                    .arg(frameNumber)
                    .arg(fragmentNumber)
                    .arg(fragsPerFrame));
        }*/

        if(fragsPerFrame < 64)
        {
            d->hiAck = 0xffffffffffffffff;
            d->loAck = 0xffffffffffffffff << fragsPerFrame;
        }
        else if(fragsPerFrame < 128)
        {
            d->hiAck = 0xffffffffffffffff << (fragsPerFrame - 64);
            d->loAck = 0ll;
        }
        else
        {
            d->hiAck = 0ll;
            d->loAck = 0ll;
        }

        d->frameNumber = frameNumber;
    }

    if(fragmentNumber < 64)
    {
        d->loAck |= (1ll << fragmentNumber);
    }
    else if(fragmentNumber < 128)
    {
        d->hiAck |= (1ll << (fragmentNumber - 64));
    }

    // Construct reply.
    QByteArray payload(2 + 8 + 8, 0x00);
    QDataStream datastream(&payload, QIODevice::WriteOnly);

    if(!d->sequenceIds.contains(ARNET_C2D_VIDEO_ACK_ID))
        d->sequenceIds.insert(ARNET_C2D_VIDEO_ACK_ID, 0x00);

    datastream.setByteOrder(QDataStream::LittleEndian);
    datastream
            << (quint16)frameNumber
            << (quint64)d->hiAck
            << (quint64)d->loAck;

    sendFrame(ARController::LowLatencyData, ARNET_C2D_VIDEO_ACK_ID, payload.constData(), payload.size());
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
