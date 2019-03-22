
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <locale>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <queue>
#include <ctime>

#include "Object.h"
#include "Camera.h"
#include "Shader.h"


#include "SOIL.h"

void init();
bool isCollision();
bool readSetScene(const std::string& filename);
bool saveScene(const std::string& filename);

void display();
void reshape(int, int);
void idle();
void keyboard(unsigned char, int, int);
void special(int, int, int);

GLuint  program;
GLint   loc_a_texcoord;
GLint   loc_u_texid;

GLint	loc_a_vertex;
GLint	loc_a_normal;

GLint	loc_u_pvm_matrix;
GLint	loc_u_vm_matrix;
GLint	loc_u_normal_matrix;

GLint	loc_u_light_position;
GLint	loc_u_light_ambient;
GLint	loc_u_light_diffuse;
GLint	loc_u_light_specular;

GLint	loc_u_material_ambient;
GLint	loc_u_material_diffuse;
GLint	loc_u_material_specular;
GLint	loc_u_material_shininess ;


GLint       mode=0,game=0, level=0;

float model_scale = 1.0f;
float model_angle = 0.0f;

std::chrono::time_point<std::chrono::system_clock> prev, curr;


kmuvcl::math::vec4f material_ambient  = kmuvcl::math::vec4f(0.4f, 0.693f, 0.094f, 1.0f);
kmuvcl::math::vec4f material_diffuse  = kmuvcl::math::vec4f(0.4f, 0.693f, 0.094f, 1.0f);
kmuvcl::math::vec4f material_specular = kmuvcl::math::vec4f(1.0f, 1.0f, 1.0f, 1.0f);

float     material_shininess= 50.0f;
int rot_cnt =0;
int obj_cnt = 0;

kmuvcl::math::mat4x4f mat_PVM, mat_VM;
kmuvcl::math::mat3x3f mat_Normal;

kmuvcl::math::vec3f cur_position;
kmuvcl::math::vec3f next_position;

std::vector<kmuvcl::math::vec3f> vertices;
std::vector<kmuvcl::math::vec2f> texcoords;

GLfloat timeValue = 0.0f;

std::string g_filename;

Object	choice,tmp;

GLfloat hit=30.0f;

Object my_character;
std::vector<Object> objects;
std::vector<Object> buildings;
std::vector<Object> cho_objs;
std::queue <Object> buffer;
std::queue <Object> game_buffer;
std::vector <Object> life;


GLint char_index=0;

   // object
Camera  g_camera;			// viewer (you)


void MyTimer(int value)
{
	timeValue += 0.01;
	glutPostRedisplay();
	glutTimerFunc(20, MyTimer, 1);
}

int main(int argc, char* argv[])
{
  if (argc > 1)
    {
      g_filename = argv[1];
    }
    else
    {
      g_filename = "./scene/scene_1.txt";
    }

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(10-0, 1000);
  glutInitWindowSize(640, 640);
  glutCreateWindow("Modeling & Navigating Your Studio");

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  glutIdleFunc(idle);
  readSetScene(g_filename);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "failed to initialize glew" << std::endl;
		return -1;
	}

  init();
  glutTimerFunc(40, MyTimer, 1);
  glutMainLoop();

  return 0;
}

