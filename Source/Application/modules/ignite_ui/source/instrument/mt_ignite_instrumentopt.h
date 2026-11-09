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

#pragma once
#include <common_ui_controls/source/common_keyboard.h>

namespace ui
{
	class AppTrackOpt;
	class IgniteInstruments;
	class InstrumentPreset;

	class IgniteInstrumentOpt :
		public juce::Component,
		public CommonButton::Listener
	{
	public:
		IgniteInstrumentOpt(IgniteInstruments* iPtr);
		~IgniteInstrumentOpt() override;
		void paint(juce::Graphics& g) override;
		void resized() override;
		virtual void buttonClicked(juce::Button* buttonThatWasClicked) override;
		void refreshData();
		String getPortName(int index) { return mt::Ignite::portList[index]; }
		void updateLableText(mt::Ignite::instrumentsDataIndex item_id, bool state);
		void updateAllLablesText(bool state);
		int isEditing();
		bool PannelKeyHandler(paramId_t param, uint32_t KeyValue);
		void PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue);
		void mouseUp(const MouseEvent& event) override;
		void getAssets();
		void updateMidiPorts();

		static IgniteInstrumentOpt* GetInstrumentInstanceOpt(IgniteInstruments* tPtr)
		{
			static IgniteInstrumentOpt* instanceInstrumentOpt;
			if (NULL == instanceInstrumentOpt)
			{
				instanceInstrumentOpt = new IgniteInstrumentOpt(tPtr);
			}
			return instanceInstrumentOpt;
		}

#define INSTRUMENT_DETAL_VIEW 7
#define INSTRUMENT_ROW_ITEMS 3
		enum InstrumentDetailsViewItemID
		{
			NO_FOCUS = 0,
			PORT, //Row 1
			CHANNEL,
			FTS, //Row 4
			//PGM,
			//PGM_CHANGE, // Row 2
			//CC0_BANKL,
			//CC32_BANKM,
			INTERNAL_SOUND, // Row 3
			PRESET,
			//THRU,
			XPOSE,
			FOCUS_MAX = XPOSE
		};
		uint8_t InstrumentDetailsView[INSTRUMENT_DETAL_VIEW] = { NO_FOCUS, PORT, CHANNEL, FTS, INTERNAL_SOUND, PRESET, XPOSE };
		//uint8_t InstrumentDetailsView[INSTRUMENT_DETAL_VIEW] = { NO_FOCUS, PORT, CHANNEL, PGM, PGM_CHANGE, CC0_BANKL, CC32_BANKM, INTERNAL_SOUND, PRESET, THRU, FTS, XPOSE };
		int KeyFocusedItem = InstrumentDetailsViewItemID::NO_FOCUS;

	private:
		TooltipWindow tooltipWindow{ this, 10 };
		IgniteInstruments* instrPtr = nullptr;
		InstrumentPreset* instrPresetPtr = nullptr;
		juce::FlexBox InstrBox_colume1;
		juce::FlexBox InstrBox_colume2;
		juce::FlexBox InstrBox_colume3;
		juce::FlexBox InstrBox_colume4;
		juce::FlexBox InstrBox_colume5;
		CommonInfoWindow* _pInfoWindow = nullptr;
		IgniteControlOptions* optPtr = nullptr;

		CommonKey instrName{ "" };
		CommonImageButton xposeButton{ "XPOSE","toggle-xpose-default.png","toggle-xpose-active.png","toggle-xpose-hover.png" };
		CommonImageButton edtrkButton{ "Record Monitor","toggle-edtrk-default.png","toggle-edtrk-active.png","toggle-edtrk-hover.png"};
		CommonImageButton ftsButton{ "FTS","toggle-inst-fts-default.png","toggle-inst-fts-active.png","toggle-inst-fts-hover.png" };
		//CommonImageButton pgmButton{ "PRE SEND PGM:Off","toggle-presendpgm-default.png","toggle-presendpgm-active.png","toggle-presendpgm-hover.png" };
		CommonImageButton intSoundButton{ "INTERNAL SOUND:On","toggle-internalsound-default.png","toggle-internalsound-active.png","toggle-internalsound-hover.png" };

		CommonNumber midiChannel{ "CHANNEL", 1, 16, 0 };
		//CommonNumber cc0Button{ "PROGRAM BANK: LSB", 0, 127, 0 };
		//CommonNumber cc32Button{ "PROGRAM BANK: MSB", 0, 127, 0 };
		//CommonNumber pgmChangeButton{ "PROGRAM NUMBER", 0, 127, 0 };
		CommonListBox midiPort{ "PORT", 4 };
		Image imageDefaultImage, imageActiveImage, imageBtnActiveImage, imageBtnActiveHoverImage;
		CommonButton presetPgm{ "PRESET" };
		juce::Viewport dialogViewPort;
		int currentPreset = 0;

		void refreshInternalSoundData(bool is_internal_sound_enabled);
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IgniteInstrumentOpt)
	};
};
