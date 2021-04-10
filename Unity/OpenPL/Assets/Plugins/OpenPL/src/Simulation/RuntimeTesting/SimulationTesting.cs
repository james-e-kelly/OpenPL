using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

namespace OpenPL
{
    public class SimulationTesting : MonoBehaviour
    {
        System SystemInstance;
        Scene SceneInstance;

        public bool DebugMeshes;
        public bool ShowVoxels;
        public float VoxelSize = 1f;
        public Vector3 simulationSize = new Vector3(10, 10, 10);

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

        void Start()
        {
            CheckResult(Debug.Initialize(DEBUG_CALLBACK_METHOD), "Debug.Init");

            CheckResult(System.Create(out SystemInstance), "System.Create");

            if(!SystemInstance.HasHandle())
            {
                return;
            }

            CheckResult(SystemInstance.CreateScene(out SceneInstance), "System.CreateScene");

            if (!SceneInstance.HasHandle())
            {
                return;
            }

            CheckResult(SceneInstance.CreateVoxels(simulationSize.ToPLVector(), VoxelSize), "Scene.CreateVoxels");

            AcousticGeometry[] allGameObjects = FindObjectsOfType<AcousticGeometry>();

            for (int i = 0; i < allGameObjects.Length; i++)
            {
                AcousticGeometry currentGameObject = allGameObjects[i];

                if (!currentGameObject)
                {
                    continue;
                }

                MeshFilter mesh = currentGameObject.GetComponent<MeshFilter>();

                if (!mesh)
                {
                    UnityEngine.Debug.LogError("[OpenPL] Acoustic Geometry object found with no mesh. This should not be possible");
                    continue;
                }

                if (!mesh.mesh.isReadable)
                {
                    UnityEngine.Debug.LogWarning("Mesh is not readable. Won't be added to the simulation");
                    continue;
                }

                List<Vector3> Verts = new List<Vector3>();
                mesh.mesh.GetVertices(Verts);
                List<int> Inds = new List<int>();
                mesh.mesh.GetIndices(Inds, 0);

                GCHandle VertHandle = GCHandle.Alloc(Verts.ToArray(), GCHandleType.Pinned);
                GCHandle IndHandle = GCHandle.Alloc(Inds.ToArray(), GCHandleType.Pinned);

                int Index = 0;

                try
                {
                    PLVector pos = mesh.transform.position.ToPLVector();
                    PLQuaternion rot = mesh.transform.rotation.ToPLQuaternion();
                    PLVector scal = mesh.transform.lossyScale.ToPLVector();
                    RESULT MeshResult = SceneInstance.AddMesh(ref pos, ref rot, ref scal, VertHandle.AddrOfPinnedObject(), Verts.Count, IndHandle.AddrOfPinnedObject(), Inds.Count, out Index);
                }
                finally
                {
                    VertHandle.Free();
                    IndHandle.Free();
                }
            }

            CheckResult(SceneInstance.FillVoxelsWithGeometry(), "Scene.FillVoxels");

            if (DebugMeshes)
            {
                CheckResult(SceneInstance.Debug(), "Scene.Debug");
            }

            int Count = 0;
            SceneInstance.GetVoxelsCount(ref Count);

            CheckResult(SceneInstance.Simulate(), "Scene.Simulate");
        }

        void OnDestroy()
        {
            CheckResult(SystemInstance.Release(), "System.Release");
        }

        void OnDrawGizmos()
        {
            Gizmos.color = Color.white;
            Gizmos.DrawWireCube(Vector3.zero, simulationSize);

            if (SystemInstance.HasHandle() && SceneInstance.HasHandle())
            {
                int Count = 0;

                SceneInstance.GetVoxelsCount(ref Count);

                if (Count != 0)
                {
                    for (int i = 0; i < Count; i++)
                    {
                        PLVector Location = new PLVector();
                        float Absorpivity = 0.0f;
                        SceneInstance.GetVoxelLocation(ref Location, i);
                        SceneInstance.GetVoxelAbsorpivity(ref Absorpivity, i);
                        if (Absorpivity > 0f)
                        {
                            Gizmos.color = Color.green;
                            Gizmos.DrawWireCube(Location.ToVector3(), new Vector3(VoxelSize, VoxelSize, VoxelSize));
                        }
                        else if (ShowVoxels)
                        {
                            Color color = Color.white;
                            color.a = 0.1f;
                            Gizmos.color = color;
                            Gizmos.DrawWireCube(Location.ToVector3(), new Vector3(VoxelSize, VoxelSize, VoxelSize));
                        }
                    }
                }
            }
        }
    }
}
