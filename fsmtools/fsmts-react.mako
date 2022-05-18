# coding: utf-8
<%!
from textwrap import wrap
import re
%><%
def parseCondition(c):
    def replacer(s):
        try:
            return '(action.event === Events.%s)' % (capitalCase(machine.event(s.group()).name),)
        except KeyError:
            return 'delegate.%s(action.payload, resultState.data)' % (machine.input(s.group()).name,)
    try:
        return 'action.event === Events.%s' % (capitalCase(machine.event(c.strip()).name),)
    except KeyError:
        return re.sub(r'\b(\w+)\b', replacer, c);

def formatAction(a):
    if a.type == 'output':
        return 'delegate.%s(action.payload, resultState.data)' % (a.name,)
    else:
        raise 'Sending events to self is not supported in reducer'

def fromCamelCase(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()
%>\

import React, { useReducer, Reducer, useCallback } from "react";
% if options.debug:
import dayjs from 'dayjs';
% endif

export enum States {
% for (index, s) in enumerate(machine.states):
    ${capitalCase(s.name)} = '${s.name}',
% endfor
}

export enum Events {
    Init = 0,
% for (index, s) in enumerate(machine.events):
    ${capitalCase(s.name)} = ${index + 1},
% endfor
}

export interface Action<P> {
    event: Events;
    payload: P;
}

export interface Delegate<S, P> {
    // inputs
% for (index, s) in enumerate(machine.inputs):
    % if s.comment:
    /**
        % for l in wrap(s.comment, 80):
        ${l}
        % endfor
    */
    % endif
    ${s.name}(x: P, prevState: Readonly<S>): boolean;
% endfor

    // outputs
% for (index, s) in enumerate(machine.outputs):
    % if s.comment:
    /**
        % for l in wrap(s.comment, 80):
        ${l}
        % endfor
    */
    % endif
    ${s.name}(params: P, prevState: Readonly<S>): S;
% endfor
}

interface State<S> {
    currentState: States;
    data: S;
}

export function use${capitalCase(machine.name)}<S, P = any>(delegate: Delegate<S, P>, initialState: S): [State<S>, React.Dispatch<Action<P>>] {

    const reducer: Reducer<State<S>, Action<P>> = useCallback((prevState, action) => {
        let resultState = prevState;
% if options.debug:
        console.log('${machine.name}: ' + dayjs().format('HH:mm:ss.SSS :') + stateName(prevState.currentState) + " : event " + eventName(action.event));
% endif

//    assert(_.includes(_.values(Events), event), 'Unknown event ' + event + ' passed');

        if (action.event === Events.Init) {
            resultState = {currentState: States.${machine.initialState}, data: initialState};
        % for a in machine.state(machine.initialState).incomeActions:
            % if options.debug:
            console.log('${machine.name}: ' + '${a.type} ${a.name}');
            % endif
            resultState = {...resultState, data: ${formatAction(a)}};
        % endfor
% if options.debug:
        console.log(stateName(prevState.currentState) + " : event " + eventName(action.event) + ' processing complete');
% endif
            return resultState;
        }

        switch (prevState.currentState) {
    % for s in machine.states:
        case States.${capitalCase(s.name)}:
        % for (i, t) in enumerate(s.transitions):
            // ${t.name}
            % if i ==0:
            if (${parseCondition(t.condition)}) {
            % else:
            else if (${parseCondition(t.condition)}) {
            % endif
            % if t.destination:
                % if options.debug:
                console.log('${machine.name}: ${capitalCase(s.name)} -> ${t.destination.name}: ${t.name}');
                % endif
                resultState = {...resultState, currentState: States.${t.destination.name}};
                % for a in s.exitingActions:
                    % if options.debug:
                console.log("${machine.name}:    - ${a.name}");
                    % endif
                resultState = {...resultState, data: ${formatAction(a)}};
                % endfor
            % endif
            % for a in t.actions:
                    % if options.debug:
                console.log('${machine.name}:    - ${a.name}');
                    % endif
                resultState = {...resultState, data: ${formatAction(a)}};
            % endfor
            % if t.destination:
                % for a in t.destination.incomeActions:
                    % if options.debug:
                console.log('${machine.name}:    - ${a.name}');
                    % endif
                resultState = {...resultState, data: ${formatAction(a)}};
                % endfor
            % endif
            }
        % endfor
        %if options.debug:
            %if len(s.transitions) > 0:
            else {
                console.log('${machine.name}: impasse state for event: ' + eventName(action.event));
            }
            %else:
            console.log('${machine.name}: impasse state for event: ' + eventName(action.event));
            %endif

        %endif
            break;
    % endfor
        }

% if options.debug:
        console.log('${machine.name}: ' + stateName(prevState.currentState) + " : event " + eventName(action.event) + ' processing complete');
% endif
        return resultState;
    }, [delegate]);

    return useReducer(reducer, {currentState: States.${machine.initialState}, data: initialState});
}

export function eventName(event: Events): string {
    switch (event) {
        case Events.Init:
            return '<Initialization event>';
    % for (index, s) in enumerate(machine.events):
        case Events.${capitalCase(s.name)}:
            return '${s.name}';
    % endfor
    }
    return "Unknown (" + event + ")";
}

export function stateName(state: States) {
    switch (state) {
    % for (index, s) in enumerate(machine.states):
        case States.${capitalCase(s.name)}:
            return '${s.name}';
    % endfor
    }
    return "Unknown (" + state + ")";
}

export function create${capitalCase(machine.name)}Context<S, P>() {
  const ${capitalCase(machine.name)}Context = React.createContext<[State<S>, React.Dispatch<Action<P>>] | undefined>(undefined);
  function use${capitalCase(machine.name)}Context() {
    const c = React.useContext(${capitalCase(machine.name)}Context);
    if (c === undefined)
      throw new Error("use${capitalCase(machine.name)}Context must be inside a Provider with a value");
    return c;
  }
  return [use${capitalCase(machine.name)}Context, ${capitalCase(machine.name)}Context.Provider] as const; // 'as const' makes TypeScript infer a tuple
}

## export const [use${capitalCase(machine.name)}Context, ${capitalCase(machine.name)}ContextProvider] = createCtx<typeof use${capitalCase(machine.name)}>();


// const [state, dispatch] = use${capitalCase(machine.name)}({
% for (index, s) in enumerate(machine.inputs):
//     ${s.name} : function(eventPayload, data) {
//         console.log('stub for ${s.name}, params: ' + JSON.stringify(eventPayload));
//         return false;
//     },
% endfor

% for (index, s) in enumerate(machine.outputs):
//     ${s.name}: function(eventPayload, data) {
//         console.log('stub for ${s.name}, params: ' + JSON.stringify(eventPayload));
//         return data;
//     },
% endfor
// }, {initialValue1: 1, initialValue2: 'abc'});
