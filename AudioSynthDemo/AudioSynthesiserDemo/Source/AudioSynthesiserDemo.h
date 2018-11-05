

#pragma once

#include "DemoUtilities.h"
#include "AudioLiveScrollingDisplay.h"

//==============================================================================
/** Our demo synth sound is just a basic sine wave.. */
struct SineWaveSound : public SynthesiserSound
{
    SineWaveSound() {}

    bool appliesToNote (int /*midiNoteNumber*/) override    { return true; }
    bool appliesToChannel (int /*midiChannel*/) override    { return true; }
};

//==============================================================================
/* Our demo synth voice just plays a sine wave.. */





//==============================================================================
// This is an audio source that streams the output of our demo synth.
struct SynthAudioSource  : public AudioSource
{
    SynthAudioSource (MidiKeyboardState& keyState)  : keyboardState (keyState)
    {
        // Add some voices to our synth, to play the sounds..
        for (auto i = 0; i < 4; ++i)
        {
           
            synth.addVoice (new SamplerVoice());    // and these ones play the sampled sounds
        }
    }


    void setUsingBell()
    {
        WavAudioFormat wavFormat;

        std::unique_ptr<AudioFormatReader> audioReader (wavFormat.createReaderFor (createAssetInputStream("H:\\Juce-spacework\\SynthDemo\\AudioSynthDemo\\AudioSynthesiserDemo\\Bell.wav"), true)); //<-- location?


        BigInteger allNotes;
        allNotes.setRange (0, 128, true);

        synth.clearSounds();
        synth.addSound (new SamplerSound ("demo sound",
                                          *audioReader,
                                          allNotes,
                                          74,   // root midi note
                                          0.1,  // attack time
                                          0.1,  // release time
                                          10.0  // maximum sample length
                                          ));
		
    }

	void setUsingGuitar()
	{
		WavAudioFormat wavFormat;

		std::unique_ptr<AudioFormatReader> audioReader(wavFormat.createReaderFor(createAssetInputStream("H:\\Juce-spacework\\SynthDemo\\AudioSynthDemo\\AudioSynthesiserDemo\\Guitar.wav"), true)); 


		BigInteger allNotes;
		allNotes.setRange(0, 128, true);

		synth.clearSounds();
		synth.addSound(new SamplerSound("demo sound",
			*audioReader,
			allNotes,
			74,   // root midi note
			0.0,  // attack time
			0.1,  // release time
			10.0  // maximum sample length
		));

	}


    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        midiCollector.reset (sampleRate);

        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    void releaseResources() override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {

        bufferToFill.clearActiveBufferRegion();

        MidiBuffer incomingMidi;

        midiCollector.removeNextBlockOfMessages (incomingMidi, bufferToFill.numSamples);

        keyboardState.processNextMidiBuffer (incomingMidi, 0, bufferToFill.numSamples, true);

        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples);
    }


    MidiMessageCollector midiCollector;

    MidiKeyboardState& keyboardState;

    Synthesiser synth;
};

//==============================================================================
class AudioSynthesiserDemo  : public Component
{
public:
    AudioSynthesiserDemo()
    {
		

        addAndMakeVisible (keyboardComponent);

        addAndMakeVisible (bellButton);
		bellButton.setRadioGroupId (321);
		bellButton.onClick = [this] { synthAudioSource.setUsingBell(); };

		addAndMakeVisible(guitarButton);
		guitarButton.setRadioGroupId(321);
		guitarButton.onClick = [this] { synthAudioSource.setUsingGuitar(); };

        addAndMakeVisible (liveAudioDisplayComp);
        audioDeviceManager.addAudioCallback (&liveAudioDisplayComp);
        audioSourcePlayer.setSource (&synthAudioSource);

		

       #ifndef JUCE_DEMO_RUNNER
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [this] (bool granted)
                                     {
                                         int numInputChannels = granted ? 2 : 0;
                                         audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                     });
       #endif

        audioDeviceManager.addAudioCallback (&audioSourcePlayer);
        audioDeviceManager.addMidiInputCallback ({}, &(synthAudioSource.midiCollector));

        setOpaque (true);
        setSize (800, 600);
    }

    ~AudioSynthesiserDemo()
    {
        audioSourcePlayer.setSource (nullptr);
        audioDeviceManager.removeMidiInputCallback ({}, &(synthAudioSource.midiCollector));
        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
        audioDeviceManager.removeAudioCallback (&liveAudioDisplayComp);
	
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        keyboardComponent   .setBounds (8, 96, getWidth() - 16, 64);
        //sineButton          .setBounds (16, 176, 150, 24);
		bellButton			.setBounds (16, 200, 150, 24);
		guitarButton	  .setBounds(16, 225, 150, 24);
        liveAudioDisplayComp.setBounds (8, 8, getWidth() - 16, 64);
    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

	AudioDeviceManager otherDeviceManager;
	std::unique_ptr <AudioDeviceSelectorComponent> audioSettings;

    MidiKeyboardState keyboardState;
    AudioSourcePlayer audioSourcePlayer;
    SynthAudioSource synthAudioSource        { keyboardState };
    MidiKeyboardComponent keyboardComponent  { keyboardState, MidiKeyboardComponent::horizontalKeyboard};

    //ToggleButton sineButton     { "Use sine wave" };
    ToggleButton  bellButton{ "Bell" };
	ToggleButton guitarButton{ "Guitar" };

    LiveScrollingAudioDisplay liveAudioDisplayComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSynthesiserDemo)
};
