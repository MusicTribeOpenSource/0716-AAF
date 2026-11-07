/*
 * Copyright (c) - 2022 - MUSIC Tribe, MY.
 * This file is part of IGNITE.
 *
 * IGNITE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * IGNITE is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with IGNITE. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

using namespace Ignite;

namespace ui
{
	// midiPortList (DIN + USB only) is a display-order subset of portList (DIN, CV, USB),
	// which is kept in sync with midiDataIOPorts and is what the engine indexes MidiCnfg by.
	// Map each displayed port back to its real engine port id instead of reusing the display loop index.
	static uint8_t midiPortEngineId(const juce::String& name)
	{
		for (uint8_t idx = 0; idx < mt::Ignite::portList.size(); ++idx)
		{
			if (mt::Ignite::portList[idx] == name)
				return idx;
		}
		return 0;
	}

	////////////////////////////////////////////////
	//   Class: Ignite MidiConfig
	////////////////////////////////////////////////
	IgniteMidiConfig::IgniteMidiConfig()
	{
		midiPort.setFont(juce::Font(20, juce::Font::plain).withTypefaceStyle("Regular"));
		midiPort.setJustificationType(juce::Justification::centredLeft);
		midiPort.setEditable(false, false, false);
		midiPort.setColour(juce::Label::backgroundColourId, juce::Colour(CommonColorID::neutralGrey4));
		midiPort.setColour(juce::Label::outlineColourId, Colours::white);
		midiPort.makeVisible(this);

		softThru.setFont(juce::Font(20, juce::Font::plain).withTypefaceStyle("Regular"));
		softThru.setJustificationType(juce::Justification::centredLeft);
		softThru.setEditable(false, false, false);
		softThru.setColour(juce::Label::backgroundColourId, juce::Colour(CommonColorID::neutralGrey4));
		softThru.setColour(juce::Label::outlineColourId, Colours::white);
		softThru.makeVisible(this);

		helpInfoButton.makeVisibleAndAddListner(this);
		helpInfoButton.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());

		thruFts.setFont(juce::Font(20, juce::Font::plain).withTypefaceStyle("Regular"));
		thruFts.setJustificationType(juce::Justification::left);
		thruFts.setEditable(false, false, false);
		thruFts.setColour(juce::Label::backgroundColourId, juce::Colour(CommonColorID::neutralGrey4));
		thruFts.setColour(juce::Label::outlineColourId, Colours::white);
		thruFts.makeVisible(this);

		recordEnable.setFont(juce::Font(20, juce::Font::plain).withTypefaceStyle("Regular"));
		recordEnable.setJustificationType(juce::Justification::centredLeft);
		recordEnable.setEditable(false, false, false);
		recordEnable.setColour(juce::Label::backgroundColourId, juce::Colour(CommonColorID::neutralGrey4));
		recordEnable.setColour(juce::Label::outlineColourId, Colours::white);
		recordEnable.makeVisible(this);

		/*mclkSend.setFont(juce::Font(20, juce::Font::plain).withTypefaceStyle("Regular"));
		mclkSend.setJustificationType(juce::Justification::centredLeft);
		mclkSend.setEditable(false, false, false);
		mclkSend.setColour(juce::Label::backgroundColourId, ui::Theme::Instance().getColour(customOrange));
		mclkSend.setColour(juce::Label::outlineColourId, Colours::white);
		mclkSend.makeVisible(this);*/

		sendMtc.setFont(juce::Font(20, juce::Font::plain).withTypefaceStyle("Regular"));
		sendMtc.setJustificationType(juce::Justification::centredLeft);
		sendMtc.setEditable(false, false, false);
		sendMtc.setColour(juce::Label::backgroundColourId, juce::Colour(CommonColorID::neutralGrey4));
		sendMtc.setColour(juce::Label::outlineColourId, Colours::white);
		sendMtc.makeVisible(this);

		port.setFont(juce::Font(20, juce::Font::plain).withTypefaceStyle("Regular"));
		port.setJustificationType(juce::Justification::centredLeft);
		port.setEditable(false, false, false);
		port.setColour(juce::Label::backgroundColourId, juce::Colour(CommonColorID::neutralGrey4));
		port.setColour(juce::Label::outlineColourId, Colours::white);
		port.makeVisible(this);

		channel.setFont(juce::Font(20, juce::Font::plain).withTypefaceStyle("Regular"));
		channel.setJustificationType(juce::Justification::centredLeft);
		channel.setEditable(false, false, false);
		channel.setColour(juce::Label::backgroundColourId, juce::Colour(CommonColorID::neutralGrey4));
		channel.setColour(juce::Label::outlineColourId, Colours::white);
		channel.makeVisible(this);

		remoteControl.makeVisibleAndAddListner(this);
		latMatchDelay.makeVisible(this);
		remoteControl.showTitle(true);
		latMatchDelay.showTitle(true);

		mtcFps.makeVisible(this);
		mtcFps.setComponentID("mtcFps");
		mtcFps.setDisplayTitle(false);
		mtcFps.setJustificationType(juce::Justification::centredLeft);
		mtcFps.setTextColour(Theme::Instance().getColour(systemBlue));

		mtcFpsLable.setFont(juce::Font(font_11, juce::Font::plain).withTypefaceStyle("Regular"));
		mtcFpsLable.setJustificationType(juce::Justification::centredLeft);
		mtcFpsLable.setEditable(false, false, false);
		mtcFpsLable.setColour(juce::Label::backgroundColourId, juce::Colour(CommonColorID::neutralGrey4));
		mtcFpsLable.setColour(juce::Label::outlineColourId, Colours::white);
		mtcFpsLable.makeVisible(this);

		midiOptCtl = IgniteControlOptions::GetAppOptInstance();
		addChildComponent(midiOptCtl);
		midiOptCtl->setBounds(xStartOptctl, yStartOptctl, widthOptctl, heightOptctlt);
		midiOptCtl->addOptCtlListener(this);

		portOptionsBox.items.add(FlexItem(latMatchDelay).withMargin(FlexItem::Margin(20, 10, 5, 5)).withFlex(1));
		portOptionsBox.items.add(FlexItem(remoteControl).withMargin(FlexItem::Margin(20, 30, 5, 5)).withFlex(1));
		portOptionsBox.items.add(FlexItem(mtcFpsLable).withMargin(FlexItem::Margin(20, 30, 5, 5)).withFlex(1));
		portOptionsBox.items.add(FlexItem(mtcFps).withMargin(FlexItem::Margin(20, 10, 5, 5)).withFlex(1));

		addAndMakeVisible(view);
		addAndMakeVisible(container);
		view.setViewedComponent(&container, true);
		view.setBounds(5, 116, 1020, 357);

		view.setScrollBarsShown(true, false, true, false);
		view.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all);

		popupPtr = CommonPopUp::getCommonPopupInstance();
		AppOptions::GetOptionsInstance()->changebtnstate(ui::IgniteControlOptions::MIDI_SONG, true);
		for (uint8_t i = 0; i < mt::Ignite::midiPortList.size(); i++)
		{
			auto tmpPort = new IgniteMidiPortConfig(mt::Ignite::midiPortList[i], midiPortEngineId(mt::Ignite::midiPortList[i]));
			addChildComponent(tmpPort);
			tmpPort->setBounds(5, 60 * i, 1004, 60);
			container.addAndMakeVisible(tmpPort);
			midiPorts.push_back(tmpPort);
		}
		container.setSize(1010, 890);
		getAssets();
		mtcFps.onValueChanged = [this](int value)
		{
		};
	}

	IgniteMidiConfig::~IgniteMidiConfig()
	{
	}

	void IgniteMidiConfig::updateMidiPorts()
	{
		auto index = 0;
		for (const auto& iter : midiPorts)
		{
			container.removeChildComponent(index++);
			(void)iter;
		}
		midiPorts.clear();
		for (uint8_t i = 0; i < mt::Ignite::midiPortList.size(); i++)
		{
			auto tmpPort = new IgniteMidiPortConfig(mt::Ignite::midiPortList[i], midiPortEngineId(mt::Ignite::midiPortList[i]));
			addChildComponent(tmpPort);
			tmpPort->setBounds(5, 60 * i, 1004, 60);
			container.addAndMakeVisible(tmpPort);
			midiPorts.push_back(tmpPort);
		}
	}

	void  IgniteMidiConfig::refreshUI()
	{
		auto index = 0;

		(AppOptions::GetOptionsInstance()->getBtnState(IgniteControlOptions::MIDI_SONG)) ?
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::SettingDataLocation, 1) :
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::SettingDataLocation, 0);

		for (auto& midiConfigPort : midiPorts)
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiActivePort, index++);
			midiConfigPort->updateMidiData();
		}
	}

	void IgniteMidiConfig::getAssets()
	{
		imageMidithBg = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("midi-th-bg.png"));
		imageQuestionDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("question-circle.png"));
		imageQuestionHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("question-circle-hover.png"));
	}
	void IgniteMidiConfig::paint(juce::Graphics& g)
	{
		if (midiConfigDataUpdated &&  !isAutosave)
			MenuMode::GetMenuModeInstance()->setHeader("MIDI Configuration*");
		else
			MenuMode::GetMenuModeInstance()->setHeader("MIDI Configuration");

		g.drawImage(imageMidithBg, ui::Rectangle<float>{ 10, 64, 1004, 52 });
		g.drawImage((helpInfoButton.isMouseOver() || helpInfoButton.isFocused) ? imageQuestionHover:imageQuestionDefault , ui::Rectangle<float>{ 575, 80, 21, 21 }, RectanglePlacement::centred);
	}

	void IgniteMidiConfig::resized()
	{
		midiPort.setBounds(15, 75, 100, 30);
		//mclkSend.setBounds(210, 75, 100, 30);
		softThru.setBounds(475, 75, 100, 30);
		helpInfoButton.setBounds(575, 80, 21, 21);
		port.setBounds(705, 75, 100, 30);
		channel.setBounds(870, 75, 150, 30);
	}

	void IgniteMidiConfig::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		if (buttonThatWasClicked == &helpInfoButton)
		{
			CommonHelpInfoPopUp* ptr = new CommonHelpInfoPopUp("Thru Mode", 604, 360);
			String helpInfo = "Soft Thru type selection\n\nloopback - To same midi port\n<thru-port> thru channel - To selected port, but same channel\n<thru port> Only -> <thru channel> - To selected port, only the selected channel, unchanged\n<thru port> All -> <thru channel> - To selected port, every incoming channel remapped to the selected channel";
			ptr->setInfoText(helpInfo);
			ptr->show();
		}
	}

	void IgniteMidiConfig::enableControlButton(IgniteControlOptions::eKey key, bool state)
	{
		midiOptCtl->enableKey(key, state);
	}

	void IgniteMidiConfig::saveMidiConfigSettings(int8_t SaveLocation)
	{
		_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramSaveMidiCnfgSetting, SaveLocation);
		midiOptCtl->enableKey(IgniteControlOptions::SAVE, false);
		midiOptCtl->enableKey(IgniteControlOptions::UNDO, false);
		midiConfigDataUpdated = false;
	}
	void IgniteMidiConfig::onOptCtlButtonClicked(IgniteControlOptions::eKey option)
	{
		if (option == IgniteControlOptions::SAVEALLON)
		{
			midiConfigDataUpdated = false;
		}
		else if (option == IgniteControlOptions::AUTOSAVEON)
		{
			isAutosave = true;
		}
		else if (option == IgniteControlOptions::AUTOSAVEOFF)
		{
			isAutosave = false;
		}

		if (this->isVisible())
		{
			if (option == IgniteControlOptions::SAVE)
			{
				if (popupPtr->isDialogOpen.get() != true)
				{
					popupPtr->close();
					popupPtr->setPopUpPropories("Save ", 604, 290);
					auto saveMsg = "";
					(AppOptions::GetOptionsInstance()->getBtnState(IgniteControlOptions::MIDI_SONG)) ? (saveMsg = "Save Active Song Config edits ?") :
						(saveMsg = "Save Deafult Config edits ?");

					popupPtr->setbtnsize(265, 44);
					popupPtr->setInfoText(String(saveMsg));
					popupPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
					popupPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
					popupPtr->popupType = POPUP_WITHOKCANCEL;
					popupPtr->isCloseButtonVisible(false);
					popupPtr->show();
					popupPtr->onSelectionChanged = [this](int r)
					{
						if (r == 1)
						{
							(AppOptions::GetOptionsInstance()->getBtnState(IgniteControlOptions::MIDI_SONG)) ? saveMidiConfigSettings(1) : saveMidiConfigSettings(0);
						}

						if (auto dw = getCurrentlyModalComponent())
							dw->exitModalState(0);

						(AppOptions::GetOptionsInstance()->getBtnState(IgniteControlOptions::MIDI_SONG)) ?
							_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::SettingDataLocation, 1) :
							_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::SettingDataLocation, 0);
					};
				}
			}

			else if (option == IgniteControlOptions::UNDO)
			{
				if (midiConfigDataUpdated)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramUndoCmd, 0);

					std::vector<IgniteParamList> midiParamList;
					IgniteParamList params;

					params.param = (paramId_t)SettingsMidiCnfgParam::paramUndoListCmd;
					midiParamList.push_back(params);
					midiParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, midiParamList);
					if (midiParamList.size() == 1)
					{
						midiConfigDataUpdated = false;
						midiOptCtl->enableKey(IgniteControlOptions::SAVE, false);
						midiOptCtl->enableKey(IgniteControlOptions::UNDO, false);
					}
					refreshUI();
				}
			}
			else if (option == IgniteControlOptions::MIDI_GLOBAL)
			{
				AppOptions::GetOptionsInstance()->changebtnstate(ui::IgniteControlOptions::MIDI_SONG, false);
				saveMidiConfigSettings(1);
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::SettingDataLocation, 0);
				refreshUI();
			}
			else if (option == IgniteControlOptions::MIDI_SONG)
			{
				AppOptions::GetOptionsInstance()->changebtnstate(ui::IgniteControlOptions::MIDI_GLOBAL, false);
				saveMidiConfigSettings(0);
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::SettingDataLocation, 1);
				refreshUI();
			}
		}
	}

	int IgniteMidiConfig::isEditing()
	{
		// here isEditing is used to change current port based on mouse click 
		for (int i = 0;i < midiPorts.size(); i++)
		{
			if (midiPorts[i] == EditPtr)
			{
				currentPort = (uint8_t)i;
				return true;
			}
		}
		return 0;
	}

	bool IgniteMidiConfig::PannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		bool setfocus = true;

		if (popupPtr != nullptr && popupPtr->IsEditing())
			return popupPtr->FPKeyHandler(true, param, KeyValue);

		if ((FrontPannelParam)param == FrontPannelParam::paramKey_PAGESHIFT)
		{
			PannelSendKey(false, param, KeyValue);
			if (option_handler_focus == MIDI_DETAIL)
			{
				option_handler_focus = MIDI_OPTION;
			}
			else
			{
				option_handler_focus = MIDI_DETAIL;
			}
			PannelSendKey(true, param, KeyValue);
			return true;
		}

		if (option_handler_focus == MIDI_DETAIL)
		{
			if (KeyFocusedItem == MidiConfigItemID::NO_FOCUS)
			{
				/*1st key after Page visible */

				KeyFocusedItem = MidiConfigItemID::THRU_MODE;
				if (FrontPannelParam::paramKey_RETURN_NAV == (FrontPannelParam)param)
					AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::MENU_WINDOW);
			}
			else
			{
				if (EditPtr !=nullptr)
				{
					if (midiPorts[currentPort] != EditPtr)
						isEditing();
					EditPtr->PannelSendKey(true, param, KeyValue);
					return true;
				}

				PannelSendKey(false, param, KeyValue); // clear last Focus

				if ((KeyFocusedItem == THRU_CHANNEL) && (currentPort + 1) < midiPorts.size() && (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT))
				{
					KeyFocusedItem = THRU_MODE - 1;
					currentPort += 1;
					if (currentPort == 6)
					{
						auto newPosition = view.getPosition();
						newPosition.setY(newPosition.getY() + 250);
						view.setViewPosition(newPosition);
					}
				}
				if ((KeyFocusedItem == THRU_MODE) && (param == (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT))
				{
					if ((currentPort) > 0)
						currentPort -= 1;
					KeyFocusedItem = THRU_CHANNEL;

					if (currentPort == 5)
					{
						auto newPosition = view.getPosition();
						newPosition.setY(newPosition.getY() - 250);
						view.setViewPosition(newPosition);
					}
				}
				if (param == (paramId_t)FrontPannelParam::paramRot_ROW && (((KeyValue == ROTARY_RIGHT || KeyValue == ROTARY_FASTRIGHT || KeyValue == ROTARY_SUPERFASTRIGHT))))
				{
					if (currentPort < midiPorts.size()-1)
					{
						currentPort++;
						
					}
					else
					{
						currentPort = 0;
					}
					if (currentPort == 6)
					{
						auto newPosition = view.getPosition();
						newPosition.setY(newPosition.getY() + 250);
						view.setViewPosition(newPosition);
						repaint();
					}
					if (currentPort == 0)
					{
						auto newPosition = view.getPosition();
						newPosition.setY(0);
						view.setViewPosition(newPosition);

					}
					KeyFocusedItem = MidiConfigItemID::NO_FOCUS + 1;
				}
				else if(param == (paramId_t)FrontPannelParam::paramRot_ROW && ((KeyValue == ROTARY_LEFT || KeyValue == ROTARY_FASTLEFT || KeyValue == ROTARY_SUPERFASTLEFT)))
				{
					if (currentPort > 0)
						currentPort--;
					else
						currentPort = static_cast<uint8_t>(midiPorts.size())-1;
					if (currentPort == 5 || currentPort == mt::Ignite::midiPortList.size()-1)
					{
						auto newPosition = view.getPosition();
						newPosition.setY(newPosition.getY() - 250);
						view.setViewPosition(newPosition);
					}

					if (currentPort == 0)
					{
						auto newPosition = view.getPosition();
						newPosition.setY(0);
						view.setViewPosition(newPosition);
						
					}
					KeyFocusedItem = MidiConfigItemID::NO_FOCUS + 1;
				}
				switch ((FrontPannelParam)param)
				{
				case FrontPannelParam::paramKey_RMOVELEFT:
					KeyFocusedItem = (KeyFocusedItem + 1 >= MidiConfigItemID::FOCUS_MAX ? (MidiConfigItemID::FOCUS_MAX) : KeyFocusedItem + 1);
					break;
				case FrontPannelParam::paramKey_RMOVERIGHT:
					KeyFocusedItem = (KeyFocusedItem - 1 <= MidiConfigItemID::NO_FOCUS ? (MidiConfigItemID::NO_FOCUS + 1) : KeyFocusedItem - 1);
					break;
				case FrontPannelParam::paramKey_RETURN_NAV:
					AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::MENU_WINDOW);
					break;
				}
			}
		}
		else
		{
			PannelSendKey(false, param, KeyValue);
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				OptFocudedItem = (OptFocudedItem + 1 >= MIDIOptionsViewItemID::OP_FOCUS_MAX ? (MIDIOptionsViewItemID::OP_FOCUS_MAX) : OptFocudedItem + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				OptFocudedItem = (OptFocudedItem - 1 <= MIDIOptionsViewItemID::OP_NO_FOCUS ? (MIDIOptionsViewItemID::OP_NO_FOCUS + 1) : OptFocudedItem - 1);
				break;
			case FrontPannelParam::paramKey_RETURN_NAV:
				AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::MENU_WINDOW);
				break;

			}
		}	
		
		if (setfocus)
		{
			PannelSendKey(true, param, KeyValue);
		}
		return true;
	}


	void IgniteMidiConfig::PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		bool state=false;
		if (option_handler_focus == MIDI_DETAIL)
		{
			switch (MidiConfig[KeyFocusedItem])
			{
			case THRU_MODE:
			case THRU_PORT:
			case THRU_CHANNEL:
				return midiPorts[currentPort]->PannelSendKey(isFocuse, param, KeyValue);
				break;
			default:
				break;
			}
		}
		else
		{
			switch (OptionsView[OptFocudedItem])
			{
			case MIDI_UNDO:
				state=midiOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::UNDO, isFocuse, param, KeyValue);
				break;
			case MIDI_DEFULT:
				state=midiOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MIDI_GLOBAL, isFocuse, param, KeyValue);
				break;
			case MIDI_ACTIVE_SONG:
				state=midiOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::MIDI_SONG, isFocuse, param, KeyValue);
				break;
			default:
				break;
			}
			if (state == false)
			{
				OptFocudedItem++;
				if (OptFocudedItem == OP_FOCUS_MAX)
					OptFocudedItem = MIDI_UNDO;
				PannelSendKey(isFocuse, param, KeyValue);
			}

		}
		
	}

	////////////////////////////////////////////////
