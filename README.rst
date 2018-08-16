Packeteer library
=================

A library for reasonably reliable async and packetized communications 
between MCUs, typically with UARTs.

Application code only needs to provide a send function, a receive function,
and an 'available()' function for getting the number of bytes waiting to be read.
Providing a yield function is optional. Check the header for details.

Written in C to allow usage with embedded software written in C alone.