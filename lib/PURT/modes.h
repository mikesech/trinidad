/* Copyright 2009-2011 Michael Sechooler
 *
 * This file is part of PURT.
 * 
 * PURT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * PURT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with PURT.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Defines the modes that PURT can operate in. Source files should include  *
 * this file and define PURT_MODE to the applicable mode before including   *
 * other files.																*/

#ifndef PURT_MODES_H
#define PURT_MODES_H

#define PURT_RAW		1
#define PURT_SD_DIRECT	2
#define PURT_SD_BUFFER	4

//Predicates
#define PURT_IS_RAW() (PURT_MODE & PURT_RAW)
#define PURT_IS_SD_DIRECT() (PURT_MODE & PURT_SD_DIRECT)
#define PURT_IS_SD_BUFFER() (PURT_MODE & PURT_SD_BUFFER)
#define PURT_IS_SD()  (PURT_MODE & (PURT_SD_DIRECT | PURT_SD_BUFFER))

#endif
