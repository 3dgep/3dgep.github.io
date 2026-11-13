---
title: "GPU Skinning of MD5 Models in OpenGL and Cg"
description: "In this tutorial, I will extend the MD5 model rendering to provide support for GPU skinning. I will also provide an example shader that will perform the vertex skinning in the vertex shader and do…"
slug: gpu-skinning-of-md5-models-in-opengl-and-cg
lastUpdated: 2023-12-07
---

<span id="attachment-1555"></span><img src="/images/Bob-with-Lamp-GPU-Skinning.jpg" alt="Bob with Lamp (GPU Skinning)" width="150" class="thumb" />

:::caution
NVIDIA discontinued the Cg language and toolkit in 2012. The skinning technique shown here is unchanged in modern shading languages: the Cg vertex program maps almost line for line to GLSL or HLSL (`float4x4` becomes `mat4`, `mul(M, v)` becomes `M * v`).
:::

This tutorial builds upon the previous article titled [\[Loading and Animating MD5 Models with OpenGL\]](/loading-and-animating-md5-models-with-opengl/). It is highly recommended that you read the previous article before following this one. In this tutorial, I will extend the MD5 model rendering to provide support for GPU skinning. I will also provide an example shader that will perform the vertex skinning in the vertex shader and do per-fragment lighting on the model using a single point light. For a complete discussion on lighting in CgFX, you can refer to my previous article titled *Transformation and Lighting in Cg*.  
  

## Introduction

In skeletal animation, vertex skinning is the process of transforming the vertex position and normal of a mesh based on the matrix of animated bones that the vertex is weighted to. Before the advent of the programmable shader pipeline in graphics hardware, it was necessary to compute the position and normal of every vertex of a mesh on the CPU and upload the vertex information to the GPU before the animated model could be rendered correctly. Using a programmable vertex shader, we can upload the vertex information of the mesh to GPU memory then for subsequent renders, we pass the transformed bones of the animated skeleton to the GPU and allow the vertex program to compute the animated vertex position and normals. The benifit is that instead of sending thousands of vertices to the GPU every frame, only a small fraction of data needs to be sent to animate the entire model.

In this example, I will use vertex buffer objects (VBO’s) to store the model’s vertex information directly in GPU memory and render the animated model using a custom vertex shader and fragment shader written in the Cg shader language.

## Dependencies

The demo shown in this article uses several 3rd party libraries to simplify the development process.

