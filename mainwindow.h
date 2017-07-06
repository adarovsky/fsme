#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUndoGroup>

namespace Ui {
    class MainWindow;
}

class BrowseableManager;
class StateMachine;
class ActionFactory;
class QSpinBox;
class QSessionManager;
class DiagramScene;
class QItemSelection;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void diagramScaleChanged( double );

    virtual void fileNew();
    virtual void fileOpen();
    virtual void fileSave();
    virtual void fileSaveAs();
    virtual void filePrint();

    void addNew();
    void addNewEvent();
    void addNewInput();
    void addNewOutput();
    void addNewState();
    void addNewTransition();
    void deleteItems();

    virtual void fileOpen( QString file );

    void commitDataRequest(QSessionManager & manager);
    void cleanChanged(bool);
    void fileNameChanged(const QString& newName);

protected:
    bool event(QEvent *event);
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
protected slots:
    void openRecent( QAction * );
    void ensureTreeIndexesVisible( const QItemSelection& selected, const QItemSelection& deselected );
    void edit( const QModelIndex& index );

private:
    void showChoicePopup();
    void showStateChoicePopup();

    Ui::MainWindow *m_ui;

    QSpinBox * scaleFactor;
    QSharedPointer<StateMachine> m_stateMachine;
    DiagramScene * m_scene;
    QUndoGroup * m_undoGroup;
    QString startFilePos;
    bool checkModified();
    void updateRecentMenu();
    QList<QString> recent;
    QString fileName;
};

#endif // MAINWINDOW_H
