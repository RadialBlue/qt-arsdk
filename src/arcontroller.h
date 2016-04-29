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
#ifndef ARCONTROLLER_H
#define ARCONTROLLER_H

#include <QObject>
#include <QJSValue>
#include <QQmlListProperty>

class ARDiscoveryDevice;
class ARCommandInfo;
class ARCommandListener;
class ARFrame;

class ARController : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("DefaultProperty", "commandListeners")

    Q_PROPERTY(QString controllerType READ controllerType WRITE setControllerType NOTIFY controllerTypeChanged)
    Q_PROPERTY(QString controllerName READ controllerName WRITE setControllerName NOTIFY controllerNameChanged)
    Q_PROPERTY(QString controllerAddress READ controllerAddress WRITE setControllerAddress NOTIFY controllerAddressChanged)
    Q_PROPERTY(quint16 controllerPort READ controllerPort WRITE setControllerPort NOTIFY controllerPortChanged)

    Q_PROPERTY(ARDiscoveryDevice* discoveryDevice READ discoveryDevice NOTIFY discoveryDeviceChanged)

    Q_PROPERTY(ControllerStatus status READ status NOTIFY statusChanged)

    Q_PROPERTY(bool isConnected READ isConnected NOTIFY statusChanged)

    Q_PROPERTY(QString errorString READ errorString NOTIFY error)

    Q_PROPERTY(QQmlListProperty<ARCommandListener> commandListeners READ commandListeners)

    Q_ENUMS(FrameType)

    Q_PROPERTY(bool    commsLogEnabled READ commsLogEnabled WRITE setCommsLogEnabled NOTIFY commsLogEnabledChanged)
    Q_PROPERTY(QString commsLogLocation READ commsLogLocation WRITE setCommsLogLocation NOTIFY commsLogEnabledChanged)

public:
    typedef enum {
        Uninitialized = 0,
        Discovering,
        DiscoveryError,
        Connecting,
        Connected,
        Disconnected
    } ControllerStatus;
    Q_ENUMS(ControllerStatus)

    // TODO: Remove ...
    typedef enum {
        NotInitialized = 0,
        Acknowledge,
        Data,
        LowLatencyData,
        AcknowledgeData
    } FrameType;

    explicit ARController(QObject *parent = 0);
            ~ARController();

    QString controllerType() const;
    Q_INVOKABLE void setControllerType(const QString &controllerType);

    QString controllerName() const;
    Q_INVOKABLE void setControllerName(const QString &controllerName);

    QString controllerAddress() const;
    void setControllerAddress(const QString &controllerAddress);

    quint16 controllerPort() const;
    Q_INVOKABLE void setControllerPort(quint16 controllerPort);

    ARDiscoveryDevice* discoveryDevice() const;

    QString errorString() const;

    ControllerStatus status() const;

    bool isConnected() const;

    // TODO: Remove (Needs to be refactored into different device API modules)
    QQmlListProperty<ARCommandListener> commandListeners(); // QML Property
    Q_INVOKABLE int  appendCommandListener(int projId, const QString &className, const QString &command, QVariant callback);
    Q_INVOKABLE void removeCommandListener(int handlerId);

    // TODO: Remove (Needs to be replaced with proper device API modules)
    Q_INVOKABLE bool sendCommand(int projId, const QString &className, const QString &command, const QVariantMap &params);

    // TODO: Move into separate Logger module.
    bool commsLogEnabled() const;
    Q_INVOKABLE void setCommsLogEnabled(bool enabled = true);

    QString commsLogLocation() const;
    Q_INVOKABLE void setCommsLogLocation(const QString &location);

Q_SIGNALS:
    void controllerTypeChanged();
    void controllerNameChanged();
    void controllerAddressChanged();
    void controllerPortChanged();

    void discoveryDeviceChanged();
    void statusChanged();
    void error();

    void commandReceived(int project, const QString &className, const QString &command, const QVariantMap &params);
    void videoFrameData(const QByteArray &data, bool isKey);

    void commsLogEnabledChanged();
    void commsLogLocationChanged();

public Q_SLOTS:
    bool connectToDevice(const QString &address, quint16 port = 44444);
    void shutdown();

protected Q_SLOTS:
    void onDiscovered(ARDiscoveryDevice *discoveryDevice);
    void onDiscoveryError();

    void d2cRead();

    bool sendFrame(quint8 type, quint8 id, const char* data, quint32 dataSize);
    bool sendCommand(ARCommandInfo *command, const QVariantMap &params);
    bool sendCommand(int projId, int classId, int commandId, const QVariantMap &params);

protected:
    void onPing(const ARFrame &frame);
    void onNavdata(const ARFrame &frame);
    void onVideoData(const ARFrame &frame);

private:
    class ARControllerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(ARController)
};

#endif // ARCONTROLLER_H
