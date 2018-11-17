/*
    Session.h - This file is part of Element
    Copyright (C) 2016 Kushview, LLC.  All rights reserved.
*/

#pragma once

#include "ElementApp.h"
#include "session/ControllerDevice.h"
#include "session/Node.h"
#include "Signals.h"

#define EL_TEMPO_MIN 20
#define EL_TEMPO_MAX 999

namespace Element {
    class Session;
    class Globals;
    
    /** Session, the main interface between the engine and model layers */
    class Session : public ObjectModel,
                    public ReferenceCountedObject,
                    public ChangeBroadcaster,
                    public ValueTree::Listener
    {
    public:
        struct ScopedFrozenLock
        {
            ScopedFrozenLock (const Session& s) : session(s)
            {
                wasFrozen = session.freezeChangeNotification;
                session.freezeChangeNotification = true;
            } 

            ~ScopedFrozenLock()
            {
                session.freezeChangeNotification = wasFrozen;
            }

        private:
            const Session& session;
            bool wasFrozen;
        };

        virtual ~Session();
        
        inline int getNumGraphs() const { return objectData.getChildWithName(Tags::graphs).getNumChildren(); }
        inline Node getGraph (const int index) const { return Node (getGraphValueTree(index), false); }
        Node getCurrentGraph() const { return getActiveGraph();}
        Node getActiveGraph() const;
        int getActiveGraphIndex() const;
        
        bool addGraph (const Node &node, const bool setActive);
        
        ValueTree getValueTree() const { return objectData; }
        bool loadData (const ValueTree& data);
        void clear();
        
        inline void setName (const String& name) { setProperty (Slugs::name, name); }
        inline String getName()            const { return objectData.getProperty(Slugs::name, "Invalid Session"); }
        inline Value getNameValue()              { return getPropertyAsValue (Slugs::name); }
        
        inline bool useExternalClock()      const { return (bool) getProperty ("externalSync", false); }
        
        inline bool notificationsFrozen()   const { return freezeChangeNotification; }

        XmlElement* createXml();
        
        void saveGraphState();
        void restoreGraphState();
        
        inline int getNumControllerDevices() const { return getControllerDevicesValueTree().getNumChildren(); }
       
        inline ValueTree getControllerDeviceValueTree (const int i) const
        {
            return getControllerDevicesValueTree().getChild(i);
        }
        
        inline ControllerDevice getControllerDevice (const int index) const
        {
            ControllerDevice device (getControllerDeviceValueTree (index));
            return device;
        }
        
        inline int indexOf (const ControllerDevice& device) const
        {
            return getControllerDevicesValueTree().indexOf (device.getValueTree());
        }
        
        inline int getNumControllerMaps() const { return getControllerMapsValueTree().getNumChildren(); }
        inline ControllerMap getControllerMap (const int index) const { return ControllerMap (getControllerMapsValueTree().getChild (index)); }
        inline int indexOf (const ControllerMap& controllerMap) const { return getControllerMapsValueTree().indexOf (controllerMap.getValueTree()); }
        
        Node findNodeById (const Uuid&);
        ControllerDevice findControllerDeviceById (const Uuid&);

        void cleanOrphanControllerMaps();

        typedef std::function<void(const ValueTree& tree)> ValueTreeFunction;
        void forEach (ValueTreeFunction handler) const;

    protected:
        void forEach (const ValueTree tree, ValueTreeFunction handler) const;

        Session();
        friend class Globals;
 
        /** Set a property. */
        inline void setProperty (const Identifier& prop, const var& val) { objectData.setProperty (prop, val, nullptr); }

        friend class ValueTree;
        virtual void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property);
        virtual void valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded);
        virtual void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int);
        virtual void valueTreeChildOrderChanged (ValueTree& parentTreeWhoseChildrenHaveMoved, int, int);
        virtual void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged);
        virtual void valueTreeRedirected (ValueTree& treeWhichHasBeenChanged);

    private:
        class Private;
        ScopedPointer<Private> priv;
        void setMissingProperties (bool resetExisting = false);
        
        inline ValueTree getGraphsValueTree()                   const { return objectData.getChildWithName (Tags::graphs); }
        inline ValueTree getGraphValueTree (const int index)    const { return getGraphsValueTree().getChild(index); }
        inline ValueTree getControllerDevicesValueTree()        const { return objectData.getChildWithName(Tags::controllers); }
        inline ValueTree getControllerMapsValueTree()           const { return objectData.getChildWithName(Tags::maps); }

        friend class SessionController;
        friend struct ScopedFrozenLock;
        mutable bool freezeChangeNotification = false;
        void notifyChanged();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Session);
    
    public:
        boost::signals2::signal<void(const ControllerDevice&)> controllerDeviceAdded;
        boost::signals2::signal<void(const ControllerDevice&)> controllerDeviceRemoved;
        boost::signals2::signal<void(const ControllerDevice::Control&)> controlAdded;
        boost::signals2::signal<void(const ControllerDevice::Control&)> controlRemoved;
    };
    
    typedef ReferenceCountedObjectPtr<Session> SessionPtr;
    typedef SessionPtr SessionRef;

    struct ControllerMapObjects
    {
        ControllerMapObjects() = default;
        ControllerMapObjects (SessionPtr s, const ControllerMap& m)
            : session (s), controllerMap (m)
        {
            if (session != nullptr)
            {
                device = session->findControllerDeviceById (Uuid (controllerMap.getProperty(Tags::controller)));
                control = device.findControlById (Uuid (controllerMap.getProperty (Tags::control)));
                node = session->findNodeById (Uuid (controllerMap.getProperty(Tags::node)));
            }
        }

        ControllerMapObjects (const ControllerMapObjects& o) { operator= (o); }
        ControllerMapObjects& operator= (const ControllerMapObjects& o)
        {
            this->session = o.session;
            this->controllerMap = o.controllerMap;
            this->node = o.node;
            this->device = o.device;
            this->control = o.control;
            return *this;
        }

        ~ControllerMapObjects() = default;
        
        inline bool isValid() const { return device.isValid() && control.isValid() && node.isValid(); }

        SessionPtr session;
        ControllerMap controllerMap;
        Node node;
        ControllerDevice device;
        ControllerDevice::Control control;
    };
}
