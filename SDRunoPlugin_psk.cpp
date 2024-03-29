#include	<sstream>
#include	<unoevent.h>
#include	<iunoplugincontroller.h>
#include	<vector>
#include	<sstream>
#include	<chrono>
#include	<Windows.h>
#include	<mutex>
#include	"SDRunoPlugin_psk.h"
#include	"SDRunoPlugin_pskUi.h"

#include        "utilities.h"
#include        "up-filter.h"

#define	IN_RATE			192000
#define pskK                    5
#define pskPOLY1                0x17
#define pskPOLY2                0x19
#define PSK31_SPEED             31.25
#define PSK63_SPEED             62.50
#define PSK125_SPEED            125.0

#define PSK_IF			0

#define	NO_OFFSET_FOUND		-500
#define	SEARCH_WIDTH 800
//
#define  _USE_MATH_DEFINES
#include	<math.h>
//
SDRunoPlugin_psk::
	     SDRunoPlugin_psk (IUnoPluginController& controller) :
	                                         IUnoPlugin  (controller),
	                                         m_form   (*this, controller),
	                                         m_worker (nullptr),
	                                         pskSourceBuffer (16 * 32768),
	                                         pskAudioBuffer (16 * 32768),
	                                         passbandFilter (35,
	                                                         -PSKRATE / 2,
	                                                         PSKRATE / 2,
	                                                         IN_RATE),
	                                         theDecimator (IN_RATE / 2000),
	                                         pskToneBuffer (128),
	                                         newFFT (PSKRATE, 0, PSKRATE - 1){
	
	m_controller		= &controller;
//
//	psk specifics
	psk_IF			= PSK_IF;
	running. store (false);

	pskFilterDegree	= 12;		/* default	*/
	pskAfcon	= false;
	pskSquelchLevel	= 5;
	pskReverse	= false;
	searchRange	= SEARCH_WIDTH;

	avgSignal	= 0;
	for (int i = 0; i < 16; i ++)
	   pskBuffer [i] = std::complex<float> (0, 0);

	pskBitclk	= 0;
//	pskMode		= get_pskMode (m_form. get_pskMode ());
	pskMode		= MODE_PSK31;
	int pskSpeed	= speedofPskMode (pskMode);
	psk_baseFilter	= new pskBandFilter (2 * pskFilterDegree + 1,
	                                     psk_IF - 500,
	                                     psk_IF + 500,
	                                     PSKRATE);
	BPM_Filter	= new pskBandFilter (2 * pskFilterDegree + 1,
	                                     psk_IF - pskSpeed,
	                                     psk_IF + pskSpeed,
	                                     PSKRATE);

//	m_controller	-> RegisterStreamProcessor (0, this);
	m_controller	-> RegisterAudioProcessor (0, this);
	m_controller    -> SetDemodulatorType (0,
	                          IUnoPluginController::DemodulatorIQOUT);

	pskAudioRate	= m_controller -> GetAudioSampleRate (0);

	pskToneBuffer. resize (pskAudioRate);
	for (int i = 0; i < pskAudioRate; i++) {
	   float term = (float)i / pskAudioRate * 2 * M_PI;
	   pskToneBuffer [i] = std::complex<float> (cos (term),
	                                            sin (term));
	}

	pskDecimatorCount	= 0;
	pskTonePhase     = 0;
	audioFilter     = new upFilter (25, PSKRATE, pskAudioRate);

	m_worker        = new std::thread (&SDRunoPlugin_psk::WorkerFunction,
	                                                               this);
}

	SDRunoPlugin_psk::~SDRunoPlugin_psk () {
	running. store (false);
	m_worker	-> join ();
	m_controller	-> UnregisterStreamProcessor (0, this);
	m_controller	-> UnregisterAudioProcessor (0, this);
	delete m_worker;
	m_worker = nullptr;

	delete psk_baseFilter;
	delete BPM_Filter;
	delete audioFilter;
}

