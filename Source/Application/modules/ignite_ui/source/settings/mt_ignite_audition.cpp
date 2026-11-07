/*
 * Copyright (c) - 2024 - MUSIC Tribe, MY.
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
	// Widget slot i -> real engine port id (midiDataIOPorts). Slots 0-10 are DIN+USB device ports, named from
	// midiPortList; slots 11-15 are the 5 USB Host ports, which have no entry in midiPortList at all.
	static uint8_t auditionWidgetEngineId(int widgetIndex)
	{
		if (widgetIndex < (int)mt::Ignite::midiPortList.size())
		{
			const auto& name = mt::Ignite::midiPortList[widgetIndex];
			for (uint8_t idx = 0; idx < mt::Ignite::portList.size(); ++idx)
			{
				if (mt::Ignite::portList[idx] == name)
					return idx;
			}
			return 0;
		}
		return (uint8_t)midiDataIOPorts::usbHostMidi1 + (uint8_t)(widgetIndex - (int)mt::Ignite::midiPortList.size());
	}

	// Inverse of auditioningPresetParamToPort in SettingsGeneralCnfg.cpp: paramAuditioningPreset1..17 cover port ids
	// 0-16 contiguously; 18..24 were appended later (ports 17-23) and need their own base.
	static uint8_t auditioningPortToParam(uint8_t port)
	{
		if (port >= 17)
			return (uint8_t)SettingsGeneralCnfgParam::paramAuditioningPreset18 + (port - 17);
		return (uint8_t)SettingsGeneralCnfgParam::paramAuditioningPreset1 + port;
	}

	////////////////////////////////////////////////
	//   Class: IgniteAuditionSettings
	////////////////////////////////////////////////
	IgniteAuditionSettings::IgniteAuditionSettings()
	{
		auditionOptCtl = IgniteControlOptions::GetAppOptInstance();
		addChildComponent(auditionOptCtl);
		auditionOptCtl->setBounds(xStartOptctl, yStartOptctl, widthOptctl, heightOptctlt);
		auditionOptCtl->addOptCtlListener(this);

		popupPtr = CommonPopUp::getCommonPopupInstance();

		setSize(getWidth(), getHeight());
		for (auto i = 0; i < 16; i++)
		{
			String name = " USB HOST " + String(i - 10);
			if(i < 11)
				name = mt::Ignite::midiPortList[i] + " Preset";
			auto tmplistbox = new CommonListBox(name + " Preset", 2, false);
			tmplistbox->updateListOptions(mt::Ignite::auditionPresetList, 0);
			tmplistbox->setDisplayFormat(CommonListBox::BoxType::withTitleBottom);
			tmplistbox->setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, ui::Rectangle{ 0,0,604,450 });
			tmplistbox->setSelected(0);
			tmplistbox->setValueColour(Theme::Instance().getColour(customOrange));

			tmplistbox->onSelectionChangedWithName = [this, i](int index, const String& name, const String& listBoxName)
				{
					uint8_t port = auditionWidgetEngineId(i);
					uint8_t paramId = auditioningPortToParam(port);
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)paramId, index - 1);
					enableControlButton(IgniteControlOptions::SAVE, true);
					enableControlButton(IgniteControlOptions::UNDO, true);
					auditionConfigDataUpdated = true;
				};

			tmplistbox->onFocusChanged = [this](bool a)
			{
				repaint();
			};

			if (i >= 0 && i < 4)
				ctlBox1.items.add(FlexItem(*tmplistbox).withMargin(0).withFlex(1));
			else if (i >= 4 && i < 8)
				ctlBox2.items.add(FlexItem(*tmplistbox).withMargin(0).withFlex(1));
			else if (i >= 8 && i < 12)
				ctlBox3.items.add(FlexItem(*tmplistbox).withMargin(0).withFlex(1));
			else if (i >= 12 && i < 16)
				ctlBox4.items.add(FlexItem(*tmplistbox).withMargin(0).withFlex(1));

			midiPresets.push_back(tmplistbox);
			addAndMakeVisible(midiPresets[i]);
		}
		setUsbHostVisibility();
	}

	IgniteAuditionSettings::~IgniteAuditionSettings()
	{
	}

	void IgniteAuditionSettings::setUsbHostVisibility()
	{
		auto numOfHost = mt::Ignite::midiPortList.size() - 11;

		for (auto i = 0; i < 16; i++)
			midiPresets[i]->setVisible(i < 11 + numOfHost);
	}

	void IgniteAuditionSettings::updateUSBMidiName()
	{
		for (auto i = 0; i < mt::Ignite::midiPortList.size(); i++)
		{
			midiPresets[i]->setTitleText(mt::Ignite::midiPortList[i] + " Preset");
		}
		setUsbHostVisibility();
	}

	void IgniteAuditionSettings::paint(juce::Graphics& g)
	{
		if (auditionConfigDataUpdated && !isAutosave)
			MenuMode::GetMenuModeInstance()->setHeader("Audition Configuration*");
		else
			MenuMode::GetMenuModeInstance()->setHeader("Audition Configuration");

		g.setColour(ui::Theme::Instance().getColour((neutralBlack)));
		g.fillAll();

		g.drawImage(midiPresets[0]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 10, 80, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[1]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 263, 80, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[2]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 516, 80, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[3]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 769, 80, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[4]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 10, 180, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[5]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 263, 180, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[6]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 516, 180, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[7]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 769, 180, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[8]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 10, 280, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[9]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 263, 280, 243, 90 }, RectanglePlacement::centred);
		g.drawImage(midiPresets[10]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 516, 280, 243, 90 }, RectanglePlacement::centred);
		if(midiPresets[11]->isVisible())
			g.drawImage(midiPresets[11]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{769, 280, 243, 90}, RectanglePlacement::centred);
		if (midiPresets[12]->isVisible())
		g.drawImage(midiPresets[12]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 10, 380, 243, 90 }, RectanglePlacement::centred);
		if (midiPresets[13]->isVisible())
			g.drawImage(midiPresets[13]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 263, 380, 243, 90 }, RectanglePlacement::centred);
		if (midiPresets[14]->isVisible())
			g.drawImage(midiPresets[14]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 516, 380, 243, 90 }, RectanglePlacement::centred);
		if (midiPresets[15]->isVisible())
			g.drawImage(midiPresets[15]->getFocusState() ? OverImage : defaultImage, ui::Rectangle<float>{ 769, 380, 243, 90 }, RectanglePlacement::centred);
	}

	void IgniteAuditionSettings::updateData()
	{
		std::vector<IgniteParamList> audParams;
		IgniteParamList params;
		for (uint8_t widgetIndex = 0; widgetIndex < 16; widgetIndex++)
		{
			params.param = (paramId_t)auditioningPortToParam(auditionWidgetEngineId(widgetIndex));
			audParams.push_back(params);
		}

		audParams = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, audParams);

		for (auto widgetIndex = 0; widgetIndex < 16; widgetIndex++)
			midiPresets[widgetIndex]->setSelected(audParams[widgetIndex].MsgValue.get< int >() + 1);
	}

	void IgniteAuditionSettings::resized()
	{
		ctlBox1.performLayout(ui::Rectangle(10, yStartSettingBox + 10, 1012, 90));
		ctlBox2.performLayout(ui::Rectangle(10, yStartSettingBox + 110, 1012, 90));
		ctlBox3.performLayout(ui::Rectangle(10, yStartSettingBox + 210, 1012, 90));
		ctlBox4.performLayout(ui::Rectangle(10, yStartSettingBox + 310, 1012, 90));
	}

	void IgniteAuditionSettings::onOptCtlButtonClicked(IgniteControlOptions::eKey option)
	{
		if (option == IgniteControlOptions::SAVEALLON)
		{
			auditionConfigDataUpdated = false;
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
					popupPtr->setbtnsize(265, 44);
					popupPtr->setInfoText(String("Save Audition Configuration edits ? "));
					popupPtr->addImageButton("no", "btn-secondary-no-267x44-default.png", "btn-secondary-no-267x44-default-hover.png", 2);
					popupPtr->addImageButton("yes", "btn-yes-267x44-default.png", "btn-yes-267x44-default-hover.png", 1);
					popupPtr->popupType = POPUP_WITHOKCANCEL;
					popupPtr->isCloseButtonVisible(false);
					popupPtr->show();
					popupPtr->onSelectionChanged = [this](int r)
						{
							if (r == 1)
							{
								_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgSave, 0);
								enableControlButton(IgniteControlOptions::SAVE, false);
								enableControlButton(IgniteControlOptions::UNDO, false);
								auditionConfigDataUpdated = false;
							}
							if (auto dw = getCurrentlyModalComponent())
								dw->exitModalState(0);
						};
				}
			}
			else if (option == IgniteControlOptions::UNDO)
			{
				if (auditionConfigDataUpdated)
				{
					_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, (paramId_t)SettingsGeneralCnfgParam::paramUndoCmd, 0);

					std::vector<IgniteParamList> audParamList;
					IgniteParamList params;

					params.param = (paramId_t)SettingsGeneralCnfgParam::paramUndoListCmd;
					audParamList.push_back(params);
					audParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleSettingGnrlCnfg, audParamList);
					if (audParamList.size() == 1)
					{
						auditionConfigDataUpdated = false;
						enableControlButton(IgniteControlOptions::SAVE, false);
						enableControlButton(IgniteControlOptions::UNDO, false);
					}
					updateData();
				}
			}
		}
	}

	int IgniteAuditionSettings::isEditing()
	{
		int index = 0;
		midiPresets[0]->IsEditing() ? index = 1 : 0;
		midiPresets[1]->IsEditing() ? index = 2 : 0;
		midiPresets[2]->IsEditing() ? index = 3 : 0;
		midiPresets[3]->IsEditing() ? index = 4 : 0;
		midiPresets[4]->IsEditing() ? index = 5 : 0;
		midiPresets[5]->IsEditing() ? index = 6 : 0;
		midiPresets[6]->IsEditing() ? index = 7 : 0;
		midiPresets[7]->IsEditing() ? index = 8 : 0;
		midiPresets[8]->IsEditing() ? index = 9 : 0;
		midiPresets[9]->IsEditing() ? index = 10 : 0;
		midiPresets[10]->IsEditing() ? index = 11 : 0;
		midiPresets[11]->IsEditing() ? index = 12 : 0;
		midiPresets[12]->IsEditing() ? index = 13 : 0;
		midiPresets[13]->IsEditing() ? index = 14 : 0;
		midiPresets[14]->IsEditing() ? index = 15 : 0;
		midiPresets[15]->IsEditing() ? index = 16 : 0;
		return index;
	}

	void IgniteAuditionSettings::handleCommandMessage(int id)
	{
		if(id==1)
			AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::SETTING_WINDOW);
	}

	bool IgniteAuditionSettings::pannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		bool handled = false;

		if (popupPtr != nullptr && popupPtr->IsEditing() && popupPtr->popupType != POPUP_RESET)
			return popupPtr->FPKeyHandler(true, param, KeyValue);

		if ((FrontPannelParam)param == FrontPannelParam::paramKey_PAGESHIFT)
		{
			pannelSendKey(false, param, KeyValue);
			if (option_handler_focus == AUD_DETAIL)
			{
				option_handler_focus = AUD_OPTION;
			}
			else
			{
				option_handler_focus = AUD_DETAIL;
			}
			pannelSendKey(true, param, KeyValue);
			return true;
		}

		if (!handled)
		{
			bool setfocus = true;
			int EditItem = AuditionViewItemID::NO_FOCUS;

			if (KeyFocusedItem == AuditionViewItemID::NO_FOCUS)
			{
				/*1st key after Page visible */
				KeyFocusedItem = AuditionViewItemID::PORT_1_PRESET;

				if (FrontPannelParam::paramKey_RETURN_NAV == (FrontPannelParam)param)
				{
					this->postCommandMessage(1);
					return true;
				}
			}
			else
			{
				EditItem = isEditing();
				if (EditItem)
				{
					if (KeyFocusedItem != EditItem)
					{
						pannelSendKey(false, param, KeyValue);
						KeyFocusedItem = EditItem;
					}
					setfocus = true;
				}
				else
				{
					pannelSendKey(false, param, KeyValue); // clear last Focus
					switch ((FrontPannelParam)param)
					{
					case FrontPannelParam::paramKey_RMOVELEFT:
						KeyFocusedItem = (KeyFocusedItem + 1 >= AuditionViewItemID::FOCUS_MAX ? (AuditionViewItemID::FOCUS_MAX) : KeyFocusedItem + 1);
						break;
					case FrontPannelParam::paramKey_RMOVERIGHT:
						KeyFocusedItem = (KeyFocusedItem - 1 <= AuditionViewItemID::NO_FOCUS ? (AuditionViewItemID::NO_FOCUS + 1) : KeyFocusedItem - 1);
						break;
					case FrontPannelParam::paramKey_RETURN_NAV:
						this->postCommandMessage(1);
						break;
					case FrontPannelParam::paramRot_ROW:
						if ((KeyValue == ROTARY_RIGHT || KeyValue == ROTARY_FASTRIGHT || KeyValue == ROTARY_SUPERFASTRIGHT))
						{
							pannelSendKey(false, (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT, 0x47);
							KeyFocusedItem = (KeyFocusedItem + AUD_ROW_ITEMS >= AuditionViewItemID::FOCUS_MAX ? ((KeyFocusedItem + AUD_ROW_ITEMS) == (AuditionViewItemID::FOCUS_MAX) ? (AuditionViewItemID::FOCUS_MAX) : KeyFocusedItem) : KeyFocusedItem + AUD_ROW_ITEMS);
							pannelSendKey(true, (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT, 0x47);
						}
						else if ((KeyValue == ROTARY_LEFT || KeyValue == ROTARY_FASTLEFT || KeyValue == ROTARY_SUPERFASTLEFT))
						{
							pannelSendKey(false, (paramId_t)FrontPannelParam::paramKey_RMOVELEFT, 0x48);
							KeyFocusedItem = (KeyFocusedItem - AUD_ROW_ITEMS <= AuditionViewItemID::NO_FOCUS ? (KeyFocusedItem) : KeyFocusedItem - AUD_ROW_ITEMS);
							pannelSendKey(true, (paramId_t)FrontPannelParam::paramKey_RMOVELEFT, 0x48);
						}
						repaint();
						setfocus = false;
						break;
					}
				}
			}
			if (setfocus)
			{
				pannelSendKey(true, param, KeyValue);
			}
			handled = true;
		}
		else
		{
			pannelSendKey(false, param, KeyValue); // clear last Focus
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				OptFocusedItem = (OptFocusedItem + 1 >= AudOptionsViewItemID::OP_FOCUS_MAX ? (AudOptionsViewItemID::OP_FOCUS_MAX) : OptFocusedItem + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				OptFocusedItem = (OptFocusedItem - 1 <= AudOptionsViewItemID::OP_NO_FOCUS ? (AudOptionsViewItemID::OP_NO_FOCUS + 1) : OptFocusedItem - 1);
				break;
			case FrontPannelParam::paramKey_RETURN_NAV:
				AppMenu::GetAppMenuInstance()->changeMenuOptionsVisibilty(AppMenu::MENU_WINDOWS::SETTING_WINDOW);
				break;
			}
			pannelSendKey(true, param, KeyValue);
		}
		return handled;
	}

	void IgniteAuditionSettings::pannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{

		if (param == (paramId_t)FrontPannelParam::paramKey_RSELECT && (KeyValue == (uint8_t)FrontPannelKeyStatus::paramKeyStatusReleased))
			return;

		if (option_handler_focus == AUD_DETAIL)
		{
			switch (AudMenuView[KeyFocusedItem])
			{
			case PORT_1_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[0]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_2_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[1]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_3_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[2]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_4_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[3]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_5_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[4]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_6_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[5]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_7_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[6]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_8_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[7]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_9_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[8]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_10_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[9]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_11_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[10]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_12_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[11]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_13_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[12]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_14_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[13]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_15_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[14]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_16_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[15]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			case PORT_17_PRESET:
				if (popupPtr != nullptr && popupPtr->IsEditing())
				{
					popupPtr->FPKeyHandler(isFocuse, param, KeyValue);
				}
				else
					midiPresets[16]->FPKeyHandler(isFocuse, param, KeyValue);
				break;
			default:
				break;
			}
			repaint();
		}
		else
		{
			switch (OptionsView[OptFocusedItem])
			{
			case OPT_UNDO:
				auditionOptCtl->PannelKeyHandler(IgniteControlOptions::eKey::UNDO, isFocuse, param, KeyValue);
				break;
			}
		}
	}
};
