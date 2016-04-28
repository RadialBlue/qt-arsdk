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
#ifndef ARDISCOVERYDEVICE_H
#define ARDISCOVERYDEVICE_H

#define ARDISCOVERY_KEY_STATUS                              "status"
#define ARDISCOVERY_KEY_C2DPORT                             "c2d_port"
#define ARDISCOVERY_KEY_D2CPORT                             "d2c_port"
#define ARDISCOVERY_KEY_ARSTREAM_FRAGMENT_SIZE              "arstream_fragment_size"
#define ARDISCOVERY_KEY_ARSTREAM_FRAGMENT_MAXIMUM_NUMBER    "arstream_fragment_maximum_number"
#define ARDISCOVERY_KEY_ARSTREAM_MAX_ACK_INTERVAL           "arstream_max_ack_interval"
#define ARDISCOVERY_KEY_CONTROLLER_TYPE                     "controller_type"
#define ARDISCOVERY_KEY_CONTROLLER_NAME                     "controller_name"
#define ARDISCOVERY_KEY_DEVICE_ID                           "device_id"
#define ARDISCOVERY_KEY_C2D_UPDATE_PORT                     "c2d_update_port"
#define ARDISCOVERY_KEY_C2D_USER_PORT                       "c2d_user_port"
#define ARDISCOVERY_KEY_SKYCONTROLLER_VERSION               "skycontroller_version"
#define ARDISCOVERY_KEY_FEATURES                            "features"

#define ARDISCOVERY_KEY_ARSTREAM2_CLIENT_STREAM_PORT        "arstream2_client_stream_port"
#define ARDISCOVERY_KEY_ARSTREAM2_CLIENT_CONTROL_PORT       "arstream2_client_control_port"
#define ARDISCOVERY_KEY_ARSTREAM2_SERVER_STREAM_PORT        "arstream2_server_stream_port"
#define ARDISCOVERY_KEY_ARSTREAM2_SERVER_CONTROL_PORT       "arstream2_server_control_port"
#define ARDISCOVERY_KEY_ARSTREAM2_MAX_PACKET_SIZE           "arstream2_max_packet_size"
#define ARDISCOVERY_KEY_ARSTREAM2_MAX_LATENCY               "arstream2_max_latency"
#define ARDISCOVERY_KEY_ARSTREAM2_MAX_NETWORK_LATENCY       "arstream2_max_network_latency"
#define ARDISCOVERY_KEY_ARSTREAM2_MAX_BITRATE               "arstream2_max_bitrate"
#define ARDISCOVERY_KEY_ARSTREAM2_PARAMETER_SETS            "arstream2_parameter_sets"

#define ARDISCOVERY_KEY_AUDIO_CODEC_VERSION                 "audio_codec"

#include <QObject>
#include <QJsonObject>

class ARDiscoveryDevice : public QObject
{
    Q_OBJECT

    // Target device address
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(QJsonObject parameters READ parameters NOTIFY parametersChanged)

public:
    explicit ARDiscoveryDevice(QObject *parent = 0);
            ~ARDiscoveryDevice();

    QString address() const;
    Q_INVOKABLE void setAddress(const QString &address);

    QJsonObject parameters() const;
    void setParameters(const QJsonObject &parameters);

Q_SIGNALS:
    void addressChanged();
    void parametersChanged();

private:
    class ARDiscoveryDevicePrivate *d_ptr;

    Q_DECLARE_PRIVATE(ARDiscoveryDevice)
};

#endif // ARDISCOVERYDEVICE_H
