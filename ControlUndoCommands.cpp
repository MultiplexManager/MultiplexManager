#include "ControlUndoCommands.h"

#include <QSpinBox>
#include <QCheckBox>
#include "OptionData.h"

#include <cassert>

bool SpinnerUndoCommand::s_InUndoRedo = false;

void SpinnerUndoCommand::undo()
{
    s_InUndoRedo = true ;
    m_Spinner->setValue( m_Before );
    s_InUndoRedo = false;
}

void SpinnerUndoCommand::redo()
{    
    s_InUndoRedo = true ;
    m_Spinner->setValue( m_After );
    s_InUndoRedo = false;
}

bool SpinnerUndoCommand::mergeWith( const QUndoCommand* command )
{
    if ( command->id() != id () )
    {
        return false;
    }

    const SpinnerUndoCommand* cmd = static_cast<const SpinnerUndoCommand*>(command);
    
    m_After = cmd->m_After;

    return true;
}


bool CheckUndoCommand::s_InUndoRedo = false;

void CheckUndoCommand::undo()
{
    s_InUndoRedo = true ;
    m_Check->setChecked( !m_After );
    s_InUndoRedo = false;
}

void CheckUndoCommand::redo()
{    
    s_InUndoRedo = true ;
    m_Check->setChecked( m_After );
    s_InUndoRedo = false;
}

bool CheckUndoCommand::mergeWith( const QUndoCommand* command )
{
    if ( command->id() != id () )
    {
        return false;
    }

    const CheckUndoCommand* cmd = static_cast<const CheckUndoCommand*>(command);
    
    m_After = cmd->m_After;

    return true;
}


void CriteriaUndoCommand::undo()
{
	m_OptionData->m_Criteria = m_Before;
}

    
void CriteriaUndoCommand::redo()
{
	m_OptionData->m_Criteria = m_After;
}

bool CriteriaUndoCommand::mergeWith( const QUndoCommand* command )
{
    if ( command->id() != id () )
    {
        return false;
    }

    const CriteriaUndoCommand* cmd = static_cast<const CriteriaUndoCommand*>(command);
    
    m_After = cmd->m_After;

    return true;
}
