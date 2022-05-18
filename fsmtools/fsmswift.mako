# coding: utf-8
<%!
from textwrap import wrap
import re
%><%
def parseCondition(c):
    def replacer(s):
        try:
            return '(event == Event.%s)' % (camelCase(machine.event(s.group()).name))
        except KeyError:
            return '(delegate?.%s(params) ?? false)' % machine.input(s.group()).name
    try:
        return 'event == Event.%s' % (camelCase(machine.event(c.strip()).name))
    except KeyError:
        return re.sub(r'\b(\w+)\b', replacer, c);

def formatAction(a):
    if a.type == 'output':
        return 'delegate?.%s(params)' % a.name
    else:
        return 'process(event: .%s, params: params)' % camelCase(a.name)

%>\
import Foundation
import Combine
% if options.debug:
import os
% endif

${machine.comment}

    // MARK: - Delegate Protocol
protocol ${machine.name}Delegate : class {
    associatedtype Params

    // MARK: Inputs
% for i in machine.inputs:
    % if i.comment:
    /**
        % for l in wrap(i.comment, 80):
        ${l}
        % endfor
    */
    % endif
    func ${i.name}(_ params: Params?) -> Bool
% endfor

    // MARK: Outputs
% for o in machine.outputs:
    %if o.comment:
    /**
    % for l in wrap(o.comment, 80):
        ${l}
    % endfor
    */
    % endif
    func ${o.name}(_ params: Params?)
% endfor
}

class ${machine.name}<Delegate> where Delegate : ${machine.name}Delegate {

    // MARK: - FSM
    // MARK: States
    enum State {
% for (index, s) in enumerate(machine.states):
        case ${camelCase(s.name)}
% endfor
        func isIn(_ states: State...) -> Bool {
            states.contains(self)
        }
    }

    enum Event {
        case _init
% for (index, s) in enumerate(machine.events):
        case ${camelCase(s.name)}
% endfor
    }

    func name(event: Event) -> String {
        switch (event) {
            case ._init:
                return "<Initialization event>"
    % for s in machine.events:
            case .${camelCase(s.name)}:
                return "${s.name}"
    % endfor
        }
    }

    func name(state: State) -> String {
        switch (state) {
    % for s in machine.states:
            case .${camelCase(s.name)}:
                return "${s.name}"
    % endfor
        }
    }

    private var eventQueue : [(Event, Delegate.Params?)] = []
    @Published private(set) var currentState = State.${camelCase(machine.initialState)}
    weak var delegate: Delegate?

% if options.debug:
    let logger: OSLog
% endif

    init(delegate: Delegate) {
        self.delegate = delegate
% if options.debug:
        logger = OSLog(subsystem: "com.vyulabs.vydeosdk", category: "${machine.name}")
% endif
    }

    func process(event: Event) {
        process(event: event, params: nil)
    }

    func process(event:Event, params: Delegate.Params?) {
        let __empty = eventQueue.isEmpty;
        eventQueue.append((event, params));
        if __empty {
            while !eventQueue.isEmpty {
                let (ev, pars) = eventQueue.first!
                process_(event: ev, params: pars)
                eventQueue.removeFirst()
            }
        }
    }

    private func process_(event: Event, params: Delegate.Params?) {
% if options.debug:
         os_log("%{public}@", dso: #dsohandle, log: logger, type: .debug, "\(name(state: currentState)) : event \(name(event: event)), params \(String(describing: params))")
% endif
        switch(event) {
        case ._init:
            currentState = .${camelCase(machine.initialState)}
            % for a in machine.state(machine.initialState).incomeActions:
                % if options.debug:
                os_log("%{public}@", dso: #dsohandle, log: logger, type: .debug, "${a.type} ${a.name}")
                % endif
            ${formatAction(a)}
            % endfor
            return
        default:
            break
        }

        switch (currentState) {
        % for s in machine.states:
            case .${camelCase(s.name)}:
            % for (i, t) in enumerate(s.transitions):
                // ${t.name}
                % if i == 0:
                if ${parseCondition(t.condition)} {
                % else:
                else if ${parseCondition(t.condition)} {
                % endif
                % if t.destination:
                    % if options.debug:
                    os_log("%{public}@", dso: #dsohandle, log: logger, type: .info, "${s.name} -> ${t.destination.name}: ${t.name}")
                    % endif
                    currentState = .${camelCase(t.destination.name)}
                    % for a in s.exitingActions:
                        % if options.debug:
                    os_log("%{public}@", dso: #dsohandle, log: logger, type: .debug, "- ${a.name}")
                        % endif
                    ${formatAction(a)}
                    % endfor
                % endif
                % for a in t.actions:
                        % if options.debug:
                    os_log("%{public}@", dso: #dsohandle, log: logger, type: .debug, "${a.name}")
                        % endif
                    ${formatAction(a)}
                % endfor
                % if t.destination:
                    % for a in t.destination.incomeActions:
                        % if options.debug:
                    os_log("%{public}@", dso: #dsohandle, log: logger, type: .debug, "${a.type} ${a.name}")
                        % endif
                    ${formatAction(a)}
                    % endfor
                % endif
                }
            % endfor
            % if options.debug:
                % if len(s.transitions) == 0:
                os_log("%{public}@", dso: #dsohandle, log: logger, type: .info, "${s.name}: impasse with event \(name(event: event))")
                % else:
                else {
                    os_log("%{public}@", dso: #dsohandle, log: logger, type: .info, "${s.name}: impasse with event \(name(event: event))")
                }
                % endif
            % else:
                break
            % endif
        % endfor
        }
    }
}

// class MyModel : ${machine.name}Delegate {
//     enum FSMParams {
//         case someParam
//     }
//
//     typealias Params = FSMParams
//
//     private var fsm: ${machine.name}<MyModel>!
//     override init() {
//         super.init()
//         fsm = ${machine.name}<MyModel>(delegate: self)
//     }
//
//     // MARK: Input implementations
% for i in machine.inputs:
//     func ${i.name}(_ params: FSMParams?) -> Bool {
//         return false
//     }
% endfor
//
//    // MARK: Output implementations
% for o in machine.outputs:
//    func ${o.name}(_ params: FSMParams?) {
//        // implement side effects here
//    }
% endfor
// }
