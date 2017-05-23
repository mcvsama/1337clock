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

#ifndef CLOCK_1337__SWITCH__INCLUDED
#define CLOCK_1337__SWITCH__INCLUDED

/**
 * Switch press handler.
 */
class Switch
{
  public:
	// Ctor
	Switch (MCU::Pin switch_pin, uint32_t threshold_samples, uint16_t debounce_samples);

	/**
	 * Reset switch to default state and don't count anything until the switch is left unpushed.
	 */
	void
	reset_press_state();

	/**
	 * Call on each loop cycle.
	 */
	void
	sample();

	/**
	 * Return length of the press while button is pressed.
	 */
	uint32_t
	push_length() const;

	/**
	 * Report length of the current press of the button.
	 * Calling this function clears the state and the next call will return 0 until another time threshold is crossed.
	 */
	uint8_t
	report_current_press_length();

	/**
	 * Report length of the last press of the button.
	 * This will be reported only after the button has been released.
	 * Calling this function clears the state and the next call will return 0
	 * by default.
	 */
	uint8_t
	report_last_press_length();

	/**
	 * Set new number of threshold samples.
	 */
	void
	set_threshold_samples (uint32_t);

	/**
	 * Set new number of debounce samples.
	 */
	void
	set_debounce_samples (uint16_t);

  private:
	Debouncer	_debouncer;
	uint32_t	_threshold_samples;
	uint32_t	_counter					= 0;
	uint8_t		_current_press_length		= 0;
	uint8_t		_current_press_length_prev	= 0;
	uint8_t		_last_press_length			= 0;
	bool		_waiting_for_button_reset	= false;
};


Switch::Switch (MCU::Pin switch_pin, uint32_t threshold_samples, uint16_t debounce_samples):
	_debouncer (switch_pin, debounce_samples),
	_threshold_samples (threshold_samples)
{ }


void
Switch::reset_press_state()
{
	_counter = 0;
	_current_press_length = 0;
	_current_press_length_prev = 0;
	_last_press_length = 0;
	_waiting_for_button_reset = true;
}


void
Switch::sample()
{
	_debouncer.sample();

	bool pressed = !_debouncer.get();

	if (!pressed)
		_waiting_for_button_reset = false;

	if (!_waiting_for_button_reset)
	{
		if (pressed)
		{
			if (_counter < 0xffffffff)
				_counter++;

			auto const pl = push_length();

			if (pl > _current_press_length_prev)
			{
				_current_press_length_prev = pl;
				_current_press_length = pl;
			}
		}
		else
		{
			_current_press_length_prev = 0;
			_current_press_length = 0;
			_last_press_length = push_length();
			_counter = 0;
		}
	}
}


inline uint32_t
Switch::push_length() const
{
	if (_counter > 0)
		return _counter / _threshold_samples + 1;
	else
		return 0;
}


inline uint8_t
Switch::report_current_press_length()
{
	return exchange (_current_press_length, 0);
}

inline uint8_t
Switch::report_last_press_length()
{
	return exchange (_last_press_length, 0);
}


inline void
Switch::set_threshold_samples (uint32_t samples)
{
	_threshold_samples = samples;
}


inline void
Switch::set_debounce_samples (uint16_t samples)
{
	_debouncer.set_debounce_samples (samples);
}

#endif

