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

namespace ui
{
	class AppDnp;
	class IgniteMidiPortConfig;
	class IgniteControlOptions;

	class IgniteMidiConfig :
		public juce::Component,
		public juce::Button::Listener,
		public IgniteControlOptions::Listener
	{
	public:
#define MIDI_CONFIG_VIEW 4
#define MIDI_OPT_VIEW 5
		enum MidiConfigItemID
		{
			NO_FOCUS = 0,
			//MCLK_SEND,
			THRU_MODE,
			THRU_PORT,
			THRU_CHANNEL,
			NEXT_PORT,
			FOCUS_MAX = NEXT_PORT
		};
		enum MIDICONFIG_FOCUS
		{
			MIDI_DETAIL,
			MIDI_OPTION
		};

		enum MIDIOptionsViewItemID
		{
			OP_NO_FOCUS = 0,
			MIDI_UNDO,
			MIDI_DEFULT,
			MIDI_ACTIVE_SONG,
			OP_FOCUS_MAX=MIDI_ACTIVE_SONG
		};
		
		uint8_t option_handler_focus = MIDI_DETAIL;
		uint8_t OptionsView[MIDI_OPT_VIEW] = { OP_NO_FOCUS,MIDI_UNDO,MIDI_DEFULT,MIDI_ACTIVE_SONG,OP_FOCUS_MAX };
		uint8_t MidiConfig[MIDI_CONFIG_VIEW] = { NO_FOCUS, /*MCLK_SEND,*/ THRU_MODE, THRU_PORT, THRU_CHANNEL };
		int KeyFocusedItem = MidiConfigItemID::NO_FOCUS;
		int OptFocudedItem = MIDI_UNDO;
		IgniteMidiPortConfig* EditPtr = nullptr;

		bool midiConfigDataUpdated = false;
		uint8_t currentPort = 0;
		bool isAutosave = false;
		IgniteMidiConfig();
		~IgniteMidiConfig() override;
		void updateMidiPorts();

		void resized() override;
		void  refreshUI();
		void enableControlButton(IgniteControlOptions::eKey key, bool state);
		int isEditing();
		bool PannelKeyHandler(paramId_t param, uint32_t KeyValue);
		void PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue);
		void getAssets();
		static IgniteMidiConfig* GetAppMidiConfigInstance()
		{
			static IgniteMidiConfig* GetAppMidiConfigObj;
			if (NULL == GetAppMidiConfigObj)
			{
				GetAppMidiConfigObj = new IgniteMidiConfig();
			}

			return GetAppMidiConfigObj;
		}

	private:
		CommonButton helpInfoButton;
		CommonNameLabel midiPort{ "Port" };
		CommonNameLabel softThru{ "Thru Mode" };
		CommonNameLabel thruFts{ "Thru FTS" };
		CommonNameLabel recordEnable{ "Rec Enable" };
		//CommonNameLabel mclkSend{ "MCLK Send" };
		CommonNameLabel sendMtc{ "Send MTC" };
		CommonNameLabel port{ "Thru Port" };
		CommonNameLabel channel{ "Thru Channel" };
		CommonButton remoteControl{ "Remote Control" };
		CommonButton latMatchDelay{ "Latency Matching Delay" };
		CommonNameLabel mtcFpsLable{ "MTC FPS :" };
		CommonNumber mtcFps{ "MTC FPS", 0, 100, 0 };
		IgniteControlOptions* midiOptCtl = nullptr;
		juce::FlexBox portConfigLableBox;
		juce::FlexBox portOptionsBox;

		std::vector<IgniteMidiPortConfig*> midiPorts;
		CommonPopUp* popupPtr = nullptr;

		juce::Viewport view;
		juce::Component container;
		Image imageMidithBg, imageQuestionDefault, imageQuestionHover;

		void paint(juce::Graphics& g) override;
		void buttonClicked(juce::Button* buttonThatWasClicked) override;
		void onOptCtlButtonClicked(IgniteControlOptions::eKey option);
		void saveMidiConfigSettings(int8_t SaveLocation);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IgniteMidiConfig)
	};

	class IgniteMidiPortConfig :
		public juce::Component,
		public juce::Button::Listener
	{
	public:
		IgniteMidiPortConfig(String port_name, uint8_t port_number);
		~IgniteMidiPortConfig() override;
		void updateMidiData();
		void PannelSendKey(bool isFocuse, paramId_t param, uint32_t KeyValue);
		void getAssets();
	private:
		CommonNameLabel midiPortNumber{ "1" };
		CommonListBox softThru{ "Thru", {"Thru Mode: Off", "loopback", "<thru port>- thru channel", "<thru port> Only -> <thru channel>", "<thru port> All -> <thru channel>"}, 0, 1 };
		//CommonToggleButton mclkSend{ "MCLK Send:Off" };
		CommonListBox recordEnable{ "Rec Enable", {"off", "on", "on/thru"}, 0, 1 };
		CommonToggleButton thruFts{ "Thru FTS:Off" };
		CommonToggleButton mtc{ "MIDI Time Code" };
		CommonListBox midiPorts{ "Soft Thru Port", mt::Ignite::portList, 0, 4 };
		CommonNumber midiChannel{ "CHANNEL", 1, 16, 1 };
		IgniteMidiConfig * parent = nullptr;
		juce::FlexBox portConfigBox;
		uint8_t softThruState = 0;
		uint8_t active_port_num = 0;
		int index;
		bool isFocused = { false };
		String softThruOff = "Thru Mode: Off";
		Image row1MidiConfigHover, row1MidiConfigDefault, row2MidiConfigHover, row2MidiConfigDefault;
		Image midiMediumDefault, midiMediumHover, midiMediumActive, midiMediumActiveHover;
		Image midiLargeDefault, midiLargeHover, midiLargeActive, midiLargeActiveHover;
		Image midiSmallDefault, midiSmallDefaultHover, midiSmallActive, midiSmallActiveHover, midiSmallDisabled;

		void paint(juce::Graphics& g) override;
		void resized() override;
		void buttonClicked(juce::Button* buttonThatWasClicked) override;
		void mouseMove(const MouseEvent& event) override
		{
			isFocused = true;
			repaint();
		};
		void mouseExit(const MouseEvent& event) override
		{
			isFocused = false;
			repaint();
		};

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IgniteMidiPortConfig)
	};
};
