/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef CLOCK_1337__MCU__INCLUDED
#define CLOCK_1337__MCU__INCLUDED

// Mulabs:
#include <mulabs_avr/mcu/atmega32-u4.h>

using namespace mulabs::avr;


class MCU: public ATMega32U4
{
  public:
	static void
	initialize();
};


void
MCU::initialize()
{
	JTAG::disable();
	// Give time for initialization of all devices:
	sleep_ms<1000>();
}

#endif

