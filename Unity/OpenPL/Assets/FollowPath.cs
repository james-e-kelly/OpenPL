using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FollowPath : MonoBehaviour
{
    public Transform[] path;

    int currentTarget = 0;

    public float movementSpeed = 1;

    IEnumerator Start()
    {
        while (true)
        {
            Transform currentTransform = path[currentTarget];

            transform.position = Vector3.MoveTowards(transform.position, currentTransform.position, Time.deltaTime * movementSpeed);

            if (Vector3.Distance(transform.position, currentTransform.position) < 0.01f)
            {
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
