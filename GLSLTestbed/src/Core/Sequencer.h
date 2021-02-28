#pragma once
#include "PrecompiledHeader.h"
#include "Core/IService.h"
#include "Utilities/Ref.h"

#define PK_STEP_T(S, D) static_cast<PKECS::IStep<D>*>(S)
#define PK_STEP_C(S, D) static_cast<PKECS::IConditionalStep<D>*>(S)
#define PK_STEP_S(S) static_cast<PKECS::ISimpleStep*>(S)

namespace PKECS
{
    class IBaseStep
    {
        protected: virtual ~IBaseStep() = 0 {};
    };

    template <typename T>
    class IStep : public IBaseStep
    {
        protected: virtual ~IStep() = 0 {};
        public: virtual void Step(T* token) = 0;
    };

    template <typename T>
    class IConditionalStep : public IBaseStep
    {
        protected: virtual ~IConditionalStep() = 0 {};
        public: virtual void Step(T* token, int condition) = 0;
    };

    class ISimpleStep : public IConditionalStep<void>
    {
        protected:
            virtual ~ISimpleStep() = 0 {};
        public:
            virtual void Step(int condition) = 0;
            void Step(void* token, int condition) { Step(condition); }
    };

    typedef IBaseStep* StepPtr;
    typedef std::unordered_map<int, std::vector<StepPtr>> BranchSteps;

    class To
    {
        public:
            To(std::initializer_list<BranchSteps::value_type> branchSteps);
            To(std::initializer_list<StepPtr> commonSteps);
            To(std::initializer_list<BranchSteps::value_type> branchSteps, std::initializer_list<StepPtr> commonSteps);
            To(std::initializer_list<StepPtr> commonSteps, std::initializer_list<BranchSteps::value_type> steps);

            const std::vector<StepPtr>* GetSteps(int condition);
            const std::vector<StepPtr>* GetCommonSteps() const { return &m_commonSteps; }
 
        private:
            BranchSteps m_branchSteps;
            std::vector<StepPtr> m_commonSteps;
    };

    typedef std::unordered_map<const void*, To> Steps;

    class Sequencer : public IService
    {
        public:
            void SetSteps(std::initializer_list<Steps::value_type> steps);
            void SetRootSequence(std::initializer_list<int> sequence);
            void ExecuteRootSequence();

            const void* GetRoot() { return this; }

            template<typename T>
            void Next(const void* engine, T* token, int condition)
            {
                if (m_steps.count(engine) < 1)
                {
                    return;
                }

                auto& target = m_steps.at(engine);
                const auto* branchSteps = target.GetSteps(condition);

                if (branchSteps != nullptr)
                {
                    InvokeSteps(branchSteps, token, condition);
                }

                InvokeSteps(target.GetCommonSteps(), token, condition);
            }

            void Release() { m_steps.clear(); }

        private:
            template<typename T>
            void InvokeSteps(const std::vector<StepPtr>* branchSteps, T* token, int condition)
            {
                auto& steps = *branchSteps;

                for (auto& i : steps)
                {
                    if (auto* conditionalStep = dynamic_cast<IConditionalStep<T>*>(i))
                    {
                        conditionalStep->Step(token, condition);
                    }

                    if (auto* step = dynamic_cast<IStep<T>*>(i))
                    {
                        step->Step(token);
                    }
                }
            }

            Steps m_steps;
            std::vector<int> m_rootSequence;
    };
}