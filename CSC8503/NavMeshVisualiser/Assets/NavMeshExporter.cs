using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AI;

public class NavMeshExporter : MonoBehaviour
{
    [SerializeField]
    bool createDebugMesh = false;
    [SerializeField]
    string filename;
    // Start is called before the first frame update

    [SerializeField]
    bool invertZ;

    void Start()
    {
        ExtractToFile(filename);
    }

    class NavTri
    {
        public int[] indices    = new int[3];
        public int[] neighbours = new int[3];

        public int neighbourCount = 0;

        public NavTri()
        {
            indices[0] = indices[1] = indices[2] = -1;
            neighbours[0] = neighbours[1] = neighbours[2] = -1;
        }

        public bool HasIndex(int index)
        {
            for(int i = 0; i < 3; ++i)
            {
                if(indices[i] == index)
                {
                    return true;
                }
            }
            return false;
        }

        public bool SharesEdgeWith(NavTri t)
        {
            for(int i = 0; i < 3; ++i)
            {
                int a = indices[i];
                int b = indices[(i + 1) % 3];

                if (t.HasEdge(a, b))
                {
                    return true;
                }
            }
            return false;
        }

        bool HasEdge(int ea, int eb)
        {
            for (int i = 0; i < 3; ++i)
            {
                int a = indices[i];
                int b = indices[(i + 1) % 3];

                if(a == ea && b == eb)
                {
                    return true; //they'd be having the same winding?
                }
                if (b == ea && a == eb)
                {
                    return true; 
                }
            }
            return false;
        }

        public void AddNeighbour(int newIndex)
        {
            if(neighbourCount == 3)
            {
                Debug.Log("Tri has too many neighbours?");
                return;
            }
            neighbours[neighbourCount++] = newIndex;
        }

        public bool HasNeighbour(int nIndex)
        {
            for (int i = 0; i < 3; ++i)
            {
                if(neighbours[i] == nIndex)
                {
                    return true;
                }
            }
            return false;
        }
    };

    void Weld(Vector3[] verts, int[] indices, float threshold = 0.0001f)
    {
        int weldCount = 0;

        List<int> outIndices = new List<int>();

        for (int i = 0; i < indices.Length; i++)
        {
            Vector3 iv = verts[indices[i]];
            for (int j = i + 1; j < indices.Length; j++)
            {
                if(indices[j] == indices[i])
                {
                    continue;
                }
                Vector3 jv = verts[indices[j]];

                if ((iv - jv).magnitude < threshold)
                {
                    indices[j] = indices[i];
                    weldCount++;
                    break;
                }
            }
        }
        Debug.Log("Welded " + weldCount + " vertices");
    }

    void ExtractToFile(string filename)
    {
        NavMeshTriangulation tris = NavMesh.CalculateTriangulation();

        Vector3[]   allverts   = tris.vertices;
        int[]       allIndices = tris.indices;

        /*
         * We have a problem - Unity doesn't weld vertices properly,
         * So verts that are actually the same don't get shared
         * which kinda breaks the neighbours code
         */
        Weld(allverts, allIndices);

        List<NavTri> allTris            = new List<NavTri>();

        for (int i = 0; i < allIndices.Length; i += 3)
        {
            NavTri t = new NavTri();
            //We need to swap the indices to maintain the winding
            if(invertZ) 
            {
                t.indices[0] = allIndices[i + 0];
                t.indices[1] = allIndices[i + 2];
                t.indices[2] = allIndices[i + 1];
            }
            else
            {
                t.indices[0] = allIndices[i + 0];
                t.indices[1] = allIndices[i + 1];
                t.indices[2] = allIndices[i + 2];
            }

            allTris.Add(t);
        }

        //Build up the neighbours

        for (int j = 0; j < allTris.Count; ++j)
        {
            if (allTris[j].neighbourCount == 3)
            {
                continue;
            }
            for (int i = j+1; i < allTris.Count; ++i)
            {
                if(allTris[i].neighbourCount == 3)
                {
                    continue;
                }
                if(allTris[j].SharesEdgeWith(allTris[i]))
                {
                    allTris[j].AddNeighbour(i);
                    allTris[i].AddNeighbour(j);
                }
            }
        }

        //Sanity checking the neighbours!

        for(int j = 0; j < allTris.Count; ++j)
        {
            for(int i = 0; i < 3; ++i)
            {
                int nIndex = allTris[j].neighbours[i];

                if(nIndex == -1)
                {
                    continue;
                }
                if(!allTris[nIndex].HasNeighbour(j))
                {
                    Debug.Log("Tri " + j + " thinks it is connected to tri " + nIndex + " but not vice versa?!");
                }
                else
                {
                    int sharedCount = 0;
                    for (int jj = 0; jj < 3; ++jj)
                    {

                        //ok, they think they are neighbours, but should they be?
                        if (allTris[nIndex].HasIndex(allTris[j].indices[jj]))
                        {
                            sharedCount++;
                        }
                        if (allTris[j].HasIndex(allTris[nIndex].indices[jj]))
                        {
                            sharedCount++;
                        }
                    }
                    if(sharedCount == 0)
                    {
                        Debug.Log("These tris shouldn't be connected? No shared indices...");
                    }
                }
            }
        }



        using (System.IO.StreamWriter file = new System.IO.StreamWriter(filename))
        {
            file.Write(allverts.Length); file.Write('\n');
            file.Write(allIndices.Length); file.Write('\n');

            foreach (Vector3 v in allverts)
            {
                string s = "";
                s += v.x;
                s += " ";    
                s += v.y;
                s += " ";
                if(invertZ)
                {
                    s += -v.z;
                }
                else
                {
                    s += v.z;
                }

                s += '\n';
                file.Write(s);
            }

            foreach (NavTri t in allTris)
            {
                string s = "";
                s += t.indices[0];
                s += " ";
                s += t.indices[1];
                s += " ";
                s += t.indices[2];
                s += '\n';
                file.Write(s);
            }

            foreach (NavTri t in allTris)
            {
                string s = "";
                s += t.neighbours[0];
                s += " ";
                s += t.neighbours[1];
                s += " ";
                s += t.neighbours[2];
                s += '\n';
                file.Write(s);
            }
        }
        if (createDebugMesh)
        {
            Mesh m = new Mesh();
            List<Vector3> testMeshVerts = new List<Vector3>(allverts);
            m.SetVertices(testMeshVerts);
            m.SetIndices(allIndices, MeshTopology.Triangles, 0);

            MeshFilter mf = gameObject.AddComponent<MeshFilter>();
            mf.mesh = m;
            MeshRenderer mr = gameObject.AddComponent<MeshRenderer>();
        }
    }
}
