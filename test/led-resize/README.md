# Color-light LED count regression

Run `python test/led-resize/test.py <compiler> [compiler arguments]`.

The test compiles the production resize/allocation functions, individual LED
callbacks and script range executor. It covers 1 -> 256 -> 16 LEDs, repeated
increases/decreases without color leaks or duplicate handlers, growth failing
partway through allocation, NeoPixel internal allocation failure, out-of-range
and reversed script selections after shrinking, dynamic ALL and status-only
output. It runs with automation enabled and disabled.

On the device, keep Waves or a script running, change the count repeatedly
(e.g. 16, 256, 64, 256), and check that uptime continues, the last pixel works,
and free DRAM returns to a stable level. Use a matching row count for the
intended matrix shape. Test both an increasing and a decreasing count while
an explicit script range spans the old end of the strip. Resend coordinate
scripts after changing the matrix shape, as their bytecode uses physical indices.
