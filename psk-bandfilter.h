#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDRuno psk decoder
 *
 *    psk decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    psk decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with psk decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __BANDPASS_FILTER_H
#define __BANDPASS_FILTER_H

#include	<complex>
#include	<vector>

class	pskBandFilter {
public:
			pskBandFilter	(int16_t, int32_t, int32_t, int32_t);
			~pskBandFilter	();
	std::complex<float> Pass (std::complex<float>);
	void		update	(int, int);
private:
	std::vector<float> filterBase;
	std::vector<std::complex<float>> filterKernel;
	std::vector<std::complex<float>> Buffer;
	int	sampleRate;
	int	ip;
	int	filterSize;
};

#endif

