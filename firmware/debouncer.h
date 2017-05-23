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

#ifndef CLOCK_1337__DEBOUNCER__INCLUDED
#define CLOCK_1337__DEBOUNCER__INCLUDED

/**
 * Switch debouncer.
 */
class Debouncer
{
  public:
	// Ctor
	explicit
	Debouncer (MCU::Pin, uint16_t debouce_samples);

	/**
	 * Sample pin value.
	 * Call in loop.
	 */
	void
	sample();

	/**
	 * Get debounced pin level.
	 */
	bool
	get() const;

	/**
	 * Set new number of debounce samples.
	 */
	void
	set_debounce_samples (uint16_t);

  private:
	MCU::Pin	_pin;
	uint16_t	_debounce_samples;
	uint16_t	_counter		= 0;
	bool		_debounced_value;
};


Debouncer::Debouncer (MCU::Pin pin, uint16_t debouce_samples):
	_pin (pin),
	_debounce_samples (debouce_samples),
	_debounced_value (pin.get())
{
	_pin = false;
	_pin.configure_as_input();
}


void
Debouncer::sample()
{
	bool value = _pin.get();

	if (value != _debounced_value)
	{
		if (_counter < _debounce_samples)
			_counter++;
		else
			_debounced_value = value;
	}
	else
		_counter = 0;
}


inline bool
Debouncer::get() const
{
	return _debounced_value;
}


inline void
Debouncer::set_debounce_samples (uint16_t samples)
{
	_debounce_samples = samples;
}

#endif

