QT += widgets xml
HEADERS += mainwindow.h       \
    qdombasic.h        \
    qdomqvaluelist.h   \
    statemachine.h     \
    streamable.h \
    eiobase.h \
    nameexception.h \
    event.h \
    input.h \
    output.h \
    root.h \
    state.h \
    transition.h \
    diagramview.h \
    diagramscene.h \
    diagramstateitem.h \
    gsavers.h \
    diagramtransitionitem.h \
    chaikinarrow.h \
    scopeguard.h \
    diagramtransitionmanipulator.h \
    my_algo.h \
    transitionundocommand.h \
    stateaddplaceholder.h \
    stateundocommand.h \
    editorpanel.h \
    editorbase.h \
    editortransition.h \
    editorstate.h \
    editorevent.h \
    editorinput.h \
    editoroutput.h \
    outputlistmodel.h \
    availableactionsmodel.h \
    syntax/exception.h \
    syntax/lex.h \
    syntax/scanner.h \
    syntax/scanner_atm.h \
    syntax/semantic.h \
    syntax/syntax.h \
    syntax/type.h \
    syntaxhighlighter.h \
    eventsinputsmodel.h \
    transitiontargetsmodel.h \
    linearsubsetmodel.h \
    textedit.h \
    application.h \
    editorroot.h \
    rootundocommand.h


SOURCES += main.cpp           \
    mainwindow.cpp     \
    qdombasic.cpp      \
    statemachine.cpp   \
    streamable.cpp \
    eiobase.cpp \
    event.cpp \
    input.cpp \
    output.cpp \
    root.cpp \
    state.cpp \
    transition.cpp \
    diagramview.cpp \
    diagramscene.cpp \
    diagramstateitem.cpp \
    gsavers.cpp \
    diagramtransitionitem.cpp \
    chaikinarrow.cpp \
    diagramtransitionmanipulator.cpp \
    transitionundocommand.cpp \
    stateaddplaceholder.cpp \
    stateundocommand.cpp \
    editorpanel.cpp \
    editorbase.cpp \
    editortransition.cpp \
    editorstate.cpp \
    editorevent.cpp \
    editorinput.cpp \
    editoroutput.cpp \
    outputlistmodel.cpp \
    availableactionsmodel.cpp \
    syntax/func.cpp \
    syntax/functions.cpp \
    syntax/keywords.cpp \
    syntax/scanner.cpp \
    syntax/scanner_atm.cpp \
    syntax/semantic.cpp \
    syntax/syntax.cpp \
    syntax/type.cpp \
    syntaxhighlighter.cpp \
    eventsinputsmodel.cpp \
    transitiontargetsmodel.cpp \
    linearsubsetmodel.cpp \
    textedit.cpp \
    application.cpp \
    editorroot.cpp \
    rootundocommand.cpp

FSMS = diagramscenefsm.fsm manipfsm.fsm

DEFINES += USE_SHARED_PTR

RESOURCES += ui.qrc
FORMS += mainwindow.ui \
    editorbase.ui \
    editorstate.ui \
    editortransition.ui \
    editorroot.ui

mac {
    ICON = fsme.icns
    QMAKE_INFO_PLIST = Info.plist
}

fsmheader.output  = ${QMAKE_FILE_BASE}.h
fsmheader.commands = $${PWD}/fsmtools/mkfsm.py -d -t $${PWD}/fsmtools/fsmheader.mako ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
fsmheader.input = FSMS
fsmheader.name = fsm headers
fsmheader.variable_out = HEADERS

fsmbody.output  = ${QMAKE_FILE_BASE}.cpp
fsmbody.commands = $${PWD}/fsmtools/mkfsm.py -d -t $${PWD}/fsmtools/fsmbody.mako ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
fsmbody.depends = ${QMAKE_FILE_BASE}.h ${QMAKE_FILE_NAME}
fsmbody.input = FSMS
fsmbody.name = fsm implementations
fsmbody.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += fsmheader fsmbody
