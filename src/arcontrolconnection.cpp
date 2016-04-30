#include "common.h"
#include "config.h"

#include "arcontrolconnection.h"
#include "arcontroller.h"

#include "arcommandcodec.h"
#include "arcommanddictionary.h"
#include "arcommandlistener.h"

#include "ardiscoverydevice.h"

#include <QtEndian>
#include <QDataStream>
#include <QUdpSocket>

#include <QJSEngine>

class ARControlConnectionPrivate
{
public:
    ARControlConnectionPrivate(ARController *c, ARControlConnection *q)
        : controller(c),
          d2c(NULL),
          c2d(NULL),
          codec(new ARCommandCodec(q)),
          commands(new ARCommandDictionary(q)),
          q_ptr(q)
    {/* ... */}

    ARController *controller;

    // Device-to-Controller and Controller-to-Device sockets.
    QUdpSocket *d2c;
    QUdpSocket *c2d;

    // Stores the current sequence ids for each frame buffer.
    QHash<quint8, quint8> sequenceIds;

    // Navdata decoding.
    ARCommandCodec *codec;
    ARCommandDictionary *commands;

    QString errorString;

    // TODO: Refactor out into FrameDataProcessor? (This is deprecated, StreamV2 ftw)
    // Video streaming data
    quint16 frameNumber;
    quint64 hiAck;
    quint64 loAck;

    ARControlConnection *q_ptr;
};

