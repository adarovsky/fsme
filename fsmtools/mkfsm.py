#!/usr/bin/env python
# coding: utf-8
import sys, types, optparse, os

#import bundled MakoTemplates module
sys.path.insert(0, os.path.join(os.path.dirname(sys.argv[0]), 'MakoTemplates.zip'))

from xml.etree import ElementTree as tree
from mako.runtime import Context
from mako.template import Template

def camelCase(s):
    if not s:
        return s
    return s[0].lower() + s[1:]

def capitalCase(s):
    if not s:
        return s
    return s[0].upper() + s[1:]

class Base(object):
    def __init__(self, name, comment):
        self.name = name
        self.comment = comment

class Event(Base):
    @property
    def type(self):
        return 'event'

class Input(Base):
    @property
    def type(self):
        return 'input'

class Output(Base):
    @property
    def type(self):
        return 'output'


class State(Base):
    def __init__(self, name, comment, incomeActions, exitingActions):
        Base.__init__(self, name, comment)
        self.incomeActions = incomeActions
        self.exitingActions = exitingActions
        self.transitions = []

    @property
    def type(self):
        return 'state'

class Transition(Base):
    def __init__(self, name, comment, condition, transType, destination, actions):
        Base.__init__(self, name, comment)
        self.condition = condition
        self.transitionType = transType
        self.actions = actions
        self.destination = destination
        self.transiteDestination = None

    @property
    def type(self):
        return 'transition'


class FiniteStateMachine(Base):
    def __init__(self, name, comment, initialState):
        Base.__init__(self, name, comment)
        self.initialState = initialState
        self.events = []
        self.inputs = []
        self.outputs = []
        self.states = []

    def event(self, name):
        for e in self.events:
            if e.name == name:
                return e
        raise KeyError(name)

    def input(self, name):
        for e in self.inputs:
            if e.name == name:
                return e
        raise KeyError(name)

    def output(self, name):
        for e in self.outputs:
            if e.name == name:
                return e
        raise KeyError(name)

    def state(self, name):
        for e in self.states:
            if e.name == name:
                return e
        raise KeyError(name)

parser = optparse.OptionParser(usage = "usage: %prog [options] <file>.fsm")

parser.add_option("-o", "--output", dest="output", default='-',
                  help="write output to FILE. If missing or '-', write output to stdout", metavar="FILE")

parser.add_option("-t", "--template", dest="template",
                  help="Use template for output generation")

parser.add_option("-d", "--debug", dest="debug", action="store_true", default=False,
                  help="Generate debugging information")

(options, args) = parser.parse_args()

if len(args) != 1:
    print >> sys.stderr, "A file with finite state machine required!"
    sys.exit(1)

f = file(args[0], 'r')

tree = tree.ElementTree(file = f)
template = Template(filename=options.template)

if (options.output != '-'):
    output = file(options.output, 'w')
else:
    output = sys.stdout

root = tree.getroot().find('statemachine')

fsm = FiniteStateMachine(root.find('name').text.strip(), root.find('comment').text or '', (root.find('initialstate').text or '').strip())

for e in root.findall('event'):
    fsm.events.append( Event(e.find('name').text.strip(), e.find('comment').text or '') )

for i in root.findall('input'):
    fsm.inputs.append( Input(i.find('name').text.strip(), i.find('comment').text or '') )

for o in root.findall('output'):
    fsm.outputs.append( Output(o.find('name').text.strip(), o.find('comment').text or '') )

for s in root.findall('state'):
    incoming = []
    exiting = []
    for i in s.find('incomeactions'):
        if i.get('type', 'output') == 'output':
            incoming.append(fsm.output(i.text.strip()))
        else:
            incoming.append(fsm.event(i.text.strip()))

    for i in s.find('outcomeactions'):
        if i.get('type', 'output') == 'output':
            exiting.append(fsm.output(i.text.strip()))
        else:
            exiting.append(fsm.event(i.text.strip()))

    state = State(s.find('name').text.strip(), s.find('comment').text or '', incoming, exiting)
    fsm.states.append( state )

    for t in s.find('transitions'):
        actions = []
        for i in t.find('transitionactions'):
            if i.get('type', 'output') == 'output':
                actions.append(fsm.output(i.text.strip()))
            else:
                actions.append(fsm.event(i.text.strip()))

        if t.find('destination').text is not None:
            dest = t.find('destination').text.strip()
        else:
            dest = None
        transition = Transition(t.find('name').text.strip(),
                                t.find('comment').text or '',
                                t.find('condition').text.strip(),
                                t.get('type', 'simple').strip(),
                                dest,
                                actions)
        if transition.transitionType == 'transite':
            transition.transiteDestination = t.find('transite_destination').text.strip()
        state.transitions.append(transition)


for s in fsm.states:
    for t in s.transitions:
        if isinstance(t.destination, types.StringTypes):
            t.destination = fsm.state(t.destination)


        if isinstance(t.transiteDestination, types.StringTypes):
            t.transiteDestination = fsm.state(t.transiteDestination)

fsm.events.sort(key=lambda e: e.name)
fsm.inputs.sort(key=lambda e: e.name)
fsm.outputs.sort(key=lambda e: e.name)
fsm.states.sort(key=lambda e: e.name)

output.write(template.render_unicode(machine=fsm, camelCase=camelCase, capitalCase=capitalCase, options=options).encode('utf-8'))

