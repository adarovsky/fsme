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
            if options.isAsync:
                return '_local_%s' % machine.input(s.group()).name
            return '(delegate?.%s(params) ?? false)' % machine.input(s.group()).name
    try:
        return 'event == Event.%s' % (camelCase(machine.event(c.strip()).name))
    except KeyError:
        return re.sub(r'\b(\w+)\b', replacer, c);

def formatAction(a):
    if a.type == 'output':
        return '%sdelegate?.%s(params)' % ('await ' if options.isAsync else '', a.name)
    else:
        return '%sprocess(event: .%s, params: params)' % ('await ' if options.isAsync else '', camelCase(a.name))

def isInput(name):
    for i in machine.inputs:
        if name.strip() == i.name:
            return True
    return False

def usedInputs(c):
    used = re.findall(r'\b(\w+)\b', c)
    return filter(isInput, used)

%>\
import Foundation
import Combine
% if options.debug:
import os
% endif

${machine.comment}

    // MARK: - Delegate Protocol
protocol ${machine.name}Delegate : AnyObject {
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
    func ${i.name}(_ params: Params?) \
    % if options.isAsync:
async \
    % endif
-> Bool
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
    func ${o.name}(_ params: Params?) \
    % if options.isAsync:
async\
    % endif

% endfor
}

%if options.isAsync:
actor \
%else:
class \
%endif
${machine.name}<Delegate> where Delegate : ${machine.name}Delegate {

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

    static func name(for event: Event) -> String {
        switch (event) {
            case ._init:
                return "<Initialization event>"
    % for s in machine.events:
            case .${camelCase(s.name)}:
                return "${s.name}"
    % endfor
        }
    }

    static func name(for state: State) -> String {
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
    let logger: Logger
% endif

    init(delegate: Delegate) {
        self.delegate = delegate
% if options.debug:
        logger = Logger(subsystem: "com.vyulabs.vydeosdk", category: "${machine.name}")
% endif
    }

    func process(event: Event)\
        % if options.isAsync:
 async \
        % endif
 {
    % if options.isAsync:
        await \
    % else:
        \
    % endif
process(event: event, params: nil)
    }

    func process(event:Event, params: Delegate.Params?)\
        % if options.isAsync:
 async\
        % endif
 {
        let __empty = eventQueue.isEmpty
        eventQueue.append((event, params))
        if __empty {
            while !eventQueue.isEmpty {
                let (ev, pars) = eventQueue.first!
                % if options.isAsync:
                await \
                % else:
                 \
                % endif
process_(event: ev, params: pars)
                eventQueue.removeFirst()
            }
        }
    }

    private func process_(event: Event, params: Delegate.Params?) \
        % if options.isAsync:
 async \
        % endif
 {
% if options.debug:
        logger.log("\(${machine.name}.name(for: self.currentState), privacy: .public) : event \(${machine.name}.name(for: event), privacy: .public), params \(String(describing: params))")
% endif
        switch(event) {
        case ._init:
            currentState = .${camelCase(machine.initialState)}
            % for a in machine.state(machine.initialState).incomeActions:
                % if options.debug:
                logger.log(" - ${a.type} ${a.name}")
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
            <%
            initedInputs = set()
            %>
            % for (i, t) in enumerate(s.transitions):
                // ${t.name}
                <%
                allInputs = set(usedInputs(t.condition))
                inputs = allInputs.difference(initedInputs)
                initedInputs.update(allInputs)
                %>
                % if options.isAsync and len(inputs) > 0:
                    % for u in inputs:
                let _local_${u}: Bool
                    % endfor
                if let d = delegate {
                    %if len(inputs) <= 1:
                    _local_${u} = await d.${u}(params)
                    %else:
                        % for u in inputs:
                    async let _local_${u}$ = d.${u}(params)
                        % endfor
                        % for u in inputs:
                    _local_${u} = await _local_${u}$
                        % endfor
                    %endif
                }
                else {
                        % for u in inputs:
                    _local_${u} = false
                        % endfor
                }
                % endif

                % if i == 0 or options.isAsync:
                if ${parseCondition(t.condition)} {
                % else:
                else if ${parseCondition(t.condition)} {
                % endif
                % if t.destination:
                    % if options.debug:
                    logger.log("${s.name} -> ${t.destination.name}: ${t.name}")
                    % endif
                    currentState = .${camelCase(t.destination.name)}
                    % for a in s.exitingActions:
                        % if options.debug:
                    logger.log(" - ${a.type} ${a.name}")
                        % endif
                    ${formatAction(a)}
                    % endfor
                % endif
                % for a in t.actions:
                        % if options.debug:
                    logger.log(" - ${a.type} ${a.name}")
                        % endif
                    ${formatAction(a)}
                % endfor
                % if t.destination:
                    % for a in t.destination.incomeActions:
                        % if options.debug:
                    logger.log(" - ${a.type} ${a.name}")
                        % endif
                    ${formatAction(a)}
                    % endfor
                    %if options.isAsync:
                    return
                    %endif
                % endif
                }
            % endfor
            % if options.debug:
                % if len(s.transitions) == 0 or options.isAsync:
                logger.log("${s.name}: impasse with event \(${machine.name}.name(for: event))")
                % else:
                else {
                    logger.log("${s.name}: impasse with event \(${machine.name}.name(for: event))")
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
//     func ${i.name}(_ params: FSMParams?) \
    % if options.isAsync:
async \
    % endif
-> Bool {
//         return false
//     }
% endfor
//
//    // MARK: Output implementations
% for o in machine.outputs:
//    func ${o.name}(_ params: FSMParams?) \
    % if options.isAsync:
async \
    % endif
{
//        // implement side effects here
//    }
% endfor
// }
