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

#ifndef CLOCK_1337__RTC__INCLUDED
#define CLOCK_1337__RTC__INCLUDED

constexpr uint8_t
rtc_write_byte_to_reg (uint8_t command_byte)
{
	return (command_byte >> 1) & 0b11111;
}


/**
 * Support for DS1302 RTC clock.
 */
class RTC
{
	static constexpr MCU::Pin	_rtc_sclk		{ MCU::port_d.pin (1) };
	static constexpr MCU::Pin	_rtc_io			{ MCU::port_d.pin (2) };
	static constexpr MCU::Pin	_rtc_ce			{ MCU::port_d.pin (3) };

	enum class Direction: uint8_t
	{
		Read	= 0b0000'0001,
		Write	= 0b0000'0000,
	};

	enum class Storage: uint8_t
	{
		RAM		= 0b0100'0000,
		Clock	= 0b0000'0000,
	};

	enum class Register: uint8_t
	{
		Seconds			= rtc_write_byte_to_reg (0x80),
		Minutes			= rtc_write_byte_to_reg (0x82),
		Hours			= rtc_write_byte_to_reg (0x84),
		DayOfMonth		= rtc_write_byte_to_reg (0x86),
		Month			= rtc_write_byte_to_reg (0x88),
		DayOfWeek		= rtc_write_byte_to_reg (0x8a),
		Year			= rtc_write_byte_to_reg (0x8c),
		Control			= rtc_write_byte_to_reg (0x8e),
		TrickleCharger	= rtc_write_byte_to_reg (0x90),
	};

  public:
	// Ctor
	RTC();

	Time
	get_time() const;

	void
	set_time (Time const&);

	uint8_t
	get_seconds() const;

	void
	set_seconds (uint8_t);

	uint8_t
	get_minutes() const;

	void
	set_minutes (uint8_t);

	uint8_t
	get_hours() const;

	void
	set_hours (uint8_t);

	uint8_t
	get_day_of_month() const;

	uint8_t
	get_month() const;

	uint8_t
	get_day_of_week() const;

	uint8_t
	get_year() const;

  private:
	uint8_t
	read_register (Register) const;

	void
	write_register (Register, uint8_t value);

	void
	open_channel() const;

	void
	close_channel() const;

	void
	send_bytes (uint8_t, uint8_t);

	uint8_t
	send_receive_byte (uint8_t) const;

	uint8_t
	make_command (Direction, Storage, Register) const;

	void
	clk (bool level) const;
};


