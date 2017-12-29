/*
	* dstar.
	* Copyright (C) 2018 Jacopo De Luca
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
#include <argsx.h>

#include <dstar.h>
#include "Core.h"

using namespace std;

int main(int argc, char **argv) {
    Options options{};
    ax_lopt lopt[] = {{(char *) "flood",      ARGSX_NOARG,   1},
                      {(char *) "release",    ARGSX_REQ_ARG, 2},
                      {(char *) "starvation", ARGSX_NOARG,   3},
                      {(char *) "help",       ARGSX_NOARG,   'h'},
                      {(char *) "version",    ARGSX_NOARG,   'v'}};
    Core core{};
    int opt;

    if (argc < 2) {
        usage();
        return 0;
    }

    while ((opt = argsx(argc, argv, (char *) "hv", lopt, sizeof(lopt), '-')) != -1) {
        switch (opt) {
            case 1:
                options.mode |= ATKMODE_FLOOD;
                break;
            case 2:
                options.mode |= ATKMODE_RELEASE;
                break;
            case 3:
                options.mode |= ATKMODE_STARVATION;
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
            case ARGSX_BAD_OPT:
                return -1;
            case ARGSX_FEW_ARGS:
                return -1;
            default:
                options.iface = std::string(ax_arg);
                break;
        }
    }

    printWelcome();

    if (options.mode & ATKMODE_FLOOD) {

    } else if (options.mode & ATKMODE_RELEASE) {

    } else if (options.mode & ATKMODE_STARVATION) {

    }

    core.openSocket(options.iface);

    return 0;
}

void printWelcome() {
    static const std::string welcome(R"(

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
    printf("v%d.%d.%d (%s)\n\n", 1, 0, 0, "alpha");
}

void usage() {
    printf("\n%s - ", NAME);
    printf("\nUsage: %s <iface> --<flood|release|starvation>\n", NAME);
}