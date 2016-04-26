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
#include "arcommandlistener.h"
#include "common.h"

struct ARCommandListenerPrivate
{
    ARCommandListenerPrivate()
        : projectId(0), className(""), commandName("")
    {/*...*/}

    int listenerId;
    int projectId;
    QString className;
    QString commandName;
    QVariant callback;
};

ARCommandListener::ARCommandListener(QObject *parent)
    : QObject(parent), d_ptr(new ARCommandListenerPrivate)
{
    TRACE
}

ARCommandListener::~ARCommandListener()
{
    TRACE
    delete d_ptr;
}

int ARCommandListener::listenerId() const
{
    Q_D(const ARCommandListener);
    return d->listenerId;
}

void ARCommandListener::setListenerId(int listenerId)
{
    Q_D(ARCommandListener);
    if(d->listenerId != listenerId)
    {
        d->listenerId = listenerId;
        emit listenerIdChanged();
    }
}

int ARCommandListener::projectId() const
{
    Q_D(const ARCommandListener);
    return d->projectId;
}

void ARCommandListener::setProjectId(int id)
{
    Q_D(ARCommandListener);
    if(d->projectId != id)
    {
        d->projectId = id;
        emit projectIdChanged();
    }
}

QString ARCommandListener::className() const
{
    Q_D(const ARCommandListener);
    return d->className;
}

void ARCommandListener::setClassName(const QString &className)
{
    Q_D(ARCommandListener);
    if(d->className != className)
    {
        d->className = className;
        emit classNameChanged();
    }
}

QString ARCommandListener::commandName() const
{
    Q_D(const ARCommandListener);
    return d->commandName;
}

void ARCommandListener::setCommandName(const QString &commandName)
{
    Q_D(ARCommandListener);
    if(d->commandName != commandName)
    {
        d->commandName = commandName;
        emit commandNameChanged();
    }
}

QVariant ARCommandListener::callback() const
{
    Q_D(const ARCommandListener);
    return d->callback;
}

void ARCommandListener::setCallback(QVariant callback)
{
    Q_D(ARCommandListener);
    d->callback = callback;
}
