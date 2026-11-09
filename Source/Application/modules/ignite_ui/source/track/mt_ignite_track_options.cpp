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

namespace ui
{
	extern juce::Array<mt::Ignite::instrumentsData> gInstruments;
	extern juce::Array<mt::Ignite::patternData> gPatterns;

	AppTrackOpt::AppTrackOpt(IgniteTrack* trackPtr)
	{
		_pTrack = trackPtr;
		back.makeVisibleAndAddListner(this);
		back.setColour(juce::TextButton::buttonColourId, Theme::Instance().getColour(neutralGrey1));
		back.setBounds(Rectangle(12, 11, widthTrackValueBackBtn, heightTrackValueBackBtn));

		header.makeVisible(this);
		header.setDisplayTitle(false);
		header.setEditable(false, false, false);
		header.setJustificationType(Justification::centred);
		header.setBounds(xStartSongRow, yStartSongRow + 64, widthSongRow, heightSongRow);

		addAndMakeVisible(instrumentOutBox);
		instrumentOutBox.updateListOptions(instrumentList);
		instrumentOutBox.setSelected(0);
		instrumentOutBox.setDisplayFormat(CommonListBox::BoxType::withTitleBottom);
		instrumentOutBox.setValueColour(Theme::Instance().getColour(customOrange));

		addAndMakeVisible(instrumentInBox);
		instrumentInBox.updateListOptions(instrumentList);
		instrumentInBox.setSelected(0);
		instrumentInBox.setDisplayFormat(CommonListBox::BoxType::withTitleBottom);
		instrumentInBox.setValueColour(Theme::Instance().getColour(customOrange));

		addAndMakeVisible(patternBox);
		patternBox.updateListOptions(patternList);
		patternBox.setSelected(0);
		patternBox.setDisplayFormat(CommonListBox::BoxType::withTitleBottom);
		patternBox.setValueColour(Theme::Instance().getColour(customOrange));

		addAndMakeVisible(pFillBox);
		pFillBox.updateListOptions(pFillList);
		pFillBox.setSelected(0);
		pFillBox.setDisplayFormat(CommonListBox::BoxType::withTitleBottom);
		pFillBox.setValueColour(Theme::Instance().getColour(customOrange));

		addAndMakeVisible(pTrackValuesBox);
		pTrackValuesBox.updateListOptions(pTrackValuesList);
		pTrackValuesBox.setSelected(0);
		pTrackValuesBox.setDisplayFormat(CommonListBox::BoxType::withTitleBottom);
		pTrackValuesBox.setValueColour(Theme::Instance().getColour(customOrange));

		addAndMakeVisible(soloMode);
		soloMode.addListener(this);
		soloMode.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
		soloMode.setColour(juce::TextButton::textColourOffId, ui::Theme::Instance().getColour(neutralWhite));
		soloMode.setClickingTogglesState(true);
		soloMode.setToggleable(true);
		soloMode.showBorder(false);
		soloMode.setTextColour(soloMode.getToggleState() ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));

