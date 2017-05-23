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

#ifndef CLOCK_1337__CLOCK__INCLUDED
#define CLOCK_1337__CLOCK__INCLUDED

// Mulabs:
#include <mulabs_avr/utility/numeric.h>


using namespace mulabs::avr;


/**
 * Main clock class.
 */
class Clock
{
	static constexpr uint16_t	kClickSoundMs			{ 4 };
	static constexpr uint16_t	kShortBeepMs			{ 1000 / 10 };
	static constexpr uint16_t	kLongBeepMs				{ 400 };
	static constexpr uint32_t	kBouncingTimeMs			{ 5 };
	static constexpr uint32_t	kButtonThresholdMs		{ 1000 };
	static constexpr Time		kMagicTimeOfDay			{ 13, 37, 00 };
	static constexpr uint8_t	kChangePrecisionPushLen	{ 1 };
	static constexpr uint8_t	kNumberUpPushLen		{ 1 };
	static constexpr uint8_t	kNextPushLen			{ 2 };
	static constexpr uint8_t	kChangeModePushLen		{ 2 };
	static constexpr uint8_t	kChangeBeepSettings		{ 3 };
	static constexpr uint8_t	kEnterSetupPushLength	{ 4 };

	static constexpr MCU::Pin	_buzzer					{ MCU::port_b.pin (0) };
	static constexpr MCU::Pin	_trigger_out			{ MCU::port_e.pin (6) };
	static constexpr MCU::Pin	_switch_pin				{ MCU::port_d.pin (4) };

	enum class ClockMode
	{
		DisplayClock,
		BeepSetup,
		TimeSetup,
	};

	enum class DisplayMode
	{
		// Display count-down clock to 13:37:
		Leet,
		// Display normal clock:
		Normal,
	};

	enum class DisplayPrecision
	{
		// HH:MM:
		HoursMinutes,
		// :SS:
		Seconds,
	};

	enum class DisplayOverride
	{
		None,
		Norm,
		Leet,
		Beep,
		Set,
	};

	enum class SetupDigit
	{
		Hours10,
		Hours1,
		Minutes10,
		Minutes1,
	};

  public:
	// Ctor
	Clock();

	void
	loop();

  private:
	void
	request_beep (uint16_t milliseconds);

	void
	handle_buzzer();

	void
	handle_button();

	void
	update_display();

	void
	print_clocks();

	void
	print_time (Time const&, DisplayPrecision);

	bool
	is_alarm() const;

  private:
	ClockMode			_clock_mode			{ ClockMode::DisplayClock };
	DisplayMode			_display_mode		{ DisplayMode::Leet };
	DisplayPrecision	_display_precision	{ DisplayPrecision::HoursMinutes };
	DisplayOverride		_display_override	{ DisplayOverride::None };
	LoopCalibrator		_calibrator;
	RTC					_rtc;
	Switch				_switch				{ _switch_pin, 1000, 1000 };
	Display				_display;
	Time				_time;
	// Related to time setup:
	Time				_setup_time;
	SetupDigit			_setup_digit		{ SetupDigit::Hours10 };
	uint16_t			_requested_beep		{ 0 };
	Time				_last_beep_time;
	bool				_beeper_enabled		{ true };
};


constexpr MCU::Pin Clock::_switch_pin;


Clock::Clock()
{
	_buzzer = false;
	_trigger_out = false;

	_buzzer.configure_as_output();
	_trigger_out.configure_as_output();
}


void
Clock::loop()
{
	_display.set_enabled (true);

	while (true)
	{
		_time = _rtc.get_time();

		// The trigger-out will last for minute:
		_trigger_out = _clock_mode == ClockMode::DisplayClock && is_alarm();

		handle_buzzer();
		handle_button();
		update_display();

		_calibrator.calibrate (_time);
	}
}


void
Clock::request_beep (uint16_t milliseconds)
{
	if (_beeper_enabled && _calibrator.calibrated())
	{
		auto len = static_cast<float> (milliseconds) * _calibrator.cycles_per_second() / 1000L;

		if (len > _requested_beep)
			_requested_beep = len;
	}
}


void
Clock::handle_buzzer()
{
	_buzzer = _requested_beep > 0;

	if (_requested_beep > 0)
		_requested_beep--;
}


