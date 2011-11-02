
#pragma comment(lib, "opengl32.lib")



#include "common.h"

#include "camera.h"
#include "shapes/shapes.h"

#include "depends/noise/perlin.h"

#include <vector>
#include <algorithm>
#include <functional>


#define M_PI       3.14159265358979323846f
#define TWO_PI     6.28318530717958647692f

template <class T> T DEGREES_TO_RADIANS(T degrees)
{
	return degrees * T(M_PI) / T(180.f);
}

template <class T> T RADIANS_TO_DEGREES(T radians)
{
	return radians / T(M_PI) * T(180.f);
}






class ProtoGraphics
{
private:
	// Disallowing copying. Please pass protographics about as a const ptr or refrence!
	ProtoGraphics(const ProtoGraphics&); // no implementation 
	ProtoGraphics& operator=(const ProtoGraphics&); // no implementation 


public:
	ProtoGraphics()
	{
		instance = this;

		isRunning = false;

		time = 0.f;
		old_time = 0.f;
		delta = 0.f;

		numframes = 0;

		mousx = 0;
		mousy = 0;

		light_pos = glm::vec3( 0.f, 0.f, 0.f );

		for( int i = 0; i < 256; i++ )
		{
			key_array[i] = false;
		}

		blend_state = blending::SOLID_BLEND;
		colorState = glm::vec4( 1.f );
		move_to_state = glm::vec2(0.f, 0.f);
	}

	~ProtoGraphics()
	{

	}

	// fast float random in interval -1,1
	// source by RGBA: http://www.rgba.org/articles/sfrand/sfrand.htm
	float sfrand( void )
	{
		unsigned int a=(rand()<<16)|rand();  //we use the bottom 23 bits of the int, so one
		//16 bit rand() won't cut it.
		a=(a&0x007fffff) | 0x40000000;  

		return( *((float*)&a) - 3.0f );
	}

	void dump_stats()
	{
		printf("------------------------\n");
		printf("num opaque: %d\n", num_opaque);
		printf("num blended: %d\n", num_blended);
	}

	

	bool init(int xres, int yres)
	{
		int ok = glfwInit();
		if ( ok==0 ) return false;
		this->xres = xres;
		this->yres = yres;

		// TODO: use GL 3.3 and core profile that wont run any deprecated stuff
		// TODO will fail if vidcard doesnt support gl 3.3... make a function that tries highest first, then steps down
		// or just remove for release build
		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
		glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
		ok = glfwOpenWindow(xres,yres,8,8,8,8,24,16,GLFW_WINDOW);
		if (ok == 0 ) return false;


		glfwSetWindowTitle("ProtoWizard script server");
		glfwSwapInterval(1);
		
		glewExperimental = GL_TRUE;
		int err = glewInit();
		if ( GLEW_OK != err )
		{
			printf("Failed to init GLEW\n");
			return false;
		}

		glfwSetWindowCloseCallback( &_closeCallback );

		// Clear error caused by GLEW using glGetString instead of glGetStringi( char*, int )
		for ( GLenum Error = glGetError( ); ( GL_NO_ERROR != Error ); Error = glGetError( ) )
		{
		}


		camera.pos = glm::vec3( 0.0f, 0.0f, -50.0f );



		geo_lib.init();

		


		if ( shader_lines2d.install("assets/line2d_shader.vert", "assets/line2d_shader.frag") == false )
		{
			return false;
		}else{
			shader_list.push_back( &shader_lines2d );
		}

		if ( shader_2d.install("assets/shader2d.vert", "assets/shader2d.frag") == false )
		{
			return false;
		}
		shader_list.push_back( &shader_2d );

		if ( sphere_shader.install("assets/sphere_shader.vert", "assets/sphere_shader.frag") == false )
		{
			return false;
		}
		shader_list.push_back( &sphere_shader );

		if (cylinder_shader.install("assets/drawcone.vert", "assets/drawcone.frag") == false)
		{
			return false;
		}
		shader_list.push_back( &cylinder_shader );

		if (cube_shader.install("assets/pointlight.vert", "assets/pointlight.frag") == false)
		//if (cube_shader.install("assets/cube_shader.vert", "assets/cube_shader.frag") == false)
		{
			return false;
		}else{
			shader_list.push_back( &cube_shader );
		}

		if (plane_shader.install("assets/drawcone.vert", "assets/drawcone.frag") == false)
		{
			return false;
		}else{
			shader_list.push_back( &plane_shader );
		}

		if ( phong_shader.install("assets/phong.vert", "assets/phong.frag") == false )
		{
			return false;
		}else{
			shader_list.push_back( &phong_shader );
		}
		

		glEnable(GL_MULTISAMPLE);

		GetError();
			
		isRunning = true;
		return true;
	}


