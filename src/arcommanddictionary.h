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
#ifndef ARCOMMANDDICTIONARY_H
#define ARCOMMANDDICTIONARY_H

#include <QObject>
#include <QStringList>

struct ARCommandClassInfo
{
    quint8  id;
    quint8  project;
    QString name;
};

struct ARCommandArgumentInfo
{
    QString name;
    QString type;

    QStringList enumeration; // Used if type == 'enum'.
};

struct ARCommandInfo
{
    ARCommandInfo()
        : bufferId(0x0d)
    {/*...*/}

    ~ARCommandInfo()
    {
        foreach(ARCommandArgumentInfo *a, arguments) delete a;
    }

    quint16 id;
    QString name;

    ARCommandClassInfo *klass;

    QList<ARCommandArgumentInfo*> arguments;

    quint8 bufferId;
};

class ARCommandDictionary : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<ARCommandInfo*> commands READ commands)

public:
    explicit ARCommandDictionary(QObject *parent = 0);
            ~ARCommandDictionary();

    QList<ARCommandInfo*> commands() const;

    ARCommandInfo* find(quint8 pId, quint8 cId, quint16 commandId) const;
    ARCommandInfo* find(quint8 pId, const QString &className, const QString &commandName) const;

    bool import(const QString &path);

private:
    class ARCommandDictionaryPrivate *d_ptr;
    Q_DECLARE_PRIVATE(ARCommandDictionary)
};

#endif // ARCOMMANDDICTIONARY_H
