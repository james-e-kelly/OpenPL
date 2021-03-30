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
        public float VoxelSize = 1f;

        void CheckResult(RESULT Result, string Message)
        {
            if (Result != RESULT.OK)
            {
                UnityEngine.Debug.LogError($"[OpenPL] {Result} : {Message}");
            }
        }

        void Start()
        {
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

            GameObject[] allGameObjects = FindObjectsOfType<GameObject>();

            for (int i = 0; i < allGameObjects.Length; i++)
            {
                GameObject currentGameObject = allGameObjects[i];

                if (!currentGameObject || !currentGameObject.isStatic)
                {
                    continue;
                }

                MeshFilter mesh = currentGameObject.GetComponent<MeshFilter>();

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


            PLVector Pos = new Vector3(0, 0, 0).ToPLVector();
            PLVector Size = new Vector3(10, 10, 10).ToPLVector();

            CheckResult(SceneInstance.Voxelise(ref Pos, ref Size, VoxelSize), "Scene.Voxelise");

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
                        //else
                        //{
                        //    Color color = Color.white;
                        //    color.a = 0.1f;
                        //    Gizmos.color = color;

                        //}

                    }
                }
            }
        }
    }
}
