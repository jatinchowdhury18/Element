#ifndef EL_GRAPH_NODE_H
#define EL_GRAPH_NODE_H

#include "ElementApp.h"

namespace Element {

class GraphProcessor;

/** Represents one of the nodes, or processors, in an AudioProcessorGraph.

    To create a node, call ProcessorGraph::addNode().
*/
class GraphNode : public ReferenceCountedObject
{
public:
    /** The ID number assigned to this node.
        This is assigned by the graph that owns it, and can't be changed. */
    const uint32 nodeId;

    virtual ~GraphNode();
    
    /** A set of user-definable properties that are associated with this node.

        This can be used to attach values to the node for whatever purpose seems
        useful. For example, you might store an x and y position if your application
        is displaying the nodes on-screen.
    */
    NamedValueSet properties;

    /** Create a node suitable for binding to a root graph */
    static GraphNode* createForRoot (GraphProcessor*);
    
    /** Returns the processor as an AudioProcessor */
    AudioProcessor* getAudioProcessor() const noexcept { return proc; }
    
    /** Returns the actual processor object that this node represents. */
    Processor* getProcessor() const noexcept { return dynamic_cast<Processor*> (proc.get()); }

    /** Returns the processor as an Audio Plugin Instance */
    AudioPluginInstance* getAudioPluginInstance() const;

    /** Returns the total number of audio inputs */
    int getNumAudioInputs() const;

    /** Returns the total number of audio ouputs */
    int getNumAudioOutputs() const;
    
    PortType getPortType (const uint32 port) const;
    uint32 getNumPorts() const;
    int getNumPorts (const PortType type, const bool isInput) const;
    uint32 getPortForChannel (const PortType type, const int channel, const bool isInput) const;
    
    int getChannelPort (const uint32 port) const;
    
    int getNthPort (const PortType portType, const int inputChan, bool, bool) const;
    
    /** Returns true if an input port */
    bool isPortInput (const uint32 port) const;

    /** Returns true if an output port */
    bool isPortOutput (const uint32 port) const;
    
    bool isGraph() const noexcept;
    bool isRootGraph() const noexcept;
    bool isSubGraph() const noexcept;
    
    const String& getTypeString() const;

    uint32 getMidiInputPort() const;
    uint32 getMidiOutputPort() const;
    
    /** If an audio plugin instance, fill the details */
    void getPluginDescription (PluginDescription& desc);
    
    /** The actual processor object dynamic_cast'd to ProcType */
    template<class ProcType>
    inline ProcType* processor() const { return dynamic_cast<ProcType*> (proc.get()); }

    /** Returns true if the processor is suspended */
    bool isSuspended() const;
    
    /** Suspend processing */
    void suspendProcessing (const bool);

    /** Set the Input Gain of this Node */
    void setInputGain (const float f);

    /** Set the Gain of this Node */
    void setGain (const float f);

    inline float getInputGain() const { return inputGain.get(); }
    inline float getGain() const { return gain.get(); }
    inline float getLastGain() const { return lastGain.get(); }
    inline float getLastInputGain() const { return lastInputGain.get(); }
    inline void updateGain()
    {
        if (lastGain.get() != gain.get())
            lastGain = gain;
        if (lastInputGain.get() != inputGain.get())
            lastInputGain = inputGain;
    }

    ValueTree getMetadata() const { return metadata; }

    bool isAudioIONode() const;
    bool isMidiIONode() const;

    /* returns the parent graph. If one has not been set, then
       this will return nullptr */
    GraphProcessor* getParentGraph() const;

    void setInputRMS (int chan, float val);
    float getInputRMS(int chan) const { return (chan < inRMS.size()) ? inRMS.getUnchecked(chan)->get() : 0.0f; }
    void setOutputRMS (int chan, float val);
    float getOutputRMS (int chan) const { return (chan < outRMS.size()) ? outRMS.getUnchecked(chan)->get() : 0.0f; }

    void connectAudioTo (const GraphNode* other);

    /** Enable or disable this node */
    void setEnabled (const bool shouldBeEnabled);

    /** Returns true if this node is enabled */
    inline bool isEnabled()  const { return enabled.get() == 1; }

    inline void setKeyRange (const int low, const int high)
    {
        jassert (low <= high);
        jassert (isPositiveAndBelow (low, 128));
        jassert (isPositiveAndBelow (high, 128));
        keyRangeLow.set (low); keyRangeHigh.set (high);
    }

    inline void setKeyRange (const Range<int>& range) { setKeyRange (range.getStart(), range.getEnd()); }

    inline Range<int> getKeyRange() const { return Range<int> { keyRangeLow.get(), keyRangeHigh.get() }; }

    inline void setTransposeOffset (const int value)
    {
        jassert (value >= -24 && value <= 24);
        transposeOffset.set (value);
    }

    inline int getTransposeOffset() const { return transposeOffset.get(); }

    CriticalSection& getPropertyLock() { return propertyLock; }

    inline void setMidiChannels (const BigInteger& ch)
    { 
        ScopedLock sl (propertyLock);
        midiChannels.setChannels (ch);
    }

    inline const MidiChannels& getMidiChannels() const { return midiChannels; }

    Signal<void(GraphNode*)> enablementChanged;

private:
    friend class GraphProcessor;
    friend class GraphController;
    friend class EngineController;
    friend class Node;

    MemoryBlock pluginState;

    ScopedPointer<AudioProcessor> proc;
    bool isPrepared = false;
    Atomic<int> enabled { 1 };

    GraphNode (uint32 nodeId, AudioProcessor*) noexcept;

    void setParentGraph (GraphProcessor*);
    void prepare (double sampleRate, int blockSize, GraphProcessor*, bool willBeEnabled = false);
    void unprepare();
    void resetPorts();
    
    Atomic<float> gain, lastGain, inputGain, lastInputGain;
    OwnedArray<AtomicValue<float> > inRMS, outRMS;
    
    ChannelConfig channels;
    ValueTree metadata, node;
    GraphProcessor* parent; 
    Atomic<int> keyRangeLow { 0 };
    Atomic<int> keyRangeHigh { 127 };
    Atomic<int> transposeOffset { 0 };
    MidiChannels midiChannels;

    CriticalSection propertyLock;
    struct EnablementUpdater : public AsyncUpdater
    {
        EnablementUpdater (GraphNode& g) : graph (g) { }
        ~EnablementUpdater() { }
        void handleAsyncUpdate() override;
        GraphNode& graph;
    } enablement;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphNode)
};

/** A convenient typedef for referring to a pointer to a node object. */
typedef ReferenceCountedObjectPtr<GraphNode> GraphNodePtr;

}

#endif // EL_GRAPH_NODE_H
