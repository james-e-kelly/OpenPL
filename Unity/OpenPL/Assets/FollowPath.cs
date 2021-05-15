using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FollowPath : MonoBehaviour
{
    public Transform[] path;

    int currentTarget = 0;

    public float movementSpeed = 1;
    public float currentLerp = 0;

    Vector3 startPosition;

    IEnumerator Start()
    {
        startPosition = transform.position;

        while (true)
        {
            Transform currentTransform = path[currentTarget];

            transform.position = Vector3.Lerp(startPosition, currentTransform.position, currentLerp);

            currentLerp += Time.deltaTime * movementSpeed;

            if (currentLerp >= 1)
            {
                currentLerp = 0;

                startPosition = transform.position;

                currentTarget++;

                if (currentTarget >= path.Length)
                {
                    currentTarget = 0;
                }
            }
            yield return null;
        }
    }
}
