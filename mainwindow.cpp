#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QSpinBox>
#include <QSettings>
#include <QDomDocument>
#include <QCloseEvent>
#include <QSessionManager>
#include <QMenu>
#include <QtDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "statemachine.h"
#include "diagramscene.h"
#include "scopeguard.h"
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_scene( new DiagramScene(this) ),
    m_undoGroup(new QUndoGroup(this))
{
    m_ui->setupUi(this);
    statusBar()->hide();

    QAction * undo = m_undoGroup->createUndoAction(this);
    undo->setIcon(QIcon(":/images/undo.png"));
    undo->setShortcut(QKeySequence::Undo);
    QAction * redo = m_undoGroup->createRedoAction(this);
    redo->setIcon(QIcon(":/images/redo.png"));
    redo->setShortcut(QKeySequence::Redo);

    m_ui->actionUndo->setShortcut(QKeySequence::Undo);
    connect(m_undoGroup, &QUndoGroup::canUndoChanged, m_ui->actionUndo, &QAction::setEnabled);
    connect(m_undoGroup, &QUndoGroup::undoTextChanged, m_ui->actionUndo, &QAction::setToolTip);
    connect(m_ui->actionUndo, &QAction::triggered, m_undoGroup, &QUndoGroup::undo);

    m_ui->actionRedo->setShortcut(QKeySequence::Redo);
    connect(m_undoGroup, &QUndoGroup::canRedoChanged, m_ui->actionRedo, &QAction::setEnabled);
    connect(m_undoGroup, &QUndoGroup::redoTextChanged, m_ui->actionRedo, &QAction::setToolTip);
    connect(m_ui->actionRedo, &QAction::triggered, m_undoGroup, &QUndoGroup::redo);

    m_ui->menu_Edit->addAction(undo);
    m_ui->menu_Edit->addAction(redo);

//    scaleFactor = new QSpinBox();
//    scaleFactor->setMinimum(10);
//    scaleFactor->setMaximum(1000);
//    scaleFactor->setValue(100);
//    scaleFactor->setSuffix(tr(" %"));
//    m_ui->toolBar->addWidget(scaleFactor);

//    m_ui->diagramView->setAlignment( 0 );

    m_ui->diagramView->setScene(m_scene);
    m_ui->diagramView->setSceneRect(0, 0, 1, 1);

    QSettings s;

    startFilePos = s.value("/daa/fsme/recent dir" ).toString();

    connect(m_ui->actionAdd, SIGNAL(triggered()), SLOT(addNew()));
    connect(m_ui->actionFitScale, &QAction::triggered, m_ui->diagramView, [=]() {
        m_ui->diagramView->fitInView(m_ui->diagramView->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
    });
    connect(m_ui->actionDefaultScale, &QAction::triggered, m_ui->diagramView, [=]() {
        m_ui->diagramView->setTransform( QTransform::fromScale( 1, 1 ) );
    });

//    connect(m_ui->diagram, SIGNAL(scaleChanged(double)), SLOT(diagramScaleChanged(double)));
//    connect(scaleFactor, SIGNAL(valueChanged(int)), SLOT(updateScale(int)));

    connect (m_ui->action_New, SIGNAL(triggered()), SLOT(fileNew()));
    connect (m_ui->action_Open, SIGNAL(triggered()), SLOT(fileOpen()));
    connect (m_ui->actionSave, &QAction::triggered, this, &MainWindow::fileSave);
    connect (m_ui->actionSaveAs, &QAction::triggered, this, &MainWindow::fileSaveAs);
    connect (m_ui->action_Quit, SIGNAL(triggered()), SLOT(close()));
    connect (m_undoGroup, SIGNAL(cleanChanged(bool)), SLOT(cleanChanged(bool)));
    connect (qApp, SIGNAL(commitDataRequest(QSessionManager&)), SLOT(commitDataRequest(QSessionManager&)));

//    connect (m_ui->action_Print, SIGNAL(triggered()), m_ui->diagram, SLOT(print()));
    connect(m_ui->recentFiles, SIGNAL(triggered(QAction*)), SLOT(openRecent(QAction *)));

    connect(m_scene, SIGNAL(requestEditingItem(QModelIndex)), SLOT(edit(QModelIndex)));
    connect(m_scene, &QGraphicsScene::sceneRectChanged, this, [=](const QRectF&) {
        auto items = m_scene->itemsBoundingRect();
        auto prev = m_ui->diagramView->sceneRect();
        m_ui->diagramView->setSceneRect( items.adjusted(-items.width(), -items.height(), items.width(), items.height()) );
        auto offset = QPointF((items.width() - prev.width()) / 2, (items.height() - prev.height()) / 2);
        m_ui->diagramView->translate( offset.x(), offset.y() );
    });

    QByteArray geometry = s.value("/daa/fsme/tree slider").toByteArray();
    if (!geometry.isEmpty())
        m_ui->treeSplitter->restoreState(geometry);

    geometry = s.value("/daa/fsme/editor slider").toByteArray();
    if (!geometry.isEmpty())
        m_ui->editorSplitter->restoreState(geometry);

    int recentSize = s.beginReadArray("/daa/fsme/recent files");
    for(int index = 0; index < recentSize; ++index) {
        s.setArrayIndex(index);
        recent.append(s.value("recentFile").toString());
    }
    s.endArray();
    updateRecentMenu();

    QByteArray windowSave;

    windowSave = s.value("/daa/fsme/main window geometry").toByteArray();
    if (!windowSave.isEmpty())
        restoreGeometry(windowSave);

    windowSave = s.value("/daa/fsme/main window state").toByteArray();
    if (!windowSave.isEmpty())
        restoreState(windowSave);

    fileNew();
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
        return m_scene->handleCancel(dynamic_cast<QKeyEvent*>(event));

    return QMainWindow::event(event);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

MainWindow::~MainWindow()
{
    m_scene->clearSelection();
    m_scene->clear();
    m_stateMachine.clear();
    delete m_ui;
}

void MainWindow::fileNew()
{
    QSharedPointer<StateMachine> old = m_stateMachine;
    m_stateMachine.reset( new StateMachine() );
    m_stateMachine->clear();
    fileName = QString::null;
    m_undoGroup->setActiveStack(m_stateMachine->undoStack());
    if (old) old->undoStack()->clear();

    m_ui->treeView->setModel(m_stateMachine.data());
    m_ui->treeView->expandToDepth(1);
    m_scene->setStateMachine( m_stateMachine );
    m_ui->editorStack->setStateMachine( m_stateMachine );
    m_scene->setItemSelectionModel( m_ui->treeView->selectionModel() );
    connect( m_ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(ensureTreeIndexesVisible(QItemSelection,QItemSelection)) );
    m_ui->diagramView->centerOn(QPointF(0, 0));
    m_ui->actionDelete->setEnabled( false );
    emit fileNameChanged(fileName);
}

void MainWindow::fileOpen()
{
    if( checkModified() ) {
        QString file = QFileDialog::getOpenFileName( this,
                                                     tr("Choose File to open"),
                                                     startFilePos,
                                                     tr("Finite State Machines (*.fsm)\nAllFiles (*)"));
        if( !file.isNull() ) {
            fileOpen( file );
        }
    }
}

void MainWindow::fileOpen( QString file )
{
    fileName = file;
    QFileInfo fi(fileName);
    startFilePos = fi.dir().absolutePath();

    recent.removeAll(fi.absoluteFilePath());
    recent.push_front(fi.absoluteFilePath());
    updateRecentMenu();
    emit fileNameChanged(fileName);

    QDomDocument doc;
    QFile f( fileName );
    QString error;
    int     errorLine, errorCol;
    if( !doc.setContent( &f, false, &error, &errorLine, &errorCol ) ) {
        QMessageBox::critical( this, tr("State Machine Editor"),
                               tr("%1:%2:Error reading XML description: %3").arg(errorLine).arg(errorCol).arg(error),
                               QMessageBox::Cancel, QMessageBox::NoButton );
        fileNew();
        return;
    }

    QSharedPointer<StateMachine> old = m_stateMachine;
    m_stateMachine.reset( new StateMachine() );
    m_stateMachine->clear();

    m_stateMachine->load( doc );

    m_ui->treeView->setModel(m_stateMachine.data());
    m_scene->setStateMachine( m_stateMachine );
    m_ui->editorStack->setStateMachine( m_stateMachine );
    m_undoGroup->setActiveStack(m_stateMachine->undoStack());
    if (old) old->undoStack()->clear();

    m_ui->treeView->expandToDepth(1);
    m_scene->setItemSelectionModel( m_ui->treeView->selectionModel() );
    m_ui->actionDelete->setEnabled( false );
    connect( m_ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(ensureTreeIndexesVisible(QItemSelection,QItemSelection)) );
    m_ui->diagramView->centerOn(m_scene->itemsBoundingRect().center());
}



void MainWindow::fileSave()
{
    if( fileName.isEmpty() )
        fileSaveAs();
    else {
        QDomDocument doc("FSM");
        QDomElement root = doc.createElement( "FSM" );
        doc.appendChild( root );
        m_stateMachine->save(doc, root);

        QFile f( fileName );

        QFileInfo fi( f );
        recent.removeAll(fi.absoluteFilePath());
        recent.push_front(fi.absoluteFilePath());
        updateRecentMenu();

        if( !f.open(QIODevice::WriteOnly) ) {
            QMessageBox::critical( this, tr("State Machine Editor"),
                                   tr("Cannot open file %1").arg(fileName),
                                   QMessageBox::Cancel, QMessageBox::NoButton );
            return;
        }
        QTextStream stream( &f );
        doc.save( stream, 4 );
        m_undoGroup->activeStack()->setClean();
    }
}

void MainWindow::fileSaveAs()
{
    QString file = QFileDialog::getSaveFileName( this,
                                                 tr("Choose File to save to"),
                                                 startFilePos,
                                                 tr("Finite State Machines (*.fsm)\nAllFiles (*)"));

    if( !file.isEmpty() ) {
        fileName = file;
        QFileInfo fi( fileName );
        if( fi.completeSuffix().isEmpty() )
            fileName += ".fsm";
        fileSave();

        fi.setFile( fileName );
        startFilePos = fi.dir().absolutePath();

        recent.removeAll(fi.absoluteFilePath());
        recent.push_front(fi.absoluteFilePath());
        updateRecentMenu();
        emit fileNameChanged(fileName);
    }
}

void MainWindow::filePrint()
{
    qWarning( "MainWindow::filePrint(): Not implemented yet" );
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_undoGroup->isClean()) {
        int x = QMessageBox::warning(this,
                                     tr("Changes are not saved"),
                                     tr("Do you want to save your changes?"),
                                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);


        switch (x) {
        case QMessageBox::Save:
            fileSave();
            event->accept();
        case QMessageBox::Discard:
            event->accept();
            break;
        case QMessageBox::Cancel:
        default:
            event->ignore();
        }
    }

    QSettings s;
    s.setValue("/daa/fsme/main window geometry", saveGeometry());
    s.setValue("/daa/fsme/main window state", saveState());

    s.setValue("/daa/fsme/editor slider", m_ui->editorSplitter->saveState());
    s.setValue("/daa/fsme/tree slider", m_ui->treeSplitter->saveState());

    s.beginWriteArray("/daa/fsme/recent files", recent.size());
    int index = 0;
    foreach(QString f, recent) {
        s.setArrayIndex(index++);
        s.setValue("recentFile", f);
    }
    s.endArray();
}

void MainWindow::commitDataRequest(QSessionManager& manager)
{
    if (!m_undoGroup->isClean() && manager.allowsInteraction()) {
        int x = QMessageBox::warning(this,
                                     tr("Changes are not saved"),
                                     tr("Do you want to save your changes?"),
                                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);


        switch (x) {
        case QMessageBox::Save:
            fileSave();
            manager.release();
        case QMessageBox::Discard:
            break;
        case QMessageBox::Cancel:
        default:
            manager.cancel();
        }
    }
}

bool MainWindow::checkModified()
{
    if( !m_undoGroup->isClean() )
        return QMessageBox::question(this,
                                     tr("Changes are not saved"),
                                     tr("Do you really want to discard your changes?"),
                                     QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Discard;
    else
        return true;
}

void MainWindow::cleanChanged(bool clean)
{
    m_ui->actionSave->setEnabled( !clean );
    setWindowModified(!clean);
}

void MainWindow::fileNameChanged(const QString& newName)
{
    if( !newName.isEmpty() ) {
        QFileInfo fi(newName);
        setWindowTitle(tr( "%1 — State Machine Editor").arg(fi.baseName()));
        setWindowFilePath(newName);
    }
    else {
        setWindowTitle(tr( "State Machine Editor - untitled"));
        setWindowFilePath(QString::null);
    }
}

void MainWindow::diagramScaleChanged( double s )
{
    scaleFactor->blockSignals( true );
    scaleFactor->setValue( int(s*100) );
    scaleFactor->blockSignals( false );
}

void MainWindow::openRecent( QAction * action )
{
    if( checkModified() )
        fileOpen( action->data().toString() );
}

typedef QPair<QFileInfo, int> QFileVariant;
static QString lastName(QFileVariant item)
{
    if (item.second == 0)
        return item.first.fileName();
    else {
        QDir d = item.first.dir();
        for(int i = 1; i < item.second; ++i)
            d.cdUp();
        if (d.isRoot())
            return QString::null;
        return d.dirName();
    }
}

void MainWindow::updateRecentMenu()
{
    m_ui->recentFiles->clear();
    QVector <QFileVariant> variants;
    variants.reserve(recent.count());

    foreach( QString s, recent ) {
        QFileInfo f (s);
        if ( f.exists() )
            variants.append(qMakePair(f, 0));
    }

    forever {
        bool noDuplicates = true;
        for (int i = 0; i < variants.count(); ++i) {
            QString li = lastName(variants[i]);
            bool dup = false;
            for (int j = 0; j < variants.count(); ++j) {
                QString lj = lastName(variants[j]);
                if (i != j && variants[i].first.fileName() == variants[j].first.fileName()
                       && !lj.isNull() && li == lj) {
                    dup = true;
                    variants[j].second++;
                }
            }
            if (dup) {
                variants[i].second++;
                noDuplicates = false;
            }
        }

        if (noDuplicates)
            break;
    }

    foreach (QFileVariant pair, variants) {
        QString name = pair.second == 0 ? pair.first.fileName() : QString("%1 - %2").arg(pair.first.fileName(), lastName(pair));
        QAction * action = new QAction(name, this);
        action->setData(pair.first.filePath());
        m_ui->recentFiles->addAction(action);
    }
}

void MainWindow::ensureTreeIndexesVisible(const QItemSelection& selected, const QItemSelection& deselected )
{
    Q_UNUSED( deselected );
    bool canDelete = false;
    foreach (auto i, selected.indexes()) {
        m_ui->treeView->scrollTo( i );

        EIOBase * p = reinterpret_cast<EIOBase*>( i.internalPointer() );
        if (p && p->canDelete())
            canDelete = true;
    }

    m_ui->editorStack->selectionChanged(selected, deselected);

    m_ui->actionDelete->setEnabled( canDelete );

    QString addNewTitle = tr("Add New...");

    QModelIndex index = m_ui->treeView->selectionModel()->selectedIndexes().count() == 1 ? m_ui->treeView->selectionModel()->selectedIndexes().first() : QModelIndex();
    EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
    if ( !p ) {
        if (index == m_stateMachine->eventsFolder())
            m_ui->actionAdd->setText( tr("Add Event") );
        else if (index == m_stateMachine->inputsFolder())
            m_ui->actionAdd->setText( tr("Add Input") );
        else if (index == m_stateMachine->outputsFolder())
            m_ui->actionAdd->setText( tr("Add Output") );
        else if (index == m_stateMachine->statesFolder())
            m_ui->actionAdd->setText( tr("Add State") );
        else
            m_ui->actionAdd->setText( tr("Add New...") );
        return;
    }

    if ( dynamic_cast<Event*>( p ) ) {
        m_ui->actionAdd->setText( tr("Add Event") );
        return;
    }
    else if ( dynamic_cast<Input*>( p ) ) {
        m_ui->actionAdd->setText( tr("Add Input") );
        return;
    }
    else if ( dynamic_cast<Output*>( p ) ) {
        m_ui->actionAdd->setText( tr("Add Output") );
        return;
    }
    else if ( dynamic_cast<State*>( p ) ) {
        m_ui->actionAdd->setText( tr("Add Transition...") );
        return;
    }
    else if ( dynamic_cast<Transition*>( p ) ) {
        m_ui->actionAdd->setText( tr("Add Transition...") );
        return;
    }
    m_ui->actionAdd->setText( tr("Add New...") );
}

void MainWindow::edit(const QModelIndex& item )
{
    m_ui->treeView->scrollTo( item );
    m_ui->treeView->selectionModel()->select( item, QItemSelectionModel::ClearAndSelect );
    m_ui->treeView->edit( item );

    EIOBase * p = reinterpret_cast<EIOBase*>( item.internalPointer() );
    bool canDelete = false;
    if (p && p->canDelete())
        canDelete = true;

    m_ui->actionDelete->setEnabled( canDelete );
}

void MainWindow::deleteItems()
{
    m_undoGroup->activeStack()->beginMacro(tr("Delete"));
    scope_guard macro([=]() {
        m_undoGroup->activeStack()->endMacro();
    });

    foreach (auto i, m_ui->treeView->selectionModel()->selectedIndexes()) {
        EIOBase * p = reinterpret_cast<EIOBase*>( i.internalPointer() );
        if (p && p->canDelete())
            m_undoGroup->activeStack()->push( p->deleteCommand() );
    }
}

void MainWindow::addNew()
{
    QModelIndex index = m_ui->treeView->selectionModel()->selectedIndexes().count() == 1 ? m_ui->treeView->selectionModel()->selectedIndexes().first() : QModelIndex();
    EIOBase * p = reinterpret_cast<EIOBase*>( index.internalPointer() );
    if ( !p ) {
        if (index == m_stateMachine->eventsFolder())
            addNewEvent();
        else if (index == m_stateMachine->inputsFolder())
            addNewInput();
        else if (index == m_stateMachine->outputsFolder())
            addNewOutput();
        else if (index == m_stateMachine->statesFolder())
            addNewState();
        else
            showChoicePopup();
        return;
    }

    if ( dynamic_cast<Event*>( p ) ) {
        addNewEvent();
        return;
    }
    else if ( dynamic_cast<Input*>( p ) ) {
        addNewInput();
        return;
    }
    else if ( dynamic_cast<Output*>( p ) ) {
        addNewOutput();
        return;
    }
    else if ( dynamic_cast<State*>( p ) ) {
        showStateChoicePopup();
        return;
    }
    else if ( dynamic_cast<Transition*>( p ) ) {
        m_ui->treeView->selectionModel()->select( m_stateMachine->indexForItem( dynamic_cast<Transition*>( p )->parentState().data() ), QItemSelectionModel::ClearAndSelect );
        addNewTransition();
        return;
    }

    showChoicePopup();
}

void MainWindow::addNewEvent()
{
    auto * cmd = new EventAddCommand( m_stateMachine );
    m_stateMachine->undoStack()->push( cmd );
    auto e = cmd->object();
    m_ui->treeView->edit( m_stateMachine->indexForItem( e.data() ) );
}

void MainWindow::addNewInput()
{
    auto * cmd = new InputAddCommand( m_stateMachine );
    m_stateMachine->undoStack()->push( cmd );
    auto e = cmd->object();
    m_ui->treeView->edit( m_stateMachine->indexForItem( e.data() ) );
}

void MainWindow::addNewOutput()
{
    auto * cmd = new OutputAddCommand( m_stateMachine );
    m_stateMachine->undoStack()->push( cmd );
    auto e = cmd->object();
    m_ui->treeView->edit( m_stateMachine->indexForItem( e.data() ) );
}

void MainWindow::addNewState()
{
    m_scene->prepareToAddState();
}

void MainWindow::addNewTransition()
{
    m_scene->prepareToAddTransition();
}

void MainWindow::showChoicePopup()
{
    QMenu * menu = new QMenu( this );
    menu->addAction( tr("Add &Event", "Add|Event"), this, SLOT(addNewEvent()), QKeySequence(tr("E", "Add|Event")) );
    menu->addAction( tr("Add &Input", "Add|Input"), this, SLOT(addNewInput()), QKeySequence(tr("I", "Add|Input")) );
    menu->addAction( tr("Add &Output", "Add|Output"), this, SLOT(addNewInput()), QKeySequence(tr("O", "Add|Output")) );
    menu->addAction( tr("Add &State", "Add|State"), this, SLOT(addNewState()), QKeySequence(tr("S", "Add|State")) );
    menu->addAction( tr("Add &Transition", "Add|Transition"), this, SLOT(addNewTransition()), QKeySequence(tr("T", "Add|Transition")) );

    menu->exec( QCursor::pos() );
}

void MainWindow::showStateChoicePopup()
{
    QMenu * menu = new QMenu( this );
    menu->addAction( tr("Add &State", "Add|State"), this, SLOT(addNewState()), QKeySequence(tr("S", "Add|State")) );
    auto a = menu->addAction( tr("Add &Transition", "Add|Transition"), this, SLOT(addNewTransition()), QKeySequence(tr("T", "Add|Transition")) );
    QRect bounds = menu->actionGeometry(a);

    menu->exec( QCursor::pos() - QPoint(32, bounds.height() / 2), a );
}
