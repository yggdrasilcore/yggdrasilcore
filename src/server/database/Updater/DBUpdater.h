/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBUpdater_h__
#define DBUpdater_h__

#include "Config.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "Define.h"
#include "Duration.h"
#include "Log.h"
#include "QueryResult.h"
#include "StartProcess.h"
#include "StringFormat.h"
#include "UpdateFetcher.h"
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

template <class T>
class DatabaseWorkerPool;

class AC_DATABASE_API UpdateException : public std::exception
{
public:
    UpdateException(std::string const& msg) : _msg(msg) { }
    ~UpdateException() throw() { }

    char const* what() const throw() override { return _msg.c_str(); }

private:
    std::string const _msg;
};

enum BaseLocation
{
    LOCATION_REPOSITORY,
    LOCATION_DOWNLOAD
};

class AC_DATABASE_API DBUpdaterUtil
{
public:
    static std::string GetCorrectedMySQLExecutable();

    static bool CheckExecutable();

    // Locates the module that ships the SQL tree for a given database folder by
    // scanning the modules directory, instead of hardcoding a module folder name.
    static std::string FindModuleDirectory(std::string const& databaseFolder);

private:
    static std::string& corrected_path();
};

/// Per-connection metadata for module-owned databases. Core databases declare
/// explicit DBUpdater specializations below; every other connection type is
/// treated as a module database whose SQL tree is located by folder. The
/// metadata is filled in when the database is registered with a DatabaseLoader.
template <class T>
struct DBUpdaterModuleInfo
{
    static inline std::string TableName;
    static inline std::string SourceDirectory;
    static inline std::string BaseFilesDirectory;
    static inline std::string ModuleName;
};

template <class T>
class AC_DATABASE_API DBUpdater
{
public:
    using Path = std::filesystem::path;

    static inline std::string GetConfigEntry();
    static inline std::string GetTableName();
    static std::string GetSourceDirectory();
    static std::string GetBaseFilesDirectory();
    static bool IsEnabled(uint32 const updateMask);
    static BaseLocation GetBaseLocationType();
    static bool Create(DatabaseWorkerPool<T>& pool);
    static bool Update(DatabaseWorkerPool<T>& pool, std::string_view modulesList = {});
    static bool Update(DatabaseWorkerPool<T>& pool, std::vector<std::string> const* setDirectories);
    static bool Populate(DatabaseWorkerPool<T>& pool);

    // module
    static std::string GetDBModuleName();

private:
    static QueryResult Retrieve(DatabaseWorkerPool<T>& pool, std::string const& query);
    static void Apply(DatabaseWorkerPool<T>& pool, std::string const& query);
    static void ApplyFile(DatabaseWorkerPool<T>& pool, Path const& path);
    static void ApplyFile(DatabaseWorkerPool<T>& pool, std::string const& host, std::string const& user,
                          std::string const& password, std::string const& port_or_socket, std::string const& database, std::string const& ssl, Path const& path);
};

// Core database specializations (defined in DBUpdater.cpp). Declared here so the
// generic implementations below are not implicitly instantiated for them.
template<> std::string DBUpdater<LoginDatabaseConnection>::GetConfigEntry();
template<> std::string DBUpdater<LoginDatabaseConnection>::GetTableName();
template<> std::string DBUpdater<LoginDatabaseConnection>::GetSourceDirectory();
template<> std::string DBUpdater<LoginDatabaseConnection>::GetBaseFilesDirectory();
template<> bool DBUpdater<LoginDatabaseConnection>::IsEnabled(uint32 const updateMask);
template<> std::string DBUpdater<LoginDatabaseConnection>::GetDBModuleName();

template<> std::string DBUpdater<WorldDatabaseConnection>::GetConfigEntry();
template<> std::string DBUpdater<WorldDatabaseConnection>::GetTableName();
template<> std::string DBUpdater<WorldDatabaseConnection>::GetSourceDirectory();
template<> std::string DBUpdater<WorldDatabaseConnection>::GetBaseFilesDirectory();
template<> bool DBUpdater<WorldDatabaseConnection>::IsEnabled(uint32 const updateMask);
template<> std::string DBUpdater<WorldDatabaseConnection>::GetDBModuleName();

template<> std::string DBUpdater<CharacterDatabaseConnection>::GetConfigEntry();
template<> std::string DBUpdater<CharacterDatabaseConnection>::GetTableName();
template<> std::string DBUpdater<CharacterDatabaseConnection>::GetSourceDirectory();
template<> std::string DBUpdater<CharacterDatabaseConnection>::GetBaseFilesDirectory();
template<> bool DBUpdater<CharacterDatabaseConnection>::IsEnabled(uint32 const updateMask);
template<> std::string DBUpdater<CharacterDatabaseConnection>::GetDBModuleName();

