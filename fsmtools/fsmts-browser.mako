# coding: utf-8
<%!
from textwrap import wrap
import re
%><%
def parseCondition(c):
    def replacer(s):
        try:
            return '(event === %sEvents.%s)' % (capitalCase(machine.name), capitalCase(machine.event(s.group()).name))
        except KeyError:
            return '(%sthis.delegate.%s(params))' % ('await ' if options.isAsync else '', machine.input(s.group()).name)
    try:
        return 'event === %sEvents.%s' % (capitalCase(machine.name), capitalCase(machine.event(c.strip()).name))
    except KeyError:
        return re.sub(r'\b(\w+)\b', replacer, c);

def formatAction(a):
    if a.type == 'output':
        return '%sthis.delegate.%s(params);' % ('await ' if options.isAsync else '', a.name)
    else:
        return '%sthis.processEvent(${capitalCase(machine.name)}Events.%s, params);' % ('await ' if options.isAsync else '', capitalCase(a.name))

def fromCamelCase(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()
%>\

import {BehaviorSubject, Observable} from "rxjs";

export enum ${capitalCase(machine.name)}States {
% for (index, s) in enumerate(machine.states):
    ${capitalCase(s.name)} = '${s.name}',
% endfor
}

export enum ${capitalCase(machine.name)}Events {
    Init = 0,
% for (index, s) in enumerate(machine.events):
    ${capitalCase(s.name)} = ${index + 1},
% endfor
}

export interface ${capitalCase(machine.name)}Delegate {
    // inputs
% for (index, s) in enumerate(machine.inputs):
    % if s.comment:
    /**
        % for l in wrap(s.comment, 80):
        ${l}
        % endfor
    */
    % endif
    %if options.isAsync:
    ${s.name}(x: any): Promise<boolean>;
    %else:
    ${s.name}(x: any): boolean;
    %endif
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
    %if options.isAsync:
    ${s.name}(x: any): Promise<void>;
    %else:
    ${s.name}(x: any): void;
    %endif
% endfor
}

export class ${capitalCase(machine.name)} {
    private _events: Array<[${capitalCase(machine.name)}Events, any]> = [];
    private _state = new BehaviorSubject<${capitalCase(machine.name)}States>(${capitalCase(machine.name)}States.${machine.initialState});
    delegate: ${capitalCase(machine.name)}Delegate;

    constructor(delegate: ${capitalCase(machine.name)}Delegate) {
        this.delegate = delegate;
    }
    eventName(event: ${capitalCase(machine.name)}Events): string {
        switch (event) {
            case ${capitalCase(machine.name)}Events.Init:
                return '<Initialization event>';
        % for (index, s) in enumerate(machine.events):
            case ${capitalCase(machine.name)}Events.${capitalCase(s.name)}:
                return '${s.name}';
        % endfor
        }
        return "Unknown (" + event + ")";
    }

    stateName(state: ${capitalCase(machine.name)}States) {
        switch (state) {
        % for (index, s) in enumerate(machine.states):
            case ${capitalCase(machine.name)}States.${capitalCase(s.name)}:
                return '${s.name}';
        % endfor
        }
        return "Unknown (" + state + ")";
    }

%if options.isAsync:
    async processEvent(event: ${capitalCase(machine.name)}Events, params: any): Promise<void> {
%else:
    processEvent(event: ${capitalCase(machine.name)}Events, params: any): void {
%endif
        const empty = this._events.length === 0;

        this._events.push([event, params]);
        if (empty) {
%if options.isAsync:
        await this._processQueue();
%else:
        this._processQueue();
%endif
        }
    }

    isIn (): boolean {
        const self = this;
        const args = Array.prototype.slice.call(arguments);
        for(let i = 0; i < args.length; ++i) {
            if (args[i] === self._state.value)
                return true;
        }
        return false;
    }

    getCurrentState(): ${capitalCase(machine.name)}States {
        return this._state.value;
    }

    get state(): ${capitalCase(machine.name)}States {
        return this.getCurrentState();
    }

    getStateObservable(): Observable<${capitalCase(machine.name)}States> {
        return this._state;
    }

    get stateObservable(): Observable<${capitalCase(machine.name)}States> {
        return this.getStateObservable();
    }

%if options.isAsync:
    async _processQueue(): Promise<void> {
%else:
    _processQueue(): void {
%endif
        const self = this;

        while (self._events.length > 0) {
            const [event, params] = self._events[0];
%if options.isAsync:
    await \
%endif
            self._processEvent(event, params);
            self._events.shift();
        }
    }

%if options.isAsync:
    async _processEvent(event: ${capitalCase(machine.name)}Events, params: any): Promise<void> {
%else:
    _processEvent(event: ${capitalCase(machine.name)}Events, params: any): void {
%endif
% if options.debug:
        console.log('${machine.name}: ' + this.stateName(this._state.value) + " : event " + this.eventName(event));
% endif

//    assert(_.includes(_.values(${capitalCase(machine.name)}Events), event), 'Unknown event ' + event + ' passed');

        if (event === ${capitalCase(machine.name)}Events.Init) {
            this._state.next(${capitalCase(machine.name)}States.${machine.initialState});
        % for a in machine.state(machine.initialState).incomeActions:
            % if options.debug:
            console.log('${machine.name}: ${a.type} ${a.name}');
            % endif
            ${formatAction(a)}
        % endfor
% if options.debug:
        console.log(this.stateName(this._state.value) + " : event " + this.eventName(event) + ' processing complete');
% endif
            return;
        }

        switch (this._state.value) {
    % for s in machine.states:
        case ${capitalCase(machine.name)}States.${capitalCase(s.name)}:
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
                this._state.next(${capitalCase(machine.name)}States.${t.destination.name});
                % for a in s.exitingActions:
                    % if options.debug:
                console.log("${machine.name}:    - ${a.name}");
                    % endif
                ${formatAction(a)}
                % endfor
            % endif
            % for a in t.actions:
                    % if options.debug:
                console.log('${machine.name}:    - ${a.name}');
                    % endif
                ${formatAction(a)}
            % endfor
            % if t.destination:
                % for a in t.destination.incomeActions:
                    % if options.debug:
                console.log('${machine.name}:    - ${a.name}');
                    % endif
                ${formatAction(a)}
                % endfor
            % endif
            }
        % endfor
        %if options.debug:
            %if len(s.transitions) > 0:
            else {
                console.log('${machine.name}: impasse state for event: ' + this.eventName(event));
            }
            %else:
            console.log('${machine.name}: impasse state for event: ' + this.eventName(event));
            %endif

        %endif
            break;
    % endfor
        }

% if options.debug:
        console.log('${machine.name}: ' + this.stateName(this._state.value) + " : event " + this.eventName(event) + ' processing complete');
% endif
    }
}

// const fsm = new ${capitalCase(machine.name)}({
% for (index, s) in enumerate(machine.inputs):
//     ${s.name} : function(x) {
//         console.log('stub for ${s.name}, params: ' + JSON.stringify(x));
//         return false;
//     },
% endfor

% for (index, s) in enumerate(machine.outputs):
//     ${s.name}: function(x) {
//         console.log('stub for ${s.name}, params: ' + JSON.stringify(x));
//         return;
//     },
% endfor
// });
