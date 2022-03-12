
-----------------------------------------------------------------
Simple psk decoder plugin
-----------------------------------------------------------------

The psk plugin for SDRuno is - as the name suggests - a plugin
for decoding psk signals transmitted on shortwave

![overview](/psk-example.png?raw=true)

-----------------------------------------------------------------------------
  READ THIS FIRST installing the plugin
-----------------------------------------------------------------------------

Since the psk signals are a small band signals (psk31 about 30 Hz in
this plugin), the samplerate used as input for the plugin
is 192000 samples/second.
The plugin uses the SP1 stream, provided by the SDRuno platform,
a stream with a rate of 192000.
Note that the zooming facility on the main spectrum display can be
used to show a spectrum where signals with such a small width
can be made visible.

The plugin itself can be stored in the folder for community plugins

The plugin is - as other plugins - developed under MSVC. Its functioning
depends on lots of other "dll's" (Dynamic Load Libraries);

If a 0x000012f error is encountered on trying to load the plugin,
it means that dll's from the Visual C++ Redistributable(VS 2015) are
not found.

--------------------------------------------------------------------
psk decoding
--------------------------------------------------------------------

psk31 is a signal with a small bandwidth, psk31 has a bandwidth of
31 Hz. Tuning psk is difficult, a few Hz off and decoding is not well
possible.

Tuning is difficult. The main spectrum display shows a signal with a
width of 62500 Hz. Happily, the display has a zoom possibility.
There is a smaller display, showing only a fraction of the complete
band, and this display has an excellent zoom function (see the picture).
For tuning the VFO has to be exactly on the center of the psk signal,
using the zoom function, and using the numerical display, makes it possible
to tune to the signal.
However, since most transmissions in amateur bands are brief, correct
tuning to a signal requires some training.
In case you are almost om the right frequency, there is a button, labeled "tune", that will look into a small segment
of the spectrum and adjust the frequency.

The widget has three comboboxes,  one for the afc, one for reversing
the decoding of the modulation and one for selecting the psk Mode.

Below these comboboxes, two spinboxes are visible, the left
one for setting the degree of the (final) lowpass filter to eliminate
noise in the neighbourhood of the signal, the second one
for setting the noise floor.

Below these spinboxes, there are two number displays visible, one showing the
quality of the signal (100 is perfect), the second one showing the
computed frequency offset (of course only non-zero when the afc is on).

The picture shows a signal quality of 89, which is reasonable, the 
computed correction for the frequency is 4.5 Hz.

The implementation of the decoder is taken from the psk decoder of the
swradio-8