		addAndMakeVisible(trackMuteBtn);
		trackMuteBtn.addListener(this);
		trackMuteBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
		trackMuteBtn.setColour(juce::TextButton::textColourOffId, ui::Theme::Instance().getColour(neutralWhite));
		trackMuteBtn.setClickingTogglesState(true);
		trackMuteBtn.setToggleable(true);
		trackMuteBtn.showBorder(false);
		trackMuteBtn.setTextColour(trackMuteBtn.getToggleState() ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));

		addAndMakeVisible(trackRecBtn);
		trackRecBtn.addListener(this);
		trackRecBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
		trackRecBtn.setColour(juce::TextButton::textColourOffId, ui::Theme::Instance().getColour(neutralWhite));
		trackRecBtn.setClickingTogglesState(true);
		trackRecBtn.setToggleable(true);
		trackRecBtn.showBorder(false);
		trackRecBtn.setTextColour(trackRecBtn.getToggleState() ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));

		addAndMakeVisible(trackValueBtn);
		trackValueBtn.addListener(this);
		trackValueBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
		trackValueBtn.setColour(juce::TextButton::textColourOffId, ui::Theme::Instance().getColour(customOrange));
		trackValueBtn.showBorder(false);
		trackValueBtn.setEnabled(false);
		trackValueBtn.setTooltip("Please select instrument from the instrument list");
		trackValueBtn.setHelpText("Please select instrument from the instrument list");

		trackValueptr = trackValueptr->GetAppTrackValueInstance();
		scenePtr = IgniteScene::getAppSceneInstance();

		paramBox1.items.add(FlexItem(patternBox).withMargin({ 0,10,0,0 }).withFlex(1));
		paramBox1.items.add(FlexItem(pFillBox).withMargin({ 0,10,0,0 }).withFlex(1));
		paramBox1.items.add(FlexItem(instrumentOutBox).withMargin({ 0,0,0,0 }).withFlex(1));

		paramBox2.items.add(FlexItem(trackMuteBtn).withMargin({ 0,10,0,0 }).withFlex(1));
		paramBox2.items.add(FlexItem(soloMode).withMargin({ 0,10,0,0 }).withFlex(1));
		paramBox2.items.add(FlexItem(trackValueBtn).withMargin(0).withFlex(1));

		paramBox3.items.add(FlexItem(trackRecBtn).withMargin({ 0,10,0,0 }).withFlex(1));
		paramBox3.items.add(FlexItem(instrumentInBox).withMargin(0).withFlex(1));

		trackMuteBtn.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		trackRecBtn.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		soloMode.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		trackValueBtn.setLookAndFeel(EmptyBackgroundLooknFeel::getLnf());
		repaint();

		patternBox.setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, ui::Rectangle{ 0,0,604,450 });
		pFillBox.setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, ui::Rectangle{ 0,0,604,450 });
		instrumentOutBox.setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, ui::Rectangle{ 0,0,604,450 });
		instrumentInBox.setDailogViewType(mt::Ignite::DailogViewType::CustomView, true, ui::Rectangle{ 0,0,604,450 });

		warnPopupPtr = CommonWarningPopUp::getCommonWarningPopupInstance();
		_pInfoWindow = CommonInfoWindow::GetInfoWindowInstance();

		patternBox.onFocusChanged = [this](bool a)
		{
			repaint();
		};

		pFillBox.onFocusChanged = [this](bool a)
		{
			repaint();
		};

		instrumentOutBox.onFocusChanged = [this](bool a)
		{
			repaint();
		};

		instrumentInBox.onFocusChanged = [this](bool a)
		{
			repaint();
		};

		patternBox.onApplyValueChanged = [this](int value, const juce::String& name)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;
			if ((_pTrack->getTrackData(mt::Ignite::trackDataIndex::TRACK_PATTERN).toString() != name) || (!value))
				handlePattern(value);
		};

		pFillBox.onApplyValueChanged = [this](int value, const juce::String& name)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;

			if ((_pTrack->getTrackData(mt::Ignite::trackDataIndex::TRACK_PFILL).toString() != name) || (!value))
				handleFillPattern(value);
		};

		instrumentOutBox.onApplyValueChanged = [this](int value, const juce::String& name)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;
			if ((_pTrack->getTrackData(mt::Ignite::trackDataIndex::TRACK_OUT_INSTR).toString() != name) || (!value))
				handleInstrumentDefinition(true, value);
		};

		instrumentInBox.onApplyValueChanged = [this](int value, const juce::String& name)
		{
			Component::BailOutChecker checker(this);

			if (checker.shouldBailOut())
				return;
			if ((_pTrack->getTrackData(mt::Ignite::trackDataIndex::TRACK_INSTR).toString() != name) || (!value))
				handleInstrumentDefinition(false, value);
		};

		_pTrack->setInstrDataDirtyFlag(true);
		/* ---> TODO: Remove after HW test, seting Dirty flag to load data later, not on boot time
		auto patternNameList = _PUiVirtualEngine->EngineGetPatternList(true);
		updatePatternBox(patternNameList);
		updateFillPatternBox(patternNameList); <--- */

		std::vector<IgniteParamList> instrumentParamList;
		IgniteParamList params;
		params.param = (paramId_t)InstrumentParam::paramInstrumentNameList;
		instrumentParamList.push_back(params);
		instrumentParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrumentParamList);
		updateInstrumentBox(instrumentParamList);
	}

	AppTrackOpt::~AppTrackOpt()
	{

	}

	void AppTrackOpt::addToFillList(String pattern_name)
	{
		pFillList.push_back(pattern_name);
		pFillBox.updateListOptions(pFillList, ++pfid);
		pFillBox.setSelected(0);
	}

	void AppTrackOpt::addToPatternList(String pattern_name)
	{
		patternList.push_back(pattern_name);
		patternBox.updateListOptions(patternList, pid++);

	}

	int AppTrackOpt::getPatternIndex(String pattern_name)
	{
		int index = 0;
		for (auto& patternName : patternList)
		{
			if (pattern_name == patternName)
				return index;
			index++;
		}
		return 0;
	}

	void AppTrackOpt::handleInstrumentDefinition(bool isOutput, int index)
	{
		auto instrComp = IgniteInstruments::GetAppInstrumentsInstance();

		if (index < 0)
		{
			if (isOutput)
			{
				instrumentOutBox.setSelected(0);
				_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_OUT_INSTR, " ", false);
			}
			else
			{
				instrumentInBox.setSelected(0);
				_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_INSTR, " ", false);
			}
			for (int i = 0; i < 8; i++)
				_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)((uint8_t)FrontPannelParam::paramKey_RELEASE + i), (uint16_t)0, false);
		}
		else if (!index)
		{
			std::vector<IgniteParamList> instrumentParamList;
			IgniteParamList params;
			params.param = (paramId_t)InstrumentParam::paramInstrumentNameList;
			instrumentParamList.push_back(params);
			instrComp->addInstrument(true);
			instrumentParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrumentParamList);
			updateInstrumentBox(instrumentParamList);
			if (isOutput)
			{
				instrumentOutBox.setSelected((int)instrumentList.size()) ;
				_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_OUT_INSTR, gInstruments[(int)instrumentFSNameList.size() - 1].name, false);
				instrComp->setSelected((int)instrumentList.size());
			}
			else
			{
				instrumentInBox.setSelected((int)instrumentList.size());
				_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_INSTR, gInstruments[(int)instrumentFSNameList.size() - 1].name, false);
				instrComp->setSelected((int)instrumentList.size());
			}

			for (int i = 0; i < 8; i++)
				trackValueptr->getdatafromEngine(i);
		}
		else
		{
			if (instrComp->isInstrumentUsed(gInstruments[index - 1].name))
			{
				warnPopupPtr->close();
				warnPopupPtr->setWarningText("Instrument " + instrumentList[index] + " assigned to other tracks also !!!");
				warnPopupPtr->show();
				warnPopupPtr->onSelectionChanged = [this](int r)
				{
					if (auto dw = getCurrentlyModalComponent())
						dw->exitModalState(0);
				};
			}
			if (isOutput)
			{
				instrumentOutBox.setSelected(index);
				// Same display-name/internal-name distinction as the input-instrument case below.
				_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_OUT_INSTR, gInstruments[index - 1].name, false);
			}
			else
			{
				instrumentInBox.setSelected(index);
				// instrumentList[index] is the display name; the engine's setTrackRxInstrument()
				// looks instruments up by their internal name (gInstruments[].name), not DispName.
				// Sending the display name here left trackRxInstrumentName blank on the engine side
				// whenever the two differ.
				_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_INSTR, gInstruments[index - 1].name, false);
			}
			IgniteInstruments::GetAppInstrumentsInstance()->setSelected(index);
			_PUiVirtualEngine->SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, (paramId_t)InstrumentParam::paramSetActiveInstrument, gInstruments[index - 1].name.toStdString());
			for (int i = 0; i < 8; i++)
				trackValueptr->getdatafromEngine(i);
		}

		if (!trackValueBtn.isEnabled() && isOutput)
		{
			trackValueBtn.setEnabled(true);
			trackValueBtn.setHelpText("");
		}
		if (!IgniteTrackConfig::GetAppTrackConfigInstance()->isAutoSaveEnabled())
		{
			_pTrack->trackDataUpdated = true;
			//instrumentBox.setTitleText("Instrument*");
			_pTrack->enableControlButton(IgniteControlOptions::SAVE, true);
			_pTrack->enableControlButton(IgniteControlOptions::UNDO, true);
		}

		mt::Ignite::trackData* gTrackRef = IgniteScene::getAppSceneInstance()->getTrackDataRef();
		if (gTrackRef->outputInstrument != " " && gTrackRef->pattern != " ")
		{
			trackMuteBtn.setEnabled(true);
			trackRecBtn.setEnabled(true);
			soloMode.setEnabled(true);
		}
		else
		{
			trackMuteBtn.setEnabled(false);
			trackRecBtn.setEnabled(false);
			soloMode.setEnabled(false);
		}

		if (gTrackRef->outputInstrument != " ")
			trackValueBtn.setEnabled(true);
		else
			trackValueBtn.setEnabled(false);

		IgniteSong::getAppSongInstance()->isSongSetupComplete();

		if ((gTrackRef->pattern == " " || gTrackRef->outputInstrument == " "))
		{
			setTrackBtnState(false);
			setTrackRecBtnState(false);
			setTrackSoloBtnState(false);
		}
	}

	void AppTrackOpt::handlePattern(int index)
	{
		IgnitePattern* pt = IgnitePattern::getAppPatternInstance();

		mt::Ignite::patternData pdata;
		String pName;

		if (index < 0)
		{
			patternBox.setSelected(0);
			_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_PATTERN, " ", false);
		}
		else if (!index)
		{
			pt->setDefaultPdata(&pdata, mt::Ignite::ptype::MONO);
			pt->addPattern(&pdata, true, true);
			auto patternNameList = _PUiVirtualEngine->EngineGetPatternList(true);
			updatePatternBox(patternNameList);
			updateFillPatternBox(patternNameList);
			patternBox.setSelected((int)patternNameList.size());
			_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_PATTERN, patternList[(int)patternNameList.size()], false);
			if (((int)patternNameList.size() - 1) >= 0)
			{
				IgnitePattern::getAppPatternInstance()->setPatternIndex((int)patternNameList.size() - 1);
				IgnitePattern::getAppPatternInstance()->setEditPattern((int)patternNameList.size() - 1);
			}
		}
		else
		{
			patternBox.setSelected(index);
			_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_PATTERN, patternList[index], true);
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleTrack,
				(paramId_t)TrackParam::paramTrackPattern, patternFSNameList[index].toStdString());
			if ((index - 1) >= 0)
			{
				IgnitePattern::getAppPatternInstance()->setPatternIndex(index - 1);
				IgnitePattern::getAppPatternInstance()->setEditPattern(index - 1);
			}
		}

		if (!IgniteTrackConfig::GetAppTrackConfigInstance()->isAutoSaveEnabled())
		{
			_pTrack->trackDataUpdated = true;
			//patternBox.setTitleText("Pattern*");
			_pTrack->enableControlButton(IgniteControlOptions::SAVE, true);
			_pTrack->enableControlButton(IgniteControlOptions::UNDO, true);
		}
		IgniteSong::getAppSongInstance()->isSongSetupComplete();
		mt::Ignite::trackData* gTrackRef = IgniteScene::getAppSceneInstance()->getTrackDataRef();

		if ((gTrackRef->pattern == " " || gTrackRef->outputInstrument == " "))
			setTrackBtnState(false);

		_pTrack->updateSelectionInfo();
		if (gTrackRef->outputInstrument != " " && gTrackRef->pattern != " ")
		{
			trackMuteBtn.setEnabled(true);
			trackRecBtn.setEnabled(true);
			soloMode.setEnabled(true);
		}
		else
		{
			trackMuteBtn.setEnabled(false);
			trackRecBtn.setEnabled(false);
			soloMode.setEnabled(false);
		}
		IgnitePattern::getAppPatternInstance()->changePatternViewVisibility(IgnitePattern::PatternView::LIST, true);
	}

	void AppTrackOpt::handleFillPattern(int index)
	{
		IgnitePattern* pt = IgnitePattern::getAppPatternInstance();
		mt::Ignite::patternData pdata;

		if (index < 0)
		{
			pFillBox.setSelected(0);
			_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_PFILL, " ", false);
		}
		else if (!index)
		{
			pt->setDefaultPdata(&pdata, mt::Ignite::ptype::MONO);
			pt->addPattern(&pdata, true, true);
			auto patternNameList = _PUiVirtualEngine->EngineGetPatternList(true);
			updatePatternBox(patternNameList);
			updateFillPatternBox(patternNameList);
			pFillBox.setSelected((int)patternNameList.size());
			_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_PFILL, pFillList[(int)patternNameList.size()], false);
			if (((int)patternNameList.size() - 1) >= 0)
			{
				IgnitePattern::getAppPatternInstance()->setPatternIndex((int)patternNameList.size() - 1);
				IgnitePattern::getAppPatternInstance()->setEditPattern((int)patternNameList.size() - 1);
			}
		}
		else
		{
			pFillBox.setSelected(index);
			_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_PFILL, pFillList[index], false);
		}
		if (!IgniteTrackConfig::GetAppTrackConfigInstance()->isAutoSaveEnabled())
		{
			_pTrack->trackDataUpdated = true;
			//pFillBox.setTitleText("Fill Pattern*");
			_pTrack->enableControlButton(IgniteControlOptions::SAVE, true);
			_pTrack->enableControlButton(IgniteControlOptions::UNDO, true);
		}
	}

	void AppTrackOpt::updateInstrumentBox(std::vector<IgniteParamList> instrumentParamList)
	{
		auto index = 0;
		instrumentList.clear();
		instrumentFSNameList.clear();
		instrumentList.push_back("Add New Instrument");
		instrumentInBox.updateListOptions(instrumentList, 0);
		instrumentOutBox.updateListOptions(instrumentList, 0);

		if (instrumentParamList.size() != 0)
		{
			for (auto& i : instrumentParamList)
			{
				if (i.param == (paramId_t)InstrumentParam::paramInstrumentName)
				{
					instrumentFSNameList.push_back(i.MsgValue.get<std::string>());

				}
				else if (i.param == (paramId_t)InstrumentParam::paramInstrumentDispName)
				{
					instrumentList.push_back(i.MsgValue.get<std::string>());
					instrumentInBox.updateListOptions(instrumentList, index + 1);
					instrumentOutBox.updateListOptions(instrumentList, index + 1);
					index++; // Increment index after both Name and Disp name updated
				}
			}
		}
	}

	void AppTrackOpt::updatePatternBox(std::list<std::string> patternNameList)
	{

		patternList.clear();
		patternList.push_back("Add New Pattern");
		patternFSNameList.clear();
		patternFSNameList.push_back("Add New Pattern"); // To make the index match with Disp name
		patternBox.updateListOptions(patternList, 0);

		auto patternFsList = _PUiVirtualEngine->EngineGetPatternList();
		auto index = 0;
		if (patternNameList.size() != 0)
		{
			auto itFSList = patternFsList.begin();
			for (auto& i : patternNameList)
			{
				patternList.push_back(i);
				patternBox.updateListOptions(patternList, index + 1);
				patternFSNameList.push_back(*itFSList);
				itFSList++;
				index++;
			}
		}
	}

	void AppTrackOpt::updateFillPatternBox(std::list<std::string> patternNameList)
	{

		pFillList.clear();
		pFillList.push_back("No Pattern");
		pFillBox.updateListOptions(pFillList, 0);

		auto index = 0;
		if (patternNameList.size() != 0)
		{
			for (auto& i : patternNameList)
			{
				pFillList.push_back(i);
				pFillBox.updateListOptions(patternList, index + 1);
				index++;
			}
		}
	}

	void AppTrackOpt::setTrackBtnState(bool state)
	{
		trackMuteBtn.setButtonText(state ? "Track : Enabled" : "Track : Disabled");
		_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_STATE, state, false);
		trackMuteBtn.setTextColour(state ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));
		if(!state)
			trackMuteBtn.setToggleState(false, dontSendNotification);
	}

	void AppTrackOpt::setTrackSoloBtnState(bool state)
	{
		_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_SOLO_MODE, state, false);
		soloMode.setButtonText(state ? "SOLO : Enabled" : "SOLO : Disabled");
		soloMode.setTextColour(state ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));
		if (!state)
			soloMode.setToggleState(false, dontSendNotification);
	}

	void AppTrackOpt::setTrackRecBtnState(bool state)
	{
		_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_REC_STATE, trackRecBtn.getToggleState(), false);
		trackRecBtn.setButtonText(state ? "Record : Enabled" : "Record : Disabled");
		trackRecBtn.setTextColour(state ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));
		if (!state)
			trackRecBtn.setToggleState(false, dontSendNotification);
	}

	void AppTrackOpt::buttonClicked(juce::Button* buttonThatWasClicked)
	{
		mt::Ignite::trackData* gTrackRef = IgniteScene::getAppSceneInstance()->getTrackDataRef();
		auto trackId = gTrackRef->id;
		bool* trackMutePtr = AppMainmode::getMainModeInstance()->getTrackMuteStatus();
		bool* trackSoloPtr = AppMainmode::getMainModeInstance()->getTrackSoloStatus();

		if (buttonThatWasClicked == &back)
		{
			auto songPtr = IgniteSong::getAppSongInstance();
			auto trackGridPtr = IgniteTrackGrid::GetAppTrackGridInstance(_pTrack);
			bool isTrackConfigured = songPtr->isTrackConfigured(trackId - 1);

			back.setVisible(false);
			trackGridPtr->updateTrackGridData();
			_pTrack->changeTrackOptionsVisibility(IgniteTrack::trackScreenIndex::TRACK_WINDOW);
			AppOptions::GetOptionsInstance()->taboptionChanges(mt::Ignite::tabs::TRACK, 0);
			back.setToggleState(false, dontSendNotification);
			_pTrack->setTrackData(mt::Ignite::trackDataIndex::TRACK_STATE, isTrackConfigured, false);
			trackGridPtr->setTrackBtnState(trackId, isTrackConfigured);
			trackGridPtr->setEditTrackState(trackId, isTrackConfigured);
			trackGridPtr->updateTrackGridData();
			_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam((int)(FrontPannelParam::paramKey_TRACKROTARY1)+trackId), fpLEDPattern::EN_LED_PATT_OFF);
		}
		else if (buttonThatWasClicked == &trackValueBtn)
		{
			if (gInstruments.size())
			{
				trackValueptr->setInstSoundStatus(gInstruments[instrumentOutBox.getSelectedIndex() - 1].isIntSoundEnabled);
			}
			trackValueptr->updateTrackValueScreen();
			back.setVisible(true);
		}
		else if (buttonThatWasClicked == &soloMode)
		{
			auto state = soloMode.getToggleState();
			if ((gTrackRef->pattern == " " || gTrackRef->outputInstrument == " ") && state)
			{
				soloMode.setToggleState(false, dontSendNotification);
				state = false;
				_pInfoWindow->setTextandShow("Pattern and Output Instrument not assigned");
			}
			else
			{
				if (!IgniteTrackConfig::GetAppTrackConfigInstance()->isAutoSaveEnabled())
				{
					_pTrack->trackDataUpdated = true;
					//soloMode.setButtonText("SOLO Mode*");
					_pTrack->enableControlButton(IgniteControlOptions::SAVE, true);
					_pTrack->enableControlButton(IgniteControlOptions::UNDO, true);
				}
				soloMode.setTextColour(soloMode.getToggleState() ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));

				auto mainModeInstance = AppMainmode::getMainModeInstance();
				bool fillKeyStatus = mainModeInstance->getFillKeyStatus();
				bool soloKeyStatus = mainModeInstance->getSoloKeyStatus();
				int selectedIndex = pFillBox.getSelectedIndex();
				auto paramId = static_cast<paramId_t>(FrontPannelParam((int)(FrontPannelParam::param_RGB_DISP1)+((trackId - 1) % 16)));
				auto ledPattern = fpLEDPattern::EN_LED_PATT_OFF;

				if (soloKeyStatus)
				{
					if (fillKeyStatus)
					{
						if (selectedIndex != 0)
							ledPattern = soloMode.getToggleState() ? fpLEDPattern::EN_RGB_LED_RED : fpLEDPattern::EN_RGB_LED_WHITE;
						else
							ledPattern = soloMode.getToggleState() ? fpLEDPattern::EN_RGB_LED_GREEN : fpLEDPattern::EN_LED_PATT_OFF;
					}
					else
					{
						ledPattern = soloMode.getToggleState() ? fpLEDPattern::EN_RGB_LED_GREEN : fpLEDPattern::EN_LED_PATT_OFF;
					}
				}

				_PUiVirtualEngine->setParam(static_cast<moduleId_t>(ModuleName::moduleUI), static_cast<moduleId_t>(ModuleName::moduleFrontPannel), paramId, ledPattern);
			}
			setTrackSoloBtnState(state);
			trackSoloPtr[trackId - 1] = state;
		}
		else if (buttonThatWasClicked == &trackMuteBtn)
		{
			auto state = trackMuteBtn.getToggleState();
			if ((gTrackRef->pattern == " " || gTrackRef->outputInstrument == " ") && state)
			{
				trackMuteBtn.setToggleState(false, dontSendNotification);
				state = false;
				_pInfoWindow->setTextandShow("Pattern and Output Instrument not assigned");
			}
			else
			{
				if (!IgniteTrackConfig::GetAppTrackConfigInstance()->isAutoSaveEnabled())
				{
					_pTrack->trackDataUpdated = true;
					_pTrack->enableControlButton(IgniteControlOptions::SAVE, true);
					_pTrack->enableControlButton(IgniteControlOptions::UNDO, true);
				}
				if (AppMainmode::getMainModeInstance()->getSoloKeyStatus() == false)
					_PUiVirtualEngine->setParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam((int)(FrontPannelParam::param_RGB_DISP1)+((trackId - 1) % 16)), (trackMuteBtn.getToggleState() ? fpLEDPattern::EN_RGB_LED_BLUE : fpLEDPattern::EN_LED_PATT_OFF));

			}
			setTrackBtnState(state);
			trackMutePtr[trackId - 1] = state;
		}
		else if (buttonThatWasClicked == &trackRecBtn)
		{
			auto state = trackRecBtn.getToggleState();
			if ((gTrackRef->pattern == " " || gTrackRef->outputInstrument == " ") && state)
			{
				trackRecBtn.setToggleState(false, dontSendNotification);
				state = false;
			}
			else
			{
				if (!IgniteTrackConfig::GetAppTrackConfigInstance()->isAutoSaveEnabled())
				{
					_pTrack->trackDataUpdated = true;
					_pTrack->enableControlButton(IgniteControlOptions::SAVE, true);
					_pTrack->enableControlButton(IgniteControlOptions::UNDO, true);
				}
			}
			setTrackRecBtnState(state);
		}
	}

	void AppTrackOpt::labelTextChanged(Label* labelThatHasChanged)
	{
	}

	int AppTrackOpt::getIndexs(mt::Ignite::trackDataIndex index, String name)
	{
		switch (index)
		{
		case mt::Ignite::trackDataIndex::TRACK_PATTERN:
			for (auto i = 0; i < patternList.size(); i++)
			{
				if (patternList[i] == name)
					return i;
			}
			return -1; // not found -- index 0 is a real row ("Add New Pattern..."), not a safe default
			break;
		case  mt::Ignite::trackDataIndex::TRACK_PFILL:
			for (auto i = 0; i < pFillList.size(); i++)
			{
				if (pFillList[i] == name)
					return i;
			}
			return -1;
			break;
		case mt::Ignite::trackDataIndex::TRACK_INSTR:
		case mt::Ignite::trackDataIndex::TRACK_OUT_INSTR:
			for (auto i = 0; i < instrumentList.size(); i++)
			{
				if (instrumentList[i] == name)
					return i;
			}
			return -1; // not found -- index 0 is a real row ("Add New Instrument..."), not a safe default
			break;
		}
		return -1;
	}

	bool AppTrackOpt::isAssingedPatternValid(String pattern_name)
	{
		for (auto& list : patternList)
		{
			if (pattern_name == list)
				return true;
		}
		return false;
	}

	bool AppTrackOpt::isAssingedFillPatternValid(String fillpattern_name)
	{
		for (auto& list : pFillList)
		{
			if (fillpattern_name == list)
				return true;
		}
		return false;
	}

	bool AppTrackOpt::isAssingedInstrumentValid(String instr_name)
	{
		for (auto& list : instrumentList)
		{
			if (instr_name == list)
				return true;
		}
		return false;
	}

	void AppTrackOpt::refreshData()
	{
		if (this->isVisible())
		{
			int  pattern_index, instr_index, instr_out_index, pfill_index;
			mt::Ignite::trackData* gTrackRef = IgniteScene::getAppSceneInstance()->getTrackDataRef();

			if (_pTrack->getInstrDataDirtyFlag())
			{
				auto patternNameList = _PUiVirtualEngine->EngineGetPatternList(true);
				std::vector<IgniteParamList> instrumentParamList;
				IgniteParamList params;
				params.param = (paramId_t)InstrumentParam::paramInstrumentNameList;
				instrumentParamList.push_back(params);
				instrumentParamList = _PUiVirtualEngine->getParam((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleInstrument, instrumentParamList);

				updateInstrumentBox(instrumentParamList);
				updatePatternBox(patternNameList);
				updateFillPatternBox(patternNameList);
				_pTrack->setInstrDataDirtyFlag(false);
			}
			pattern_index = getIndexs(mt::Ignite::trackDataIndex::TRACK_PATTERN, gTrackRef->pattern);
			pfill_index = getIndexs(mt::Ignite::trackDataIndex::TRACK_PFILL, gTrackRef->fillPattern);
			instr_index = getIndexs(mt::Ignite::trackDataIndex::TRACK_INSTR, gTrackRef->instrument);
			instr_out_index = getIndexs(mt::Ignite::trackDataIndex::TRACK_OUT_INSTR, gTrackRef->outputInstrument);

			header.setText(gTrackRef->name, NotificationType::dontSendNotification);
			// -1 means the rebuilt list didn't (yet) contain the track's actual assignment -- leave the
			// widget showing what it already shows rather than forcing it to row 0, which is a real,
			// actionable "Add New..." entry, not a safe "nothing selected" default.
			if (pattern_index != -1)
				patternBox.setSelected(pattern_index);
			if (pfill_index != -1)
				pFillBox.setSelected(pfill_index);
			if (instr_index != -1)
				instrumentInBox.setSelected(instr_index);
			if (instr_out_index != -1)
				instrumentOutBox.setSelected(instr_out_index);
			if (instr_out_index > 0)
			{
				trackValueBtn.setEnabled(true);
				trackValueBtn.setHelpText("");
			}
			else
			{
				trackValueBtn.setEnabled(false);
				trackValueBtn.setHelpText("Please select instrument from the instrument list");
			}

			trackValueBtn.setButtonText("Track Control Assignment");

			auto soloState = gTrackRef->soloMode;
			soloMode.setButtonText(soloState ? "SOLO : Enabled" : "SOLO : Disabled");
			soloMode.setToggleState(soloState, dontSendNotification);
			soloMode.setTextColour(soloState ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));

			auto recState = gTrackRef->recordState;
			trackRecBtn.setToggleState(recState, dontSendNotification);
			trackRecBtn.setButtonText(recState ? "Record : Enabled" : "Record : Disabled");
			trackRecBtn.setTextColour(recState ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));

			auto state = !gTrackRef->trackState;
			trackMuteBtn.setButtonText(state ? "Track : Enabled" : "Track : Disabled");
			trackMuteBtn.setToggleState(state, dontSendNotification);
			trackMuteBtn.setTextColour(state ? Theme::Instance().getColour(neutralWhite) : Theme::Instance().getColour(customOrange));

			if (gTrackRef->outputInstrument != " " && gTrackRef->pattern != " ")
			{
				trackMuteBtn.setEnabled(true);
				trackRecBtn.setEnabled(true);
				soloMode.setEnabled(true);
			}
			else
			{
				trackMuteBtn.setEnabled(false);
				trackRecBtn.setEnabled(false);
				soloMode.setEnabled(false);
			}

			// A disabled track still allows configuring it (pattern, fill pattern, in/out instrument,
			// CC assignment) -- that's a defensible, deliberate allowance, not a bug. Record and Solo
			// are the two that are genuinely ambiguous on a disabled track (does recording happen on a
			// disabled track? does soloing one?), so those two alone follow trackState. trackMuteBtn
			// itself (the enable/disable toggle) stays enabled so the track can be re-enabled.
			if (gTrackRef->trackState)
			{
				trackRecBtn.setEnabled(false);
				soloMode.setEnabled(false);
			}
		}
	}

	void AppTrackOpt::updateLableText(mt::Ignite::trackDataIndex item_id, bool state)
	{
		switch (item_id)
		{
		case mt::Ignite::trackDataIndex::TRACK_INSTR:
			state ? instrumentOutBox.setTitleText("Instrument*") : instrumentOutBox.setTitleText("Instrument");
			break;
		case mt::Ignite::trackDataIndex::TRACK_PATTERN:
			state ? patternBox.setTitleText("Pattern*") : patternBox.setTitleText("Pattern");
			break;
		case mt::Ignite::trackDataIndex::TRACK_PFILL:
			state ? pFillBox.setTitleText("Fill Pattern*") : pFillBox.setTitleText("Fill Pattern");
			break;
		case mt::Ignite::trackDataIndex::TRACK_SOLO_MODE:
			state ? soloMode.setButtonText("SOLO Mode*") : soloMode.setButtonText("SOLO Mode");
			break;
		}
	}

	void AppTrackOpt::paint(juce::Graphics& g)
	{
		Image defaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("song-bg-activesong.png"));
		g.drawImage(defaultImage, ui::Rectangle<float>{ 10, 64, 1004, 48 });

		String Imagename = patternBox.getFocusState() ? "edit-track-modalbox-hover.png" : "edit-track-modalbox-default.png";
		defaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
		g.drawImage(defaultImage, ui::Rectangle<float>{ 10, 118, 328, 100 });

		Imagename = pFillBox.getFocusState() ? "edit-track-modalbox-hover.png" : "edit-track-modalbox-default.png";
		defaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
		g.drawImage(defaultImage, ui::Rectangle<float>{ 348, 118, 328, 100 });

		Imagename = instrumentOutBox.getFocusState() ? "edit-track-modalbox-hover.png" : "edit-track-modalbox-default.png";
		defaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
		g.drawImage(defaultImage, ui::Rectangle<float>{ 686, 118, 328, 100 });

		auto state = _pTrack->getTrackData(mt::Ignite::trackDataIndex::TRACK_STATE);

		Imagename = trackMuteBtn.isEnabled() ? (trackMuteBtn.getToggleState() || state) ? ((trackMuteBtn.isMouseOver() || trackMuteBtn.isFocused) ? "edit-track-toggle-active-hover.png" : "edit-track-toggle-active.png") : ((trackMuteBtn.isMouseOver() || trackMuteBtn.isFocused) ? "edit-track-toggle-default-hover.png" : "edit-track-toggle-default.png") : "edit-track-toggle-disabled.png";
		defaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
		g.drawImage(defaultImage, ui::Rectangle<float>{ 10, 228, 326, 80 });

		Imagename = soloMode.isEnabled() ? soloMode.getToggleState() ? ((soloMode.isMouseOver() || soloMode.isFocused) ? "edit-track-toggle-active-hover.png" : "edit-track-toggle-active.png") : ((soloMode.isMouseOver() || soloMode.isFocused) ? "edit-track-toggle-default-hover.png" : "edit-track-toggle-default.png") : "edit-track-toggle-disabled.png";
		defaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
		g.drawImage(defaultImage, ui::Rectangle<float>{ 348, 228, 326, 80 });

		Imagename = trackValueBtn.isEnabled() ? trackValueBtn.getToggleState() ? ((trackValueBtn.isMouseOver() || trackValueBtn.isFocused) ? "edit-track-toggle-active-hover.png" : "edit-track-toggle-active.png") : ((trackValueBtn.isMouseOver() || trackValueBtn.isFocused) ? "edit-track-toggle-default-hover.png" : "edit-track-toggle-default.png") : "edit-track-toggle-disabled.png";
		defaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
		g.drawImage(defaultImage, ui::Rectangle<float>{ 686, 228, 326, 80 });

		Imagename = trackRecBtn.isEnabled() ? (trackRecBtn.getToggleState()) ? ((trackRecBtn.isMouseOver() || trackRecBtn.isFocused) ? "edit-track-toggle-active-hover.png" : "edit-track-toggle-active.png") : ((trackRecBtn.isMouseOver() || trackRecBtn.isFocused) ? "edit-track-toggle-default-hover.png" : "edit-track-toggle-default.png") : "edit-track-toggle-disabled.png";
		defaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
		g.drawImage(defaultImage, ui::Rectangle<float>{ 10, 318, 326, 80 });

		Imagename = instrumentInBox.getFocusState() ? "edit-track-modalbox-hover.png" : "edit-track-modalbox-default.png";
		defaultImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile(Imagename));
		g.drawImage(defaultImage, ui::Rectangle<float>{ 348, 318, 326, 80 });

		updateTrackEditIndication();
	}

	void AppTrackOpt::updateTrackEditIndication()
	{
		String songNameString = _pTrack->getTrackData(mt::Ignite::trackDataIndex::TRACK_NAME).toString();

		if (_pTrack->trackDataUpdated && !_pTrack->isGlobalAutosaveEnabled())
			songNameString = songNameString + "*";
		header.setText(songNameString, NotificationType::dontSendNotification);
	}

	void AppTrackOpt::resized()
	{
		paramBox1.performLayout(Rectangle(10, 118, 998, 100));
		paramBox2.performLayout(Rectangle(10, 228, 1002, 80));
		paramBox3.performLayout(Rectangle(10, 318, 667, 80));
		back.toFront(true);
		repaint();
	}

	bool AppTrackOpt::PannelKeyHandler(paramId_t param, uint32_t KeyValue)
	{
		bool handled = false, sendKey = true;
		int Maxitem = 7;
		auto trackGridPtr = IgniteTrackGrid::GetAppTrackGridInstance(_pTrack);

		switch ((FrontPannelParam)param)
		{
		case FrontPannelParam::paramKey_TRACKSEL1:
		case FrontPannelParam::paramKey_TRACKSEL2:
		case FrontPannelParam::paramKey_TRACKSEL3:
		case FrontPannelParam::paramKey_TRACKSEL4:
		case FrontPannelParam::paramKey_TRACKSEL5:
		case FrontPannelParam::paramKey_TRACKSEL6:
		case FrontPannelParam::paramKey_TRACKSEL7:
		case FrontPannelParam::paramKey_TRACKSEL8:
		case FrontPannelParam::paramKey_TRACKSEL9:
		case FrontPannelParam::paramKey_TRACKSEL10:
		case FrontPannelParam::paramKey_TRACKSEL11:
		case FrontPannelParam::paramKey_TRACKSEL12:
		case FrontPannelParam::paramKey_TRACKSEL13:
		case FrontPannelParam::paramKey_TRACKSEL14:
		case FrontPannelParam::paramKey_TRACKSEL15:
		case FrontPannelParam::paramKey_TRACKSEL16:
			if ((FrontPannelKeyStatus)KeyValue == FrontPannelKeyStatus::paramKeyStatusReleased)
				handled = true;
			else if ((FrontPannelKeyStatus)KeyValue == FrontPannelKeyStatus::paramKeyStatusPressed)
			{
				mt::Ignite::trackData* gTrackRef = IgniteScene::getAppSceneInstance()->getTrackDataRef(param - (paramId_t)FrontPannelParam::paramKey_TRACKSEL1);

				if (AppMainmode::getMainModeInstance()->getSoloKeyStatus())
				{
					handled = trackGridPtr->changeEditTrack(param - (paramId_t)FrontPannelParam::paramKey_TRACKSEL1);

					if ((gTrackRef->pattern == " " || gTrackRef->outputInstrument == " "))
						_pInfoWindow->setTextandShow("Pattern and Output Instrument not assigned");
					else
						soloMode.FPKeyHandler(true, (paramId_t)FrontPannelParam::paramKey_RSELECT, KeyValue);
				}
				else
				{
					if (!AppMainmode::getMainModeInstance()->getFillKeyStatus())
					{
						handled = trackGridPtr->changeEditTrack(param - (paramId_t)FrontPannelParam::paramKey_TRACKSEL1);

						if ((gTrackRef->pattern == " " || gTrackRef->outputInstrument == " "))
							_pInfoWindow->setTextandShow("Pattern and Output Instrument not assigned");
						else
						{
							trackMuteBtn.FPKeyHandler(true, (paramId_t)FrontPannelParam::paramKey_RSELECT, KeyValue);
							_pTrack->setTrackMuteDirtyFlag(true);
						}
					}
				}
			}
			break;
		case FrontPannelParam::paramKey_TRACKROTARY1:
		case FrontPannelParam::paramKey_TRACKROTARY2:
		case FrontPannelParam::paramKey_TRACKROTARY3:
		case FrontPannelParam::paramKey_TRACKROTARY4:
		case FrontPannelParam::paramKey_TRACKROTARY5:
		case FrontPannelParam::paramKey_TRACKROTARY6:
		case FrontPannelParam::paramKey_TRACKROTARY7:
		case FrontPannelParam::paramKey_TRACKROTARY8:
		case FrontPannelParam::paramKey_TRACKROTARY9:
		case FrontPannelParam::paramKey_TRACKROTARY10:
		case FrontPannelParam::paramKey_TRACKROTARY11:
		case FrontPannelParam::paramKey_TRACKROTARY12:
		case FrontPannelParam::paramKey_TRACKROTARY13:
		case FrontPannelParam::paramKey_TRACKROTARY14:
		case FrontPannelParam::paramKey_TRACKROTARY15:
		case FrontPannelParam::paramKey_TRACKROTARY16:
			if ((FrontPannelKeyStatus)KeyValue == FrontPannelKeyStatus::paramKeyStatusReleased)
				handled = true;
			else if ((FrontPannelKeyStatus)KeyValue == FrontPannelKeyStatus::paramKeyStatusPressed)
				handled = trackGridPtr->changeEditTrack(param - (paramId_t)FrontPannelParam::paramKey_TRACKROTARY1);
			break;
		}

		if (warnPopupPtr != nullptr && warnPopupPtr->IsEditing())
			return warnPopupPtr->FPKeyHandler(true, param, KeyValue);

		if (!handled)
		{
			if (track_option_handler_enabled && param != (paramId_t)FrontPannelParam::paramKey_PAGESHIFT)
			{
				handled = PannelKeyOptionsHandler(param, KeyValue);

			}
			else
			{
				if (patternBox.IsEditing() || pFillBox.IsEditing() || instrumentOutBox.IsEditing() || instrumentInBox.IsEditing())
				{
					PannelKeyAction(false, param, KeyValue);
					sendKey = false;
				}
				else
				{
					PannelKeyAction(false, param, KeyValue);
					switch ((FrontPannelParam)param)
					{
					case FrontPannelParam::paramKey_RMOVELEFT:
						FocusedIndex = jmin<int>(jmax<int>((FocusedIndex + 1), 0), (Maxitem));
						break;
					case FrontPannelParam::paramKey_RMOVERIGHT:
						FocusedIndex = jmin<int>(jmax<int>((FocusedIndex - 1), 0), (Maxitem));
						break;
					case FrontPannelParam::paramKey_RETURN_NAV:
						sendKey = false;
						buttonClicked(&back);
						break;
					case FrontPannelParam::paramKey_PAGESHIFT:
						track_option_handler_enabled ? track_option_handler_enabled = false : track_option_handler_enabled = true;
						handled = PannelKeyOptionsHandler(param, KeyValue);
						break;
					}
				}
				if (!track_option_handler_enabled && sendKey)
					handled = PannelKeyAction(true, param, KeyValue);
			}
		}

		return handled;
	}

	bool AppTrackOpt::PannelKeyAction(bool Focus, paramId_t param, uint32_t KeyValue)
	{
		Colour colour = (ui::Theme::Instance().getColour(neutralGrey4));

		if (Focus)
			colour = ui::Theme::Instance().getColour(systemOrange);

		switch (FocusedIndex)
		{
		case 0:
			patternBox.FPKeyHandler(Focus, param, KeyValue);
			patternBox.setShiftStatus((FrountPannel::GetFrountPannelInstance())->getShiftKeyStatus());
			break;
		case 1:
			pFillBox.FPKeyHandler(Focus, param, KeyValue);
			pFillBox.setShiftStatus((FrountPannel::GetFrountPannelInstance())->getShiftKeyStatus());
			break;
		case 2:
			instrumentOutBox.FPKeyHandler(Focus, param, KeyValue);
			instrumentOutBox.setShiftStatus((FrountPannel::GetFrountPannelInstance())->getShiftKeyStatus());
			break;
		case 3:
			if (trackMuteBtn.isEnabled())
			{
				trackMuteBtn.FPKeyHandler(Focus, param, 0);
				if ((FrontPannelParam)param == FrontPannelParam::paramKey_RSELECT)
					_pTrack->setTrackMuteDirtyFlag(true);
			}
			else
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					FocusedIndex++;
				else
					FocusedIndex--;

				PannelKeyAction(Focus, param, KeyValue);
			}
			break;
		case 4:
			if (soloMode.isEnabled())
			{
				soloMode.FPKeyHandler(Focus, param, 0);
			}
			else
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					FocusedIndex++;
				else
					FocusedIndex--;

				PannelKeyAction(Focus, param, KeyValue);
			}
			break;
		case 5:
			if (trackValueBtn.isEnabled())
			{
				trackValueBtn.FPKeyHandler(Focus, param, 0);
				if ((FrontPannelParam)param == FrontPannelParam::paramKey_RSELECT)
					buttonClicked(&trackValueBtn);
			}
			else
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					FocusedIndex++;
				else
					FocusedIndex--;

				PannelKeyAction(Focus, param, KeyValue);
			}
			break;
		case 6:
			if (trackRecBtn.isEnabled())
			{
				trackRecBtn.FPKeyHandler(Focus, param, KeyValue);
			}
			else
			{
				if (param == (paramId_t)FrontPannelParam::paramKey_RMOVELEFT)
					FocusedIndex++;
				else
					FocusedIndex--;

				PannelKeyAction(Focus, param, KeyValue);
			}
			break;
		case 7:
			instrumentInBox.FPKeyHandler(Focus, param, KeyValue);
			instrumentInBox.setShiftStatus((FrountPannel::GetFrountPannelInstance())->getShiftKeyStatus());
			break;
		default:
			break;
		}
		repaint();
		return true;
	}

	bool AppTrackOpt::PannelKeyOptionsHandler(paramId_t param, uint32_t KeyValue)
	{
		bool setfocus = true;
		if (KeyFocusedItem == TrackOptionsViewItemID::NO_FOCUS)
		{
			/*1st key after Page visible */
			KeyFocusedItem = TrackOptionsViewItemID::TRACK_INSTRUMENT;
		}
		else
		{
			PannelSendKey(false, param, KeyValue); // clear last Focus
			if (!track_option_handler_enabled)
				return true;
			switch ((FrontPannelParam)param)
			{
			case FrontPannelParam::paramKey_RMOVELEFT:
				KeyFocusedItem = (KeyFocusedItem + 1 >= TrackOptionsViewItemID::FOCUS_MAX ? (TrackOptionsViewItemID::FOCUS_MAX) : KeyFocusedItem + 1);
				break;
			case FrontPannelParam::paramKey_RMOVERIGHT:
				KeyFocusedItem = (KeyFocusedItem - 1 <= TrackOptionsViewItemID::NO_FOCUS ? (TrackOptionsViewItemID::NO_FOCUS + 1) : KeyFocusedItem - 1);
				break;
			case FrontPannelParam::paramKey_RETURN_NAV:
				_pTrack->changeTrackOptionsVisibility(IgniteTrack::trackScreenIndex::TRACK_WINDOW);
				AppOptions::GetOptionsInstance()->taboptionChanges(mt::Ignite::tabs::TRACK, 0);
				break;
			}
		}
		if (setfocus)
		{
			PannelSendKey(true, param, KeyValue);
		}
		return true;
	}

	void AppTrackOpt::PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		switch (OptionsView[KeyFocusedItem])
		{
		case TRACK_INSTRUMENT:
			_pTrack->trackOptCtlPtr->PannelKeyHandler(IgniteControlOptions::eKey::INST, isFocuse, param, KeyValue);
			break;
		case TRACK_PATTERN:
			_pTrack->trackOptCtlPtr->PannelKeyHandler(IgniteControlOptions::eKey::PATTERN, isFocuse, param, KeyValue);
			break;
		case TRACK_UNDO:
			_pTrack->trackOptCtlPtr->PannelKeyHandler(IgniteControlOptions::eKey::UNDO, isFocuse, param, KeyValue);
			break;
		default:
			break;
		}
	}
};
