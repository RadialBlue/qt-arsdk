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
#include "arcommanddictionary.h"

#include "common.h"

#include <QFile>
#include <QXmlStreamReader>

struct ARCommandDictionaryPrivate
{
    QList<ARCommandInfo*> commands;
    QHash<QString,ARCommandClassInfo*> klasses;
};

ARCommandDictionary::ARCommandDictionary(QObject *parent)
    : QObject(parent), d_ptr(new ARCommandDictionaryPrivate)
{
    TRACE
}

ARCommandDictionary::~ARCommandDictionary()
{
    TRACE
    Q_D(ARCommandDictionary);

    foreach(ARCommandInfo *command, d->commands)
        delete command;

    foreach(ARCommandClassInfo *klass, d->klasses)
        delete klass;

    delete d_ptr;
}


QList<ARCommandInfo*> ARCommandDictionary::commands() const
{
    Q_D(const ARCommandDictionary);
    return d->commands;
}

ARCommandInfo* ARCommandDictionary::find(quint8 pId, quint8 cId, quint16 commandId) const
{
    Q_D(const ARCommandDictionary);

    foreach(ARCommandInfo *command, d->commands)
    {
        if(command->klass->project == pId && command->klass->id == cId && command->id == commandId)
            return command;
    }

    return NULL;
}

ARCommandInfo* ARCommandDictionary::find(quint8 pId, const QString &className, const QString &commandName) const
{
    Q_D(const ARCommandDictionary);

    foreach(ARCommandInfo *command, d->commands)
    {
        if(command->klass->project == pId && command->klass->name == className && command->name == commandName)
            return command;
    }

    return NULL;
}

bool ARCommandDictionary::import(const QString &path)
{
    TRACE
    Q_D(ARCommandDictionary);

    QFile file(path);
    QXmlStreamReader xml(&file);

    if(!file.exists())
    {
        WARNING_T("Failed to import: File not found!");
        return false;
    }

    file.open(QIODevice::ReadOnly);

    if(!xml.readNextStartElement() || xml.name() != "project")
    {
        WARNING_T("Failed to import: Format not understood.");
        return false;
    }

    bool ok = false;
    quint8 projectId = xml.attributes().value("id").toInt(&ok);

    if(!ok)
    {
        WARNING_T("Failed to import: Format not understood, no project id specified.");
        return false;
    }

    // Currently parsing these objects.
    ARCommandClassInfo    *klass = NULL;
    ARCommandInfo         *command = NULL;
    ARCommandArgumentInfo *argument = NULL;

    // The command Id is reflected by it's position in the Classes definition.
    int commandIndex = 0;

    while(!xml.atEnd())
    {
        xml.readNext();

        if(xml.isEndElement() && xml.name() == "project") break;

        // All the information we need is in the opening elements, so ignore everything else.
        if(!xml.isStartElement() && !xml.isEndElement()) continue;

        if(xml.name() == "class")
        {
            if(!klass)
            {
                commandIndex = 0; // Reset current commandIndex value.

                klass = new ARCommandClassInfo;
                klass->id = xml.attributes().value("id").toInt(&ok);
                klass->name = xml.attributes().value("name").toString();
                klass->project = projectId;

                if(!ok || klass->name.isEmpty())
                {
                    WARNING_T("Failed to parse class!");
                    delete klass;
                    return false;
                }
            }

            if(xml.isEndElement())
            {
                d->klasses.insert(klass->name, klass);
                klass = NULL;
            }
        }
        else if(xml.name() == "cmd")
        {
            if(!command)
            {
                if(!klass)
                {
                    WARNING_T("No current class instance!");
                    return false;
                }

                command = new ARCommandInfo;
                command->id = commandIndex;
                command->name = xml.attributes().value("name").toString();
                command->klass = klass;
                command->bufferId = xml.attributes().value("buffer").toString() == "NON_ACK" ? 0x0a : 0x0b;

                if(command->name.isEmpty())
                {
                    WARNING_T("Failed to parse command!");

                    delete command;
                    return false;
                }
            }

            if(xml.isEndElement())
            {
                d->commands.append(command);
                commandIndex++;
                command = NULL;
            }
        }
        else if(xml.name() == "arg")
        {
            if(!argument)
            {
                if(!command)
                {
                    WARNING_T("No current command instance!");
                    return false;
                }

                argument = new ARCommandArgumentInfo;
                argument->name = xml.attributes().value("name").toString();
                argument->type = xml.attributes().value("type").toString();

                if(argument->name.isEmpty() || argument->type.isEmpty())
                {
                    WARNING_T("Failed to parse argument!");
                    delete argument;
                    delete command;
                    return false;
                }
            }

            if(xml.isEndElement())
            {
                command->arguments.append(argument);
                argument = NULL;
            }
        }
        else if(xml.name() == "enum")
        {
            if(xml.isEndElement()) continue;

            if(!argument)
            {
                WARNING_T("No current argument instance!");
                return false;
            }

            QString enumV = xml.attributes().value("name").toString();
            if(enumV.isEmpty())
            {
                WARNING_T("Failed to parse enumeration!");
                return false;
            }

            argument->enumeration.append(enumV);
        }
        else
        {
            DEBUG_T(QString("Ignoring unknown tag: %1").arg(xml.name().toString()));
            return false;
        }
    }

    return true;
}
