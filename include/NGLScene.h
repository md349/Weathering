//This class is extracted from BlankNGL. Original Author: Jon Macey
//It has been modified based on lecture notes from GLSL lecture on 20/11/2014

#ifndef NGLSCENE_H__
#define NGLSCENE_H__
#include "OpenGLWindow.h"
#include <ngl/Camera.h>
#include <ngl/Colour.h>
#include <ngl/Light.h>
#include <ngl/Transformation.h>
#include <ngl/Text.h>
#include <ngl/Mat4.h>
#include <ngl/Vec3.h>
#include <ngl/BBox.h>

#include <GTon.h>
#include <Surfel.h>
#include <Triangle.h>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
/// @file NGLScene.h
/// @brief this class inherits from the Qt OpenGLWindow and allows us to use NGL to draw OpenGL
/// @author Jonathan Macey
/// @version 1.0
/// @date 10/9/13
/// Revision History :
/// This is an initial version used for the new NGL6 / Qt 5 demos
/// @class NGLScene
/// @brief our main glwindow widget for NGL applications all drawing elements are
/// put in this file
//----------------------------------------------------------------------------------------------------------------------

class NGLScene : public OpenGLWindow
{
  public:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief ctor for our NGL drawing class
    /// @param [in] parent the parent window to the class
    //----------------------------------------------------------------------------------------------------------------------
    NGLScene(QWindow *_parent=0);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief dtor must close down ngl and release OpenGL resources
    //----------------------------------------------------------------------------------------------------------------------
    ~NGLScene();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the initialize class is called once when the window is created and we have a valid GL context
    /// use this to setup any default GL stuff
    //----------------------------------------------------------------------------------------------------------------------
    void initialize();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this is called everytime we want to draw the scene
    //----------------------------------------------------------------------------------------------------------------------
    void render();

private:

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Qt Event called when the window is re-sized
    /// @param [in] _event the Qt event to query for size etc
    //----------------------------------------------------------------------------------------------------------------------
    void resizeEvent(QResizeEvent *_event);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Qt Event called when a key is pressed
    /// @param [in] _event the Qt event to query for size etc
    //----------------------------------------------------------------------------------------------------------------------
    void keyPressEvent(QKeyEvent *_event);

    ngl::Mat4 m_projection;
    ngl::Mat4 m_view;

    std::vector <pwl::Surfel> m_bboxvec;
    std::vector <pwl::GTon> m_vert;
    std::vector <pwl::GTon> m_hem;

    struct vertData
    {
      GLfloat u;  //tex coord
      GLfloat v;  //tex coord
      GLfloat nx; //normal
      GLfloat ny;
      GLfloat nz;
      GLfloat x;  //position
      GLfloat y;
      GLfloat z;
    };

    //vector of data for our mesh
    std::vector <vertData> m_vboMesh;

    //vertex array object
    ngl::VertexArrayObject *m_vaoMesh;

    //bounding box to work out emitter sizes.
    ngl::BBox m_bbox;

    std::vector <pwl::Triangle> m_tris;

};

#endif
