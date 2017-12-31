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

#ifndef DSTAR_H
#define DSTAR_H

#define NAME "dstar"

#define VERSION_TYPE "alpha"
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

#define ATKMODE_FLOOD       (1<<0)
#define ATKMODE_RELEASE     (1<<1)
#define ATKMODE_STARVATION  (1<<2)
#define ATKMODE_ROGUESERVER (1<<3)

struct Options {
    std::string iface;
    int mode = 0;
};

void printWelcome();

void usage();

void sigHandler(int signum);

#endif //DSTAR_H