	void setCamera( glm::vec3 pos, glm::vec3 target, glm::vec3 up )
	//void setCamera( glm::vec3 pos, float horiz, float verti )
	{
		//glm::mat4 rmx = glm::rotate( glm::mat4(1.0), RADIANS_TO_DEGREES<float>(verti), glm::vec3(1.f, 0.f, 0.f)  );
		//glm::mat4 rmy = glm::rotate( glm::mat4(1.0), RADIANS_TO_DEGREES<float>(horiz), glm::vec3(0.f, 1.f, 0.f)  );
		//glm::mat4 rotmat = rmx * rmy;
		//mCam = rotmat;
		//mCam = glm::translate( mCam, pos );

		mCam = glm::lookAt( pos, target, up );
	}

	void shutdown()
	{	
		geo_lib.shutdown();

		for(unsigned int i=0; i<shader_list.size(); i++){
			shader_list[i]->shutdown();
		}

		isRunning = false;

		glfwCloseWindow();
		glfwTerminate();
	}

	void cls( float r, float g, float b )
	{
		assert( isRunning );
		glClearColor( r,  g,  b, 1.0f );
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	}

	void frame()
	{	
		assert( isRunning );
		GetError();

		time = klock();
		delta = time - old_time;
		delta = std::min<float>( 0.01f, delta );

		old_time = time;

		camera.update( keystatus('A'), keystatus('D'), keystatus('W'), keystatus('S'), (float)getMouseX(), (float)getMouseY(), mouseDown(), delta );


		setColor(1.f, 1.f, 1.f);
		light_pos = glm::vec3( cos(time) * 15.f, 10.0f , sin(time) * 15.f );
		drawSphere( light_pos, 1.0f );



		if ( key_array['R'] )
		{
			key_array['R'] = false;
			printf("/////////////////////////////R E L O A D I N G   S H A D E R S /////////////\n");
			for(unsigned int i=0; i<shader_list.size(); i++)
			{
				if ( shader_list[i]->reload() == true )
				{
					continue;
				}else{
					printf("could not relead shader\n");
					break;
				}
			}
		}

		if ( key_array['1'] == true )
		{
			key_array['1'] = false;
			static bool wire_frame_mode = false;
			wire_frame_mode = !wire_frame_mode;
			
			glPolygonMode(GL_FRONT_AND_BACK, wire_frame_mode ? GL_LINE : GL_FILL);
			
		}
		draw_buffered_objects();

		char title_buf[256];
		sprintf_s(title_buf, 256, "%i .... %.1f mspf alpha = %f", numframes, delta*1000.0f, colorState.a);
		glfwSetWindowTitle(title_buf);

		glfwGetMousePos(&mousx, &mousy);

		numframes++;

		glfwSwapBuffers();

		for(int i=0; i<256; i++)
		{
			glfwGetKey(i) == 1 ? key_array[i]=true : key_array[i]=false;
		}
	}

	bool isWindowOpen()
	{
		return isRunning;
	}

	int getWindowWidth() 
	{
		return xres;
	}

	int getWindowHeight() 
	{
		return yres;
	}

	float klock()
	{
		return (float) glfwGetTime();
	}

	int getMouseX() 
	{
		return mousx;
	}

	int getMouseY() 
	{
		return mousy;
	}

	bool mouseDown()
	{
		return glfwGetMouseButton( GLFW_MOUSE_BUTTON_1 ) == 1;
	}

	bool keystatus(int key)
	{
		return key_array[key];
	}

	FirstPersonCamera getCamera()
	{
		return camera;
	}


	double octaves_of_noise(int octaves, double x, double y, double z){
		double value = 0.0;
		double vec[3];
		for(int i=0; i<octaves; i++){
			vec[0] = x*pow( 2, (double)i );
			vec[1] = y*pow( 2, (double)i );
			vec[2] = z*pow( 2, (double)i );
			value += noise3( vec );
		}
		return value;
	}

	void setColor( float r, float g, float b )
	{
		colorState.r = r;
		colorState.b = b;
		colorState.g = g;
	}

