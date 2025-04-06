#ifndef CONTROL_UNDO_COMMANDS_H
#define CONTROL_UNDO_COMMANDS_H

#include <QUndoCommand>

class QSpinBox;
class QCheckBox;

class SpinnerUndoCommand : public QUndoCommand
{
public:
    SpinnerUndoCommand( QSpinBox* spinner, quint32 before, quint32 after, const QString & thing  )
        : m_Spinner( spinner ),
          m_Before( before ),
          m_After( after )
    {
        setText( "Change " + thing );

    }

    virtual void undo();
    
    virtual void redo();

    static bool InUndoRedo()
    {
        return s_InUndoRedo;
    }

    virtual int id() const
    {
        return (int) (size_t) m_Spinner;
    }
    
    virtual bool mergeWith( const QUndoCommand* command );

 
private:

    static bool s_InUndoRedo;

    QSpinBox* m_Spinner;
    quint32 m_Before;
    quint32 m_After;
    
    
};


class CheckUndoCommand : public QUndoCommand
{
public:
    CheckUndoCommand( QCheckBox* spinner, bool after, const QString & thing  )
        : m_Check( spinner ),
          m_After( after )
    {
        setText( "Change " + thing );
        
    }

    virtual void undo();
    
    virtual void redo();

    static bool InUndoRedo()
    {
        return s_InUndoRedo;
    }

    virtual int id() const
    {
        return (int) (size_t) m_Check;
    }
    
    virtual bool mergeWith( const QUndoCommand* command );

 
private:

    static bool s_InUndoRedo;

    QCheckBox* m_Check;
    bool m_After;
    
    
};


class OptionData;

class CriteriaUndoCommand : public QUndoCommand
{
public:
    CriteriaUndoCommand(  OptionData* optionData, const QList<qint32> before, const QList<qint32> after )
        : m_OptionData( optionData )
		, m_Before( before )          
		, m_After( after )
    {
        setText( "Changed Criteria Ordering"  );
    }

    virtual void undo();
    
    virtual void redo();

    virtual bool mergeWith( const QUndoCommand* command );

    virtual int id() const
    {
        return (int) (size_t) m_OptionData;
    }

	
private:
	OptionData* m_OptionData;
	QList<qint32> m_Before;
	QList<qint32> m_After;

};



#endif
