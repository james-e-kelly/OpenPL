using System;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace OpenPL
{
    public static class PLATFORM
    {
#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
        public const string dll = "OpenPL.dll";
#elif UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX
        public const string dll = "OpenPL.dylib";
#elif UNITY_EDITOR_LINUX || UNITY_STANDALONE_LIXUX
        public const string dll = "libOpenPL.so";
#else
        public const string dll = "OpenPL";
#endif
    }

    public enum RESULT
    {
        /** Method executed successfully*/
        OK,
        /** Generic error has occured without any further reason why*/
        ERR,
        /** An error occured with memory. Maybe a null reference was found or memory failed to allocate/deallocate*/
        ERR_MEMORY,
        /** An argument passed into the function is invalid. For example, an indices array not being a multiple of 3*/
        ERR_INVALID_PARAM
    }

    public enum DEBUG_LEVEL
    {
        Log,
        Warn,
        Error
    }

    public delegate RESULT DEBUG_CALLBACK([MarshalAs(UnmanagedType.LPStr)] string Message, DEBUG_LEVEL Level);

    [StructLayout(LayoutKind.Sequential)]
    public struct PLVector
    {
        public float X;
        public float Y;
        public float Z;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct PLQuaternion
    {
        public float X;
        public float Y;
        public float Z;
        public float W;
    }

    public static class Utils
    {
        public static PLVector ToPLVector(this Vector3 vector3)
        {
            PLVector Return;
            Return.X = vector3.x;
            Return.Y = vector3.y;
            Return.Z = vector3.z;
            return Return;
        }

        public static PLQuaternion ToPLQuaternion(this Quaternion quat)
        {
            PLQuaternion Return;
            Return.X = quat.x;
            Return.Y = quat.y;
            Return.Z = quat.z;
            Return.W = quat.w;
            return Return;
        }

        public static Vector3 ToVector3(this PLVector pLVector)
        {
            Vector3 vector3;
            vector3.x = pLVector.X;
            vector3.y = pLVector.Y;
            vector3.z = pLVector.Z;
            return vector3;
        }
    }

    public struct Debug
    {
        public static RESULT Initialize(DEBUG_CALLBACK Callback)
        {
            return PL_Debug_Initialize(Callback);
        }

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Debug_Initialize(DEBUG_CALLBACK Callback);
    }

    public struct System
    {
        /// <summary>
        /// Create a system object
        /// </summary>
        /// <param name="System"></param>
        /// <returns></returns>
        public static RESULT Create(out System System)
        {
            return PL_System_Create(out System.Handle);
        }

        /// <summary>
        /// Release a system object
        /// </summary>
        /// <returns></returns>
        public RESULT Release()
        {
            return PL_System_Release(Handle);
        }

        /// <summary>
        /// Create a scene objec that will be owned by this system object
        /// </summary>
        /// <param name="Scene"></param>
        /// <returns></returns>
        public RESULT CreateScene(out Scene Scene)
        {
            return PL_System_CreateScene(Handle, out Scene.Handle);
        }

        public RESULT SetListenerPosition(PLVector ListenerPosition)
        {
            return PL_System_SetListenerPosition(Handle, ListenerPosition);
        }

        public RESULT GetListenerPosition(out PLVector ListernPosition)
        {
            return PL_System_GetListenerPosition(Handle, out ListernPosition);
        }

        public IntPtr Handle;

        public System(IntPtr Ptr) { Handle = Ptr; }
        public bool HasHandle() { return Handle != IntPtr.Zero; }
        public void ClearHandle() { Handle = IntPtr.Zero; }

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_System_Create(out IntPtr OutSystem);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_System_Release(IntPtr System);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_System_CreateScene(IntPtr System, out IntPtr OutScene);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_System_SetListenerPosition(IntPtr System, PLVector ListenerPosition);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_System_GetListenerPosition(IntPtr System, out PLVector ListenerPosition);
    }

    public struct Scene
    {
        /// <summary>
        /// Release this scene object
        /// </summary>
        /// <returns></returns>
        public RESULT Release()
        {
            return PL_Scene_Release(Handle);
        }

        /// <summary>
        /// Add a game mesh to the simulation
        /// </summary>
        /// <param name="WorldPosition"></param>
        /// <param name="WorldRotation"></param>
        /// <param name="WorldScale"></param>
        /// <param name="Vertices"></param>
        /// <param name="VerticesLength"></param>
        /// <param name="Indices"></param>
        /// <param name="IndicesLength"></param>
        /// <param name="OutIndex"></param>
        /// <returns></returns>
        public RESULT AddMesh(ref PLVector WorldPosition, ref PLQuaternion WorldRotation, ref PLVector WorldScale, IntPtr Vertices, int VerticesLength, IntPtr Indices, int IndicesLength, out int OutIndex)
        {
            return PL_Scene_AddMesh(Handle, ref WorldPosition, ref WorldRotation, ref WorldScale, Vertices, VerticesLength, Indices, IndicesLength, out OutIndex);
        }

        /// <summary>
        /// Remove a mesh from the simulation
        /// </summary>
        /// <param name="IndexToRemove"></param>
        /// <returns></returns>
        public RESULT RemoveMesh(int IndexToRemove)
        {
            return PL_Scene_RemoveMesh(Handle, IndexToRemove);
        }

        /// <summary>
        /// Open an OpenGL window to debug the geometry
        /// </summary>
        /// <returns></returns>
        public RESULT Debug()
        {
            return PL_Scene_Debug(Handle);
        }

        /// <summary>
        /// Get the number of voxels in the simulation
        /// </summary>
        /// <param name="OutVoxelCount"></param>
        /// <returns></returns>
        public RESULT GetVoxelsCount(ref int OutVoxelCount)
        {
            return PL_Scene_GetVoxelsCount(Handle, ref OutVoxelCount);
        }

        /// <summary>
        /// Get the location of a voxel
        /// </summary>
        /// <param name="OutVoxelLocation"></param>
        /// <param name="Index"></param>
        /// <returns></returns>
        public RESULT GetVoxelLocation(ref PLVector OutVoxelLocation, int Index)
        {
            return PL_Scene_GetVoxelLocation(Handle, ref OutVoxelLocation, Index);
        }

        /// <summary>
        /// Get the absorpivity of a voxel
        /// </summary>
        /// <param name="OutVoxelAbsorpivity"></param>
        /// <param name="Index"></param>
        /// <returns></returns>
        public RESULT GetVoxelAbsorpivity(ref float OutVoxelAbsorpivity, int Index)
        {
            return PL_Scene_GetVoxelAbsorpivity(Handle, ref OutVoxelAbsorpivity, Index);
        }

        /// <summary>
        /// Run the wave simulation
        /// </summary>
        /// <returns></returns>
        public RESULT Simulate(PLVector SimulationLocation)
        {
            return PL_Scene_Simulate(Handle, SimulationLocation);
        }

        public RESULT CreateVoxels(PLVector SceneSize, float VoxelSize)
        {
            return PL_Scene_CreateVoxels(Handle, SceneSize, VoxelSize);
        }

        public RESULT FillVoxelsWithGeometry()
        {
            return PL_Scene_FillVoxelsWithGeometry(Handle);
        }

        public RESULT DrawGraph(PLVector GraphPosition)
        {
            return PL_Scene_DrawGraph(Handle, GraphPosition);
        }

        public RESULT Encode(PLVector EncodingPosition)
        {
            return PL_Scene_Encode(Handle, EncodingPosition);
        }

        public IntPtr Handle;

        public Scene(IntPtr Ptr) { Handle = Ptr; }
        public bool HasHandle() { return Handle != IntPtr.Zero; }
        public void ClearHandle() { Handle = IntPtr.Zero; }

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_Release(IntPtr Scene);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_AddMesh(IntPtr Scene, ref PLVector WorldPosition, ref PLQuaternion WorldRotation, ref PLVector WorldScale, IntPtr Vertices, int VerticesLength, IntPtr Indices, int IndicesLength, out int OutIndex);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_RemoveMesh(IntPtr Scene, int IndexToRemove);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_Debug(IntPtr Scene);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_GetVoxelsCount(IntPtr Scene, ref int OutVoxelCount);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_GetVoxelLocation(IntPtr Scene, ref PLVector OutVoxelLocation, int Index);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_GetVoxelAbsorpivity(IntPtr Scene, ref float OutVoxelAbsorpivity, int Index);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_Simulate(IntPtr Scene, PLVector SimulationLocation);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_CreateVoxels(IntPtr Scene, PLVector SceneSize, float VoxelSize);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_FillVoxelsWithGeometry(IntPtr Scene);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_DrawGraph(IntPtr Scene, PLVector GraphPosition);

        [DllImport(PLATFORM.dll)]
        static extern RESULT PL_Scene_Encode(IntPtr Scene, PLVector EncodingPosition);
    }
}
