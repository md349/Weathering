
//This class is from BlankNGL. Original Author : Jon Macey
//Modified using lecture notes from GLSL lecture on 20/11/2014.

#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/AbstractMesh.h>
#include <ngl/Util.h>
#include <ngl/Obj.h>

#include <iostream>
#include <vector>

#include <Vertical.h>
#include <Hemisphere.h>
#include <BBox.h>

NGLScene::NGLScene(QWindow *_parent) : OpenGLWindow(_parent)
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle("Particle Based Weathering System - Martin Davies");
  m_projection = ngl::perspective(50.0f,float(64/45),0.5,10);
  m_view = ngl::lookAt(ngl::Vec3(5,6,5), ngl::Vec3(0,0,0), ngl::Vec3(0,1,0));
}

NGLScene::~NGLScene()
{
  ngl::NGLInit *Init = ngl::NGLInit::instance();
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  Init->NGLQuit();
}

void NGLScene::resizeEvent(QResizeEvent *_event )
{
  if(isExposed())
  {
  int w=_event->size().width();
  int h=_event->size().height();
  // set the viewport for openGL
  glViewport(0,0,w,h);
  renderLater();
  }
}

void NGLScene::initialize()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::instance();
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // as re-size is not explicitly called we need to do this.
  glViewport(0,0,width(),height());

  //load obj
  std::cout<<"loading model\n";

  //load obj
  ngl::Obj mesh("models/cube.obj");

  std::cout<<"checking triangular...\n";
  //ngl only works with tri meshes
  if( ! mesh.isTriangular() )
  {
    std::cout<<"only works for tri meshes\n";
    exit(EXIT_FAILURE);
  }

  //get mesh data
  std::cout<<"getting mesh data\n";
  std::vector <ngl::Vec3> m_verts = mesh.getVertexList();
  std::vector <ngl::Face> m_faces = mesh.getFaceList();
  std::vector <ngl::Vec3> m_texs = mesh.getTextureCordList();
  std::vector <ngl::Vec3> m_normals = mesh.getNormalList();
  std::cout<<"mesh data collected\n";

  mesh.calcDimensions();
  m_bbox = mesh.getBBox(); //returns ngl::BBox

  vertData d;
  unsigned int nFaces = m_faces.size();
  unsigned int nNorm = m_normals.size();
  unsigned int nTex = m_texs.size();

  //loop for each face
  for(unsigned int i = 0; i < nFaces; ++i)
  {
    ngl::Vec3 v1;
    ngl::Vec3 v2;
    ngl::Vec3 v3;
    //for each tri
    for(int j = 0; j < 3; ++j)
    {
      //pack in vertex data first
      d.x = m_verts[m_faces[i].m_vert[j]].m_x;
      d.y = m_verts[m_faces[i].m_vert[j]].m_y;
      d.z = m_verts[m_faces[i].m_vert[j]].m_z;

      if(j == 0)
      {
        v1 = ngl::Vec3(d.x, d.y, d.z);
      }
      else if (j == 1)
      {
        v2 = ngl::Vec3(d.x, d.y, d.z);
      }
      else if (j == 3)
      {
        v3 = ngl::Vec3(d.x, d.y, d.z);
      }

      //if we have norms or tex pack them as well
      if(nNorm > 0 && nTex > 0)
      {
        //normals
        d.nx = m_normals[m_faces[i].m_norm[j]].m_x;
        d.ny = m_normals[m_faces[i].m_norm[j]].m_y;
        d.nz = m_normals[m_faces[i].m_norm[j]].m_z;

        //tex
        d.u = m_texs[m_faces[i].m_tex[j]].m_x;
        d.v = m_texs[m_faces[i].m_tex[j]].m_y;
      }
      //if neither are present
      else if(nNorm == 0 && nTex == 0)
      {
        d.nx = 0;
        d.ny = 0;
        d.nz = 0;
        d.u = 0;
        d.v = 0;
      }
      //if we have norm but no tex
      else if(nNorm > 0 && nTex == 0)
      {
        d.nx = m_normals[m_faces[i].m_norm[j]].m_x;
        d.ny = m_normals[m_faces[i].m_norm[j]].m_y;
        d.nz = m_normals[m_faces[i].m_norm[j]].m_z;
        d.u = 0;
        d.v = 0;
      }

    m_tris.push_back(pwl::Triangle(v1,v2,v3));
    m_vboMesh.push_back(d);
    }
  }

  //grab instance of VAO class. As Tri Strip
  m_vaoMesh = ngl::VertexArrayObject::createVOA(GL_TRIANGLES);
  //bind to set data
  m_vaoMesh->bind();
  unsigned int meshSize = m_vboMesh.size();

  //tell the VAO how much data we are copying (in bytes)
  m_vaoMesh->setData(meshSize*sizeof(vertData),m_vboMesh[0].u);

  //set vertexAttributePointer to the right size and type
  m_vaoMesh->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(vertData),5); //position
  m_vaoMesh->setVertexAttributePointer(1,2,GL_FLOAT,sizeof(vertData),0); //uv
  m_vaoMesh->setVertexAttributePointer(2,3,GL_FLOAT,sizeof(vertData),2); //normals

  //tell vao how many indices to draw
  m_vaoMesh->setNumIndices(meshSize);
  //unbind now we have finished
  m_vaoMesh->unbind();

  std::cout<<"Obj VAO has been created\n";

  //use library to do weathering
  std::cout<<"Starting Weathering\n";
  //count iterations
  int iterationCounter = 0;

  //load all of the PWL classes we need
  pwl::Vertical ver; //vertical emitter
  pwl::BBox bb; //bounding box emitter
  pwl::Hemisphere hem; //hemispherical emitter

  while(iterationCounter < 3)
  {
    if(iterationCounter == 0)
    {
      //setup vertical emitter
      ver.create(m_bbox, 1000);

      hem.create(m_bbox, 10000);
      std::cout<<"hemispherical emitter created\n";

      //set up bounding box emitter
      //number of surfels is per bounding box face
      bb.create(m_bbox, 1000);
      std::cout<<"bounding box emitter created\n";

      bb.goToSurface(m_bbox, m_tris);
      std::cout<<"bbox surfels emitted\n";
      m_bboxvec = bb.getBBox();

      //start propagating
      ver.propagate();
      std::cout<<"vertical gtons emitted\n";
      m_vert = ver.getVertical();

      //hemisphere gtons propagate towards centre
      hem.propagate(m_bbox);

      m_hem = hem.getHem();
    }
    else
    {

    }

    //increase counter
    iterationCounter += 1;
  }

  /**
  std::cout<<"Starting Weathering\n";
  pwl::PWL control;
  control.doWeathering(4, "models/cube.obj");
  m_bbox = control.getBBox();
  std::cout<<"got bbox\n";
  m_vert = control.getVertical();\
  std::cout<<"got vert\n";
  m_hem = control.getHem();
  std::cout<<"got hem\n";
  std::cout<<"Weathering Complete\n";**/

  //pointer to shader lib
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  //create shader program and assign name
  shader->createShaderProgram("Colour");
  //create 2 empty shaders
  shader->attachShader("ColourVertex", ngl::VERTEX);
  shader->attachShader("ColourFragment", ngl::FRAGMENT);
  //load source for shaders
  shader->loadShaderSource("ColourVertex", "shaders/ColourVertex.glsl");
  shader->loadShaderSource("ColourFragment", "shaders/ColourFragment.glsl");
  //compile shaders
  shader->compileShader("ColourVertex");
  shader->compileShader("ColourFragment");
  //attach shader to program
  shader->attachShaderToProgram("Colour", "ColourVertex");
  shader->attachShaderToProgram("Colour", "ColourFragment");
  //link shader
  shader->linkProgramObject("Colour");
  shader->use("Colour");

  //get uniform in frag shader
  GLuint id = shader->getProgramID("Colour");
  GLuint uni = glGetUniformLocation(id,"lightPos");

  glUniform3f(uni,2.0f,1.0f,1.0f);
}

void NGLScene::render()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //instance of shader lib
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();

  ngl::VAOPrimitives *prim = ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",0.01,20);

  //translate
  ngl::Mat4 trans;

  shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);

  m_vaoMesh->bind();
  m_vaoMesh->draw();

  for(unsigned int i = 0; i < m_bboxvec.size(); ++i)
  {
    ngl::Vec3 bbPos = m_bboxvec[i].getPos();
    trans.translate(bbPos[0],bbPos[1],bbPos[2]);
    shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);
    prim->draw("sphere");
  }
/**
  for(unsigned int i = 0; i < m_vert.size(); ++i)
  {
    ngl::Vec3 vPos = m_vert[i].getPos();
    trans.translate(vPos[0],vPos[1],vPos[2]);
    shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);
    prim->draw("sphere");
  }

  for(unsigned int i = 0; i < m_hem.size(); ++i)
  {
    ngl::Vec3 vPos = m_hem[i].getPos();
    trans.translate(vPos[0],vPos[1],vPos[2]);
    shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);
    prim->draw("sphere");
  }**/
}

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  default : break;
  }
  // finally update the GLWindow and re-draw
  if (isExposed())
    renderLater();
}
