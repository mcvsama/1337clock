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

#ifndef CLOCK_1337__DISPLAY__INCLUDED
#define CLOCK_1337__DISPLAY__INCLUDED

/**
 * Clock display handler.
 */
class Display
{
	static constexpr MCU::Pin	_digit1			{ MCU::port_d.pin (7) };
	static constexpr MCU::Pin	_digit2			{ MCU::port_b.pin (6) };
	static constexpr MCU::Pin	_digit3			{ MCU::port_c.pin (6) };
	static constexpr MCU::Pin	_digit4			{ MCU::port_f.pin (7) };

	static constexpr MCU::Pin	_segment_a		{ MCU::port_b.pin (4) };
	static constexpr MCU::Pin	_segment_b		{ MCU::port_d.pin (6) };
	static constexpr MCU::Pin	_segment_c		{ MCU::port_f.pin (5) };
	static constexpr MCU::Pin	_segment_d		{ MCU::port_f.pin (1) };
	static constexpr MCU::Pin	_segment_e		{ MCU::port_f.pin (0) };
	static constexpr MCU::Pin	_segment_f		{ MCU::port_b.pin (5) };
	static constexpr MCU::Pin	_segment_g		{ MCU::port_f.pin (6) };
	static constexpr MCU::Pin	_segment_dp		{ MCU::port_f.pin (4) };

	static constexpr uint8_t kDigitSymbols[] = {
		0x3f, // 0 |abcdef |
		0x06, // 1 | bc    |
		0x5b, // 2 |ab de g|
		0x4f, // 3 |abcd  g|
		0x66, // 4 | bc  fg|
		0x6d, // 5 |a cd fg|
		0x7d, // 6 |a cdefg|
		0x07, // 7 |abc    |
		0x7f, // 8 |abcdefg|
		0x6f, // 9 |abcd fg|
		0x40, // - |      g|
		0x00, //   |       |
		0x6d, // S |a cd fg|
		0x79, // E |a  defg|
		0x78, // T |   defg|
		0x54, // N |  c e g|
		0x5c, // O |  cde g|
		0x50, // R |    e g|
		0x38, // L |   def |
		0x5e, // D | bcde g|
		0x73, // P |ab  efg|
		0x7c, // B |  cdefg|
		0x71, // F |a   efg|
	};

  public:
	// Special characters to be displayed:
	enum class Sign: uint8_t
	{
		Minus	= 10,
		Empty	= 11,
		S		= 12,
		E		= 13,
		T		= 14,
		N		= 15,
		O		= 16,
		R		= 17,
		L		= 18,
		D		= 19,
		P		= 20,
		B		= 21,
		F		= 22,
	};

  public:
	/**
	 * Starts with disabled display.
	 */
	Display();

	/**
	 * Set display to defaults (display off, no blinking, etc).
	 */
	void
	reset();

	/**
	 * Enable/disable display.
	 */
	void
	set_enabled (bool enabled);

	/**
	 * Set display digit to given value.
	 */
	void
	set_digit (uint8_t digit, uint8_t value);

	/**
	 * Set display digit to given value.
	 */
	void
	set_digit (uint8_t digit, Sign);

	/**
	 * Set all digits to given sign.
	 */
	template<class Value>
		void
		set_all_digits (Value);

	/**
	 * Set all digits to given value.
	 */
	template<class D1, class D2, class D3, class D4>
		void
		set_digits (D1, D2, D3, D4);

	/**
	 * Enable/disable decimal point after given digit.
	 */
	void
	set_dp (uint8_t digit, bool lit);

	/**
	 * Enable/disable all decimal points.
	 */
	void
	set_all_dps (bool lit);

	/**
	 * Alias for set_dp (1, lit);
	 */
	void
	set_colon (bool lit);

	/**
	 * Enable/disable given digit.
	 */
	void
	set_digit_enabled (uint8_t digit, bool enabled);

	/**
	 * Enable/disable all digits.
	 */
	void
	set_all_digits_enabled (bool enabled);

	/**
	 * Switch to displaying next digit on 7-segment displays.
	 * Called in loop sweeps through all displays continuously.
	 */
	void
	update();

