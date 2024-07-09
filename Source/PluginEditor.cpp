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
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
//    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (Colours::black);
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    
    auto width = responseArea.getWidth();
    
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    // a vector will be used to store gain magnitudes of frequencies - i.e. amplitudes - processed by different filters in the chain
    std::vector<double> mags;
    // the response curve will be drawn to represent one magnitude per pixel
    mags.resize(width);
    
    // iterate across each pixel and compute the magnitude of the frequency at that pixel along the curve
    for (int pixel = 0; pixel < width; ++pixel) {
        double mag = 1.f; // starting gain of 1
        auto freq = mapToLog10((double) pixel / (double) width, 20.0, 20000.0);
        
        // the fundamental effect of our filters in this context is to change the gain of a target frequency or frequencies
        // thus, gain at a given frequency will be the product of a starting gain and the gain of that frequency after any processing done as it passes through any filters
        // we'll calculate these gain magnitudes here. if a filter is bypassed during processing of a frequency then that filter performed no processing on the frequency - we will not do calculations in those cases
        if ( !monoChain.isBypassed<ChainPositions::Peak>() )
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if ( !lowcut.isBypassed<0>() )
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if ( !lowcut.isBypassed<1>() )
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if ( !lowcut.isBypassed<2>() )
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if ( !lowcut.isBypassed<3>() )
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if ( !highcut.isBypassed<0>() )
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if ( !highcut.isBypassed<1>() )
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if ( !highcut.isBypassed<2>() )
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if ( !highcut.isBypassed<3>() )
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        mags[pixel] = Decibels::gainToDecibels(mag);
    }
    
    Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    // define map of decibel value to response area using lambda function
    // gains are from -24 to 24 as we previously defined in PluginProcessor for our gain range
    auto map = [outputMin, outputMax](double input) { return jmap(input, -24.0, 24.0, outputMin, outputMax); };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    
    for (size_t i = 1; i < mags.size(); ++i )
    {
        // connect the curve to its next point
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
    
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
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

void SimpleEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void SimpleEQAudioProcessorEditor::timerCallback()
{
    if ( parametersChanged.compareAndSetBool(false, true) )
    {
        // update the monochain
        // trigger repaint
    }
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
