---
title: "Volume Tiled Forward Shading"
description: "Volume Tiled Forward Shading: extending tiled forward rendering to 3D volume tiles with a light BVH to support millions of light sources in real time (thesis summary, video and PDF)."
slug: volume-tiled-forward-shading
lastUpdated: 2022-05-07
---

<span id="attachment-7402"></span><img src="/images/Volume_Tiled_Forward_Shading-Thumbnail.png" alt="Volume Tiled Forward Shading" width="150" class="thumb" />

In this post, Volume Tiled Forward Shading rendering is described. Volume Tiled Forward Shading is based on Tiled and Clustered Forward Shading described by Ola Olsson et. al. \[[13](#cite-13)\]\[[20](#cite-20)\]. Similar to Clustered Shading, Volume Tiled Forward Shading builds a 3D grid of volume tiles (clusters) and assigns the lights in the scene to the volumes tiles. Only the lights that are intersecting with the volume tile for the current pixel need to be considered during shading. By sorting the lights into volume tiles, the performance of the shading stage can be greatly improved. By building a Bounding Volume Hierarchy (BVH) over the lights in the scene, the performance of the light assignment to tiles phase can also be improved. The Volume Tiled Forward Shading technique combined with the BVH optimization allows for millions of light sources to be active in the scene.

## Thesis

To fulfill the requirements for my Masters in Game and Media Technology (GMT) for the University of Utrecht in the Netherlands, I wrote a thesis on a rendering technique called Volume Tiled Forward Shading. The technique is based on Clustered Forward Shading described by Ola Olsson et. al \[[20](#cite-20)\].

In 2015, I also researched Tiled Forward Shading \[[13](#cite-13)\] and documented my findings [here](/forward-plus/). In that post, I described several areas for improving the Tiled Forward Shading technique. One of the improvement points is to reduce the number false positives (lights that do not contribute to the final shading being added to a tile) that occur within tiles that contain a large depth disparity. Another improvement point was the optimization of the light assignment to tile phase.

The issue caused by the large depth disparity within the tile can be resolved by splitting the tile along the depth in view space. This can be achieved by constructing a 3D grid of volume tiles and assigning the lights in the scene to the volume tiles.

The light assignment to tiles phase can be optimized by first building a Bounding Volume Hierarchy (BVH) over the lights in the scene. During the light assignment phase, the BVH is traversed and only the nodes of the BVH that intersect with the volume tile need to be considered.

Using a grid of volume tiles and constructing a BVH over the lights in the scene, the Volume Tiled Forward Shading with BVH optimization is able to handle millions of light sources in the scene while still maintaining real-time (greater than 30 FPS) frame rates.

[<img src="/images/pdf-library-rc.png" alt="Volume Tiled Forward Shading" width="45" class="thumb" />Volume Tiled Forward Shading.pdf](/downloads/volume-tiled-forward-shading.pdf)

## Presentation

To provide a brief overview of Volume Tiled Forward Shading, I’ve created a PowerPoint Presentation.

[<img src="/images/PowerPoint.png" alt="Volume Tiled Forward Shading" width="45" class="thumb" />Volume Tiled Forward Shading.pptx](https://www.3dgep.com/wp-content/uploads/2017/07/Volume-Tiled-Forward-Shading.pptx)

## Video

To demonstrate the Volume Tiled Forward Shading technique, I’ve created a short video. In the video, Forward Rendering, Tiled Forward Shading \[[13](#cite-13)\], and a variation of Clustered Forward Shading \[[20](#cite-20)\] are compared. By constructing a Bounding Volume Hierarchy over the lights in the scene, we are able to show millions of active light sources are supported while still maintaining real-time frame rates (30 FPS).

<iframe class="embed embed-video" src="https://www.youtube-nocookie.com/embed/nyItqF3sM84?rel=0" title="YouTube video" loading="lazy" allowfullscreen></iframe>

## Demo

An application that demonstrates Volume Tiled Forward Shading was created using the DirectX 12 graphics API. The download for the demo contains all of the source code, project files, and a solution file for Visual Studio 2017. Refer to the **README.txt** file in the root folder for compilation and usage instructions. Pre-built binary executable is also provided in the **Game/bin** folder. Configuration files for loading different models is found in the **Game/Conf** folder. Rung the **Game/Conf/RegisterFileType\_Win10\_Rel\_x64.bat** batch file to automatically register a file handler for the **.3dgep** extension. Use the **UnregisterFileType\_Win10\_Rel\_x64.bat** batch script to remove all the registry entries created by the register script.

The demo has been compressed using [7-zip](https://www.7-zip.org/) because 7-Zip provides the best compression ratio when compared to WinZip or the Zip compressor built-in to Windows 10. Since the demo was created using DirectX 12, a Windows 10 64-bit computer is required to run the demo. The demo was created using the Windows 10 SDK (10.0.14393.0). If you have a newer version of the Windows 10 SDK installed on your computer then you may need to update the SDK version to the version you have. To do this, open the Visual Studio 2017 solution file in the **vs\_2017** folder. After the solution opens, right-click on the solution file and select **Retarget Solution** from the pop-up menu that appears. Select the SDK version that you currently have installed from the drop-down menu in the **Retarget Projects** dialog box that appears. After you retarget all of the projects in the solution to match the Windows SDK that you have installed, you should be able to compile the solution without errors. Please leave a comment in this post if you still encounter any problems with downloading, compiling, or running the demo.

The file is over 2 GB because it contains all of the source assets that can be used to test the application. All source assets were retrieved from Morgan McGuire’s Computer Graphics Archive [https://casual-effects.com/data/](https://casual-effects.com/data/).

[<img src="/images/7zip1600.png" alt="Volume Tiled Forward Shading.7z" width="45" class="thumb" />Volume Tiled Forward Shading.7z](https://drive.usercontent.google.com/download?id=1oYDnHaOER3LKqtHPD_SpeXsUz5CUWpda&export=download)

## References

<span id="cite-1"></span>\[1\] G. Singer, “The History of the Modern Graphics Processor”, TechSpot, 2013. \[Online\]. Available: [https://www.techspot.com/article/650-history-of-the-gpu/](https://www.techspot.com/article/650-history-of-the-gpu/). \[Accessed: 02- Sep- 2016\].

<span id="cite-2"></span>\[2\] M. Segal and K. Akeley, The OpenGL Graphics System: A Specification, 1st ed. Silicon Graphics, Inc., 1994.

<span id="cite-3"></span>\[3\] J. van Oosten, “Introduction to DirectX 11”, 3D Game Engine Programming, 2014. \[Online\]. Available: [https://www.3dgep.com/introduction-to-directx-11](/introduction-to-directx-11/). \[Accessed: 21- Sep- 2016\].

<span id="cite-4"></span>\[4\] K. Akeley, A. Akin, B. Ashbaugh, B. Beretta, J. Carmack, M. Craighead, K. Dyke, S. Glanville, M. Gold, E. Hart, M. Kilgard, B. Licea-Kane, B. Lichtenbelt, E. Lindholm, B. Lipchak, B. Mark, J. McCombe, J. Morris, B. Paul, B. Poddar, T. Roell, J. Sandmel, J. Schelter, G. Stahl, J. Stauffer and N. Triantos, “ARB\_vertex\_program”, Opengl.org, 2007. \[Online\]. Available: [https://registry.khronos.org/OpenGL/extensions/ARB/ARB_vertex_program.txt](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_vertex_program.txt). \[Accessed: 23- Sep- 2016\].

<span id="cite-5"></span>\[5\] B. Beretta, P. Brown, M. Craighead, C. Everitt, E. Hart, J. Leech, B. Licea-Kane, B. Poddar, J. Sandmel, J. Schelter, A. Seetharamaiah and N. Triantos, “ARB\_fragment\_program”, OpenGL.org, 2013. \[Online\]. Available: [https://registry.khronos.org/OpenGL/extensions/ARB/ARB_fragment_program.txt](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_fragment_program.txt). \[Accessed: 23- Sep- 2016\].

<span id="cite-6"></span>\[6\] M. Segal and K. Akeley, The OpenGL Graphics System: A Specification, 2nd ed. Silicon Graphics Inc., 2004.

<span id="cite-7"></span>\[7\] M. Deering, S. Winner, B. Schediwy, C. Duffy and N. Hunt, “The triangle processor and normal vector shader”, ACM SIGGRAPH Computer Graphics, vol. 22, no. 4, pp. 21-30, 1988.

<span id="cite-8"></span>\[8\] R. Geldreich and M. Pritchard, “GDC Vault – Deferred Shading on DX9 Class Hardware and the Xbox”, Gdcvault.com, 2004. \[Online\]. Available: [https://www.gdcvault.com/play/1015172/Deferred-Shading-on-DX9-Class](https://www.gdcvault.com/play/1015172/Deferred-Shading-on-DX9-Class). \[Accessed: 27- Sep- 2016\].

<span id="cite-9"></span>\[9\] O. Shishkovtsov, “Deferred Shading in S.T.A.L.K.E.R.”, in GPU Gems 2: Programming Techniques For High-Performance Graphics And General-Purpose Computation, 3rd ed., M. Pharr and R. Fernando, Ed. Pearson Addison Wesley Prof, 2006.

<span id="cite-10"></span>\[10\] M. van der Leeuw, “Deferred Rendering in Killzone 2”, Palo Alto, California, 2007.

<span id="cite-11"></span>\[11\] M. Mittring, “A bit more deferred – CryEngine 3”, Raleigh, North Carolina, 2009.

<span id="cite-12"></span>\[12\] T. Saito and T. Takahashi, “Comprehensible rendering of 3-D shapes”, ACM SIGGRAPH Computer Graphics, vol. 24, no. 4, pp. 197-206, 1990.

<span id="cite-13"></span>\[13\] O. Olsson and U. Assarsson, “Tiled Shading”, Journal of Graphics, GPU, and Game Tools, vol. 15, no. 4, pp. 235-251, 2011.

<span id="cite-14"></span>\[14\] T. Harada, J. McKee and J. Yang, “Forward+: Bringing Deferred Lighting to the Next Level”, 2012.

<span id="cite-15"></span>\[15\] J. McKee, “Technology Behind AMD’s “Leo Demo””, San Francisco, California, 2012.

<span id="cite-16"></span>\[16\] T. Harada, “A 2.5D culling for Forward+”, SIGGRAPH Asia 2012 Technical Briefs on – SA ’12, 2012.

<span id="cite-17"></span>\[17\] J. van Oosten, “Forward vs Deferred vs Forward+ Rendering with DirectX 11”, 3D Game Engine Programming, 2015. \[Online\]. Available: [http://www.3dgep.com/forward-plus](/forward-plus/). \[Accessed: 29- Sep- 2016\].

<span id="cite-18"></span>\[18\] C. Balestra and P. Engstad, “The technology of uncharted: Drake’s fortune”, Game Developer Conference, 2008.

<span id="cite-19"></span>\[19\] J. Andersson, “Parallel Graphics in Frostbite – Current & Future”, Siggraph, 2009.

<span id="cite-20"></span>\[20\] O. Olsson, M. Billeter and U. Assarsson, “Clustered Deferred and Forward Shading”, in Eurographics/ ACM SIGGRAPH Symposium on High Performance Graphics, Eurographics, 2012.

<span id="cite-21"></span>\[21\] C. Ericson, Real-time collision detection. Amsterdam: Elsevier, 2005.

<span id="cite-22"></span>\[22\] “Downloads”, Crytek.com, 2017. \[Online\]. Available: [https://web.archive.org/web/20151231130156/http://www.crytek.com/cryengine/cryengine3/downloads](https://web.archive.org/web/20151231130156/http://www.crytek.com/cryengine/cryengine3/downloads). \[Accessed: 04- Jan- 2017\].

<span id="cite-23"></span>\[23\] N. Wilt, The CUDA Handbook: A Comprehensive Guide to GPU Programming, 1st ed. Addison-Wesley, 2013, pp. 365-383.

<span id="cite-24"></span>\[24\] T. Karras, “Thinking Parallel, Part II: Tree Traversal on the GPU”, Parallel Forall, 2012. \[Online\]. Available: [https://developer.nvidia.com/blog/thinking-parallel-part-ii-tree-traversal-gpu/](https://developer.nvidia.com/blog/thinking-parallel-part-ii-tree-traversal-gpu/). \[Accessed: 05- Jan- 2017\].

<span id="cite-25"></span>\[25\] NVIDIA GeForce GTX 1080 Whitepaper, 1st ed. NVIDIA Corporation, 2016.

<span id="cite-26"></span>\[26\] J. van Oosten, “Optimizing CUDA Applications – 3D Game Engine Programming”, 3D Game Engine Programming, 2011. \[Online\]. Available: [https://www.3dgep.com/optimizing-cuda-applications/](https://www.3dgep.com/optimizing-cuda-applications/). \[Accessed: 06- Jan- 2017\].

<span id="cite-27"></span>\[27\] CUDA C Best Practices Guide, 1st ed. NVIDIA Corporation, 2016.

<span id="cite-28"></span>\[28\] “Programming Guide :: CUDA Toolkit Documentation”, Docs.nvidia.com, 2016. \[Online\]. Available: [https://docs.nvidia.com/cuda/cuda-programming-guide/index.html](https://docs.nvidia.com/cuda/cuda-programming-guide/index.html). \[Accessed: 13- Jan- 2017\].

<span id="cite-29"></span>\[29\] E. Young, “DirectCompute Optimizations and Best Practices”, San Jose, California, 2010.

<span id="cite-30"></span>\[30\] G. Morton, A computer oriented geodetic data base and a new technique in file sequencing, 1st ed. Ottawa: International Business Machines Co., 1966.

<span id="cite-31"></span>\[31\] R. Dickau, Lebesgue 3D curve, iteration 2. 2008.

<span id="cite-32"></span>\[32\] N. Satish, M. Harris and M. Garland, “Designing efficient sorting algorithms for manycore GPUs”, 2009 IEEE International Symposium on Parallel & Distributed Processing, 2009.

<span id="cite-33"></span>\[33\] O. Green, R. McColl and D. Bader, “GPU merge path”, Proceedings of the 26th ACM international conference on Supercomputing – ICS ’12, 2012.

<span id="cite-34"></span>\[34\] M. Harris, S. Sengupta and J. Owens, “Parallel Prefix Sum (Scan) with CUDA”, in GPU Gems 3, 1st ed., H. Nguyen, Ed. Addison-Wesley, 2008, pp. 871-873.

<span id="cite-35"></span>\[35\] G. Blelloch, “Scans as primitive parallel operations”, IEEE Transactions on Computers, vol. 38, no. 11, pp. 1526-1538, 1989.

<span id="cite-36"></span>\[36\] W. Hillis and G. Steele, “Data parallel algorithms”, Communications of the ACM, vol. 29, no. 12, pp. 1170-1183, 1986.

<span id="cite-37"></span>\[37\] O. Olsson, “Introduction to Real-Time Shading with Many Lights”, 2015.

<span id="cite-38"></span>\[38\] J. Foley, A. van Dam, S. Feiner and J. Hughes, Computer Graphics: Principles and Practice, 2nd ed. Boston: Addison-Wesley, 1996.

<span id="cite-39"></span>\[39\] H. Zhang, D. Manocha, T. Hudson and K. Hoff, “Visibility culling using hierarchical occlusion maps”, Proceedings of the 24th annual conference on Computer graphics and interactive techniques – SIGGRAPH ’97, 1997.

<span id="cite-40"></span>\[40\] J. Clark, “Hierarchical geometric models for visible surface algorithms”, Communications of the ACM, vol. 19, no. 10, pp. 547-554, 1976.

<span id="cite-41"></span>\[41\] E. Catmull, “A Subdivision Algorithm for Computer Display of Curved Surfaces”, Ph.D, University of Utah, 1974.

<span id="cite-42"></span>\[42\] S. Hargreaves and M. Harris, “Deferred Shading”, 2004.

<span id="cite-43"></span>\[43\] L. Howes, “Making GPGPU Easier – Software and Hardware Improvements in GPU Computing”, University of Texas, Austin, Texas, 2012.

<span id="cite-44"></span>\[44\] AMD Graphics Cores Next (GCN) Architecture, 1st ed. Advanced Micro Devices Inc., 2012.

<span id="cite-45"></span>\[45\] M. McGuire, “Meshes”, Graphics.cs.williams.edu, 2011. \[Online\]. Available: [http://casual-effects.com/data/index.html](http://casual-effects.com/data/index.html). \[Accessed: 02- Jun- 2017\].

<span id="cite-46"></span>\[46\] T. Lottes, FXAA. Santa Clara, California, USA: NVIDIA Corporation, 2009.

<span id="cite-47"></span>\[47\] “Rasterization Rules (Windows)”, Msdn.microsoft.com, 2017. \[Online\]. Available: [https://learn.microsoft.com/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules](https://learn.microsoft.com/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules). \[Accessed: 10- Jul- 2017\].

<span id="cite-48"></span>\[48\] “SAT (Separating Axis Theorem) – dyn4j”, Dyn4j.org, 2017. \[Online\]. Available: [https://dyn4j.org/2010/01/sat/](https://dyn4j.org/2010/01/sat/). \[Accessed: 10- Jul- 2017\].