bool readSetScene(const std::string& filename){

  	std::ifstream file(filename.c_str()); //string -> char : strcmp, strcat

  	if (!file.is_open())
  	{
  		std::cerr << "failed to open file: " << filename << std::endl;
  		return false;
  	}

    std::string type_str;
    char slash;				// get only on character '\'

    std::string line;
    std::locale loc;


    std::stringstream ss;

  	while (!file.eof())
  	{
  		std::getline(file, line);

  		ss.clear();
  		ss.str(line);

      kmuvcl::math::vec3f position;
      std::string name;
      GLfloat s,r;
      Object obj;

  		// comment or space
  		if (line[0] == '#' || std::isspace(line[0], loc))
  		{
  			continue; // skip
  		}
      else if (line.substr(0, 2) == "o ") // open material file
      {
        ss >> type_str >> name >> position(0) >> position(1) >> position(2) >> s >> r ;
        obj.load_simple_obj(name);
        obj.set_value(position,s,r);
        objects.push_back(obj);
      }
  		else if (line.substr(0, 3) == "ob ")   // vertex
  		{
        ss >> type_str >> name >> position(0)>> position(1) >> position(2) >> s >> r ;
        obj.load_simple_obj(name);
        obj.set_value(position,s,r);

				buildings.push_back(obj);
      }

  		else if (line.substr(0, 2) == "c ")   // vertex
  		{
        ss >> type_str >> name >> position(0)>> position(1) >> position(2) >> s >> r ;
        obj.load_simple_obj(name);
        obj.set_value(position,s,r);
        choice =obj;
				tmp =obj;
      }
			else if (line.substr(0, 2) == "h ")   // vertex
			{
				ss >> type_str >> name >> position(0)>> position(1) >> position(2) >> s >> r ;
				obj.load_simple_obj(name);
				obj.set_value(position,s,r);
				life.push_back(obj);
			}
    }

		float temp = buildings[0].offsetZ;
		buildings[0].offsetZ = buildings[0].offsetX;
		buildings[0].offsetX = temp;

	 	temp = buildings[1].offsetZ;
		buildings[1].offsetZ = buildings[1].offsetX;
		buildings[1].offsetX = temp;

		for(int i = 0  ; i < 3 ; i++)
		{
			buildings[i].offsetY += 300.0f;
		}
		buildings[3].offsetY = 5.0f ;

		buildings[4].offsetX = 0.0f ;buildings[4].offsetY = 0.0f ;	buildings[4].offsetZ = 0.0f ;

  	std::cout << "finished to read: " << filename << std::endl;
  	return true;
}

bool saveScene(const std::string& filename){

  	std::fstream file(filename.c_str()); //string -> char : strcmp, strcat
		std::fstream file2(filename.c_str());

  	if (!file.is_open())
  	{
  		std::cerr << "failed to open file: " << filename << std::endl;
  		return false;
  	}

    std::string type_str;
    char slash;				// get only on character '\'

    std::string line;
    std::locale loc;


    std::stringstream ss;

  	while (!file.eof())
  	{
  		std::getline(file, line);

  		ss.clear();
  		ss.str(line);

      kmuvcl::math::vec3f position;
      std::string name;
      GLfloat s,r;

			// file2 << '#' << std::endl;

			if (line[0] == '#' || std::isspace(line[0], loc))
  		{
				file2 << line << std::endl;
  		}
      else if (line.substr(0, 2) == "o ") // open material file
      {
        ss >> type_str >> name >> position(0) >> position(1) >> position(2) >> s >> r;
        file2 << type_str <<  " " << name << " " <<  objects.at(obj_cnt).position()(0) << " " << objects.at(obj_cnt).position()(1) << " " << objects.at(obj_cnt).position()(2) << " " << objects.at(obj_cnt).scale()
				<< " "<< objects.at(obj_cnt).rotate() << std::endl;
				obj_cnt++;
      }
  		else if (line.substr(0, 3) == "ob ")   // vertex
  		{
        file2 << line << std::endl;
      }

  		else if (line.substr(0, 2) == "c ")   // vertex
  		{
  			file2 << line << std::endl;
      }

			else if (line.substr(0, 2) == "h ")   // vertex
  		{
        ss >> type_str >> name;
    		file2 << line << std::endl;
      }

			else if (line.substr(0, 2) == "t ")   // vertex
  		{
        ss >> type_str >> name;
    		file2 << type_str << " " << name << std::endl;
      }


    }
		obj_cnt = 0;
  	std::cout << "finished to save: " << filename << std::endl;
  	return true;
}