//   Class: Ignite Midi Port Config
////////////////////////////////////////////////
	IgniteMidiPortConfig::IgniteMidiPortConfig(String port_name, uint8_t port_number)
	{
		active_port_num = port_number;

		midiPortNumber.setFont(juce::Font(font_15, juce::Font::plain).withTypefaceStyle("Regular"));
		midiPortNumber.setJustificationType(juce::Justification::centredLeft);
		midiPortNumber.setEditable(false, false, false);
		midiPortNumber.setColour(juce::Label::backgroundColourId, juce::Colour(CommonColorID::neutralGrey4));
		midiPortNumber.setColour(juce::Label::outlineColourId, Colours::white);
		midiPortNumber.makeVisible(this);
		midiPortNumber.setText(port_name, dontSendNotification);

		addAndMakeVisible(softThru);
		softThru.setDisplayFormat(CommonListBox::BoxType::withoutTitleBar);
		softThru.setComponentID("softThru");

		/*mclkSend.makeVisibleAndAddListner(this);
		mclkSend.setClickingTogglesState(true);
		mclkSend.setToggleable(true);
		mclkSend.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());*/
		thruFts.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		/*mclkSend.setColour(juce::TextButton::textColourOnId, ui::Theme::Instance().getColour(neutralWhite));
		mclkSend.setColour(juce::TextButton::textColourOffId, ui::Theme::Instance().getColour(customOrange));*/

		addAndMakeVisible(recordEnable);
		recordEnable.setDisplayFormat(CommonListBox::BoxType::withoutTitleBar);
		recordEnable.setValueColour(Theme::Instance().getColour(systemBlue));
		recordEnable.setComponentID("recordEnable");

		//thruFts.makeVisibleAndAddListner(this);
		//thruFts.setClickingTogglesState(true);
		//thruFts.setToggleable(true);

		mtc.makeVisibleAndAddListner(this);
		mtc.setClickingTogglesState(true);
		mtc.setToggleable(true);

		addAndMakeVisible(midiPorts);
		midiPorts.setDisplayFormat(CommonListBox::BoxType::withoutTitleBar);

		Font midiChannelFont(16.0f);
		addAndMakeVisible(midiChannel);
		midiChannel.setDisplayTitleAndBorder(false);
		midiChannel.setTextFont(midiChannelFont);

		softThru.setValueFont(16);
		midiPorts.setValueFont(16);
		midiPorts.setJustification(Justification::centred);
		softThru.setJustification(Justification::centred);
		midiChannel.setJustification(Justification::centred);

		recordEnable.setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, juce::Rectangle{ 0,0,604,400 });
		midiPorts.setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, juce::Rectangle{ 0,0,604,400 });
		softThru.setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, juce::Rectangle{ 0,0,604,400 });
		getAssets();
		softThru.onSelectionChanged = [this](int value, const juce::String&)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiActivePort, (int8_t)active_port_num);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruType, (int8_t)value);

			IgniteMidiConfig* midiConfigComp = IgniteMidiConfig::GetAppMidiConfigInstance();
			midiConfigComp->midiConfigDataUpdated = true;
			midiConfigComp->enableControlButton(IgniteControlOptions::SAVE, true);
			midiConfigComp->enableControlButton(IgniteControlOptions::UNDO, true);
			softThruState = (uint8_t)value;
			resized();
		};

		midiPorts.onSelectionChanged = [this](int value, const juce::String&)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			IgniteMidiConfig* midiConfigComp = IgniteMidiConfig::GetAppMidiConfigInstance();
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiActivePort, (int8_t)active_port_num);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestPort, (int8_t)value);
			midiConfigComp->enableControlButton(IgniteControlOptions::SAVE, true);
			midiConfigComp->enableControlButton(IgniteControlOptions::UNDO, true);
			midiConfigComp->midiConfigDataUpdated = true;
		};

		midiChannel.onValueChanged = [this](int value)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			IgniteMidiConfig* midiConfigComp = IgniteMidiConfig::GetAppMidiConfigInstance();
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiActivePort, (int8_t)active_port_num);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestChannel, (int8_t)value);
			midiConfigComp->enableControlButton(IgniteControlOptions::SAVE, true);
			midiConfigComp->enableControlButton(IgniteControlOptions::UNDO, true);
			midiConfigComp->midiConfigDataUpdated = true;
		};

		
		midiPorts.onDailogTrigger=[this](bool status)
		{
			if(parent==nullptr)
				parent = IgniteMidiConfig::GetAppMidiConfigInstance();

			if (status)
			{
				parent->PannelSendKey(false, (paramId_t)(FrontPannelParam::paramKey_Internals), false);
				parent->EditPtr = this;
				parent->KeyFocusedItem = 2;
			}
			else
			{
				parent->EditPtr = nullptr;
			}
		};

		softThru.onDailogTrigger = [this](bool status)
		{
			if (parent == nullptr)
				parent = IgniteMidiConfig::GetAppMidiConfigInstance();

			if (status)
			{
				parent->PannelSendKey(false, (paramId_t)(FrontPannelParam::paramKey_Internals), false);
				parent->EditPtr = this;
				parent->KeyFocusedItem = 1;
			}
			else
			{
				parent->EditPtr = nullptr;
			}
		};

		midiChannel.onDailogTrigger = [this](bool status)
		{
			if (parent == nullptr)
				parent = IgniteMidiConfig::GetAppMidiConfigInstance();

			if (status)
			{
				parent->PannelSendKey(false, (paramId_t)(FrontPannelParam::paramKey_Internals), false);
				parent->EditPtr = this;
				parent->KeyFocusedItem = 3;
			}
			else
			{
				parent->EditPtr = nullptr;
			}
		};

		softThru.onFocusChanged = [this](bool a)
		{
			repaint();
		};

		midiPorts.onFocusChanged = [this](bool a)
		{
			repaint();
		};

		midiChannel.onFocusChanged = [this](bool a)
		{
			repaint();
		};
	}

	IgniteMidiPortConfig::~IgniteMidiPortConfig()
	{
	}

	void IgniteMidiPortConfig::updateMidiData()
	{
		std::vector<IgniteParamList> midiParams;
		IgniteParamList params;

		params.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestPort;
		midiParams.push_back(params);
		params.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestChannel;
		midiParams.push_back(params);
		params.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruType;
		midiParams.push_back(params);
		params.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiClkTxMode;
		midiParams.push_back(params);
		params.param = (paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruFtsOn;
		midiParams.push_back(params);

		midiParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, midiParams);

		for (auto& param_id : midiParams)
		{
			switch (param_id.param)
			{
			case (paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestPort:
				midiPorts.setSelected(param_id.MsgValue.get< int8_t >());
				break;
			case (paramId_t)SettingsMidiCnfgParam::paramConfigMidiDestChannel:
				midiChannel.setText(String(param_id.MsgValue.get< int8_t >()), dontSendNotification);
				break;
			case (paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruType:
				softThru.setSelected(param_id.MsgValue.get< int8_t >());
				softThruState = param_id.MsgValue.get< int8_t >();
				break;
			/*case (paramId_t)SettingsMidiCnfgParam::paramConfigMidiClkTxMode:
				mclkSend.setToggleState((bool)param_id.MsgValue.get< int8_t >(), dontSendNotification);
				mclkSend.setButtonText((bool)param_id.MsgValue.get< int8_t >() ? "MCLK Send:On" : "MCLK Send:Off");
				break;*/
			case (paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruFtsOn:
				thruFts.setToggleState((bool)param_id.MsgValue.get< int8_t >(), dontSendNotification);
				thruFts.setButtonText((bool)param_id.MsgValue.get< int8_t >() ? "Thru FTS:On" : "Thru FTS:Off");
				break;
			}
		}
		resized();
	}

	void IgniteMidiPortConfig::getAssets()
	{
		row1MidiConfigDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("row-midi-config-row1-default.png"));
		row1MidiConfigHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("row-midi-config-row1-default-hover.png"));
		row2MidiConfigDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("row-midi-config-row2-default.png"));
		row2MidiConfigHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("row-midi-config-row2-default-hover.png"));

		midiMediumDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-medium-default.png"));
		midiMediumHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-medium-default-hover.png"));
		midiMediumActive = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-medium-active.png"));
		midiMediumActiveHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-medium-active-hover.png"));
	
		midiLargeDefault 	=juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-large-default.png"));
		midiLargeHover		= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-large-default-1.png"));
		midiLargeActive	= juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-large-active.png"));
		midiLargeActiveHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-large-active-hover.png"));

		midiSmallDisabled = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-small-disabled.png"));
		midiSmallDefault = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-small-default.png"));
		midiSmallDefaultHover = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("btn-midi-small-default-hover.png"));
	
	
	}
	void IgniteMidiPortConfig::paint(juce::Graphics& g)
	{
		Image DrawImage;

		if (active_port_num % 2)
		{
			if (isFocused)
			{
				g.drawImage(row1MidiConfigHover, getLocalBounds().toFloat());
			}
			else
			{
				g.drawImage(row1MidiConfigDefault, getLocalBounds().toFloat());
			}
		}
		else
		{
			if (isFocused)
			{
				g.drawImage(row2MidiConfigHover, getLocalBounds().toFloat()); 
			}
			else
			{
				g.drawImage(row2MidiConfigDefault, getLocalBounds().toFloat());
			}
		}

		/*if (softThruState == (uint8_t)SoftThruType::internalSound)
			DrawImage = (mclkSend.getToggleState() ? ActiveImage : defaultImage);
		else*/
			/*DrawImage = (mclkSend.getToggleState() ? ((mclkSend.isMouseOver() || mclkSend.isFocused) ? midiMediumActiveHover : midiMediumActive) : ((mclkSend.isMouseOver() || mclkSend.isFocused) ? midiMediumHover : midiMediumDefault));
		g.drawImage(DrawImage, ui::Rectangle<float>{170, 12, 160, 32});*/

		if (softThru.getSelected() == softThruOff)
			softThru.setValueColour(Theme::Instance().getColour(customOrange));
		else
			softThru.setValueColour(Theme::Instance().getColour(neutralWhite));

		DrawImage = (softThru.getSelected() != softThruOff ? (softThru.getFocusState() ? midiLargeActiveHover : midiLargeActive) : (softThru.getFocusState() ? midiLargeHover : midiLargeDefault));
		g.drawImage(DrawImage, ui::Rectangle<float>{410, 12, 210, 32});

		if (softThruState == (uint8_t)SoftThruType::differentPortSameChannel || softThruState == (uint8_t)SoftThruType::differentChannelDifferentPort || softThruState == (uint8_t)SoftThruType::allChannelsToDestChannel)
			midiPorts.setValueColour(Theme::Instance().getColour(neutralWhite));
		else
			midiPorts.setValueColour(ui::Theme::Instance().getColour(customGrey5).withAlpha((float)1.0f));

		DrawImage = (((softThruState == (uint8_t)SoftThruType::differentPortSameChannel) || (softThruState == (uint8_t)SoftThruType::differentChannelDifferentPort) || (softThruState == (uint8_t)SoftThruType::allChannelsToDestChannel)) ? (midiPorts.getFocusState() ? midiSmallDefaultHover : midiSmallDefault) : midiSmallDisabled);
		g.drawImage(DrawImage, ui::Rectangle<float>{680, 12, 120, 32});

		if (softThruState == (uint8_t)SoftThruType::differentChannelDifferentPort || softThruState == (uint8_t)SoftThruType::allChannelsToDestChannel)
			midiChannel.setTextColour(Theme::Instance().getColour(neutralWhite));
		else
			midiChannel.setTextColour(ui::Theme::Instance().getColour(customGrey5).withAlpha((float)1.0f));

		DrawImage = ((softThruState == (uint8_t)SoftThruType::differentChannelDifferentPort || softThruState == (uint8_t)SoftThruType::allChannelsToDestChannel) ? (midiChannel.getFocusState() ? midiSmallDefaultHover : midiSmallDefault) : midiSmallDisabled);
		g.drawImage(DrawImage, ui::Rectangle<float>{865, 12, 120, 32});

		//DrawImage = (thruFts.getToggleState() ? (thruFts.isMouseOver() ? ActiveHoverImage : ActiveImage) : (thruFts.isMouseOver() ? defaultHoverImage : defaultImage));
		//g.drawImage(DrawImage, ui::Rectangle<float>{400, 12, 160, 32});
	}

	void IgniteMidiPortConfig::resized()
	{
		auto b = getLocalBounds().toFloat();
		midiPortNumber.setBounds(15, 12, 120, 32);
		//mclkSend.setBounds(170, 12, 160, 32);
		//	thruFts.setBounds(360, 12, 160, 32);
		softThru.setBounds(410, 12, 210, 32);
		midiPorts.setBounds(680, 12, 120, 32);
		midiChannel.setBounds(865, 12, 120, 32);
		//mclkSend.setEnabled(true);

		switch (softThruState)
		{
		case (uint8_t)SoftThruType::differentChannelDifferentPort:
		case (uint8_t)SoftThruType::allChannelsToDestChannel:

			midiChannel.setEnabled(true);
			midiChannel.setInterceptsMouseClicks(true, false);
		case (uint8_t)SoftThruType::differentPortSameChannel:
			midiPorts.setEnabled(true);
			midiPorts.setInterceptsMouseClicks(true, false);
			break;
		/*case (uint8_t)SoftThruType::internalSound:
			mclkSend.setEnabled(false);*/
		default:
			midiPorts.setEnabled(false);
			midiChannel.setEnabled(false);
			midiPorts.setInterceptsMouseClicks(false, false);
			midiChannel.setInterceptsMouseClicks(false, false);
		}
	}

	void IgniteMidiPortConfig::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		/*if (buttonThatWasClicked == &mclkSend)
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiActivePort, active_port_num);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiClkTxMode, mclkSend.getToggleState());
			mclkSend.setButtonText(mclkSend.getToggleState() ? "MCLK Send:On" : "MCLK Send:Off");
		}
		else */if (buttonThatWasClicked == &thruFts)
		{
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiActivePort, active_port_num);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleMidiCnfg, (paramId_t)SettingsMidiCnfgParam::paramConfigMidiThruFtsOn, thruFts.getToggleState());
			thruFts.setButtonText(thruFts.getToggleState() ? "Thru FTS:On" : "Thru FTS:Off");
		}

		IgniteMidiConfig* midiConfigComp = IgniteMidiConfig::GetAppMidiConfigInstance();
		midiConfigComp->enableControlButton(IgniteControlOptions::SAVE, true);
		midiConfigComp->enableControlButton(IgniteControlOptions::UNDO, true);
		midiConfigComp->midiConfigDataUpdated = true;
	}


	void IgniteMidiPortConfig::PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		isFocused = isFocuse;
		switch (IgniteMidiConfig::GetAppMidiConfigInstance()->MidiConfig[IgniteMidiConfig::GetAppMidiConfigInstance()->KeyFocusedItem])
		{
		/*case IgniteMidiConfig::MidiConfigItemID::MCLK_SEND:
			mclkSend.FPKeyHandler(isFocuse, param, KeyValue);
			break;*/
		case IgniteMidiConfig::MidiConfigItemID::THRU_MODE:
			softThru.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		case IgniteMidiConfig::MidiConfigItemID::THRU_PORT:
			if(midiPorts.getAlpha() == 1.0)
				midiPorts.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		case IgniteMidiConfig::MidiConfigItemID::THRU_CHANNEL:
			if (midiChannel.getAlpha() == 1.0)
				midiChannel.FPKeyHandler(isFocuse, param, KeyValue);
			break;
		default:
			break;
		}
		repaint();
	}
};
