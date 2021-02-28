#include "PrecompiledHeader.h"
#include "Core/Sequencer.h"

namespace PKECS
{
    To::To(std::initializer_list<BranchSteps::value_type> branchSteps) : m_branchSteps(branchSteps) {}
    To::To(std::initializer_list<StepPtr> commonSteps) : m_commonSteps(commonSteps) {}
    To::To(std::initializer_list<BranchSteps::value_type> branchSteps, std::initializer_list<StepPtr> commonSteps) : m_branchSteps(branchSteps), m_commonSteps(commonSteps) {}

    const std::vector<StepPtr>* To::GetSteps(int condition)
    {
        if (m_branchSteps.count(condition) < 1)
        {
            return nullptr;
        }

        return &m_branchSteps.at(condition);
    }

    void Sequencer::SetSteps(std::initializer_list<Steps::value_type> steps)
    {
        m_steps = Steps(steps);
    }

    void Sequencer::SetRootSequence(std::initializer_list<int> sequence)
    {
        m_rootSequence = std::vector<int>(sequence);
    }

    void Sequencer::ExecuteRootSequence()
    {
        for (auto i : m_rootSequence)
        {
            Next<void>(this, nullptr, i);
        }
    }
}