void init()
{

  vertices.push_back(kmuvcl::math::vec3f(-1.0f, -1.0f, 0.0f));
  vertices.push_back(kmuvcl::math::vec3f(1.0f, -1.0f, 0.0f));
  vertices.push_back(kmuvcl::math::vec3f(1.0f, 1.0f, 0.0f));
  vertices.push_back(kmuvcl::math::vec3f(-1.0f, 1.0f, 0.0f));

  // texture coordinates
  texcoords.push_back(kmuvcl::math::vec2f(0.0f, 0.0f));
  texcoords.push_back(kmuvcl::math::vec2f(1.0f, 0.0f));
  texcoords.push_back(kmuvcl::math::vec2f(1.0f, 1.0f));
  texcoords.push_back(kmuvcl::math::vec2f(0.0f, 1.0f));


  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glFrontFace(GL_CCW);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glCullFace(GL_BACK);


  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    // for filled polygon rendering

  program = Shader::create_program("./shader/phong_vert.glsl", "./shader/phong_frag.glsl");

	loc_u_pvm_matrix = glGetUniformLocation(program, "u_pvm_matrix");
	loc_u_vm_matrix   = glGetUniformLocation(program, "u_vm_matrix");
	loc_u_normal_matrix = glGetUniformLocation(program, "u_normal_matrix");

	loc_u_light_position = glGetUniformLocation(program, "u_light_position");
	loc_u_light_ambient = glGetUniformLocation(program, "u_light_ambient");
	loc_u_light_diffuse = glGetUniformLocation(program, "u_light_diffuse");
	loc_u_light_specular = glGetUniformLocation(program, "u_light_specular");

	loc_u_material_ambient = glGetUniformLocation(program, "u_material_ambient");
	loc_u_material_diffuse = glGetUniformLocation(program, "u_material_diffuse");
	loc_u_material_specular = glGetUniformLocation(program, "u_material_specular");
  loc_u_material_shininess = glGetUniformLocation(program, "u_material_shininess");

	loc_a_vertex			= glGetAttribLocation(program, "a_vertex");
	loc_a_normal			= glGetAttribLocation(program, "a_normal");
  loc_a_texcoord 			= glGetAttribLocation(program, "a_texcoord");
  loc_u_texid = glGetUniformLocation(program, "u_texid");
}


