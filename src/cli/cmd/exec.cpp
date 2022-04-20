/*
 * Copyright (c) 2021. Uniontech Software Ltd. All rights reserved.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QtGlobal>
#include <QDebug>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <QFile>
#include <sys/wait.h>
#include <grp.h>

#include "cmd.h"

inline int bringDownPermissionsTo(const struct stat &fileStat)
{
    __gid_t newGid[1] = {fileStat.st_gid};

    setgroups(1, newGid);

    setgid(fileStat.st_gid);
    setegid(fileStat.st_gid);

    setuid(fileStat.st_uid);
    seteuid(fileStat.st_uid);
    return 0;
}

int execArgs(const std::vector<std::string> &args, const std::vector<std::string> &envStrVector)
{
    std::vector<char *> argVec(args.size() + 1);
    std::transform(args.begin(), args.end(), argVec.begin(), [](const std::string &str) -> char * {
        return const_cast<char *>(str.c_str());
    });
    argVec.push_back(nullptr);

    auto command = argVec.data();

    std::vector<char *> envVec(envStrVector.size() + 1);
    std::transform(envStrVector.begin(), envStrVector.end(), envVec.begin(), [](const std::string &str) -> char * {
        return const_cast<char *>(str.c_str());
    });
    envVec.push_back(nullptr);

    auto env = envVec.data();

    return execve(command[0], &command[0], &env[0]);
}

// /proc/{pid}/task/{pid}/children
QList<pid_t> childrenOf(pid_t p)
{
    auto childrenPath = QString("/proc/%1/task/%1/children").arg(p);
    QFile child(childrenPath);
    if (!child.open(QIODevice::ReadOnly)) {
        return {};
    }

    auto data = child.readAll();
    auto slice = QString::fromLatin1(data).split(" ");
    QList<pid_t> pidList;
    for (auto const &str : slice) {
        if (str.isEmpty()) {
            continue;
        }
        pidList.push_back(str.toInt());
    }
    return pidList;
}

int namespaceEnter(pid_t pid, const QStringList &args)
{
    int ret;
    struct stat fileStat = {};

    auto children = childrenOf(pid);
    if (children.isEmpty()) {
        qDebug() << "get children of pid failed" << errno << strerror(errno);
        return -1;
    }

    pid = childrenOf(pid).value(0);
    auto prefix = QString("/proc/%1/ns/").arg(pid);
    auto root = QString("/proc/%1/root").arg(pid);
    auto proc = QString("/proc/%1").arg(pid);

    ret = lstat(proc.toStdString().c_str(), &fileStat);
    if (ret < 0) {
        qDebug() << "lstat failed" << root << ret << errno << strerror(errno);
        return -1;
    }

    QStringList nameSpaceList = {
        "mnt",
        "ipc",
        "uts",
        "pid",
        // TODO: is hard to set user namespace, need carefully fork and unshare
//        "user",
        "net",
    };

    QList<int> fds;
    for (auto const &nameSpace : nameSpaceList) {
        auto currentNameSpace = (prefix + nameSpace).toStdString();
        int fd = open(currentNameSpace.c_str(), O_RDONLY);
        if (fd < 0) {
            qCritical() << "open failed" << currentNameSpace.c_str() << fd << errno << strerror(errno);
            continue;
        }
        qDebug() << "push" << fd << currentNameSpace.c_str();
        fds.push_back(fd);
    }

    int rootFd = open(root.toStdString().c_str(), O_RDONLY);

    ret = unshare(CLONE_NEWNS);
    if (ret < 0) {
        qCritical() << "unshare failed" << ret << errno << strerror(errno);
    }

    for (auto fd : fds) {
        ret = setns(fd, 0);
        if (ret < 0) {
            qCritical() << "setns failed" << fd << ret << errno << strerror(errno);
        }
        close(fd);
    }

    ret = fchdir(rootFd);
    if (ret < 0) {
        qCritical() << "chdir failed" << root << ret << errno << strerror(errno);
        return -1;
    }

    ret = chroot(".");
    if (ret < 0) {
        qCritical() << "chroot failed" << root << ret << errno << strerror(errno);
        return -1;
    }

    bringDownPermissionsTo(fileStat);

    int child = fork();
    if (child < 0) {
        qCritical() << "fork failed" << child << errno << strerror(errno);
        return -1;
    }

    if (0 == child) {
        QFile envFile("/run/app/env");
        if (!envFile.open(QIODevice::ReadOnly)) {
            qCritical() << "open failed" << envFile.fileName() << errno << strerror(errno);
        }

        std::vector<std::string> envVec;
        for (const auto &env : envFile.readAll().split('\n')) {
            if (!env.isEmpty()) {
                envVec.push_back(env.toStdString());
            }
        }

        std::vector<std::string> argVec(args.size());
        std::transform(args.begin(), args.end(), argVec.begin(), [](const QString &str) -> std::string {
            return str.toStdString();
        });

        return execArgs(argVec, envVec);
    }

    return waitpid(-1, nullptr, 0);
}