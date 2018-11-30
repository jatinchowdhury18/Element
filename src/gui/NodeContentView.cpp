
#include "controllers/AppController.h"
#include "controllers/GuiController.h"
#include "gui/LookAndFeel.h"
#include "gui/NodeContentView.h"
#include "gui/ViewHelpers.h"

namespace Element {

    static String noteValueToString (double value)
    {
        return MidiMessage::getMidiNoteName (roundToInt (value), true, true, 3);
    }

    NodeContentView::NodeContentView()
    {
        const Font font (12.f);
        setWantsKeyboardFocus (false);
        setMouseClickGrabsKeyboardFocus (false);
        setInterceptsMouseClicks (true, true);

        addAndMakeVisible (nameLabel);
        nameLabel.setText ("Name", dontSendNotification);
        nameLabel.setFont (font);
        addAndMakeVisible (nameEditor);
        

        addAndMakeVisible (transposeLabel);
        transposeLabel.setText ("Transpose", dontSendNotification);
        transposeLabel.setFont (font);
        transposeLabel.onDoubleClicked = [this](const MouseEvent&) { 
            transposeSlider.setValue (0.0, sendNotificationAsync);
        };
        addAndMakeVisible (transposeSlider);
        transposeSlider.setRange (-24, 24, 1);
        transposeSlider.setValue (0);
        transposeSlider.setSliderStyle (Slider::LinearHorizontal);
        transposeSlider.setTextBoxStyle (Slider::TextBoxRight, true, 40, 18);


        addAndMakeVisible (keyLowLabel);
        keyLowLabel.setText ("Key Start", dontSendNotification);
        keyLowLabel.setFont (font);
        keyLowLabel.onDoubleClicked = [this](const MouseEvent&) {
            keyLowSlider.setValue (0.0, sendNotificationAsync);
        };
        addAndMakeVisible (keyLowSlider);
        keyLowSlider.textFromValueFunction = noteValueToString;
        keyLowSlider.setRange (0, 127, 1.0);
        keyLowSlider.setSliderStyle (Slider::LinearHorizontal);
        keyLowSlider.setTextBoxStyle (Slider::TextBoxRight, true, 40, 18);
        keyLowSlider.setTextBoxIsEditable (false);
        keyLowSlider.setValue (0);

        addAndMakeVisible (keyHiLabel);
        keyHiLabel.setText ("Key End", dontSendNotification);
        keyHiLabel.setFont (font);
        keyHiLabel.onDoubleClicked = [this](const MouseEvent&) { 
            keyHiSlider.setValue (127.0, sendNotificationAsync); 
        };
        addAndMakeVisible (keyHiSlider);
        keyHiSlider.textFromValueFunction = noteValueToString;
        keyHiSlider.setRange (0, 127, 1.0);
        keyHiSlider.setSliderStyle (Slider::LinearHorizontal);
        keyHiSlider.setTextBoxStyle (Slider::TextBoxRight, true, 40, 18);
        keyHiSlider.setTextBoxIsEditable (false);
        keyHiSlider.setValue (127);

        addAndMakeVisible (midiChannelLabel);
        midiChannelLabel.setText ("MIDI Ch.", dontSendNotification);
        midiChannelLabel.setFont (font);
        midiChannelLabel.onDoubleClicked = [this](const MouseEvent&) {
            BigInteger chans;
            chans.setRange (0, 17, false);
            chans.setBit (0, true);
            midiChannel.setChannels (chans);
        };

        addAndMakeVisible (midiChannel);
        midiChannel.onChanged = [this]()
        {
            if (auto* o = node.getGraphNode())
            {
                o->setMidiChannels (midiChannel.getChannels());
                node.setProperty (Tags::midiChannels, midiChannel.getChannels().toMemoryBlock());

                if (auto* mb = node.getValueTree().getProperty(Tags::midiChannels).getBinaryData())
                {
                        BigInteger chans; 
                        chans.loadFromMemoryBlock (*mb);
                        DBG(node.getName());
                        for (int i = 0; i <= 16; ++i)
                            DBG("ch: " << i << " = " << (int) chans [i]);
                        // obj->setMidiChannels (chans);
                    
                }
            }
        };

        keyLowSlider.addListener (this);
        keyHiSlider.addListener (this);
        transposeSlider.addListener (this);
    }