void display()
{
	srand(time(NULL));
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

  if(mode!=1){
  objects.at(char_index).set_value(my_character.position(), my_character.scale(), my_character.rotate());
  }
	// Camera setting
	kmuvcl::math::mat4x4f   mat_Proj, mat_View, mat_Model;

	kmuvcl::math::mat4x4f mat, T_, S_, R_;

	// camera extrinsic param
  mat_View  = kmuvcl::math::lookAt(
    g_camera.position()(0), g_camera.position()(1), g_camera.position()(2),				// eye position
        g_camera.center_position()(0), g_camera.center_position()(1), g_camera.center_position()(2), // center position
  		  g_camera.up_direction()(0), g_camera.up_direction()(1), g_camera.up_direction()(2)			// up direction);
      );
	// camera intrinsic param
	mat_Proj  = kmuvcl::math::perspective(g_camera.fovy(), 1.0f, 0.001f, 10000.0f);


	 mat_Model = kmuvcl::math::scale(model_scale, model_scale, model_scale);
	 // mat_Model = kmuvcl::math::rotate(model_angle*0.7f, 0.0f, 0.0f, 1.0f) * mat_Model;
	 // mat_Model = kmuvcl::math::rotate(model_angle,      0.0f, 1.0f, 0.0f) * mat_Model;
	 // mat_Model = kmuvcl::math::rotate(model_angle*0.5f, 1.0f, 0.0f, 0.0f) * mat_Model;

	mat_VM	= mat_View*mat_Model;
	glUniformMatrix4fv(loc_u_vm_matrix, 1, false, mat_VM);

  mat_Normal(0, 0) = mat_Model(0, 0);
  mat_Normal(0, 1) = mat_Model(0, 1);
  mat_Normal(0, 2) = mat_Model(0, 2);
  mat_Normal(1, 0) = mat_Model(1, 0);
  mat_Normal(1, 1) = mat_Model(1, 1);
  mat_Normal(1, 2) = mat_Model(1, 2);
  mat_Normal(2, 0) = mat_Model(2, 0);
  mat_Normal(2, 1) = mat_Model(2, 1);
  mat_Normal(2, 2) = mat_Model(2, 2);


	glUniformMatrix3fv(loc_u_normal_matrix, 1, false, mat_Normal);

	mat_PVM   = mat_Proj*mat_VM;
	glUniformMatrix4fv(loc_u_pvm_matrix, 1, false, mat_PVM);


	kmuvcl::math::vec3f light_position =  kmuvcl::math::vec3f(10.0f, -5.0f, 10.0f);
	glUniform3fv(loc_u_light_position, 1, light_position);

	kmuvcl::math::vec4f  light_ambient = kmuvcl::math::vec4f(1.0f,1.0f,1.0f,1.0f);
	glUniform4fv(loc_u_light_ambient, 1,light_ambient);

	kmuvcl::math::vec4f  light_diffuse = kmuvcl::math::vec4f(1.0f,1.0f,1.0f,1.0f);
	glUniform4fv(loc_u_light_diffuse, 1,light_diffuse);

	kmuvcl::math::vec4f  light_specular = kmuvcl::math::vec4f(1.0f,1.0f,1.0f,1.0f);
	glUniform4fv(loc_u_light_specular, 1,light_specular);

  glUniform4fv(loc_u_material_ambient, 1, material_ambient);
  glUniform4fv(loc_u_material_diffuse, 1, material_diffuse);
  glUniform4fv(loc_u_material_specular, 1, material_specular);
  glUniform1f(loc_u_material_shininess, material_shininess);

 // 8. bind the texture object

	  glActiveTexture(GL_TEXTURE0);

for(int i=0; i<objects.size(); i++){

				mat = mat_Model = kmuvcl::math::scale(model_scale, model_scale, model_scale);
        T_ = kmuvcl::math::translate(objects.at(i).position()(0), objects.at(i).position()(1),objects.at(i).position()(2));
				S_ = kmuvcl::math::scale(objects.at(i).scale(),objects.at(i).scale(), objects.at(i).scale());
				R_ = kmuvcl::math::rotate(objects.at(i).rotate(),0.0f, 1.0f, 0.0f);

				mat = T_ * R_ * S_;


    	  glBindTexture(GL_TEXTURE_2D, objects.at(i).textureid);
    	  glUniform1i(loc_u_texid, 0);

     	  glUniformMatrix4fv(loc_u_pvm_matrix, 1, false, mat_PVM*mat);
    	  objects.at(i).draw(loc_a_vertex, loc_a_normal,  loc_a_texcoord,
        	  loc_u_material_ambient, loc_u_material_diffuse,
        	  loc_u_material_specular, loc_u_material_shininess);
 }


for(int i=0; i<buildings.size(); i++){



				T_ = kmuvcl::math::translate(buildings.at(i).position()(0), buildings.at(i).position()(1),buildings.at(i).position()(2));
				S_ = kmuvcl::math::scale(buildings.at(i).scale(),buildings.at(i).scale(), buildings.at(i).scale());
				R_ = kmuvcl::math::rotate(buildings.at(i).rotate(),0.0f, 1.0f, 0.0f);

			  mat = T_ * R_ * S_;

    	  glBindTexture(GL_TEXTURE_2D, buildings.at(i).textureid);
    	  glUniform1i(loc_u_texid, 0);

     	  glUniformMatrix4fv(loc_u_pvm_matrix, 1, false, mat_PVM*mat);
    	  buildings.at(i).draw(loc_a_vertex, loc_a_normal,  loc_a_texcoord,
        	  loc_u_material_ambient, loc_u_material_diffuse,
        	  loc_u_material_specular, loc_u_material_shininess);
 }

if(mode){



	T_ = kmuvcl::math::translate(choice.position()(0), choice.position()(1),choice.position()(2));
	S_ = kmuvcl::math::scale(choice.scale(),choice.scale(), choice.scale());
	R_ = kmuvcl::math::rotate(choice.rotate(),0.0f, 1.0f, 0.0f);

	mat = T_ * R_ * S_;


  glBindTexture(GL_TEXTURE_2D, choice.textureid);
  glUniform1i(loc_u_texid, 0);

  glUniformMatrix4fv(loc_u_pvm_matrix, 1, false,mat_PVM*mat);
  choice.draw(loc_a_vertex, loc_a_normal,  loc_a_texcoord,
      loc_u_material_ambient, loc_u_material_diffuse,
      loc_u_material_specular, loc_u_material_shininess);
}

if(game){

	if(life.empty()){
		while(!game_buffer.empty()){
			life.push_back(game_buffer.front());
			game_buffer.pop();
		}
	}

	for(int i = 0; i <life.size(); i++){
		int speed = level*10+5;
	if(i)
	{
		life.at(i).X(rand()%speed*0.1f);
		life.at(i).Z(rand()%speed*0.1f);
	}
	else{
		life.at(i).X(-rand()%speed*0.1f);
		life.at(i).Z(-rand()%speed*0.1f);
	}

	T_ = kmuvcl::math::translate(life.at(i).position()(0),1.0f,life.at(i).position()(2));
	S_ = kmuvcl::math::scale(life.at(i).scale(),life.at(i).scale(), life.at(i).scale());
	R_ = kmuvcl::math::rotate(life.at(i).rotate(),0.0f, 1.0f, 0.0f);

	mat = T_ * R_ * S_;


	glBindTexture(GL_TEXTURE_2D, life.at(i).textureid);
	glUniform1i(loc_u_texid, 0);

	glUniformMatrix4fv(loc_u_pvm_matrix, 1, false,mat_PVM*mat);
	life.at(i).draw(loc_a_vertex, loc_a_normal,  loc_a_texcoord,
			loc_u_material_ambient, loc_u_material_diffuse,
			loc_u_material_specular, loc_u_material_shininess);


if(choice.position()(2)< life.at(i).position()(2)+hit&&choice.position()(2)> life.at(i).position()(2)-hit&& choice.position()(0)<= life.at(i).position()(0)+hit&&choice.position()(0)> life.at(i).position()(0)-hit)
	{

		game_buffer.push(life.at(i));
		life.erase(life.begin()+ i);
		i=0;
			if(life.empty()) {
			choice.set_value(tmp.position(), tmp.scale(), tmp.rotate());
			game =0;
			break;
		}
	}
	}
}

	glUseProgram(0);

	Shader::check_gl_error("draw");

  	glutSwapBuffers();
}



