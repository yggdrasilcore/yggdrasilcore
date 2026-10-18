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

#include "DBUpdater.h"
#include "BuiltInConfig.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "Log.h"
#include "StartProcess.h"
#include <filesystem>

std::string DBUpdaterUtil::GetCorrectedMySQLExecutable()
{
    if (!corrected_path().empty())
        return corrected_path();
    else
        return BuiltInConfig::GetMySQLExecutable();
}

bool DBUpdaterUtil::CheckExecutable()
{
    std::filesystem::path exe(GetCorrectedMySQLExecutable());
    if (!is_regular_file(exe))
    {
        exe = Acore::SearchExecutableInPath("mysql");
        if (!exe.empty() && is_regular_file(exe))
        {
            // Correct the path to the cli
            corrected_path() = absolute(exe).generic_string();
            return true;
        }

        LOG_FATAL("sql.updates", "Didn't find any executable MySQL binary at \'{}\' or in path, correct the path in the *.conf (\"MySQLExecutable\").",
            absolute(exe).generic_string());

        return false;
    }
    return true;
}

std::string& DBUpdaterUtil::corrected_path()
{
    static std::string path;
    return path;
}

std::string DBUpdaterUtil::FindModuleDirectory(std::string const& databaseFolder)
{
    std::filesystem::path const modulesDirectory = std::filesystem::path(BuiltInConfig::GetSourceDirectory()) / "modules";
    std::filesystem::path const databaseDirectory = std::filesystem::path("data") / "sql" / databaseFolder;

    if (std::filesystem::is_directory(modulesDirectory))
    {
        std::filesystem::directory_iterator const end;
        for (std::filesystem::directory_iterator itr(modulesDirectory); itr != end; ++itr)
        {
            if (std::filesystem::is_directory(itr->path()) && std::filesystem::is_directory(itr->path() / databaseDirectory))
            {
                return itr->path().generic_string();
            }
        }
    }

    return std::string();
}

// Auth Database
template<>
std::string DBUpdater<LoginDatabaseConnection>::GetConfigEntry()
{
    return "Updates.Auth";
}

template<>
std::string DBUpdater<LoginDatabaseConnection>::GetTableName()
{
    return "Auth";
}

template<>
std::string DBUpdater<LoginDatabaseConnection>::GetSourceDirectory()
{
    return BuiltInConfig::GetSourceDirectory();
}

template<>
std::string DBUpdater<LoginDatabaseConnection>::GetBaseFilesDirectory()
{
    return DBUpdater<LoginDatabaseConnection>::GetSourceDirectory() + "/data/sql/base/db_auth/";
}

template<>
bool DBUpdater<LoginDatabaseConnection>::IsEnabled(uint32 const updateMask)
{
    // This way silences warnings under msvc
    return (updateMask & DatabaseLoader::DATABASE_LOGIN) ? true : false;
}

template<>
std::string DBUpdater<LoginDatabaseConnection>::GetDBModuleName()
{
    return "auth";
}

// World Database
template<>
std::string DBUpdater<WorldDatabaseConnection>::GetConfigEntry()
{
    return "Updates.World";
}

template<>
std::string DBUpdater<WorldDatabaseConnection>::GetTableName()
{
    return "World";
}

template<>
std::string DBUpdater<WorldDatabaseConnection>::GetSourceDirectory()
{
    return BuiltInConfig::GetSourceDirectory();
}

template<>
std::string DBUpdater<WorldDatabaseConnection>::GetBaseFilesDirectory()
{
    return DBUpdater<WorldDatabaseConnection>::GetSourceDirectory() + "/data/sql/base/db_world/";
}

template<>
bool DBUpdater<WorldDatabaseConnection>::IsEnabled(uint32 const updateMask)
{
    // This way silences warnings under msvc
    return (updateMask & DatabaseLoader::DATABASE_WORLD) ? true : false;
}

template<>
std::string DBUpdater<WorldDatabaseConnection>::GetDBModuleName()
{
    return "world";
}

// Character Database
template<>
std::string DBUpdater<CharacterDatabaseConnection>::GetConfigEntry()
{
    return "Updates.Character";
}

template<>
std::string DBUpdater<CharacterDatabaseConnection>::GetTableName()
{
    return "Character";
}

template<>
std::string DBUpdater<CharacterDatabaseConnection>::GetSourceDirectory()
{
    return BuiltInConfig::GetSourceDirectory();
}

template<>
std::string DBUpdater<CharacterDatabaseConnection>::GetBaseFilesDirectory()
{
    return DBUpdater<CharacterDatabaseConnection>::GetSourceDirectory() + "/data/sql/base/db_characters/";
}

template<>
bool DBUpdater<CharacterDatabaseConnection>::IsEnabled(uint32 const updateMask)
{
    // This way silences warnings under msvc
    return (updateMask & DatabaseLoader::DATABASE_CHARACTER) ? true : false;
}

template<>
std::string DBUpdater<CharacterDatabaseConnection>::GetDBModuleName()
{
    return "characters";
}

template class AC_DATABASE_API DBUpdater<LoginDatabaseConnection>;
template class AC_DATABASE_API DBUpdater<WorldDatabaseConnection>;
template class AC_DATABASE_API DBUpdater<CharacterDatabaseConnection>;
