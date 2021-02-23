#pragma once

/*
using System.Collections.Generic;

namespace Framework.ECS
{
    public interface IStep
    {
    }

    public interface IStep<T> : IStep
    {
        void Step(ref T token, int condition);
    }

    public interface ISimpleStep<T> : IStep
    {
        void Step(ref T token);
    }

    public interface ISequencer
    {
        void Next<T>(IEngine engine, ref T param, int condition = 0);
    }

    public class To : Dictionary<int, IStep[]>
    {
        private readonly List<IStep> commonSteps = new List<IStep>();

        public IEnumerable<IStep> CommonSteps => commonSteps;

        public void Add(IStep engine)
        {
            commonSteps.Add(engine);
        }

        public void Add(IStep[] engines)
        {
            commonSteps.AddRange(engines);
        }

        public void Add(int condition, IStep engine)
        {
            Add(condition, new[] { engine });
        }
    }

    public class Steps : Dictionary<IEngine, To>
    {
    }

    public class Sequencer : ISequencer
    {
        private Steps steps;

        public void SetSequence(Steps steps)
        {
            this.steps = steps;
        }

        public void Next<T>(IEngine engine, ref T param, int condition)
        {
            To targets;
            if (!steps.TryGetValue(engine, out targets))
            {
                return;
            }

            IStep[] branchSteps;
            if (steps[engine].TryGetValue(condition, out branchSteps))
            {
                InvokeSteps(ref param, condition, branchSteps);
            }

            InvokeSteps(ref param, condition, targets.CommonSteps);
        }

        private static void InvokeSteps<T>(ref T param, int condition, IEnumerable<IStep> branchSteps)
        {
            foreach (var step in branchSteps)
            {
                (step as IStep<T>)?.Step(ref param, condition);
                (step as ISimpleStep<T>)?.Step(ref param);
            }
        }
    }
}

*/