
-----------------------------------------------------------------
Simple psk decoder plugin
-----------------------------------------------------------------

![overview](/psk-example.png?raw=true)

--------------------------------------------------------------------
psk decoding
--------------------------------------------------------------------

psk31 is a signal with a small bandwidth, pdk31 has a bandwidth of
31 Hz. Tuning psk is difficult, a few Hz off and decoding is not well
possible.

The psk decoder assumes that the input rate is reduced to 2000000 / 32, i.e.
about 62.5 KHz. The large spectrum display shows a psk signal, after all 
it shows the energy of the signal, correct tuning on this display is
rather difficult.

Happily there is a smaller display, showing only a fraction of the complete
band, and this displayh has an excellent zoom function.
Using the zoom function, and using the numerical display, makes it possible
to tune to the signal.


The implementation of the decoder is taken from the psk decoder of the
swradio-8




