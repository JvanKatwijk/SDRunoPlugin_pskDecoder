#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>
#include <stdlib.h>
#include <Windows.h>

#include "SDRunoPlugin_psk.h"
#include "SDRunoPlugin_pskUi.h"
#include "SDRunoPlugin_pskForm.h"

// Ui constructor - load the Ui control into a thread
	SDRunoPlugin_pskUi::
	                SDRunoPlugin_pskUi (SDRunoPlugin_psk& parent,
	                                     IUnoPluginController& controller) :
	m_parent(parent),
	m_form(nullptr),
	m_controller(controller) {
	m_thread = std::thread (&SDRunoPlugin_pskUi::ShowUi, this);
}

// Ui destructor (the nana::API::exit_all();) is required if using Nana UI library
	SDRunoPlugin_pskUi::~SDRunoPlugin_pskUi () {	
	nana::API::exit_all();
	m_thread.join();	
}

// Show and execute the form
void	SDRunoPlugin_pskUi::ShowUi () {	
	m_lock.lock();
	m_form = std::make_shared<SDRunoPlugin_pskForm> (*this,
	                                                      m_controller);
	m_lock.unlock();

	m_form->Run();
}

// Load X from the ini file (if exists)
// TODO: Change Template to plugin name
int	SDRunoPlugin_pskUi::LoadX () {
	std::string tmp;
	m_controller. GetConfigurationKey ("Template.X", tmp);
	if (tmp.empty()) {
	   return -1;
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change Template to plugin name
int	SDRunoPlugin_pskUi::LoadY () {
	std::string tmp;
	m_controller.GetConfigurationKey("Template.Y", tmp);
	if (tmp.empty()) {
	   return -1;
	}
	return stoi(tmp);
}

// Handle events from SDRuno
// TODO: code what to do when receiving relevant events
void SDRunoPlugin_pskUi::HandleEvent (const UnoEvent& ev) {
	switch (ev. GetType ()) {
	   case UnoEvent::StreamingStarted:
	      break;

	   case UnoEvent::StreamingStopped:
	      break;

	   case UnoEvent::SavingWorkspace:
	      break;

	   case UnoEvent::ClosingDown:
	      FormClosed ();
	      break;

	   default:
	      break;
	}
}

// Required to make sure the plugin is correctly unloaded when closed
void	SDRunoPlugin_pskUi::FormClosed () {
	m_controller.RequestUnload (&m_parent);
}


//      going down
void	SDRunoPlugin_pskUi::show_pskText	(const std::string &s) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> show_pskText (s);
}
	
void	SDRunoPlugin_pskUi::show_qualityLabel	(float q) {
	std::lock_guard<std::mutex> l (m_lock);
        if (m_form != nullptr)
           m_form -> show_qualityLabel (q);
}

void	SDRunoPlugin_pskUi::show_pskIF	(float q) {
	std::lock_guard<std::mutex> l (m_lock);
        if (m_form != nullptr)
           m_form -> show_pskIF (q);
}

//      going upwards
void	SDRunoPlugin_pskUi::set_pskAfc		(const std::string &s) {
	m_parent. set_pskAfc (s);
}

void	SDRunoPlugin_pskUi::set_pskReverse	(const std::string &s) {
	m_parent. set_pskAfc (s);
}

void	SDRunoPlugin_pskUi::set_pskMode		(const std::string &s) {
	m_parent. set_pskMode (s);
}

void	SDRunoPlugin_pskUi::set_pskFilter	(int d) {
	m_parent. set_pskFilter (d);
}

void	SDRunoPlugin_pskUi::set_pskSquelch	(int d) {
	m_parent. set_pskSquelch (d);
}


