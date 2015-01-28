
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
//#include <ngl/Obj.h>
#include <ngl/BBox.h>

#include <iostream>
#include <vector>

#include <Vertical.h>
#include <Hemisphere.h>
#include <BBox.h>
#include <Texture.h>

const static float INCREMENT = 0.01;
const static float ZOOM = 0.1;

NGLScene::NGLScene(QWindow *_parent) : OpenGLWindow(_parent)
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle("Particle Based Weathering System - Martin Davies");

  m_rotate = false;
  //mouse rotation values set to 0
  m_spinXFace = 0;
  m_spinYFace = 0;
  m_projection = ngl::perspective(25,(float)720.0/576.0,0.001,350);
  m_view = ngl::lookAt(ngl::Vec3(16,5,15), ngl::Vec3(0,0,0), ngl::Vec3(0,1,0));
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
  m_cam->setShape(45,(float)w/h,0.05,350);
  renderLater();
  }
}

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

  //basica camera setup
  ngl::Vec3 from(5,3,10);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);

  //create camera
  m_cam = new ngl::Camera(from,to,up);
  //set shape
  m_cam->setShape(25,(float)720.0/576.0,0.001,350);

  //obj model loading from Jon Macey's Normal Mapping NGL Demo.
  //modified to fit with this project.
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

  //vector of data for our mesh
  std::vector <vertData> vboMesh;

  //centre of obj for ray tracing
  objCentre = mesh.getCenter();

  //bounding box to work out emitter sizes.
  ngl::BBox bbox = mesh.getBBox(); //returns ngl::BBox

  vertData d;
  unsigned int nFaces = m_faces.size();
  unsigned int nNorm = m_normals.size();
  unsigned int nTex = m_texs.size();

  //loop for each face
  for(unsigned int i = 0; i < nFaces; ++i)
  {
    //ngl::Vec3 v1;
    //ngl::Vec3 v2;
    //ngl::Vec3 v3;
    //for each tri
    for(int j = 0; j < 3; ++j)
    {
      //pack in vertex data first
      d.x = m_verts[m_faces[i].m_vert[j]].m_x;
      d.y = m_verts[m_faces[i].m_vert[j]].m_y;
      d.z = m_verts[m_faces[i].m_vert[j]].m_z;

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

    vboMesh.push_back(d);
    }
  }

  for(unsigned int i = 0; i < vboMesh.size(); ++i)
  {
    ngl::Vec3 v1 (vboMesh[i].x, vboMesh[i].y, vboMesh[i].z);
    ngl::Vec3 v2 (vboMesh[i+1].x, vboMesh[i+1].y, vboMesh[i+1].z);
    ngl::Vec3 v3 (vboMesh[i+2].x, vboMesh[i+2].y, vboMesh[i+2].z);

    //for last two we need to loop around
    if(i == vboMesh.size()-2)
    {
      v3[0] = vboMesh[0].x;
      v3[1] = vboMesh[0].y;
      v3[2] = vboMesh[0].z;
    }
    if(i == vboMesh.size() -1)
    {
      v2[0] = vboMesh[0].x;
      v2[1] = vboMesh[0].y;
      v2[2] = vboMesh[0].z;
      v3[0] = vboMesh[1].x;
      v3[1] = vboMesh[1].y;
      v3[2] = vboMesh[1].z;
    }

    m_tris.push_back(pwl::Triangle(v1,v2,v3));
  }

  //grab instance of VAO class. As Tri Strip
  m_vaoMesh = ngl::VertexArrayObject::createVOA(GL_TRIANGLES);
  //bind to set data
  m_vaoMesh->bind();
  unsigned int meshSize = vboMesh.size();

  //tell the VAO how much data we are copying (in bytes)
  m_vaoMesh->setData(meshSize*sizeof(vertData),vboMesh[0].u);

  //set vertexAttributePointer to the right size and type
  m_vaoMesh->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(vertData),5); //position
  m_vaoMesh->setVertexAttributePointer(1,2,GL_FLOAT,sizeof(vertData),0); //uv
  m_vaoMesh->setVertexAttributePointer(2,3,GL_FLOAT,sizeof(vertData),2); //normals

  //tell vao how many indices to draw
  m_vaoMesh->setNumIndices(meshSize);
  //unbind now we have finished
  m_vaoMesh->unbind();

  std::cout<<"Obj VAO has been created\n";

  /**
  for(unsigned int i = 0; i < m_tris.size(); ++i)
  {
    ngl::Vec3 ver0 = m_tris[i].getV0();
    ngl::Vec3 ver1 = m_tris[i].getV1();
    ngl::Vec3 ver2 = m_tris[i].getV2();
    std::cout<<"Triangle "<<i<<" :\n";
    std::cout<<"Ver 0 : ["<<ver0[0]<<", "<<ver0[1]<<", "<<ver0[2]<<"]\n";
    std::cout<<"Ver 1 : ["<<ver1[0]<<", "<<ver1[1]<<", "<<ver1[2]<<"]\n";
    std::cout<<"Ver 2 : ["<<ver2[0]<<", "<<ver2[1]<<", "<<ver2[2]<<"]\n";
  }
  **/

  //use library to do weathering
  std::cout<<"Starting Weathering\n";
  //count iterations
  int iterationCounter = 0;
  int iterationTotal = 3;

  //load all of the PWL classes we need
  pwl::Vertical ver; //vertical emitter
  pwl::BBox bb; //bounding box emitter
  pwl::Hemisphere hem; //hemispherical emitter

  while(iterationCounter < iterationTotal)
  {
    if(iterationCounter == 0)
    {
      //setup vertical emitter
      ver.create(bbox, 1000);

      hem.create(bbox, 10000);
      std::cout<<"hemispherical emitter created\n";

      //set up bounding box emitter
      //number of surfels is per bounding box face
      bb.create(bbox, 10000, objCentre);
      std::cout<<"bounding box emitter created\n";

      m_bboxvec = bb.getBBox();

      bb.goToSurface(m_tris);
      std::cout<<"bbox surfels emitted\n";

      //start propagating
      ver.propagate(m_tris);
      std::cout<<"vertical gtons emitted\n";
      m_vert = ver.getVertical();

      //hemisphere gtons propagate towards centre
      hem.propagate(m_tris);
      std::cout<<"hemispherical gtons emitted\n";
      m_hem = hem.getHem();

      //transfer carrier properties
      //ver.transfer(m_bboxvec);
      //hem.transfer(m_bboxvec);

      //Our GammaTon Map is our surfel vector in BBox;
    }
    else
    {
        //repeat the process
        //ver.repeatProp(m_tris);
        //ver.transfer(m_bboxvec);
        //hem.repeatProp(m_tris);
        //hem.transfer(m_bboxvec);

        //if it is the last iteration pull the final GTon map
        if(iterationCounter < iterationTotal-1)
        {
          //get the final gammaton map
          //m_fGTMap = bb.getBBox();
        }
    }

    //increase counter
    iterationCounter += 1;
  }

  //normalise all of the positions in
  for(unsigned int i = 0; i < m_fGTMap.size(); ++i)
  {
    ngl::Vec3 tempPos = m_fGTMap[i].getPos();
    tempPos.normalize();
    m_textureVec.push_back(tempPos);
  }

  //create textures
  pwl::Texture tex;
  tex.generateTexture(m_textureVec);

  std::cout<<"Weathering has finished.\n";

  ngl::Mat4 iv;
  iv = m_cam->getViewMatrix();
  iv.transpose();

  ngl::VAOPrimitives *prim = ngl::VAOPrimitives::instance();
  prim->createSphere("sphere1",0.03,20);

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

  glViewport(0,0,width(),height());
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Colour"]->use();
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat4 M;

  M=m_transform.getMatrix()*m_mouseGlobalTX;
  MV=M*m_cam->getViewMatrix();
  MVP=MV*m_cam->getProjectionMatrix();

  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat4("MV",MV);

}