// Module-owned databases: metadata comes from DBUpdaterModuleInfo<T>, populated
// by DatabaseLoader when the database is registered.
template<class T>
std::string DBUpdater<T>::GetConfigEntry()
{
    return {};
}

template<class T>
std::string DBUpdater<T>::GetTableName()
{
    return DBUpdaterModuleInfo<T>::TableName;
}

template<class T>
std::string DBUpdater<T>::GetSourceDirectory()
{
    return DBUpdaterModuleInfo<T>::SourceDirectory;
}

template<class T>
std::string DBUpdater<T>::GetBaseFilesDirectory()
{
    return DBUpdaterModuleInfo<T>::BaseFilesDirectory;
}

template<class T>
bool DBUpdater<T>::IsEnabled(uint32 const updateMask)
{
    // Core databases occupy the low bits; every other bit belongs to a module
    // database. A module database is enabled when one of those bits is set.
    constexpr uint32 coreMask = DatabaseLoader::DATABASE_LOGIN | DatabaseLoader::DATABASE_CHARACTER | DatabaseLoader::DATABASE_WORLD;
    return (updateMask & ~coreMask) != 0;
}

template<class T>
std::string DBUpdater<T>::GetDBModuleName()
{
    return DBUpdaterModuleInfo<T>::ModuleName;
}

// All
template<class T>
BaseLocation DBUpdater<T>::GetBaseLocationType()
{
    return LOCATION_REPOSITORY;
}

template<class T>
bool DBUpdater<T>::Create(DatabaseWorkerPool<T>& pool)
{
    LOG_WARN("sql.updates", "Database \"{}\" does not exist", pool.GetConnectionInfo()->database);

    const char* disableInteractive = std::getenv("AC_DISABLE_INTERACTIVE");

    if (!sConfigMgr->isDryRun() && (disableInteractive == nullptr || std::strcmp(disableInteractive, "1") != 0))
    {
        std::cout << "Do you want to create it? [yes (default) / no]:" << std::endl;
        std::string answer;
        std::getline(std::cin, answer);
        if (!answer.empty() && !(answer.substr(0, 1) == "y"))
            return false;
    }

    LOG_INFO("sql.updates", "Creating database \"{}\"...", pool.GetConnectionInfo()->database);

    // Path of temp file
    static Path const temp("create_table.sql");

    // Create temporary query to use external MySQL CLi
    std::ofstream file(temp.generic_string());
    if (!file.is_open())
    {
        LOG_FATAL("sql.updates", "Failed to create temporary query file \"{}\"!", temp.generic_string());
        return false;
    }

    file << "CREATE DATABASE `" << pool.GetConnectionInfo()->database << "` DEFAULT CHARACTER SET UTF8MB4 COLLATE utf8mb4_general_ci;\n\n";
    file.close();

    try
    {
        DBUpdater<T>::ApplyFile(pool, pool.GetConnectionInfo()->host, pool.GetConnectionInfo()->user, pool.GetConnectionInfo()->password,
                                pool.GetConnectionInfo()->port_or_socket, "", pool.GetConnectionInfo()->ssl, temp);
    }
    catch (UpdateException&)
    {
        LOG_FATAL("sql.updates", "Failed to create database {}! Does the user (named in *.conf) have `CREATE`, `ALTER`, `DROP`, `INSERT` and `DELETE` privileges on the MySQL server?", pool.GetConnectionInfo()->database);
        std::filesystem::remove(temp);
        return false;
    }

    LOG_INFO("sql.updates", "Done.");
    LOG_INFO("sql.updates", " ");
    std::filesystem::remove(temp);
    return true;
}

