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
#include "arcommandcodec.h"

#include "common.h"
#include "arcommanddictionary.h"

#include <QtEndian>
#include <QDataStream>

#ifdef WANT_DEBUG
static void dumpCommandInvokationInfo(const ARCommandInfo *command, const QVariantMap &params)
{
    QStringList paramspec;
    foreach(ARCommandArgumentInfo *argument, command->arguments)
    {
        QVariant param = params.value(argument->name);
        QString value;

        if(argument->type == "u8")
        {
            value = QString::number(param.toUInt());
        }
        else if(argument->type == "u16")
        {
            value = QString::number(param.toUInt());
        }
        else if(argument->type == "u32")
        {
            value = QString::number(param.toUInt());
        }
        else if(argument->type == "u64")
        {
            value = QString::number(param.toULongLong());
        }
        else if(argument->type == "i8")
        {
            value = QString::number(param.toInt());
        }
        else if(argument->type == "i16")
        {
            value = QString::number(param.toInt());
        }
        else if(argument->type == "i32")
        {
            value = QString::number(param.toInt());
        }
        else if(argument->type == "i64")
        {
            value = QString::number(param.toLongLong());
        }
        else if(argument->type == "float")
        {
            value = QString::number(param.toFloat());
        }
        else if(argument->type == "double")
        {
            value = QString::number(param.toDouble());
        }
        else if(argument->type == "string")
        {
            value = param.toString();
        }
        else if(argument->type == "enum")
        {
            value = argument->enumeration.value(param.toInt());
        }
        else
        {
            WARNING_T(QString("Unhandled argument type: %1").arg(argument->type));
        }

        paramspec.append(QString("%1:%2").arg(argument->name).arg(value));
    }

    DEBUG_T(QString("Command: %1 %2 %3(%4)")
            .arg(command->klass->project)
            .arg(command->klass->name)
            .arg(command->name)
            .arg(paramspec.join(", ")));
}
#endif

ARCommandCodec::ARCommandCodec(QObject *parent)
    : QObject(parent)
{
    TRACE
}

ARCommandCodec::~ARCommandCodec()
{
    TRACE
}

QVariantMap ARCommandCodec::decode(ARCommandInfo *command, const char *data) const
{
    uint offset = 0;
    QVariantMap params;

    foreach(ARCommandArgumentInfo *argument, command->arguments)
    {
        QVariant value;

        if(argument->type == "u8")
        {
            int v = static_cast<int>(data[offset]);
            value.setValue(v);
            offset += 1;
        }
        else if(argument->type == "u16")
        {
            quint16 v = static_cast<quint16>(data[offset]);
            value.setValue(v);
            offset += 2;
        }
        else if(argument->type == "u32")
        {
            quint32 v = static_cast<quint32>(data[offset]);
            value.setValue(v);
            offset += 4;
        }
        else if(argument->type == "u64")
        {
            quint64 v = static_cast<quint64>(data[offset]);
            value.setValue(v);
            offset += 8;
        }
        else if(argument->type == "i8")
        {
            qint8 v = static_cast<qint8>(data[offset]);
            value.setValue(v);
            offset += 1;
        }
        else if(argument->type == "i16")
        {
            qint16 v = static_cast<qint16>(data[offset]);
            value.setValue(v);
            offset += 2;
        }
        else if(argument->type == "i32")
        {
            qint32 v = static_cast<qint32>(data[offset]);
            value.setValue(v);
            offset += 4;
        }
        else if(argument->type == "i64")
        {
            qint64 v = static_cast<qint64>(data[offset]);
            value.setValue(v);
            offset += 8;
        }
        else if(argument->type == "float")
        {
            float v = qFromLittleEndian(*((float*)(data + offset)));
            value.setValue(v);
            offset += 4;
        }
        else if(argument->type == "double")
        {
            double v = qFromLittleEndian(*((double*)(data + offset)));
            value.setValue(v);
            offset += 8;
        }
        else if(argument->type == "string")
        {
            QString v(data + offset);
            value.setValue(v);
            offset += v.length() + 1;
        }
        else if(argument->type == "enum")
        {
            qint32 v = qFromLittleEndian(static_cast<qint32>(data[offset]));
            value.setValue(v);
            offset += 4;
        }
        else
        {
            WARNING_T(QString("Unhandled argument type: %1").arg(argument->type));
        }

        params.insert(argument->name, value);
    }

#ifdef WANT_DEBUG
    dumpCommandInvokationInfo(command, params);
#endif

    return params;
}

QByteArray ARCommandCodec::encode(ARCommandInfo *command, const QVariantMap &params)
{
    QByteArray  payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    foreach(ARCommandArgumentInfo *argument, command->arguments)
    {
        QVariant param = params.value(argument->name);

        if(argument->type == "u8")
        {
            stream << static_cast<quint8>(param.toInt());
        }
        else if(argument->type == "u16")
        {
            stream << static_cast<quint16>(param.toInt());
        }
        else if(argument->type == "u32")
        {
            stream << static_cast<quint32>(param.toInt());
        }
        else if(argument->type == "u64")
        {
            stream << static_cast<quint64>(param.toInt());
        }
        else if(argument->type == "i8")
        {
            stream << static_cast<qint8>(param.toInt());
        }
        else if(argument->type == "i16")
        {
            stream << static_cast<qint16>(param.toInt());
        }
        else if(argument->type == "i32")
        {
            stream << static_cast<qint32>(param.toInt());
        }
        else if(argument->type == "i64")
        {
            stream << static_cast<qint64>(param.toInt());
        }
        else if(argument->type == "float")
        {
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            stream << param.toFloat();
        }
        else if(argument->type == "double")
        {
            stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
            stream << param.toDouble();
        }
        else if(argument->type == "string")
        {
            stream << param.toString().toUtf8().constData();
        }
        else if(argument->type == "enum")
        {
            stream << static_cast<qint32>(param.toInt());
        }
        else
        {
            WARNING_T(QString("Unhandled argument type: %1").arg(argument->type));
        }
    }

#ifdef WANT_DEBUG
    dumpCommandInvokationInfo(command, params);
#endif

    return payload;
}
