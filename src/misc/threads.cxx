// ----------------------------------------------------------------------------
// threads.cxx
//
// Copyright (C) 2007-2018
//		Stelios Bounanos, M0GLD
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "threads.h"

/// This ensures that a mutex is always unlocked when leaving a function or block.

// Change to 1 to observe guard lock/unlock processing on stdout
#define DEBUG_GUARD_LOCK 0

guard_lock::guard_lock(pthread_mutex_t* m) : mutex(m) {
	pthread_mutex_lock(mutex);
}

guard_lock::~guard_lock(void) {
	pthread_mutex_unlock(mutex);
}

