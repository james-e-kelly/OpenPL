using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Attached to a game object, marks the object as neccesary for the acoustic simulation
/// </summary>
[RequireComponent(typeof(MeshFilter))]
[DisallowMultipleComponent]
public class AcousticGeometry : MonoBehaviour { }
