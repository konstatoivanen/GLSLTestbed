#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Ref.h"

namespace PKECS
{
    class IBaseStep
    {
        protected: virtual ~IBaseStep() = 0 {};
    };

    class IEngine : public std::enable_shared_from_this<IEngine>
    {
        protected: virtual ~IEngine() = 0 {};
    };

    struct DefaultToken {};

    template <typename T>
    class IStep : public IBaseStep
    {
        protected: virtual ~IStep() = 0 {};
        public: virtual void Step(T& token) = 0;
    };

    template <typename T>
    class IConditionalStep : public IBaseStep
    {
        protected: virtual ~IConditionalStep() = 0 {};
        public: virtual void Step(T& token, int condition) = 0;
    };

    class ISimpleStep : public IStep<DefaultToken>
    {
        protected:
            virtual ~ISimpleStep() = 0 {};
        public:
            virtual void Step() = 0;
            void Step(DefaultToken& token) { Step(); }
    };

    typedef Ref<IBaseStep> RStep;
    typedef Ref<IEngine> REngine;
    typedef std::unordered_map<int, std::vector<RStep>> BranchSteps;

    class To
    {
        public:
            To(std::initializer_list<BranchSteps::value_type> branchSteps);
            To(std::initializer_list<RStep> commonSteps);
            To(std::initializer_list<BranchSteps::value_type> branchSteps, std::initializer_list<RStep> commonSteps);
            To(std::initializer_list<RStep> commonSteps, std::initializer_list<BranchSteps::value_type> steps);

            std::vector<RStep>* GetSteps(int condition);
            std::vector<RStep>* GetCommonSteps() { return &m_commonSteps; }
 
        private:
            BranchSteps m_branchSteps;
            std::vector<RStep> m_commonSteps;
    };

    typedef std::unordered_map<REngine, To> Steps;

    class Sequencer
    {
        class EnginesRoot : public IEngine
        {
        };

        public:
            Sequencer();
            ~Sequencer();

            void SetSteps(std::initializer_list<Steps::value_type> steps);
            REngine& GetRoot() { return m_rootEngine; }

            template<typename T>
            void ExecuteCommand(T& token, int condition) { Next<T>(m_rootEngine, token, condition); }

            void ExecuteCommand(int condition) 
            {
                auto token = DefaultToken();
                ExecuteCommand(token, condition);
            }

            template<typename T>
            void Next(REngine engine, T& token, int condition)
            {
                if (m_steps.count(engine) < 1)
                {
                    return;
                }

                auto& to = m_steps.at(engine);

                std::vector<RStep>* branchSteps = to.GetSteps(condition);

                if (branchSteps != nullptr)
                {
                    InvokeSteps(token, condition, *branchSteps);
                }

                InvokeSteps(token, condition, *to.GetCommonSteps());
            }

            void Release()
            {
                m_rootEngine = nullptr;
                m_steps.clear();
            }

        private:
            template<typename T>
            void InvokeSteps(T& token, int condition, std::vector<RStep>& branchSteps)
            {
                for (auto& i : branchSteps)
                {
                    auto step = std::dynamic_pointer_cast<IConditionalStep<T>>(i);
                    auto simpleStep = std::dynamic_pointer_cast<IStep<T>>(i);

                    if (step)
                    {
                        step->Step(token, condition);
                    }

                    if (simpleStep)
                    {
                        simpleStep->Step(token);
                    }
                }
            }

            REngine m_rootEngine;
            Steps m_steps;
    };
}