/*
pneuCNTRlbox Firmware.
copyright hnnz 2023

GPL-3.0-or-later

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.


*/
#include "PneuCNTRL.h"

struct Inflatable {
    int safety_Stop;
    int min_pressure;
    int max_pressure;
    int inflation_inertia;
    int deflation_inertia;
};

Inflatable Baguette={

    19000,
    Baguette.min_pressure = 15500;
    Baguette.max_pressure = 18750;
    Baguette.inflation_inertia = 20;
    Baguette.deflation_inertia = 10;
}
