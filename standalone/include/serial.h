/* -*- Mode: C -*- */
/*
 * Serial interface.
 *
 * Copyright (C) 2009-2012, Julian Stecklina <jsteckli@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of Bender.
 *
 * Bender is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Bender is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#pragma once

#include <stdint.h>

/// Initialize COM1 as serial output.
void serial_init(void);

/// Initialize the serial port at the given base port.
void serial_init_port(uint16_t base_port);

/// Send a serial character to the serial port configured with one of the serial_init functions above.
void serial_send(int c);

/// Disable serial output.
void serial_disable(void);
