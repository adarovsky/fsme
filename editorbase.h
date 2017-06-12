#ifndef EDITORBASE_H
#define EDITORBASE_H

#include <QWidget>
#include <QSharedPointer>
#include <QPersistentModelIndex>

class StateMachine;
class EditorBase {
protected:
    QWeakPointer<StateMachine> m_document;
    QSharedPointer<StateMachine> fsm() const;
    QPersistentModelIndex      m_editedObject;
    bool                       m_editing;
public:
    EditorBase();
    void setStateMachine( QSharedPointer<StateMachine> fsm );
    bool edit( const QModelIndex& index );
    QModelIndex editedIndex() const { return m_editedObject; }

    virtual bool canStartEditing(const QModelIndex &index) const = 0;

    /**
     * @brief startEditing
     *
     * Fill editor fields. m_editedObject is set to the index of the object being edited
     */
    virtual void updateUI() = 0;

    /**
     * @brief endEditing
     *
     * Finish editing. m_editedObject will be set to invalid index when this method is called
     */
    virtual void endEditing();
};

#endif // EDITORBASE_H
