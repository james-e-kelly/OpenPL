using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace OpenPL
{
    public class RuntimeManager : MonoBehaviour
    {
        System system;
        Scene scene;

        public float VoxelSize = 1f;
        public Vector3 simulationSize = new Vector3(10, 10, 10);

        public System System => system;
        public Scene Scene => scene;

        static RuntimeManager instance;
        public static RuntimeManager Instance => instance;

        void CheckResult(RESULT Result, string Message)
        {
            if (Result != RESULT.OK)
            {
                UnityEngine.Debug.LogError($"[OpenPL] {Result} : {Message}");
            }
        }

        [AOT.MonoPInvokeCallback(typeof(DEBUG_CALLBACK))]
        static RESULT DEBUG_CALLBACK_METHOD(string Message, DEBUG_LEVEL Level)
        {
            if (Level == DEBUG_LEVEL.Log)
            {
                UnityEngine.Debug.Log(string.Format(("[OpenPL] {0}"), Message));
            }
            else if (Level == DEBUG_LEVEL.Warn)
            {
                UnityEngine.Debug.LogWarning(string.Format(("[OpenPL] {0}"), Message));
            }
            else if (Level == DEBUG_LEVEL.Error)
            {
                UnityEngine.Debug.LogError(string.Format(("[OpenPL] {0}"), Message));
            }
            return RESULT.OK;
        }

        private void Awake()
        {
            instance = this;
        }

        private void Start()
        {
            CheckResult(Debug.Initialize(DEBUG_CALLBACK_METHOD), "Debug.Init");

            CheckResult(System.Create(out system), "System.Create");

            if (!system.HasHandle())
            {
                return;
            }

            CheckResult(system.CreateScene(out scene), "System.CreateScene");

            if (!scene.HasHandle())
            {
                return;
            }

            CheckResult(scene.CreateVoxels(simulationSize.ToPLVector(), VoxelSize), "Scene.CreateVoxels");
        }
    }
}
