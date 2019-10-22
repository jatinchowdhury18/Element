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

#ifndef RACKVIEW_H_INCLUDED
#define RACKVIEW_H_INCLUDED

#include "ElementApp.h"

namespace Element {

class RackView :  public Component
{
public:
    RackView();
    ~RackView();

    void paint (Graphics&);
    void resized();

    void setMainComponent (Component* comp);
private:
    class Impl; friend class Impl;
    ScopedPointer<Impl> impl;
    ScopedPointer<Component> main;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RackView)
};

}

#endif  // RACKVIEW_H_INCLUDED
