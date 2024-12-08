# GL-Man

**GL-Man** is a project developed as part of the **Advanced Three-Dimensional Graphics course (2023-2024)** at the University of Basilicata, Master's degree in **Computer Engineering and Information Technologies**.

It is an **unofficial 3D implementation** of the classic video game **Pac-Man** (1980), created by Toru Iwatani and produced by Namco, developed in **C++** and based on the **OpenGL** graphics API.

---

### **Integrated 3D Rendering Engine**

During the development of **GL-Man**, a **modular 3D rendering engine** was designed and implemented, providing the graphic foundations for the game. This engine, while lacking a graphic interface, consists of a collection of generic and reusable classes, designed to manage the main aspects of three-dimensional graphics.

#### **Main Features**

- **Modular Structure**
  The classes are organized to provide specific functionalities in a flexible context with a clear separation of responsibilities to ensure code reusability and scalability for future projects. 
  <br />

- **File System Management**
  To simplify access and management of project resources, a dedicated class for interaction with the Windows operating system file system has been developed, namely the `FileSystem` class. 
  This class is designed to abstract common file reading and writing operations, ensuring flexibility and portability.
  <br />

- **Resource Management**
  All resources are loaded, stored, and managed through the `ResourceManager` class, which has the following functionalities:

  - Loading, compilation, and storage system for ***GLSL*** shaders, including vertex, fragment, and (optionally) geometry shaders. With support for all major formats like `.vs`, `.fs`, and `.gs`.
  
  - Texture loading and storage system, based on the ***stb_image*** library with support for various filtering and wrapping modes.
  
  - 3D model and material loading and storage system, based on the ***Assimp*** library, with support for the standard `.obj` format.
  <br />

- **Logging Management**
  All prints are made through the `LoggerManager` class, based on the ***spdlog*** library. The `LoggerManager` class allows: logging messages at different log levels; using dynamically formatted messages; printing using a dedicated thread separate from the rendering thread, so as not to slow down the latter.
  <br />

- **Game Object Management**
  For the rendering engine, every element drawn on screen is an instance of the `GameObjectBase` class. 
  This class, through its subclasses `GameObjectCustom` and `GameObjectFromModel`, allows the correct positioning and drawing of each Game Object. 
  `GameObjectCustom` is useful for defining a Game Object given meshes, diffuseTexture, and specularTexture. While `GameObjectFromModel` is useful for defining a Game Object given a model. 
  Both previous classes allow generating the OBB of the Game Object. An Oriented Bounding Box (OBB) is a compact representation of the bounding volume, which closely adapts to the geometries it represents. The invariance of an OBB to translation and rotation makes it ideal as an optimal and default representation of the bounding volume in a three-dimensional space. It is also fundamental for implementing a precise and optimized Collider.
  <br />

- **Post-Processing Effects Management**
  All post-processing effects are managed by the `PostProcessor` class, which handles post-processing operations in rendering, such as HDR, Bloom, and multisampled anti-aliasing (MSAA). 
  The `PostProcessor` is designed to optimize visual quality, integrating with shaders and improving the realism of scenes.
  <br />

- **Overlay Text Management**
  Overlay text is managed by the `TextRenderer` class, which is responsible for 2D text rendering using fonts loaded via the **FreeType** library. 
  The `TextRenderer` allows adding stylized text to 3D or 2D scenes, such as scores, HUD, or notifications, ensuring flexibility and high visual quality.
  <br />

#### **General Use**

This engine is designed to be independent of the specific game project. While being used as a base for the development of **GL-Man**, it can be easily integrated into other projects that require 3D rendering functionalities.

Thanks to this flexible structure, the engine represents a valid starting point for academic projects, prototypes, or more complex applications in the field of three-dimensional graphics.

#### **Please Note**

The engine, being devoid of a dedicated graphic interface, requires knowledge of **C++** programming and familiarity with **OpenGL** APIs to be used and customized. Further details about its functionalities can be found in the code comments.

---

### **Game**

The game integrates several advanced three-dimensional graphics concepts:

- **Depth Testing**: Enabled to improve visual rendering and optimize performance.
- **Stencil Testing**: Used to visually highlight the power-up model.
- **Face Culling**: Enabled to improve rendering performance by excluding polygons hidden from the view frustum.
- **Alpha Blending**: Applied to ghosts when in vulnerable state.
- **Uniform Buffer Objects**: Used to optimize rendering performance.
- **Interface Block**: Used to optimize information exchange between vertex and fragment shaders.
- **Instancing**: Used to efficiently represent walls, floor, dots, and energizers.
- **Anti-Aliasing(MSAA)**: User can enable or disable MSAA for smoother graphics.
- **Advanced Lighting**: Shaders with Blinn-Phong and gamma correction for realistic rendering.
- **High Dynamic Range(HDR)**: User can enable or disable HDR display for deeper graphics.
- **Bloom**: Used to visually emphasize energizers.

The game also includes a user interface developed with **Dear ImGui**, which allows the user to set parameters such as HDR and anti-aliasing. 
This system allows easy configuration of graphic options to adapt the game to user preferences and system capabilities.

---

### Main Project Dependencies:

- Assimp
- FreeType
- Glad
- GLFW
- GLM
- Dear ImGUI
- IrrKlang
- Spdlog
- STB

---

### **Usage**

In the `bin` folder present in the project root, there is an executable file, resulting from the compilation of the project in Windows 10 environment, Intel x86 CPU.

In case of problems, it is possible to recompile the project simply by opening the `.sln` file with Visual Studio.

---

### **Gallery**

|  |  |
|--|--|
| Home Screen <BR>![home](./res/screenshots/screenshot_home.png) | Settings Screen - Anti-Aliasing <BR>![home](./res/screenshots/screenshot_settings_anti-aliasing.png) |
|Settings Screen - HDR <BR>![home](./res/screenshots/screenshot_settings_hdr.png) | Game Screen <BR>![home](./res/screenshots/screenshot_game.png) |

---

### **Contributions**

Contributions and suggestions are welcome! Open an issue to report problems or send a pull request to propose improvements 😁.

---

### **License**

GL-Man is licensed under the MIT License, see [LICENSE.txt](./LICENSE.txt) for more information.