	void setAlpha( float a )
	{
		colorState.a = a;
	}

	

	void setBlend( bool active )
	{
		// TODO guess I want 2D blending at least... so, should each ShapeState have a BLEND_FUNC property?
		// and should blending allways be enabled? yes... maybe... maybe handle it the same way as color
		// so you do setBlend( LIGHT_BLEND ) etc. steal blitzmax' names :]
		// SOLIDBLEND (no blend, overwrite) ALPHABLEND (use alpha channel in image and specified draw color RGBA), LIGHTBLEND (additive), SHADEBLEND (multiply with backbuffer, MASKBLEND (draw if alpha > .5 )
		// see http://en.wikibooks.org/wiki/BlitzMax/Modules/Graphics/Max2D#SetBlend
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		if ( active )
		{
			blend_state = blending::ADDITIVE_BLEND;
		}else{
			blend_state = blending::SOLID_BLEND;
		}
		
	}

	void moveTo( float x, float y )
	{
		move_to_state = glm::vec2(x,y);
	}

	void lineTo( float to_x, float to_y )
	{
		LineSegmentState state;
		state.color1 = colorState;
		state.color2 = colorState;
		state.x1 = move_to_state.x;
		state.y1 = move_to_state.y;
		state.x2 = to_x;
		state.y2 = to_y;

		buffered_lines.push_back( state );

		move_to_state = glm::vec2(to_x,to_y); 
	}

	void drawCircle( float x, float y, float radius )
	{
		CircleState state;
		state.color = colorState;
		state.x = x;
		state.y = y;
		state.radius = radius;
		buffered_circles.push_back( state );
	}

	void drawSphere( glm::vec3 position, float radius ) 
	{
		SphereState *state = new SphereState;
		state->color = colorState;
		state->transform = glm::translate( glm::mat4( 1.0f ), position );
		state->radius = radius;
		state->blend_mode = blend_state;
		buffered_shapes.push_back( state );
	}

	void drawCone( glm::vec3 p1, float r1, glm::vec3 p2, float r2 ) 
	{
		CylinderState *state = new CylinderState;
		state->color = colorState;
		state->p1 = p1;
		state->p2 = p2;
		state->radius1 = r1;
		state->radius2 = r2;
		state->blend_mode = blend_state;
		buffered_shapes.push_back( state );
	}

	void drawCube( glm::vec3 position, float radius )
	{
		CubeState *state = new CubeState;
		state->color = colorState;
		state->transform = glm::translate( glm::mat4( 1.0f ), position );
		state->radius = radius;
		state->blend_mode = blend_state;
		buffered_shapes.push_back( state );
	}

	void drawPlane( glm::vec3 position, glm::vec3 normal, float radius )
	{
		PlaneState *state = new PlaneState;
		state->color = colorState;
		state->transform = glm::translate( glm::mat4( 1.0f ), position );
		state->normal = normal;
		state->radius = radius;
		state->blend_mode = blend_state;
		buffered_shapes.push_back( state );
	}

	void drawRoundedCube(float radius, float edge_radius)
	{
		float sc = radius;
		glm::vec3 v0(-sc,-sc,-sc);
		glm::vec3 v1(-sc,-sc,+sc);
		glm::vec3 v2(-sc,+sc,-sc);
		glm::vec3 v3(-sc,+sc,+sc);

		glm::vec3 v4(+sc,-sc,-sc);
		glm::vec3 v5(+sc,-sc,+sc);
		glm::vec3 v6(+sc,+sc,-sc);
		glm::vec3 v7(+sc,+sc,+sc);

#define drawline(v1,v2) (drawCone(v1,edge_radius,v2,edge_radius))
		// bottom quad
		drawline(v0,v1);
		drawline(v0,v4);
		drawline(v4,v5);
		drawline(v1,v5);
		// top quad
		drawline(v2,v3);
		drawline(v2,v6);
		drawline(v6,v7);
		drawline(v7,v3);

		// connectors top/bottom quad
		drawline(v0,v2);
		drawline(v4,v6);
		drawline(v1,v3);
		drawline(v7,v5);
#undef drawline

		drawSphere(v0, edge_radius);
		drawSphere(v1, edge_radius);
		drawSphere(v2, edge_radius);
		drawSphere(v3, edge_radius);
		drawSphere(v4, edge_radius);
		drawSphere(v5, edge_radius);
		drawSphere(v6, edge_radius);
		drawSphere(v7, edge_radius);

		glm::vec3 n0 ( 0.f, 0.f, -1.f);
		glm::vec3 n1 ( 0.f, 0.f, +1.f);
		glm::vec3 n2 ( 0.f, -1.f, 0.f);
		glm::vec3 n3 ( 0.f, +1.f, 0.f);
		glm::vec3 n4 ( -1.f, 0.f, 0.f);
		glm::vec3 n5 ( +1.f, 0.f, 0.f);
		
		float radius2 = radius + edge_radius;
		drawPlane(n0*radius2,n0, radius );
		drawPlane(n1*radius2,n1, radius );
		drawPlane(n2*radius2,n2, radius );
		drawPlane(n3*radius2,n3, radius );
		drawPlane(n4*radius2,n4, radius );
		drawPlane(n5*radius2,n5, radius );

	}


private:


