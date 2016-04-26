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
#ifndef ARCOMMANDLISTENER_H
#define ARCOMMANDLISTENER_H

#include <QObject>
#include <QVariant>

class ARCommandListener : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int listenerId READ listenerId NOTIFY listenerIdChanged)
    Q_PROPERTY(int projectId READ projectId WRITE setProjectId NOTIFY projectIdChanged)
    Q_PROPERTY(QString className READ className WRITE setClassName NOTIFY classNameChanged)
    Q_PROPERTY(QString commandName READ commandName WRITE setCommandName NOTIFY commandNameChanged)
    Q_PROPERTY(QVariant callback READ callback)

public:
    explicit ARCommandListener(QObject *parent = 0);
            ~ARCommandListener();

    int listenerId() const;
    void setListenerId(int listenerId);

    int projectId() const;
    Q_INVOKABLE void setProjectId(int id);

    QString className() const;
    Q_INVOKABLE void setClassName(const QString &className);

    QString commandName() const;
    Q_INVOKABLE void setCommandName(const QString &commandName);

    QVariant callback() const;
    void setCallback(QVariant callback);

Q_SIGNALS:
    void listenerIdChanged();
    void projectIdChanged();
    void classNameChanged();
    void commandNameChanged();

    void received(const QVariantMap &params);

private:
    class ARCommandListenerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(ARCommandListener)
};

#endif // ARCOMMANDLISTENER_H