void
Clock::handle_button()
{
	auto const n = _calibrator.cycles_per_second();

	_switch.set_debounce_samples (n / (1000L / kBouncingTimeMs));
	_switch.set_threshold_samples (n / (1000L / kButtonThresholdMs));
	_switch.sample();

	auto const push_length = _switch.push_length();

	if (_clock_mode == ClockMode::DisplayClock)
	{
		if (push_length > 0)
		{
			if (push_length == kChangeModePushLen)
			{
				switch (_display_mode)
				{
					case DisplayMode::Leet:
						_display_override = DisplayOverride::Norm;
						break;

					case DisplayMode::Normal:
						_display_override = DisplayOverride::Leet;
						break;
				}
			}
			else if (push_length == kChangeBeepSettings)
				_display_override = DisplayOverride::Beep;
			else if (push_length >= kEnterSetupPushLength)
				_display_override = DisplayOverride::Set;
		}
		else
			_display_override = DisplayOverride::None;
	}

	auto const last_press_length = _switch.report_last_press_length();
	auto const current_press_length = _switch.report_current_press_length();

	if (_clock_mode == ClockMode::DisplayClock)
	{
		if (last_press_length == kChangePrecisionPushLen)
		{
			switch (_display_precision)
			{
				case DisplayPrecision::HoursMinutes:
					_display_precision = DisplayPrecision::Seconds;
					break;

				case DisplayPrecision::Seconds:
					_display_precision = DisplayPrecision::HoursMinutes;
					break;
			}
		}
		else if (last_press_length == kChangeModePushLen)
		{
			switch (_display_mode)
			{
				case DisplayMode::Leet:
					_display_mode = DisplayMode::Normal;
					break;

				case DisplayMode::Normal:
					_display_mode = DisplayMode::Leet;
					break;
			}
		}
		else if (last_press_length == kChangeBeepSettings)
		{
			_clock_mode = ClockMode::BeepSetup;
		}
		else if (last_press_length >= kEnterSetupPushLength)
		{
			_clock_mode = ClockMode::TimeSetup;
			_setup_time = _time;
			_setup_digit = SetupDigit::Hours10;
		}
	}
	else if (_clock_mode == ClockMode::BeepSetup)
	{
		if (last_press_length == 1)
			_beeper_enabled = !_beeper_enabled;
		else if (current_press_length > 1)
		{
			_clock_mode = ClockMode::DisplayClock;
			_switch.reset_press_state();
		}
	}
	else if (_clock_mode == ClockMode::TimeSetup)
	{
		if (last_press_length == kNumberUpPushLen)
		{
			switch (_setup_digit)
			{
				case SetupDigit::Hours10:
				{
					auto h = _setup_time.hours / 10;
					h = (h + 1) % 3;
					_setup_time.hours = 10 * h + _setup_time.hours % 10;
					break;
				}

				case SetupDigit::Hours1:
				{
					auto const h10 = _setup_time.hours / 10;
					auto h = _setup_time.hours % 10;

					if (h10 == 2)
						h = (h + 1) % 4;
					else
						h = (h + 1) % 10;

					_setup_time.hours = _setup_time.hours / 10 * 10 + h;
					break;
				}

				case SetupDigit::Minutes10:
				{
					auto m = _setup_time.minutes / 10;
					m = (m + 1) % 6;
					_setup_time.minutes = 10 * m + _setup_time.minutes % 10;
					break;
				}

				case SetupDigit::Minutes1:
				{
					auto m = _setup_time.minutes % 10;
					m = (m + 1) % 10;
					_setup_time.minutes = _setup_time.minutes / 10 * 10 + m;
					break;
				}
			}
		}
		else if (last_press_length >= kNextPushLen)
		{
			if (_setup_digit == SetupDigit::Minutes1)
			{
				_setup_time.seconds = 0;
				_setup_time.sanitize();
				_rtc.set_time (_setup_time);
				_calibrator.reset();
				_switch.reset_press_state();
				_clock_mode = ClockMode::DisplayClock;
			}
		}
		// Checking push_length() instead of last_press_length to switch to next digit
		// as soon as pressing time is enough (ie. not waiting for button release).
		else if (current_press_length >= kNextPushLen)
		{
			if (_setup_digit != SetupDigit::Minutes1)
			{
				_setup_time.sanitize();
				// Clear last-press-length buffer:
				_switch.reset_press_state();
			}

			switch (_setup_digit)
			{
				case SetupDigit::Hours10:
					_setup_digit = SetupDigit::Hours1;
					break;

				case SetupDigit::Hours1:
					_setup_digit = SetupDigit::Minutes10;
					break;

				case SetupDigit::Minutes10:
					_setup_digit = SetupDigit::Minutes1;
					break;

				case SetupDigit::Minutes1:
					break;
			}
		}
	}
}


void
Clock::update_display()
{
	switch (_display_override)
	{
		case DisplayOverride::None:
			print_clocks();
			break;

		case DisplayOverride::Norm:
			_display.set_all_digits_enabled (true);
			_display.set_digits (Display::Sign::N, Display::Sign::O, Display::Sign::R, Display::Sign::Empty);
			_display.set_all_dps (false);
			break;

		case DisplayOverride::Leet:
			_display.set_all_digits_enabled (true);
			_display.set_digits (Display::Sign::L, Display::Sign::E, Display::Sign::E, Display::Sign::T);
			_display.set_all_dps (false);
			break;

		case DisplayOverride::Beep:
			_display.set_all_digits_enabled (true);
			_display.set_digits (Display::Sign::B, Display::Sign::E, Display::Sign::E, Display::Sign::P);
			_display.set_all_dps (false);
			break;

		case DisplayOverride::Set:
			_display.set_all_digits_enabled (true);
			_display.set_digits (Display::Sign::S, Display::Sign::E, Display::Sign::T, Display::Sign::Empty);
			_display.set_all_dps (false);
			break;
	}

	_display.update();
}