template<class T>
bool DBUpdater<T>::Update(DatabaseWorkerPool<T>& pool, std::string_view modulesList /*= {}*/)
{
    if (!DBUpdaterUtil::CheckExecutable())
        return false;

    LOG_INFO("sql.updates", "Updating {} database...", DBUpdater<T>::GetTableName());

    Path const sourceDirectory(DBUpdater<T>::GetSourceDirectory());

    if (!is_directory(sourceDirectory))
    {
        LOG_ERROR("sql.updates", "DBUpdater: The given source directory {} does not exist, change the path to the directory where your sql directory exists (for example c:\\source\\azerothcore). Shutting down.",
                  sourceDirectory.generic_string());
        return false;
    }

    auto CheckUpdateTable = [&](std::string const& tableName)
    {
        auto checkTable = DBUpdater<T>::Retrieve(pool, Acore::StringFormat("SHOW TABLES LIKE '{}'", tableName));
        if (!checkTable)
        {
            LOG_WARN("sql.updates", "> Table '{}' not exist! Try add based table", tableName);

            Path const temp(GetBaseFilesDirectory() + tableName + ".sql");

            try
            {
                DBUpdater<T>::ApplyFile(pool, temp);
            }
            catch (UpdateException&)
            {
                LOG_FATAL("sql.updates", "Failed apply file to database {}! Does the user (named in *.conf) have `INSERT` and `DELETE` privileges on the MySQL server?", pool.GetConnectionInfo()->database);
                return false;
            }

            return true;
        }

        return true;
    };

    if (!CheckUpdateTable("updates") || !CheckUpdateTable("updates_include"))
        return false;

    UpdateFetcher updateFetcher(sourceDirectory, [&](std::string const & query) { DBUpdater<T>::Apply(pool, query); },
    [&](Path const & file) { DBUpdater<T>::ApplyFile(pool, file); },
    [&](std::string const & query) -> QueryResult { return DBUpdater<T>::Retrieve(pool, query); }, DBUpdater<T>::GetDBModuleName(), modulesList);

    UpdateResult result;
    try
    {
        result = updateFetcher.Update(
                     sConfigMgr->GetOption<bool>("Updates.Redundancy", true),
                     sConfigMgr->GetOption<bool>("Updates.AllowRehash", true),
                     sConfigMgr->GetOption<bool>("Updates.ArchivedRedundancy", false),
                     sConfigMgr->GetOption<int32>("Updates.CleanDeadRefMaxCount", 3));
    }
    catch (UpdateException&)
    {
        return false;
    }

    std::string const info = Acore::StringFormat("Containing {} new and {} archived updates.", result.recent, result.archived);

    if (!result.updated)
        LOG_INFO("sql.updates", ">> {} database is up-to-date! {}", DBUpdater<T>::GetTableName(), info);
    else
        LOG_INFO("sql.updates", ">> Applied {} {}. {}", result.updated, result.updated == 1 ? "query" : "queries", info);

    LOG_INFO("sql.updates", " ");

    return true;
}

template<class T>
bool DBUpdater<T>::Update(DatabaseWorkerPool<T>& pool, std::vector<std::string> const* setDirectories)
{
    if (!DBUpdaterUtil::CheckExecutable())
    {
        return false;
    }

    Path const sourceDirectory(DBUpdater<T>::GetSourceDirectory());
    if (!is_directory(sourceDirectory))
    {
        return false;
    }

    auto CheckUpdateTable = [&](std::string const& tableName)
    {
        auto checkTable = DBUpdater<T>::Retrieve(pool, Acore::StringFormat("SHOW TABLES LIKE '{}'", tableName));
        if (!checkTable)
        {
            Path const temp(GetBaseFilesDirectory() + tableName + ".sql");
            try
            {
                DBUpdater<T>::ApplyFile(pool, temp);
            }
            catch (UpdateException&)
            {
                return false;
            }

            return true;
        }

        return true;
    };

    if (!CheckUpdateTable("updates") || !CheckUpdateTable("updates_include"))
    {
        return false;
    }

    UpdateFetcher updateFetcher(sourceDirectory, [&](std::string const & query) { DBUpdater<T>::Apply(pool, query); },
    [&](Path const & file) { DBUpdater<T>::ApplyFile(pool, file); },
    [&](std::string const & query) -> QueryResult { return DBUpdater<T>::Retrieve(pool, query); }, DBUpdater<T>::GetDBModuleName(), setDirectories);

    UpdateResult result;
    try
    {
        result = updateFetcher.Update(
                     sConfigMgr->GetOption<bool>("Updates.Redundancy", true),
                     sConfigMgr->GetOption<bool>("Updates.AllowRehash", true),
                     sConfigMgr->GetOption<bool>("Updates.ArchivedRedundancy", false),
                     sConfigMgr->GetOption<int32>("Updates.CleanDeadRefMaxCount", 3));
    }
    catch (UpdateException&)
    {
        return false;
    }

    return true;
}

template<class T>
bool DBUpdater<T>::Populate(DatabaseWorkerPool<T>& pool)
{
    {
        QueryResult const result = Retrieve(pool, "SHOW TABLES");
        if (result && (result->GetRowCount() > 0))
            return true;
    }

    if (!DBUpdaterUtil::CheckExecutable())
        return false;

    LOG_INFO("sql.updates", "Database {} is empty, auto populating it...", DBUpdater<T>::GetTableName());

    std::string const DirPathStr = DBUpdater<T>::GetBaseFilesDirectory();

    Path const DirPath(DirPathStr);
    if (!std::filesystem::is_directory(DirPath))
    {
        LOG_ERROR("sql.updates", ">> Directory \"{}\" not exist", DirPath.generic_string());
        return false;
    }

    if (DirPath.empty())
    {
        LOG_ERROR("sql.updates", ">> Directory \"{}\" is empty", DirPath.generic_string());
        return false;
    }

    std::filesystem::directory_iterator const DirItr;
    uint32 FilesCount = 0;

    for (std::filesystem::directory_iterator itr(DirPath); itr != DirItr; ++itr)
    {
        if (itr->path().extension() == ".sql")
            FilesCount++;
    }

    if (!FilesCount)
    {
        LOG_ERROR("sql.updates", ">> In directory \"{}\" not exist '*.sql' files", DirPath.generic_string());
        return false;
    }

    for (std::filesystem::directory_iterator itr(DirPath); itr != DirItr; ++itr)
    {
        if (itr->path().extension() != ".sql")
            continue;

        LOG_INFO("sql.updates", ">> Applying \'{}\'...", itr->path().filename().generic_string());

        try
        {
            ApplyFile(pool, itr->path());
        }
        catch (UpdateException&)
        {
            return false;
        }
    }

    LOG_INFO("sql.updates", ">> Done!");
    LOG_INFO("sql.updates", " ");
    return true;
}

