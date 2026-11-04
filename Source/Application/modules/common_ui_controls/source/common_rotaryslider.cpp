/*
 * Copyright (c) - 2021 - MUSIC Tribe, MY.
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
	/////////////////////////
	///Slider
	/////////////////////////

	common_rotaryslider::common_rotaryslider(const String& Title, Slider::SliderStyle Style, Slider::TextEntryBoxPosition TextBoxPos) :
		slider(Style, Slider::NoTextBox), textBosPos(TextBoxPos)
	{
		auto theme = Theme::Instance();
		setLookAndFeel(CommonRotaryLookandFeel::getLnf());
		addAndMakeVisible(slider);
		addAndMakeVisible(title);
		addAndMakeVisible(value);
		addAndMakeVisible(knob);
		addAndMakeVisible(name);
		title.setText(Title, NotificationType::dontSendNotification);

		title.setEditable(false, false);
		slider.setNumDecimalPlacesToDisplay(0);
		knob.addMouseListener(this, true);
		// setup title ...

		title.setInterceptsMouseClicks(true, true);
		title.setJustificationType(Justification::bottom);
		title.setColour(Label::textColourId, theme.foregroundColour(2));
		//setup value ...
		value.setInterceptsMouseClicks(false, false);
		value.setJustificationType(Justification::top);
		value.setColour(Label::textColourId, theme.foregroundColour());

		knob.setColour(Label::textColourId, theme.foregroundColour(3));
		knob.setFont(Font("Arial", 12, Font::plain).withTypefaceStyle("Bold Italic"));
		knob.setVisible(false);
		knob.toFront(false);
		title.toFront(false);
		name.setJustificationType(Justification::centred);

		slider.setColour(Slider::ColourIds::rotarySliderFillColourId, ui::Theme::Instance().getColour(customBlue18));

		slider.onValueChange = [this]()
		{
			Component::BailOutChecker checker(this);

			auto newValue = slider.getValue();
			auto sValue = slider.getTextFromValue(newValue);

			if (sValue != value.getText())
				value.setText(unitVisiblity? sValue+unit : sValue, dontSendNotification);

			if (onValueChanged)
				onValueChanged(newValue);

			rotaryListeners.callChecked(checker, [this, newValue](Listener& l) {
				l.onRotaryValueChange(this, newValue);;
				});

			if (stringRangeEnabled)
				value.setText(unitVisiblity ? stringValueList[(int)newValue]+unit: stringValueList[(int)newValue], dontSendNotification);
		};

		slider.onDragEnd = [this]()
		{
			Component::BailOutChecker checker(this);

			auto newValue = slider.getValue();

			rotaryListeners.callChecked(checker, [this, newValue](Listener& l) {
				l.onRotaryDragEnd(this, newValue);;
				});
		};

		title.onTextChange = [this]()
		{
			Component::BailOutChecker checker(this);
			rotaryListeners.callChecked(checker, [this](Listener& l) {
				l.onTitleChange(this, title.getText());;
				});

		};
		title.onEditorShow = [this]()
		{
			Component::BailOutChecker checker(this);
			rotaryListeners.callChecked(checker, [this](Listener& l) {
				l.onTitleEdit(this, title.getText());;
				});

		};
		title.onMouseClick = [this]()
		{
			Component::BailOutChecker checker(this);
			rotaryListeners.callChecked(checker, [this](Listener& l) {
				l.onbuttonClicked(this, !clickState);
				clickState = !clickState;
				});
		};
	}

	void common_rotaryslider::changeDisplayName(String Name)
	{
		name.setText(Name, dontSendNotification);
		repaint();
	}
	void common_rotaryslider::setUnit(String Unit, bool setVisble)
	{
		unit = " "+Unit;
		unitVisiblity = setVisble;
		value.setText(unitVisiblity ? slider.getTextFromValue(slider.getValue()) + unit : slider.getTextFromValue(slider.getValue()) + unit, dontSendNotification);
		repaint();
	}

	void common_rotaryslider::makeVisible(Component* Parent)
	{
		Parent->addAndMakeVisible(dynamic_cast<Component&>(*this));
	}
	void common_rotaryslider::setKnobTitle(String knobTitle, bool enable)
	{
		if (enable)
		{
			knob.setText(knobTitle, NotificationType::dontSendNotification);
			knob.setVisible(true);
		}
		else
		{
			knob.setVisible(false);
		}
	}
	void common_rotaryslider::makeVisibleAndAddListner(Component* Parent)
	{
		makeVisible(Parent);
		slider.addListener(dynamic_cast<Slider::Listener*>(Parent));
	}

	void common_rotaryslider::showTitle(const bool Show, Justification justification)
	{
		bShowTitle = Show;
		title.setJustificationType(justification);
		resized();
	}

	void common_rotaryslider::addListener(Listener* l) { rotaryListeners.add(l); }
	void common_rotaryslider::removeListener(Listener* l) { rotaryListeners.remove(l); }

	void common_rotaryslider::showValue(const bool Show, Justification justification)
	{
		bShowValue = Show;
		value.setJustificationType(justification);
		resized();
	}

	void common_rotaryslider::showTitleAndValue(const bool Show, Slider::TextEntryBoxPosition ValuePos, Justification justification)
	{
		bShowTitle = bShowValue = Show;
		textBosPos = ValuePos;
		title.setJustificationType(justification);
		value.setJustificationType(justification);
		resized();
	}

	void common_rotaryslider::setDisplayProportions(const float Slider, const float Value, const float Title)
	{
		pTitle = Title;
		pSlider = Slider;
		pValue = Value;
		resized();
	}

	void common_rotaryslider::setCentered(const bool Show)
	{
		slider.getProperties().set("centered", Show);
		resized();
	}

	bool common_rotaryslider::isCentered() const {
		return slider.getProperties()["centered"];
	};

	bool common_rotaryslider::isDisplayingTitle() const { return bShowTitle; };
	bool common_rotaryslider::isDisplayingValue() const { return bShowValue; };

	double common_rotaryslider::getMin()     const { return slider.getMinimum(); }
	double common_rotaryslider::getMax()     const { return slider.getMaximum(); }
	double common_rotaryslider::getValue()   const { return slider.getValue(); }

	void common_rotaryslider::setValue(double newValue, NotificationType notification)
	{
		slider.setValue(newValue, notification);
		value.setText(slider.getTextFromValue(slider.getValue()), dontSendNotification);
	}

	void common_rotaryslider::setFloatValue(double newValue, NotificationType notification)
	{
		slider.setValue(newValue, notification);
		value.setText(String(newValue) + unit, dontSendNotification);
	}

	void common_rotaryslider::setRange(int newMin, int newMax, float newInt)
	{
		stringRangeEnabled = false;
		slider.setRange(newMin, newMax, newInt);
		value.setText(unitVisiblity? slider.getTextFromValue(newMin)+unit:slider.getTextFromValue(newMin), dontSendNotification);
		repaint();
	}

	void common_rotaryslider::setNumDecimalPlacesToDisplay(int decimalPlacesToDisplay)
	{
		slider.setNumDecimalPlacesToDisplay(decimalPlacesToDisplay);
		value.setText(unitVisiblity ? slider.getTextFromValue(slider.getValue())+unit: slider.getTextFromValue(slider.getValue()), dontSendNotification);
	}
	void common_rotaryslider::setStringLimit(std::vector<juce::String> list, int stepSize)
	{
		stringValueList = list;
		stringRangeEnabled = true;
		slider.setRange(0.0, (double)(list.size() - 1), 0.0);
		value.setText(unitVisiblity?list[0]+unit: list[0], dontSendNotification);
	}


	void common_rotaryslider::resized()
	{
		auto bounds = getLocalBounds();

		title.setVisible(bShowTitle);
		value.setVisible(bShowValue);
		knob.setVisible(true);
		float wt = .0f, ws = .0f, wv = .0f, ht = .0f, hs = .0f, hv = .0f, t, w;

		// Calculate relative positions within bounds.
		switch (textBosPos)
		{
		break; default: case Slider::NoTextBox:
			wt = ws = wv = 1.f;
			t = pSlider + (bShowTitle ? pTitle : .0f);
			ht = bShowTitle ? pTitle / t : .0f;
			hs = pSlider / t;
		break; case Slider::TextBoxBelow: case Slider::TextBoxAbove:
			wt = ws = wv = 1.f;
			t = pSlider + (bShowTitle ? pTitle : .0f) + (bShowValue ? pValue : .0f);
			ht = bShowTitle ? pTitle / t : .0f;
			hs = pSlider / t;
			hv = bShowValue ? pValue / t : .0f;
		break; case Slider::TextBoxLeft: case Slider::TextBoxRight:
			hs = 1.f;
			t = (bShowTitle ? pTitle : .0f) + (bShowValue ? pValue : .0f);
			ht = bShowTitle ? pTitle / t : .0f;
			hv = bShowValue ? pValue / t : .0f;
			w = pSlider + jmax((bShowTitle ? pTitle : .0f), (bShowValue ? pValue : .0f));
			ws = pSlider / w;
			wv = wt = (1.f - ws);
		}

		// Set relative bounds...
		switch (textBosPos)
		{
		break; default: case Slider::NoTextBox:
			title.setBoundsRelative(.0f, .0f, wt, ht);
			slider.setBoundsRelative(.0f, ht, ws, hs);
			if (bShowValue) value.setBoundsRelative(.0f, ht, ws, hs);
		break; case Slider::TextBoxBelow:
			title.setBoundsRelative(.0f, .0f, wt, ht);
			slider.setBoundsRelative(.0f, ht, ws, hs);
			value.setBoundsRelative(.0f, ht + hs, wv, hv);
		break; case Slider::TextBoxAbove:
			title.setBoundsRelative(.0f, .0f, wt, ht);
			value.setBoundsRelative(.0f, ht, wv, hv);
			slider.setBoundsRelative(.0f, ht + hv, ws, hs);
		break; case Slider::TextBoxLeft:
			title.setBoundsRelative(.0f, .0f, wt, ht);
			value.setBoundsRelative(.0f, ht, wv, hv);
			slider.setBoundsRelative(wv, .0f, ws, hs);
		break; case Slider::TextBoxRight:
			title.setBoundsRelative(ws, .0f, wt, ht);
			value.setBoundsRelative(ws, ht, wv, hv);
			slider.setBoundsRelative(.0f, .0f, ws, hs);
		}

		knob.setBoundsRelative(.45f, .26f, 0.25f, 0.78f);
		// Set font height...
		title.setFont(Font().withHeight(title.getHeight() * .9f));
		value.setFont(Font().withHeight(value.getHeight() * .7f));
		knob.setFont(Font().withHeight(knob.getHeight() * .2f));
		knob.setInterceptsMouseClicks(false, false);
		name.setBoundsRelative(.0f, 0.3f, wt, 0.14f);
	}

	void common_rotaryslider::setValueVisible(bool Value)
	{
		if (!Value)
		{
			value.setVisible(false);
			title.setEditable(false, false);
		}
		else
		{
			value.setVisible(true);
			title.setEditable(false, true);
		}
	}

	bool common_rotaryslider::FPKeyHandler(bool isFocuse, paramId_t param, uint32_t KeyValue)
	{
		switch ((FrontPannelParam)param)
		{
		case FrontPannelParam::paramKey_RSELECT:
			this->postCommandMessage(1);
			break;
		default:
				slider.setEnabled(true);
				slider.repaint();
			auto currentValue = slider.getValue();
			if (KeyValue == 1)
			{
				if (currentValue < slider.getMaximum())
				{
					slider.setValue(slider.getValue() + slider.getInterval());
					
					if (slider.onValueChange)
						slider.onValueChange();
				}
			}
			else if(KeyValue==0)
			{
				if (currentValue > slider.getMinimum())
				{
					slider.setValue(slider.getValue() - slider.getInterval());

					if (slider.onValueChange)
						slider.onValueChange();
				}
			}
			else if (KeyValue == ROTARY_FASTLEFT)
			{
				
				if (currentValue > slider.getMinimum())
				{
					float newValue = static_cast<float>(currentValue - (slider.getInterval()*ROTARY_FASTJUMP));
					if (newValue > slider.getMinimum())
						slider.setValue(newValue);
					else 
						slider.setValue(slider.getMinimum());

					if (slider.onValueChange)
						slider.onValueChange();
				}
			}
			else if (KeyValue == ROTARY_FASTRIGHT)
			{
				if (currentValue < slider.getMaximum())
				{
					float newValue = static_cast<float>(currentValue + (slider.getInterval() * ROTARY_FASTJUMP));
					if (newValue < slider.getMaximum())
						slider.setValue(newValue);
					else
						slider.setValue(slider.getMaximum());

					if (slider.onValueChange)
						slider.onValueChange();
				}
			}
			else if (KeyValue == ROTARY_SUPERFASTLEFT)
			{

				if (currentValue > slider.getMinimum())
				{
					float newValue = static_cast<float>(currentValue - (slider.getInterval() * ROTARY_SUPERFASTJUMP));
					if (newValue > slider.getMinimum())
						slider.setValue(newValue);
					else
						slider.setValue(slider.getMinimum());

					if (slider.onValueChange)
						slider.onValueChange();
				}
			}
			else if (KeyValue == ROTARY_SUPERFASTRIGHT)
			{
				if (currentValue < slider.getMaximum())
				{
					float newValue = static_cast<float>(currentValue + (slider.getInterval() * ROTARY_SUPERFASTJUMP));
					if (newValue < slider.getMaximum())
						slider.setValue(newValue);
					else
						slider.setValue(slider.getMaximum());

					if (slider.onValueChange)
						slider.onValueChange();
				}
			}
			break;
		}
		isFocused = isFocuse;
		if (isFocused)
			slider.setColour(Slider::ColourIds::rotarySliderFillColourId, ui::Theme::Instance().getColour(customBlue19));
		else
			slider.setColour(Slider::ColourIds::rotarySliderFillColourId, ui::Theme::Instance().getColour(customBlue18));
		repaint();
		
		return true;
	}

	void common_rotaryslider::handleCommandMessage(int id)
	{
		if (id == 1)
		{
			if (titleEditable)
			{
				isEditing = true;
				Component::BailOutChecker checker(this);
				rotaryListeners.callChecked(checker, [this](Listener& l) {
					l.onDoubleClicked(this);
					});
			}
		}
	}

	/////////////////////////
	///  Slider with Title
	/////////////////////////

	common_rotarysliderwithtitle::common_rotarysliderwithtitle(const String& Title, Slider::SliderStyle Style, Slider::TextEntryBoxPosition TextBoxPos) :
		common_rotaryslider(Title, Style, TextBoxPos)
	{
		showTitle(true);
	}

	/////////////////////////
	/// Rotary Slider
	/////////////////////////

	commonrotary::commonrotary(const String& Title) :
		common_rotaryslider(Title, Slider::Rotary, Slider::NoTextBox)
	{
		showTitle(true);
	}

	CommonRotaryLookandFeel::CommonRotaryLookandFeel()
	{
		auto theme = Theme::Instance();
		setColourScheme(Theme::getColourScheme());

		// Slider
		setColour(juce::Slider::backgroundColourId, theme.backgroundColour(5));

		setColour(juce::Slider::trackColourId, juce::Colour(theme.backgroundColour(6)));
		setColour(juce::Slider::thumbColourId, juce::Colour(theme.backgroundColour(6)));
		setColour(juce::Slider::rotarySliderFillColourId, theme.backgroundColour(6));
		setColour(juce::Slider::rotarySliderOutlineColourId, theme.backgroundColour(5).withAlpha(.8f));

		setColour(juce::Slider::textBoxTextColourId, theme.textColour(1));
		setColour(juce::Slider::textBoxBackgroundColourId, theme.backgroundColour(6).withAlpha((uint8)0x94));
		setColour(juce::Slider::textBoxHighlightColourId, theme.backgroundColour(1));
		setColour(juce::Slider::textBoxOutlineColourId, theme.outlineColour(0));


	}

	void CommonRotaryLookandFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
		const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
	{
		auto theme = Theme::Instance();
		auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
		auto fill = slider.findColour(Slider::rotarySliderFillColourId);

		auto bounds = Rectangle<int>(x, y, width, height).toFloat();

		auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
		auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
		auto lineW = jmin(4.2f, radius * 0.5f);
		auto arcRadius = radius - lineW * 0.5f;
		auto innerRadius = arcRadius * 2 - (3.2f * lineW);

		// Draw background arc...
		Path backgroundArc;
		backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
		g.setColour(outline);
		g.strokePath(backgroundArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));


		// Draw filled arc...
		if (slider.isEnabled())
		{
			Path valueArc;
			valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
			g.setColour(fill);
			g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
		}

		// Draw Inner Circle...
		g.setColour(theme.backgroundColour(0));
		g.fillEllipse(Rectangle<float>(innerRadius, innerRadius).withCentre(bounds.getCentre()));

		// Thumb...
		if (slider.isEnabled())
		{
			Path positionArc;
			positionArc.addRectangle(-lineW * .4f, -(innerRadius * .4f), (lineW * .5f), (innerRadius * .4f));
			positionArc.applyTransform(juce::AffineTransform::rotation(toAngle).translated(bounds.getCentreX(), bounds.getCentreY()));
			g.setColour(slider.findColour(Slider::thumbColourId));
			g.fillPath(positionArc);
		}
	}

	void common_rotaryslider::disableTitleEdit(bool state)
	{
		title.disableTitleEdit(state);
		titleEditable = state;
	}

	void common_rotaryslider::mouseDoubleClick(const MouseEvent& event)
	{
		if (titleEditable)
		{
			Component::BailOutChecker checker(this);
			rotaryListeners.callChecked(checker, [this](Listener& l) {
				l.onDoubleClicked(this);
				});
		}
	}

	void common_rotaryslider::mouseMove(const MouseEvent& event)
	{
		isFocused = true;
		slider.setColour(Slider::ColourIds::rotarySliderFillColourId, ui::Theme::Instance().getColour(customBlue19));
		slider.repaint();
	}

	void common_rotaryslider::mouseExit(const MouseEvent& event)
	{
		isFocused = false;
		slider.setColour(Slider::ColourIds::rotarySliderFillColourId, ui::Theme::Instance().getColour(customBlue18));
		slider.repaint();
	}

} // namespace ui
