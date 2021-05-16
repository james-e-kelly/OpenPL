using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RaycastingExample : MonoBehaviour
{
    public Transform listener;

    public FMODUnity.StudioEventEmitter eventEmitter;

    [Range(0,1)]
    public float occlusionStrength;

    IEnumerator Start()
    {
        yield return null;

        while (eventEmitter && eventEmitter.IsPlaying())
        {
            Vector3 listenerLocation = listener.position;
            Vector3 emitterLocation = eventEmitter.transform.position;
            Vector3 direction = (listenerLocation - emitterLocation).normalized;

            if (Physics.Raycast(emitterLocation, direction, 40f))
            {
                eventEmitter.SetParameter("Occlusion", occlusionStrength);
            }
            else
            {
                eventEmitter.SetParameter("Occlusion", 0);
            }

            yield return new WaitForSeconds(0.1f);
        }
    }
}
