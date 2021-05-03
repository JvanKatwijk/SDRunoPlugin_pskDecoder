#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <iunoplugincontroller.h>
#include <iunoplugin.h>
#include <iunostreamobserver.h>
#include <iunoaudioobserver.h>
//#include <iunoaudioprocessor.h>
#include <iunostreamobserver.h>
#include <iunoannotator.h>
//
#include "SDRunoPlugin_pskUi.h"
//
//	for the payload we have
class	upFilter;


#include	"ringbuffer.h"
#include	<stdint.h>
#include        "utilities.h"
#include	"psk-bandfilter.h"
#include	"decimator-25.h"
#include	"psk-shifter.h"
#include	"sliding-fft.h"

#define	PSKRATE	2000

class SDRunoPlugin_psk : public IUnoPlugin,
	                             public IUnoStreamProcessor,
	                             public IUnoAudioProcessor {
public:
		SDRunoPlugin_psk (IUnoPluginController& controller);
	virtual ~SDRunoPlugin_psk ();

	virtual const
	char*	GetPluginName() const override { return "psk decoder"; }
	virtual
	void	StreamProcessorProcess (channel_t channel,
	                                Complex *buffer,
	                                int length,
	                                bool& modified) override;
        virtual
	void	AudioProcessorProcess (channel_t channel,
	                               float *buffer,
	                               int length, bool& modified) override;

	// IUnoPlugin
	virtual
	void	HandleEvent(const UnoEvent& ev) override;

//      going down
        void	show_pskText		(const std::string &);
        void	show_qualityLabel	(float);;
	void	show_pskIF		(float);

//      going upwards
        void    set_pskAfc              (const std::string &);
        void    set_pskReverse          (const std::string &);
        void    set_pskMode             (const std::string &);

        void    set_pskFilter           (int);
        void    set_pskSquelch          (int);
	void	set_searchWidth		(int);
	void	trigger_tune		();

private:
	enum PskMode {
	   MODE_PSK31 =		0100,
	   MODE_PSK63 =		0101,
	   MODE_PSK125 =	0102,
	   MODE_QPSK31 =	0110,
	   MODE_QPSK63 =	0111,
	   MODE_QPSK125 =	0112
	};
	std::mutex             m_lock;
	SDRunoPlugin_pskUi	m_form;
	std::mutex		locker;
	IUnoPluginController	*m_controller;
	void			WorkerFunction		();
	std::thread* 		m_worker;
	std::atomic<bool>	running;
	int			centerFrequency;
	int			selectedFrequency;
	slidingFFT	newFFT;
	int			searchWidth;
	int	                fillP;
	RingBuffer<std::complex<float> > pskSourceBuffer;
	RingBuffer<float> pskAudioBuffer;	
	pskBandfilter           passbandFilter;
	pskShifter		theMixer;
	decimator_25            theDecimator;
	std::vector<std::complex<float>> pskTone;
	pskShifter		localShifter;

	std::vector<std::complex<float>> convBuffer;
	int                     convIndex;
	int16_t                 mapTable_int   [PSKRATE / 100];
	float                   mapTable_float [PSKRATE / 100];

	void			process		(std::complex<float>);
	int16_t			resample	(std::complex<float>,
	                                         std::complex<float> *);
	void			processSample(std::complex<float>);
	void			doDecode	(std::complex<float>);
	void			psk_setup	();
	float			speedofPskMode	();
	bool			isBinarypskMode	();
	bool			isQuadpskMode	();	
	uint8_t			decodingIsOn	(uint8_t,
	                                         double,
	                                         uint8_t, std::complex<float> *);
	void			bpsk_bit	(uint16_t);
	void			qpsk_bit	(uint16_t);
	int16_t			pskVaridecode	(uint16_t);
	double			psk_IFadjust	(double, uint16_t);
	uint8_t			getIntPhase	(std::complex<float>);
	int16_t		DecimatingCountforpskMode	();

	void			psk_clrText();
	void		        psk_addText(char);
	int		offset(std::complex<float>*);

	void		updateFrequency		(int);
	std::vector<std::complex<float>> pskToneBuffer;
	int             pskTonePhase;
	pskBandfilter		*BPM_Filter;
//	viterbi			*viterbiDecoder;
	double			psk_IF;
	int16_t			pskDecimatorCount;
	unsigned int 		pskShiftReg;
	int16_t			pskPhaseAcc;
	uint32_t 		pskDecodeShifter;
	std::complex<float>	pskQuality;
	double			pskAfcmetrics;
	bool			pskAfcon;
	bool			pskReverse;
	int16_t			pskSquelchLevel;
	uint8_t			pskMode;
	int16_t			pskFilterDegree;
	std::complex<float>	pskOldz;
	double			psk_phasedifference;

	bool			tuning;
	int16_t			pskPrev1Phase;
	int16_t			pskPrev2Phase;
	int16_t			pskCurrPhase;
	int16_t			pskSyncCounter;
	float			pskBitclk;
	std::complex<float>	pskBuffer [16];

	bool			pskError;
	int		        pskSourceRate;
	int		        pskAudioRate;

	upFilter		*audioFilter;
};