void
Clock::print_clocks()
{
	switch (_clock_mode)
	{
		case ClockMode::DisplayClock:
		{
			int32_t const total_now = _time.seconds_since_midnight();
			int32_t const total_magic = kMagicTimeOfDay.seconds_since_midnight();
			int32_t const diff = total_magic - total_now;
			int32_t const left_secs = (diff + 86400L - 1) % 86400L;
			int32_t const right_secs = (-diff + 86400L) % 86400L;

			// Last dot is lit, when Leet mode is active:
			_display.set_dp (3, _display_mode == DisplayMode::Leet);

			switch (_display_mode)
			{
				case DisplayMode::Leet:
				{
					Time left_time;

					left_time.hours = left_secs / 3600L;
					left_time.minutes = (left_secs / 60L) % 60L;
					left_time.seconds = left_secs % 60L;

					if (left_secs < 10 * 60)
					{
						// If 10 minutes left, display mm:ss:
						_display.set_digit (3, left_time.seconds % 10);
						_display.set_digit (2, left_time.seconds / 10);
						_display.set_digit (1, left_time.minutes % 10);

						if (left_time.minutes / 10 == 0)
							_display.set_digit (0, Display::Sign::Minus);
						else
							_display.set_digit (0, left_time.minutes / 10);

						// Make colon blink faster:
						constexpr int32_t kFastColonThresholdTime = 20;

						if (left_secs > kFastColonThresholdTime)
							_display.set_colon (_calibrator.lit (2));
						else
						{
							int32_t mod = (kFastColonThresholdTime - left_secs) / 2 * 2;
							_display.set_colon (_calibrator.lit (mod + 2));
						}

						// Beep on T-60 and T-30 s marks:
						if (left_secs == 60 || left_secs == 30)
						{
							if (_time != _last_beep_time)
							{
								request_beep (kClickSoundMs);
								_last_beep_time = _time;
							}
						}
						// Longer beeps on T-5…T-1 s:
						else if (0 <= left_secs && left_secs < 5)
						{
							if (_time != _last_beep_time)
							{
								request_beep (kShortBeepMs);
								_last_beep_time = _time;
							}
						}
					}
					else if (right_secs < 60)
					{
						// Longer beep at 13:37:
						if (right_secs == 0)
						{
							if (_time != _last_beep_time)
							{
								request_beep (kLongBeepMs);
								_last_beep_time = _time;
							}
						}

						// BLINKING 13:37
						bool blink = _calibrator.lit (4);

						if (blink)
							_display.set_digits (1, 3, 3, 7);
						else
							_display.set_all_digits (Display::Sign::Empty);

						_display.set_colon (blink);
					}
					else
					{
						print_time (left_time, _display_precision);
						_display.set_colon (_calibrator.lit (2));
					}
					break;
				}

				case DisplayMode::Normal:
					print_time (_time, _display_precision);
					_display.set_colon (_calibrator.lit (2));
					break;
			}

			_display.set_all_digits_enabled (true);
			break;
		}

		case ClockMode::BeepSetup:
			if (_beeper_enabled)
				_display.set_digits (Display::Sign::Empty, Display::Sign::O, Display::Sign::N, Display::Sign::Empty);
			else
				_display.set_digits (Display::Sign::Empty, Display::Sign::O, Display::Sign::F, Display::Sign::F);
			break;

		case ClockMode::TimeSetup:
			print_time (_setup_time, DisplayPrecision::HoursMinutes);

			bool blink = _calibrator.lit (10);

			_display.set_colon (true);
			_display.set_digit_enabled (0, blink || _setup_digit != SetupDigit::Hours10);
			_display.set_digit_enabled (1, blink || _setup_digit != SetupDigit::Hours1);
			_display.set_digit_enabled (2, blink || _setup_digit != SetupDigit::Minutes10);
			_display.set_digit_enabled (3, blink || _setup_digit != SetupDigit::Minutes1);
			break;
	}
}


void
Clock::print_time (Time const& time, DisplayPrecision precision)
{
	switch (precision)
	{
		case DisplayPrecision::HoursMinutes:
			_display.set_digits (time.hours / 10L, time.hours % 10L, time.minutes / 10L, time.minutes % 10L);
			break;

		case DisplayPrecision::Seconds:
			_display.set_digits (Display::Sign::Empty, Display::Sign::Empty, time.seconds / 10L, time.seconds % 10L);
			break;
	}
}


bool
Clock::is_alarm() const
{
	return _time.hours == kMagicTimeOfDay.hours &&
		   _time.minutes == kMagicTimeOfDay.minutes;
}

#endif