  private:
	uint8_t	_current_digit		= 0;
	uint8_t	_digits[4]			= { 0, 0, 0, 0 };
	bool	_digit_enabled[4]	= { true, true, true, true };
	bool	_dps_lit[4]			= { false, false, false, false };
	bool	_enabled			= false;
};


constexpr uint8_t Display::kDigitSymbols[];


Display::Display()
{
	reset();

	_digit1 = false;
	_digit2 = false;
	_digit3 = false;
	_digit4 = false;
	_segment_a = false;
	_segment_b = false;
	_segment_c = false;
	_segment_d = false;
	_segment_e = false;
	_segment_f = false;
	_segment_g = false;
	_segment_dp = false;

	_digit1.configure_as_output();
	_digit2.configure_as_output();
	_digit3.configure_as_output();
	_digit4.configure_as_output();
	_segment_a.configure_as_output();
	_segment_b.configure_as_output();
	_segment_c.configure_as_output();
	_segment_d.configure_as_output();
	_segment_e.configure_as_output();
	_segment_f.configure_as_output();
	_segment_g.configure_as_output();
	_segment_dp.configure_as_output();
}


inline void
Display::reset()
{
	set_enabled (false);
	set_all_digits (Sign::Empty);
	set_all_dps (false);
	_current_digit = 0;

	for (auto& e: _digit_enabled)
		e = true;
}


inline void
Display::set_enabled (bool enabled)
{
	_enabled = enabled;
}


inline void
Display::set_digit (uint8_t digit, uint8_t value)
{
	_digits[digit] = value;
}


inline void
Display::set_digit (uint8_t digit, Sign sign)
{
	_digits[digit] = static_cast<uint8_t> (sign);
}


template<class Value>
	inline void
	Display::set_all_digits (Value value)
	{
		constexpr auto size = sizeof (_digits) / sizeof (_digits[0]);

		for (size_t i = 0; i < size; ++i)
			set_digit (i, value);
	}


template<class D1, class D2, class D3, class D4>
	inline void
	Display::set_digits (D1 d1, D2 d2, D3 d3, D4 d4)
	{
		set_digit (0, d1);
		set_digit (1, d2);
		set_digit (2, d3);
		set_digit (3, d4);
	}


inline void
Display::set_dp (uint8_t digit, bool lit)
{
	_dps_lit[digit] = lit;
}


void
Display::set_all_dps (bool lit)
{
	constexpr auto size = sizeof (_dps_lit) / sizeof (_dps_lit[0]);

	for (size_t i = 0; i < size; ++i)
		set_dp (i, lit);
}


inline void
Display::set_colon (bool lit)
{
	set_dp (1, lit);
}


inline void
Display::set_digit_enabled (uint8_t digit, bool enabled)
{
	_digit_enabled[digit] = enabled;
}


void
Display::set_all_digits_enabled (bool enabled)
{
	constexpr auto size = sizeof (_digits) / sizeof (_digits[0]);

	for (size_t i = 0; i < size; ++i)
		_digit_enabled[i] = enabled;
}


void
Display::update()
{
	_current_digit++;

	if (_current_digit > 4)
		_current_digit = 0;

	uint8_t const symbol = kDigitSymbols[_digits[_current_digit]];
	uint8_t const digit_bit = _enabled
		? (1 << _current_digit)
		: 0;

	_digit1 = false;
	_digit2 = false;
	_digit3 = false;
	_digit4 = false;

	auto const current_enabled = _digit_enabled[_current_digit];

	_segment_a = get_bit (symbol, 0) && current_enabled;
	_segment_b = get_bit (symbol, 1) && current_enabled;
	_segment_c = get_bit (symbol, 2) && current_enabled;
	_segment_d = get_bit (symbol, 3) && current_enabled;
	_segment_e = get_bit (symbol, 4) && current_enabled;
	_segment_f = get_bit (symbol, 5) && current_enabled;
	_segment_g = get_bit (symbol, 6) && current_enabled;

	_segment_dp = _dps_lit[_current_digit];

	_digit1 = get_bit (digit_bit, 0);
	_digit2 = get_bit (digit_bit, 1);
	_digit3 = get_bit (digit_bit, 2);
	_digit4 = get_bit (digit_bit, 3);
}

#endif

