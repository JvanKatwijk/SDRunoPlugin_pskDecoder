#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/spinbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>

// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
//#define formWidth (297)
#define formWidth (500)
#define formHeight (240)

class SDRunoPlugin_pskUi;

class SDRunoPlugin_pskForm : public nana::form {
public:

	SDRunoPlugin_pskForm      (SDRunoPlugin_pskUi& parent,
	                          IUnoPluginController& controller);		
	~SDRunoPlugin_pskForm ();
//
//	going down
void	show_pskText	        (const std::string &);
void	show_qualityLabel	(float);
void	show_pskIF	        (float);

//	going upwards
void	set_pskAfc	      (const std::string &);
void	set_pskReverse    (const std::string &);
void	set_pskMode       (const std::string &);
void	set_pskFilter	  (int);
void	set_pskSquelch	  (int);

//
void	Run			();

private:

	void Setup();

	// The following is to set up the panel graphic to look like a standard SDRuno panel
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth), formHeight - topBarHeight - bottomBarHeight) };
	nana::picture header_bar{ *this, true };
	nana::label title_bar_label{ *this, true };
	nana::dragger form_dragger;
	nana::label form_drag_label{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::paint::image img_min_normal;
	nana::paint::image img_min_down;
	nana::paint::image img_close_normal;
	nana::paint::image img_close_down;
	nana::paint::image img_header;
	nana::picture close_button {*this, nana::rectangle(0, 0, 20, 15) };
	nana::picture min_button {*this, nana::rectangle(0, 0, 20, 15) };

	// Uncomment the following 4 lines if you want a SETT button and panel
	nana::paint::image img_sett_normal;
	nana::paint::image img_sett_down;
	nana::picture sett_button{ *this, nana::rectangle(0, 0, 40, 15) };
	void SettingsButton_Click();

// TODO: Now add your UI controls here
//
	nana::label pskText {*this, nana::rectangle (20, 200, 450, 20) };

	nana::combox pskAfc {*this, nana::rectangle (60, 80, 100, 20) };
	nana::combox pskReverse {*this, nana::rectangle (200, 80, 100, 20)};
	nana::combox pskMode {*this, nana::rectangle (350, 80, 100, 20)};

	nana::spinbox pskFilter {*this, nana::rectangle (50, 100, 100, 30) };
	nana::spinbox pskSquelch {*this, nana::rectangle (200, 100, 100, 30) }; 
	
	nana::label qualityLabel {*this, nana::rectangle (50, 150, 100, 20) };
	nana::label IFLabel {*this, nana::rectangle (200, 150, 100, 20) };

	SDRunoPlugin_pskUi & m_parent;
	IUnoPluginController & m_controller;
};