void	SDRunoPlugin_psk::StreamProcessorProcess (channel_t	channel,
	                                           Complex	*buffer,
	                                           int		length,
	                                           bool		&modified) {
	(void)channel; (void)buffer; (void)length; 
	modified = false;
}

void	SDRunoPlugin_psk::AudioProcessorProcess (channel_t channel,
	                                          float* buffer,
	                                          int length,
	                                          bool& modified) {
//	Handling IQ input, note that SDRuno interchanges I and Q elements
	if (!modified) {
	   for (int i = 0; i < length; i++) {
	      std::complex<float> sample =
                           std::complex<float>(buffer [2 * i +  1],
                                               buffer [2 * i]);
              sample = passbandFilter.Pass (sample);
	      pskSourceBuffer.putDataIntoBuffer (&sample, 1);
           }
        }

	if (pskAudioBuffer. GetRingBufferReadAvailable () >= length * 2) {
	   pskAudioBuffer. getDataFromBuffer (buffer, length * 2);
	}
	modified = true;
}

void	SDRunoPlugin_psk::HandleEvent (const UnoEvent& ev) {
	switch (ev. GetType ()) {
	   case UnoEvent::FrequencyChanged:
	      break;

	   case UnoEvent::CenterFrequencyChanged:
	      break;

	   default:
	      m_form. HandleEvent (ev);
	      break;
	}
}

#define	BUFFER_SIZE	4096
void	SDRunoPlugin_psk::WorkerFunction () {
std::complex<float> buffer [BUFFER_SIZE];
int	cycleCount	= 0;

	running. store (true);
	while (running. load ()) {
	   while (running. load () &&
	              (pskSourceBuffer. GetRingBufferReadAvailable () < BUFFER_SIZE))
	      Sleep (1);
	   if (!running. load ())
	      break;
	   pskSourceBuffer. getDataFromBuffer (buffer, BUFFER_SIZE);
	   for (int i = 0; i < BUFFER_SIZE; i++) {
	      std::complex<float> sample = buffer [i];
	      sample	= psk_baseFilter -> Pass (sample);
		  if (theDecimator.Pass(sample, &sample))
	         processSample (sample);
	   }
	   cycleCount += BUFFER_SIZE;
	   if (cycleCount > IN_RATE) {
	      cycleCount		= 0;
	      show_qualityLabel (pskAfcmetrics);
//	      show_pskIF	(psk_IF);
	   }
	}
	m_form.show_pskText ("end of worker function");
	Sleep(1000);
}

static inline
std::complex<float> cmul(std::complex<float> x, float y) {
	return std::complex<float> (real(x) * y, imag(x) * y);
}

static int fftTeller = 0;
//
//	here we are on PSK_RATE samples per second
void	SDRunoPlugin_psk::processSample (std::complex<float> z) {
std::vector<std::complex<float>> tone (pskAudioRate / PSKRATE);
std::complex<float> outV [PSKRATE];
static int initCount = 0;
	if (initCount <= PSKRATE) {
	   initCount ++;
	   avgSignal += abs (z);
	   if (initCount == PSKRATE)
	      avgSignal /= PSKRATE;
	}

	audioFilter -> Filter (cmul (z, 20), tone. data ());
	for (int i = 0; i < tone. size (); i ++) {
	   tone [i] *= pskToneBuffer [pskTonePhase];
	   pskTonePhase = (pskTonePhase + 801) % pskAudioRate;
	}
	pskAudioBuffer.putDataIntoBuffer (tone. data (), tone. size () * 2);
//
	if (abs (z) > 3 * avgSignal)  {
	   newFFT. do_FFT (z, outV);
	   fftTeller++;
	   if (fftTeller >= PSKRATE / 2) {
	      int offs = offset (outV);
	      if ((offs != NO_OFFSET_FOUND) && (abs (offs) >= 3)) {
	         if ((- searchRange / 2 < psk_IF + offs) &&
	             (psk_IF + offs / 2 < searchRange)) {
	            psk_IF += offs / 2;
	            BPM_Filter -> update (psk_IF - 30, psk_IF + 30);
	            updateFrequency (offs / 2);
	            m_form.  show_pskIF (psk_IF);
	         }
	      }
	      fftTeller = 0;
	      newFFT.reset();
	   }
	}
	avgSignal = 0.99 * avgSignal + 0.01 * abs (z);
	z = BPM_Filter	-> Pass (z);
//
//	Now we are on PSKRATE and decimate to 16 samples per symbol
	if (++pskDecimatorCount < this -> DecimatingCountforpskMode (pskMode)) 
	   return;
	pskDecimatorCount	= 0;
	doDecode (z);
}
/*
 *	now we are finally back on 16 samples per symbol
 *	to apply the "standard" algorithm
 */
