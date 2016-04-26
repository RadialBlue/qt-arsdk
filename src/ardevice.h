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
#ifndef ARDEVICE_H
#define ARDEVICE_H

#include <QObject>

class ARDevice : public QObject
{
    Q_OBJECT

    // Target device address
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)

    // Controller -> Device port details
    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(quint16 updatePort READ updatePort WRITE setUpdatePort NOTIFY updatePortChanged)
    Q_PROPERTY(quint16 userPort READ userPort WRITE setUserPort NOTIFY userPortChanged)

    // Device Stream Properties
    Q_PROPERTY(int streamFragmentSize READ streamFragmentSize WRITE setStreamFragmentSize NOTIFY streamFragmentSizeChanged)
    Q_PROPERTY(int streamFragmentMax READ streamFragmentMax WRITE setStreamFragmentMax NOTIFY streamFragmentMaxChanged)
    Q_PROPERTY(int streamMaxAckInterval READ streamMaxAckInterval WRITE setStreamMaxAckInterval NOTIFY streamMaxAckIntervalChanged)

public:
    explicit ARDevice(QObject *parent = 0);
            ~ARDevice();

    QString address() const;
    Q_INVOKABLE void setAddress(const QString &address);

    quint16 port() const;
    Q_INVOKABLE void setPort(quint16 port);

    quint16 updatePort() const;
    Q_INVOKABLE void setUpdatePort(quint16 port);

    quint16 userPort() const;
    Q_INVOKABLE void setUserPort(quint16 port);

    int streamFragmentSize() const;
    Q_INVOKABLE void setStreamFragmentSize(int size);

    int streamFragmentMax() const;
    Q_INVOKABLE void setStreamFragmentMax(int num);

    int streamMaxAckInterval() const;
    Q_INVOKABLE void setStreamMaxAckInterval(int interval);

Q_SIGNALS:
    void addressChanged();
    void portChanged();
    void updatePortChanged();
    void userPortChanged();

    void streamFragmentSizeChanged();
    void streamFragmentMaxChanged();
    void streamMaxAckIntervalChanged();

private:
    class ARDevicePrivate *d_ptr;

    Q_DECLARE_PRIVATE(ARDevice)
};

#endif // ARDEVICE_H