-   [The Cg Toolkit (Version 3)](https://developer.nvidia.com/cg-toolkit): The Cg Toolkit provides the tools and API needed to integrate the Cg shader programs in your application.
-   [Boost (1.46.1)](https://www.boost.org/): Boost has some very useful libraries that I use throughout my demo applications. In this demo, I use the Signals, Filesystem, Function, and Bind boost libraries to provide a generic, platform independent functionality that simplifies some of the features used in this demo.
-   [Simple DirectMedia Layer (1.2.14)](https://www.libsdl.org/): Simple DirectMedia Layer (SDL) is a cross-platform multimedia library that I use to create the main application window, initialize OpenGL, and handle keyboard, mouse and joystick input.
-   [OpenGL Mathmatics (GLM)](https://glm.g-truc.net/): An OpenGL centric mathmatics library for 3D graphics applications.
-   [Simple OpenGL Image Library (SOIL)](https://web.archive.org/web/20200728145723/http://lonesock.net/soil.html): SOIL is a tiny C library used primarily for uploading textures into OpenGL.

Any dependencies used by the demo are included in the source distribution available at the bottom of the article so you can hopefully just unzip, compile and run the included samples.

## Skeletal Animation

When an animator creates a skeletal animated character in a modeling package (like 3D Studio Max, Maya, or Blender) the animator must perform a process of weighting each vertex of the mesh to a number of bones that represents the skeleton. This process of weighting vertices to bones is called “rigging”.

Once the model is correctly rigged the animator will export the model together with the animations that are associated with that model. In some cases the same animation can be applied to multiple models. In order to correctly animate the model together with a particular animation we must be able to transform the animated skeletal structure into a form that makes sense to the model we are trying to animate. In order to do that we need some reference pose that represents the mesh in it’s “identity” pose (the pose of the model if no animation is applied). This “identity” pose is called the “bind” pose.

The bind pose is very important for vertex skinning because we will use the bind pose to transform the animated bone matrices back into a form that makes sense for our model.

Once we have the animated bone matrices we can apply them to the vertex positions and normals based on the amount of weight that each vertex is assigned to the bone. The result is an animated character model with correct vertex positions and normals.

Let’s see how we can do this in practice.

## The MD5Model Class

At this point you should have throughly read the previous article on loading and animating MD5 models because now I will only discuss the differences between that implementation and one that performs the vertex transformations on the GPU.

### The Header File

In order to optimize the mesh rendering, it makes sense to store the vertex information in vertex buffer objects (VBOs) and upload the vertex information in the model’s bind pose to the GPU when the model is loaded the first time. In order to support the VBOs, we need to store VBO ID’s for each sub-mesh of the model.

We also need to store two additional streams that will be used to transform the vertex positions on the GPU.

The bone weights will be stored in the **m\_BoneWeights** buffer. Each vertex will store up to four weights for a maximum of four bones that can be weighted to each vertex. Generally four bones is enough to animate the vertices of the mesh and for this demo, the MD5 model we are loading also does not use more than four bones per vertex. The bone weights for each vertex will be packed into a 4-component floating-point vector.

The bone indices will be stored in the **m\_BoneIndex** buffer. Each vertex will store up to four indices for a maximum of four bones per vertex that can be applied to the animated vertex position.

```cpp showLineNumbers startLineNumber=82
;"  title="MD5Model.h"]    struct Mesh
    {
        Mesh()
            : m_GLPositionBuffer(0)
            , m_GLNormalBuffer(0)
            , m_GLBoneWeights(0)
            , m_GLBoneIndex(0)
            , m_GLTexCoord(0)
            , m_GLIndexBuffer(0)
        {}

        std::string     m_Shader;
        Material        m_Material;
        // This vertex list stores the vertices's in the bind pose.
        VertexList      m_Verts;
        TriangleList    m_Tris;
        WeightList      m_Weights;

        // A texture ID for the material
        GLuint          m_TexID;

        // These buffers are used for rendering the animated mesh
        PositionBuffer  m_PositionBuffer;   // Vertex position stream
        NormalBuffer    m_NormalBuffer;     // Vertex normals stream
        WeightBuffer    m_BoneWeights;      // Bone weights buffer
        BoneIndexBuffer m_BoneIndex;       // Bone index buffer

        Tex2DBuffer     m_Tex2DBuffer;      // Texture coordinate set
        IndexBuffer     m_IndexBuffer;      // Vertex index buffer

        // Vertex buffer Object IDs for the vertex streams
        GLuint          m_GLPositionBuffer;
        GLuint          m_GLNormalBuffer;
        GLuint          m_GLBoneWeights;
        GLuint          m_GLBoneIndex;
        GLuint          m_GLTexCoord;
        GLuint          m_GLIndexBuffer;        
    };
```

I’ve highlighted additional declarations for the Mesh object.

Since we will be using a CgFX shader to transform our vertex positions, we need to associate an effect to the model. So I added a parameterized constructor that takes a reference to an **Effect**.

```cpp title="MD5Model.h" showLineNumbers startLineNumber=19
    explicit MD5Model(Effect& effect);
```

In addition to the constructor, I’ve also added a function to compute the bind pose and the inverse bind pose matrices for every joint in the model from the model’s initial joints.

```cpp title="MD5Model.h" showLineNumbers startLineNumber=123
    // Build the bind-pose and the inverse bind-pose matrix array for the model.
    void BuildBindPose( const JointList& joints );
```

And a few additional member variables to store the bind pose and inverse bind pose matrices for each joint of the model. And an array of matrices that will store the animated bone matrices pre-multiplied by the inverse bind pose.

```cpp title="MD5Model.h" showLineNumbers startLineNumber=160
    typedef std::vector<glm::mat4x4> MatrixList;

    MatrixList          m_BindPose;
    MatrixList          m_InverseBindPose;

    // Animated bone matrix from the animation with the inverse bind pose applied.
    MatrixList          m_AnimatedBones;
```

And of course, we need to store the reference to the effect that will be used to render the model.

```cpp title="MD5Model.h" showLineNumbers startLineNumber=171
    // The Cg shader effect that is used to render this model.
    Effect&             m_Effect;
```

This model class supports both CPU and GPU vertex skinning so we define a member variable that lets us switch between the two skinning modes.

```cpp title="MD5Model.h" showLineNumbers startLineNumber=176
    enum VertexSkinning
    {
        VS_CPU,
        VS_GPU
    };

    // Perform vertex skinning on the CPU or the GPU
    VertexSkinning      m_VertexSkinning;
```

### The MD5ModelClass File

When the model is destroyed, we also have to delete all of the vertex buffer objects that were created for the meshes. For simplicity, we’ll create a few helper functions that we can use to create and destroy vertex buffer objects.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=29
void DeleteVertexBuffer( GLuint& vboID )
{
    if ( vboID != 0 )
    {
        glDeleteBuffersARB( 1, &vboID );
        vboID = 0;
    }
}

inline void CreateVertexBuffer( GLuint& vboID )
{
    // Make sure we don't loose the reference to the previous VBO if there is one
    DeleteVertexBuffer( vboID );
    glGenBuffersARB( 1, &vboID );
}
```

And in the model’s destructor, we have to delete the vertex buffer object for all the submeshes of the model.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=58
MD5Model::~MD5Model()
{
    MeshList::iterator iter = m_Meshes.begin();
    while ( iter != m_Meshes.end() )
    {
        DestroyMesh( *iter );
        ++iter;
    }
    m_Meshes.clear();
}

void MD5Model::DestroyMesh( Mesh& mesh )
{
    // Delete all the VBO's
    DeleteVertexBuffer( mesh.m_GLPositionBuffer );
    DeleteVertexBuffer( mesh.m_GLNormalBuffer );
    DeleteVertexBuffer( mesh.m_GLTexCoord );
    DeleteVertexBuffer( mesh.m_GLBoneIndex );
    DeleteVertexBuffer( mesh.m_GLBoneWeights );
    DeleteVertexBuffer( mesh.m_GLIndexBuffer );
}
```

The **MD5Model::LoadModel** method has also been slightly modified to build the bind pose and the inverse bind pose matrices for each joint of the model. Sine the joint’s bind-pose is defined in the “joints” section of the MD5 model file, we can build the bind pose matrices after the joints have been read in.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=152
        else if ( param == "joints" )
        {
            Joint joint;
            file >> junk; // Read the '{' character
            for ( int i = 0; i < m_iNumJoints; ++i )
            {
                file >> joint.m_Name >> joint.m_ParentID >> junk
                     >> joint.m_Pos.x >> joint.m_Pos.y >> joint.m_Pos.z >> junk >> junk
                     >> joint.m_Orient.x >> joint.m_Orient.y >> joint.m_Orient.z >> junk;

                RemoveQuotes( joint.m_Name );
                ComputeQuatW( joint.m_Orient );

                m_Joints.push_back(joint);
                // Ignore everything else on the line up to the end-of-line character.
                IgnoreLine( file, fileLength );
            }
            file >> junk; // Read the '}' character

            BuildBindPose( m_Joints );
        }
```

I’ve highlighted the additional line.

Also, after each mesh has been imported in the **MD5Model::LoadModel** method, we will call a method to create and populate the vertex buffer objects of each mesh.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=265
            PrepareMesh(mesh);
            PrepareNormals(mesh);
            CreateVertexBuffers(mesh);

            m_Meshes.push_back(mesh);
```

Again, I have highlighted the additional line of code.

### The MD5Model::BuildBindPose Method

In the **MD5Model::BuildBindPose** method we will use the model’s “joints” definition to build the bind-pose, and an inverse bind-pose matrix for each joint of the model.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=315
void MD5Model::BuildBindPose( const JointList& joints )
{
    m_BindPose.clear();
    m_InverseBindPose.clear();

    JointList::const_iterator iter = joints.begin();
    while ( iter != joints.end() )
    {
        const Joint& joint = (*iter);
        glm::mat4x4 boneTranslation = glm::translate( joint.m_Pos );
        glm::mat4x4 boneRotation = glm::toMat4( joint.m_Orient );

        glm::mat4x4 boneMatrix = boneTranslation * boneRotation;

        glm::mat4x4 inverseBoneMatrix = glm::inverse( boneMatrix );

        m_BindPose.push_back( boneMatrix );
        m_InverseBindPose.push_back( inverseBoneMatrix );

        ++iter;
    }
}
```

To build the bind-pose matrix array of the MD5 model, we simply build a translation and rotation matrix from the joint’s position and orientation parameters and create the combined matrix of the joint by multiplying these two matrices. The inverse bind-pose matrix is simply the inverse of the bind-pose matrix as seen on line 329.

Since the joint’s orientation is stored as a quaternion, we need to convert it to a 4×4 rotation matrix before we can create the compound homogeneous transformation matrix. Luckily, the GLM math library provides a function for doing this conversion.

Then we store these matrices for each joint in the model in the bind-pose and the inverse bind-pose vector containers.

It is not actually necessary to store the bind-pose matrix after we have calculated the inverse bind pose of the joints. Only the inverse bind-pose matrix is needed when updating the animation of the model.

### The MD5Model::PrepareMesh Method

There are also a few changes that need to be made to the **MD5Model::PrepareMesh** method that take the additional buffers I mentioned earlier (the bone index buffer and the bone weight buffer).

The bone index and bone weight information is extracted from the weight information that is defined for each vertex.

```cpp showLineNumbers startLineNumber=340
;"  title="MD5Model.cpp"]bool MD5Model::PrepareMesh( Mesh& mesh )
{
    mesh.m_PositionBuffer.clear();
    mesh.m_Tex2DBuffer.clear();
    mesh.m_BoneIndex.clear();
    mesh.m_BoneWeights.clear();

    // Compute vertex positions
    for ( unsigned int i = 0; i < mesh.m_Verts.size(); ++i )
    {
        glm::vec3 finalPos(0);
        Vertex& vert = mesh.m_Verts[i];

        vert.m_Pos = glm::vec3(0);
        vert.m_Normal = glm::vec3(0);
        vert.m_BoneWeights = glm::vec4(0);
        vert.m_BoneIndices = glm::vec4(0);

        // Sum the position of the weights
        for ( int j = 0; j < vert.m_WeightCount; ++j )
        {
            assert( j < 4 );

            Weight& weight = mesh.m_Weights[vert.m_StartWeight + j];
            Joint& joint = m_Joints[weight.m_JointID];

            // Convert the weight position from Joint local space to object space
            glm::vec3 rotPos = joint.m_Orient * weight.m_Pos;

            vert.m_Pos += ( joint.m_Pos + rotPos ) * weight.m_Bias;
            vert.m_BoneIndices[j] = (float)weight.m_JointID;
            vert.m_BoneWeights[j] = weight.m_Bias;
        }

        mesh.m_PositionBuffer.push_back(vert.m_Pos);
        mesh.m_Tex2DBuffer.push_back(vert.m_Tex0);
        mesh.m_BoneIndex.push_back(vert.m_BoneIndices);
        mesh.m_BoneWeights.push_back(vert.m_BoneWeights);
    }

    return true;
}
```

As a precaution, I’ve added the assert on line 361 to make sure that no vertex has been weighted to more than four bones.

### The MD5Model::CreateVertexBuffers Method

Since we will only be manipulating the mesh vertices on the GPU, we can upload the vertex data to vertex buffer objects (VBOs). If you’ve followed my article on terrains *Multi-textured Terrain in OpenGL* then you should be familiar with using vertex buffers.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=437
bool MD5Model::CreateVertexBuffers( Mesh& mesh )
{
    CreateVertexBuffer( mesh.m_GLPositionBuffer );
    CreateVertexBuffer( mesh.m_GLNormalBuffer );
    CreateVertexBuffer( mesh.m_GLTexCoord );
    CreateVertexBuffer( mesh.m_GLBoneWeights );
    CreateVertexBuffer( mesh.m_GLBoneIndex );
    CreateVertexBuffer( mesh.m_GLIndexBuffer );

    // Populate the VBO's
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLPositionBuffer );
    glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(glm::vec3) * mesh.m_PositionBuffer.size(), &(mesh.m_PositionBuffer[0]), GL_STATIC_DRAW_ARB );

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLNormalBuffer );
    glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(glm::vec3) * mesh.m_NormalBuffer.size(), &(mesh.m_NormalBuffer[0]), GL_STATIC_DRAW_ARB );

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLTexCoord );
    glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(glm::vec2) * mesh.m_Tex2DBuffer.size(), &(mesh.m_Tex2DBuffer[0]), GL_STATIC_DRAW_ARB );

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLBoneWeights );
    glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(glm::vec4) * mesh.m_BoneWeights.size(), &(mesh.m_BoneWeights[0]), GL_STATIC_DRAW_ARB );

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLBoneIndex );
    glBufferDataARB( GL_ARRAY_BUFFER_ARB, sizeof(glm::vec4) * mesh.m_BoneIndex.size(), &(mesh.m_BoneIndex[0]), GL_STATIC_DRAW_ARB );

    glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, mesh.m_GLIndexBuffer );
    glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(GLuint) * mesh.m_IndexBuffer.size(), &(mesh.m_IndexBuffer[0]), GL_STATIC_DRAW_ARB ); 

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );

    CheckError();

    return true;
}
```

### The MD5Model::Update Method

The **MD5Model::Update** method also needs to be modified to account for the animated joints from the animation class are now stored as matrices so they can be easily sent to the GPU shader program.

The important thing to note is that before the animated joints can be applied to the vertices of the mesh, they need to un-transform the bind-pose positions and rotations of the mesh to get the vertices into the correct space. This is done by multiplying the animated joints by the inverse of the bind pose matrix.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=473
void MD5Model::Update( float fDeltaTime )
{
    if ( m_bHasAnimation )
    {
        m_Animation.Update(fDeltaTime);

        const MatrixList& animatedSkeleton = m_Animation.GetSkeletonMatrixList();
        // Multiply the animated skeleton joints by the inverse of the bind pose.
        for ( int i = 0; i < m_iNumJoints; ++i )
        {
            m_AnimatedBones[i] = animatedSkeleton[i] * m_InverseBindPose[i];
        }        
    }
    else
    {
        // No animation.. Just use identity matrix for each bone.
        m_AnimatedBones.assign(m_iNumJoints, glm::mat4x4(1.0) );
    }

    for ( unsigned int i = 0; i < m_Meshes.size(); ++i )
    {
        // NOTE: This only needs to be done for CPU skinning, but if I want to render the
        // animated normals, I still have to update the mesh on the CPU.
        PrepareMesh( m_Meshes[i], m_AnimatedBones );
    }
}
```

On line 479, the animated joints are retrieved from the animation class and then we loop through all the joints and multiply them by the inverse bind-pose matrix of that joint.

The **MD5Model::PrepareMesh** method will transform the vertex positions and normals on the CPU using the animated skeleton. This is not actually necessary to do when we are doing the vertex skinning on the GPU but if I want to render the transformed normals of the model (for debugging) then I still need to do this step on the CPU (since I can’t read-back the transformed normals from the vertex program on the GPU).

### The MD5Model::RenderMesh Method

The **MD5Model::RenderMesh** method will render the model’s sub-mesh using the OpenGL API. Since we now support GPU skinning, we will render the mesh using either the **MD5Model::RenderCPU** or the **MD5Model::RenderGPU** method.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=631
void MD5Model::RenderMesh( const Mesh& mesh )
{
    switch ( m_VertexSkinning )
    {
    case VS_CPU:
        {
            RenderCPU( mesh );
        }
        break;
    case VS_GPU:
        {
            RenderGPU( mesh );
        }
        break;
    }
}
```

If we are doing vertex skinning on the CPU, we’ll use the **RenderCPU** method and if we are doing the vertex skinning on the GPU, we’ll use the **RenderGPU** method.

The **MD5Model::RenderCPU** method is pretty much identical to the **MD5Model::RenderMesh** method from the [\[Loading and Animating MD5 Models with OpenGL\]](/loading-and-animating-md5-models-with-opengl/) article. I’ve added support for materials in this version, but that’s about it.

Let’s take a look at the **RenderGPU** method.

### The MD5Model::RenderGPU Method

For the **MD5Model::RenderGPU** method we will use the effect shader framework that is introduced in the *Introduction to Cg Runtime with OpenGL*. We will also use the vertex buffers that were initialized previously.

The first thing we will do is to setup the effect parameters that are used for the shader.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=553
void MD5Model::RenderGPU( const Mesh& mesh )
{
    EffectManager& mgr = EffectManager::Get();
    mgr.SetWorldMatrix( m_LocalToWorldMatrix );
    mgr.SetMaterial( mesh.m_Material );

    EffectParameter& baseSampler = m_Effect.GetParameterByName("baseSampler");
    baseSampler.Set( mesh.m_TexID );

    EffectParameter& boneMatrix = m_Effect.GetParameterByName("gBoneMatrix");
    boneMatrix.Set( m_AnimatedBones );

    Technique& technique = m_Effect.GetFirstValidTechnique();
    Pass& pass0 = technique.GetPassByName("p0");

    mgr.UpdateSharedParameters();
    m_Effect.UpdateParameters();
```

The **baseSampler** effect parameter takes the texture object ID that defines the texture that is used to map onto the mesh.

The **boneMatrix** parameter accepts the array of matrices that defines the animated joints of the model that we got from the MD5Animation class in the **MD5Model::Update** method.

In order to use the effect to render the model, we need to get a reference to the pass that defines the vertex and fragment programs. The pass is accessible via the technique.

Before we can use the parameters in the shader, they have to be committed to the GPU program. This is done by using the **EffectManager::UpdateSharedParameters** method and the **Effect::UpdateParameters** method.

The **EffectManager::UpdateSharedParameters** method will commit the parameters that are shared by all effects that are loaded by the effect manager class. To find out which shared parameters that are supported by the **EffectManager** please refer to the **EffectManager::CreateSharedParameters** method in the example source code provided at the end of this article.

The **Effect::UpdateParameters** method will updated all parameters that are unique to the shader effect.

Next we want to bind all of the vertex stream data that will be used to render the model.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=571
    // Position data
    glEnableClientState( GL_VERTEX_ARRAY );
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLPositionBuffer );
    glVertexPointer( 3, GL_FLOAT, 0, BUFFER_OFFSET(0) );

    // Normal data
    glEnableClientState( GL_NORMAL_ARRAY );
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLNormalBuffer );
    glNormalPointer( GL_FLOAT, 0, BUFFER_OFFSET(0) );

    // TEX0
    glActiveTextureARB( GL_TEXTURE0_ARB );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, mesh.m_TexID );

    // TEXCOORD0 (Base texture coordinates)
    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLTexCoord );
    glTexCoordPointer( 2, GL_FLOAT, 0, BUFFER_OFFSET(0) );

    // TEXCOORD1 (Blend weights)
    glClientActiveTextureARB( GL_TEXTURE1_ARB );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLBoneWeights );
    glTexCoordPointer( 4, GL_FLOAT, 0, BUFFER_OFFSET(0) );

    // TEXCOORD2 (Bone indices)
    glClientActiveTextureARB( GL_TEXTURE2_ARB );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glBindBufferARB( GL_ARRAY_BUFFER_ARB, mesh.m_GLBoneIndex );
    glTexCoordPointer( 4, GL_FLOAT, 0, BUFFER_OFFSET(0) );
```

And draw the mesh geometry using the shader effect.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=604
    pass0.BeginPass();

    // Draw mesh from index buffer
    glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, mesh.m_GLIndexBuffer );
    glDrawElements( GL_TRIANGLES, mesh.m_IndexBuffer.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0) );

    pass0.EndPass();
```

The **Pass::BeginPass** method will bind the vertex and fragment shader programs to the rendering pipeline and it will also make sure the texture unit is correctly bound to the correct texture stage.

The **Pass::EndPass** method will disconnect the vertex and fragment programs from the rendering pipeline so we can once again render geometry using the fixed-function pipeline in OpenGL.

And we also have to make sure we restore other OpenGL states and disconnect the textures and vertex buffer objects.

```cpp title="MD5Model.cpp" showLineNumbers startLineNumber=612
    glClientActiveTextureARB( GL_TEXTURE2_ARB );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glClientActiveTextureARB( GL_TEXTURE1_ARB );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glClientActiveTextureARB( GL_TEXTURE0_ARB );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glActiveTextureARB( GL_TEXTURE0_ARB );
    glDisable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, 0 );

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

    glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
    glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
}
```

In additions to the changes to the model class, there are a few changes made to the animation class.

## The MD5Animation Class

The MD5Animation class is almost identical to the original implementation as described in the previous article [\[Loading and Animating MD5 Models with OpenGL\]](/loading-and-animating-md5-models-with-opengl/). The only difference I made is I added an additional parameter to store the animated skeleton joint as a matrix. When the animation frames are interpolated, I compute a resulting matrix for each joint of the animated skeleton.

### The Header File

The only addition I made to the **MD5Animation.h** file is the additional matrix list that stores a 4×4 transformation matrix for each joint of the animated skeleton.

```cpp showLineNumbers startLineNumber=65
"  title="MD5Animation.h"]    typedef std::vector<SkeletonJoint> SkeletonJointList;
    typedef std::vector<glm::mat4x4> SkeletonMatrixList;

    // A frame skeleton stores the joints of the skeleton for a single frame.
    struct FrameSkeleton
    {
        SkeletonMatrixList  m_BoneMatrices;
        SkeletonJointList   m_Joints;
    };
    typedef std::vector<FrameSkeleton> FrameSkeletonList;
```

I’ve highlighted the additional lines in the code sample shown.

I also added an access method for the matrices of the animated skeleton.

```cpp title="MD5Animation.h" showLineNumbers startLineNumber=81
    const SkeletonMatrixList& GetSkeletonMatrixList() const
    {
        return m_AnimatedSkeleton.m_BoneMatrices;
    }
```

The **MD5Animation::GetSkeletonMatrixList** method is used in the **MD5Model::Update** method shown earlier to get the animated skeleton joints. These matrices are transformed by the inverse of the bind pose to get them in the final space to be applied to the vertices of the mesh for rendering.

### The Class File

When the skeletons of the animation are interpolated to compute the final transformation for each joint, I also store the final matrix transformation so the resulting joint matrices can be applied to the vertex shader.

```cpp showLineNumbers startLineNumber=261
"  title="MD5Animation.cpp"]void MD5Animation::InterpolateSkeletons( FrameSkeleton& finalSkeleton, const FrameSkeleton& skeleton0, const FrameSkeleton& skeleton1, float fInterpolate )
{
    for ( int i = 0; i < m_iNumJoints; ++i )
    {
        SkeletonJoint& finalJoint = finalSkeleton.m_Joints[i];
        glm::mat4x4& finalMatrix = finalSkeleton.m_BoneMatrices[i];

        const SkeletonJoint& joint0 = skeleton0.m_Joints[i];
        const SkeletonJoint& joint1 = skeleton1.m_Joints[i];

        finalJoint.m_Parent = joint0.m_Parent;

        finalJoint.m_Pos = glm::lerp( joint0.m_Pos, joint1.m_Pos, fInterpolate );
        finalJoint.m_Orient = glm::mix( joint0.m_Orient, joint1.m_Orient, fInterpolate );

        // Build the bone matrix for GPU skinning.
        finalMatrix = glm::translate( finalJoint.m_Pos ) * glm::toMat4( finalJoint.m_Orient );
    }
}
```

And that is the only changes I made to the MD5Animation class file from the previous implementation. The only thing left to show is the effect that is used to render the model.

## The Shader Effect File

The shader effect used for this demo combines both the vertex program which will apply the bone matrices to compute the final vertex position and normal of the animated skeleton, and a fragment program which will light the model using a single point light.

I’ve already shown you how to create the streams buffers for the bone index and bone weights for each vertex in the **MD5Model::PrepareMesh** method. And I’ve shown how you can connect the shader parameters and bind the streams using vertex buffer objects in the **MD5Model::RenderGPU** method. In this section I will only show the implementation of the shader program.

### Global Variable Definition

The first thing we do in the shader program is define the global variables and structures that are used in the shader program.

```hlsl title="C6E5_skin4m.cgfx" showLineNumbers
#ifndef MAX_BONES
#define MAX_BONES 58
#endif 

// From page 128
struct Material {
    float4 Ke       : EMISSIVE;
    float4 Ka       : AMBIENT;
    float4 Kd       : DIFFUSE;
    float4 Ks       : SPECULAR;
    float shininess : SPECULARPOWER;
};

// From page 138
struct Light {
    float3 position;
    float4 color;
    float kC;
    float kL;
    float kQ;
    float3 direction;
    float cosInnerCone;
    float cosOuterCone;
};

texture baseTexture;
sampler2D baseSampler = sampler_state
{
    Texture = <baseTexture>
    MinFilter = Linear;
    MagFilter = Linear;
};

Material gMaterial;
Light    gLight;

float4x4 gModelViewProj : WORLDVIEWPROJECTION;
float4   gGlobalAmbient : GLOBALAMBIENT;
float3   gEyePosition;
float4x4 gBoneMatrix[MAX_BONES];
```

First we define a constant to indicate the maximum number of bones that our animated skeleton can have. For this demo, the model we are using only contains 33 joints so this is fine. If your models contain more than 58 joints, then you will need to increase the **MAX\_BONES** limit but you should be aware that each profile has a limit to the maximum number of GPU storage locations for variables.

These global variables are the ones that are being set in the MD5Model::RenderGPU method shown earlier.

At the time of this writing, an explanation of the different profiles could be found on the NVidia website [http://http.developer.nvidia.com/Cg/index\_profiles.html](https://web.archive.org/web/20170120121137/http://http.developer.nvidia.com/Cg/index_profiles.html).

### The Vertex Program

The vertex program will take the incoming vertex positions and vertex normals in object space and transform it into the animated positions and normals.

We also need to compute the clip-space position of the vertex and pass it as an-out parameter from the function.

```hlsl title="C6E5_skin4m.cgfx" showLineNumbers startLineNumber=43
void C6E5v_skin4m( float3   position    : POSITION,
                   float3   normal      : NORMAL,
                   float2   texCoord    : TEXCOORD0,
                   float4   weight      : TEXCOORD1,
                   float4   matrixIndex : TEXCOORD2,

               out float4   oPosition   : POSITION,
               out float4   objPos      : TEXCOORD0,
               out float4   objNormal   : TEXCOORD1,
               out float2   oTexCoord   : TEXCOORD2,

           uniform float4x4 boneMatrix[MAX_BONES],
           uniform float4x4 modelViewProj        )
{
    float4x4 matTransform = boneMatrix[matrixIndex.x] * weight.x;
    matTransform += boneMatrix[matrixIndex.y] * weight.y;
    matTransform += boneMatrix[matrixIndex.z] * weight.z;
    float finalWeight = 1.0f - ( weight.x + weight.y + weight.z );
    matTransform += boneMatrix[matrixIndex.w] * finalWeight;

    objPos = mul( matTransform, float4( position, 1.0 ) );
    objNormal = mul( matTransform, float4( normal, 0.0 ) );

    oTexCoord = texCoord;
    oPosition = mul( modelViewProj, objPos );
}
```

The incoming vertex position is bound to the **POSITION** semantic and the OpenGL API uses the **glVertexPointer** method to define how that data is passed to the GPU.

The vertex normal is bound to the **NORMAL** semantic and the incoming data is passed using the **glNormalPointer** method in OpenGL.

The three **TEXCOORDn** semantics are bound to the input streams defined in the application using the **glTexCoordPointer** pointer. The correct semantic is determined by the current active texture stage defined by the **glClientActiveTexture** method in OpenGL.

The two **uniform** parameters are passed as arguments to the vertex program when the program is compiled (this is done in the definition for the pass shown later).

On lines 57-61 the summed matrix transform for the vertex is computed by multiplying the animated bone matrices by the bone weight. The **finalWeight** is computed manually to ensure that the sum of the weights adds to one.

On line 63 and 64 the animated position and normal of the vertex is computed by multiplying the incoming vertex and normal by the summed matrix.

On line 66, the texture coordinate is simply passed-through to the fragment program.

And finally, on line 67 the clip-space position of the vertex is computed from the **WORLDVIEWPROJECTION** matrix and the object space vertex position.

### The Fragment Program

The fragment program accepts the output parameters from the vertex program and outputs a single color value that is bound to the **COLOR** semantic.

The fragment program shown here is identical to the fragment program for the Blinn-Phong lighting model that was shown in the article titled *Transformation and Lighting in Cg*. The only addition here is the texture sampler that is used to define the base color of the fragment.

```cpp showLineNumbers startLineNumber=70
"  title="C6E5_skin4m.cgfx"]void C5E3f_basicLight(float4    position  : TEXCOORD0,                        
                      float4    normal    : TEXCOORD1,
                      float2    texCoord  : TEXCOORD2,

                  out float4    color     : COLOR,

              uniform sampler2D diffuseSampler : TEX0,
              uniform float4    globalAmbient,
              uniform Light     light,
              uniform Material  material,
              uniform float3    eyePosition )
{
    float3 P = position.xyz;
    float3 N = normalize(normal.xyz);

    // Compute emissive term
    float4 emissive = material.Ke;

    // Compute ambient term
    float4 ambient = material.Ka * globalAmbient;

    // Compute the diffuse term
    float3 L = normalize(light.position - P);
    float diffuseLight = max(dot(L, N), 0);
    float4 diffuse = material.Kd * light.color * diffuseLight;

    // Compute the specular term
    float3 V = normalize(eyePosition - P);
    float3 H = normalize(L + V);
    float specularLight = pow(max(dot(H, N), 0), material.shininess);
    if (diffuseLight <= 0) specularLight = 0;
    float4 specular = material.Ks * light.color * specularLight;

    float4 baseColor = tex2D( diffuseSampler, texCoord );

    color = baseColor * ( emissive + ambient + diffuse + specular );
    color.w = 1;
}
```

For a throughout explanation of the lighting model shown here, take a look at my previous article *Transformation and Lighting in Cg*. The only additional part here are the highlighted lines where the texture sampler is used to determine the base color of the fragment.

### Technique and Passes

We only define a single technique for this effect and only a single pass for that technique.

```hlsl title="C6E5_skin4m.cgfx" showLineNumbers startLineNumber=109
technique main
{
    pass p0
    {
        VertexProgram = compile latest C6E5v_skin4m( gBoneMatrix, gModelViewProj );
        FragmentProgram = compile latest C5E3f_basicLight( baseSampler, gGlobalAmbient, gLight, gMaterial, gEyePosition );
    }
}
```

The special profile **latest** is used to indicate that this pass should use the latest vertex and fragment profiles that are supported on the current platform.

If everything goes right, then the final result should be something similar to what is shown below.

<iframe class="embed embed-video" src="https://www.youtube-nocookie.com/embed/Uj0ZR5bkGjc?rel=0" title="YouTube video" loading="lazy" allowfullscreen></iframe>

## References

In addition to the references that were credited in the original article titled [\[Loading and Animating MD5 Models with OpenGL\]](/loading-and-animating-md5-models-with-opengl/) I also used the following books as a reference.

<div class="book-ref">

<img src="/images/The-Cg-Tutorial-Front-Cover.jpg" alt="The Cg Tutorial" width="100" class="thumb" />

[The Cg Tutorial: The Definitive Guide to Programmable Real-Time Graphics (2003). Randima Fernando and Mark J. Kilgard. Addison Wesley.](https://developer.nvidia.com/w/CgTutorial/cg_tutorial_chapter01.html)

</div>

## Download the Source

:::note
The original download (GPUAnimation.zip) is no longer available.
:::