void	SDRunoPlugin_psk::doDecode (std::complex<float> v) {
uint16_t	pskBufferp;
float		sum	= 0;
float		ampsum	= 0;
uint8_t	bits;

	pskBufferp				= (int) pskBitclk;
	pskBuffer [pskBufferp % 16]		= v;

	for (int i = 0; i < 16 / 2; i ++) {
	   sum    += abs (pskBuffer [i]) - abs (pskBuffer [16 / 2 + i]);
	   ampsum += abs (pskBuffer [i]) + abs (pskBuffer [16 / 2 + i]);
	}

	sum = (ampsum < 1.0 / 16 ? 0 : sum / ampsum);
	pskBitclk -= sum / 5.0;
	pskBitclk += 1;
	if (pskBitclk < 0)
	   pskBitclk += 16;
	if (pskBitclk < 16) 
	   return;

	pskBitclk -= 16;
	psk_phasedifference = arg (pskBuffer [4] * conj (pskBuffer [12]));
	if (psk_phasedifference < 0)
	   psk_phasedifference += 2 * M_PI;

	if (this -> isQuadpskMode ()) {
	   bits = (uint8_t(psk_phasedifference / (M_PI / 2) + 0.5)) & 03;
	   if (decodingIsOn (4, psk_phasedifference, bits, &pskQuality))
	      qpsk_bit (bits);
	}
	else {  //if (isBinarypskMode (pskMode)) 
	   bits = (((uint8_t)(psk_phasedifference / M_PI + 0.5)) & 1) << 1;
	   if (decodingIsOn (2, psk_phasedifference, bits, &pskQuality))
	      bpsk_bit  (!bits);
	}
	
	pskAfcmetrics	= 0.99 * pskAfcmetrics +
	                               0.01 * (100 * norm (pskQuality));

	if (pskAfcon)
	   psk_IF += psk_IFadjust (psk_phasedifference, bits);
}

uint8_t	SDRunoPlugin_psk::decodingIsOn (uint8_t n,
	                                double phase,
	                                uint8_t bits, 
	                                std::complex<float> *pskQuality) {
	pskDecodeShifter	= (pskDecodeShifter << 2) | bits;

	switch (pskDecodeShifter) {
	   case 0xAAAAAAAA:	/* "1010101010..." preAmble	*/
	      *pskQuality	= std::complex<float> (1.0, 0.0);
	      return true;

	   case 0x0:		/* postamble or garbage		*/
	      *pskQuality	= 0;
	      return false;

	   default:
	      *pskQuality	 =  std::complex<float> (
	                                        0.05 * cos (n * phase) +
		                                0.95 * real (*pskQuality),
		                                0.05 * sin (n * phase) +
		                                0.95 * imag (*pskQuality));
	      return
		 ((100.0 * norm (*pskQuality)) > (pskSquelchLevel));
	}
}
/*
 *	in case of complete sync, phase = bits * PI,
 *	otherwise, find a basis for correcting the IF
 *	phase and bits are related
 */
double	SDRunoPlugin_psk::psk_IFadjust (double phasedifference,
	                                              uint16_t bits) {
double	error;

	if (pskAfcmetrics < 0.1)
	   return 0.0;

	error = phasedifference - (double)bits * (M_PI / 2);
	if (error < - M_PI / 2)
	   error += 2 * M_PI;
	else
	if (error >= M_PI /2)
	   error -= 2 * M_PI;

	error /= 2 * M_PI;
	error *= (double)(PSKRATE / DecimatingCountforpskMode (pskMode)) / 8;
	return  - error / 256;
}