    NodeContentView::~NodeContentView()
    {
        keyLowSlider.removeListener (this);
        keyHiSlider.removeListener (this);
        transposeSlider.removeListener (this);
        selectedNodeConnection.disconnect();
    }

    void NodeContentView::paint (Graphics& g)
    {
        g.fillAll (Element::LookAndFeel::backgroundColor);
    }

    void NodeContentView::resized()
    {
        auto r (getLocalBounds().reduced (2));
        r.removeFromTop (4);
        r.removeFromRight (4);
        layoutComponent (r, nameLabel, nameEditor);
        layoutComponent (r, midiChannelLabel, midiChannel, 
                         midiChannel.getSuggestedHeight (r.getWidth()));
        layoutComponent (r, keyLowLabel, keyLowSlider);
        layoutComponent (r, keyHiLabel, keyHiSlider);
        layoutComponent (r, transposeLabel, transposeSlider);
    }

    void NodeContentView::layoutComponent (Rectangle<int>& r, Label& l, Component& c,
                                           int preferedHeight)
    {
        static const int settingHeight = 20;
        static const int labelWidth = 64;
        static const int spacing = 6;
        auto r2 = r.removeFromTop (preferedHeight > 0 ? preferedHeight : settingHeight);
        l.setBounds (r2.removeFromLeft (labelWidth));
        c.setBounds (r2);
        r.removeFromTop (spacing);
    }

    void NodeContentView::stabilizeContent()
    {
        auto *cc = ViewHelpers::findContentComponent(this);
        jassert(cc);
        auto& gui = *cc->getAppController().findChild<GuiController>();
        if (! selectedNodeConnection.connected())
            selectedNodeConnection = gui.nodeSelected.connect (std::bind (
                &NodeContentView::stabilizeContent, this));

        node = gui.getSelectedNode();

        if (node.isValid() && ! node.isIONode())
        {
            setEnabled (true);
            nameEditor.getTextValue().referTo (node.getPropertyAsValue (Tags::name));
            updateMidiChannels();
            updateSliders();
        }
        else
        {
            if (node.isValid() && node.isIONode())
            {
                nameEditor.getTextValue().referTo (node.getPropertyAsValue (Tags::name));
            }
            else
            {
                nameEditor.getTextValue().referTo (Value());
                nameEditor.setText (String(), false);
            }
            
            setEnabled (false);
        }
    }

    void NodeContentView::sliderValueChanged (Slider* slider)
    {
        GraphNodePtr object = node.getGraphNode();
        if (object == nullptr || ! node.isValid())
            return;
        
        if (slider == &keyLowSlider)
        {
            auto keyRange (object->getKeyRange());
            keyRange.setStart (roundToInt (slider->getValue()));
            object->setKeyRange (keyRange);
        }
        else if (slider == &keyHiSlider)
        {
            auto keyRange (object->getKeyRange());
            keyRange.setEnd (roundToInt (slider->getValue()));
            object->setKeyRange (keyRange);
        }
        else if (slider == &transposeSlider)
        {
            object->setTransposeOffset (roundToInt (slider->getValue()));
        }
        
        ValueTree data (node.getValueTree());
        auto range (object->getKeyRange());
        data.setProperty (Tags::keyStart, range.getStart(), nullptr)
            .setProperty (Tags::keyEnd, range.getEnd(), nullptr)
            .setProperty (Tags::transpose, object->getTransposeOffset(), nullptr);

        updateSliders(); // in case graph node changes requested values
    }

    void NodeContentView::updateSliders()
    {
        if (GraphNodePtr object = node.getGraphNode())
        {
            const auto range (object->getKeyRange());
            keyLowSlider.setValue ((double) range.getStart(), dontSendNotification);
            keyHiSlider.setValue ((double) range.getEnd(), dontSendNotification);
            transposeSlider.setValue ((double) object->getTransposeOffset(), dontSendNotification);
        }
    }

    void NodeContentView::updateMidiChannels()
    {
        if (GraphNodePtr object = node.getGraphNode())
        {
            BigInteger chans;
            {
                ScopedLock sl (object->getPropertyLock());
                chans = object->getMidiChannels().get();
            }

            midiChannel.setChannels (chans, false);
        }
    }
}
