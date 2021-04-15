using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;

namespace OpenPL
{
    public class SimulationTesting : MonoBehaviour
    {
        System SystemInstance => RuntimeManager.Instance.System;
        Scene SceneInstance => RuntimeManager.Instance.Scene;

        public FMODUnity.StudioEventEmitter eventEmitter;

        public bool DebugMeshes;
        public bool ShowVoxels;
        public float VoxelSize = 1f;
        public Vector3 simulationSize = new Vector3(10, 10, 10);
        public string irSamplePath;

        void Start()
        {
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

            RuntimeManager.CheckResult(SceneInstance.FillVoxelsWithGeometry(), "Scene.FillVoxels");

            if (DebugMeshes)
            {
                RuntimeManager.CheckResult(SceneInstance.Debug(), "Scene.Debug");
            }

            int Count = 0;
            SceneInstance.GetVoxelsCount(ref Count);

            RuntimeManager.CheckResult(SceneInstance.Simulate(), "Scene.Simulate");

            StartCoroutine(WaitForEventInstance());
        }

        IEnumerator WaitForEventInstance()
        {
            yield return new WaitForSeconds(5);

            if (!eventEmitter)
            {
                UnityEngine.Debug.LogError("No event");
                yield break;
            }

            if (eventEmitter.EventInstance.getChannelGroup(out FMOD.ChannelGroup group) == FMOD.RESULT.OK)
            {
                UnityEngine.Debug.Log("Got channel group");
                int dspNum = -1;
                group.getNumDSPs(out dspNum);

                for (int i = 0; i < dspNum; i++)
                {
                    UnityEngine.Debug.Log("Loop");
                    FMOD.DSP dsp;
                    if (group.getDSP(i, out dsp) == FMOD.RESULT.OK)
                    {
                        UnityEngine.Debug.Log("Got dsp");
                        FMOD.DSP_TYPE type;
                        dsp.getType(out type);

                        UnityEngine.Debug.Log(type);

                        if (type == FMOD.DSP_TYPE.CONVOLUTIONREVERB)
                        {
                            UnityEngine.Debug.Log("FOUND REVERB!");

                            int channels, sampleRate;
                            float[] ir = WAV.Read(irSamplePath, out channels, out sampleRate);

                            byte[] array = new byte[ir.Length + 1];
                            array[0] = (byte)channels;
                            Buffer.BlockCopy(array, 0, ir, 1, array.Length);

                            UnityEngine.Debug.Log(dsp.setParameterData((int)FMOD.DSP_CONVOLUTION_REVERB.IR, array));
                        }
                    }
                }
            }
            else
            {
                UnityEngine.Debug.Log("No channel group");
            }
            yield return null;
        }

        void OnDrawGizmos()
        {
            Gizmos.color = Color.white;
            Gizmos.DrawWireCube(Vector3.zero, simulationSize);

            if (RuntimeManager.Instance == null)
            {
                return;
            }

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