void	SDRunoPlugin_psk::bpsk_bit (uint16_t bit) {
int16_t	c;

	pskShiftReg = (pskShiftReg << 1) | !!bit;
	if ((pskShiftReg & 03) == 0) {
	   c = pskVaridecode (pskShiftReg >> 2);
	   if (c != -1) 
	      psk_addText ((char)c);
	   pskShiftReg = 0;
	}
}

void	SDRunoPlugin_psk::qpsk_bit (uint16_t bits) {
uint8_t sym [2];
int16_t	c;

	if (pskReverse)
	   bits = (4 - bits) & 03;

	sym [0] = (bits & 01) ? 255 : 0;
	sym [1] = (bits & 02) ? 0 : 255;	// top bit is flipped

//	c = viterbiDecoder -> viterbi_decode (sym, NULL);

	if (c != -1) {
	   bpsk_bit (c & 0x80);
	   bpsk_bit (c & 0x40);
	   bpsk_bit (c & 0x20);
	   bpsk_bit (c & 0x10);
	   bpsk_bit (c & 0x08);
	   bpsk_bit (c & 0x04);
	   bpsk_bit (c & 0x02);
	   bpsk_bit (c & 0x01);
	}
}
/*
 * The code table in a format suitable for decoding.
 */
static unsigned int varicodetab2[] = {
	0x2AB, 0x2DB, 0x2ED, 0x377, 0x2EB, 0x35F, 0x2EF, 0x2FD, 
	0x2FF, 0x0EF, 0x01D, 0x36F, 0x2DD, 0x01F, 0x375, 0x3AB, 
	0x2F7, 0x2F5, 0x3AD, 0x3AF, 0x35B, 0x36B, 0x36D, 0x357, 
	0x37B, 0x37D, 0x3B7, 0x355, 0x35D, 0x3BB, 0x2FB, 0x37F, 
	0x001, 0x1FF, 0x15F, 0x1F5, 0x1DB, 0x2D5, 0x2BB, 0x17F, 
	0x0FB, 0x0F7, 0x16F, 0x1DF, 0x075, 0x035, 0x057, 0x1AF, 
	0x0B7, 0x0BD, 0x0ED, 0x0FF, 0x177, 0x15B, 0x16B, 0x1AD, 
	0x1AB, 0x1B7, 0x0F5, 0x1BD, 0x1ED, 0x055, 0x1D7, 0x2AF, 
	0x2BD, 0x07D, 0x0EB, 0x0AD, 0x0B5, 0x077, 0x0DB, 0x0FD, 
	0x155, 0x07F, 0x1FD, 0x17D, 0x0D7, 0x0BB, 0x0DD, 0x0AB, 
	0x0D5, 0x1DD, 0x0AF, 0x06F, 0x06D, 0x157, 0x1B5, 0x15D, 
	0x175, 0x17B, 0x2AD, 0x1F7, 0x1EF, 0x1FB, 0x2BF, 0x16D, 
	0x2DF, 0x00B, 0x05F, 0x02F, 0x02D, 0x003, 0x03D, 0x05B, 
	0x02B, 0x00D, 0x1EB, 0x0BF, 0x01B, 0x03B, 0x00F, 0x007, 
	0x03F, 0x1BF, 0x015, 0x017, 0x005, 0x037, 0x07B, 0x06B, 
	0x0DF, 0x05D, 0x1D5, 0x2B7, 0x1BB, 0x2B5, 0x2D7, 0x3B5, 
	0x3BD, 0x3BF, 0x3D5, 0x3D7, 0x3DB, 0x3DD, 0x3DF, 0x3EB, 
	0x3ED, 0x3EF, 0x3F5, 0x3F7, 0x3FB, 0x3FD, 0x3FF, 0x555, 
	0x557, 0x55B, 0x55D, 0x55F, 0x56B, 0x56D, 0x56F, 0x575, 
	0x577, 0x57B, 0x57D, 0x57F, 0x5AB, 0x5AD, 0x5AF, 0x5B5, 
	0x5B7, 0x5BB, 0x5BD, 0x5BF, 0x5D5, 0x5D7, 0x5DB, 0x5DD, 
	0x5DF, 0x5EB, 0x5ED, 0x5EF, 0x5F5, 0x5F7, 0x5FB, 0x5FD, 
	0x5FF, 0x6AB, 0x6AD, 0x6AF, 0x6B5, 0x6B7, 0x6BB, 0x6BD, 
	0x6BF, 0x6D5, 0x6D7, 0x6DB, 0x6DD, 0x6DF, 0x6EB, 0x6ED, 
	0x6EF, 0x6F5, 0x6F7, 0x6FB, 0x6FD, 0x6FF, 0x755, 0x757, 
	0x75B, 0x75D, 0x75F, 0x76B, 0x76D, 0x76F, 0x775, 0x777, 
	0x77B, 0x77D, 0x77F, 0x7AB, 0x7AD, 0x7AF, 0x7B5, 0x7B7, 
	0x7BB, 0x7BD, 0x7BF, 0x7D5, 0x7D7, 0x7DB, 0x7DD, 0x7DF, 
	0x7EB, 0x7ED, 0x7EF, 0x7F5, 0x7F7, 0x7FB, 0x7FD, 0x7FF, 
	0xAAB, 0xAAD, 0xAAF, 0xAB5, 0xAB7, 0xABB, 0xABD, 0xABF, 
	0xAD5, 0xAD7, 0xADB, 0xADD, 0xADF, 0xAEB, 0xAED, 0xAEF, 
	0xAF5, 0xAF7, 0xAFB, 0xAFD, 0xAFF, 0xB55, 0xB57, 0xB5B
};