RTC::RTC()
{
	_rtc_sclk = false;
	_rtc_io = false;
	_rtc_ce = false;

	_rtc_sclk.configure_as_output();
	_rtc_io.configure_as_input();
	_rtc_ce.configure_as_output();

	// Make sure writes are enabled (write-protect bit off):
	write_register (Register::Control, 0b0000'0000);

	// Start the clock:
	uint8_t seconds = read_register (Register::Seconds);
	clear_bit (seconds, 7); // Bit 7 of the seconds register is Clock-Halt flag.
	write_register (Register::Seconds, seconds);

	// Use 24-hour format:
	uint8_t hours = read_register (Register::Hours);
	clear_bit (hours, 7);
	write_register (Register::Hours, hours);
}


Time
RTC::get_time() const
{
	return Time { get_hours(), get_minutes(), get_seconds() };
}


void
RTC::set_time (Time const& time)
{
	set_hours (time.hours);
	set_minutes (time.minutes);
	set_seconds (time.seconds);
}


uint8_t
RTC::get_seconds() const
{
	uint8_t val = read_register (Register::Seconds);
	return ((val & 0b0111'0000) >> 4) * 10 + (val & 0b1111);
}


void
RTC::set_seconds (uint8_t seconds)
{
	uint8_t val = (seconds % 10) | (static_cast<uint8_t> (seconds / 10) << 4);
	write_register (Register::Seconds, val);
}


uint8_t
RTC::get_minutes() const
{
	uint8_t val = read_register (Register::Minutes);
	return ((val & 0b0111'0000) >> 4) * 10 + (val & 0b1111);
}


void
RTC::set_minutes (uint8_t minutes)
{
	uint8_t val = (minutes % 10) | (static_cast<uint8_t> (minutes / 10) << 4);
	write_register (Register::Minutes, val);
}


uint8_t
RTC::get_hours() const
{
	uint8_t val = read_register (Register::Hours);
	return ((val & 0b0011'0000) >> 4) * 10 + (val & 0b1111);
}


void
RTC::set_hours (uint8_t hours)
{
	uint8_t val = (hours % 10) | (static_cast<uint8_t> (hours / 10) << 4);
	write_register (Register::Hours, val);
}


uint8_t
RTC::get_day_of_month() const
{
	uint8_t val = read_register (Register::DayOfMonth);
	return ((val & 0b0011'0000) >> 4) * 10 + (val & 0b1111);
}


uint8_t
RTC::get_month() const
{
	uint8_t val = read_register (Register::Month);
	return ((val & 0b0001'0000) >> 4) * 10 + (val & 0b1111);
}


uint8_t
RTC::get_day_of_week() const
{
	uint8_t val = read_register (Register::DayOfWeek);
	return val & 0b111;
}


uint8_t
RTC::get_year() const
{
	uint8_t val = read_register (Register::DayOfWeek);
	uint16_t year = ((val & 0b1111'0000) >> 4) * 10 + (val & 0b1111);
	return year + 2000;
}


inline uint8_t
RTC::read_register (Register reg) const
{
	open_channel();
	uint8_t data = send_receive_byte (make_command (Direction::Read, Storage::Clock, reg));
	close_channel();
	return data;
}


inline void
RTC::write_register (Register reg, uint8_t value)
{
	open_channel();
	send_bytes (make_command (Direction::Write, Storage::Clock, reg), value);
	close_channel();
}


inline void
RTC::open_channel() const
{
	_rtc_ce = true;
	MCU::sleep_us<1>();
}


inline void
RTC::close_channel() const
{
	_rtc_ce = false;
	MCU::sleep_us<1>();
}


void
RTC::send_bytes (uint8_t byte1, uint8_t byte2)
{
	_rtc_io.configure_as_output();

	uint8_t bytes[2] = { byte1, byte2 };

	for (size_t i = 0; i < 2; ++i)
	{
		uint8_t byte = bytes[i];

		for (uint8_t b = 0; b < 8; ++b)
		{
			_rtc_io = !!((byte >> b) & 1);
			clk (true);
			clk (false);
		}
	}

	_rtc_io = false;
	_rtc_io.configure_as_input();
}


uint8_t
RTC::send_receive_byte (uint8_t byte) const
{
	_rtc_io.configure_as_output();

	uint8_t result = 0;

	// Send on 8 rising edges, receive on 8 falling edges.
	for (uint8_t b = 0; b < 8; ++b)
	{
		clk (false);
		_rtc_io = !!((byte >> b) & 1);
		clk (true);
	}

	_rtc_io = false;
	_rtc_io.configure_as_input();

	for (uint8_t b = 0; b < 8; ++b)
	{
		clk (true);
		clk (false);
		result |= _rtc_io.get() << b;
	}

	return result;
}


uint8_t
RTC::make_command (Direction direction, Storage storage, Register reg) const
{
	uint8_t data = static_cast<uint8_t> (reg);
	data = data << 1;
	data |= 0b1000'0000;
	data |= static_cast<uint8_t> (storage);
	data |= static_cast<uint8_t> (direction);
	return data;
}


void
RTC::clk (bool level) const
{
	MCU::sleep_us<5>(); // <1>
	_rtc_sclk = level;
	MCU::sleep_us<10>(); // <4>
}

#endif

