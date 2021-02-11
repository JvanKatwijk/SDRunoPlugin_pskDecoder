
-----------------------------------------------------------------
Simple psk decoder plugin
-----------------------------------------------------------------

![overview](/psk-example.png?raw=true)

--------------------------------------------------------------------
psk decoding
--------------------------------------------------------------------

psk31 is a signal with a small bandwidth, psk31 has a bandwidth of
31 Hz. Tuning psk is difficult, a few Hz off and decoding is not well
possible.

The psk decoder assumes that the input rate is reduced to 2000000 / 32, i.e.
about 62.5 KHz. The large spectrum display shows a psk signal, after all 
it shows the energy of the signal, correct tuning on this display is
rather difficult.

Happily there is a smaller display, showing only a fraction of the complete
band, and this display has an excellent zoom function (see the picture).
For tuning the VFO has to be exactly on the center of the psk signal,
using the zoom function, and using the numerical display, makes it possible
to tune to the signal.
However, since most transmissions in amateur bands are brief, correct
tuning to a signal requires some training.

The widget has three comboboxes,  one for the afc, one for reversing
the decoding of the modulation and one for selecting the psk Mode.

Below these comboboxes, two spinboxes are visible, the left
on for setting the degree of the (final) lowpass filter to eliminate
noise in the neighbourhood of the signal, the second one
for setting the noise floor.

Below these spinboxes, there are two number displays visible, one showing the
quality of the signal (100 is perfect), the second one showing the
computed frequency offset (of course only non-zero when the afc is on).

The picture shows a signal quality of 89, which is reasonable, the 
computed correction for the frequency is 4.5 Hz.

The implementation of the decoder is taken from the psk decoder of the
swradio-8




