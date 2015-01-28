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
#include <ngl/Obj.h>


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

    int m_spinXFace;
    int m_spinYFace;


    bool m_rotate;
    bool m_translate;

    int m_origX;
    int m_origY;

    int m_origXPos;
    int m_origYPos;


    ngl::Mat4 m_mouseGlobalTX;

    ngl::Camera *m_cam;

    ngl::Transformation m_transform;

    ngl::Vec3 m_modelPos;

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

    void loadMatricesToShader();

    void loadGTonShader(ngl::Mat4 trans);

    ngl::Mat4 m_projection;
    ngl::Mat4 m_view;

    std::vector <pwl::Surfel *> m_bboxvec;
    std::vector <pwl::GTon> m_vert;
    std::vector <pwl::GTon> m_hem;

    ngl::Vec3 objCentre;

    //vertex array object
    ngl::VertexArrayObject *m_vaoMesh;

    std::vector <pwl::Triangle> m_tris;

    //holds surfels for gammaton map
    std::vector <pwl::Surfel> m_fGTMap;

    //holds normalised positions of surfels for texture
    std::vector<ngl::Vec3> m_textureVec;
};

#endif
