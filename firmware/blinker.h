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

#ifndef CLOCK_1337__BLINKER__INCLUDED
#define CLOCK_1337__BLINKER__INCLUDED

/**
 * Blinking colon and digits handler.
 */
class Blinker
{
  public:
	/**
	 * Call on each loop cycle with current time.
	 */
	void
	calibrate (Time now);

	/**
	 * Return true if colon should be lit.
	 */
	bool
	lit (uint8_t modulo);

  private:
	uint16_t	_cycles_per_second	= 0;
	uint16_t	_cycles				= 0;
	uint8_t		_prev_seconds		= 0;
};


void
Blinker::calibrate (Time now)
{
	_cycles++;

	if (now.seconds != _prev_seconds)
	{
		_cycles_per_second = _cycles;
		_cycles = 0;
		_prev_seconds = now.seconds;
	}
}


inline bool
Blinker::lit (uint8_t modulo)
{
	uint16_t k = _cycles_per_second / modulo;
	uint16_t c = _cycles / k;
	return c % 2 == 0;
}

#endif

