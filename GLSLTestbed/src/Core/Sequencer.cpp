#include "PrecompiledHeader.h"
#include "Core/Sequencer.h"

namespace PKECS
{
    To::To(std::initializer_list<BranchSteps::value_type> branchSteps) : m_branchSteps(branchSteps) {}
    To::To(std::initializer_list<RStep> commonSteps) : m_commonSteps(commonSteps) {}
    To::To(std::initializer_list<BranchSteps::value_type> branchSteps, std::initializer_list<RStep> commonSteps) : m_branchSteps(branchSteps), m_commonSteps(commonSteps) {}

    std::vector<RStep>* To::GetSteps(int condition)
    {
        if (m_branchSteps.count(condition) < 1)
        {
            return nullptr;
        }

        return &m_branchSteps.at(condition);
    }

    Sequencer::Sequencer()
    {
        m_rootEngine = CreateRef<EnginesRoot>();
    }

    Sequencer::~Sequencer()
    {
        m_rootEngine = nullptr;
        m_steps.clear();
    }

    void Sequencer::SetSteps(std::initializer_list<Steps::value_type> steps)
    {
        m_steps = Steps(steps);
    }
}