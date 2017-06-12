# coding: utf-8
<%!
from textwrap import wrap
import re
%><%
def parseCondition(c):
    def replacer(s):
        try:
            return 'event == %s::%s' % (machine.name, capitalCase(machine.event(s.group()).name),)
        except KeyError:
            return 'delegate->%s(params)' % machine.input(s.group()).name

    return re.sub(r'\b(\w+)\b', replacer, c);

def formatAction(a):
    if a.type == 'output':
        return 'delegate->%s(params);' % a.name
    else:
        return 'processEvent(%s, params);' % (capitalCase(a.name),)

%>#include "${machine.name.lower()}.h"
% if options.debug:
#include <QtDebug>
Q_LOGGING_CATEGORY(${capitalCase(machine.name)}Logging, "fsm.${machine.name}")
% endif

${machine.name}::${machine.name}(${machine.name}::Delegate* aDelegate)
    : delegate(aDelegate), _currentState(${machine.initialState})
{
}

void ${machine.name}::processEventInternal(${machine.name}::Event event, const QVariantMap& params)
{
% if options.debug:
    qCDebug(${capitalCase(machine.name)}Logging) << "- event " << eventName(event) << ", params:" << params;
% endif

    switch (_currentState) {
% for s in machine.states:
    case ${machine.name}::${capitalCase(s.name)}:
    % for (i, t) in enumerate(s.transitions):
        // ${t.name}
        % if i == 0:
        if (${parseCondition(t.condition)}) {
        % else:
        else if (${parseCondition(t.condition)}) {
        % endif
        % if t.destination:
            % for a in s.exitingActions:
% if options.debug:
            qCDebug(${capitalCase(machine.name)}Logging) << "   - ${a.type}:${a.name}()";
% endif
            ${formatAction(a)}
            % endfor
        % endif
        % for a in t.actions:
% if options.debug:
            qCDebug(${capitalCase(machine.name)}Logging) << "   - ${a.type}:${a.name}()";
% endif
            ${formatAction(a)}
        % endfor
        % if t.destination:
% if options.debug:
            qCDebug(${capitalCase(machine.name)}Logging) << " - ${capitalCase(s.name)} -> ${t.destination.name}: ${t.name}";
% endif
            _currentState = ${machine.name}::${t.destination.name};
            % for a in t.destination.incomeActions:
% if options.debug:
            qCDebug(${capitalCase(machine.name)}Logging) << "   - ${a.type}:${a.name}()";
% endif
            ${formatAction(a)}
            % endfor
        % endif
            break;
        }
    % endfor
    % if len(s.transitions) == 0:
% if options.debug:
        qCDebug(${capitalCase(machine.name)}Logging) << "- ${s.name}: impasse with event" << eventName( event );
% endif
    % else:
        else {
% if options.debug:
            qCDebug(${capitalCase(machine.name)}Logging) << "- ${s.name}: impasse with event" << eventName( event );
% endif
        }
    % endif
        break;
% endfor
    }
}

void ${machine.name}::processEvent(${machine.name}::Event event, const QVariantMap& params)
{
    bool __empty = eventQueue.isEmpty();

    eventQueue.enqueue(qMakePair(event, params));
    if (__empty) {
        while (!eventQueue.isEmpty()) {
            QPair<Event, QVariantMap> item = eventQueue.head();
            processEventInternal(item.first, item.second);
            eventQueue.dequeue();
        }
    }
}

QString ${machine.name}::eventName(${machine.name}::Event event)
{
    switch (event) {
% for s in machine.events:
    case ${capitalCase(s.name)}:
        return QString::fromLatin1("${capitalCase(s.name)}");
% endfor
    }
    return QString::fromLatin1("Unknown");
}

QString ${machine.name}::stateName(${machine.name}::State state)
{
    switch (state) {
% for s in machine.states:
    case ${capitalCase(s.name)}:
        return QString::fromLatin1("${capitalCase(s.name)}");
% endfor
    }
    return QString::fromLatin1("Unknown");
}

${machine.name}::State ${machine.name}::currentState() const
{
    return _currentState;
}