void NGLScene::loadGTonShader(ngl::Mat4 _trans)
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Colour"]->use();
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat4 M;

  M=m_transform.getMatrix()*m_mouseGlobalTX;
  MV=M*m_cam->getViewMatrix();
  MVP=MV*m_cam->getProjectionMatrix()*_trans*m_view;

  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat4("MV",MV);

}

void NGLScene::render()
{

  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //instance of shader lib
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  //translate
  ngl::Mat4 trans;

  shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);

  m_vaoMesh->bind();
  m_vaoMesh->draw();

  /**

  for(unsigned int i = 0; i < m_bboxvec.size(); ++i)
  {
    ngl::Vec3 bbPos = m_bboxvec[i]->getPos();
    //std::cout<<"bbPos : ["<<bbPos[0]<<", "<<bbPos[1]<<", "<<bbPos[2]<<"]\n";
    trans.translate(bbPos[0],bbPos[1],bbPos[2]);
    shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);
    ngl::VAOPrimitives::instance()->draw("sphere1");



		ngl::Vec3 rayStart = objCentre;
		ngl::Vec3 rayEnd = m_bboxvec[i]->getPos();

		ngl::VertexArrayObject *vao= ngl::VertexArrayObject::createVOA(GL_LINES);
		vao->bind();
		ngl::Vec3 points[2];
		points[0]= rayStart;
		points[1]= rayEnd;
		vao->setData(2*sizeof(ngl::Vec3),points[0].m_x);
		vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(ngl::Vec3),0);
		vao->setNumIndices(2);
		shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);
		vao->draw();
		vao->removeVOA();
		delete vao;
	}


  for(unsigned int i = 0; i < m_vert.size(); ++i)
  {
    ngl::Vec3 vPos = m_vert[i].getPos();
    trans.translate(vPos[0],vPos[1],vPos[2]);
    shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);
    ngl::VAOPrimitives::instance()->draw("sphere1");
  }**/

  for(unsigned int i = 0; i < m_hem.size(); ++i)
  {
    ngl::Vec3 vPos = m_hem[i].getPos();
    trans.translate(vPos[0],vPos[1],vPos[2]);
    shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);
    ngl::VAOPrimitives::instance()->draw("sphere1");

		ngl::Vec3 rayStart = objCentre;
		ngl::Vec3 rayEnd = m_hem[i].getPos();

		ngl::VertexArrayObject *vao= ngl::VertexArrayObject::createVOA(GL_LINES);
		vao->bind();
		ngl::Vec3 points[2];
		points[0]= rayStart;
		points[1]= rayEnd;
		vao->setData(2*sizeof(ngl::Vec3),points[0].m_x);
		vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(ngl::Vec3),0);
		vao->setNumIndices(2);
		shader->setShaderParamFromMat4("MVP",trans*m_view*m_projection);
		vao->draw();
		vao->removeVOA();
		delete vao;
	}
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
