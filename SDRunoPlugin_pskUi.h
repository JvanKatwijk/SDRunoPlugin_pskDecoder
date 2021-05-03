#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <iunoplugin.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>
#include "SDRunoPlugin_pskForm.h"

// Forward reference
class SDRunoPlugin_psk;

class SDRunoPlugin_pskUi {
public:

		SDRunoPlugin_pskUi (SDRunoPlugin_psk& parent,
	                                 IUnoPluginController& controller);
		~SDRunoPlugin_pskUi ();

	void	HandleEvent		(const UnoEvent& evt);
	void	FormClosed		();

	void	ShowUi			();

	int	LoadX			();
	int	LoadY			();

//      going down
	void	show_pskText		(const std::string &);
	void	show_qualityLabel	(float);
	void	show_pskIF		(float);

//      going upwards
	void	set_pskAfc		(const std::string &);
	void	set_pskReverse		(const std::string &);
	void	set_pskMode		(const std::string &);

	void	set_pskFilter		(int);
	void	set_pskSquelch		(int);
	void	set_searchWidth		(int);
	void	trigger_tune		();

private:
	
	SDRunoPlugin_psk & m_parent;
	std::thread m_thread;
	std::shared_ptr<SDRunoPlugin_pskForm> m_form;

	bool m_started;
	std::mutex m_lock;
	IUnoPluginController & m_controller;
};
