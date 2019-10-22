/*
    This file is part of Element
    Copyright (C) 2019  Kushview, LLC.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#pragma once

#include "gui/nodes/NodeEditorComponent.h"
#include "session/DeviceManager.h"

namespace Element {

class AudioIONodeEditor : public NodeEditorComponent,
                         public ChangeListener
{
public:
    AudioIONodeEditor (const Node& node, DeviceManager& devs, bool ins = true, bool outs = true)
        : NodeEditorComponent (node), devices (devs), showIns(ins), showOuts(outs)
    {
       #if ! EL_RUNNING_AS_PLUGIN
        content.reset (new Content (*this));
        view.setViewedComponent (content.get(), false);
        view.setScrollBarsShown (true, false);
        addAndMakeVisible (view);
        devices.addChangeListener (this);
       #endif
    }

    ~AudioIONodeEditor()
    {
       #if ! EL_RUNNING_AS_PLUGIN
        devices.removeChangeListener (this);
        view.setViewedComponent (nullptr, false);
        content.reset();
       #endif
    }
    
    void paint (Graphics& g) override
    {
       #if EL_RUNNING_AS_PLUGIN
        g.setFont (13.f);
        g.setColour (LookAndFeel::textColor);
        String text = "Host Audio ";
        if (getNode().isAudioInputNode())
            text << "Input";
        else if (getNode().isAudioOutputNode())
            text << "Output";
        g.drawText (text, getLocalBounds(), Justification::centred);
       #endif
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
       #if ! EL_RUNNING_AS_PLUGIN
        content->updateDevices();
       #endif
    }

    void resized() override
    {
       #if ! EL_RUNNING_AS_PLUGIN
        view.setBounds (getLocalBounds());
        content->setSize (view.getWidth(), content->getHeight());
       #endif
    }

private:
    DeviceManager& devices;
    bool showIns = true;
    bool showOuts = true;
    Viewport view;
    friend class Content;

    struct Content : public Component,
                     public Button::Listener
    {
        Content (AudioIONodeEditor& ed) 
            : owner (ed)
        {
            if (owner.showOuts)
            {
                
            }

            if (owner.showIns)
            {   
                
            }

            updateDevices();
        }
        
        int computeHeight()
        {
            return 100;
        }

        void updateDevices()
        {
            if (owner.showIns)
                updateInputs();
            if (owner.showOuts)
                updateOutputs();
        }

        void updateInputs()
        {
            
        }

        void updateOutputs()
        {
            
        }

        void buttonClicked (Button* button) override
        {
        }

        void resized() override
        {
            
        }
        
        AudioIONodeEditor& owner;
    };

    std::unique_ptr<Content> content;
};

}