void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
}




void keyboard(unsigned char key, int x, int y)
{

	if ('a' == key || 'A' == key)
	{
		if(!mode){g_camera.rotate_left(5.1f);}
		else if(mode==2) {my_character.R(5.5f);
    }
	}
	else if ('d' == key || 'D' == key)
	{
		if(!mode){g_camera.rotate_right(5.1f);}
		else if(mode==2){my_character.R(-5.5f);
    }
	}

	else if ('w' == key || 'W' == key)
	{
		if(!mode){
			cur_position = g_camera.position();
			g_camera.add_y(1.0f);
			next_position = g_camera.position();
			if(isCollision())
				g_camera.add_y(-1.0f);
		}
		else if(mode==2){my_character.Y(1.5f);
		}
	}
	else if ('s' == key || 'S' == key){
		if(!mode)
		{
			cur_position = g_camera.position();
			g_camera.add_y(-1.0f);
			next_position = g_camera.position();
			if(isCollision())
				g_camera.add_y(1.0f);
		}
		else if(mode==2){my_character.Y(-1.5f);
		}
	}

	else if ('q' == key || 'Q' == key)
	{
		exit(0);
	}

	else if ('f' == key || 'F' == key)
	{
		saveScene(g_filename);
	}


	else if('z' == key || 'Z' == key)
	{
		if(mode==1 && !objects.empty()){
			buffer.push(objects.at(char_index));
			objects.erase(objects.begin()+char_index);
		 choice.set_value(tmp.position(), tmp.scale(), tmp.rotate());
		 char_index--;
		 if(char_index<0) char_index =0;
	}
}

	else if('v' == key || 'V' == key)
	{
			if(mode==1 && !buffer.empty()){
			objects.push_back(buffer.front());
			buffer.pop();
			choice.set_value(tmp.position(), tmp.scale(), tmp.rotate());
		}
	}

	else if('m' == key || 'M' == key)
	{
    if(mode==1){
    my_character.set_value(objects.at(char_index).position(),objects.at(char_index).scale(),objects.at(char_index).rotate());
		 choice.set_value(tmp.position(), tmp.scale(), tmp.rotate());
    }
		mode++;
    mode %= 3;
  }

	else if('c' == key || 'C' == key)
 {
	 if(mode==1){
		 char_index++;
		 char_index %=objects.size();
		 choice.set_value(objects.at(char_index).position(),70,0);
		 choice.Y(10.0f);
		 choice.X(40.0f);
		 choice.Z(-30.0f);
	 }
 }

 else if('i' == key || 'I' == key)
 {
		 if(mode==1){
			 game =1;
			 choice.set_value(kmuvcl::math::vec3f(80.0f,-25.0f,1.0f), tmp.scale(), tmp.rotate());
			 level++;
			 level %= 6;
		 }
 }

	else if('j' == key || 'J' == key)
	{
		if(mode ==1)game =0;
	}
  glutPostRedisplay();

}

