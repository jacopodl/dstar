/*
	* dstar
	* Copyright (C) 2017 - 2018 Jacopo De Luca
	*
	* This program is free software: you can redistribute it and/or modify
	* it under the terms of the GNU General Public License as published by
	* the Free Software Foundation, either version 3 of the License, or
	* (at your option) any later version.
	* This program is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	* GNU General Public License for more details.
	* You should have received a copy of the GNU General Public License
	* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <unistd.h>

#include <argsx.h>
#include <dstar.h>
#include <csignal>

#include <Options.h>
#include <Core.h>

using namespace std;

Core core{};

int main(int argc, char **argv) {
    ax_lopt lopt[] = {{(char *) "flood",      ARGSX_NOARG,   1},
                      {(char *) "starvation", ARGSX_NOARG,   2},
                      {(char *) "no-release", ARGSX_NOARG,   3},
                      {(char *) "help",       ARGSX_NOARG,   'h'},
                      {(char *) "version",    ARGSX_NOARG,   'v'},
                      {(char *) "bcast",      ARGSX_NOARG,   'b'},
                      {(char *) "server",     ARGSX_NOARG,   's'},
                      {(char *) "gateway",    ARGSX_REQ_ARG, 'g'},
                      {(char *) "lease",      ARGSX_NOARG,   'l'},
                      {(char *) "dns",        ARGSX_REQ_ARG, 'd'}};
    int opt;

    if (argc < 2) {
        usage();
        return 0;
    }

    while ((opt = argsx(argc, argv, (char *) "hvbsg!ld!", lopt, sizeof(lopt), '-')) != -1) {
        switch (opt) {
            case 1:
                core.options.mode |= ATKMODE_FLOOD;
                break;
            case 2:
                core.options.mode |= ATKMODE_STARVATION;
                break;
            case 3:
                core.options.releaseOnExit = false;
                break;
            case 'h':
                usage();
                return 0;
            case 'v':
                printf("\n%s (%s) v%d.%d.%d\n", NAME, VERSION_TYPE, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
#ifdef __GNUC__
                printf("Compiled with G++ v%d.%d.%d - Date: %s - %s\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__,
                       __DATE__, __TIME__);
#endif
                return 0;
            case 'b':
                core.options.serverFlags |= DHCP_FLAGS_BROADCAST;
                break;
            case 's':
                core.options.enableServer = true;
                break;
            case 'g':
                if (!ip_parse_addr(ax_arg, &core.options.gateway)) {
                    cerr << "Invalid gateway address!\n";
                    return -1;
                }
                break;
            case 'l':
                core.options.lease = (unsigned short) strtoul(ax_arg, nullptr, 10);
                break;
            case 'd':
                if (!ip_parse_addr(ax_arg, &core.options.primaryDns)) {
                    cerr << "Invalid DNS address!\n";
                    return -1;
                }
                break;
            case ARGSX_BAD_OPT:
                return -1;
            case ARGSX_FEW_ARGS:
                return -1;
            default:
                core.options.iface = string(ax_arg);
                break;
        }
    }

    if (getuid()) {
        cerr << "Required elevated privileges!\n";
        return -1;
    }

    if (core.registerAction() != 0) {
        printWelcome();
        if (core.options.enableServer)
            cout << "[+] DHCP server\n";
        else
            cout << "[-] DHCP server\n";
        signal(SIGINT, sigHandler);
        core.openSocket();
    } else
        printf("Nothing to do here! Zzzz\n"
                       "Psss, you can try with %s --help ;)\n", NAME);

    return 0;
}

void printWelcome() {
    static const string welcome(R"(

@@@@@@@    @@@@@@   @@@@@@@   @@@@@@   @@@@@@@
@@@@@@@@  @@@@@@@   @@@@@@@  @@@@@@@@  @@@@@@@@
@@!  @@@  !@@         @@!    @@!  @@@  @@!  @@@
!@!  @!@  !@!         !@!    !@!  @!@  !@!  @!@
@!@  !@!  !!@@!!      @!!    @!@!@!@!  @!@!!@!
!@!  !!!   !!@!!!     !!!    !!!@!!!!  !!@!@!
!!:  !!!       !:!    !!:    !!:  !!!  !!: :!!
:!:  !:!      !:!     :!:    :!:  !:!  :!:  !:!
 :::: ::  :::: ::      ::    ::   :::  ::   :::
:: :  :   :: : :       :      :   : :   :   : :
                                                 )");
    cout << welcome;
    printf("v%d.%d.%d (%s)\n\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TYPE);
}

void usage() {
    printf("\n%s - v%d.%d.%d (%s)"
                   "\nUsage: %s <iface> --<flood|starvation>\n"
                   "\t-h, --help   \tPrint this help and exit.\n"
                   "\t-v, --version\tPrint version and exit.\n"
                   "\t-b, --bcast  \tUse broadcast reply.\n"
                   "\t-s, --server \tEnable dhcp rogue server.\n"
                   "\t-l, --lease  \tSet lease time.\n"
                   "\t-d, --dns    \tSet primary DNS address.\n"
                   "\t-g, --gateway\tSet gateway address.\n"
                   "\t--no-release \tDo not release stolen addresses on exit.\n",
           NAME,
           VERSION_MAJOR,
           VERSION_MINOR,
           VERSION_PATCH,
           VERSION_TYPE,
           NAME);
}

void sigHandler(int signum) {
    cout << "Stopping..." << endl;
    core.options.stop = true;
}