template<class T>
QueryResult DBUpdater<T>::Retrieve(DatabaseWorkerPool<T>& pool, std::string const& query)
{
    return pool.Query(query.c_str());
}

template<class T>
void DBUpdater<T>::Apply(DatabaseWorkerPool<T>& pool, std::string const& query)
{
    pool.DirectExecute(query.c_str());
}

template<class T>
void DBUpdater<T>::ApplyFile(DatabaseWorkerPool<T>& pool, Path const& path)
{
    DBUpdater<T>::ApplyFile(pool, pool.GetConnectionInfo()->host, pool.GetConnectionInfo()->user, pool.GetConnectionInfo()->password,
                            pool.GetConnectionInfo()->port_or_socket, pool.GetConnectionInfo()->database, pool.GetConnectionInfo()->ssl, path);
}

template<class T>
void DBUpdater<T>::ApplyFile(DatabaseWorkerPool<T>& pool, std::string const& host, std::string const& user,
                             std::string const& password, std::string const& port_or_socket, std::string const& database, std::string const& ssl, Path const& path)
{
    std::string configTempDir = sConfigMgr->GetOption<std::string>("TempDir", "");

    auto tempDir = configTempDir.empty() ? std::filesystem::temp_directory_path().string() : configTempDir;

    tempDir = Acore::String::AddSuffixIfNotExists(tempDir, std::filesystem::path::preferred_separator);

    std::string confFileName = "mysql_ac.conf";

    std::ofstream outfile (tempDir + confFileName);

    outfile << "[client]\npassword = \"" << password << '"' << std::endl;

    outfile.close();

    std::vector<std::string> args;
    args.reserve(9);

    args.emplace_back("--defaults-extra-file="+tempDir + confFileName+"");

    // CLI Client connection info
    args.emplace_back("-h" + host);
    args.emplace_back("-u" + user);

    // Check if we want to connect through ip or socket (Unix only)
#ifdef _WIN32

    if (host == ".")
        args.emplace_back("--protocol=PIPE");
    else
        args.emplace_back("-P" + port_or_socket);

#else

    if (!std::isdigit(port_or_socket[0]))
    {
        // We can't check if host == "." here, because it is named localhost if socket option is enabled
        args.emplace_back("-P0");
        args.emplace_back("--protocol=SOCKET");
        args.emplace_back("-S" + port_or_socket);
    }
    else
        // generic case
        args.emplace_back("-P" + port_or_socket);

#endif

    // Set the default charset to utf8
    args.emplace_back("--default-character-set=utf8");

    // Set max allowed packet to 1 GB
    args.emplace_back("--max-allowed-packet=1GB");

    if (ssl == "ssl")
        args.emplace_back("--ssl-mode=REQUIRED");

    // Database
    if (!database.empty())
        args.emplace_back(database);

    // Invokes a mysql process which doesn't leak credentials to logs
    int const ret = Acore::StartProcess(DBUpdaterUtil::GetCorrectedMySQLExecutable(), args,
        "sql.updates", path.generic_string(), true);

    if (ret != EXIT_SUCCESS)
    {
        LOG_FATAL("sql.updates", "Applying of file \'{}\' to database \'{}\' failed!" \
            " If you are a user, please pull the latest revision from the repository. "
            "Also make sure you have not applied any of the databases with your sql client. "
            "You cannot use auto-update system and import sql files from AzerothCore repository with your sql client. "
            "If you are a developer, please fix your sql query.",
            path.generic_string(), pool.GetConnectionInfo()->database);

        if (!sConfigMgr->isDryRun())
        {
            if (uint32 delay = sConfigMgr->GetOption<uint32>("Updates.ExceptionShutdownDelay", 10000))
                std::this_thread::sleep_for(Milliseconds(delay));

            throw UpdateException("update failed");
        }
    }
}

#endif // DBUpdater_h__
