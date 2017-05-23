/**
 * Expected fuse values:
 *   E: 0xf3
 *   H: 0xd9
 *   L: 0xd2
 */

// Mulabs:
#include <avr/interrupt.h>

template<class T, class U = T>
	T exchange(T& obj, U&& new_value)
	{
		T old_value = obj;
		obj = new_value;
		return old_value;
	}

// Local:
#include "mcu.h"
#include "time.h"
#include "loop_calibrator.h"
#include "rtc.h"
#include "debouncer.h"
#include "switch.h"
#include "display.h"
#include "clock.h"


int
main()
{
	MCU::initialize();

	Clock clock;
	clock.loop();
}

