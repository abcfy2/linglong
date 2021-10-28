/*
 * Copyright (c) 2021. Uniontech Software Ltd. All rights reserved.
 *
 * Author:     huqinghong@uniontech.com
 *
 * Maintainer: huqinghong@uniontech.com
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gtest/gtest.h>

#include "module/repo/ostree_repohelper.h"

TEST(OstreeRepoHelperT01, ensureRepoEnv)
{
    char curpath[512] = {'\0'};
    getcwd(curpath, 512);
    string path = curpath;
    const QString repoPath = QString::fromStdString(path + "/repotest");
    //const QString repoPath = "/home/xxxx/8.linglong/GitPrj/debug/linglong/build/repotest";
    QString err = "";
    linglong::OstreeRepoHelper repo;
    bool ret = repo.ensureRepoEnv(repoPath, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    } else {
        //std::cout << repoPath.toStdString();
        qInfo() << repoPath;
    }
    //std::cout.flush();
    EXPECT_EQ(ret, true);
}

//必须先使用ostree命令添加仓库,拷贝/usr/local/var/lib/flatpak/repo/flathub.trustedkeys.gpg  gpg 文件
// 否则无法下载flatpak仓库软件包
// 注意使用ostree添加的仓库和flatpak remote-add 结果不一样
//ostree 添加远端仓库命令
//ostree --repo=repo remote add --no-gpg-verify flathub https://flathub.org/repo/flathub.flatpakrepo
//添加玲珑测试仓库
//ostree --repo=repo remote add --no-gpg-verify repo http://10.20.54.2/linglong/pool/repo/
// flatpak命令添加远端仓库的命令如下：
//flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
// 上述命令执行完成后 会在/usr/local/var/lib/flatpak 目录生成一个 repo 目录 repo目录的config文件 就是仓库配置文件。
// [core]
// repo_version=1
// mode=bare-user-only

// [remote "flathub"]
// gpg-verify=true
// gpg-verify-summary=true
// url=https://dl.flathub.org/repo/
// xa.title=Flathub
// xa.title-is-set=true
//执行OstreeRepoHelperT02之后的用例需要将OstreeRepoHelperT01中生成的repo目录中的config文件改成上面的内容（相当于添加本地仓库操作）
// repoPath 目录要保证当前操作的用户可写，否则mkdir报错
TEST(OstreeRepoHelperT02, getRemoteRepoList)
{
    char curpath[512] = {'\0'};
    getcwd(curpath, 512);
    string path = curpath;
    const QString repoPath = QString::fromStdString(path + "/repotest");
    QString err = "";
    linglong::OstreeRepoHelper repo;
    bool ret = repo.ensureRepoEnv(repoPath, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    }
    EXPECT_EQ(ret, true);
    QVector<QString> qvec;
    ret = repo.getRemoteRepoList(repoPath, qvec, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
        EXPECT_EQ(ret, false);
    } else {
        for (auto iter = qvec.begin(); iter != qvec.end(); ++iter) {
            //std::cout << (*iter).toStdString() << endl;
            //std::cout.flush();
            qInfo() << *iter;
        }
        EXPECT_EQ(ret, true);
    }
}

TEST(OstreeRepoHelperT03, getRemoteRefs)
{
    char curpath[512] = {'\0'};
    getcwd(curpath, 512);
    string path = curpath;
    const QString repoPath = QString::fromStdString(path + "/repotest");
    QString err = "";
    linglong::OstreeRepoHelper repo;
    bool ret = repo.ensureRepoEnv(repoPath, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    }
    EXPECT_EQ(ret, true);
    QVector<QString> qvec;
    ret = repo.getRemoteRepoList(repoPath, qvec, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
        EXPECT_EQ(ret, false);
        return;
    } else {
        for (auto iter = qvec.begin(); iter != qvec.end(); ++iter) {
            //std::cout << (*iter).toStdString() << endl;
            qInfo() << *iter;
        }
    }
    EXPECT_EQ(ret, true);
    QMap<QString, QString> outRefs;
    ret = repo.getRemoteRefs(repoPath, qvec[0], outRefs, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    } else {
        //  for (auto iter = outRefs.begin(); iter != outRefs.end(); ++iter) {
        //      std::cout << "ref:" << iter.key().toStdString() << ", commit value:" << iter.value().toStdString() << endl;
        //      std::cout.flush();
        //  }
    }
    EXPECT_EQ(ret, true);
}

TEST(OstreeRepoHelperT04, resolveMatchRefs)
{
    char curpath[512] = {'\0'};
    getcwd(curpath, 512);
    string path = curpath;
    const QString repoPath = QString::fromStdString(path + "/repotest");
    QString err = "";
    linglong::OstreeRepoHelper repo;
    bool ret = repo.ensureRepoEnv(repoPath, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    }
    EXPECT_EQ(ret, true);
    QVector<QString> qrepoList;
    ret = repo.getRemoteRepoList(repoPath, qrepoList, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
        EXPECT_EQ(ret, false);
        return;
    } else {
        for (auto iter = qrepoList.begin(); iter != qrepoList.end(); ++iter) {
            //std::cout << (*iter).toStdString() << endl;
            qInfo() << *iter;
        }
    }
    EXPECT_EQ(ret, true);
    QMap<QString, QString> outRefs;
    ret = repo.getRemoteRefs(repoPath, qrepoList[0], outRefs, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    } else {
        //  for (auto iter = outRefs.begin(); iter != outRefs.end(); ++iter) {
        //      std::cout << "ref:" << iter.key().toStdString() << ", commit value:" << iter.value().toStdString() << endl;
        //      std::cout.flush();
        //  }
    }
    EXPECT_EQ(ret, true);

    QString matchRef = "";
    QString pkgName = "org.deepin.calculator";
    //QString pkgName = "us.zoom.Zoom";
    QString arch = "x86_64";
    ret = repo.queryMatchRefs(repoPath, qrepoList[0], pkgName, arch, matchRef, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    } else {
        //std::cout << matchRef.toStdString();
        qInfo() << matchRef;
    }
    EXPECT_EQ(ret, true);
}

TEST(OstreeRepoHelperT05, repoPull)
{
    char curpath[512] = {'\0'};
    getcwd(curpath, 512);
    string path = curpath;
    const QString repoPath = QString::fromStdString(path + "/repotest");
    QString err = "";
    linglong::OstreeRepoHelper repo;
    bool ret = repo.ensureRepoEnv(repoPath, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    }
    EXPECT_EQ(ret, true);
    QVector<QString> qrepoList;
    ret = repo.getRemoteRepoList(repoPath, qrepoList, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
        EXPECT_EQ(ret, false);
        return;
    } else {
        for (auto iter = qrepoList.begin(); iter != qrepoList.end(); ++iter) {
           qInfo() << *iter;
        }
    }
    EXPECT_EQ(ret, true);
    QMap<QString, QString> outRefs;
    ret = repo.getRemoteRefs(repoPath, qrepoList[0], outRefs, err);
    if (!ret) {
        qInfo() << err;
    } else {
        // for (auto iter = outRefs.begin(); iter != outRefs.end(); ++iter) {
        //     std::cout << "ref:" << iter.key().toStdString() << ", commit value:" << iter.value().toStdString() << endl;
        //     std::cout.flush();
        // }
    }
    EXPECT_EQ(ret, true);
    QString matchRef = "";
    //QString pkgName = "org.deepin.calculator";
    QString pkgName = "us.zoom.Zoom";
    QString arch = "x86_64";
    ret = repo.queryMatchRefs(repoPath, qrepoList[0], pkgName, arch, matchRef, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    } else {
        //std::cout << matchRef.toStdString();
        qInfo() << matchRef;
    }
    EXPECT_EQ(ret, true);

    ret = repo.repoPull(repoPath, qrepoList[0], pkgName, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    }
    EXPECT_EQ(ret, true);
}

TEST(OstreeRepoHelperT06, checkOutAppData)
{
    char curpath[512] = {'\0'};
    getcwd(curpath, 512);
    string path = curpath;
    const QString repoPath = QString::fromStdString(path + "/repotest");
    // 目录如果不存在 会自动创建目录
    const QString dstPath = repoPath + "/AppData";
    QString err = "";
    QVector<QString> qrepoList;
    linglong::OstreeRepoHelper repo;

    bool ret = repo.ensureRepoEnv(repoPath, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    }

    ret = repo.getRemoteRepoList(repoPath, qrepoList, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
        EXPECT_EQ(ret, false);
        return;
    } else {
        for (auto iter = qrepoList.begin(); iter != qrepoList.end(); ++iter) {
            //std::cout << (*iter).toStdString() << endl;
           qInfo() << *iter;
        }
    }
    EXPECT_EQ(ret, true);
    QString matchRef = "";
    //QString pkgName = "org.deepin.calculator";
    QString pkgName = "us.zoom.Zoom";
    QString arch = "x86_64";
    ret = repo.queryMatchRefs(repoPath, qrepoList[0], pkgName, arch, matchRef, err);
    if (!ret) {
        qInfo() << err;
        return;
    } else {
        //std::cout << matchRef.toStdString();
        qInfo() << matchRef;
    }
    EXPECT_EQ(ret, true);

    ret = repo.checkOutAppData(repoPath, qrepoList[0], matchRef, dstPath, err);
    if (!ret) {
        qInfo() << err;
    }
}

TEST(RepoHelperT06, repoPullbyCmd)
{
    char curpath[512] = {'\0'};
    getcwd(curpath, 512);
    string path = curpath;
    const QString repoPath = QString::fromStdString(path + "/repotest");
    QString err = "";
    linglong::OstreeRepoHelper repo;
    bool ret = repo.ensureRepoEnv(repoPath, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    }
    EXPECT_EQ(ret, true);
    QVector<QString> qrepoList;
    ret = repo.getRemoteRepoList(repoPath, qrepoList, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
        EXPECT_EQ(ret, false);
        return;
    } else {
        for (auto iter = qrepoList.begin(); iter != qrepoList.end(); ++iter) {
           qInfo() << *iter;
        }
    }
    EXPECT_EQ(ret, true);
    QMap<QString, QString> outRefs;
    ret = repo.getRemoteRefs(repoPath, qrepoList[0], outRefs, err);
    if (!ret) {
        qInfo() << err;
    } else {
        // for (auto iter = outRefs.begin(); iter != outRefs.end(); ++iter) {
        //     std::cout << "ref:" << iter.key().toStdString() << ", commit value:" << iter.value().toStdString() << endl;
        //     std::cout.flush();
        // }
    }
    EXPECT_EQ(ret, true);
    QString matchRef = "";
    QString pkgName = "org.deepin.calculator";
    //QString pkgName = "us.zoom.Zoom";
    QString arch = "x86_64";
    ret = repo.queryMatchRefs(repoPath, qrepoList[0], pkgName, arch, matchRef, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    } else {
        //std::cout << matchRef.toStdString();
        qInfo() << matchRef;
    }
    EXPECT_EQ(ret, true);

    ret = repo.repoPullbyCmd(repoPath, qrepoList[0], matchRef, err);
    if (!ret) {
        //std::cout << err.toStdString();
        qInfo() << err;
    }
    EXPECT_EQ(ret, true);
}