	static int _closeCallback(void)
	{
		instance->shutdown();
		
		return 0; // Do not close OpenGL context yet...
	}

	void draw_buffered_lines()
	{		
		shader_lines2d.begin();
		unsigned int loc = shader_lines2d.GetVariable("mvp");
		glm::mat4 orthomat = glm::ortho(  0.f, (float)xres, (float)yres, 0.f );
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(orthomat) );
		geo_lib.line.draw(buffered_lines);
		buffered_lines.clear();
	}

	void draw_buffered_circles()
	{		
		shader_2d.begin();

		// TODO, uniform locs could be moved to init code, so they are only set once, 
		// but since its convenient to modify them, they stay for now.
		unsigned int mvLoc;
		unsigned int projLoc;
		unsigned int colorLoc;
		mvLoc    = shader_2d.GetVariable("modelviewMatrix");
		projLoc  = shader_2d.GetVariable("projMatrix");
		colorLoc = shader_2d.GetVariable("vColor");

		VSML *vsml = VSML::getInstance();
		vsml->initUniformLocs(mvLoc, projLoc);
		// send allready init-ed ortho to shader
		vsml->matrixToUniform(VSML::PROJECTION); 

		for (unsigned int i=0; i<buffered_circles.size(); i++)
		{
			float x = buffered_circles[i].x;
			float y = buffered_circles[i].y;
			float radius = buffered_circles[i].radius;

			glm::vec4 color = buffered_circles[i].color;
			 // Todo. use float vector here? and direct pointer to &color.r
			shader_2d.SetFloat4( colorLoc, color.r, color.g, color.b, color.a );

			vsml->loadIdentity(VSML::MODELVIEW);
			vsml->translate(x, y, 0.0f);
			// Get the scaling factor
			float scale_factor = radius; 
			vsml->scale(scale_factor,scale_factor,scale_factor);

			vsml->matrixToUniform(VSML::MODELVIEW);

			geo_lib.circle.draw();
		}
		buffered_circles.clear();
	}

	void _init_phong()
	{
		unsigned int loc1 = phong_shader.GetVariable("lightIntensity");
		unsigned int loc2 = phong_shader.GetVariable("ambientIntensity");

		phong_shader.SetFloat4( loc1, 0.8f, 0.8f, 0.8f, 1.0f );
		phong_shader.SetFloat4( loc2, 0.2f, 0.2f, 0.2f, 1.0f );

		phong_shader.SetVec3( phong_shader.GetVariable("cameraSpaceLightPos"), light_pos );
		phong_shader.SetFloat( phong_shader.GetVariable("lightAttenuation"), 0.02f );
		phong_shader.SetFloat( phong_shader.GetVariable("shininess"), 10.0f );
	}


	void draw_buffered_shapes()
	{
		// TODO find out how expensive it is to sort opaque and translucent objects into buckets...
		// probably better methods out there, also, these two buckets should only be allocated once
		// instead of alloc/dealloc on each call to draw_buffered....
		std::vector<BaseState3D*> opaque;
		std::vector<BaseState3D*> translucent;

		for (unsigned int i=0; i<buffered_shapes.size(); i++)
		{
			if ( buffered_shapes[i]->blend_mode != blending::SOLID_BLEND )
			{
				translucent.push_back( buffered_shapes[i] );
			}else{
				opaque.push_back( buffered_shapes[i] );
			}
		}

		num_opaque = opaque.size();
		num_blended = translucent.size();

		//////////////////////////////////////////////////////////////////////////
		phong_shader.begin();
		_init_phong();

		unsigned int projLoc = phong_shader.GetVariable("projMatrix");
		unsigned int mvLoc = phong_shader.GetVariable("modelviewMatrix");
		
		VSML *vsml = VSML::getInstance();
		vsml->initUniformLocs(mvLoc, projLoc);
		
		vsml->matrixToUniform(VSML::PROJECTION);

		unsigned int colorLoc = phong_shader.GetVariable("diffuseColor");
		//////////////////////////////////////////////////////////////////////////


		// useful article on blending
		// http://blogs.msdn.com/b/shawnhar/archive/2009/02/18/depth-sorting-alpha-blended-objects.aspx

		for (unsigned int i=0; i<opaque.size(); i++)
		{
				BaseState3D *state = opaque[i];
				phong_shader.SetVec4( colorLoc, state->color );
				state->pre_draw( vsml, camera.getViewMatrix() );
				state->draw(&geo_lib);	
		}

		struct SortFunctor : public std::binary_function<BaseState3D*,BaseState3D*,bool> 
		{
			SortFunctor( const FirstPersonCamera &cam ) : camera( cam )
			{
			}

			// a < b ? 
			bool operator() ( BaseState3D* a, BaseState3D* b )
			{
				return a->distance_from_camera(camera.pos) > b->distance_from_camera(camera.pos); // the one furthest away is to be drawn first.
			}

			const FirstPersonCamera &camera;
		};

		std::sort( translucent.begin(), translucent.end(), SortFunctor(camera) );

		if ( translucent.size() > 0 )
		{
			glDepthMask(GL_FALSE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glEnable(GL_BLEND);
		}
		for (unsigned int i=0; i<translucent.size(); i++)
		{
			BaseState3D *state = translucent[i];
			phong_shader.SetVec4( colorLoc, state->color );
			state->pre_draw( vsml, camera.getViewMatrix() );
			translucent[i]->draw( &geo_lib );
		}
		if ( translucent.size() > 0 )
		{
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}

		opaque.clear();
		translucent.clear();
		buffered_shapes.clear();
	}

	void draw_buffered_objects()
	{
		// Draw 3D first, set up our matrices
		VSML *vsml = VSML::getInstance();

		
		glm::mat4 projection =
			glm::perspective(60.0f, xres/(float)yres, 1.0f, 1000.f);
		vsml->loadMatrix( VSML::PROJECTION, glm::value_ptr(projection) );

		// We want depth test enabled for 3D
		glEnable( GL_DEPTH_TEST );

		// turn on culling to make things go a little faster
		glEnable( GL_CULL_FACE );

		// http://www.opengl.org/sdk/docs/man/xhtml/glCullFace.xml
		glCullFace( GL_BACK );

		if ( buffered_shapes.size() > 0 ) 
			draw_buffered_shapes();

		// for 2D we want overdraw in all cases, so turn depth test off
		glDisable( GL_DEPTH_TEST );

		// Setup projection for 2D, but now modelview, as we will modify 
		// modelview in most 2D drawcalls using translation, rotation and scaling.
		vsml->loadIdentity(VSML::PROJECTION);
		vsml->ortho(0.0f, (float)xres, (float)yres, 0.f, -1.f , 1.f);

		if ( buffered_lines.size() > 0 ) 
			draw_buffered_lines();

		if ( buffered_circles.size() > 0 ) 
			draw_buffered_circles();
	}






private:
	static ProtoGraphics *instance;
	bool isRunning;

	int xres, yres;
	int mousx, mousy;
	bool key_array[256];

	glm::vec4 colorState;
	glm::vec2 move_to_state;
	int blend_state;

	glm::vec3 light_pos;

	FirstPersonCamera camera;
	glm::mat4 mCam;

	GeometryLibrary geo_lib;
	
	Shader shader_2d;
	Shader shader_lines2d;
	Shader sphere_shader;
	Shader cylinder_shader;
	Shader cube_shader;
	Shader plane_shader;
	Shader phong_shader;
	
	std::vector< LineSegmentState > buffered_lines;
	std::vector< CircleState > buffered_circles;

	std::vector< BaseState3D* > buffered_shapes;

	std::vector< Shader* > shader_list;

	float delta;
	float time;
	float old_time;

	unsigned int numframes;

	int num_opaque;
	int num_blended;
};