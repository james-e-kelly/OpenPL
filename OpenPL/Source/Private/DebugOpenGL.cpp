/*
  ==============================================================================

    DebugOpenGL.cpp
    Created: 8 Apr 2021 1:50:11pm
    Author:  James Kelly

  ==============================================================================
*/

#include "DebugOpenGL.h"
#include <igl/opengl/glfw/Viewer.h>
#include "PL_SCENE.h"

PL_RESULT OpenOpenGLDebugWindow(PL_SCENE* Scene)
{
    igl::opengl::glfw::Viewer viewer;
    const std::vector<PL_MESH>* Meshes = nullptr;
    PL_RESULT GetMeshesResult = Scene->GetMeshes(&Meshes);
    
    if (GetMeshesResult != PL_OK || Meshes == nullptr)
    {
        DebugWarn("Could not get meshes to render");
        return PL_ERR;
    }
    
    for (auto& Mesh : *Meshes)
    {
        // Transpose the matrices because we store vectors as:
        // {x1, x2, x3}
        // {y1, y2, y3}
        // {z1, z2, z3}
        // However, OpenGL expects:
        // {x1, y1, z1}
        // {x2, y2, z2}
        // {x3, y3, z3}
        viewer.data().set_mesh(Mesh.Vertices.transpose(), Mesh.Indices.transpose());
        viewer.append_mesh(true);
        
        Eigen::Vector3d MeshMin = Mesh.Vertices.rowwise().minCoeff();
        Eigen::Vector3d MeshMax = Mesh.Vertices.rowwise().maxCoeff();
        Eigen::AlignedBox<double, 3> MeshBounds (MeshMin, MeshMax);
        
        VertexMatrix BoundingBoxPoints(8,3);
        BoundingBoxPoints <<
        MeshMin(0), MeshMin(1), MeshMin(2),
        MeshMax(0), MeshMin(1), MeshMin(2),
        MeshMax(0), MeshMax(1), MeshMin(2),
        MeshMin(0), MeshMax(1), MeshMin(2),
        MeshMin(0), MeshMin(1), MeshMax(2),
        MeshMax(0), MeshMin(1), MeshMax(2),
        MeshMax(0), MeshMax(1), MeshMax(2),
        MeshMin(0), MeshMax(1), MeshMax(2);
        
        // Edges of the bounding box
        IndiceMatrix E_box(12,2);
        E_box <<
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        7 ,3;
        
        viewer.data().add_points(BoundingBoxPoints,Eigen::RowVector3d(1,0,0));

        // Plot the edges of the bounding box
        for (unsigned i=0;i<E_box.rows(); ++i)
          viewer.data().add_edges
          (
           BoundingBoxPoints.row(E_box(i,0)),
           BoundingBoxPoints.row(E_box(i,1)),
            Eigen::RowVector3d(1,0,0)
          );
    }
    
    int Success = viewer.launch();
    
    if (Success == EXIT_SUCCESS)
    {
        return PL_OK;
    }
    
    return PL_ERR;
}
