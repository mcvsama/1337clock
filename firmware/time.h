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

#ifndef CLOCK_1337__TIME__INCLUDED
#define CLOCK_1337__TIME__INCLUDED

/**
 * Time of day representation.
 */
class Time
{
  public:
	uint8_t	hours		= 0;
	uint8_t	minutes		= 0;
	uint8_t	seconds		= 0;

  public:
	constexpr bool
	operator== (Time const& other) const;

	constexpr bool
	operator!= (Time const& other) const;

	constexpr uint32_t
	seconds_since_midnight() const;

	constexpr void
	update_modulo();

	constexpr void
	sanitize();
};


constexpr bool
Time::operator== (Time const& other) const
{
	return hours == other.hours &&
		   minutes == other.minutes &&
		   seconds == other.seconds;
}


constexpr bool
Time::operator!= (Time const& other) const
{
	return !(*this == other);
}


constexpr uint32_t
Time::seconds_since_midnight() const
{
	return 1L * seconds + 60L * minutes + 3600L * hours;
}


constexpr void
Time::update_modulo()
{
	hours = hours % 24;
	minutes = minutes % 60;
	seconds = seconds % 60;
}


constexpr void
Time::sanitize()
{
	if (hours > 23)
		hours = 23;

	if (minutes > 59)
		minutes = 59;

	if (seconds > 59)
		seconds = 59;
}

#endif