int16_t	SDRunoPlugin_psk::pskVaridecode (uint16_t symbol) {
int i;

	for (i = 0; i < 256; i++)
	   if (symbol == varicodetab2[i])
	      return i;

	return -1;
}

//     going down
void	SDRunoPlugin_psk::show_pskText		(const std::string &s) {
	m_form. show_pskText (s);
}

void	SDRunoPlugin_psk::show_qualityLabel	(float n) {
	m_form. show_qualityLabel (n);
}

void	SDRunoPlugin_psk::show_pskIF		(float f) {
	m_form. show_pskIF	(f);
}

//      going upwards

bool	SDRunoPlugin_psk::isBinarypskMode () {
	if ((pskMode == MODE_PSK31) ||
	    (pskMode == MODE_PSK63) ||
	    (pskMode == MODE_PSK125)) return true;
	return false;
}

bool	SDRunoPlugin_psk::isQuadpskMode () {

	if ((pskMode == MODE_QPSK31) ||
	    (pskMode == MODE_QPSK63) ||
	    (pskMode == MODE_QPSK125)) return true;
	return false;
}

float	SDRunoPlugin_psk::speedofPskMode (int mode) {
	switch (mode) {
	   default:
	   case MODE_PSK31:
	   case MODE_QPSK31:
	      return PSK31_SPEED;

	   case MODE_PSK63:
	   case MODE_QPSK63:
	      return PSK63_SPEED;

	   case MODE_PSK125:
	   case MODE_QPSK125:
	      return PSK125_SPEED;
	}
}

int16_t SDRunoPlugin_psk::DecimatingCountforpskMode (int mode) {
	switch (mode) {
	   default:
	   case MODE_PSK31:
	   case MODE_QPSK31:
	      return 4;
	   case MODE_PSK63:
	   case MODE_QPSK63:
	      return 2;
	   case MODE_PSK125:
	   case MODE_QPSK125:
	      return 1;
	}
}

void	SDRunoPlugin_psk::set_pskAfc		(const std::string &s) {
	pskAfcon = s == "Afc on";
	psk_setup ();
}

void	SDRunoPlugin_psk::set_pskReverse	(const std::string &s) {
	pskReverse = s != "normal";
	psk_setup ();
}

