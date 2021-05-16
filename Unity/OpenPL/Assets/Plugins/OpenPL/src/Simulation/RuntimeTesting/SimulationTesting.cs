using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;

namespace OpenPL
{
    public enum MiddlewarePlatform
    {
        FMOD,
        Wwise
    }

    public class SimulationTesting : MonoBehaviour
    {
        System SystemInstance => RuntimeManager.Instance.System;
        Scene SceneInstance => RuntimeManager.Instance.Scene;

        public GameObject Listener;

        public MiddlewarePlatform middlewarePlatform;

        public FMODUnity.StudioEventEmitter eventEmitter;

        public GameObject wwiseEmitter;

        public bool DebugMeshes;
        public bool ShowVoxels;

        public bool disable;

        void Start()
        {
            if (disable)
            {
                return;
            }

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

            listenerLocation = Listener.transform.position;
            SystemInstance.SetListenerPosition(listenerLocation.ToPLVector());

            int Count = 0;
            SceneInstance.GetVoxelsCount(ref Count);

            emitterLocation = eventEmitter.transform.position;

            RuntimeManager.CheckResult(SceneInstance.Simulate(listenerLocation.ToPLVector()), "Scene.Simulate");

            if (middlewarePlatform == MiddlewarePlatform.FMOD)
            {
                StartCoroutine(WaitForEventInstance());
            }
            else if (middlewarePlatform == MiddlewarePlatform.Wwise)
            {
                StartCoroutine(UpdateWwiseSimulation());
            }
        }

        Vector3 emitterLocation;
        Vector3 listenerLocation;

        IEnumerator UpdateWwiseSimulation()
        {
            yield return null;

            while (wwiseEmitter != null)
            {
                listenerLocation = Listener.transform.position;
                emitterLocation = wwiseEmitter.transform.position;
                Vector3 emitterLocationWithListenerY = new Vector3(emitterLocation.x, listenerLocation.y, emitterLocation.z);

                RuntimeManager.CheckResult(SceneInstance.Simulate(listenerLocation.ToPLVector()), "Scene.Simulate");

                float Occlusion;
                RuntimeManager.CheckResult(SceneInstance.GetOcclusion(emitterLocationWithListenerY.ToPLVector(), out Occlusion), "Get Occlusion");

                AkSoundEngine.SetObjectObstructionAndOcclusion(wwiseEmitter, Listener, 0, 1 - Mathf.Clamp01(Occlusion));

                yield return new WaitForSeconds(0.1f);
            }
        }

        IEnumerator UpdateSimulation()
        {
            while (eventEmitter && eventEmitter.IsPlaying())
            {
                listenerLocation = Listener.transform.position;
                emitterLocation = eventEmitter.transform.position;
                Vector3 emitterLocationWithListenerY = new Vector3(emitterLocation.x, listenerLocation.y, emitterLocation.z);

                RuntimeManager.CheckResult(SceneInstance.Simulate(listenerLocation.ToPLVector()), "Scene.Simulate");

                float Occlusion;
                RuntimeManager.CheckResult(SceneInstance.GetOcclusion(emitterLocationWithListenerY.ToPLVector(), out Occlusion), "Get Occlusion");
                //Occlusion -= 1;
                //Occlusion = Mathf.Clamp01(Occlusion);

                //eventEmitter.EventInstance.setVolume(Mathf.Clamp01(Occlusion));
                eventEmitter.EventInstance.getChannelGroup(out FMOD.ChannelGroup group);
                //group.setVolumeRamp(true);
                //group.setVolume(Mathf.Clamp01(Occlusion));
                eventEmitter.SetParameter("Occlusion", 1 - Mathf.Clamp01(Occlusion));


                //UnityEngine.Debug.Log(Occlusion);

                yield return new WaitForSeconds(0.1f);
            }

            yield return null;
        }

        IEnumerator WaitForEventInstance()
        {
            yield return null;

            if (!eventEmitter)
            {
                UnityEngine.Debug.LogError("No event");
                yield break;
            }
            
            yield return UpdateSimulation();
        }

        void OnDrawGizmos()
        {
            if (!RuntimeManager.Instance)
            {
                return;
            }

            Vector3 simulationSize = RuntimeManager.Instance.simulationSize;
            float VoxelSize = RuntimeManager.Instance.VoxelSize;

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
