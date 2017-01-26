### cpuinfo ###
`cpuinfo` reports CPU clock frequency using hardware Performance Counters
via MSR registers. In particular:
 
  * Fixed Counter 1 counts the number of clock cycles while the core is not in 
    halted state
  * Fixed Counter 2 counts the number of reference clock cycles, at the base operating
    frequency, while the core is not in halted state

The clock frequency of the core when not in halted state is obtained as follows:
```
clock_frequency = base_operating_frequency * (unhalted_core_cycles / unhalted_reference_cycles)
```

### License ###
`cpuinfo` is licensed under the GPLv3 license.

```
cpuinfo - Tool which reports CPU clock frequency using hardware Performance 
Counters via MSR registers

Copyright (C) 2016 Marco Guerri <marco.guerri.dev@fastmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>