ARControlConnection::ARControlConnection(ARController *controller)
    : QObject(controller), d_ptr(new ARControlConnectionPrivate(controller, this))
{
    Q_D(ARControlConnection);
    ARDiscoveryDevice *device = d->controller->discoveryDevice();

    DEBUG_T("Loading command dictionary data...");
    d->commands->import(":/ARSDK/packages/libARCommands/Xml/ARDrone3_commands.xml");
    d->commands->import(":/ARSDK/packages/libARCommands/Xml/common_commands.xml");
    d->commands->import(":/ARSDK/packages/libARCommands/Xml/common_debug.xml");
    d->commands->import(":/ARSDK/packages/libARCommands/Xml/SkyController_commands.xml");

    DEBUG_T("Creating D2C UDP communications socket...");
    // Setup UDP port for D2C comms.
    d->d2c = new QUdpSocket(this);
    d->d2c->bind(QHostAddress(d->controller->controllerAddress()),
                 d->controller->controllerPort());

    QObject::connect(d->d2c, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

    DEBUG_T("Creating C2D UDP communications socket...");
    // Setup UDP port for C2D comms.
    d->c2d = new QUdpSocket(this);
    d->c2d->connectToHost(device->address(),
                          device->parameters().value(ARDISCOVERY_KEY_C2DPORT).toInt());
}

ARControlConnection::~ARControlConnection()
{
    Q_D(ARControlConnection);
    if(d->c2d != NULL)
    {
        d->c2d->close();
        d->c2d->deleteLater();
        d->c2d = NULL;
    }

    if(d->d2c != NULL)
    {
        d->d2c->close();
        d->d2c->deleteLater();
        d->d2c = NULL;
    }

    delete d_ptr;
}

bool ARControlConnection::sendFrame(quint8 type, quint8 id, quint8 seq, const char *data, quint32 dataSize)
{
    Q_D(ARControlConnection);

    // Check we're actually able to send.
    if(d->c2d == NULL || !d->c2d->isWritable())
    {
        d->errorString = "Control connection not establised.";
        emit error();
        return false;
    }

    QByteArray  datagram(ARNETWORK_FRAME_HEADER_SIZE + dataSize, 0x00);
    QDataStream stream(&datagram, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Write frame header to buffer.
    stream << type
           << id
           << seq
           << (quint32)datagram.size();

    // Write frame payload to buffer.
    if(dataSize > 0) stream.writeRawData(data, dataSize);

    qint64 result = d->c2d->write(datagram.constData(), datagram.size());
    if(result != datagram.size())
    {
        //TODO: Maybe retry here ..
        WARNING_T("Failed to send complete message");
        return false;
    }

    if(type != ARControlConnection::LowLatencyData)
    {
        DEBUG_T(QString(">> %1:%2 [%3]")
                .arg(d->c2d->peerAddress().toString())
                .arg(d->c2d->peerPort())
                .arg(QString(datagram.toHex())));
    }

    return true;
}

bool ARControlConnection::sendFrame(quint8 type, quint8 id, const char *data, quint32 dataSize)
{
    Q_D(ARControlConnection);

    // Pre-populate buffer sequence id if counter doesn't already exist.
    if(!d->sequenceIds.contains(id)) d->sequenceIds.insert(id, 0x00);

    quint8 seq = d->sequenceIds.value(id);

    if(!this->sendFrame(type, id, seq, data, dataSize)) return false;

    d->sequenceIds.insert(id, seq + 1);
    return true;
}

bool ARControlConnection::sendCommand(ARCommandInfo *command, const QVariantMap &params)
{
    Q_D(ARControlConnection);

    QByteArray  paramData = d->codec->encode(command, params);
    QByteArray  payload(ARNETWORK_COMMAND_HEADER_SIZE + paramData.length(), 0x00);

    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << (quint8)command->klass->project;
    stream << (quint8)command->klass->id;
    stream << (quint16)command->id;
    stream.writeRawData(paramData.constData(), paramData.length());

    return sendFrame(ARControlConnection::Data,
                     command->bufferId,
                     payload.constData(),
                     payload.size());
}

bool ARControlConnection::sendCommand(int projId, int classId, int commandId, const QVariantMap &params)
{
    Q_D(ARControlConnection);

    ARCommandInfo *command = d->commands->find(projId, classId, commandId);
    if(!command)
    {
        d->errorString = "Unable to resolve command";
        emit error();
        return false;
    }

    return sendCommand(command, params);
}

struct ARControlFrame {
    quint8     type;
    quint8     id;
    quint8     seq;
    quint32    size;
    QByteArray payload;
};

void ARControlConnection::onReadyRead()
{
    Q_D(ARControlConnection);

    while(d->d2c->hasPendingDatagrams())
    {
        QHostAddress remoteAddr;
        quint16      remotePort;

        // If we've not received enough data, then return and wait for more.
        if(d->d2c->pendingDatagramSize() < ARNETWORK_FRAME_HEADER_SIZE) break;

        QByteArray datagram(d->d2c->pendingDatagramSize(), 0x00);
        quint32 offset = 0;

        d->d2c->readDatagram(datagram.data(), datagram.size(), &remoteAddr, &remotePort);

        while(offset < datagram.size())
        {
            ARControlFrame frame;
            frame.type = static_cast<quint8>(datagram[offset + 0]);
            frame.id = static_cast<quint8>(datagram[offset + 1]);
            frame.seq = static_cast<quint8>(datagram[offset + 2]);
            frame.size = qFromLittleEndian(*((quint32*)(datagram.constData() + offset + 3)));

            // Copy datagram data to frame if the frame size is larger than just the header.
            if(frame.size > ARNETWORK_FRAME_HEADER_SIZE)
            {
                const char *data = datagram.constData();
                frame.payload.insert(0, data + offset + ARNETWORK_FRAME_HEADER_SIZE, frame.size - ARNETWORK_FRAME_HEADER_SIZE);
            }

            // Output comms debug info (if it's not a video data frame).
            if(frame.id != ARNET_D2C_VIDEO_DATA_ID)
            {
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
            }
            else
            {
                WARNING_T(QString("Unhandled frame id: %1").arg(frame.id));
            }

            // Increment datagram offset to continue processing next frame.
            offset += frame.size;
        }
    }
}

// TODO: Keep-Alive timer for connectivity monitoring.
//       Monitor latency/frequency for signal quality.
void ARControlConnection::onPing(const ARControlFrame &frame)
{
    sendFrame(frame.type,
              (quint8)ARNET_C2D_PONG_ID,
              frame.seq,
              frame.payload.data(),
              frame.payload.size());
}

void ARControlConnection::onNavdata(const ARControlFrame &frame)
{
    Q_D(ARControlConnection);

    // TODO: Should we do this, or should we allow application to decide
    // when/if an acknowledge occurs?
    // Construct acknowledge frame, if this incoming frame requires it.
    if(frame.type == ARControlConnection::AcknowledgeData)
    {
        QByteArray payload(1, 0x00);
        QDataStream datastream(&payload, QIODevice::WriteOnly);
        datastream.setByteOrder(QDataStream::LittleEndian);
        datastream << (quint8)frame.seq;
        sendFrame(ARControlConnection::Acknowledge, ARNET_C2D_NAVDATA_ACK_ID, payload.constData(), payload.size());
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
        WARNING_T(QString("Unrecognised command: %1 %2 %3")
                  .arg(project)
                  .arg(klass)
                  .arg(id));
        return;
    }

    // Use command codec to decode command parameters.
    QVariantMap params = d->codec->decode(command, data);
    DEBUG_T(QString("Decoded Command %1 %2 %3").arg(command->klass->project).arg(command->klass->name).arg(command->name));

    d->controller->onCommandReceived(*command, params);
}

void ARControlConnection::onVideoData(const ARControlFrame &frame)
{
    TRACE
    Q_D(ARControlConnection);

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
        else
        {
            DEBUG_T(QString("Video Frame Header (%2 %3 %4)")
                    .arg(frameNumber)
                    .arg(fragmentNumber)
                    .arg(fragsPerFrame));
        }

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

    sendFrame(ARControlConnection::LowLatencyData, ARNET_C2D_VIDEO_ACK_ID, payload.constData(), payload.size());
}
