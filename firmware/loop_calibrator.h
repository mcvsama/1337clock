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

#ifndef CLOCK_1337__LOOP_CALIBRATOR__INCLUDED
#define CLOCK_1337__LOOP_CALIBRATOR__INCLUDED

/**
 * Computes number of loop cycles per 1 second.
 */
class LoopCalibrator
{
  public:
	/**
	 * Reset to default - uncalibrated state.
	 */
	void
	reset();

	/**
	 * Call on each loop cycle with current time.
	 */
	void
	calibrate (Time now);

	/**
	 * Return true if loop is calibrated.
	 */
	bool
	calibrated() const;

	/**
	 * Return computed number of cycles per second.
	 */
	uint32_t
	cycles_per_second() const;

	/**
	 * Blinker function - return true on odd subcycles of a loop cycle.
	 * Number of subcycles is determined by the modulo argument.
	 */
	bool
	lit (uint8_t modulo) const;

  private:
	uint32_t	_cycles_per_second	= 0;
	uint32_t	_cycles				= 0;
	uint8_t		_prev_seconds		= 0;
};


void
LoopCalibrator::reset()
{
	_cycles_per_second = 0;
	_cycles = 0;
	_prev_seconds = 0;
}


void
LoopCalibrator::calibrate (Time now)
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
LoopCalibrator::calibrated() const
{
	return _cycles_per_second > 0;
}


inline uint32_t
LoopCalibrator::cycles_per_second() const
{
	return _cycles_per_second;
}


inline bool
LoopCalibrator::lit (uint8_t modulo) const
{
	uint32_t k = _cycles_per_second / modulo;
	uint32_t c = _cycles / k;
	return c % 2 == 0;
}

#endif

