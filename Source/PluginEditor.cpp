/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
        peakFreqSliderAttachment(audioProcessor.apvts, "Peak_Freq", peakFreqSlider),
        peakGainSliderAttachment(audioProcessor.apvts, "Peak_Gain", peakGainSlider),
        peakQualitySliderAttachment(audioProcessor.apvts, "Peak_Quality", peakQualitySlider),
        lowCutFreqSliderAttachment(audioProcessor.apvts, "LoCut_Freq", lowCutFreqSlider),
        highCutFreqSliderAttachment(audioProcessor.apvts, "HiCut_Freq", highCutFreqSlider),
        lowCutSlopeSliderAttachment(audioProcessor.apvts, "LoCut_Slope", lowCutSlopeSlider),
        highCutSlopeSliderAttachment(audioProcessor.apvts, "HiCut_Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for ( auto* comp : getComps() )
    {
        addAndMakeVisible(comp);
    }
    
    setSize (600, 400);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33); // the top 33% of the whole window vertically
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33); // the left 33% of the bottom 67% of the window
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5); // the right half of the remaining horizontal 67% (the right 33.5%)
    
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(bounds.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(bounds.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);
    
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33)); // the top 3rd of the remaining centre column
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5)); // the middle 3rd of the centre column
    peakQualitySlider.setBounds(bounds); // the remaining bottom middle 3rd of the window
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}
