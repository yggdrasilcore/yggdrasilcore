/*
 * This file is part of the YggdrasilCore Project. See AUTHORS file for Copyright information
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

#include "Banner.h"
#include "GitRevision.h"
#include "StringFormat.h"

void Ycore::Banner::Show(std::string_view applicationName, void(*log)(std::string_view text), void(*logExtraInfo)())
{
    log(Ycore::StringFormat("{} ({})", GitRevision::GetFullVersion(), applicationName));
    log("<Ctrl-C> to stop.\n");
    log("          _______  _______  ______   _______  _______  _______ _________ _");
    log("|\\     /|(  ____ \\(  ____ \\(  __  \\ (  ____ )(  ___  )(  ____ \\\\__   __/( \\");
    log("( \\   / )| (    \\/| (    \\/| (  \\  )| (    )|| (   ) || (    \\/   ) (   | (");
    log(" \\ (_) / | |      | |      | |   ) || (____)|| (___) || (_____    | |   | |");
    log("  \\   /  | | ____ | | ____ | |   | ||     __)|  ___  |(_____  )   | |   | |");
    log("   ) (   | | \\_  )| | \\_  )| |   ) || (\\ (   | (   ) |      ) |   | |   | |");
    log("   | |   | (___) || (___) || (__/  )| ) \\ \\__| )   ( |/\\____) |___) (___| (____/\\");
    log("   \\_/   (_______)(_______)(______/ |/   \\__/|/     \\|\\_______)\\_______/(_______/");
    log("");
    log(" _______  _______  _______  _______");
    log("(  ____ \\(  ___  )(  ____ )(  ____ \\");
    log("| (    \\/| (   ) || (    )|| (    \\/");
    log("| |      | |   | || (____)|| (__");
    log("| |      | |   | ||     __)|  __)");
    log("| |      | |   | || (\\ (   | (");
    log("| (____/\\| (___) || ) \\ \\__| (____/\\");
    log("(_______/(_______)|/   \\__/(_______/");
    log("");
    log("     YggdrasilCore 3.3.5a  -  yggdrasilcore.github.io\n");

    if (logExtraInfo)
    {
        logExtraInfo();
    }

    log(" ");
}