void	SDRunoPlugin_psk::set_pskSquelch	(int n) {
	pskSquelchLevel = n;
}


int	SDRunoPlugin_psk::get_pskMode		(const std::string &s) {
	if (s == "psk31")
	   return  MODE_PSK31;
	else
	if (s == "qpsk31")
	   return  MODE_QPSK31;
	else
	if (s == "psk63")
	   return  MODE_PSK63;
	else
	if (s == "qpsk63")
	   return MODE_QPSK63;
	else
	if (s == "psk125")
	   return  MODE_PSK125;
	else
	   return  MODE_QPSK125;
}

void	SDRunoPlugin_psk::set_pskMode		(const std::string &s) {
	pskMode	= get_pskMode (s);
	psk_setup ();
}

void	SDRunoPlugin_psk::set_pskFilter		(int n) {
	pskFilterDegree = n;
	psk_setup ();
}

void	SDRunoPlugin_psk::psk_setup	() {

	if (!pskAfcon) {
	   psk_IF	= PSK_IF;
	}

//	pskReverse
//	pskSquelchLevel
//	pskMode
//	pskFilterDegree


	pskPhaseAcc		= 0;
	pskOldz			= 0;
	psk_phasedifference	= 0.0;
	pskShiftReg		= 0;
	pskDecodeShifter	= 0;
	pskSyncCounter		= 0;
	pskAfcmetrics		= 0;
	pskShiftReg		= 0;
	pskQuality		= 0;
	pskBitclk		= 0;
	pskDecimatorCount	= 0;

	delete	BPM_Filter;
	BPM_Filter	= new pskBandFilter (2 * pskFilterDegree + 1,
	                                     PSK_IF - speedofPskMode (pskMode),
	                                     PSK_IF + speedofPskMode (pskMode),
	                                     PSKRATE);
}

static  std::string pskTextLine;
void    SDRunoPlugin_psk::psk_clrText		()	{
	m_form.  show_pskText ("");
	pskTextLine        = "";
}

void	SDRunoPlugin_psk::psk_addText		(char c)  {
	if (c < ' ')
	   c = ' ';
	pskTextLine. append (1, c);
	if (pskTextLine. length () > 65)
	   pskTextLine. erase (0, 1);
	m_form. show_pskText (pskTextLine);
}

void	SDRunoPlugin_psk::set_searchWidth	(int w) {
	searchRange	= w;
}

#define	MAX_SIZE 20
int	SDRunoPlugin_psk::offset (std::complex<float> *v) {
float avg	= 0;
float max	= 0;
float	supermax	= 0;
int	superIndex	= 0;

	if (searchRange < 25)
	   return 0;
	for (int i = 0 ; i < searchRange; i ++)
	   avg += abs (v [ (PSKRATE - searchRange / 2 + i) % PSKRATE]);
	avg /= searchRange;
	int	index	= PSKRATE - searchRange / 2;
	for (int i = 0; i < MAX_SIZE; i ++)
	   max +=  abs (v [index + i]);

	supermax	= max;
	for (int i = MAX_SIZE; i < 400; i ++) {
	   max -=  abs (v [(index + i - MAX_SIZE) % PSKRATE]);
	   max +=  abs (v [(index + i) % PSKRATE]);
	   if (max > supermax) {
	      superIndex = (index + i - MAX_SIZE / 2);
	      supermax = max;
	   }
	}

	if (supermax / MAX_SIZE > 3 * avg)
	   return superIndex - PSKRATE;
	else
	   return NO_OFFSET_FOUND;
}

void	SDRunoPlugin_psk::updateFrequency (int offs) {
	if (abs (offs) <= 2)
	   return;
	int	currentFreq	=
	                m_controller -> GetVfoFrequency (0);
	m_controller -> SetVfoFrequency (0, currentFreq + offs);
}

void	SDRunoPlugin_psk::trigger_tune	() {
	newFFT. reset ();
	fftTeller	= 0;
	psk_IF		= PSK_IF;
	show_pskIF	(psk_IF);
}

