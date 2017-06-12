# coding: utf-8
<%!
from textwrap import wrap
%>\
#include <QVariant>
#include <QQueue>
#include <QPair>
% if options.debug:
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(${capitalCase(machine.name)}Logging)
% endif

class ${machine.name} {
public:
    enum State {
    % for s in machine.states[:-1]:
        ${capitalCase(s.name)},
    % endfor
        ${capitalCase(machine.states[-1].name)}
    };

    enum Event {
    % for e in machine.events[:-1]:
        % for l in wrap(e.comment, 80):
        //  ${l}
        % endfor
        ${capitalCase(e.name)},
    % endfor
        % for l in wrap(machine.events[-1].comment, 80):
        //  ${l}
        % endfor
        ${capitalCase(machine.events[-1].name)}
    };

    class Delegate {
    public:
    % for i in machine.inputs:
        %if i.comment:
            %for l in wrap(i.comment, 80):
        //  ${l}
            %endfor
        %endif
        virtual bool ${i.name}(const QVariantMap& params) const = 0;
    % endfor

    % for o in machine.outputs:
        %if o.comment:
            % for l in wrap(o.comment, 80):
        //  ${l}
            % endfor
        % endif
        virtual void ${o.name}(const QVariantMap& params) = 0;
    % endfor
    };

    State currentState() const;
    Delegate* delegate;
    QQueue<QPair<Event, QVariantMap> > eventQueue;


    ${machine.name}(Delegate * delegate);
    void processEvent(Event event, const QVariantMap& params);
    static QString eventName(Event event);
    static QString stateName(State s);

private:
    void processEventInternal(Event event, const QVariantMap& params);
    State _currentState;
};