void special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		if(!mode)
		{
			cur_position = g_camera.position();
			g_camera.move_forward(1.0f);
			next_position = g_camera.position();
			if(isCollision())
				g_camera.move_forward(-1.0f);
		}

		else if(mode==2){  my_character.Z(-0.5f); }
		else if(mode==1 && game==1){  choice.Z(-5.5f); }

		break;
	case GLUT_KEY_DOWN:
		if(!mode)
		{
			cur_position = g_camera.position();
			g_camera.move_backward(1.0f);
			next_position = g_camera.position();
			if(isCollision())
				g_camera.move_backward(-1.0f);
		}

		else if(mode==2){ my_character.Z(0.5f); }
		else if(mode==1 && game==1){  choice.Z(5.5f); }
		break;
	case GLUT_KEY_LEFT:
		if(!mode)
		{
			cur_position = g_camera.position();
			g_camera.move_left(1.0f);
			next_position = g_camera.position();
			if(isCollision())
				g_camera.move_left(-1.0f);
		}
		else if(mode==2){my_character.X(-0.5f);}
		else if(mode==1 && game==1){  choice.X(-5.5f); }
		break;
	case GLUT_KEY_RIGHT:
		if(!mode)
		{
			cur_position = g_camera.position();
			g_camera.move_right(1.0f);
			next_position = g_camera.position();
			if(isCollision())
				g_camera.move_right(-1.0f);
		}
		else if(mode==2){my_character.X(0.5f);}
			else if(mode==1 && game==1){  choice.X(5.5f); }
    break;

  default:
		break;
	}
	glutPostRedisplay();
}

void idle()
{
  curr = std::chrono::system_clock::now();

  std::chrono::duration<float> elaped_seconds = (curr - prev);

  model_angle += 10 * elaped_seconds.count();

  prev = curr;

  glutPostRedisplay();
}

bool isCollision()
{

	for(int i=0; i<objects.size(); i++)
	{

			if( (objects[i].position()(0)-objects[i].offsetX/2) <= next_position(0) && next_position(0) <= (objects[i].position()(0) + objects[i].offsetX/2) )
				if( (objects[i].position()(1)-objects[i].offsetY/2) <= next_position(1) && next_position(1) <= (objects[i].position()(1) + objects[i].offsetY/2) )
						if( (objects[i].position()(2)-objects[i].offsetZ/2) <= next_position(2) && next_position(2) <= (objects[i].position()(2) + objects[i].offsetZ/2) )
								{
									std::cout << "물체충돌됨" << std::endl;
									return true;
								}
	}
	for(int i=0; i<buildings.size(); i++)
	{

		if( (buildings[i].position()(0)-buildings[i].offsetX/2) <= next_position(0) && next_position(0) <= (buildings[i].position()(0) + buildings[i].offsetX/2) )
			if( (buildings[i].position()(1)-buildings[i].offsetY/2) <= next_position(1) && next_position(1) <= (buildings[i].position()(1) + buildings[i].offsetY/2) )
					if( (buildings[i].position()(2)-buildings[i].offsetZ/2) <= next_position(2) && next_position(2) <= (buildings[i].position()(2) + buildings[i].offsetZ/2) )
						{
							std::cout << "벽충돌됨" << std::endl;
							return true;
						}
	}
	return false;
}
